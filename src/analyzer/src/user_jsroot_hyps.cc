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

// #include "AftHelper.hh"
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
#define TAG_PL_ADCNo 0


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
// const auto& gAftHelper = AftHelper::GetInstance();
auto&       gMsT      = MsTParamMan::GetInstance();
const auto& gUser     = UserParamMan::GetInstance();
const auto& gHodo     = HodoParamMan::GetInstance();
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
  gConfMan.InitializeParameter<UserParamMan>("USER");
  gConfMan.InitializeParameter<DCGeomMan>("DCGEO");
  gConfMan.InitializeParameter<DCTdcCalibMan>("DCTDC");
  gConfMan.InitializeParameter<DCDriftParamMan>("DCDRFT");

  if(!gConfMan.IsGood()) return -1;

  Int_t port = 9090;
  if(argv.size()==4)
    port = TString(argv[3]).Atoi();

  gHttp.SetPort(port);
  gHttp.Open();
  gHttp.CreateItem("/", "HYPS Online Analyzer");
  gHttp.CreateItem("/Tag", "Tag Check");
  gHttp.SetItemField("/Tag", "_kind", "Text");
  std::stringstream ss;
  ss << "<div style='color: white; background-color: black;"
     << "width: 100%; height: 100%;'>"
     << "Tag cheker is running" << "</div>";
  gHttp.SetItemField("/Tag", "value", ss.str().c_str());
  gHttp.Register(gHist.createRF());
  gHttp.Register(gHist.createTAG_SF());
  gHttp.Register(gHist.createTAG_PL());
  gHttp.Register(gHist.createV1725_STOP());
  gHttp.Register(gHist.createU_Veto());
  gHttp.Register(gHist.createT0());
  gHttp.Register(gHist.createSAC());
  gHttp.Register(gHist.createSDC0());
  gHttp.Register(gHist.createSDC1());
  gHttp.Register(gHist.createSDC2());
  gHttp.Register(gHist.createSDC3());
  gHttp.Register(gHist.createE_Veto());
  gHttp.Register(gHist.createTOF());
  // gHttp.Register(gHist.createCorrelation());
  // gHttp.Register(gHist.createTriggerFlag());
  // gHttp.Register(gHist.createDCEff());
  // gHttp.Register(gHist.createBTOF());
  gHttp.Register(gHist.createCFT());
  gHttp.Register(gHist.createCatchBGO());
  gHttp.Register(gHist.createPiID());
  gHttp.Register(gHist.createCorrelation_catch());
  gHttp.Register(gHist.createDAQ());

  if(0 != gHist.setHistPtr(hptr_array)){ return -1; }

  //___ Macro for HttpServer
  gHttp.Register(http::Counter_TDC());
  gHttp.Register(http::SDCIn_HitPat());
  gHttp.Register(http::SDCOut_HitPat());
  gHttp.Register(http::SDC_Correlation());
  gHttp.Register(http::CFTHighGain2D_check1());
  gHttp.Register(http::CFTHighGain2D_check2());
  gHttp.Register(http::CFTHitPat());
  gHttp.Register(http::BGOHitMulti());
  gHttp.Register(http::PiIDADC2DHitMulti());
  gHttp.Register(http::TAG_SFF_TDC1());
  gHttp.Register(http::TAG_SFF_TDC2());
  gHttp.Register(http::TAG_SFF_TDC3());
  gHttp.Register(http::TAG_SFF_TDC4());
  gHttp.Register(http::TAG_SFF_TDC5());
  gHttp.Register(http::TAG_SFB_TDC1());
  gHttp.Register(http::TAG_SFB_TDC2());
  gHttp.Register(http::TAG_SFB_TDC3());
  gHttp.Register(http::TAG_SFB_TDC4());
  gHttp.Register(http::TAG_SFB_TDC5());
  gHttp.Register(http::TAG_PL_TDC());
  gHttp.Register(http::TAG_PL_FADC_V1725_STOP());
  gHttp.Register(http::TAG_Multi());
  gHttp.Register(http::U_Veto());
  gHttp.Register(http::T0());
  gHttp.Register(http::SAC());
  gHttp.Register(http::SDCIn_TDC());
  gHttp.Register(http::SDCIn_TDC1st());
  gHttp.Register(http::SDCIn_TOT());
  gHttp.Register(http::SDCIn_Multi());
  gHttp.Register(http::SDCIn_TDC2D());
  gHttp.Register(http::SDCIn_TDC2DC());
  gHttp.Register(http::SDCIn_TOTTDC2D());
  gHttp.Register(http::SDCOut_TDC());
  gHttp.Register(http::SDCOut_TDC1st());
  gHttp.Register(http::SDCOut_TOT());
  gHttp.Register(http::SDCOut_Multi());
  gHttp.Register(http::SDCOut_TDC2D());
  gHttp.Register(http::SDCOut_TDC2DC());
  gHttp.Register(http::SDCOut_TOTTDC2D());
  gHttp.Register(http::E_Veto());
  gHttp.Register(http::TOF_ADCU1());
  gHttp.Register(http::TOF_ADCU2());
  gHttp.Register(http::TOF_ADCU3());
  gHttp.Register(http::TOF_ADCD1());
  gHttp.Register(http::TOF_ADCD2());
  gHttp.Register(http::TOF_ADCD3());
  gHttp.Register(http::TOF_TDCU1());
  gHttp.Register(http::TOF_TDCU2());
  gHttp.Register(http::TOF_TDCU3());
  gHttp.Register(http::TOF_TDCD1());
  gHttp.Register(http::TOF_TDCD2());
  gHttp.Register(http::TOF_TDCD3());
  gHttp.Register(http::CFTTDC());
  gHttp.Register(http::CFTTDC2D());
  gHttp.Register(http::CFTTOT());
  gHttp.Register(http::CFTTOT2D());
  gHttp.Register(http::CFTCTOT2D());
  gHttp.Register(http::CFTHighGain());
  gHttp.Register(http::CFTLowGain());
  gHttp.Register(http::CFTPedestal());
  gHttp.Register(http::CFTHighGain2D());
  gHttp.Register(http::CFTLowGain2D());
  gHttp.Register(http::CFTPedestal2D());
  gHttp.Register(http::CFTCHighGain2D());
  gHttp.Register(http::CFTCLowGain2D());
  gHttp.Register(http::CFTHighGainxTOT());
  gHttp.Register(http::CFTLowGainxTOT());
  gHttp.Register(http::CFTMulti());
  gHttp.Register(http::CFTClusterHighGain());
  gHttp.Register(http::CFTClusterLowGain());
  gHttp.Register(http::CFTClusterHighGain2D());
  gHttp.Register(http::CFTClusterLowGain2D());
  gHttp.Register(http::CFTClusterTDC());
  gHttp.Register(http::CFTClusterTDC2D());
  gHttp.Register(http::BGOFADC());
  gHttp.Register(http::BGOFADCwTDC());
  gHttp.Register(http::BGOADC());
  gHttp.Register(http::BGOTDC());
  gHttp.Register(http::BGOADCTDC2D());
  gHttp.Register(http::PiIDTDC());
  gHttp.Register(http::PiIDHighGain());
  gHttp.Register(http::PiIDLowGain());
  gHttp.Register(http::Correlation_CATCH());
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
  gSystem->ProcessEvents();
  static Int_t run_number = -1;
  {
    if(run_number != gUnpacker.get_root()->get_run_number()){
      if(run_number != -1)
	gHttp.MakePs(run_number);
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

#if 0
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

#if FLAG_DAQ
  { ///// DAQ
    //___ node id
    static const Int_t k_eb = gUnpacker.get_fe_id("k18eb");
    std::vector<Int_t> vme_fe_id;
    std::vector<Int_t> hul_fe_id;
    std::vector<Int_t> vea0c_fe_id;
    for(auto&& c : gUnpacker.get_root()->get_child_list()){
      if(!c.second) continue;
      TString n = c.second->get_name();
      auto id = c.second->get_id();
      if(n.Contains("vme") || n.Contains("optlink"))
	vme_fe_id.push_back(id);
      if(n.Contains("hul"))
	hul_fe_id.push_back(id);
      if(n.Contains("easiroc"))
	vea0c_fe_id.push_back(id);
    }

    //___ sequential id
    static const Int_t eb_hid = gHist.getSequentialID(kDAQ, kEB, kHitPat);
    static const Int_t vme_hid = gHist.getSequentialID(kDAQ, kVME, kHitPat2D);
    static const Int_t hul_hid = gHist.getSequentialID(kDAQ, kHUL, kHitPat2D);
    static const Int_t vea0c_hid = gHist.getSequentialID(kDAQ, kVEASIROC, kHitPat2D);
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
    // { //___ MultiHitTdc
    //   { // SDC1
    // 	static const Int_t k_device   = gUnpacker.get_device_id("SDC1");
    // 	static const Int_t k_leading  = gUnpacker.get_data_id("SDC1", "leading");
    // 	for(Int_t l=0; l<NumOfLayersSDC1; ++l) {
    // 	  for(Int_t w=0; w<NumOfWireSDC1; ++w) {
    // 	    Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
    // 	    hptr_array[multihit_hid]->Fill(w, nhit_l);
    // 	  }
    // 	  ++multihit_hid;
    // 	}
    //   }
    //   { // SDC2
    // 	static const Int_t k_device   = gUnpacker.get_device_id("SDC2");
    // 	static const Int_t k_leading  = gUnpacker.get_data_id("SDC2", "leading");
    // 	for(Int_t l=0; l<NumOfLayersSDC2; ++l) {
    // 	  for(Int_t w=0; w<NumOfWireSDC2; ++w) {
    // 	    Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
    // 	    hptr_array[multihit_hid]->Fill(w, nhit_l);
    // 	  }
    // 	  ++multihit_hid;
    // 	}
    //   }
    // }
  }

#endif

  //------------------------------------------------------------------
  // RF
  //------------------------------------------------------------------
  std::vector<Int_t> hitseg_RF;
  { ///// RF
    static const auto device_id = gUnpacker.get_device_id("RF");
    static const auto adc_id    = gUnpacker.get_data_id("RF", "adc");
    static const auto tdc_id    = gUnpacker.get_data_id("RF", "tdc");
    static const auto tdc_min   = gUser.GetParameter("TdcRF", 0);
    static const auto tdc_max   = gUser.GetParameter("TdcRF", 1);
    static const auto adc_hid   = gHist.getSequentialID(kRF, 0, kADC,     0);
    static const auto tdc_hid   = gHist.getSequentialID(kRF, 0, kTDC,     0);
    static const auto awt_hid   = gHist.getSequentialID(kRF, 0, kADCwTDC, 0);
    static const auto hit_hid   = gHist.getSequentialID(kRF, 0, kHitPat,  0);
    static const auto mul_hid   = gHist.getSequentialID(kRF, 0, kMulti,   0);
    Int_t multiplicity = 0;
    for(Int_t seg=0; seg<NumOfSegRF; ++seg) {
      // ADC
      UInt_t adc = 0;
      auto nhit = gUnpacker.get_entries(device_id, 0, seg, 0, adc_id);
      if (nhit != 0) {
	adc = gUnpacker.get(device_id, 0, seg, 0, adc_id);
	hptr_array[adc_hid + seg]->Fill(adc);
      }
      // TDC
      Bool_t is_in_gate = false;
      for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, seg, 0, tdc_id);
	  m<n; ++m) {
	auto tdc = gUnpacker.get(device_id, 0, seg, 0, tdc_id, m);
	if (tdc != 0) {
	  hptr_array[tdc_hid + seg]->Fill(tdc);
	  if (tdc_min<tdc && tdc<tdc_max && adc > 0) {
	    is_in_gate = true;
	  }
	}
	if (is_in_gate) {
	  if (gUnpacker.get_entries(device_id, 0, seg, 0, adc_id)>0){
	    Int_t adc = gUnpacker.get(device_id, 0, seg, adc_id);
	    hptr_array[awt_hid + seg]->Fill(adc);
	  }
	}
	++multiplicity;
	hptr_array[hit_hid]->Fill(seg);
      }
    }
    hptr_array[mul_hid]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

#if 1
  std::vector<Int_t> hitseg_SFF;
  std::vector<Int_t> hitseg_SFB;
  { ///// TAG_SF
    static const auto device_id = gUnpacker.get_device_id("TAG-SF");
    static const auto tdc_id    = gUnpacker.get_data_id("TAG-SF", "tdc");
    static const auto tdc_min   = gUser.GetParameter("TdcSF", 0);
    static const auto tdc_max   = gUser.GetParameter("TdcSF", 1);
    //static const auto tdc_min   = 600;
    //static const auto tdc_max   = 700;
    static const auto tdc_hid   = gHist.getSequentialID(kTAG_SF, 0, kTDC,     0);
    static const auto hit_hid   = gHist.getSequentialID(kTAG_SF, 0, kHitPat,  0);
    static const auto mul_hid   = gHist.getSequentialID(kTAG_SF, 0, kMulti,   0);
    Int_t multiplicity = 0;
    for(Int_t l=0; l<NumOfLayersTAG_SF;++l){
      for(Int_t seg=0; seg<NumOfSegTAG_SF; ++seg) {
	Int_t tdc=0;
	// TDC
	for(Int_t m=0, n=gUnpacker.get_entries(device_id, l, seg, 0, tdc_id);
	    m<n; ++m) {
	  Bool_t is_in_gate = false;
	  tdc = gUnpacker.get(device_id, l, seg, 0, tdc_id, m);
	  if (tdc != 0) {
	    hptr_array[tdc_hid + l*NumOfSegTAG_SF + seg]->Fill(tdc);
	    if (tdc_min<tdc && tdc<tdc_max) {
	      is_in_gate = true;
	    }
	  }
	  if (is_in_gate) {
	    ++multiplicity;
	    hptr_array[hit_hid + l]->Fill(seg);
	  }
	}
      }
      hptr_array[mul_hid + l]->Fill(multiplicity);
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  std::vector<Int_t> hitseg_PLF;
  std::vector<Int_t> hitseg_PLB;
  { ///// TAG_PL
    static const auto device_id  = gUnpacker.get_device_id("TAG-PL");
    // static const auto adc_id     = gUnpacker.get_data_id("TAG-PL", "adc");
    static const auto fadc_id    = gUnpacker.get_data_id("TAG-PL", "adc");
    static const auto tdc_id     = gUnpacker.get_data_id("TAG-PL", "tdc");
    static const auto tdc_min    = gUser.GetParameter("TdcPL", 0);
    static const auto tdc_max    = gUser.GetParameter("TdcPL", 1);
    //static const auto tdc_min   = 580;
    //static const auto tdc_max   = 640;
    // static const auto adc_hid   = gHist.getSequentialID(kTAG_PL, 0, kADC,     0);
    static const auto fadc_hid   = gHist.getSequentialID(kTAG_PL, 0, kFADC,    0);
    static const auto tdc_hid    = gHist.getSequentialID(kTAG_PL, 0, kTDC,     0);
    static const auto awt_hid    = gHist.getSequentialID(kTAG_PL, 0, kADCwTDC, 0);
    static const auto hit_hid    = gHist.getSequentialID(kTAG_PL, 0, kHitPat,  0);
    static const auto mul_hid    = gHist.getSequentialID(kTAG_PL, 0, kMulti,   0);
    Int_t multiplicity = 0;
    for(Int_t seg=0; seg<NumOfSegTAG_PL; ++seg) {
      Int_t adc=0;
      Int_t tdc=0;
      Int_t fadc=0;
#if TAG_PL_ADCNo
      // ADC
      auto nhit = gUnpacker.get_entries(device_id, 0, seg, 0, adc_id);
      if (nhit != 0) {
	adc = gUnpacker.get(device_id, 0, seg, 0, adc_id);
	hptr_array[adc_hid + seg]->Fill(adc);
      }
#endif

      // FADC
      auto nhit_f = gUnpacker.get_entries(device_id, 0, seg, 0, fadc_id);
      for (Int_t m=0; m<nhit_f; ++m) {
	fadc = gUnpacker.get(device_id, 0, seg, 0, fadc_id, m);
	hptr_array[fadc_hid + seg]->Fill(m, fadc);
      }

      // TDC
      for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, seg, 0, tdc_id);
	  m<n; ++m) {
	Bool_t is_in_gate = false;
	tdc = gUnpacker.get(device_id, 0, seg, 0, tdc_id, m);
	if (tdc != 0) {
	  hptr_array[tdc_hid + seg]->Fill(tdc);
	  if (tdc_min<tdc && tdc<tdc_max) {
	    is_in_gate = true;
	  }
	}
	if (is_in_gate) {
#if TAG_PL_ADCNo
	  if (gUnpacker.get_entries(device_id, 0, seg, 0, adc_id)>0){
	    Int_t adc = gUnpacker.get(device_id, 0, seg, 0, adc_id);
	    hptr_array[awt_hid + seg]->Fill(adc);
	  }
#endif
	  ++multiplicity;
	  hptr_array[hit_hid]->Fill(seg);
	}
      }
    }
    hptr_array[mul_hid]->Fill(multiplicity);


#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }
#endif

#if DEBUG
std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  //------------------------------------------------------------------
  // V1725_STOP
  //------------------------------------------------------------------
  { ///// V1725-STOP : copy(TAG-PL)
    static const auto device_id = gUnpacker.get_device_id("V1725-STOP");
    static const auto fadc_id    = gUnpacker.get_data_id("V1725-STOP", "adc");
    //static const auto tdc_min   = 580;
    //static const auto tdc_max   = 640;
    static const auto fadc_hid   = gHist.getSequentialID(kV1725_STOP, 0, kFADC,     0);

    Int_t fadc=0;
    // FADC
    auto nhit_f = gUnpacker.get_entries(device_id, 0, 0, 0, fadc_id);
    for (Int_t m=0; m<nhit_f; ++m) {
      fadc = gUnpacker.get(device_id, 0, 0, 0, fadc_id, m);
      hptr_array[fadc_hid]->Fill(m, fadc);
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  //------------------------------------------------------------------
  // U_Veto
  //------------------------------------------------------------------
  std::vector<Int_t> hitseg_u_veto;
  { ///// U_Veto
    static const auto device_id = gUnpacker.get_device_id("UpVeto");
    static const auto adc_id    = gUnpacker.get_data_id("UpVeto", "adc");
    static const auto tdc_id    = gUnpacker.get_data_id("UpVeto", "tdc");
    static const auto tdc_min   = gUser.GetParameter("TdcUpVeto", 0);
    static const auto tdc_max   = gUser.GetParameter("TdcUpVeto", 1);
    static const auto adc_hid   = gHist.getSequentialID(kU_Veto, 0, kADC,     0);
    static const auto tdc_hid   = gHist.getSequentialID(kU_Veto, 0, kTDC,     0);
    static const auto awt_hid   = gHist.getSequentialID(kU_Veto, 0, kADCwTDC, 0);
    static const auto hit_hid   = gHist.getSequentialID(kU_Veto, 0, kHitPat,  0);
    static const auto mul_hid   = gHist.getSequentialID(kU_Veto, 0, kMulti,   0);
    Int_t hit_flag;
    Int_t multiplicity = 0;
    for(Int_t seg=0; seg<NumOfSegU_Veto; ++seg) {
      hit_flag = 0;
      // ADC
      UInt_t adc = 0;
      auto nhit = gUnpacker.get_entries(device_id, 0, seg, 0, adc_id);
      if (nhit != 0) {
	adc = gUnpacker.get(device_id, 0, seg, 0, adc_id);
	hptr_array[adc_hid + seg]->Fill(adc);
      }
      // TDC
      for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, seg, 0, tdc_id);
	  m<n; ++m) {
	auto tdc = gUnpacker.get(device_id, 0, seg, 0, tdc_id, m);
	if (tdc != 0) {
	  hptr_array[tdc_hid + seg]->Fill(tdc);
	  if (tdc_min<tdc && tdc<tdc_max && adc > 0) {
	    hit_flag = 1;
	  }
	}
      }
      if (hit_flag == 1) {
	hptr_array[awt_hid + seg]->Fill(adc);
	hptr_array[hit_hid]->Fill(seg);
	hitseg_u_veto.push_back(seg);
      }
    }
    hptr_array[mul_hid]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif



  //------------------------------------------------------------------
  // T0
  //------------------------------------------------------------------
  std::vector<Int_t> hitseg_t0;
  { ///// T0
    static const auto device_id = gUnpacker.get_device_id("T0");
    static const auto adc_id    = gUnpacker.get_data_id("T0", "adc");
    static const auto tdc_id    = gUnpacker.get_data_id("T0", "tdc");
    static const auto tdc_min   = gUser.GetParameter("TdcT0", 0);
    static const auto tdc_max   = gUser.GetParameter("TdcT0", 1);
    static const auto adc_hid   = gHist.getSequentialID(kT0, 0, kADC,     0);
    static const auto tdc_hid   = gHist.getSequentialID(kT0, 0, kTDC,     0);
    static const auto awt_hid   = gHist.getSequentialID(kT0, 0, kADCwTDC, 0);
    static const auto hit_hid   = gHist.getSequentialID(kT0, 0, kHitPat,  0);
    static const auto mul_hid   = gHist.getSequentialID(kT0, 0, kMulti,   0);
    std::vector<std::vector<Int_t>> hit_flag(NumOfSegT0);
    Int_t multiplicity = 0;
    for(Int_t seg=0; seg<NumOfSegT0; ++seg) {
      hit_flag[seg].resize(kLorR);
      for(Int_t lr=0; lr<kLorR; ++lr) {
	hit_flag[seg][lr] = 0;
	// ADC
	UInt_t adc = 0;
	auto nhit = gUnpacker.get_entries(device_id, 0, seg, lr, adc_id);
	if (nhit != 0) {
	  adc = gUnpacker.get(device_id, 0, seg, lr, adc_id);
	  hptr_array[adc_hid + lr*NumOfSegT0 + seg]->Fill(adc);
	}
	// TDC
	for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, seg, lr, tdc_id);
	    m<n; ++m) {
	  auto tdc = gUnpacker.get(device_id, 0, seg, lr, tdc_id, m);
	  if (tdc != 0) {
	    hptr_array[tdc_hid + lr*NumOfSegT0 + seg]->Fill(tdc);
	    if (tdc_min<tdc && tdc<tdc_max && adc > 0) {
	      hit_flag[seg][lr] = 1;
	    }
	  }
	}
	if (hit_flag[seg][lr] == 1) {
	  hptr_array[awt_hid + lr*NumOfSegT0 + seg]->Fill(adc);
	}
      }
      if (hit_flag[seg][kL] == 1 && hit_flag[seg][kR] == 1) {
	++multiplicity;
	hptr_array[hit_hid]->Fill(seg);
	hitseg_t0.push_back(seg);
      }
    }
    hptr_array[mul_hid]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

#if 1
  //------------------------------------------------------------------
  // SAC
  //------------------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("SAC");
    static const Int_t k_adc    = gUnpacker.get_data_id("SAC","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("SAC","tdc");

    // sequential id
    static const Int_t saca_id   = gHist.getSequentialID(kSAC, 0, kADC,     0);
    static const Int_t sact_id   = gHist.getSequentialID(kSAC, 0, kTDC,     0);
    static const Int_t sacawt_id = gHist.getSequentialID(kSAC, 0, kADCwTDC, 0);
    static const Int_t sach_id   = gHist.getSequentialID(kSAC, 0, kHitPat,  0);
    static const Int_t sacm_id   = gHist.getSequentialID(kSAC, 0, kMulti,   0);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcSAC", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcSAC", 1);

    Int_t multiplicity = 0;
    for(Int_t seg = 0; seg<NumOfSegSAC; ++seg) {
      // ADC
      Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      if (nhit_a!=0) {
	Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[saca_id + seg]->Fill(adc);
      }
      // TDC
      Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      Bool_t is_in_gate = false;

      for(Int_t m = 0; m<nhit_t; ++m) {
	Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
	hptr_array[sact_id + seg]->Fill(tdc);

	if (tdc_min < tdc && tdc < tdc_max) {
	  is_in_gate = true;
	}// tdc range is ok
      }// for(m)

      if (is_in_gate) {
	// ADC w/TDC
	if (gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0) {
	  Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	  hptr_array[sacawt_id + seg]->Fill(adc);
	}
	hptr_array[sach_id]->Fill(seg);
	++multiplicity;
      }// flag is OK
    }

    hptr_array[sacm_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }// SAC

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

#endif



  //------------------------------------------------------------------
  // SDC0
  //------------------------------------------------------------------
  std::vector< std::vector<Int_t> > SDC0HitCont(6);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("SDC0");
    static const Int_t k_leading  = gUnpacker.get_data_id("SDC0", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("SDC0", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcSDC0", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcSDC0", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("MinTotSDC0", 0);
    // static const Int_t tot_max = gUser.GetParameter("MinTotSDC1", 1);

    // sequential id
    static const Int_t sdc0t_id     = gHist.getSequentialID(kSDC0, 0, kTDC,    0);
    static const Int_t sdc0tot_id   = gHist.getSequentialID(kSDC0, 0, kADC,    0);
    static const Int_t sdc0t1st_id  = gHist.getSequentialID(kSDC0, 0, kTDC2D,  0);
    static const Int_t sdc0hit_id   = gHist.getSequentialID(kSDC0, 0, kHitPat, 0);
    static const Int_t sdc0mul_id   = gHist.getSequentialID(kSDC0, 0, kMulti,  0);
    static const Int_t sdc0mulwt_id = gHist.getSequentialID(kSDC0, 0, kMulti,  NumOfLayersSDC0);

    static const Int_t sdc0t_wide_id    = gHist.getSequentialID(kSDC0, 0, kTDC,    10);
    static const Int_t sdc0t_ctot_id    = gHist.getSequentialID(kSDC0, 0, kTDC,    kTOTcutOffset);
    static const Int_t sdc0tot_ctot_id  = gHist.getSequentialID(kSDC0, 0, kADC,    kTOTcutOffset);
    static const Int_t sdc0t1st_ctot_id = gHist.getSequentialID(kSDC0, 0, kTDC2D,  kTOTcutOffset);
    static const Int_t sdc0tot1st_id    = gHist.getSequentialID(kSDC0, 0, kTDC2D,  10+kTOTcutOffset);
    static const Int_t sdc0t2D_id       = gHist.getSequentialID(kSDC0, 0, kTDC2D,  20+kTOTcutOffset);
    static const Int_t sdc0t2D_ctot_id  = gHist.getSequentialID(kSDC0, 0, kTDC2D,  30+kTOTcutOffset);
    static const Int_t sdc0tot2D_id     = gHist.getSequentialID(kSDC0, 0, kTDC2D,  40+kTOTcutOffset);
    static const Int_t sdc0_tot_tdc2D_id
      = gHist.getSequentialID(kSDC0, 0, kTOTTDC2D,  kTOTcutOffset);
    static const Int_t sdc0hit_ctot_id  = gHist.getSequentialID(kSDC0, 0, kHitPat, kTOTcutOffset);
    static const Int_t sdc0mul_ctot_id  = gHist.getSequentialID(kSDC0, 0, kMulti,  kTOTcutOffset);
    static const Int_t sdc0mulwt_ctot_id
      = gHist.getSequentialID(kSDC0, 0, kMulti, NumOfLayersSDC0 + kTOTcutOffset);
    static const Int_t sdc0layer_correlation_id
      = gHist.getSequentialID(kSDC0, 0, kCorr,  20);
    // static const Int_t sdc0self_corr_id  = gHist.getSequentialID(kSDC0, kSelfCorr, 0, 0);

    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersSDC0; ++l) {
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t tot1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      for(Int_t w=0; w<NumOfWireSDC0; ++w) {
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
	// hptr_array[sdc0hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for(Int_t m = 0; m<nhit_l; ++m) {
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[sdc0t_id + l]->Fill(tdc);
	  hptr_array[sdc0t_wide_id + l]->Fill(tdc); //TDCwide
	  if (tdc1st<tdc) tdc1st = tdc;

	  // tdc 2D
	  hptr_array[sdc0t2D_id + l]->Fill(w,tdc);

	  // Drift time check
	  if (tdc_min < tdc && tdc < tdc_max) {
	    flag_hit_wt = true;
	  }
	}

	if (tdc1st!=0) hptr_array[sdc0t1st_id +l]->Fill(tdc1st);
	if (flag_hit_wt) {
	  ++multiplicity_wt;
	  hptr_array[sdc0hit_id + l]->Fill(w);
	}

	tdc1st = 0;
	tot1st = 0;
	if (nhit_l == nhit_t && hit_l_max > hit_t_max) {
	  ++multiplicity_ctot;
	  for(Int_t m = 0; m<nhit_l; ++m) {
	    tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	    tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
	    tot = tdc - tdc_t;
	    hptr_array[sdc0tot_id+l]->Fill(tot);
	    hptr_array[sdc0tot2D_id+l]->Fill(w,tot);
            hptr_array[sdc0_tot_tdc2D_id+l]->Fill(tot,tdc);
            if (tot1st<tot) tot1st = tot;
            if (tot < tot_min) continue;
	    hptr_array[sdc0t_ctot_id + l]->Fill(tdc);
	    hptr_array[sdc0t2D_ctot_id + l]->Fill(w,tdc); //2D
	    hptr_array[sdc0tot_ctot_id+l]->Fill(tot);
            if (tdc1st<tdc) tdc1st = tdc;
	    if (tdc_min < tdc && tdc < tdc_max) {
	      flag_hit_wt_ctot = true;
	    }
	  }
	}

	if (tdc1st!=0) hptr_array[sdc0t1st_ctot_id +l]->Fill(tdc1st);
        if (tot1st!=0) hptr_array[sdc0tot1st_id +l]->Fill(tot);
	if (flag_hit_wt_ctot) {
	  ++multiplicity_wt_ctot;
	  hptr_array[sdc0hit_ctot_id + l]->Fill(w);
	  SDC0HitCont[l].push_back(w);
	}
      }

      hptr_array[sdc0mul_id + l]->Fill(multiplicity);
      hptr_array[sdc0mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[sdc0mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[sdc0mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }

    for(Int_t l=0; l<2; ++l){
      for(Int_t w=0; w<NumOfWireSDC0; ++w) {
        Int_t nhit_l_1 = gUnpacker.get_entries(k_device, 2*l, 0, w, k_leading);
	if (nhit_l_1 == 0) continue;
	for(Int_t x=0; x<NumOfWireSDC0; ++x) {
	  Int_t nhit_l_2 = gUnpacker.get_entries(k_device, 2*l+1, 0, x, k_leading);
	  if (nhit_l_2 == 0) continue;
	  hptr_array[sdc0layer_correlation_id + l]->Fill(w,x);
	}
      }
    }


    // for(Int_t s=0; s<NumOfDimSDC0 ;s++) {
    //   Int_t corr=2*s;
    //   for(UInt_t i=0; i<SDC0HitCont[corr].size() ;i++) {
    // 	for(UInt_t j=0; j<SDC0HitCont[corr+1].size() ;j++) {
    // 	  hptr_array[sdc0self_corr_id + s]->Fill(SDC0HitCont[corr][i],SDC0HitCont[corr+1][j]);
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

  //------------------------------------------------------------------
  // SDC1
  //------------------------------------------------------------------
  std::vector< std::vector<Int_t> > SDC1HitCont(NumOfLayersSDC1);
  // std::vector< std::vector<Int_t> > SDC1HitCont(6);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("SDC1");
    static const Int_t k_leading  = gUnpacker.get_data_id("SDC1", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("SDC1", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcSDC1", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcSDC1", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("MinTotSDC1", 0);
    // static const Int_t tot_max = gUser.GetParameter("MinTotSDC1", 1);

    // sequential id
    static const Int_t sdc1t_id    = gHist.getSequentialID(kSDC1, 0, kTDC, 0);
    static const Int_t sdc1tot_id  = gHist.getSequentialID(kSDC1, 0, kADC, 0);
    static const Int_t sdc1t1st_id = gHist.getSequentialID(kSDC1, 0, kTDC2D, 0);
    static const Int_t sdc1hit_id  = gHist.getSequentialID(kSDC1, 0, kHitPat, 0);
    static const Int_t sdc1mul_id  = gHist.getSequentialID(kSDC1, 0, kMulti, 0);
    static const Int_t sdc1mulwt_id
      = gHist.getSequentialID(kSDC1, 0, kMulti, NumOfLayersSDC1);

    static const Int_t sdc1t_wide_id    = gHist.getSequentialID(kSDC1, 0, kTDC,    10);
    static const Int_t sdc1t_ctot_id    = gHist.getSequentialID(kSDC1, 0, kTDC,    kTOTcutOffset);
    static const Int_t sdc1tot_ctot_id  = gHist.getSequentialID(kSDC1, 0, kADC,    kTOTcutOffset);
    static const Int_t sdc1t1st_ctot_id = gHist.getSequentialID(kSDC1, 0, kTDC2D,  kTOTcutOffset);
    static const Int_t sdc1tot1st_id    = gHist.getSequentialID(kSDC1, 0, kTDC2D,  10+kTOTcutOffset);
    static const Int_t sdc1t2D_id       = gHist.getSequentialID(kSDC1, 0, kTDC2D,  20+kTOTcutOffset);
    static const Int_t sdc1t2D_ctot_id  = gHist.getSequentialID(kSDC1, 0, kTDC2D,  30+kTOTcutOffset);
    static const Int_t sdc1tot2D_id     = gHist.getSequentialID(kSDC1, 0, kTDC2D,  40+kTOTcutOffset);
    static const Int_t sdc1_tot_tdc2D_id
      = gHist.getSequentialID(kSDC1, 0, kTOTTDC2D,  kTOTcutOffset);
    static const Int_t sdc1hit_ctot_id  = gHist.getSequentialID(kSDC1, 0, kHitPat, kTOTcutOffset);
    static const Int_t sdc1mul_ctot_id  = gHist.getSequentialID(kSDC1, 0, kMulti,  kTOTcutOffset);
    static const Int_t sdc1mulwt_ctot_id
      = gHist.getSequentialID(kSDC1, 0, kMulti, NumOfLayersSDC1 + kTOTcutOffset);
    static const Int_t sdc1layer_correlation_id
      = gHist.getSequentialID(kSDC1, 0, kCorr,  20);


    // static const Int_t sdc1self_corr_id  = gHist.getSequentialID(kSDC1, kSelfCorr, 0, 0);

    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersSDC1; ++l) {
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t tot1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      for(Int_t w=0; w<NumOfWireSDC1_HYPS[l]; ++w) {
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
	// hptr_array[sdc1hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for(Int_t m = 0; m<nhit_l; ++m) {
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[sdc1t_id + l]->Fill(tdc);
	  hptr_array[sdc1t_wide_id + l]->Fill(tdc); //TDCwide
	  // std::cout << GREEN << "<SDC1> layer: " << l << ", wire: " << w << ", hit#: " << m << ", TDC: " << tdc << CLEAR << std::endl;
	  if (tdc1st<tdc) tdc1st = tdc;

	  // tdc 2D
	  hptr_array[sdc1t2D_id + l]->Fill(w,tdc);

	  // Drift time check
	  if (tdc_min < tdc && tdc < tdc_max) {
	    flag_hit_wt = true;
	  }
	}

	if (tdc1st!=0) hptr_array[sdc1t1st_id +l]->Fill(tdc1st);
	if (flag_hit_wt) {
	  ++multiplicity_wt;
	  hptr_array[sdc1hit_id + l]->Fill(w);
	}

	tdc1st = 0;
	tot1st = 0;
	if (nhit_l == nhit_t && hit_l_max > hit_t_max) {
	  ++multiplicity_ctot;
	  for(Int_t m = 0; m<nhit_l; ++m) {
	    tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	    tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
	    tot = tdc - tdc_t;
	    hptr_array[sdc1tot_id+l]->Fill(tot);
	    hptr_array[sdc1tot2D_id+l]->Fill(w,tot);
	    hptr_array[sdc1_tot_tdc2D_id+l]->Fill(tot,tdc);
	    if (tot1st<tot) tot1st = tot;
	    if (tot < tot_min) continue;
	    hptr_array[sdc1t_ctot_id + l]->Fill(tdc);
	    hptr_array[sdc1t2D_ctot_id + l]->Fill(w,tdc); //2D
	    hptr_array[sdc1tot_ctot_id+l]->Fill(tot);
	    if (tdc1st<tdc) tdc1st = tdc;
	    if (tdc_min < tdc && tdc < tdc_max) {
	      flag_hit_wt_ctot = true;
	    }
	  }
	}

	if (tdc1st!=0) hptr_array[sdc1t1st_ctot_id +l]->Fill(tdc1st);
	if (tot1st!=0) hptr_array[sdc1tot1st_id +l]->Fill(tot);
	if (flag_hit_wt_ctot) {
	  ++multiplicity_wt_ctot;
	  hptr_array[sdc1hit_ctot_id + l]->Fill(w);
	  SDC1HitCont[l].push_back(w);
	}
      }

      hptr_array[sdc1mul_id + l]->Fill(multiplicity);
      hptr_array[sdc1mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[sdc1mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[sdc1mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }

    for(Int_t l=0; l<2; ++l){
      for(Int_t w=0; w<NumOfWireSDC1; ++w) {
        Int_t nhit_l_1 = gUnpacker.get_entries(k_device, 2*l+2, 0, w, k_leading);
        if (nhit_l_1 == 0) continue;
        for(Int_t x=0; x<NumOfWireSDC1; ++x) {
          Int_t nhit_l_2 = gUnpacker.get_entries(k_device, 2*l+3, 0, x, k_leading);
          if (nhit_l_2 == 0) continue;
          hptr_array[sdc1layer_correlation_id + l]->Fill(w,x);
        }
      }
    }

    // for(Int_t s=0; s<NumOfDimSDC1 ;s++) {
    //   Int_t corr=2*s;
    //   for(UInt_t i=0; i<SDC1HitCont[corr].size() ;i++) {
    // 	for(UInt_t j=0; j<SDC1HitCont[corr+1].size() ;j++) {
    // 	  // hptr_array[sdc1self_corr_id + s]->Fill(SDC1HitCont[corr][i],SDC1HitCont[corr+1][j])
    // 	    ;
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

  //------------------------------------------------------------------
  // SDC2
  //------------------------------------------------------------------
  // std::vector< std::vector<Int_t> > SDC2HitCont(6);
  std::vector< std::vector<Int_t> > SDC2HitCont(NumOfLayersSDC2);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("SDC2");
    static const Int_t k_leading  = gUnpacker.get_data_id("SDC2", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("SDC2", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcSDC2", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcSDC2", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("MinTotSDC2", 0);
    // static const Int_t tot_max = gUser.GetParameter("MinTotSDC2", 1);

    // sequential id
    static const Int_t sdc2t_id    = gHist.getSequentialID(kSDC2, 0, kTDC, 0);
    static const Int_t sdc2tot_id  = gHist.getSequentialID(kSDC2, 0, kADC, 0);
    static const Int_t sdc2t1st_id = gHist.getSequentialID(kSDC2, 0, kTDC2D, 0);
    static const Int_t sdc2hit_id  = gHist.getSequentialID(kSDC2, 0, kHitPat, 0);
    static const Int_t sdc2mul_id  = gHist.getSequentialID(kSDC2, 0, kMulti, 0);
    static const Int_t sdc2mulwt_id
      = gHist.getSequentialID(kSDC2, 0, kMulti, NumOfLayersSDC2);

    static const Int_t sdc2t_wide_id    = gHist.getSequentialID(kSDC2, 0, kTDC,    10);
    static const Int_t sdc2t_ctot_id    = gHist.getSequentialID(kSDC2, 0, kTDC,    kTOTcutOffset);
    static const Int_t sdc2tot_ctot_id  = gHist.getSequentialID(kSDC2, 0, kADC,    kTOTcutOffset);
    static const Int_t sdc2t1st_ctot_id = gHist.getSequentialID(kSDC2, 0, kTDC2D,  kTOTcutOffset);
    static const Int_t sdc2tot1st_id    = gHist.getSequentialID(kSDC2, 0, kTDC2D,  10+kTOTcutOffset);
    static const Int_t sdc2t2D_id       = gHist.getSequentialID(kSDC2, 0, kTDC2D,  20+kTOTcutOffset);
    static const Int_t sdc2t2D_ctot_id  = gHist.getSequentialID(kSDC2, 0, kTDC2D,  30+kTOTcutOffset);
    static const Int_t sdc2tot2D_id     = gHist.getSequentialID(kSDC2, 0, kTDC2D,  40+kTOTcutOffset);
    static const Int_t sdc2_tot_tdc2D_id
      = gHist.getSequentialID(kSDC2, 0, kTOTTDC2D,  kTOTcutOffset);
    static const Int_t sdc2hit_ctot_id  = gHist.getSequentialID(kSDC2, 0, kHitPat, kTOTcutOffset);
    static const Int_t sdc2mul_ctot_id  = gHist.getSequentialID(kSDC2, 0, kMulti,  kTOTcutOffset);
    static const Int_t sdc2mulwt_ctot_id
      = gHist.getSequentialID(kSDC2, 0, kMulti, NumOfLayersSDC2 + kTOTcutOffset);
    static const Int_t sdc2layer_correlation_id
      = gHist.getSequentialID(kSDC2, 0, kCorr,  20);
    // static const Int_t sdc2self_corr_id  = gHist.getSequentialID(kSDC2, kSelfCorr, 0, 0);


    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersSDC2; ++l) {
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      for(Int_t w=0; w<NumOfWireSDC2_HYPS[l]; ++w) {
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
	// hptr_array[sdc2hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for(Int_t m = 0; m<nhit_l; ++m) {
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[sdc2t_id + l]->Fill(tdc);
	  hptr_array[sdc2t_wide_id + l]->Fill(tdc); //TDCwide
	  // std::cout << PINK << "<SDC2> layer: " << l << ", wire: " << w << ", hit#: " << m << ", TDC: " << tdc << CLEAR << std::endl;
	  if (tdc1st<tdc) tdc1st = tdc;

	  // tdc 2D
	  hptr_array[sdc2t2D_id + l]->Fill(w,tdc);

	  // Drift time check
	  // if (tdc_min < tdc && tdc < tdc_max) {
	  if (0 < tdc && tdc < 2000) {
	    flag_hit_wt = true;
	  }
	}

	if (tdc1st!=0) hptr_array[sdc2t1st_id +l]->Fill(tdc1st);
	if (flag_hit_wt) {
	  ++multiplicity_wt;
	  hptr_array[sdc2hit_id + l]->Fill(w);
	}

	tdc1st = 0;
	if (nhit_l == nhit_t && hit_l_max > hit_t_max) {
	  ++multiplicity_ctot;
	  for(Int_t m = 0; m<nhit_l; ++m) {
	    tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	    tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
	    tot = tdc - tdc_t;
	    hptr_array[sdc2tot_id+l]->Fill(tot);
	    hptr_array[sdc2tot2D_id+l]->Fill(w,tot);
	    hptr_array[sdc2_tot_tdc2D_id+l]->Fill(tot,tdc);
	    if (tot < tot_min) continue;
	    hptr_array[sdc2t_ctot_id + l]->Fill(tdc);
	    hptr_array[sdc2t2D_ctot_id + l]->Fill(w,tdc); //2D
	    hptr_array[sdc2tot_ctot_id+l]->Fill(tot);
	    if (tdc1st<tdc) tdc1st = tdc;
	    if (tdc_min < tdc && tdc < tdc_max) {
	      flag_hit_wt_ctot = true;
	    }
	  }
	}

	if (tdc1st!=0){
	  hptr_array[sdc2t1st_ctot_id +l]->Fill(tdc1st);
	  hptr_array[sdc2tot1st_id +l]->Fill(tot);
	}
	if (flag_hit_wt_ctot) {
	  ++multiplicity_wt_ctot;
	  hptr_array[sdc2hit_ctot_id + l]->Fill(w);
	  SDC2HitCont[l].push_back(w);
	}
      }

      hptr_array[sdc2mul_id + l]->Fill(multiplicity);
      hptr_array[sdc2mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[sdc2mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[sdc2mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }
    for(Int_t l=0; l<2; ++l){
      for(Int_t w=0; w<NumOfWireSDC2_HYPS[2*l+1]; ++w) {
        Int_t nhit_l_1 = gUnpacker.get_entries(k_device, 2*l+1, 0, w, k_leading);
        if (nhit_l_1 == 0) continue;
        for(Int_t x=0; x<NumOfWireSDC2_HYPS[2*l+2]; ++x) {
          Int_t nhit_l_2 = gUnpacker.get_entries(k_device, 2*l+2, 0, x, k_leading);
          if (nhit_l_2 == 0) continue;
          hptr_array[sdc2layer_correlation_id + l]->Fill(w,x);
        }
      }
    }


    // for(Int_t s=0; s<NumOfDimSDC2 ;s++) {
    //   Int_t corr=2*s;
    //   for(UInt_t i=0; i<SDC2HitCont[corr].size() ;i++) {
    // 	for(UInt_t j=0; j<SDC2HitCont[corr+1].size() ;j++) {
    // 	  // hptr_array[sdc2self_corr_id + s]->Fill(SDC2HitCont[corr][i],SDC2HitCont[corr+1][j])
    // 	    ;
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

  //------------------------------------------------------------------
  // SDC3
  //------------------------------------------------------------------
  std::vector< std::vector<Int_t> > SDC3HitCont(NumOfLayersSDC3);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("SDC3");
    static const Int_t k_leading  = gUnpacker.get_data_id("SDC3", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("SDC3", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcSDC3", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcSDC3", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("MinTotSDC3", 0);
    // static const Int_t tot_max = gUser.GetParameter("MinTotSDC3", 1);

    // sequential id
    static const Int_t sdc3t_id    = gHist.getSequentialID(kSDC3, 0, kTDC, 0);
    static const Int_t sdc3tot_id  = gHist.getSequentialID(kSDC3, 0, kADC, 0);
    static const Int_t sdc3t1st_id = gHist.getSequentialID(kSDC3, 0, kTDC2D, 0);
    static const Int_t sdc3hit_id  = gHist.getSequentialID(kSDC3, 0, kHitPat, 0);
    static const Int_t sdc3mul_id  = gHist.getSequentialID(kSDC3, 0, kMulti, 0);
    static const Int_t sdc3mulwt_id
      = gHist.getSequentialID(kSDC3, 0, kMulti, NumOfLayersSDC3);

    static const Int_t sdc3t_wide_id    = gHist.getSequentialID(kSDC3, 0, kTDC,    10);
    static const Int_t sdc3t_ctot_id    = gHist.getSequentialID(kSDC3, 0, kTDC,    kTOTcutOffset);
    static const Int_t sdc3tot_ctot_id  = gHist.getSequentialID(kSDC3, 0, kADC,    kTOTcutOffset);
    static const Int_t sdc3t1st_ctot_id = gHist.getSequentialID(kSDC3, 0, kTDC2D,  kTOTcutOffset);
    static const Int_t sdc3tot1st_id    = gHist.getSequentialID(kSDC3, 0, kTDC2D,  10+kTOTcutOffset);
    static const Int_t sdc3t2D_id       = gHist.getSequentialID(kSDC3, 0, kTDC2D,  20+kTOTcutOffset);
    static const Int_t sdc3t2D_ctot_id  = gHist.getSequentialID(kSDC3, 0, kTDC2D,  30+kTOTcutOffset);
    static const Int_t sdc3tot2D_id     = gHist.getSequentialID(kSDC3, 0, kTDC2D,  40+kTOTcutOffset);
    static const Int_t sdc3_tot_tdc2D_id
      = gHist.getSequentialID(kSDC3, 0, kTOTTDC2D,  kTOTcutOffset);
    static const Int_t sdc3hit_ctot_id  = gHist.getSequentialID(kSDC3, 0, kHitPat, kTOTcutOffset);
    static const Int_t sdc3mul_ctot_id  = gHist.getSequentialID(kSDC3, 0, kMulti,  kTOTcutOffset);
    static const Int_t sdc3mulwt_ctot_id
      = gHist.getSequentialID(kSDC3, 0, kMulti, NumOfLayersSDC3 + kTOTcutOffset);
    static const Int_t sdc3layer_correlation_id
      = gHist.getSequentialID(kSDC3, 0, kCorr,  20);
    // static const Int_t sdc3self_corr_id  = gHist.getSequentialID(kSDC3, kSelfCorr, 0, 0);


    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersSDC3; ++l) {
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      for(Int_t w=0; w<NumOfWireSDC3_HYPS[l]; ++w) {
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
	// hptr_array[sdc3hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for(Int_t m = 0; m<nhit_l; ++m) {
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[sdc3t_id + l]->Fill(tdc);
	  hptr_array[sdc3t_wide_id + l]->Fill(tdc); //TDCwide
	  // std::cout << CYAN << "<SDC3> layer: " << l << ", wire: " << w << ", hit#: " << m << ", TDC: " << tdc << CLEAR << std::endl;
	  if (tdc1st<tdc) tdc1st = tdc;

	  // tdc 2D
	  hptr_array[sdc3t2D_id + l]->Fill(w,tdc);

	  // Drift time check
	  // if (tdc_min < tdc && tdc < tdc_max) {
	  if (0 < tdc && tdc < 2000) {
	    flag_hit_wt = true;
	  }
	}

	if (tdc1st!=0) hptr_array[sdc3t1st_id +l]->Fill(tdc1st);
	if (flag_hit_wt) {
	  ++multiplicity_wt;
	  hptr_array[sdc3hit_id + l]->Fill(w);
	}

	tdc1st = 0;
	if (nhit_l == nhit_t && hit_l_max > hit_t_max) {
	  ++multiplicity_ctot;
	  for(Int_t m = 0; m<nhit_l; ++m) {
	    tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	    tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
	    tot = tdc - tdc_t;
	    hptr_array[sdc3tot_id+l]->Fill(tot);
	    hptr_array[sdc3tot2D_id+l]->Fill(w,tot);
	    hptr_array[sdc3_tot_tdc2D_id+l]->Fill(tot,tdc);
	    if (tot < tot_min) continue;
	    hptr_array[sdc3t_ctot_id + l]->Fill(tdc);
	    hptr_array[sdc3t2D_ctot_id + l]->Fill(w,tdc); //2D
	    hptr_array[sdc3tot_ctot_id+l]->Fill(tot);
	    if (tdc1st<tdc) tdc1st = tdc;
	    if (tdc_min < tdc && tdc < tdc_max) {
	      flag_hit_wt_ctot = true;
	    }
	  }
	}

	if (tdc1st!=0){
	  hptr_array[sdc3t1st_ctot_id +l]->Fill(tdc1st);
	  hptr_array[sdc3tot1st_id +l]->Fill(tot);
	}
	if (flag_hit_wt_ctot) {
	  ++multiplicity_wt_ctot;
	  hptr_array[sdc3hit_ctot_id + l]->Fill(w);
	  SDC3HitCont[l].push_back(w);
	}
      }

      hptr_array[sdc3mul_id + l]->Fill(multiplicity);
      hptr_array[sdc3mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[sdc3mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[sdc3mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }

    for(Int_t l=0; l<2; ++l){
      for(Int_t w=0; w<NumOfWireSDC3_HYPS[2*l+1]; ++w) {
        Int_t nhit_l_1 = gUnpacker.get_entries(k_device, 2*l+1, 0, w, k_leading);
        if (nhit_l_1 == 0) continue;
        for(Int_t x=0; x<NumOfWireSDC3_HYPS[2*l+2]; ++x) {
          Int_t nhit_l_2 = gUnpacker.get_entries(k_device, 2*l+2, 0, x, k_leading);
          if (nhit_l_2 == 0) continue;
          hptr_array[sdc3layer_correlation_id + l]->Fill(w,x);
        }
      }
    }

    // for(Int_t s=0; s<NumOfDimSDC3 ;s++) {
    //   Int_t corr=2*s;
    //   for(UInt_t i=0; i<SDC3HitCont[corr].size() ;i++) {
    // 	for(UInt_t j=0; j<SDC3HitCont[corr+1].size() ;j++) {
    // 	  // hptr_array[sdc3self_corr_id + s]->Fill(SDC3HitCont[corr][i],SDC3HitCont[corr+1][j])
    // 	    ;
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

#if 1
  //------------------------------------------------------------------
  // E_Veto
  //------------------------------------------------------------------
  std::vector<Int_t> hitseg_e_veto;
  { ///// E_Veto
    static const auto device_id = gUnpacker.get_device_id("E_Veto");
    static const auto adc_id    = gUnpacker.get_data_id("E_Veto", "adc");
    static const auto tdc_id    = gUnpacker.get_data_id("E_Veto", "tdc");
    static const auto tdc_min   = gUser.GetParameter("TdcE_Veto", 0);
    static const auto tdc_max   = gUser.GetParameter("TdcE_Veto", 1);
    static const auto adc_hid   = gHist.getSequentialID(kE_Veto, 0, kADC,     0);
    static const auto tdc_hid   = gHist.getSequentialID(kE_Veto, 0, kTDC,     0);
    static const auto awt_hid   = gHist.getSequentialID(kE_Veto, 0, kADCwTDC, 0);
    static const auto hit_hid   = gHist.getSequentialID(kE_Veto, 0, kHitPat,  0);
    static const auto mul_hid   = gHist.getSequentialID(kE_Veto, 0, kMulti,   0);
    std::vector<std::vector<Int_t>> hit_flag(NumOfSegE_Veto);
    Int_t multiplicity = 0;
    for(Int_t seg=0; seg<NumOfSegE_Veto; ++seg) {
      hit_flag[seg].resize(kLorR);
      for(Int_t lr=0; lr<kLorR; ++lr) {
	hit_flag[seg][lr] = 0;
	// ADC
	UInt_t adc = 0;
	auto nhit = gUnpacker.get_entries(device_id, 0, seg, lr, adc_id);
	if (nhit != 0) {
	  adc = gUnpacker.get(device_id, 0, seg, lr, adc_id);
	  hptr_array[adc_hid + lr*NumOfSegE_Veto + seg]->Fill(adc);
	}
	// TDC
	for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, seg, lr, tdc_id);
	    m<n; ++m) {
	  auto tdc = gUnpacker.get(device_id, 0, seg, lr, tdc_id, m);
	  if (tdc != 0) {
	    hptr_array[tdc_hid + lr*NumOfSegE_Veto + seg]->Fill(tdc);
	    if (tdc_min<tdc && tdc<tdc_max && adc > 0) {
	      hit_flag[seg][lr] = 1;
	    }
	  }
	}
	if (hit_flag[seg][lr] == 1) {
	  hptr_array[awt_hid + lr*NumOfSegE_Veto + seg]->Fill(adc);
	}
      }
      if (hit_flag[seg][kL] == 1 && hit_flag[seg][kR] == 1) {
	++multiplicity;
	hptr_array[hit_hid]->Fill(seg);
	hitseg_e_veto.push_back(seg);
      }
    }
    hptr_array[mul_hid]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

#endif

  //------------------------------------------------------------------
  // TOF
  //------------------------------------------------------------------
  std::vector<Int_t> hitseg_tof;
  { ///// TOF
    static const auto device_id = gUnpacker.get_device_id("TOF");
    static const auto adc_id    = gUnpacker.get_data_id("TOF", "adc");
    static const auto tdc_id    = gUnpacker.get_data_id("TOF", "tdc");
    //static const auto tdc_min   = gUser.GetParameter("TdcTOF", 0);
    //static const auto tdc_max   = gUser.GetParameter("TdcTOF", 1);
    Int_t tdc_min = 0;
    Int_t tdc_max = 0;
    static const auto tdc_min_ku   = gUser.GetParameter("TdcTOF_KURAMA_U", 0);
    static const auto tdc_min_kd   = gUser.GetParameter("TdcTOF_KURAMA_D", 0);
    static const auto tdc_min_lu   = gUser.GetParameter("TdcTOF_LEPS_U", 0);
    static const auto tdc_min_ld   = gUser.GetParameter("TdcTOF_LEPS_D", 0);
    static const auto tdc_max_ku   = gUser.GetParameter("TdcTOF_KURAMA_U", 1);
    static const auto tdc_max_kd   = gUser.GetParameter("TdcTOF_KURAMA_D", 1);
    static const auto tdc_max_lu   = gUser.GetParameter("TdcTOF_LEPS_U", 1);
    static const auto tdc_max_ld   = gUser.GetParameter("TdcTOF_LEPS_D", 1);
    static const auto adc_hid   = gHist.getSequentialID(kTOF, 0, kADC,     0);
    static const auto tdc_hid   = gHist.getSequentialID(kTOF, 0, kTDC,     0);
    static const auto awt_hid   = gHist.getSequentialID(kTOF, 0, kADCwTDC, 0);
    static const auto hit_hid   = gHist.getSequentialID(kTOF, 0, kHitPat,  0);
    static const auto mul_hid   = gHist.getSequentialID(kTOF, 0, kMulti,   0);
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
	for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, seg, ud, tdc_id); m<n; ++m) {
	  auto tdc = gUnpacker.get(device_id, 0, seg, ud, tdc_id, m);
	  if (tdc != 0) {
	    hptr_array[tdc_hid + ud*NumOfSegTOF + seg]->Fill(tdc);
	    /*   if (tdc_min<tdc && tdc<tdc_max && adc > 0) {
	      hit_flag[seg][ud] = 1;
	      }*/
	    if (seg<12 || seg>36) { // LEPS TOF
	      if (ud == kU){
		tdc_min = tdc_min_lu;
		tdc_max = tdc_max_lu;
	      }else{
		tdc_min = tdc_min_ld;
		tdc_max = tdc_max_ld;
	      }
	    }else{// KURAMA TOF
	      if (ud == kU){
		tdc_min = tdc_min_ku;
		tdc_max = tdc_max_ku;
	      }else{
		tdc_min = tdc_min_kd;
		tdc_max = tdc_max_kd;
	      }
	    }
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

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

#if 0
  //------------------------------------------------------------------
  // CaenV792
  //------------------------------------------------------------------
  {
    static const auto device_id = gUnpacker.get_device_id("CaenV792");
    static const auto adc_id   = gUnpacker.get_data_id("CaenV792", "adc");
    static const auto adc_hid  = gHist.getSequentialID(kCaenV792, 0, kADC, 0);

    for(Int_t seg=0; seg<NumOfSegCaenV792; ++seg){
      Int_t n = gUnpacker.get_entries(device_id, 0, seg, 0, adc_id);
      for(Int_t m=0; m<n; ++m){
        Int_t adc = gUnpacker.get(device_id, 0, seg, 0, adc_id, m);
        hptr_array[adc_hid + seg]->Fill(adc);
      }
    }
#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(device_id);
#endif
  }//

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  //------------------------------------------------------------------
  // TOF_HRTDC
  //------------------------------------------------------------------
  {
    static const auto device_id = gUnpacker.get_device_id("TOF_HRTDC");
    static const auto tdc_id    = gUnpacker.get_data_id("TOF_HRTDC", "tdc");
    static const auto tdc_min   = gUser.GetParameter("TdcTOF", 0);
    static const auto tdc_max   = gUser.GetParameter("TdcTOF", 1);
    static const auto tdc_hid   = gHist.getSequentialID(kTOF_HRTDC, 0, kTDC,     0);

    for(Int_t seg=0; seg<NumOfSegTOF_HRTDC; ++seg) {
      // TDC
      for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, seg, 0, tdc_id);
	  m<n; ++m) {
	auto tdc = gUnpacker.get(device_id, 0, seg, 0, tdc_id, m);
	if (tdc != 0) {
	  hptr_array[tdc_hid + seg]->Fill(tdc);
	}
      }
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  //------------------------------------------------------------------
  // CaenV1725
  //------------------------------------------------------------------
  {
    static const auto device_id = gUnpacker.get_device_id("CaenV1725");
    static const auto fadc_id   = gUnpacker.get_data_id("CaenV1725", "fadc");
    static const auto fadc_hid  = gHist.getSequentialID(kCaenV1725, 0, kFADC, 0);

    for(Int_t seg=0; seg<NumOfSegCaenV1725; ++seg){
      Int_t n = gUnpacker.get_entries(device_id, 0, seg, 0, fadc_id);
      for(Int_t m=0; m<n; ++m){
        Int_t fadc = gUnpacker.get(device_id, 0, seg, 0, fadc_id, m);
        hptr_array[fadc_hid + seg]->Fill(m, fadc);
      }
    }
#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(device_id);
#endif
  }//

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

#endif // CaenV792, TOF_HRTDC, CaenV1725

#if 0 // Corre, BTOF, TF_TF, TF_GN1, TF_GN2
  //------------------------------------------------------------------
  // Correlation(2D histograms)
  //------------------------------------------------------------------
  {
    // data type
    static const Int_t k_device_bh1  = gUnpacker.get_device_id("BH1");
    static const Int_t k_device_bh2  = gUnpacker.get_device_id("BH2");
    static const Int_t k_device_bc3  = gUnpacker.get_device_id("BC3");
    static const Int_t k_device_bc4  = gUnpacker.get_device_id("BC4");
    static const Int_t k_device_sdc1 = gUnpacker.get_device_id("SDC1");
    // static const Int_t k_device_sdc2 = gUnpacker.get_device_id("SDC2");
    static const Int_t k_device_sdc3 = gUnpacker.get_device_id("SDC3");
    static const Int_t k_device_sdc4 = gUnpacker.get_device_id("SDC4");
    static const Int_t k_device_tof = gUnpacker.get_device_id("TOF");
    static const Int_t k_device_ac1 = gUnpacker.get_device_id("AC1");
    static const Int_t k_device_wc = gUnpacker.get_device_id("WC");

    // sequential id
    Int_t cor_id = gHist.getSequentialID(kCorrelation, 0, 0, 1);
    Int_t mtx2d_id = gHist.getSequentialID(kCorrelation, 1, 0, 1);
    Int_t mtx3d_id = gHist.getSequentialID(kCorrelation, 2, 0, 1);

    // BH1 vs BFT
    TH2* hcor_bh1bft = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t seg1 = 0; seg1<NumOfSegBH1; ++seg1) {
      for(const auto& seg2: hitseg_bftu) {
	Int_t nhitBH1 = gUnpacker.get_entries(k_device_bh1, 0, seg1, 0, 1);
	if (nhitBH1 == 0) continue;
	Int_t tdcBH1 = gUnpacker.get(k_device_bh1, 0, seg1, 0, 1);
	Bool_t hitBH1 = (tdcBH1 > 0);
	if (hitBH1) {
	  hcor_bh1bft->Fill(seg1, seg2);
	}
      }
    }

    // BH1 vs BH2
    TH2* hcor_bh1bh2 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t seg1 = 0; seg1<NumOfSegBH1; ++seg1) {
      for(Int_t seg2 = 0; seg2<NumOfSegBH2; ++seg2) {
	Int_t hitBH1 = gUnpacker.get_entries(k_device_bh1, 0, seg1, 0, 1);
	Int_t hitBH2 = gUnpacker.get_entries(k_device_bh2, 0, seg2, 0, 1);
	if (hitBH1 == 0 || hitBH2 == 0)continue;
	Int_t tdcBH1 = gUnpacker.get(k_device_bh1, 0, seg1, 0, 1);
	Int_t tdcBH2 = gUnpacker.get(k_device_bh2, 0, seg2, 0, 1);
	if (tdcBH1 != 0 && tdcBH2 != 0) {
	  hcor_bh1bh2->Fill(seg1, seg2);
	}
      }
    }

    // BC3 vs BC4
    TH2* hcor_bc3bc4 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t wire1 = 0; wire1<NumOfWireBC3; ++wire1) {
      for(Int_t wire2 = 0; wire2<NumOfWireBC4; ++wire2) {
	Int_t hitBC3 = gUnpacker.get_entries(k_device_bc3, 0, 0, wire1, 0);
	Int_t hitBC4 = gUnpacker.get_entries(k_device_bc4, 5, 0, wire2, 0);
	if (hitBC3 == 0 || hitBC4 == 0)continue;
	hcor_bc3bc4->Fill(wire1, wire2);
      }
    }

    // SDC3 vs SDC1
    TH2* hcor_sdc1sdc3 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t wire1 = 0; wire1<NumOfWireSDC1; ++wire1) {
      for(Int_t wire3 = 0; wire3<NumOfWireSDC3; ++wire3) {
	Int_t hitSDC1 = gUnpacker.get_entries(k_device_sdc1, 0, 0, wire1, 0);
	Int_t hitSDC3 = gUnpacker.get_entries(k_device_sdc3, 0, 0, wire3, 0);
	if (hitSDC1 == 0 || hitSDC3 == 0) continue;
	hcor_sdc1sdc3->Fill(wire1, wire3);
      }
    }

    // SDC3 vs SDC4
    TH2* hcor_sdc3sdc4 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t wire3 = 0; wire3<NumOfWireSDC3; ++wire3) {
      for(Int_t wire4 = 0; wire4<NumOfWireSDC4; ++wire4) {
	Int_t hitSDC3 = gUnpacker.get_entries(k_device_sdc3, 0, 0, wire3, 0);
	Int_t hitSDC4 = gUnpacker.get_entries(k_device_sdc4, 0, 0, wire4, 0);
	if (hitSDC3 == 0 || hitSDC4 == 0) continue;
	hcor_sdc3sdc4->Fill(wire3, wire4);
      }
    }

    // TOF vs SDC4
    TH2* hcor_tofsdc4 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(const auto& seg_tof: hitseg_tof) {
      for(Int_t wire=0; wire<NumOfWireSDC4X; ++wire) {
	Int_t hitSDC4 = gUnpacker.get_entries(k_device_sdc4, 2, 0, wire, 0);
	if (hitSDC4 == 0) continue;
	hcor_tofsdc4->Fill(wire, seg_tof);
      }
    }

    // AC1 vs TOF
    TH2* hcor_ac1tof = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t seg1 = 0; seg1<NumOfSegAC1-2; ++seg1) {
      for(Int_t seg2 = 0; seg2<NumOfSegTOF; ++seg2) {
	Int_t hitAC1 = gUnpacker.get_entries(k_device_ac1, 0, seg1, 0, 1);
	Int_t hitTOF = gUnpacker.get_entries(k_device_tof, 0, seg2, 0, 1);
	if (hitAC1 == 0 || hitTOF == 0)continue;
	Int_t tdcac1 = gUnpacker.get(k_device_ac1, 0, seg1, 0, 1);
	Int_t tdctof = gUnpacker.get(k_device_tof, 0, seg2, 0, 1);
	if (tdcac1 != 0 && tdctof != 0) {
	  hcor_ac1tof->Fill(seg1, seg2);
	}
      }
    }

    // WC vs TOF
    TH2* hcor_wctof = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t seg1 = 0; seg1<NumOfSegWC; ++seg1) {
      for(Int_t seg2 = 0; seg2<NumOfSegTOF; ++seg2) {
	Int_t hitWC = gUnpacker.get_entries(k_device_wc, 0, seg1, 0, 1);
	Int_t hitTOF = gUnpacker.get_entries(k_device_tof, 0, seg2, 0, 1);
	if (hitWC == 0 || hitTOF == 0)continue;
	Int_t tdcwc = gUnpacker.get(k_device_wc, 0, seg1, 0, 1);
	Int_t tdctof = gUnpacker.get(k_device_tof, 0, seg2, 0, 1);
	if (tdcwc != 0 && tdctof != 0) {
	  hcor_wctof->Fill(seg1, seg2);
	}
      }
    }

  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  //------------------------------------------------------------------
  // BTOF
  //------------------------------------------------------------------
  {
    // Unpacker
    static const Int_t k_d_bh1  = gUnpacker.get_device_id("BH1");
    static const Int_t k_d_bh2  = gUnpacker.get_device_id("BH2");
    static const Int_t k_tdc    = gUnpacker.get_data_id("BH1", "tdc");
    // HodoParam
    static const Int_t cid_bh1  = 1;
    static const Int_t cid_bh2  = 2;
    static const Int_t plid     = 0;
    // Sequential ID
    static const Int_t btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);
    static const auto& hodoMan = HodoParamMan::GetInstance();
    // TDC gate range
    static const UInt_t tdc_min_bh1 = gUser.GetParameter("TdcBH1", 0);
    static const UInt_t tdc_max_bh1 = gUser.GetParameter("TdcBH1", 1);
    static const UInt_t tdc_min_bh2 = gUser.GetParameter("TdcBH2", 0);
    static const UInt_t tdc_max_bh2 = gUser.GetParameter("TdcBH2", 1);
    // BH2
    Double_t t0  = 1e10;
    Double_t ofs = 0;
    for(Int_t seg=0; seg<NumOfSegBH2; ++seg) {
      Int_t nhitu = gUnpacker.get_entries(k_d_bh2, 0, seg, kU, k_tdc);
      Int_t nhitd = gUnpacker.get_entries(k_d_bh2, 0, seg, kD, k_tdc);
      for(Int_t mu=0; mu<nhitu; ++mu) {
	auto tdcu = gUnpacker.get(k_d_bh2, 0, seg, kU, k_tdc, mu);
	if (tdcu < tdc_min_bh2 || tdc_max_bh2 < tdcu) continue;
	for(Int_t md=0; md<nhitd; ++md) {
	  auto tdcd = gUnpacker.get(k_d_bh2, 0, seg, kD, k_tdc, md);
	  if (tdcd < tdc_min_bh2 || tdc_max_bh2 < tdcd) continue;
	  Double_t bh2ut, bh2dt;
	  hodoMan.GetTime(cid_bh2, plid, seg, kU, tdcu, bh2ut);
	  hodoMan.GetTime(cid_bh2, plid, seg, kD, tdcd, bh2dt);
	  Double_t bh2mt = (bh2ut + bh2dt)/2.;
	  if (TMath::Abs(t0) > TMath::Abs(bh2mt)) {
	    hodoMan.GetTime(cid_bh2, plid, seg, 2, 0, ofs);
	    t0 = bh2ut;
	  }
	}
      }
    }
    // BH1
    for(Int_t seg=0; seg<NumOfSegBH1; ++seg) {
      Int_t nhitu = gUnpacker.get_entries(k_d_bh1, 0, seg, kU, k_tdc);
      Int_t nhitd = gUnpacker.get_entries(k_d_bh1, 0, seg, kD, k_tdc);
      for(Int_t mu=0; mu<nhitu; ++mu) {
	auto tdcu = gUnpacker.get(k_d_bh1, 0, seg, kU, k_tdc, mu);
	if (tdcu < tdc_min_bh1 || tdc_max_bh1 < tdcu) continue;
	for(Int_t md=0; md<nhitd; ++md) {
	  auto tdcd = gUnpacker.get(k_d_bh1, 0, seg, kD, k_tdc, md);
	  if (tdcd < tdc_min_bh1 || tdc_max_bh1 < tdcd) continue;
	  Double_t bh1tu, bh1td;
	  hodoMan.GetTime(cid_bh1, plid, seg, kU, tdcu, bh1tu);
	  hodoMan.GetTime(cid_bh1, plid, seg, kD, tdcd, bh1td);
	  Double_t mt = (bh1tu+bh1td)/2.;
	  Double_t btof = mt-(t0+ofs);
	  hptr_array[btof_id]->Fill(btof);
	}// if (tdc)
      }// if (nhit)
    }// for(seg)
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  //------------------------------------------------------------------
  // TF_TF
  //------------------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("TF_TF");
    static const Int_t k_adc    = gUnpacker.get_data_id("TF_TF","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("TF_TF","tdc");

    static const Int_t a_id   = gHist.getSequentialID(kTF_TF, 0, kADC);
    static const Int_t t_id   = gHist.getSequentialID(kTF_TF, 0, kTDC);
    static const Int_t awt_id = gHist.getSequentialID(kTF_TF, 0, kADCwTDC);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcTF_TF", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcTF_TF", 1);

    for(Int_t seg = 0; seg<NumOfSegTF_TF; ++seg) {
      // ADC
      Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      if (nhit_a!=0) {
	Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[a_id + seg]->Fill(adc);
      }
      // TDC
      Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      Bool_t is_in_gate = false;

      for(Int_t m = 0; m<nhit_t; ++m) {
        Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
        hptr_array[t_id + seg]->Fill(tdc);

        if (tdc_min < tdc && tdc < tdc_max) {
          is_in_gate = true;
        }// tdc range is ok
      }// for(m)
    if (is_in_gate) {
        // ADC w/TDC
        if (gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0) {
          Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
          hptr_array[awt_id + seg]->Fill(adc);
        }
        //hptr_array[h_id]->Fill(seg);
        //hptr_array[e72para_id]->Fill(e72parasite::kE90SAC1 + seg);
        //++multiplicity[seg];
      }// flag is OK
    }



#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//TF_TF

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  //------------------------------------------------------------------
  // TF_GN1
  //------------------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("TF_GN1");
    //static const Int_t k_adc    = gUnpacker.get_data_id("TF_GN1","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("TF_GN1","tdc");

    //static const Int_t a_id   = gHist.getSequentialID(kTF_GN1, 0, kADC);
    static const Int_t t_id   = gHist.getSequentialID(kTF_GN1, 0, kTDC);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcTF_GN1", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcTF_GN1", 1);

    for(Int_t seg = 0; seg<NumOfSegTF_GN1; ++seg) {
      // ADC
    //  Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
    //  if (nhit_a!=0) {
    //    Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
    //    hptr_array[a_id + seg]->Fill(adc);
    //  }
      // TDC
      Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      Bool_t is_in_gate = false;

      for(Int_t m = 0; m<nhit_t; ++m) {
        Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
        hptr_array[t_id + seg]->Fill(tdc);

        if (tdc_min < tdc && tdc < tdc_max) {
          is_in_gate = true;
        }// tdc range is ok
      }// for(m)

    }


#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//TF_GN1

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  //------------------------------------------------------------------
  // TF_GN2
  //------------------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("TF_GN2");
    //static const Int_t k_adc    = gUnpacker.get_data_id("TF_GN2","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("TF_GN2","tdc");

    //static const Int_t a_id   = gHist.getSequentialID(kTF_GN2, 0, kADC);
    static const Int_t t_id   = gHist.getSequentialID(kTF_GN2, 0, kTDC);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcTF_GN2", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcTF_GN2", 1);

    for(Int_t seg = 0; seg<NumOfSegTF_GN2; ++seg) {
      // ADC
    //  Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
    //  if (nhit_a!=0) {
    //    Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
    //    hptr_array[a_id + seg]->Fill(adc);
    //  }
      // TDC
      Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      Bool_t is_in_gate = false;

      for(Int_t m = 0; m<nhit_t; ++m) {
        Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
        hptr_array[t_id + seg]->Fill(tdc);

        if (tdc_min < tdc && tdc < tdc_max) {
          is_in_gate = true;
        }// tdc range is ok
      }// for(m)

    }


#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//TF_GN2

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

#endif // Corre, BTOF, TF_TF, TF_GN1, TF_GN2

#if 1
  //------------------------------------------------------------------
  // BGO
  //------------------------------------------------------------------
  Bool_t bgo_is_hit = false;
  {
    // data type
    static const int k_device   = gUnpacker.get_device_id("BGO");
    // static const int k_adc      = gUnpacker.get_data_id("BGO", "adc");
    // static const int k_tdc      = gUnpacker.get_data_id("BGO", "tdc");
    static const int k_fadc      = gUnpacker.get_data_id("BGO", "adc");
    static const int k_leading   = gUnpacker.get_data_id("BGO", "tdc");
    // static const int k_trailing   = gUnpacker.get_data_id("BGO", "trailing");

    // TDC gate range
    static const unsigned int tdc_min = gUser.GetParameter("TdcBGO", 0);
    static const unsigned int tdc_max = gUser.GetParameter("TdcBGO", 1);

    int bgo_fa_id   = gHist.getSequentialID(kBGO, 0, kFADC);
    int bgo_fawt_id = gHist.getSequentialID(kBGO, 0, kFADCwTDC);
    int bgo_a_id    = gHist.getSequentialID(kBGO, 0, kADC);
    int bgo_awt_id  = gHist.getSequentialID(kBGO, 0, kADCwTDC);
    int bgo_a2d_id  = gHist.getSequentialID(kBGO, 0, kADC2D);
    int bgo_t_id    = gHist.getSequentialID(kBGO, 0, kTDC);
    int bgo_t2d_id  = gHist.getSequentialID(kBGO, 0, kTDC2D);
    int bgo_hit_id  = gHist.getSequentialID(kBGO, 0, kHitPat,  1);
    int bgo_chit_id = gHist.getSequentialID(kBGO, 0, kHitPat,  2);
    int bgo_mul_id  = gHist.getSequentialID(kBGO, 0, kMulti,   1);
    int bgo_cmul_id = gHist.getSequentialID(kBGO, 0, kMulti,   2);

    unsigned int multiplicity  = 0;
    unsigned int cmultiplicity = 0;
    std::vector<Int_t> de_array( NumOfSegBGO );
    for(int seg=0; seg<NumOfSegBGO; ++seg){
      Int_t de  = 0;
      Int_t ped = 0;
      // FADC
      int nhit_f = gUnpacker.get_entries(k_device, 0, seg, 0, k_fadc);
      if(nhit_f==0){
        de_array.at(seg) = 0;
        continue;
      }
      for (int i=0; i<nhit_f; ++i) {
        unsigned int fadc = gUnpacker.get(k_device, 0, seg, 0, k_fadc ,i);
	if( fadc == 0xffff )
	  continue;
	if( ped == 0 && fadc>14000)
	  ped = fadc;
	hptr_array[bgo_fa_id + seg]->Fill( i+1, fadc);
	if (ped>0)
	  de += (ped - fadc);
      }
      hptr_array[bgo_a_id + seg]->Fill( de );
      hptr_array[bgo_a2d_id]->Fill( seg, de );
      // TDC && Hit pattern && multiplicity
      unsigned int nhit_l = gUnpacker.get_entries(k_device, 0, seg, 0, k_leading);
//      unsigned int nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_trailing);
//      unsigned int hit_l_max = 0;
//      unsigned int hit_t_max = 0;
//
//      if(nhit_l != 0){
//        hit_l_max = gUnpacker.get(k_device, 0, seg, 0, k_leading,  nhit_l - 1);
//      }
//      if(nhit_t != 0){
//        hit_t_max = gUnpacker.get(k_device, 0, seg, 0, k_trailing, nhit_t - 1);
//      }


      if(nhit_l!=0){
	// ADC wTDC
	for (int i=0; i<nhit_f; ++i) {
	  unsigned int fadc = gUnpacker.get(k_device, 0, seg, 0, k_fadc ,i);
	  hptr_array[bgo_fawt_id + seg]->Fill( i+1, fadc);
	}
	Bool_t is_hit = false;
        for(unsigned int m = 0; m<nhit_l; ++m){
	  unsigned int tdc = gUnpacker.get(k_device, 0, seg, 0, k_leading, m);
      	  if(tdc!=0){
      	    hptr_array[bgo_t_id + seg]->Fill(tdc);
      	    hptr_array[bgo_t2d_id]->Fill( seg, tdc );
	    hptr_array[bgo_hit_id]->Fill( seg );
	    ++multiplicity;
	    if(true && tdc_min < tdc && tdc < tdc_max){
	      bgo_is_hit = true;
	      is_hit = true;
              hptr_array[bgo_chit_id]->Fill(seg);
	      ++cmultiplicity;
	    }
      	  }
        }
	if( is_hit ){
	  de_array.at(seg) = de;
	  hptr_array[bgo_awt_id + seg]->Fill( de );
	  hptr_array[bgo_a2d_id + 1]->Fill( seg, de );
	}
      } else {
	de_array.at(seg) = 0;
      }
    }

    /*
    for( Int_t i=0, n=de_array.size(); i<n; ++i ){
      for( Int_t j=i+1; j<n; ++j ){
	if( de_array[i] > 0 && de_array[j] > 0 )
	  hptr_array[bgo_a2d_id+2]->Fill( de_array[i], de_array[j] );
      }
    }
    */

    /*
    for(int seg=0; seg<NumOfSegBGO_T; ++seg){
      unsigned int nhit_l = gUnpacker.get_entries(k_device, 1, seg, 0, k_leading);
      nhit_l = gUnpacker.get_entries(k_device, 1, seg, 0, k_leading);
      if(nhit_l!=0){
        for(unsigned int m = 0; m<nhit_l; ++m){
      	unsigned int tdc = gUnpacker.get(k_device, 1, seg, 0, k_leading, m);
      	  if(tdc!=0){
      	    hptr_array[bgo_t_id + NumOfSegBGO + seg]->Fill(tdc);
      	  }
        }
      }
    }
    */

    hptr_array[bgo_mul_id]->Fill(multiplicity);
    hptr_array[bgo_cmul_id]->Fill(cmultiplicity); // CMulti

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }// BGO


#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

#endif

#if 1
  //------------------------------------------------------------------
  // CFT
  //------------------------------------------------------------------
  {
    // data type
    static const int k_device   = gUnpacker.get_device_id("CFT");
    static const int k_leading  = gUnpacker.get_data_id("CFT" , "leading");
    static const int k_trailing = gUnpacker.get_data_id("CFT", "trailing");
    static const int k_highgain = gUnpacker.get_data_id("CFT" , "highgain");
    static const int k_lowgain  = gUnpacker.get_data_id("CFT", "lowgain");
    // TDC gate range
    static const int tdc_min = gUser.GetParameter("TdcCFT", 0);
    static const int tdc_max = gUser.GetParameter("TdcCFT", 1);

    // SequentialID
    int cft_t_id    = gHist.getSequentialID(kCFT, 0, kTDC,     1);
    int cft_t_2d_id = gHist.getSequentialID(kCFT, 0, kTDC2D,   1);

    int cft_tot_id     = gHist.getSequentialID(kCFT, 0, kADC,    1);
    int cft_tot_2d_id  = gHist.getSequentialID(kCFT, 0, kADC2D,  1);
    int cft_ctot_id    = gHist.getSequentialID(kCFT, 0, kADC,   11);
    int cft_ctot_2d_id = gHist.getSequentialID(kCFT, 0, kADC2D, 11);

    int cft_hg_id = gHist.getSequentialID(kCFT, 0, kHighGain, 1);
    int cft_pe_id = gHist.getSequentialID(kCFT, 0, kPede    , 1);
    int cft_lg_id = gHist.getSequentialID(kCFT, 0, kLowGain,  1);

    int cft_hg_2d_id = gHist.getSequentialID(kCFT, 0, kHighGain, 11);
    int cft_pe_2d_id = gHist.getSequentialID(kCFT, 0, kPede    , 11);
    int cft_lg_2d_id = gHist.getSequentialID(kCFT, 0, kLowGain,  11);

    int cft_chg_id = gHist.getSequentialID(kCFT, 0, kHighGain, 21);
    int cft_clg_id = gHist.getSequentialID(kCFT, 0, kLowGain,  21);

    int cft_chg_2d_id = gHist.getSequentialID(kCFT, 0, kHighGain, 31);
    int cft_clg_2d_id = gHist.getSequentialID(kCFT, 0, kLowGain,  31);

    int cft_hg_tot_id = gHist.getSequentialID(kCFT, 0, kHighGain, 41);
    int cft_lg_tot_id = gHist.getSequentialID(kCFT, 0, kLowGain,  41);

    int cft_hit_id     = gHist.getSequentialID(kCFT, 0, kHitPat,  1);
    int cft_chit_id    = gHist.getSequentialID(kCFT, 0, kHitPat, 11);
    int cft_chitbgo_id = gHist.getSequentialID(kCFT, 0, kMulti,  21);
    int cft_mul_id     = gHist.getSequentialID(kCFT, 0, kMulti,   1);
    int cft_cmul_id    = gHist.getSequentialID(kCFT, 0, kMulti,  11);
    int cft_cmulbgo_id = gHist.getSequentialID(kCFT, 0, kMulti,  21);

    int cft_cl_hgadc_id =gHist.getSequentialID(kCFT, kCluster, kHighGain, 1);
    int cft_cl_lgadc_id =gHist.getSequentialID(kCFT, kCluster, kLowGain,  1);
    int cft_cl_tdc_id   =gHist.getSequentialID(kCFT, kCluster, kTDC,      1);

    int cft_cl_hgadc2d_id =gHist.getSequentialID(kCFT, kCluster, kHighGain, 11);
    int cft_cl_lgadc2d_id =gHist.getSequentialID(kCFT, kCluster, kLowGain, 11);
    int cft_cl_tdc2d_id =gHist.getSequentialID(kCFT,kCluster, kTDC2D, 1);

    int multiplicity[NumOfLayersCFT] ;
    int cmultiplicity[NumOfLayersCFT];
    int cbgomultiplicity[NumOfLayersCFT];

    for(int l=0; l<NumOfLayersCFT; ++l){
      multiplicity[l]     = 0;
      cmultiplicity[l]    = 0;
      cbgomultiplicity[l] = 0;
      for(int i = 0; i<NumOfSegCFT[l]; ++i){
        int nhit_l = gUnpacker.get_entries(k_device, l, i, 0, k_leading );
        int nhit_t = gUnpacker.get_entries(k_device, l, i, 0, k_trailing );
        int hit_l_max = 0;
        int hit_t_max = 0;
        if(nhit_l==0)
	  continue;
	for(int m = 0; m<nhit_l; ++m){
	  int tdc = gUnpacker.get(k_device, l, i, 0, k_leading, m);
	  hptr_array[cft_t_id+l]->Fill(tdc);
	  hptr_array[cft_t_2d_id+l]->Fill(i, tdc);
	  hptr_array[cft_hit_id+l]->Fill(i);
	  ++multiplicity[l];
	  if(tdc_min < tdc && tdc < tdc_max){
	    ++cmultiplicity[l];
	    if( bgo_is_hit )
	      ++cbgomultiplicity[l];
	    hptr_array[cft_chit_id+l]->Fill(i);
	    if( bgo_is_hit )
	      hptr_array[cft_chitbgo_id+l]->Fill(i);
	  }
	}

	if(nhit_l != 0){
	  hit_l_max = gUnpacker.get(k_device, l, i, 0, k_leading,  nhit_l - 1);
	}
	if(nhit_t != 0){
	  hit_t_max = gUnpacker.get(k_device, l, i, 0, k_trailing, nhit_t - 1);
	}
	if(nhit_l == nhit_t && hit_l_max > hit_t_max){
	  for(int m = 0; m<nhit_l; ++m){
	    int tdc = gUnpacker.get(k_device, l, i, 0, k_leading, m);
	    int tdc_t = gUnpacker.get(k_device, l, i, 0, k_trailing, m);
	    int tot = tdc - tdc_t;
	    hptr_array[cft_tot_id+l]->Fill(tot);
	    hptr_array[cft_tot_2d_id+l]->Fill(i, tot);

	    if(tdc_min < tdc && tdc < tdc_max){
	      hptr_array[cft_ctot_id+l]->Fill(tot);
	      hptr_array[cft_ctot_2d_id+l]->Fill(i, tot);

	      if(m!= nhit_l-1 )continue;
	      for(int j=0; j<2; j++){
		if(j==0){
		  int nhit_hg = gUnpacker.get_entries(k_device, l, i, 0, k_highgain);
		  if(nhit_hg==0)continue;
		  int adc_hg = gUnpacker.get(k_device, l, i, 0, k_highgain, 0);
		  hptr_array[cft_chg_id+l]->Fill(adc_hg);
		  hptr_array[cft_chg_2d_id+l]->Fill(i, adc_hg);
		  hptr_array[cft_hg_tot_id+l]->Fill(adc_hg, tot);
		}
		if(j==1){
		  int nhit_lg = gUnpacker.get_entries(k_device, l, i, 0, k_lowgain);
		  if(nhit_lg==0)continue;
		  int adc_lg = gUnpacker.get(k_device, l, i, 0, k_lowgain, 0);
		  hptr_array[cft_clg_id+l]->Fill(adc_lg);
		  hptr_array[cft_clg_2d_id+l]->Fill(i, adc_lg);
		  hptr_array[cft_lg_tot_id+l]->Fill(adc_lg, tot);
		}
	      }
	    }
	  }
	}
      }

      hptr_array[cft_mul_id+l]->Fill(multiplicity[l]);
      hptr_array[cft_cmul_id+l]->Fill(cmultiplicity[l]);
      hptr_array[cft_cmulbgo_id+l]->Fill(cbgomultiplicity[l]);

      for(int i = 0; i<NumOfSegCFT[l]; ++i){
        int nhit_hg = gUnpacker.get_entries(k_device, l, i, 0, k_highgain);
        if(nhit_hg==0)continue;
	for(int m = 0; m<nhit_hg; ++m){
	  int adc_hg = gUnpacker.get(k_device, l, i, 0, k_highgain, m);
	  hptr_array[cft_hg_id+l]->Fill(adc_hg);
	  hptr_array[cft_hg_2d_id+l]->Fill(i, adc_hg);
	  double pedeCFT = 0.;
	  double gainCFT = 1.;
	  pedeCFT = gHodo.GetP0(k_device,l,i,0);
	  // gainCFT = gHodo.GetGain(DetIdCFT,layer,i,0);
	  double adc_pe = ((double)adc_hg - pedeCFT)/gainCFT;
	  hptr_array[cft_pe_id+l]->Fill(adc_pe);
	  hptr_array[cft_pe_2d_id+l]->Fill(i, adc_pe);
	}
      }
      for(int i = 0; i<NumOfSegCFT[l]; ++i){
        int nhit_lg = gUnpacker.get_entries(k_device, l, i, 0, k_lowgain );
        if(nhit_lg==0)continue;
	for(int m = 0; m<nhit_lg; ++m){
	  int adc_lg = gUnpacker.get(k_device, l, i, 0, k_lowgain, m);
	  hptr_array[cft_lg_id+l]->Fill(adc_lg);
	  hptr_array[cft_lg_2d_id+l]->Fill(i, adc_lg);
	}
      }
    }
    hptr_array[cft_mul_id+NumOfLayersCFT]->Fill(multiplicity[0] + multiplicity[2]
						+ multiplicity[4] + multiplicity[6]);
    hptr_array[cft_mul_id+NumOfLayersCFT + 1]->Fill(multiplicity[1] + multiplicity[3]
						    + multiplicity[5] + multiplicity[7]);
    hptr_array[cft_cmul_id+NumOfLayersCFT]->Fill(cmultiplicity[0] + cmultiplicity[2]
						 + cmultiplicity[4] + cmultiplicity[6]);
    hptr_array[cft_cmul_id+NumOfLayersCFT + 1]->Fill(cmultiplicity[1] + cmultiplicity[3]
						     + cmultiplicity[5] + cmultiplicity[7]);

#if 0
    //clustering analysis
  hodoAna->DecodeCFTHits(rawData);
  // Fiber Cluster
  for(int p = 0; p<NumOfPlaneCFT; ++p){
    //int nhits = hodoAna->GetNHitsCFT(p);
    //std::cout<<"plane= "<<p<<",NHits="<<nhits<<std::endl;

    //hodoAna->TimeCutCFT(p, -30, 30); // CATCH@J-PARC
    hodoAna->AdcCutCFT(p, 0, 4000); // CATCH@J-PARC
    //hodoAna->AdcCutCFT(p, 50, 4000); // CATCH@J-PARC  for proton
    //hodoAna->WidthCutCFT(p, 60, 300); // pp scattering
    //hodoAna->WidthCutCFT(p, 30, 300); // cosmic ray

    int ncl = hodoAna->GetNClustersCFT(p);
    //std::cout<<"plane= "<<p<<",NClusters="<<ncl<<std::endl;
    for(int i=0; i<ncl; ++i){
      FiberCluster *cl = hodoAna->GetClusterCFT(p,i);
      if(!cl) continue;
      //      double size  = cl->ClusterSize();
      //double seg = cl->MeanSeg();
      //double ctime = cl->CMeanTime();
      //double width = -cl->minWidth();
      //double width = cl->Width();
      //double sumADC= cl->SumAdcLow();
      //double sumMIP= cl->SumMIPLow();
      //double sumdE = cl->SumdELow();
      //double r     = cl->MeanPositionR();
      //double phi   = cl->MeanPositionPhi();
      int Mseg  = cl->MaxSeg();
      int MADCHi  = cl->MaxAdcHi();
      int MADCLow = cl->MaxAdcLow();
      //std::cout<<"cluster="<<i<<" , Maxseg="<<Mseg<<" , MaxADC"<<MADCHi<<std::endl;
      hptr_array[cft_cl_hgadc_id + p] ->Fill(MADCHi);
      hptr_array[cft_cl_lgadc_id + p] ->Fill(MADCLow);
      hptr_array[cft_cl_hgadc2d_id + p] ->Fill(Mseg, MADCHi);
      hptr_array[cft_cl_lgadc2d_id + p] ->Fill(Mseg, MADCLow);
      int fmulti =hodoAna->GetNHitsCFT(p);
      for(int j=0; j<fmulti; ++j){
	FiberHit *fhit = hodoAna->GetHitCFT(p,j);
	int seg_temp = fhit->PairId();
	if(seg_temp == Mseg){
	  int Mhit =fhit ->GetNumOfHit();
	  for(int k=0; k<Mhit; ++k){
	    double Mtime =fhit->GetLeading(k);
	    hptr_array[cft_cl_tdc_id + p] ->Fill(Mtime);
	    hptr_array[cft_cl_tdc2d_id + p] ->Fill(Mseg, Mtime);
	  }
	}
      }
    }
  }

  delete rawData;
  delete hodoAna;
#endif

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//CFT

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  //------------------------------------------------------------------
  // PiID
  //------------------------------------------------------------------
  {
    // data type
    static const int k_device   = gUnpacker.get_device_id("PiID");
    static const int k_hg      = gUnpacker.get_data_id("PiID", "highgain");
    static const int k_lg      = gUnpacker.get_data_id("PiID", "lowgain");
    static const int k_tdc      = gUnpacker.get_data_id("PiID", "leading");

    // TDC gate range
    static const unsigned int tdc_min = gUser.GetParameter("TdcPiID", 0);
    static const unsigned int tdc_max = gUser.GetParameter("TdcPiID", 1);

    int piidhg_id   = gHist.getSequentialID(kPiID, 0, kHighGain);
    int piidlg_id   = gHist.getSequentialID(kPiID, 0, kLowGain);
    int piidt_id   = gHist.getSequentialID(kPiID, 0, kTDC);
    int piidhgwt_id = gHist.getSequentialID(kPiID, 0, kADCwTDC);
    int piidlgwt_id = gHist.getSequentialID(kPiID, 0, kADCwTDC,NumOfSegPiID +1);
    int piidhg2d_id = gHist.getSequentialID(kPiID, 0, kADC2D, 1);
    int piidlg2d_id = gHist.getSequentialID(kPiID, 0, kADC2D, 11);

    for(int seg=0; seg<NumOfSegPiID; ++seg){
      // High ADC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, 0, k_hg);
      if(nhit!=0){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, 0, k_hg);
	hptr_array[piidhg_id + seg]->Fill(adc);
	hptr_array[piidhg2d_id]->Fill( seg, adc );
      }

      // Low ADC
      nhit = gUnpacker.get_entries(k_device, 0, seg, 0, k_lg);
      if(nhit!=0){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, 0, k_lg);
	hptr_array[piidlg_id + seg]->Fill(adc);
	hptr_array[piidlg2d_id]->Fill( seg, adc );
      }

      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      if(nhit!=0){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc);
	if(tdc!=0){
	  hptr_array[piidt_id + seg]->Fill(tdc);
	  //ADCwTDC
	  if( gUnpacker.get_entries(k_device, 0, seg, 0, k_hg)>0 ){
	    unsigned int hadc = gUnpacker.get(k_device, 0, seg, 0, k_hg);
	    hptr_array[piidhgwt_id + seg]->Fill( hadc );
	  }
	  if( gUnpacker.get_entries(k_device, 0, seg, 0, k_lg)>0 ){
	    unsigned int ladc = gUnpacker.get(k_device, 0, seg, 0, k_lg);
	    hptr_array[piidlgwt_id + seg]->Fill( ladc );
	  }
	}
      }
    }

    // // Hit pattern && multiplicity
    static const int piidhit_id = gHist.getSequentialID(kPiID, 0, kHitPat);
    static const int piidmul_id = gHist.getSequentialID(kPiID, 0, kMulti);
    int multiplicity  = 0;
    int cmultiplicity = 0;
    for(int seg=0; seg<NumOfSegPiID; ++seg){
      int nhit_piid = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      // AND
      if(nhit_piid!=0){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc);
	// TDC AND
	if(tdc != 0){
	  hptr_array[piidhit_id]->Fill(seg);
	  ++multiplicity;
	  // TDC range
	  if(true
	     && tdc_min < tdc && tdc < tdc_max
	     ){
	    hptr_array[piidhit_id+1]->Fill(seg); // CHitPat
	    ++cmultiplicity;
	  }// TDC range OK
	}// TDC AND
      }// AND
    }// for(seg)

    hptr_array[piidmul_id]->Fill(multiplicity);
    hptr_array[piidmul_id+1]->Fill(cmultiplicity); // CMulti

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }// PiID

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif
#if 1
  // Correlation (2D histograms) -------------------------------------
  {
    static const int k_device_cft   = gUnpacker.get_device_id("CFT");
    static const int k_device_bgo   = gUnpacker.get_device_id("BGO");
    static const int k_device_piid   = gUnpacker.get_device_id("PiID");
    static const int k_leading  = gUnpacker.get_data_id("CFT" , "leading");
//    static const int k_fadc      = gUnpacker.get_data_id("BGO", "fadc");
    static const int k_BGOleading      = gUnpacker.get_data_id("BGO", "tdc");
    static const int k_PiIDleading      = gUnpacker.get_data_id("PiID", "leading");
//    int BGO_Vth = 1000;
//    static const int BGO_Vth = gUser.GetParameter("BGO_Vth", 0);

    // sequential id
    int cor_id= gHist.getSequentialID(kCorrelation_catch, 0, 0, 1);

    // CFT vs BGO
    for(int l = 0; l<NumOfLayersCFT-4; l++){
        for(int seg1 = 0; seg1<NumOfSegCFT[l*2+1]; ++seg1){
            for(int seg2 = 0; seg2<NumOfSegBGO; ++seg2){
                int hitCFT = gUnpacker.get_entries(k_device_cft, l*2+1, seg1, 0, k_leading);
                int hitBGO = gUnpacker.get_entries(k_device_bgo, 0, seg2, 0, k_BGOleading);
                if(hitCFT == 0 || hitBGO == 0)continue;
                int tdcCFT = gUnpacker.get(k_device_cft, l*2+1, seg1, 0, k_leading);
                int tdcBGO = gUnpacker.get(k_device_bgo, 0, seg2, 0, k_BGOleading);
                if(tdcCFT != 0&&tdcBGO !=0){
		  hptr_array[cor_id + l]->Fill(seg1, seg2);
                }
            }
        }
    }

   //  BGO vs PiID
    cor_id= gHist.getSequentialID(kCorrelation_catch, 1, 0, 1);
    for(int seg1 = 0; seg1<NumOfSegBGO; ++seg1){
      for(int seg2 = 0; seg2<NumOfSegPiID; ++seg2){
	int hitBGO = gUnpacker.get_entries(k_device_bgo, 0, seg1, 0, k_BGOleading);
	int hitPiID = gUnpacker.get_entries(k_device_piid, 0, seg2, 0, k_PiIDleading);
	if(hitBGO == 0 || hitPiID == 0)
	  continue;
	int tdcBGO = gUnpacker.get(k_device_bgo, 0, seg1, 0, k_BGOleading);
	int tdcPiID = gUnpacker.get(k_device_piid, 0, seg2, 0, k_PiIDleading);
	if(tdcBGO != 0 && tdcPiID !=0){
	  hptr_array[cor_id]->Fill(seg1, seg2);
	}
      }
    }

  }// Correlation
#endif
#endif
#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

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
  //   if(host.Contains("online") &&
  //      event_number > 1 && curr_time - prev_time > 3){
  //     std::cout << "exec tagslip sound!" << std::endl;
  //     gSystem->Exec("ssh db-hyps \"aplay /misc/software/online-analyzer/dev/sound/tagslip.wav\" &");
  //     prev_time = curr_time;
  //   }
  // }

  gSystem->ProcessEvents();

  return 0;
}

}
