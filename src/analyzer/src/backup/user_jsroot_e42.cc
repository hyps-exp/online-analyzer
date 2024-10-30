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
#include "TpcPadHelper.hh"
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
      auto& gTpcPad   = TpcPadHelper::GetInstance();
auto&       gMsT      = MsTParamMan::GetInstance();
const auto& gUser     = UserParamMan::GetInstance();
}

namespace analyzer
{

//____________________________________________________________________________
int
process_begin( const std::vector<std::string>& argv )
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
  gConfMan.InitializeParameter<TpcPadHelper>("TPCPAD");
  gConfMan.InitializeParameter<UserParamMan>("USER");
  if( !gConfMan.IsGood() ) return -1;

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
  gHttp.Register(gHist.createBH1());
  gHttp.Register(gHist.createBFT());
  gHttp.Register(gHist.createBC3());
  gHttp.Register(gHist.createBC4());
  gHttp.Register(gHist.createBAC());
  gHttp.Register(gHist.createBH2());
  gHttp.Register(gHist.createHTOF());
  gHttp.Register(gHist.createSDC1());
  gHttp.Register(gHist.createSDC2());
  gHttp.Register(gHist.createSCH());
  gHttp.Register(gHist.createSDC3());
  gHttp.Register(gHist.createSDC4());
  gHttp.Register(gHist.createBVH());
  gHttp.Register(gHist.createTOF());
  gHttp.Register(gHist.createLAC());
  gHttp.Register(gHist.createWC());
  gHttp.Register(gHist.createTPC());
  gHttp.Register(gHist.createCorrelation());
  gHttp.Register(gHist.createTriggerFlag());
  gHttp.Register(gHist.createDAQ());
  gHttp.Register(gHist.createDCEff());
  gHttp.Register(gHist.createBTOF());
  // gHttp.Register(gHist.createMatrix());

  if(0 != gHist.setHistPtr(hptr_array)){ return -1; }

  //___ Macro for HttpServer
  gHttp.Register(http::BH1ADC());
  gHttp.Register(http::BH1TDC());
  gHttp.Register(http::BFT());
  gHttp.Register(http::BC3TDCTOT());
  gHttp.Register(http::BC3HitMulti());
  gHttp.Register(http::BC4TDCTOT());
  gHttp.Register(http::BC4HitMulti());
  gHttp.Register(http::BAC());
  gHttp.Register(http::BH2ADC());
  gHttp.Register(http::BH2TDC());
  // gHttp.Register(http::T0());
  gHttp.Register(http::HTOFADCU());
  gHttp.Register(http::HTOFADCD());
  gHttp.Register(http::HTOFTDCU());
  gHttp.Register(http::HTOFTDCD());
  gHttp.Register(http::BeamHitMulti());
  gHttp.Register(http::SDC1TDCTOT());
  gHttp.Register(http::SDC1HitMulti());
  gHttp.Register(http::SCHTDCU());
  gHttp.Register(http::SCHTDCD());
  gHttp.Register(http::SCHTOTU());
  gHttp.Register(http::SCHTOTD());
  gHttp.Register(http::SCHHitMulti());
  gHttp.Register(http::SDC2TDCTOT());
  gHttp.Register(http::SDC2HitMulti());
  gHttp.Register(http::SDC3TDCTOT());
  gHttp.Register(http::SDC3HitMulti());
  gHttp.Register(http::SDC4TDCTOT());
  gHttp.Register(http::SDC4HitMulti());
  gHttp.Register(http::BVHTDC());
  gHttp.Register(http::TOFADCU());
  gHttp.Register(http::TOFADCD());
  gHttp.Register(http::TOFTDCU());
  gHttp.Register(http::TOFTDCD());
  gHttp.Register(http::LACTDC());
  gHttp.Register(http::WCADCU());
  gHttp.Register(http::WCADCD());
  gHttp.Register(http::WCADCSUM());
  gHttp.Register(http::WCTDCU());
  gHttp.Register(http::WCTDCD());
  gHttp.Register(http::WCTDCSUM());
  gHttp.Register(http::ScatHitMulti());
  gHttp.Register(http::TPC());
  gHttp.Register(http::TPC2D());
  gHttp.Register(http::TPC3D());
  gHttp.Register(http::TPCADCPAD());
  gHttp.Register(http::TPCTDCPAD());
  gHttp.Register(http::TPCHTOFPAD());
  gHttp.Register(http::TPCFADC());
  gHttp.Register(http::TriggerFlagU());
  gHttp.Register(http::TriggerFlagD());
  gHttp.Register(http::TriggerFlagHitPat());
  gHttp.Register(http::HitPatternBeam());
  gHttp.Register(http::HitPatternScat());
  gHttp.Register(http::BcOutEfficiency());
  gHttp.Register(http::SdcInOutEfficiency());
  gHttp.Register(http::Correlation());
  gHttp.Register(http::BFTSCHTOT());
  gHttp.Register(http::DAQ());

  for( Int_t i=0, n=hptr_array.size(); i<n; ++i ){
    hptr_array[i]->SetDirectory(0);
  }

  return 0;
}

//____________________________________________________________________________
int
process_end( void )
{
  hptr_array.clear();
  return 0;
}

//____________________________________________________________________________
int
process_event( void )
{
  static Int_t run_number = -1;
  {
    if( run_number != gUnpacker.get_root()->get_run_number() ){
      for( Int_t i=0, n=hptr_array.size(); i<n; ++i ){
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
	  // if( buf.Contains("bUuSELselthdg") ){
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
    if( !( trigger_flag[trigger::kSpillEnd] |
           trigger_flag[trigger::kLevel1OR] ) ){
      hddaq::cerr << "#W Trigger flag is missing!!! "
        	  << trigger_flag << std::endl;
    }
#if 0
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

#if TIME_STAMP
  // TimeStamp --------------------------------------------------------
  {
    static const auto hist_id = gHist.getSequentialID( kTimeStamp, 0, kTDC );
    Int_t i = 0;
    for( auto&& c : gUnpacker.get_root()->get_child_list() ){
      if( !c.second )
	continue;
      auto t = gUnpacker.get_node_header( c.second->get_id(),
                                          DAQNode::k_unix_time );
      hptr_array[hist_id+i]->Fill( t );
      ++i;
    }
  }
#endif

  UInt_t cobo_data_size = 0;
#if FLAG_DAQ
  { ///// DAQ
    //___ node id
    static const Int_t k_eb = gUnpacker.get_fe_id("k18eb");
    std::vector<Int_t> vme_fe_id;
    std::vector<Int_t> hul_fe_id;
    std::vector<Int_t> ea0c_fe_id;
    std::vector<Int_t> cobo_fe_id;
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
      if(n.Contains("cobo"))
	cobo_fe_id.push_back(id);
    }

    //___ sequential id
    static const Int_t eb_hid = gHist.getSequentialID(kDAQ, kEB, kHitPat);
    static const Int_t vme_hid = gHist.getSequentialID(kDAQ, kVME, kHitPat2D);
    static const Int_t hul_hid = gHist.getSequentialID(kDAQ, kHUL, kHitPat2D);
    static const Int_t ea0c_hid = gHist.getSequentialID(kDAQ, kEASIROC, kHitPat2D);
    static const Int_t cobo_hid = gHist.getSequentialID(kDAQ, kCoBo, kHitPat2D);

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

    { // EASIROC & VMEEASIROC node
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

    { //___ CoBo node
      for(Int_t i=0, n=cobo_fe_id.size(); i<n; ++i){
        auto data_size = gUnpacker.get_node_header(cobo_fe_id[i], DAQNode::k_data_size);
        hptr_array[cobo_hid]->Fill(i, data_size);
	cobo_data_size += data_size;
      }
    }
  }

#endif

  if(trigger_flag[trigger::kSpillEnd])
    return 0;

// //   // MsT -----------------------------------------------------------
// //   {
// //     static const Int_t k_device = gUnpacker.get_device_id("MsT");
// //     static const Int_t k_tof    = gUnpacker.get_plane_id("MsT", "TOF");
// //     static const Int_t k_sch    = gUnpacker.get_plane_id("MsT", "SCH");
// //     static const Int_t k_tag    = gUnpacker.get_plane_id("MsT", "tag");
// //     static const Int_t k_ch     = 0;
// //     static const Int_t k_tdc    = 0;
// //     static const Int_t k_n_flag = 7;

// //     static const Int_t toft_id     = gHist.getSequentialID(kMsT, 0, kTDC);
// //     static const Int_t scht_id     = gHist.getSequentialID(kMsT, 0, kTDC, NumOfSegTOF*2 +1);
// //     static const Int_t toft_2d_id  = gHist.getSequentialID(kMsT, 0, kTDC2D);

// //     static const Int_t tofhit_id   = gHist.getSequentialID(kMsT, 0, kHitPat, 0);
// //     static const Int_t schhit_id   = gHist.getSequentialID(kMsT, 0, kHitPat, 1);
// //     static const Int_t flag_id     = gHist.getSequentialID(kMsT, 0, kHitPat, 2);

// //     // TOF
// //     for(Int_t seg=0; seg<NumOfSegTOF; ++seg){
// //       Int_t nhit = gUnpacker.get_entries(k_device, k_tof, seg, k_ch, k_tdc);
// //       for(Int_t m = 0; m<nhit; ++m){
// // 	hptr_array[tofhit_id]->Fill(seg);

// // 	UInt_t tdc = gUnpacker.get(k_device, k_tof, seg, k_ch, k_tdc, m);
// // 	if(tdc!=0){
// // 	  hptr_array[toft_id + seg]->Fill(tdc);
// // 	  hptr_array[toft_2d_id]->Fill(seg, tdc);
// // 	}
// //       }
// //     }// TOF

// //     // SCH
// //     for(Int_t seg=0; seg<NumOfSegSCH; ++seg){
// //       Int_t nhit = gUnpacker.get_entries(k_device, k_sch, seg, k_ch, k_tdc);
// //       if(nhit!=0){
// // 	hptr_array[schhit_id]->Fill(seg);
// //       }
// //       for(Int_t m = 0; m<nhit; ++m){
// // 	UInt_t tdc = gUnpacker.get(k_device, k_sch, seg, k_ch, k_tdc, m);
// // 	if(tdc!=0){
// // 	  hptr_array[scht_id + seg]->Fill(tdc);
// // 	}
// //       }
// //     }// SCH

// //     // FLAG
// //     for(Int_t i=0; i<k_n_flag; ++i){
// //       Int_t nhit = gUnpacker.get_entries(k_device, k_tag, 0, k_ch, i);
// //       if(nhit!=0){
// // 	Int_t flag = gUnpacker.get(k_device, k_tag, 0, k_ch, i, 0);
// // 	if(flag) hptr_array[flag_id]->Fill(i);
// //       }
// //     }// FLAG

// // #if 0
// //     // Debug, dump data relating this detector
// //     gUnpacker.dump_data_device(k_device);
// // #endif
// //   }// MsT

// // #if DEBUG
// //   std::cout << __FILE__ << " " << __LINE__ << std::endl;
// // #endif

  std::vector<Int_t> hitseg_bh1;
  { ///// BH1
    static const auto device_id = gUnpacker.get_device_id("BH1");
    static const auto adc_id = gUnpacker.get_data_id("BH1", "adc");
    static const auto tdc_id = gUnpacker.get_data_id("BH1", "fpga_leading");
    static const auto tdc_min = gUser.GetParameter("BH1_TDC", 0);
    static const auto tdc_max = gUser.GetParameter("BH1_TDC", 1);
    static const auto adc_hid = gHist.getSequentialID(kBH1, 0, kADC);
    static const auto tdc_hid = gHist.getSequentialID(kBH1, 0, kTDC);
    static const auto awt_hid = gHist.getSequentialID(kBH1, 0, kADCwTDC);
    static const auto hit_hid = gHist.getSequentialID(kBH1, 0, kHitPat);
    static const auto mul_hid = gHist.getSequentialID(kBH1, 0, kMulti);
    std::vector<std::vector<Int_t>> hit_flag(NumOfSegBH1);
    UInt_t multiplicity = 0;
    for(Int_t seg=0; seg<NumOfSegBH1; ++seg){
      hit_flag[seg].resize(kUorD);
      for(Int_t ud=0; ud<kUorD; ++ud){
	hit_flag[seg][ud] = 0;
	///// ADC
	UInt_t adc = 0;
	auto nhit = gUnpacker.get_entries(device_id, 0, seg, ud, adc_id);
	if(nhit != 0){
	  adc = gUnpacker.get(device_id, 0, seg, ud, adc_id);
	  hptr_array[adc_hid + seg + ud*NumOfSegBH1]->Fill(adc);
	}
	///// TDC
	for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, seg, ud, tdc_id);
	    m<n; ++m){
	  auto tdc = gUnpacker.get(device_id, 0, seg, ud, tdc_id, m);
	  if(tdc != 0){
	    hptr_array[tdc_hid + seg + ud*NumOfSegBH1]->Fill(tdc);
	    // ADC wTDC
	    if( tdc > tdc_min && tdc < tdc_max && adc > 0 ){
	      hit_flag[seg][ud] = 1;
	    }
	  }
	}
	if(hit_flag[seg][ud] == 1){
	  hptr_array[awt_hid + seg + ud*NumOfSegBH1]->Fill(adc);
	}
      }
      if(hit_flag[seg][kU] == 1 && hit_flag[seg][kD] == 1){
	++multiplicity;
	hptr_array[hit_hid]->Fill(seg);
	hitseg_bh1.push_back(seg);
      }
    }
    hptr_array[mul_hid]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }// BH1

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  std::vector<Int_t> hitseg_bftu;
  { ///// BFT
    static const auto device_id = gUnpacker.get_device_id("BFT");
    static const auto leading_id = gUnpacker.get_data_id("BFT", "leading");
    static const auto trailing_id = gUnpacker.get_data_id("BFT", "trailing");
    static const auto tdc_min = gUser.GetParameter("BFT_TDC", 0);
    static const auto tdc_max = gUser.GetParameter("BFT_TDC", 1);
    static const auto tdc_hid = gHist.getSequentialID(kBFT, 0, kTDC);
    static const auto tot_hid = gHist.getSequentialID(kBFT, 0, kADC);
    static const auto hit_hid = gHist.getSequentialID(kBFT, 0, kHitPat);
    static const auto mul_hid = gHist.getSequentialID(kBFT, 0, kMulti);
    static const auto tdc_2d_hid = gHist.getSequentialID(kBFT, 0, kTDC2D);
    static const auto tot_2d_hid = gHist.getSequentialID(kBFT, 0, kADC2D);
    UInt_t multiplicity = 0; // includes both u and d planes.
    for(Int_t i=0; i<NumOfSegBFT; ++i){
      for(Int_t ud=0; ud<kUorD; ++ud){
	UInt_t tdc_prev = 0;
	Bool_t is_in_range = false;
	for(Int_t m=0, n=gUnpacker.get_entries(device_id, ud, 0, i, leading_id);
	    m<n; ++m){
	  auto tdc = gUnpacker.get(device_id, ud, 0, i, leading_id, m);
	  auto tdc_t = gUnpacker.get(device_id, ud, 0, i, trailing_id, m);
	  auto tot = tdc - tdc_t;
	  if(tdc_prev == tdc || tdc <= 0 || tot <= 0)
	    continue;
	  tdc_prev = tdc;
	  hptr_array[tdc_hid + ud]->Fill(tdc);
	  hptr_array[tot_hid + ud]->Fill(tot);
	  if(tdc_min < tdc && tdc < tdc_max){
	    is_in_range = true;
	  }
	  hptr_array[tdc_2d_hid + ud]->Fill(i, tdc);
	  hptr_array[tot_2d_hid + ud]->Fill(i, tot);
	}
	if(is_in_range){
	  ++multiplicity;
	  hptr_array[hit_hid + ud]->Fill(i);
	  hitseg_bftu.push_back(i);
	}
      }
    }
    hptr_array[mul_hid]->Fill(multiplicity);

#if 0
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BC3 -------------------------------------------------------------
  //for HULMHTDC
  std::vector< std::vector<Int_t> > BC3HitCont(6);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("BC3");
    static const Int_t k_leading  = gUnpacker.get_data_id("BC3", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("BC3", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("BC3_TDC", 0);
    static const Int_t tdc_max = gUser.GetParameter("BC3_TDC", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("BC3_TOT", 0);
    // static const Int_t tot_max = gUser.GetParameter("BC3_TOT", 1);

    // sequential id
    static const Int_t bc3t_id    = gHist.getSequentialID(kBC3, 0, kTDC);
    static const Int_t bc3tot_id  = gHist.getSequentialID(kBC3, 0, kADC);
    static const Int_t bc3t1st_id = gHist.getSequentialID(kBC3, 0, kTDC2D);
    static const Int_t bc3hit_id  = gHist.getSequentialID(kBC3, 0, kHitPat);
    static const Int_t bc3mul_id  = gHist.getSequentialID(kBC3, 0, kMulti);
    static const Int_t bc3mulwt_id
      = gHist.getSequentialID(kBC3, 0, kMulti, 1+NumOfLayersBC3);

    static const Int_t bc3t_ctot_id    = gHist.getSequentialID(kBC3, 0, kTDC,    1+kTOTcutOffset);
    static const Int_t bc3tot_ctot_id  = gHist.getSequentialID(kBC3, 0, kADC,    1+kTOTcutOffset);
    static const Int_t bc3t1st_ctot_id = gHist.getSequentialID(kBC3, 0, kTDC2D,  1+kTOTcutOffset);
    static const Int_t bc3hit_ctot_id  = gHist.getSequentialID(kBC3, 0, kHitPat, 1+kTOTcutOffset);
    static const Int_t bc3mul_ctot_id  = gHist.getSequentialID(kBC3, 0, kMulti,  1+kTOTcutOffset);
    static const Int_t bc3mulwt_ctot_id
      = gHist.getSequentialID(kBC3, 0, kMulti, 1+NumOfLayersBC3 + kTOTcutOffset);
    static const Int_t bc3self_corr_id  = gHist.getSequentialID(kBC3, kSelfCorr, 0, 1);


    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersBC3; ++l){
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      for( Int_t w=0; w<NumOfWireBC3; ++w ){
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if( nhit_l == 0 ) continue;

        Int_t hit_l_max = 0;
        Int_t hit_t_max = 0;

        if(nhit_l != 0){
          hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
        }
        if(nhit_t != 0){
          hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
        }

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[bc3hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for( Int_t m = 0; m<nhit_l; ++m ){
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[bc3t_id + l]->Fill(tdc);
	  if( tdc1st<tdc ) tdc1st = tdc;

	  // Drift time check
	  if( tdc_min < tdc && tdc < tdc_max ){
	    flag_hit_wt = true;
	  }
	}

	if( tdc1st!=0 ) hptr_array[bc3t1st_id +l]->Fill( tdc1st );
	if( flag_hit_wt ){
	  ++multiplicity_wt;
	  hptr_array[bc3hit_id + l]->Fill( w );
	}

	tdc1st = 0;
        if(nhit_l == nhit_t && hit_l_max > hit_t_max){
	  ++multiplicity_ctot;
          for(Int_t m = 0; m<nhit_l; ++m){
            tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
            tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
            tot = tdc - tdc_t;
            hptr_array[bc3tot_id+l]->Fill(tot);
            if(tot < tot_min) continue;
	    hptr_array[bc3t_ctot_id + l]->Fill(tdc);
	    hptr_array[bc3tot_ctot_id+l]->Fill(tot);
	    if( tdc1st<tdc ) tdc1st = tdc;
	    if( tdc_min < tdc && tdc < tdc_max ){
	      flag_hit_wt_ctot = true;
	    }
          }
        }

	if( tdc1st!=0 ) hptr_array[bc3t1st_ctot_id +l]->Fill( tdc1st );
	if( flag_hit_wt_ctot ){
	  ++multiplicity_wt_ctot;
	  hptr_array[bc3hit_ctot_id + l]->Fill( w );
          BC3HitCont[l].push_back(w);
        }
      }

      hptr_array[bc3mul_id + l]->Fill(multiplicity);
      hptr_array[bc3mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[bc3mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[bc3mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }


    for(Int_t s=0; s<NumOfDimBC3 ;s++){
      Int_t corr=2*s;
      for(UInt_t i=0; i<BC3HitCont[corr].size() ;i++){
        for(UInt_t j=0; j<BC3HitCont[corr+1].size() ;j++){
          hptr_array[bc3self_corr_id + s]->Fill(BC3HitCont[corr][i],BC3HitCont[corr+1][j]);
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

  // BC4 -------------------------------------------------------------
  //for HULMHTDC
  std::vector< std::vector<Int_t> > BC4HitCont(6);
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("BC4");
    static const Int_t k_leading = gUnpacker.get_data_id("BC4", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("BC4", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("BC4_TDC", 0);
    static const Int_t tdc_max = gUser.GetParameter("BC4_TDC", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("BC4_TOT", 0);
    // static const Int_t tot_max = gUser.GetParameter("BC4_TOT", 1);

    // sequential id
    static const Int_t bc4t_id = gHist.getSequentialID(kBC4, 0, kTDC);
    static const Int_t bc4tot_id = gHist.getSequentialID(kBC4, 0, kADC);
    static const Int_t bc4t1st_id = gHist.getSequentialID(kBC4, 0, kTDC2D);
    static const Int_t bc4hit_id = gHist.getSequentialID(kBC4, 0, kHitPat);
    static const Int_t bc4mul_id = gHist.getSequentialID(kBC4, 0, kMulti);
    static const Int_t bc4mulwt_id
      = gHist.getSequentialID(kBC4, 0, kMulti, 1+NumOfLayersBC4);

    static const Int_t bc4t_ctot_id = gHist.getSequentialID(kBC4, 0, kTDC, 1+kTOTcutOffset);
    static const Int_t bc4tot_ctot_id = gHist.getSequentialID(kBC4, 0, kADC, 1+kTOTcutOffset);
    static const Int_t bc4t1st_ctot_id = gHist.getSequentialID(kBC4, 0, kTDC2D, 1+kTOTcutOffset);
    static const Int_t bc4hit_ctot_id = gHist.getSequentialID(kBC4, 0, kHitPat, 1+kTOTcutOffset);
    static const Int_t bc4mul_ctot_id = gHist.getSequentialID(kBC4, 0, kMulti, 1+kTOTcutOffset);
    static const Int_t bc4mulwt_ctot_id
      = gHist.getSequentialID(kBC4, 0, kMulti, 1+NumOfLayersBC4 + kTOTcutOffset);
    static const Int_t bc4self_corr_id  = gHist.getSequentialID(kBC4, kSelfCorr, 0, 1);

    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersBC4; ++l){
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      for( Int_t w=0; w<NumOfWireBC4; ++w ){
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if( nhit_l == 0 ) continue;

        Int_t hit_l_max = 0;
        Int_t hit_t_max = 0;

        if(nhit_l != 0){
          hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
        }
        if(nhit_t != 0){
          hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
        }

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[bc4hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for( Int_t m = 0; m<nhit_l; ++m ){
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[bc4t_id + l]->Fill(tdc);
	  if( tdc1st<tdc ) tdc1st = tdc;

	  // Drift time check
	  if( tdc_min < tdc && tdc < tdc_max ){
	    flag_hit_wt = true;
	  }
	}

	if( tdc1st!=0 ) hptr_array[bc4t1st_id +l]->Fill( tdc1st );
	if( flag_hit_wt ){
	  ++multiplicity_wt;
	  hptr_array[bc4hit_id + l]->Fill( w );
	}

	tdc1st = 0;
        if(nhit_l == nhit_t && hit_l_max > hit_t_max){
	  ++multiplicity_ctot;
          for(Int_t m = 0; m<nhit_l; ++m){
            tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
            tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
            tot = tdc - tdc_t;
            hptr_array[bc4tot_id+l]->Fill(tot);
            if(tot < tot_min) continue;
	    hptr_array[bc4t_ctot_id + l]->Fill(tdc);
	    hptr_array[bc4tot_ctot_id+l]->Fill(tot);
	    if( tdc1st<tdc ) tdc1st = tdc;
	    if( tdc_min < tdc && tdc < tdc_max ){
	      flag_hit_wt_ctot = true;
	    }
          }
        }

	if( tdc1st!=0 ) hptr_array[bc4t1st_ctot_id +l]->Fill( tdc1st );
	if( flag_hit_wt_ctot ){
	  ++multiplicity_wt_ctot;
	  hptr_array[bc4hit_ctot_id + l]->Fill( w );
          BC4HitCont[l].push_back(w);
        }
      }

      hptr_array[bc4mul_id + l]->Fill(multiplicity);
      hptr_array[bc4mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[bc4mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[bc4mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }


    for(Int_t s=0; s<NumOfDimBC4 ;s++){
      Int_t corr=2*s;
      for(UInt_t i=0; i<BC4HitCont[corr].size() ;i++){
        for(UInt_t j=0; j<BC4HitCont[corr+1].size() ;j++){
          hptr_array[bc4self_corr_id + s]->Fill(BC4HitCont[corr][i],BC4HitCont[corr+1][j]);
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

  std::vector<Int_t> hitseg_bh2;
  { ///// BH2
    static const auto device_id = gUnpacker.get_device_id("BH2");
    static const auto adc_id = gUnpacker.get_data_id("BH2", "adc");
    static const auto tdc_id = gUnpacker.get_data_id("BH2", "fpga_leading");
    static const auto tdc_min = gUser.GetParameter("BH2_TDC", 0);
    static const auto tdc_max = gUser.GetParameter("BH2_TDC", 1);
    static const auto adc_hid = gHist.getSequentialID(kBH2, 0, kADC);
    static const auto tdc_hid = gHist.getSequentialID(kBH2, 0, kTDC);
    static const auto awt_hid = gHist.getSequentialID(kBH2, 0, kADCwTDC);
    static const auto hit_hid = gHist.getSequentialID(kBH2, 0, kHitPat);
    static const auto mul_hid = gHist.getSequentialID(kBH2, 0, kMulti);
    std::vector<std::vector<Int_t>> hit_flag(NumOfSegBH2);
    UInt_t multiplicity = 0;
    for(Int_t seg=0; seg<NumOfSegBH2; ++seg){
      hit_flag[seg].resize(kUorD);
      for(Int_t ud=0; ud<2; ++ud){
	auto nhit = gUnpacker.get_entries(device_id, 0, seg, ud, adc_id);
	UInt_t adc = 0;
	if(nhit != 0){
	  adc = gUnpacker.get(device_id, 0, seg, ud, adc_id);
	  hptr_array[adc_hid + seg + ud*NumOfSegBH2]->Fill(adc);
	}
	// TDC
	for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, seg, ud, tdc_id);
	    m<n; ++m){
	  auto tdc = gUnpacker.get(device_id, 0, seg, ud, tdc_id, m);
	  if(tdc != 0){
	    hptr_array[tdc_hid + seg + ud*NumOfSegBH2]->Fill(tdc);
	    if(tdc_min < tdc && tdc < tdc_max && adc > 0){
	      hit_flag[seg][ud] = 1;
	    }
	  }
	}
	if(hit_flag[seg][ud] == 1){
	  hptr_array[awt_hid + seg + ud*NumOfSegBH2]->Fill(adc);
	}
      }
      if(hit_flag[seg][kU] == 1 && hit_flag[seg][kD] == 1){
	  ++multiplicity;
	  hptr_array[hit_hid]->Fill(seg);
	  hitseg_bh2.push_back(seg);
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

  { ///// HTOF
    static const auto device_id = gUnpacker.get_device_id("HTOF");
    static const auto adc_id = gUnpacker.get_data_id("HTOF", "adc");
    static const auto tdc_id = gUnpacker.get_data_id("HTOF", "fpga_leading");
    static const auto tdc_min = gUser.GetParameter("HTOF_TDC", 0);
    static const auto tdc_max = gUser.GetParameter("HTOF_TDC", 1);
    static const auto htofa_id = gHist.getSequentialID(kHTOF, 0, kADC);
    static const auto htoft_id = gHist.getSequentialID(kHTOF, 0, kTDC);
    static const auto htofawt_id = gHist.getSequentialID(kHTOF, 0, kADCwTDC);
    static const auto htofhit_id = gHist.getSequentialID(kHTOF, 0, kHitPat);
    static const auto htofmul_id = gHist.getSequentialID(kHTOF, 0, kMulti);

    hptr_array[htofhit_id+1]->Reset();

    std::vector<std::vector<Int_t>> hit_flag(NumOfSegHTOF);
    UInt_t multiplicity = 0;
    for(Int_t seg=0; seg<NumOfSegHTOF; ++seg){
      hit_flag[seg].resize(kUorD);
      for(Int_t ud=0; ud<kUorD; ++ud){
	hit_flag[seg][ud] = 0;
	///// ADC
	UInt_t adc = 0;
	auto nhit = gUnpacker.get_entries(device_id, 0, seg, ud, adc_id);
	if(nhit != 0){
	  adc = gUnpacker.get(device_id, 0, seg, ud, adc_id);
	  hptr_array[htofa_id + seg + ud*NumOfSegHTOF]->Fill(adc);
	}
	///// TDC
	for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, seg, ud, tdc_id);
	    m<n; ++m){
	  auto tdc = gUnpacker.get(device_id, 0, seg, ud, tdc_id, m);
	  if(tdc != 0){
	    hptr_array[htoft_id + seg + ud*NumOfSegHTOF]->Fill(tdc);
	    // ADC w/TDC
	    if(tdc_min < tdc && tdc < tdc_max && adc > 0){
	      hit_flag[seg][ud] = 1;
	    }
	  }
	}
	if(hit_flag[seg][ud] == 1){
	  hptr_array[htofawt_id + seg + ud*NumOfSegHTOF]->Fill(adc);
	}
      }//ud
      if(hit_flag[seg][kU] == 1 && hit_flag[seg][kD] == 1){
	hptr_array[htofhit_id]->Fill(seg);
	Int_t binid=0;
	if( seg==0 ) binid=1;
	else if( seg==1 || seg==2 ) binid=2;
	else if( seg==3 || seg==4 ) binid=3;
	else binid=seg-1;
	hptr_array[htofhit_id+1]->SetBinContent( binid, 1 );
	++multiplicity;
      }
    }//seg
    hptr_array[htofmul_id]->Fill(multiplicity);
#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }
#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  std::vector<Int_t> hitseg_sch;
  { ///// SCH
    static const auto device_id = gUnpacker.get_device_id("SCH");
    static const auto leading_id = gUnpacker.get_data_id("SCH", "leading");
    static const auto trailing_id = gUnpacker.get_data_id("SCH", "trailing");
    static const auto tdc_min = gUser.GetParameter("SCH_TDC", 0);
    static const auto tdc_max = gUser.GetParameter("SCH_TDC", 1);
    static const auto tdc_hid = gHist.getSequentialID(kSCH, 0, kTDC, 1);
    static const auto tot_hid = gHist.getSequentialID(kSCH, 0, kADC, 1);
    static const auto tdcall_hid = gHist.getSequentialID(kSCH, 0, kTDC, NumOfSegSCH+1);
    static const auto totall_hid = gHist.getSequentialID(kSCH, 0, kADC, NumOfSegSCH+1);
    static const auto hit_hid = gHist.getSequentialID(kSCH, 0, kHitPat, 1);
    static const auto mul_hid = gHist.getSequentialID(kSCH, 0, kMulti, 1);
    static const auto tdc2d_hid = gHist.getSequentialID(kSCH, 0, kTDC2D, 1);
    static const auto tot2d_hid = gHist.getSequentialID(kSCH, 0, kADC2D, 1);
    UInt_t multiplicity = 0;
    for(Int_t i=0; i<NumOfSegSCH; ++i){
      UInt_t tdc_prev = 0;
      Bool_t is_in_gate = false;
      for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, i, 0, leading_id);
	  m<n; ++m){
	auto tdc = gUnpacker.get(device_id, 0, i, 0, leading_id, m);
	auto trailing = gUnpacker.get(device_id, 0, i, 0, trailing_id, m);
	auto tot = tdc - trailing;
	if(tdc_prev == tdc || tdc <= 0 || tot <= 0)
	  continue;
	tdc_prev = tdc;
	hptr_array[tdc_hid +i]->Fill(tdc);
	hptr_array[tdcall_hid]->Fill(tdc);
	hptr_array[tot_hid +i]->Fill(tot);
	hptr_array[totall_hid]->Fill(tot);
	hptr_array[tdc2d_hid]->Fill(i, tdc);
	hptr_array[tot2d_hid]->Fill(i, tot);
	if(tdc_min<tdc && tdc<tdc_max){
	  is_in_gate = true;
	}
      }
      if(is_in_gate){
	++multiplicity;
	hptr_array[hit_hid]->Fill(i);
	hitseg_sch.push_back(i);
      }
    }
    hptr_array[mul_hid]->Fill(multiplicity);

#if 0
    gUnpacker.dump_data_device(device_id);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // SDC1 ------------------------------------------------------------
  std::vector< std::vector<Int_t> > SDC1HitCont(6);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("SDC1");
    static const Int_t k_leading  = gUnpacker.get_data_id("SDC1", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("SDC1", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("SDC1_TDC", 0);
    static const Int_t tdc_max = gUser.GetParameter("SDC1_TDC", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("SDC1_TOT", 0);
    // static const Int_t tot_max = gUser.GetParameter("SDC1_TOT", 1);

    // sequential id
    static const Int_t sdc1t_id    = gHist.getSequentialID(kSDC1, 0, kTDC);
    static const Int_t sdc1tot_id  = gHist.getSequentialID(kSDC1, 0, kADC);
    static const Int_t sdc1t1st_id = gHist.getSequentialID(kSDC1, 0, kTDC2D);
    static const Int_t sdc1hit_id  = gHist.getSequentialID(kSDC1, 0, kHitPat);
    static const Int_t sdc1mul_id  = gHist.getSequentialID(kSDC1, 0, kMulti);
    static const Int_t sdc1mulwt_id
      = gHist.getSequentialID(kSDC1, 0, kMulti, 1+NumOfLayersSDC1);

    static const Int_t sdc1t_ctot_id    = gHist.getSequentialID(kSDC1, 0, kTDC,    1+kTOTcutOffset);
    static const Int_t sdc1tot_ctot_id  = gHist.getSequentialID(kSDC1, 0, kADC,    1+kTOTcutOffset);
    static const Int_t sdc1t1st_ctot_id = gHist.getSequentialID(kSDC1, 0, kTDC2D,  1+kTOTcutOffset);
    static const Int_t sdc1hit_ctot_id  = gHist.getSequentialID(kSDC1, 0, kHitPat, 1+kTOTcutOffset);
    static const Int_t sdc1mul_ctot_id  = gHist.getSequentialID(kSDC1, 0, kMulti,  1+kTOTcutOffset);
    static const Int_t sdc1mulwt_ctot_id
      = gHist.getSequentialID(kSDC1, 0, kMulti, 1+NumOfLayersSDC1 + kTOTcutOffset);
    static const Int_t sdc1self_corr_id  = gHist.getSequentialID(kSDC1, kSelfCorr, 0, 1);

    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersSDC1; ++l){
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      for( Int_t w=0; w<NumOfWireSDC1; ++w ){
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if( nhit_l == 0 ) continue;

        Int_t hit_l_max = 0;
        Int_t hit_t_max = 0;

        if(nhit_l != 0){
          hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
        }
        if(nhit_t != 0){
          hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
        }

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[sdc1hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for( Int_t m = 0; m<nhit_l; ++m ){
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[sdc1t_id + l]->Fill(tdc);
	  if( tdc1st<tdc ) tdc1st = tdc;
	  // Drift time check
	  if( tdc_min < tdc && tdc < tdc_max ){
	    flag_hit_wt = true;
	  }
	}

	if( tdc1st!=0 ) hptr_array[sdc1t1st_id +l]->Fill( tdc1st );
	if( flag_hit_wt ){
	  ++multiplicity_wt;
	  hptr_array[sdc1hit_id + l]->Fill( w );
	}

	tdc1st = 0;
        if(nhit_l == nhit_t && hit_l_max > hit_t_max){
	  ++multiplicity_ctot;
          for(Int_t m = 0; m<nhit_l; ++m){
            tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
            tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
            tot = tdc - tdc_t;
            hptr_array[sdc1tot_id+l]->Fill(tot);
            if(tot < tot_min) continue;
	    hptr_array[sdc1t_ctot_id + l]->Fill(tdc);
	    hptr_array[sdc1tot_ctot_id+l]->Fill(tot);
	    if( tdc1st<tdc ) tdc1st = tdc;
	    if( tdc_min < tdc && tdc < tdc_max ){
	      flag_hit_wt_ctot = true;
	    }
          }
        }

	if( tdc1st!=0 ) hptr_array[sdc1t1st_ctot_id +l]->Fill( tdc1st );
	if( flag_hit_wt_ctot ){
	  ++multiplicity_wt_ctot;
	  hptr_array[sdc1hit_ctot_id + l]->Fill( w );
          SDC1HitCont[l].push_back(w);
        }
      }

      hptr_array[sdc1mul_id + l]->Fill(multiplicity);
      hptr_array[sdc1mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[sdc1mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[sdc1mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }


    for(Int_t s=0; s<NumOfDimSDC1 ;s++){
      Int_t corr=2*s;
      for(UInt_t i=0; i<SDC1HitCont[corr].size() ;i++){
        for(UInt_t j=0; j<SDC1HitCont[corr+1].size() ;j++){
          hptr_array[sdc1self_corr_id + s]->Fill(SDC1HitCont[corr][i],SDC1HitCont[corr+1][j]);
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

  // SDC2 ------------------------------------------------------------
  std::vector< std::vector<Int_t> > SDC2HitCont(4);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("SDC2");
    static const Int_t k_leading  = gUnpacker.get_data_id("SDC2", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("SDC2", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("SDC2_TDC", 0);
    static const Int_t tdc_max = gUser.GetParameter("SDC2_TDC", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("SDC2_TOT", 0);
    // static const Int_t tot_max = gUser.GetParameter("SDC2_TOT", 1);

    // sequential id
    static const Int_t sdc2t_id    = gHist.getSequentialID(kSDC2, 0, kTDC);
    static const Int_t sdc2tot_id  = gHist.getSequentialID(kSDC2, 0, kADC);
    static const Int_t sdc2t1st_id = gHist.getSequentialID(kSDC2, 0, kTDC2D);
    static const Int_t sdc2hit_id  = gHist.getSequentialID(kSDC2, 0, kHitPat);
    static const Int_t sdc2mul_id  = gHist.getSequentialID(kSDC2, 0, kMulti);
    static const Int_t sdc2mulwt_id
      = gHist.getSequentialID(kSDC2, 0, kMulti, 1+NumOfLayersSDC2);

    static const Int_t sdc2t_ctot_id    = gHist.getSequentialID(kSDC2, 0, kTDC,    1+kTOTcutOffset);
    static const Int_t sdc2tot_ctot_id  = gHist.getSequentialID(kSDC2, 0, kADC,    1+kTOTcutOffset);
    static const Int_t sdc2t1st_ctot_id = gHist.getSequentialID(kSDC2, 0, kTDC2D,  1+kTOTcutOffset);
    static const Int_t sdc2hit_ctot_id  = gHist.getSequentialID(kSDC2, 0, kHitPat, 1+kTOTcutOffset);
    static const Int_t sdc2mul_ctot_id  = gHist.getSequentialID(kSDC2, 0, kMulti,  1+kTOTcutOffset);
    static const Int_t sdc2mulwt_ctot_id
      = gHist.getSequentialID(kSDC2, 0, kMulti, 1+NumOfLayersSDC2 + kTOTcutOffset);
    static const Int_t sdc2self_corr_id  = gHist.getSequentialID(kSDC2, kSelfCorr, 0, 1);

    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersSDC2; ++l){
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      Int_t sdc2_nwire = 0;
      if( l==0 || l==1 )
	sdc2_nwire = NumOfWireSDC2X;
      if( l==2 || l==3 )
	sdc2_nwire = NumOfWireSDC2Y;

      for( Int_t w=0; w<sdc2_nwire; ++w ){
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if( nhit_l == 0 ) continue;
        Int_t hit_l_max = 0;
        Int_t hit_t_max = 0;
        if(nhit_l != 0){
          hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
        }
        if(nhit_t != 0){
          hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
        }
	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[sdc2hit_id + l]->Fill(w, nhit);
	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for( Int_t m = 0; m<nhit_l; ++m ){
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[sdc2t_id + l]->Fill(tdc);
	  if( tdc1st<tdc ) tdc1st = tdc;
	  // Drift time check
	  if( tdc_min < tdc && tdc < tdc_max ){
	    flag_hit_wt = true;
	  }
	}

	if( tdc1st!=0 ) hptr_array[sdc2t1st_id +l]->Fill( tdc1st );
	if( flag_hit_wt ){
	  ++multiplicity_wt;
	  hptr_array[sdc2hit_id + l]->Fill( w );
	}

	tdc1st = 0;
        if(nhit_l == nhit_t && hit_l_max > hit_t_max){
	  ++multiplicity_ctot;
          for(Int_t m = 0; m<nhit_l; ++m){
            tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
            tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
            tot = tdc - tdc_t;
            hptr_array[sdc2tot_id+l]->Fill(tot);
            if(tot < tot_min) continue;
	    hptr_array[sdc2t_ctot_id + l]->Fill(tdc);
	    hptr_array[sdc2tot_ctot_id+l]->Fill(tot);
	    if( tdc1st<tdc ) tdc1st = tdc;
	    if( tdc_min < tdc && tdc < tdc_max ){
	      flag_hit_wt_ctot = true;
	    }
          }
        }

	if( tdc1st!=0 ) hptr_array[sdc2t1st_ctot_id +l]->Fill( tdc1st );
	if( flag_hit_wt_ctot ){
	  ++multiplicity_wt_ctot;
	  hptr_array[sdc2hit_ctot_id + l]->Fill( w );
          SDC2HitCont[l].push_back(w);
        }
      }
      hptr_array[sdc2mul_id + l]->Fill(multiplicity);
      hptr_array[sdc2mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[sdc2mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[sdc2mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }

    for(Int_t s=0; s<NumOfDimSDC2 ;s++){
      Int_t corr=2*s;
      for(UInt_t i=0; i<SDC2HitCont[corr].size() ;i++){
        for(UInt_t j=0; j<SDC2HitCont[corr+1].size() ;j++){
          hptr_array[sdc2self_corr_id + s]->Fill(SDC2HitCont[corr][i],SDC2HitCont[corr+1][j]);
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
  // SDC3
  //------------------------------------------------------------------
  std::vector< std::vector<Int_t> > SDC3HitCont(4);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("SDC3");
    static const Int_t k_leading  = gUnpacker.get_data_id("SDC3", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("SDC3", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("SDC3_TDC", 0);
    static const Int_t tdc_max = gUser.GetParameter("SDC3_TDC", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("SDC3_TOT", 0);
    // static const Int_t tot_max = gUser.GetParameter("SDC3_TOT", 1);

    // sequential id
    static const Int_t sdc3t_id    = gHist.getSequentialID(kSDC3, 0, kTDC);
    static const Int_t sdc3tot_id  = gHist.getSequentialID(kSDC3, 0, kADC);
    static const Int_t sdc3t1st_id = gHist.getSequentialID(kSDC3, 0, kTDC2D);
    static const Int_t sdc3hit_id  = gHist.getSequentialID(kSDC3, 0, kHitPat);
    static const Int_t sdc3mul_id  = gHist.getSequentialID(kSDC3, 0, kMulti);
    static const Int_t sdc3mulwt_id
      = gHist.getSequentialID(kSDC3, 0, kMulti, 1+NumOfLayersSDC3);
    static const Int_t sdc3t_ctot_id    = gHist.getSequentialID(kSDC3, 0, kTDC,    11);
    static const Int_t sdc3tot_ctot_id  = gHist.getSequentialID(kSDC3, 0, kADC,    11);
    static const Int_t sdc3t1st_ctot_id = gHist.getSequentialID(kSDC3, 0, kTDC2D,  11);
    static const Int_t sdc3hit_ctot_id  = gHist.getSequentialID(kSDC3, 0, kHitPat, 11);
    static const Int_t sdc3mul_ctot_id  = gHist.getSequentialID(kSDC3, 0, kMulti,  11);
    static const Int_t sdc3mulwt_ctot_id
      = gHist.getSequentialID(kSDC3, 0, kMulti, 1+NumOfLayersSDC3 + 10);
    static const Int_t sdc3self_corr_id  = gHist.getSequentialID(kSDC3, kSelfCorr, 0, 1);
    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersSDC3; ++l){
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      for( Int_t w=0; w<NumOfWireSDC3; ++w ){
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if( nhit_l == 0 ) continue;

        Int_t hit_l_max = 0;
        Int_t hit_t_max = 0;

        if(nhit_l != 0){
          hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
        }
        if(nhit_t != 0){
          hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
        }

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[sdc3hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for( Int_t m = 0; m<nhit_l; ++m ){
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[sdc3t_id + l]->Fill(tdc);
	  if( tdc1st<tdc ) tdc1st = tdc;

	  // Drift time check
	  if( tdc_min < tdc && tdc < tdc_max ){
	    flag_hit_wt = true;
	  }
	}

	if( tdc1st!=0 ) hptr_array[sdc3t1st_id +l]->Fill( tdc1st );
	if( flag_hit_wt ){
	  ++multiplicity_wt;
	  hptr_array[sdc3hit_id + l]->Fill( w );
	}

	tdc1st = 0;
        if(nhit_l == nhit_t && hit_l_max > hit_t_max){
	  ++multiplicity_ctot;
          for(Int_t m = 0; m<nhit_l; ++m){
            tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
            tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
            tot = tdc - tdc_t;
            hptr_array[sdc3tot_id+l]->Fill(tot);
            if(tot < tot_min) continue;
	    hptr_array[sdc3t_ctot_id + l]->Fill(tdc);
	    hptr_array[sdc3tot_ctot_id+l]->Fill(tot);
	    if( tdc1st<tdc ) tdc1st = tdc;
	    if( tdc_min < tdc && tdc < tdc_max ){
	      flag_hit_wt_ctot = true;
	    }
          }
        }

	if( tdc1st!=0 ) hptr_array[sdc3t1st_ctot_id +l]->Fill( tdc1st );
	if( flag_hit_wt_ctot ){
	  ++multiplicity_wt_ctot;
	  hptr_array[sdc3hit_ctot_id + l]->Fill( w );
          SDC3HitCont[l].push_back(w);
        }
      }

      hptr_array[sdc3mul_id + l]->Fill(multiplicity);
      hptr_array[sdc3mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[sdc3mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[sdc3mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }


    for(Int_t s=0; s<NumOfDimSDC3 ;s++){
      Int_t corr=2*s;
      for(UInt_t i=0; i<SDC3HitCont[corr].size() ;i++){
        for(UInt_t j=0; j<SDC3HitCont[corr+1].size() ;j++){
          hptr_array[sdc3self_corr_id + s]->Fill(SDC3HitCont[corr][i],SDC3HitCont[corr+1][j]);
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
  // SDC4
  //------------------------------------------------------------------
  std::vector< std::vector<Int_t> > SDC4HitCont(4);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("SDC4");
    static const Int_t k_leading  = gUnpacker.get_data_id("SDC4", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("SDC4", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("SDC4_TDC", 0);
    static const Int_t tdc_max = gUser.GetParameter("SDC4_TDC", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("SDC4_TOT", 0);
    static const Int_t tot_max = gUser.GetParameter("SDC4_TOT", 1);


    // sequential id
    static const Int_t sdc4t_id    = gHist.getSequentialID(kSDC4, 0, kTDC);
    static const Int_t sdc4tot_id  = gHist.getSequentialID(kSDC4, 0, kADC);
    static const Int_t sdc4t1st_id = gHist.getSequentialID(kSDC4, 0, kTDC2D);
    static const Int_t sdc4hit_id  = gHist.getSequentialID(kSDC4, 0, kHitPat);
    static const Int_t sdc4mul_id  = gHist.getSequentialID(kSDC4, 0, kMulti);
    static const Int_t sdc4mulwt_id
      = gHist.getSequentialID(kSDC4, 0, kMulti, 1+NumOfLayersSDC4);

    static const Int_t sdc4t_ctot_id    = gHist.getSequentialID(kSDC4, 0, kTDC,    11);
    static const Int_t sdc4tot_ctot_id  = gHist.getSequentialID(kSDC4, 0, kADC,    11);
    static const Int_t sdc4t1st_ctot_id = gHist.getSequentialID(kSDC4, 0, kTDC2D,  11);
    static const Int_t sdc4hit_ctot_id  = gHist.getSequentialID(kSDC4, 0, kHitPat, 11);
    static const Int_t sdc4mul_ctot_id  = gHist.getSequentialID(kSDC4, 0, kMulti,  11);
    static const Int_t sdc4mulwt_ctot_id
      = gHist.getSequentialID(kSDC4, 0, kMulti, 1+NumOfLayersSDC4 + 10);
    static const Int_t sdc4self_corr_id  = gHist.getSequentialID(kSDC4, kSelfCorr, 0, 1);

    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersSDC4; ++l){
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      Int_t sdc4_nwire = 0;
      if( l==0 || l==1 )
	sdc4_nwire = NumOfWireSDC4Y;
      if( l==2 || l==3 )
	sdc4_nwire = NumOfWireSDC4X;

      for( Int_t w=0; w<sdc4_nwire; ++w ){
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if( nhit_l == 0 ) continue;

        Int_t hit_l_max = 0;
        Int_t hit_t_max = 0;

        if(nhit_l != 0){
          hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
        }
        if(nhit_t != 0){
          hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
        }

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[sdc4hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for( Int_t m = 0; m<nhit_l; ++m ){
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[sdc4t_id + l]->Fill(tdc);
	  if( tdc1st<tdc ) tdc1st = tdc;

	  // Drift time check
	  if( tdc_min < tdc && tdc < tdc_max ){
	    flag_hit_wt = true;
	  }
	}

	if( tdc1st!=0 ) hptr_array[sdc4t1st_id +l]->Fill( tdc1st );
	if( flag_hit_wt ){
	  ++multiplicity_wt;
	  hptr_array[sdc4hit_id + l]->Fill( w );
	}

	tdc1st = 0;
        if(nhit_l == nhit_t && hit_l_max > hit_t_max){
	  ++multiplicity_ctot;
          for(Int_t m = 0; m<nhit_l; ++m){
            tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
            tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
            tot = tdc - tdc_t;
            hptr_array[sdc4tot_id+l]->Fill(tot);
            if(tot < tot_min || tot >tot_max) continue;
	    hptr_array[sdc4t_ctot_id + l]->Fill(tdc);
	    hptr_array[sdc4tot_ctot_id+l]->Fill(tot);
	    if( tdc1st<tdc ) tdc1st = tdc;
	    if( tdc_min < tdc && tdc < tdc_max ){
	      flag_hit_wt_ctot = true;
	    }
          }
        }

	if( tdc1st!=0 ) hptr_array[sdc4t1st_ctot_id +l]->Fill( tdc1st );
	if( flag_hit_wt_ctot ){
	  ++multiplicity_wt_ctot;
	  hptr_array[sdc4hit_ctot_id + l]->Fill( w );
          SDC4HitCont[l].push_back(w);
        }
      }

      hptr_array[sdc4mul_id + l]->Fill(multiplicity);
      hptr_array[sdc4mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[sdc4mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[sdc4mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }

    for(Int_t s=0; s<NumOfDimSDC4 ;s++){
      Int_t corr=2*s;
      for(UInt_t i=0; i<SDC4HitCont[corr].size() ;i++){
        for(UInt_t j=0; j<SDC4HitCont[corr+1].size() ;j++){
          hptr_array[sdc4self_corr_id + s]->Fill(SDC4HitCont[corr][i],SDC4HitCont[corr+1][j]);
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

  std::vector<Int_t> hitseg_bvh;
  { ///// BVH
    static const auto k_device = gUnpacker.get_device_id("BVH");
    static const auto k_tdc    = gUnpacker.get_data_id("BVH", "tdc");
    static const auto tdc_hid = gHist.getSequentialID(kBVH, 0, kTDC);
    static const auto hit_hid = gHist.getSequentialID(kBVH, 0, kHitPat);
    static const auto mul_hid = gHist.getSequentialID(kBVH, 0, kMulti);
    static const auto tdc_min = gUser.GetParameter("BVH_TDC", 0);
    static const auto tdc_max = gUser.GetParameter("BVH_TDC", 1);
    Int_t multiplicity = 0;
    for(Int_t seg=0; seg<NumOfSegBVH; ++seg){
      Bool_t is_in_gate = false;
      for(Int_t m=0, n=gUnpacker.get_entries(k_device, 0, seg, kU, k_tdc);
	  m<n; ++m){
	auto tdc = gUnpacker.get(k_device, 0, seg, kU, k_tdc, m);
	hptr_array[tdc_hid + seg]->Fill(tdc);
	if(tdc_min < tdc && tdc < tdc_max) is_in_gate = true;
      }
      if(is_in_gate){
	hptr_array[hit_hid]->Fill(seg);
	++multiplicity;
	hitseg_bvh.push_back(seg);
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

  std::vector<Int_t> hitseg_tof;
  { ///// TOF
    static const auto device_id = gUnpacker.get_device_id("TOF");
    static const auto adc_id = gUnpacker.get_data_id("TOF", "adc");
    static const auto tdc_id = gUnpacker.get_data_id("TOF", "fpga_leading");
    static const auto tdc_min = gUser.GetParameter("TOF_TDC", 0);
    static const auto tdc_max = gUser.GetParameter("TOF_TDC", 1);
    static const auto adc_hid = gHist.getSequentialID(kTOF, 0, kADC);
    static const auto tdc_hid = gHist.getSequentialID(kTOF, 0, kTDC);
    static const auto awt_hid = gHist.getSequentialID(kTOF, 0, kADCwTDC);
    static const auto hit_hid = gHist.getSequentialID(kTOF, 0, kHitPat);
    static const auto mul_hid = gHist.getSequentialID(kTOF, 0, kMulti);
    std::vector<std::vector<Int_t>> hit_flag(NumOfSegTOF);
    Int_t multiplicity = 0;
    for(Int_t seg=0; seg<NumOfSegTOF; ++seg){
      hit_flag[seg].resize(kUorD);
      for(Int_t ud=0; ud<kUorD; ++ud){
	hit_flag[seg][ud] = 0;
	// ADC
	UInt_t adc = 0;
	auto nhit = gUnpacker.get_entries(device_id, 0, seg, ud, adc_id);
	if(nhit != 0){
	  adc = gUnpacker.get(device_id, 0, seg, ud, adc_id);
	  hptr_array[adc_hid + ud*NumOfSegTOF + seg]->Fill(adc);
	}
	// TDC
	for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, seg, ud, tdc_id);
	    m<n; ++m){
	  auto tdc = gUnpacker.get(device_id, 0, seg, ud, tdc_id, m);
	  if(tdc != 0){
	    hptr_array[tdc_hid + ud*NumOfSegTOF + seg]->Fill(tdc);
	    if(tdc_min<tdc && tdc<tdc_max && adc > 0){
	      hit_flag[seg][ud] = 1;
	    }
	  }
	}
        if(hit_flag[seg][ud] == 1){
          hptr_array[awt_hid + ud*NumOfSegTOF + seg]->Fill(adc);
        }
      }
      if(hit_flag[seg][kU] == 1 && hit_flag[seg][kD] == 1){
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

  //------------------------------------------------------------------
  //LAC
  //------------------------------------------------------------------
  {
    // data typep
    static const Int_t k_device = gUnpacker.get_device_id("LAC");
    static const Int_t k_tdc    = gUnpacker.get_data_id("LAC","tdc");
    static const Int_t lachit_id = gHist.getSequentialID(kLAC, 0, kHitPat);
    static const Int_t lacmul_id = gHist.getSequentialID(kLAC, 0, kMulti);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("LAC_TDC", 0);
    static const Int_t tdc_max = gUser.GetParameter("LAC_TDC", 1);

    Int_t lact_id   = gHist.getSequentialID(kLAC, 0, kTDC);
    Int_t multiplicity = 0;
    for(Int_t seg = 0; seg<NumOfSegLAC; ++seg){
      Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, kU, k_tdc);
      Bool_t is_in_gate = false;
      for(Int_t m = 0; m<nhit; ++m){
	Int_t tdc = gUnpacker.get(k_device, 0, seg, kU, k_tdc, m);
	hptr_array[lact_id + seg]->Fill(tdc);

	if(tdc_min < tdc && tdc < tdc_max) is_in_gate = true;
      }

      // Hit pattern && multiplicity
      if(is_in_gate){
	hptr_array[lachit_id]->Fill(seg);
	++multiplicity;
      }
    }

    hptr_array[lacmul_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // WC -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("WC");
    static const Int_t k_sum    = 2; // sum
    static const Int_t k_adc    = gUnpacker.get_data_id("WC", "adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("WC", "tdc");
    // static const Int_t k_mt     = gUnpacker.get_data_id("WC", "fpga_meantime");

    // TDC gate range
    static const UInt_t tdc_min = gUser.GetParameter("WC_TDC", 0);
    static const UInt_t tdc_max = gUser.GetParameter("WC_TDC", 1);

    // UP
    Int_t wca_id   = gHist.getSequentialID(kWC, 0, kADC);
    Int_t wct_id   = gHist.getSequentialID(kWC, 0, kTDC);
    Int_t wcawt_id = gHist.getSequentialID(kWC, 0, kADCwTDC);    // UP

    for(Int_t seg=0; seg<NumOfSegWC; ++seg){
      // ADC
      Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, kU, k_adc);
      if(nhit != 0){
	UInt_t adc = gUnpacker.get(k_device, 0, seg, kU, k_adc);
	hptr_array[wca_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, kU, k_tdc);
      for(Int_t m = 0; m<nhit; ++m){
      	UInt_t tdc = gUnpacker.get(k_device, 0, seg, kU, k_tdc, m);
      	if(tdc!=0){
      	  hptr_array[wct_id + seg]->Fill(tdc);
      	  // ADC w/TDC
      	  if( tdc_min<tdc && tdc<tdc_max &&
              gUnpacker.get_entries(k_device, 0, seg, kU, k_adc)>0 ){
      	    UInt_t adc = gUnpacker.get(k_device, 0, seg, kU, k_adc);
      	    hptr_array[wcawt_id + seg]->Fill(adc);
      	  }
      	}
      }
    }

    // DOWN
    wca_id   = gHist.getSequentialID(kWC, 0, kADC,     NumOfSegWC+1);
    wct_id   = gHist.getSequentialID(kWC, 0, kTDC,     NumOfSegWC+1);
    wcawt_id = gHist.getSequentialID(kWC, 0, kADCwTDC, NumOfSegWC+1);    // Down

    for(Int_t seg=0; seg<NumOfSegWC; ++seg){
      // ADC
      Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, kD, k_adc);
      if(nhit != 0){
	UInt_t adc = gUnpacker.get(k_device, 0, seg, kD, k_adc);
	hptr_array[wca_id + seg]->Fill(adc);
      }
      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, kD, k_tdc);
      for(Int_t m = 0; m<nhit; ++m){
      	UInt_t tdc = gUnpacker.get(k_device, 0, seg, kD, k_tdc, m);
      	if( tdc!=0 ){
      	  hptr_array[wct_id + seg]->Fill(tdc);
      	  // ADC w/TDC
      	  if( tdc_min<tdc && tdc<tdc_max &&
              gUnpacker.get_entries(k_device, 0, seg, kD, k_adc)>0 ){
      	    UInt_t adc = gUnpacker.get(k_device, 0, seg, kD, k_adc);
      	    hptr_array[wcawt_id + seg]->Fill( adc );
      	  }
      	}
      }
    }

    // SUM
    wca_id   = gHist.getSequentialID(kWC, 0, kADC,     NumOfSegWC*2+1);
    wct_id   = gHist.getSequentialID(kWC, 0, kTDC,     NumOfSegWC*2+1);
    wcawt_id = gHist.getSequentialID(kWC, 0, kADCwTDC, NumOfSegWC*2+1);    // Sum
    Int_t multi = 0;
    for(Int_t seg=0; seg<NumOfSegWC; ++seg){
      // ADC
      Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, k_sum, k_adc);
      if(nhit != 0){
	UInt_t adc = gUnpacker.get(k_device, 0, seg, k_sum, k_adc);
	hptr_array[wca_id + seg]->Fill(adc);
      }
      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_sum, k_tdc);
      Bool_t is_in_gate = false;
      for(Int_t m = 0; m<nhit; ++m){
      	UInt_t tdc = gUnpacker.get(k_device, 0, seg, k_sum, k_tdc, m);
      	if( tdc!=0 ){
      	  hptr_array[wct_id + seg]->Fill(tdc);
      	  // ADC w/TDC
      	  if( tdc_min<tdc && tdc<tdc_max &&
              gUnpacker.get_entries(k_device, 0, seg, k_sum, k_adc)>0 ){
            is_in_gate = true;
      	    UInt_t adc = gUnpacker.get(k_device, 0, seg, k_sum, k_adc);
      	    hptr_array[wcawt_id + seg]->Fill( adc );
      	  }
      	}
      }
      if( is_in_gate ){
        hptr_array[gHist.getSequentialID( kWC, 0, kHitPat )]->Fill( seg );
        ++multi;
      }
    }
    hptr_array[gHist.getSequentialID( kWC, 0, kMulti )]->Fill( multi );

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

  // Correlation (2D histograms) -------------------------------------
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

    // sequential id
    Int_t cor_id= gHist.getSequentialID(kCorrelation, 0, 0, 1);
    Int_t mtx2d_id = gHist.getSequentialID(kCorrelation, 1, 0, 1);
    Int_t mtx3d_id = gHist.getSequentialID(kCorrelation, 2, 0, 1);

    // BH1 vs BFT
    TH2* hcor_bh1bft = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t seg1 = 0; seg1<NumOfSegBH1; ++seg1){
      for(const auto& seg2: hitseg_bftu){
	Int_t nhitBH1 = gUnpacker.get_entries(k_device_bh1, 0, seg1, 0, 1);
	if( nhitBH1 == 0 ) continue;
	Int_t tdcBH1 = gUnpacker.get(k_device_bh1, 0, seg1, 0, 1);
	Bool_t hitBH1 = ( tdcBH1 > 0 );
	if( hitBH1 ){
	  hcor_bh1bft->Fill(seg1, seg2);
	}
      }
    }

    // BH1 vs BH2
    TH2* hcor_bh1bh2 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t seg1 = 0; seg1<NumOfSegBH1; ++seg1){
      for(Int_t seg2 = 0; seg2<NumOfSegBH2; ++seg2){
	Int_t hitBH1 = gUnpacker.get_entries(k_device_bh1, 0, seg1, 0, 1);
	Int_t hitBH2 = gUnpacker.get_entries(k_device_bh2, 0, seg2, 0, 1);
	if(hitBH1 == 0 || hitBH2 == 0)continue;
	Int_t tdcBH1 = gUnpacker.get(k_device_bh1, 0, seg1, 0, 1);
	Int_t tdcBH2 = gUnpacker.get(k_device_bh2, 0, seg2, 0, 1);
	if(tdcBH1 != 0 && tdcBH2 != 0){
	  hcor_bh1bh2->Fill(seg1, seg2);
	}
      }
    }

    // TOF vs SCH
    auto hcor_tofsch = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    auto hmtx2d1 = dynamic_cast<TH2*>(hptr_array[mtx2d_id]);
    // auto hmtx2d2 = dynamic_cast<TH2*>(hptr_array[mtx2d_id+1]);
    for(const auto& seg_sch: hitseg_sch){
      // TOF
      for(const auto& seg_tof: hitseg_tof){
	hcor_tofsch->Fill(seg_sch, seg_tof);
	if(trigger_flag[trigger::kTrigAPS]){
	  hmtx2d1->Fill(seg_sch, seg_tof);
	  for(const auto& seg_bh2: hitseg_bh2){
	    auto hmtx3d = dynamic_cast<TH2*>(hptr_array[mtx3d_id+seg_bh2]);
	    hmtx3d->Fill(seg_sch, seg_tof);
	  }
	}
      }
      // BVH
      for(const auto& seg_bvh: hitseg_bvh){
	Int_t seg_tof = seg_bvh + NumOfSegTOF;
	hcor_tofsch->Fill(seg_sch, seg_tof);
	if(trigger_flag[trigger::kTrigAPS]){
	  hmtx2d1->Fill(seg_sch, seg_tof);
	  for(const auto& seg_bh2: hitseg_bh2){
	    auto hmtx3d = dynamic_cast<TH2*>(hptr_array[mtx3d_id+seg_bh2]);
	    hmtx3d->Fill(seg_sch, seg_tof);
	  }
	}
      }
    }

    // BC3 vs BC4
    TH2* hcor_bc3bc4 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t wire1 = 0; wire1<NumOfWireBC3; ++wire1){
      for(Int_t wire2 = 0; wire2<NumOfWireBC4; ++wire2){
	Int_t hitBC3 = gUnpacker.get_entries(k_device_bc3, 0, 0, wire1, 0);
	Int_t hitBC4 = gUnpacker.get_entries(k_device_bc4, 5, 0, wire2, 0);
	if(hitBC3 == 0 || hitBC4 == 0)continue;
	hcor_bc3bc4->Fill(wire1, wire2);
      }
    }

    // SDC3 vs SDC1
    TH2* hcor_sdc1sdc3 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t wire1 = 0; wire1<NumOfWireSDC1; ++wire1){
      for(Int_t wire3 = 0; wire3<NumOfWireSDC3; ++wire3){
	Int_t hitSDC1 = gUnpacker.get_entries(k_device_sdc1, 0, 0, wire1, 0);
	Int_t hitSDC3 = gUnpacker.get_entries(k_device_sdc3, 0, 0, wire3, 0);
	if( hitSDC1 == 0 || hitSDC3 == 0 ) continue;
	hcor_sdc1sdc3->Fill( wire1, wire3 );
      }
    }

    // TOF vs SDC4
    TH2* hcor_tofsdc4 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(const auto& seg_tof: hitseg_tof){
      for( Int_t wire=0; wire<NumOfWireSDC4X; ++wire ){
	Int_t hitSDC4 = gUnpacker.get_entries(k_device_sdc4, 2, 0, wire, 0);
	if( hitSDC4 == 0 ) continue;
	hcor_tofsdc4->Fill(wire, seg_tof);
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
    static const Int_t k_tdc    = gUnpacker.get_data_id("BH1", "fpga_leading");
    // HodoParam
    static const Int_t cid_bh1  = 1;
    static const Int_t cid_bh2  = 2;
    static const Int_t plid     = 0;
    // Sequential ID
    static const Int_t btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);
    static const auto& hodoMan = HodoParamMan::GetInstance();
    // TDC gate range
    static const UInt_t tdc_min_bh1 = gUser.GetParameter("BH1_TDC", 0);
    static const UInt_t tdc_max_bh1 = gUser.GetParameter("BH1_TDC", 1);
    static const UInt_t tdc_min_bh2 = gUser.GetParameter("BH2_TDC", 0);
    static const UInt_t tdc_max_bh2 = gUser.GetParameter("BH2_TDC", 1);
    // BH2
    Double_t t0  = 1e10;
    Double_t ofs = 0;
    for(Int_t seg=0; seg<NumOfSegBH2; ++seg){
      Int_t nhitu = gUnpacker.get_entries(k_d_bh2, 0, seg, kU, k_tdc);
      Int_t nhitd = gUnpacker.get_entries(k_d_bh2, 0, seg, kD, k_tdc);
      for(Int_t mu=0; mu<nhitu; ++mu){
	auto tdcu = gUnpacker.get(k_d_bh2, 0, seg, kU, k_tdc, mu);
	if(tdcu < tdc_min_bh2 || tdc_max_bh2 < tdcu) continue;
	for(Int_t md=0; md<nhitd; ++md){
	  auto tdcd = gUnpacker.get(k_d_bh2, 0, seg, kD, k_tdc, md);
	  if(tdcd < tdc_min_bh2 || tdc_max_bh2 < tdcd) continue;
          Double_t bh2ut, bh2dt;
          hodoMan.GetTime(cid_bh2, plid, seg, kU, tdcu, bh2ut);
          hodoMan.GetTime(cid_bh2, plid, seg, kD, tdcd, bh2dt);
	  Double_t bh2mt = (bh2ut + bh2dt)/2.;
          if(TMath::Abs(t0) > TMath::Abs(bh2mt)){
	    hodoMan.GetTime(cid_bh2, plid, seg, 2, 0, ofs);
            t0 = bh2ut;
          }
        }
      }
    }
    // BH1
    for(Int_t seg=0; seg<NumOfSegBH1; ++seg){
      Int_t nhitu = gUnpacker.get_entries(k_d_bh1, 0, seg, kU, k_tdc);
      Int_t nhitd = gUnpacker.get_entries(k_d_bh1, 0, seg, kD, k_tdc);
      for(Int_t mu=0; mu<nhitu; ++mu){
	auto tdcu = gUnpacker.get(k_d_bh1, 0, seg, kU, k_tdc, mu);
	if(tdcu < tdc_min_bh1 || tdc_max_bh1 < tdcu) continue;
	for(Int_t md=0; md<nhitd; ++md){
	  auto tdcd = gUnpacker.get(k_d_bh1, 0, seg, kD, k_tdc, md);
	  if(tdcd < tdc_min_bh1 || tdc_max_bh1 < tdcd) continue;
	  Double_t bh1tu, bh1td;
	  hodoMan.GetTime(cid_bh1, plid, seg, kU, tdcu, bh1tu);
	  hodoMan.GetTime(cid_bh1, plid, seg, kD, tdcd, bh1td);
	  Double_t mt = (bh1tu+bh1td)/2.;
	  Double_t btof = mt-(t0+ofs);
	  hptr_array[btof_id]->Fill(btof);
	}// if(tdc)
      }// if(nhit)
    }// for(seg)
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  //------------------------------------------------------------------
  // BH1-6_BH2-3
  //------------------------------------------------------------------
  {
    // Unpacker
    static const Int_t k_d_bh1  = gUnpacker.get_device_id("BH1");
    static const Int_t k_d_bh2  = gUnpacker.get_device_id("BH2");
    static const Int_t k_tdc    = gUnpacker.get_data_id("BH1", "fpga_leading");
    // Sequential ID
    static const Int_t btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);
    // BH2
    Int_t    multiplicity = 0;
    Double_t t0  = 1e10;
    Double_t ofs = 0;
    Int_t seg = 3;
    Int_t nhitu = gUnpacker.get_entries(k_d_bh2, 0, seg, kU, k_tdc);
    Int_t nhitd = gUnpacker.get_entries(k_d_bh2, 0, seg, kD, k_tdc);
    if( nhitu != 0 && nhitd != 0 ){
      Int_t tdcu = gUnpacker.get(k_d_bh2, 0, seg, kU, k_tdc);
      Int_t tdcd = gUnpacker.get(k_d_bh2, 0, seg, kD, k_tdc);
      if( tdcu != 0 && tdcd != 0 ){
        ++multiplicity;
        t0 = (Double_t)(tdcu+tdcd)/2.;
      }//if(tdc)
    }// if(nhit)
    if( multiplicity == 1 ){
      seg = 5;
      // BH1
      Int_t nhitu = gUnpacker.get_entries(k_d_bh1, 0, seg, kU, k_tdc);
      Int_t nhitd = gUnpacker.get_entries(k_d_bh1, 0, seg, kD, k_tdc);
      if(nhitu != 0 &&  nhitd != 0){
	Int_t tdcu = gUnpacker.get(k_d_bh1, 0, seg, kU, k_tdc);
	Int_t tdcd = gUnpacker.get(k_d_bh1, 0, seg, kD, k_tdc);
	if(tdcu != 0 && tdcd != 0){
	  Double_t mt = (Double_t)(tdcu+tdcd)/2.;
	  Double_t btof = mt-(t0+ofs);
	  hptr_array[btof_id +1]->Fill(btof);
	}// if(tdc)
      }// if(nhit)
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BAC -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("BAC");
    static const Int_t k_adc    = gUnpacker.get_data_id("BAC","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("BAC","tdc");
    static const Int_t baca_id   = gHist.getSequentialID(kBAC, 0, kADC);
    static const Int_t bact_id   = gHist.getSequentialID(kBAC, 0, kTDC);
    static const Int_t bacawt_id = gHist.getSequentialID(kBAC, 0, kADCwTDC);
    static const Int_t bach_id   = gHist.getSequentialID(kBAC, 0, kHitPat);
    static const Int_t bacm_id   = gHist.getSequentialID(kBAC, 0, kMulti);
    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("BAC_TDC", 0);
    static const Int_t tdc_max = gUser.GetParameter("BAC_TDC", 1);
    Int_t multiplicity = 0;
    for(Int_t seg = 0; seg<NumOfSegBAC; ++seg){
      // ADC
      Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      if( nhit_a!=0 ){
	Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[baca_id + seg]->Fill( adc );
      }
      // TDC
      Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      Bool_t is_in_gate = false;
      for(Int_t m = 0; m<nhit_t; ++m){
	Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
	hptr_array[bact_id + seg]->Fill( tdc );
	if(tdc_min < tdc && tdc < tdc_max){
	  is_in_gate = true;
	}// tdc range is ok
      }// for(m)
      if( is_in_gate ){
	// ADC w/TDC
	if( gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0 ){
	  Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	  hptr_array[bacawt_id + seg]->Fill( adc );
	}
	hptr_array[bach_id]->Fill(seg);
	++multiplicity;
      }// flag is OK
    }

    hptr_array[bacm_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//BAC

  { ///// TPC
    if(cobo_data_size > 0){
      static const Int_t k_device   = gUnpacker.get_device_id( "TPC" );
      static const Int_t k_adc      = gUnpacker.get_data_id( "TPC", "adc" );
      // static const Int_t k_tdc_high = gUnpacker.get_data_id( "TPC", "tdc_high" );
      // static const Int_t k_tdc_low  = gUnpacker.get_data_id( "TPC", "tdc_low" );
      // sequential id
      static const Int_t tpca_id    = gHist.getSequentialID( kTPC, 0, kADC );
      static const Int_t tpct_id    = gHist.getSequentialID( kTPC, 0, kTDC );
      static const Int_t rms_id     = gHist.getSequentialID( kTPC, 0, kPede );
      static const Int_t tpcfa_id   = gHist.getSequentialID( kTPC, 0, kFADC );
      static const Int_t tpca2d_id  = gHist.getSequentialID( kTPC, 0, kADC2D );
      static const Int_t tpcmul_id  = gHist.getSequentialID( kTPC, 0, kMulti );
      static const Int_t tpcbp_id   = gHist.getSequentialID( kTPC, 2, kTDC );
      static const Int_t tpccs_id   = gHist.getSequentialID( kTPC, 2, kMulti );
      static const Int_t tpczy_id   = gHist.getSequentialID( kTPC, 0, kTDC2D );
      static const Int_t tpcxy_id   = gHist.getSequentialID( kTPC, 1, kTDC2D );
      static const Int_t agetmul_id = gHist.getSequentialID( kTPC, 3, kMulti );
      static const Int_t amulmax_id = gHist.getSequentialID( kTPC, 4, kMulti );

//      const Int_t tpcbp_padid[34] = {2641, 2431, 2226, 2020, 1806,
//	      			     1589, 1384, 1160,  938,  740,
//				      566,  416,  290,  188,  110,
//				       56,    6,   41,   87,  153,
//				      243,  357,  495,  657,  843,
//				     1053, 1287, 1511, 1732, 1963,
//				     2193, 2413, 2634, 2858};

      // FADC
      hptr_array[tpca2d_id]->Reset();
      hptr_array[tpca2d_id+1]->Reset();
      hptr_array[tpca2d_id+2]->Reset();
      hptr_array[tpczy_id+2]->Reset();
      hptr_array[tpcxy_id+2]->Reset();
      hptr_array[agetmul_id]->Reset();

      Int_t n_active_pad = 0;
      std::vector<Double_t> max_fadc( NumOfTimeBucket );
      Int_t max_adc = -1;
      Int_t max_tb  = -1;
      Int_t max_pad = -1;
      Int_t cluster_size[42] = {0};
      for( Int_t layer=0; layer<NumOfLayersTPC; ++layer ){
	for( Int_t ch=0; ch<272; ++ch ){
	  Int_t pad = gTpcPad.GetPadId( layer, ch );
	  if( pad < 0 ) continue;
	  Int_t nhit = gUnpacker.get_entries( k_device, layer, 0, ch, k_adc );
	  // if( nhit == 0 ){
	  if( nhit != NumOfTimeBucket ){
	    // hptr_array[tpca2d_id]->SetBinContent( pad + 1, 0 );
	    // hptr_array[tpca2d_id+1]->SetBinContent( pad + 1, 0 );
	    // hptr_array[tpca2d_id+2]->SetBinContent( pad + 1, 0 );
	    continue;
	  }
	  std::vector<Double_t> fadc( nhit );
	  for( Int_t i=0; i<nhit; ++i ){
	    Int_t adc = gUnpacker.get( k_device, layer, 0, ch, k_adc, i );
	    fadc[i] = adc;
	    if( max_adc < adc && nhit == NumOfTimeBucket ){
	      max_adc = adc;
	      max_tb  = i;
	      max_pad = pad;
	    }
	  }
	  if( max_pad == pad ){
	    max_fadc = fadc;
	  }
	  Double_t mean = TMath::Mean( nhit, fadc.data() );
	  Double_t rms = TMath::RMS( nhit, fadc.data() );
	  Double_t max_adc = TMath::MaxElement( nhit, fadc.data() );
	  Int_t loc_max = TMath::LocMax( nhit, fadc.data() );
	  if( max_adc - mean <= 0 ) continue;
	  Int_t aget = gTpcPad.GetParam( layer, ch )->AGetId();
	  Int_t asad = gTpcPad.GetParam( layer, ch )->AsAdId();
	  hptr_array[agetmul_id]->Fill( asad*4+aget );
	  hptr_array[tpca_id]->Fill( max_adc - mean );
	  hptr_array[rms_id]->Fill( rms );
	  hptr_array[tpct_id]->Fill( loc_max );
	  hptr_array[tpca2d_id]->SetBinContent( pad + 1, max_adc - mean );
	  hptr_array[tpca2d_id+1]->SetBinContent( pad + 1, rms );
	  hptr_array[tpca2d_id+2]->SetBinContent( pad + 1, loc_max );
	  Double_t pad_z = gTpcPad.GetPoint( pad ).Z();
	  Double_t pad_x = gTpcPad.GetPoint( pad ).X();
	  //Double_t pad_y = (max_tb+6)*4.-312;
	  if( max_adc - mean > 0 ){
            if(max_adc-mean>300 && max_tb>35 && max_tb<155)
              hptr_array[tpca2d_id+3]->Fill( pad_z, pad_x );
	   // hptr_array[tpczy_id]->Fill( pad_z, pad_y );
	   // hptr_array[tpcxy_id]->Fill( pad_x, pad_y );
	    hptr_array[tpczy_id]->Fill( pad_z, max_tb );
	    hptr_array[tpcxy_id]->Fill( pad_x, max_tb );
	    hptr_array[tpczy_id+2]->Fill( pad_z, max_tb );
	    hptr_array[tpcxy_id+2]->Fill( pad_x, max_tb );
	    ++n_active_pad;
	    if( max_tb >= 60 && max_tb < 100
   		&& pad_z >= -153-40 && pad_z <= -153
		&& max_adc - mean > 300
		){
              hptr_array[tpcbp_id]->Fill( pad_x );
	    }

	    if( max_tb >= 70 && max_tb < 85 ){
	      if( layer < 10 && pad_z < -143 ){
		  cluster_size[-layer-1+10]++;
	      } else {
		cluster_size[layer+10]++;
	      }
	    }

	    //for( Int_t i=0; i<34; ++i ){
	    //  if( tpcbp_padid[i] == pad ) hptr_array[tpcbp_id]->Fill( i );
	    //}
	  }
	}
      }
	hptr_array[amulmax_id]->Fill( hptr_array[agetmul_id]->GetMaximum() );
	for( Int_t i=0; i<42; i++ ){
	  if(cluster_size[i])
	    hptr_array[tpccs_id]->Fill( i-10, cluster_size[i] );
	}
      if( max_adc > 0 ){
	for( Int_t i=0; i<NumOfTimeBucket; ++i ){
	  hptr_array[tpcfa_id]->Fill( i, max_fadc[i] );
	}
      }
      std::cout << "run# " << run_number << "  ev# " << event_number
		<< "  active pad = " << n_active_pad << std::endl;
      hptr_array[tpcmul_id]->Fill( n_active_pad );

      // TDC (Time Stamp)
      // UInt_t tdc_h = gUnpacker.get( k_device, 0, 0, 0, k_tdc_high );
      // UInt_t tdc_l = gUnpacker.get( k_device, 0, 0, 0, k_tdc_low );
      // std::cout << tdc_h << ", " << tdc_l << std::endl;

#if 0
      gUnpacker.dump_data_device(k_device);
#endif
    }
    { ///// TPC-CLOCK
      static const auto device_id = gUnpacker.get_device_id("HTOF");
      static const auto leading_id = gUnpacker.get_data_id("HTOF", "fpga_leading");
      static const auto tdc_hid = gHist.getSequentialID(kTPC, 1, kTDC);
      static const auto mul_hid = gHist.getSequentialID(kTPC, 1, kMulti);
      static const Int_t seg = 34;
      UInt_t multiplicity = 0;
      for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, seg, 0, leading_id);
          m<n; ++m){
        auto tdc = gUnpacker.get(device_id, 0, seg, 0, leading_id, m);
        if(tdc != 0){
          hptr_array[tdc_hid]->Fill(tdc);
          ++multiplicity;
	}
      }
      hptr_array[mul_hid]->Fill(multiplicity);
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // Update
  if( gUnpacker.get_counter()%100 == 0 ){
    auto prev_level = gErrorIgnoreLevel;
    gErrorIgnoreLevel = kError;
    http::UpdateBcOutEfficiency();
    http::UpdateSdcInOutEfficiency();
    // http::UpdateT0PeakFitting();
    http::UpdateTOTPeakFitting();
    gErrorIgnoreLevel = prev_level;
  }

  if(!gUnpacker.is_good()){
    std::cout << "[Warning] Tag is not good." << std::endl;
    static const TString host(gSystem->Getenv("HOSTNAME"));
    static auto prev_time = std::time(0);
    auto        curr_time = std::time(0);
    if(host.Contains("k18term4") &&
       event_number > 1 && curr_time - prev_time > 5){
      std::cout << "exec tagslip sound!" << std::endl;
      gSystem->Exec("ssh axis@eb0 \"aplay ~/sound/tagslip.wav\" &");
    }
    prev_time = curr_time;
  }

  gSystem->ProcessEvents();

  return 0;
}

}
