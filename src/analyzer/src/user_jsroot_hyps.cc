// -*- C++ -*-

#include <iomanip>
#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <TCanvas.h>
#include <TGFileBrowser.h>
#include <TH1.h>
#include <TH2.h>
#include <THttpServer.h>
#include <TKey.h>
#include <TMath.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TText.h>
#include <TTimeStamp.h>
#include <TVector3.h>

#include <TEveManager.h>
#include <TEveBox.h>

#include <user_analyzer.hh>
#include <Unpacker.hh>
#include <UnpackerConfig.hh>
#include <UnpackerManager.hh>
#include <DAQNode.hh>
#include <filesystem_util.hh>

#include "AftHelper.hh"
#include "ConfMan.hh"
#include "DetectorID.hh"
#include "DCAnalyzer.hh"
#include "DCDriftParamMan.hh"
#include "DCGeomMan.hh"
#include "DCTdcCalibMan.hh"
#include "EMCParamMan.hh"
#include "EventAnalyzer.hh"
#include "FiberCluster.hh"
#include "FiberHit.hh"
#include "HistMaker.hh"
#include "HodoAnalyzer.hh"
#include "HodoParamMan.hh"
#include "HodoPHCMan.hh"
#include "HodoRawHit.hh"
#include "HttpServer.hh"
#include "MacroBuilder.hh"
#include "MatrixParamMan.hh"
#include "MsTParamMan.hh"
#include "UserParamMan.hh"

#define DEBUG    0
#define FLAG_DAQ 1

namespace
{
using hddaq::unpacker::GConfig;
using hddaq::unpacker::GUnpacker;
using hddaq::unpacker::DAQNode;
std::vector<TH1*> hptr_array;
const auto& gUnpacker = GUnpacker::get_instance();
auto&       gHist     = HistMaker::getInstance();
auto&       gHttp     = HttpServer::GetInstance();
auto&       gMatrix   = MatrixParamMan::GetInstance();
const auto& gAftHelper = AftHelper::GetInstance();
auto&       gMsT      = MsTParamMan::GetInstance();
const auto& gUser     = UserParamMan::GetInstance();
}

namespace analyzer
{

//____________________________________________________________________________
int
process_begin(const std::vector<std::string>& argv)
{
  // gROOT->SetBatch(kTRUE);
  gStyle->SetOptStat(1110);
  gStyle->SetTitleW(.4);
  gStyle->SetTitleH(.1);
  // gStyle->SetStatW(.42);
  // gStyle->SetStatH(.35);
  gStyle->SetStatW(.32);
  gStyle->SetStatH(.25);
  // gStyle->SetPalette(55);

  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  gConfMan.InitializeParameter<HodoParamMan>("HDPRM");
  gConfMan.InitializeParameter<HodoPHCMan>("HDPHC");
  gConfMan.InitializeParameter<DCGeomMan>("DCGEO");
  gConfMan.InitializeParameter<DCTdcCalibMan>("DCTDC");
  gConfMan.InitializeParameter<DCDriftParamMan>("DCDRFT");
  gConfMan.InitializeParameter<UserParamMan>("USER");
  if(!gConfMan.IsGood()) return -1;

  Int_t port = 9090;
  if(argv.size()==4)
    port = TString(argv[3]).Atoi();

  gHttp.SetPort(port);
  gHttp.Open();
  gHttp.CreateItem("/", "Online Analyzer");
  gHttp.CreateItem("/Tag", "Tag Check");
  gHttp.SetItemField("/Tag", "_kind", "Text");
  std::stringstream ss;
  ss << "<div style='color: white; background-color: black;"
     << "width: 100%; height: 100%;'>"
     << "Tag cheker is running" << "</div>";
  gHttp.SetItemField("/Tag", "value", ss.str().c_str());
  gHttp.Register(gHist.createDC1());
  gHttp.Register(gHist.createDC2());
  gHttp.Register(gHist.createDC3());
  gHttp.Register(gHist.createTOF());
  gHttp.Register(gHist.createCorrelation());
  gHttp.Register(gHist.createTriggerFlag());
  gHttp.Register(gHist.createDAQ());
  gHttp.Register(gHist.createDCEff());
  gHttp.Register(gHist.createBTOF());

  if(0 != gHist.setHistPtr(hptr_array)){ return -1; }

  //___ Macro for HttpServer
  // gHttp.Register(http::SDC1TDCTOT());
  // gHttp.Register(http::SDC1HitMulti());
  // gHttp.Register(http::SDC2TDCTOT());
  // gHttp.Register(http::SDC2HitMulti());
  // gHttp.Register(http::SDC3TDCTOT());
  // gHttp.Register(http::SDC3HitMulti());
  // gHttp.Register(http::TOFADCU());
  // gHttp.Register(http::TOFADCD());
  // gHttp.Register(http::TOFTDCU());
  // gHttp.Register(http::TOFTDCD());
  // gHttp.Register(http::TriggerFlagU());
  // gHttp.Register(http::TriggerFlagD());
  // gHttp.Register(http::TriggerFlagHitPat());
  // gHttp.Register(http::HitPatternBeam());
  // gHttp.Register(http::HitPatternScat());
  // gHttp.Register(http::BcOutEfficiency());
  // gHttp.Register(http::SdcInOutEfficiency());
  // gHttp.Register(http::Correlation());
  gHttp.Register(http::DAQ());

  for(Int_t i=0, n=hptr_array.size(); i<n; ++i){
    hptr_array[i]->SetDirectory(0);
  }

  return 0;
}

//____________________________________________________________________________
int
process_end(void)
{
  hptr_array.clear();
  return 0;
}

//____________________________________________________________________________
int
process_event(void)
{
  static Int_t run_number = -1;
  {
    if(run_number != gUnpacker.get_root()->get_run_number()){
      for(Int_t i=0, n=hptr_array.size(); i<n; ++i){
	hptr_array[i]->Reset();
      }
      run_number = gUnpacker.get_root()->get_run_number();
    }
  }
  auto event_number = gUnpacker.get_event_number();

  { ///// Tag Checker
    static const auto& gConfig = GConfig::get_instance();
    static const TString tout(gConfig.get_control_param("tout"));
    std::stringstream ss;
    ss << "<div style='color: white; background-color: black;"
       << "width: 100%; height: 100%;'>";
    ss << "RUN " << run_number << "   Event " << event_number
       << "<br>";
    if(!gUnpacker.is_good()){
      std::ifstream ifs(tout);
      if(ifs.good()){
	TString buf;
	while(!ifs.eof()){
	  buf.ReadLine(ifs, false);
	  if(buf.Contains("!") && !buf.Contains("............!"))
	    buf = "<font color='yellow'>" + buf + "</font>";
	  // if(buf.Contains("bUuSELselthdg")){
	  //   ss << TString(' ', 24);
	  ss << buf << "<br>";
	}
	ifs.close();
	hddaq::tag_summary.seekp(0, std::ios_base::beg);
      }else{
	ss << Form("Failed to read %s", tout.Data());
      }
      ss << "</div>";
      gHttp.SetItemField("/Tag", "value", ss.str().c_str());
    }
  }

  // TriggerFlag ---------------------------------------------------
  std::bitset<NumOfSegTFlag> trigger_flag;
  {
    static const Int_t k_device = gUnpacker.get_device_id("TFlag");
    static const Int_t k_tdc    = gUnpacker.get_data_id("TFlag", "tdc");
    static const Int_t tdc_id   = gHist.getSequentialID(kTriggerFlag, 0, kTDC);
    static const Int_t hit_id   = gHist.getSequentialID(kTriggerFlag, 0, kHitPat);
    for(Int_t seg=0; seg<NumOfSegTFlag; ++seg){
      auto nhit = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      if(nhit > 0){
	auto tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc);
	if(tdc > 0){
	  trigger_flag.set(seg);
	  hptr_array[tdc_id+seg]->Fill(tdc);
	  hptr_array[hit_id]->Fill(seg);
	}
      }
    }
#if 0
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // Bool_t spill_end = (trigger_flag[trigger::kSpillOnEnd] |
  // 		      trigger_flag[trigger::kSpillOffEnd]);
  // Bool_t l1_flag =
  //   trigger_flag[trigger::kL1SpillOn] ||
  //   trigger_flag[trigger::kL1SpillOff] ||
  //   trigger_flag[trigger::kSpillOnEnd] ||
  //   trigger_flag[trigger::kSpillOffEnd];
  // if(!l1_flag)
  //   hddaq::cerr << "#W Trigger flag is missing : "
  // 		<< trigger_flag << std::endl;

#if TIME_STAMP
  // TimeStamp --------------------------------------------------------
  {
    static const auto hist_id = gHist.getSequentialID(kTimeStamp, 0, kTDC);
    Int_t i = 0;
    for(auto&& c : gUnpacker.get_root()->get_child_list()){
      if(!c.second)
	continue;
      auto t = gUnpacker.get_node_header(c.second->get_id(),
                                          DAQNode::k_unix_time);
      hptr_array[hist_id+i]->Fill(t);
      ++i;
    }
  }
#endif

#if FLAG_DAQ
  { ///// DAQ
    //___ node id
    static const Int_t k_eb = gUnpacker.get_fe_id("k18eb");
    std::vector<Int_t> vme_fe_id;
    std::vector<Int_t> hul_fe_id;
    std::vector<Int_t> ea0c_fe_id;
    std::vector<Int_t> vea0c_fe_id;
    for(auto&& c : gUnpacker.get_root()->get_child_list()){
      if(!c.second) continue;
      TString n = c.second->get_name();
      auto id = c.second->get_id();
      if(n.Contains("vme"))
	vme_fe_id.push_back(id);
      if(n.Contains("hul"))
	hul_fe_id.push_back(id);
      if(n.Contains("easiroc"))
	ea0c_fe_id.push_back(id);
      if(n.Contains("aft"))
	vea0c_fe_id.push_back(id);
    }

    //___ sequential id
    static const Int_t eb_hid = gHist.getSequentialID(kDAQ, kEB, kHitPat);
    static const Int_t vme_hid = gHist.getSequentialID(kDAQ, kVME, kHitPat2D);
    static const Int_t hul_hid = gHist.getSequentialID(kDAQ, kHUL, kHitPat2D);
    static const Int_t ea0c_hid = gHist.getSequentialID(kDAQ, kEASIROC, kHitPat2D);
    static const Int_t vea0c_hid = gHist.getSequentialID(kDAQ, kVMEEASIROC, kHitPat2D);
    Int_t multihit_hid = gHist.getSequentialID(kDAQ, 0, kMultiHitTdc);


    { //___ EB
      auto data_size = gUnpacker.get_node_header(k_eb, DAQNode::k_data_size);
      hptr_array[eb_hid]->Fill(data_size);
    }

    { //___ VME
      for(Int_t i=0, n=vme_fe_id.size(); i<n; ++i){
	auto data_size = gUnpacker.get_node_header(vme_fe_id[i], DAQNode::k_data_size);
        hptr_array[vme_hid]->Fill(i, data_size);
      }
    }

    { // EASIROC
      for(Int_t i=0, n=ea0c_fe_id.size(); i<n; ++i){
        auto data_size = gUnpacker.get_node_header(ea0c_fe_id[i], DAQNode::k_data_size);
        hptr_array[ea0c_hid]->Fill(i, data_size);
      }
    }

    { //___ HUL node
      for(Int_t i=0, n=hul_fe_id.size(); i<n; ++i){
        auto data_size = gUnpacker.get_node_header(hul_fe_id[i], DAQNode::k_data_size);
        hptr_array[hul_hid]->Fill(i, data_size);
      }
    }

    { //___ VMEEASIROC node
      for(Int_t i=0, n=vea0c_fe_id.size(); i<n; ++i){
        auto data_size = gUnpacker.get_node_header(vea0c_fe_id[i], DAQNode::k_data_size);
        hptr_array[vea0c_hid]->Fill(i, data_size);
      }
    }
    { //___ MultiHitTdc
      { // BC3
	static const Int_t k_device   = gUnpacker.get_device_id("BC3");
	static const Int_t k_leading  = gUnpacker.get_data_id("BC3", "leading");
	for(Int_t l=0; l<NumOfLayersBC3; ++l) {
	  for(Int_t w=0; w<NumOfWireBC3; ++w) {
	    Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	    hptr_array[multihit_hid]->Fill(w, nhit_l);
	  }
	  ++multihit_hid;
	}
      }
      { // BC4
	static const Int_t k_device   = gUnpacker.get_device_id("BC4");
	static const Int_t k_leading  = gUnpacker.get_data_id("BC4", "leading");
	for(Int_t l=0; l<NumOfLayersBC4; ++l) {
	  for(Int_t w=0; w<NumOfWireBC4; ++w) {
	    Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	    hptr_array[multihit_hid]->Fill(w, nhit_l);
	  }
	  ++multihit_hid;
	}
      }
      { // SDC1
	static const Int_t k_device   = gUnpacker.get_device_id("SDC1");
	static const Int_t k_leading  = gUnpacker.get_data_id("SDC1", "leading");
	for(Int_t l=0; l<NumOfLayersSDC1; ++l) {
	  for(Int_t w=0; w<NumOfWireSDC1; ++w) {
	    Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	    hptr_array[multihit_hid]->Fill(w, nhit_l);
	  }
	  ++multihit_hid;
	}
      }
      { // SDC2
	static const Int_t k_device   = gUnpacker.get_device_id("SDC2");
	static const Int_t k_leading  = gUnpacker.get_data_id("SDC2", "leading");
	for(Int_t l=0; l<NumOfLayersSDC2; ++l) {
	  for(Int_t w=0; w<NumOfWireSDC2; ++w) {
	    Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	    hptr_array[multihit_hid]->Fill(w, nhit_l);
	  }
	  ++multihit_hid;
	}
      }
    }
  }

#endif

  // DC1 -------------------------------------------------------------
  //for HUL MH-TDC
  std::vector< std::vector<Int_t> > DC1HitCont(NumOfLayersDC1);
  // std::vector< std::vector<Int_t> > DC1HitCont(6);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("DC1");
    static const Int_t k_leading  = gUnpacker.get_data_id("DC1", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("DC1", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcBC3", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcBC3", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("MinTotBcOut", 0);
    // static const Int_t tot_max = gUser.GetParameter("MinTotBcOut", 1);

    // sequential id
    static const Int_t dc1t_id     = gHist.getSequentialID(kDC1, 0, kTDC, 0);
    static const Int_t dc1tot_id   = gHist.getSequentialID(kDC1, 0, kADC, 0);
    static const Int_t dc1t1st_id  = gHist.getSequentialID(kDC1, 0, kTDC2D, 0);
    static const Int_t dc1hit_id   = gHist.getSequentialID(kDC1, 0, kHitPat, 0);
    static const Int_t dc1mul_id   = gHist.getSequentialID(kDC1, 0, kMulti, 0);
    static const Int_t dc1mulwt_id = gHist.getSequentialID(kDC1, 0, kMulti, NumOfLayersDC1);

    static const Int_t dc1t_wide_id     = gHist.getSequentialID(kDC1, 0, kTDC,    10);
    static const Int_t dc1t_ctot_id     = gHist.getSequentialID(kDC1, 0, kTDC,    kTOTcutOffset);
    static const Int_t dc1tot_ctot_id   = gHist.getSequentialID(kDC1, 0, kADC,    kTOTcutOffset);
    static const Int_t dc1tot2D_id      = gHist.getSequentialID(kDC1, 0, kADC2D,  0);
    static const Int_t dc1t1st_ctot_id  = gHist.getSequentialID(kDC1, 0, kTDC2D,  kTOTcutOffset);
    static const Int_t dc1t2D_id        = gHist.getSequentialID(kDC1, 0, kTDC2D,  20 + kTOTcutOffset);
    static const Int_t dc1t2D_ctot_id   = gHist.getSequentialID(kDC1, 0, kTDC2D,  30 + kTOTcutOffset);
    static const Int_t dc1hit_ctot_id   = gHist.getSequentialID(kDC1, 0, kHitPat, kTOTcutOffset);
    static const Int_t dc1mul_ctot_id   = gHist.getSequentialID(kDC1, 0, kMulti,  kTOTcutOffset);
    static const Int_t dc1mulwt_ctot_id = gHist.getSequentialID(kDC1, 0, kMulti, NumOfLayersDC1 + kTOTcutOffset);
    static const Int_t dc1self_corr_id  = gHist.getSequentialID(kDC1, kSelfCorr, 0, 0);


    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersDC1; ++l) {
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      for(Int_t w=0; w<NumOfWireDC1[l]; ++w) {
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if (nhit_l == 0) continue;

	Int_t hit_l_max = 0;
	Int_t hit_t_max = 0;

	if (nhit_l != 0) {
	  hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
	}
	if (nhit_t != 0) {
	  hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
	}

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[dc1hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for(Int_t m = 0; m<nhit_l; ++m) {
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[dc1t_id + l]->Fill(tdc);
	  hptr_array[dc1t_wide_id + l]->Fill(tdc); //TDCwide
	  if (tdc1st < tdc) tdc1st = tdc;

	  // tdc 2D
	  // hptr_array[dc1t2D_id + l]->Fill(w,tdc);

	  // Drift time check
	  if (tdc_min < tdc && tdc < tdc_max) {
	    flag_hit_wt = true;
	  }
	}

	if (tdc1st != 0)
	  hptr_array[dc1t1st_id +l]->Fill(tdc1st);
	if (flag_hit_wt) {
	  ++multiplicity_wt;
	  hptr_array[dc1hit_id + l]->Fill(w);
	}

	tdc1st = 0;
	if (nhit_l == nhit_t && hit_l_max > hit_t_max) {
	  ++multiplicity_ctot;
	  for(Int_t m = 0; m<nhit_l; ++m) {
	    // tdc   = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	    // tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
	    // tot   = tdc - tdc_t;
	    // hptr_array[dc1tot_id + l]->Fill(tot);
     	    // hptr_array[dc1tot2D_id + l]->Fill(w,tot); //2D
	    if (tot < tot_min) continue;
	    // hptr_array[dc1t_ctot_id + l]->Fill(tdc);
	    // hptr_array[dc1t2D_ctot_id + l]->Fill(w,tdc); //2D
	    // hptr_array[dc1tot_ctot_id + l]->Fill(tot);
	    if (tdc1st < tdc) tdc1st = tdc;
	    if (tdc_min < tdc && tdc < tdc_max) {
	      flag_hit_wt_ctot = true;
	    }
	  }
	}

	// if (tdc1st != 0) hptr_array[dc1t1st_ctot_id +l]->Fill(tdc1st);
	if (flag_hit_wt_ctot) {
	  ++multiplicity_wt_ctot;
	  // hptr_array[dc1hit_ctot_id + l]->Fill(w);
	  DC1HitCont[l].push_back(w);
	}
      }

      // hptr_array[dc1mul_id + l]->Fill(multiplicity);
      // hptr_array[dc1mulwt_id + l]->Fill(multiplicity_wt);
      // hptr_array[dc1mul_ctot_id   + l]->Fill(multiplicity_ctot);
      // hptr_array[dc1mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }


    // for(Int_t s=0; s<NumOfDimDC1 ;s++) {
    //   Int_t corr=2*s;
    //   for(UInt_t i=0; i<DC1HitCont[corr].size() ;i++) {
    // 	for(UInt_t j=0; j<DC1HitCont[corr+1].size() ;j++) {
    // 	  hptr_array[dc1self_corr_id + s]->Fill(DC1HitCont[corr][i], DC1HitCont[corr+1][j]);
    // 	}
    //   }
    // }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // DC2 -------------------------------------------------------------
  //for HUL MH-TDC
  std::vector< std::vector<Int_t> > DC2HitCont(NumOfLayersDC2);
  // std::vector< std::vector<Int_t> > DC2HitCont(6);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("DC2");
    static const Int_t k_leading  = gUnpacker.get_data_id("DC2", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("DC2", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcBC3", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcBC3", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("MinTotBcOut", 0);
    // static const Int_t tot_max = gUser.GetParameter("MinTotBcOut", 1);

    // sequential id
    static const Int_t dc2t_id     = gHist.getSequentialID(kDC2, 0, kTDC, 0);
    static const Int_t dc2tot_id   = gHist.getSequentialID(kDC2, 0, kADC, 0);
    static const Int_t dc2t1st_id  = gHist.getSequentialID(kDC2, 0, kTDC2D, 0);
    static const Int_t dc2hit_id   = gHist.getSequentialID(kDC2, 0, kHitPat, 0);
    static const Int_t dc2mul_id   = gHist.getSequentialID(kDC2, 0, kMulti, 0);
    static const Int_t dc2mulwt_id = gHist.getSequentialID(kDC2, 0, kMulti, NumOfLayersDC2);

    static const Int_t dc2t_wide_id     = gHist.getSequentialID(kDC2, 0, kTDC,    10);
    static const Int_t dc2t_ctot_id     = gHist.getSequentialID(kDC2, 0, kTDC,    kTOTcutOffset);
    static const Int_t dc2tot_ctot_id   = gHist.getSequentialID(kDC2, 0, kADC,    kTOTcutOffset);
    static const Int_t dc2tot2D_id      = gHist.getSequentialID(kDC2, 0, kADC2D,  0);
    static const Int_t dc2t1st_ctot_id  = gHist.getSequentialID(kDC2, 0, kTDC2D,  kTOTcutOffset);
    static const Int_t dc2t2D_id        = gHist.getSequentialID(kDC2, 0, kTDC2D,  20 + kTOTcutOffset);
    static const Int_t dc2t2D_ctot_id   = gHist.getSequentialID(kDC2, 0, kTDC2D,  30 + kTOTcutOffset);
    static const Int_t dc2hit_ctot_id   = gHist.getSequentialID(kDC2, 0, kHitPat, kTOTcutOffset);
    static const Int_t dc2mul_ctot_id   = gHist.getSequentialID(kDC2, 0, kMulti,  kTOTcutOffset);
    static const Int_t dc2mulwt_ctot_id = gHist.getSequentialID(kDC2, 0, kMulti, NumOfLayersDC2 + kTOTcutOffset);
    static const Int_t dc2self_corr_id  = gHist.getSequentialID(kDC2, kSelfCorr, 0, 0);


    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersDC2; ++l) {
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      for(Int_t w=0; w<NumOfWireDC2[l]; ++w) {
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if (nhit_l == 0) continue;

	Int_t hit_l_max = 0;
	Int_t hit_t_max = 0;

	if (nhit_l != 0) {
	  hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
	}
	if (nhit_t != 0) {
	  hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
	}

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[dc2hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for(Int_t m = 0; m<nhit_l; ++m) {
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[dc2t_id + l]->Fill(tdc);
	  hptr_array[dc2t_wide_id + l]->Fill(tdc); //TDCwide
	  if (tdc1st < tdc) tdc1st = tdc;

	  // tdc 2D
	  // hptr_array[dc2t2D_id + l]->Fill(w,tdc);

	  // Drift time check
	  if (tdc_min < tdc && tdc < tdc_max) {
	    flag_hit_wt = true;
	  }
	}

	if (tdc1st != 0)
	  hptr_array[dc2t1st_id + l]->Fill(tdc1st);
	if (flag_hit_wt) {
	  ++multiplicity_wt;
	  hptr_array[dc2hit_id + l]->Fill(w);
	}

	tdc1st = 0;
	if (nhit_l == nhit_t && hit_l_max > hit_t_max) {
	  ++multiplicity_ctot;
	  for(Int_t m = 0; m<nhit_l; ++m) {
	    tdc   = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	    tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
	    tot   = tdc - tdc_t;
	    // hptr_array[dc2tot_id+l]->Fill(tot);
     	    // hptr_array[dc2tot2D_id+l]->Fill(w,tot); //2D
	    if (tot < tot_min) continue;
	    // hptr_array[dc2t_ctot_id + l]->Fill(tdc);
	    // hptr_array[dc2t2D_ctot_id + l]->Fill(w,tdc); //2D
	    // hptr_array[dc2tot_ctot_id+l]->Fill(tot);
	    if (tdc1st < tdc) tdc1st = tdc;
	    if (tdc_min < tdc && tdc < tdc_max) {
	      flag_hit_wt_ctot = true;
	    }
	  }
	}

	if (tdc1st != 0) hptr_array[dc2t1st_ctot_id + l]->Fill(tdc1st);
	if (flag_hit_wt_ctot) {
	  ++multiplicity_wt_ctot;
	  // hptr_array[dc2hit_ctot_id + l]->Fill(w);
	  DC2HitCont[l].push_back(w);
	}
      }

      // hptr_array[dc2mul_id + l]->Fill(multiplicity);
      // hptr_array[dc2mulwt_id + l]->Fill(multiplicity_wt);
      // hptr_array[dc2mul_ctot_id   + l]->Fill(multiplicity_ctot);
      // hptr_array[dc2mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }


    // for(Int_t s=0; s<NumOfDimDC2 ;s++) {
    //   Int_t corr=2*s;
    //   for(UInt_t i=0; i<DC2HitCont[corr].size() ;i++) {
    // 	for(UInt_t j=0; j<DC2HitCont[corr+1].size() ;j++) {
    // 	  hptr_array[dc2self_corr_id + s]->Fill(DC2HitCont[corr][i], DC2HitCont[corr+1][j]);
    // 	}
    //   }
    // }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // DC3 -------------------------------------------------------------
  //for HUL MH-TDC
  std::vector< std::vector<Int_t> > DC3HitCont(NumOfLayersDC3);
  // std::vector< std::vector<Int_t> > DC3HitCont(6);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("DC3");
    static const Int_t k_leading  = gUnpacker.get_data_id("DC3", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("DC3", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcBC3", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcBC3", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("MinTotBcOut", 0);
    // static const Int_t tot_max = gUser.GetParameter("MinTotBcOut", 1);

    // sequential id
    static const Int_t dc3t_id     = gHist.getSequentialID(kDC3, 0, kTDC, 0);
    static const Int_t dc3tot_id   = gHist.getSequentialID(kDC3, 0, kADC, 0);
    static const Int_t dc3t1st_id  = gHist.getSequentialID(kDC3, 0, kTDC2D, 0);
    static const Int_t dc3hit_id   = gHist.getSequentialID(kDC3, 0, kHitPat, 0);
    static const Int_t dc3mul_id   = gHist.getSequentialID(kDC3, 0, kMulti, 0);
    static const Int_t dc3mulwt_id = gHist.getSequentialID(kDC3, 0, kMulti, NumOfLayersDC3);

    static const Int_t dc3t_wide_id     = gHist.getSequentialID(kDC3, 0, kTDC,    10);
    static const Int_t dc3t_ctot_id     = gHist.getSequentialID(kDC3, 0, kTDC,    kTOTcutOffset);
    static const Int_t dc3tot_ctot_id   = gHist.getSequentialID(kDC3, 0, kADC,    kTOTcutOffset);
    static const Int_t dc3tot2D_id      = gHist.getSequentialID(kDC3, 0, kADC2D,  0);
    static const Int_t dc3t1st_ctot_id  = gHist.getSequentialID(kDC3, 0, kTDC2D,  kTOTcutOffset);
    static const Int_t dc3t2D_id        = gHist.getSequentialID(kDC3, 0, kTDC2D,  20 + kTOTcutOffset);
    static const Int_t dc3t2D_ctot_id   = gHist.getSequentialID(kDC3, 0, kTDC2D,  30 + kTOTcutOffset);
    static const Int_t dc3hit_ctot_id   = gHist.getSequentialID(kDC3, 0, kHitPat, kTOTcutOffset);
    static const Int_t dc3mul_ctot_id   = gHist.getSequentialID(kDC3, 0, kMulti,  kTOTcutOffset);
    static const Int_t dc3mulwt_ctot_id = gHist.getSequentialID(kDC3, 0, kMulti, NumOfLayersDC3 + kTOTcutOffset);
    static const Int_t dc3self_corr_id  = gHist.getSequentialID(kDC3, kSelfCorr, 0, 0);


    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersDC3; ++l) {
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      for(Int_t w=0; w<NumOfWireDC3[l]; ++w) {
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if (nhit_l == 0) continue;

	Int_t hit_l_max = 0;
	Int_t hit_t_max = 0;

	if (nhit_l != 0) {
	  hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
	}
	if (nhit_t != 0) {
	  hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
	}

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[dc3hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for(Int_t m = 0; m<nhit_l; ++m) {
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[dc3t_id + l]->Fill(tdc);
	  hptr_array[dc3t_wide_id + l]->Fill(tdc); //TDCwide
	  if (tdc1st < tdc) tdc1st = tdc;

	  // tdc 2D
	  // hptr_array[dc3t2D_id + l]->Fill(w,tdc);

	  // drift time check
	  if (tdc_min < tdc && tdc < tdc_max) {
	    flag_hit_wt = true;
	  }
	}

	if (tdc1st != 0)
	  hptr_array[dc3t1st_id +l]->Fill(tdc1st);
	if (flag_hit_wt) {
	  ++multiplicity_wt;
	  hptr_array[dc3hit_id + l]->Fill(w);
	}

	tdc1st = 0;
	if (nhit_l == nhit_t && hit_l_max > hit_t_max) {
	  ++multiplicity_ctot;
	  for(Int_t m = 0; m<nhit_l; ++m) {
	    tdc   = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	    tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
	    tot   = tdc - tdc_t;
	    // hptr_array[dc3tot_id+l]->Fill(tot);
     	    // hptr_array[dc3tot2D_id+l]->Fill(w,tot); //2D
	    if (tot < tot_min) continue;
	    // hptr_array[dc3t_ctot_id + l]->Fill(tdc);
	    // hptr_array[dc3t2D_ctot_id + l]->Fill(w,tdc); //2D
	    // hptr_array[dc3tot_ctot_id + l]->Fill(tot);
	    if (tdc1st < tdc) tdc1st = tdc;
	    if (tdc_min < tdc && tdc < tdc_max) {
	      flag_hit_wt_ctot = true;
	    }
	  }
	}

	if (tdc1st!=0) hptr_array[dc3t1st_ctot_id +l]->Fill(tdc1st);
	if (flag_hit_wt_ctot) {
	  ++multiplicity_wt_ctot;
	  // hptr_array[dc3hit_ctot_id + l]->Fill(w);
	  DC3HitCont[l].push_back(w);
	}
      }

      // hptr_array[dc3mul_id + l]->Fill(multiplicity);
      // hptr_array[dc3mulwt_id + l]->Fill(multiplicity_wt);
      // hptr_array[dc3mul_ctot_id   + l]->Fill(multiplicity_ctot);
      // hptr_array[dc3mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }


    // for(Int_t s=0; s<NumOfDimDC3 ;s++) {
    //   Int_t corr=2*s;
    //   for(UInt_t i=0; i<DC3HitCont[corr].size() ;i++) {
    // 	for(UInt_t j=0; j<DC3HitCont[corr+1].size() ;j++) {
    // 	  hptr_array[bc3self_corr_id + s]->Fill(DC3HitCont[corr][i], DC3HitCont[corr+1][j]);
    // 	}
    //   }
    // }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  std::vector<Int_t> hitseg_tof;
  { ///// TOF
    static const auto device_id = gUnpacker.get_device_id("TOF");
    static const auto adc_id = gUnpacker.get_data_id("TOF", "adc");
    static const auto tdc_id = gUnpacker.get_data_id("TOF", "tdc");
    static const auto tdc_min = gUser.GetParameter("TdcTOF", 0);
    static const auto tdc_max = gUser.GetParameter("TdcTOF", 1);
    static const auto adc_hid = gHist.getSequentialID(kTOF, 0, kADC);
    static const auto tdc_hid = gHist.getSequentialID(kTOF, 0, kTDC);
    static const auto awt_hid = gHist.getSequentialID(kTOF, 0, kADCwTDC);
    static const auto hit_hid = gHist.getSequentialID(kTOF, 0, kHitPat);
    static const auto mul_hid = gHist.getSequentialID(kTOF, 0, kMulti);
    std::vector<std::vector<Int_t>> hit_flag(NumOfSegTOF);
    Int_t multiplicity = 0;
    for(Int_t seg=0; seg<NumOfSegTOF; ++seg) {
      hit_flag[seg].resize(kUorD);
      for(Int_t ud=0; ud<kUorD; ++ud) {
	hit_flag[seg][ud] = 0;
	// ADC
	UInt_t adc = 0;
	auto nhit = gUnpacker.get_entries(device_id, 0, seg, ud, adc_id);
	if (nhit != 0) {
	  adc = gUnpacker.get(device_id, 0, seg, ud, adc_id);
	  hptr_array[adc_hid + ud*NumOfSegTOF + seg]->Fill(adc);
	}
	// TDC
	for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, seg, ud, tdc_id);
	    m<n; ++m) {
	  auto tdc = gUnpacker.get(device_id, 0, seg, ud, tdc_id, m);
	  if (tdc != 0) {
	    hptr_array[tdc_hid + ud*NumOfSegTOF + seg]->Fill(tdc);
	    if (tdc_min<tdc && tdc<tdc_max && adc > 0) {
	      hit_flag[seg][ud] = 1;
	    }
	  }
	}
	if (hit_flag[seg][ud] == 1) {
	  hptr_array[awt_hid + ud*NumOfSegTOF + seg]->Fill(adc);
	}
      }
      if (hit_flag[seg][kU] == 1 && hit_flag[seg][kD] == 1) {
	++multiplicity;
	hptr_array[hit_hid]->Fill(seg);
	hitseg_tof.push_back(seg);
      }
    }
    hptr_array[mul_hid]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

  // if(spill_end)
  //   return 0;

  // Update
  // if(gUnpacker.get_counter()%100 == 0){
  //   auto prev_level = gErrorIgnoreLevel;
  //   gErrorIgnoreLevel = kError;
  //   http::UpdateBcOutEfficiency();
  //   http::UpdateSdcInOutEfficiency();
  //   // http::UpdateT0PeakFitting();
  //   http::UpdateTOTPeakFitting();
  //   http::UpdateAFTEfficiency();
  //   gErrorIgnoreLevel = prev_level;
  // }

  // if(!gUnpacker.is_good()){
  //   std::cout << "[Warning] Tag is not good." << std::endl;
  //   static const TString host(gSystem->Getenv("HOSTNAME"));
  //   static auto prev_time = std::time(0);
  //   auto        curr_time = std::time(0);
  //   if(host.Contains("k18term4") &&
  //      event_number > 1 && curr_time - prev_time > 5){
  //     std::cout << "exec tagslip sound!" << std::endl;
  //     gSystem->Exec("ssh k18epics.monitor.k18net \"aplay ~/sound/tagslip.wav\" &");
  //   }
  //   prev_time = curr_time;
  // }

  gSystem->ProcessEvents();

  return 0;
}

}
