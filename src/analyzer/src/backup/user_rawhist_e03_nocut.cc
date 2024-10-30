// -*- C++ -*-

#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <TGFileBrowser.h>
#include <TH1.h>
#include <TH2.h>
#include <TMath.h>
#include <TStyle.h>

#include <DAQNode.hh>
#include <filesystem_util.hh>
#include <Unpacker.hh>
#include <UnpackerManager.hh>

#include "Controller.hh"
#include "user_analyzer.hh"

#include "ConfMan.hh"
#include "DCDriftParamMan.hh"
#include "DCGeomMan.hh"
#include "DCTdcCalibMan.hh"
#include "DetectorID.hh"
#include "GuiPs.hh"
#include "HistMaker.hh"
#include "HodoParamMan.hh"
#include "HodoPHCMan.hh"
#include "MacroBuilder.hh"
#include "MatrixParamMan.hh"
#include "MsTParamMan.hh"
#include "ProcInfo.hh"
#include "PsMaker.hh"
#include "SsdAnalyzer.hh"
#include "UserParamMan.hh"

#define DEBUG      0
#define FLAG_DAQ   1
#define TIME_STAMP 0

namespace
{
using hddaq::unpacker::GUnpacker;
using hddaq::unpacker::DAQNode;
const auto& gUnpacker = GUnpacker::get_instance();
      auto& gHist     = HistMaker::getInstance();
const auto& gMatrix   = MatrixParamMan::GetInstance();
const auto& gUser     = UserParamMan::GetInstance();
std::vector<TH1*> hptr_array;
Bool_t flag_event_cut = false;
Int_t event_cut_factor = 1; // for fast semi-online analysis
}

namespace analyzer
{

//____________________________________________________________________________
Int_t
process_begin( const std::vector<std::string>& argv )
{
  auto& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  gConfMan.InitializeParameter<HodoParamMan>("HDPRM");
  gConfMan.InitializeParameter<HodoPHCMan>("HDPHC");
  gConfMan.InitializeParameter<DCGeomMan>("DCGEO");
  gConfMan.InitializeParameter<DCTdcCalibMan>("DCTDC");
  gConfMan.InitializeParameter<DCDriftParamMan>("DCDRFT");
  gConfMan.InitializeParameter<MatrixParamMan>( "MATRIX2D1",
                                                "MATRIX2D2",
                                                "MATRIX3D" );
  gConfMan.InitializeParameter<UserParamMan>("USER");
  if( !gConfMan.IsGood() ) return -1;
  // unpacker and all the parameter managers are initialized at this stage

  if( argv.size()==4 ){
    Int_t factor = std::strtod( argv[3].c_str(), NULL );
    if( factor!=0 ) event_cut_factor = std::abs( factor );
    flag_event_cut = true;
    std::cout << "#D Event cut flag on : factor="
	      << event_cut_factor << std::endl;
  }

  // Make tabs
  hddaq::gui::Controller& gCon = hddaq::gui::Controller::getInstance();
  TGFileBrowser *tab_hist  = gCon.makeFileBrowser("Hist");
  TGFileBrowser *tab_macro = gCon.makeFileBrowser("Macro");
  TGFileBrowser *tab_misc   = gCon.makeFileBrowser("Misc");

  // Add macros to the Macro tab
  //tab_macro->Add(hoge());
  tab_macro->Add(macro::Get("clear_all_canvas"));
  tab_macro->Add(macro::Get("clear_canvas"));
  tab_macro->Add(macro::Get("split22"));
  tab_macro->Add(macro::Get("split32"));
  tab_macro->Add(macro::Get("split33"));
  tab_macro->Add(macro::Get("dispBH1"));
  tab_macro->Add(macro::Get("dispBFT"));
  tab_macro->Add(macro::Get("dispBC3"));
  tab_macro->Add(macro::Get("dispBC4"));
  tab_macro->Add(macro::Get("dispBH2"));
  tab_macro->Add(macro::Get("dispACs"));
  tab_macro->Add(macro::Get("dispSDC1"));
  tab_macro->Add(macro::Get("dispSDC2"));
  tab_macro->Add(macro::Get("dispSCH"));
  tab_macro->Add(macro::Get("dispSDC3"));
  tab_macro->Add(macro::Get("dispSDC4"));
  tab_macro->Add(macro::Get("dispTOF"));
  tab_macro->Add(macro::Get("dispLAC"));
  tab_macro->Add(macro::Get("dispWC"));
  tab_macro->Add(macro::Get("dispGeAdc"));
  tab_macro->Add(macro::Get("dispGeTdc"));
  tab_macro->Add(macro::Get("dispGe2dhist"));
  tab_macro->Add(macro::Get("dispGeAdc_60Co"));
  tab_macro->Add(macro::Get("dispGeAdc_LSO"));
  tab_macro->Add(macro::Get("dispBGO"));
  tab_macro->Add(macro::Get("dispMatrix"));
  tab_macro->Add(macro::Get("dispTriggerFlag"));
  tab_macro->Add(macro::Get("dispHitPat"));
  tab_macro->Add(macro::Get("dispCorrelation"));
  tab_macro->Add(macro::Get("effBcOut"));
  tab_macro->Add(macro::Get("effSdcInOut"));
  tab_macro->Add(macro::Get("effWC"));
  tab_macro->Add(macro::Get("dispBH2Fit"));
  tab_macro->Add(macro::Get("dispDAQ"));

  // Add histograms to the Hist tab
  tab_hist->Add(gHist.createBH1());
  tab_hist->Add(gHist.createBFT());
  tab_hist->Add(gHist.createBC3());
  tab_hist->Add(gHist.createBC4());
  tab_hist->Add(gHist.createBH2());
  tab_hist->Add(gHist.createBAC());
  tab_hist->Add(gHist.createPVAC());
  tab_hist->Add(gHist.createFAC());
  tab_hist->Add(gHist.createSDC1());
  tab_hist->Add(gHist.createSDC2());
  tab_hist->Add(gHist.createSCH());
  tab_hist->Add(gHist.createSDC3());
  tab_hist->Add(gHist.createSDC4());
  tab_hist->Add(gHist.createTOF());
  tab_hist->Add(gHist.createLAC());
  tab_hist->Add(gHist.createWC());
  tab_hist->Add(gHist.createGe());
  tab_hist->Add(gHist.createBGO());
  tab_hist->Add(gHist.createCorrelation());
  tab_hist->Add(gHist.createTriggerFlag());
  // tab_hist->Add(gHist.createMsT());
#if FLAG_DAQ
  tab_hist->Add(gHist.createDAQ());
#endif
#if TIME_STAMP
  tab_hist->Add(gHist.createTimeStamp( false ));
#endif
  tab_hist->Add(gHist.createDCEff());

  //misc tab
  Int_t btof_id = gHist.getUniqueID(kMisc, 0, kTDC);
  tab_misc->Add(gHist.createTH1(btof_id, "BTOF",
                               300, -10, 5,
                               "BTOF [ns]", ""
                               ));

  tab_misc->Add( gHist.createTH1( btof_id + 1, "BH1-6_BH2-3",
                                  600, 50000, 350000,
                                  "[ch]", "" ) );
  //___ Matrix ___
  gMatrix.Print2D1();
  gMatrix.Print2D2();
  gMatrix.Print3D();
  { // Mtx2D
    Int_t mtx2d_id = gHist.getUniqueID( kMisc, kHul2D, kHitPat2D );
    for( Int_t i=0; i<2; ++i ){
      gHist.createTH2( mtx2d_id + i, Form( "Mtx2D pattern #%d", i+1 ),
                       NumOfSegSCH, 0, NumOfSegSCH,
                       NumOfSegTOF, 0, NumOfSegTOF,
                       "SCH seg", "TOF seg" );
    }
  }
  { // Mtx3D
    Int_t mtx3d_id = gHist.getUniqueID( kMisc, kHul3D, kHitPat2D );
    for( Int_t i=0; i<NumOfSegBH2; ++i ){
      gHist.createTH2( mtx3d_id+i, Form( "Mtx3D pattern BH2-%d", i ),
                       NumOfSegSCH, 0, NumOfSegSCH,
                       NumOfSegTOF, 0, NumOfSegTOF,
                       "SCH seg", "TOF seg" );
    }
  }

  // Set histogram pointers to the vector sequentially.
  // This vector contains both TH1 and TH2.
  // Then you need to do down cast when you use TH2.
  if(0 != gHist.setHistPtr(hptr_array)){ return -1; }

  // Users don't have to touch this section (Make Ps tab),
  // but the file path should be changed.
  // ----------------------------------------------------------
  auto& gPsMaker = PsMaker::getInstance();
  std::vector<TString> detList;
  std::vector<TString> optList;
  gHist.getListOfPsFiles(detList);
  gPsMaker.getListOfOption(optList);

  auto& gPsTab = hddaq::gui::GuiPs::getInstance();
  gPsTab.setFilename(Form("%s/PSFile/pro/default.ps", std::getenv("HOME")));
  gPsTab.initialize(optList, detList);
  // ----------------------------------------------------------

  gStyle->SetOptStat(1110);
  gStyle->SetTitleW(.400);
  gStyle->SetTitleH(.100);
  // gStyle->SetStatW(.420);
  // gStyle->SetStatH(.350);
  gStyle->SetStatW(.320);
  gStyle->SetStatH(.250);

  return 0;
}

//____________________________________________________________________________
Int_t
process_end( void )
{
  hptr_array.clear();
  return 0;
}

//____________________________________________________________________________
Int_t
process_event( void )
{
#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  const Int_t event_number = gUnpacker.get_event_number();
  if( flag_event_cut && event_number%event_cut_factor!=0 )
    return 0;

  // TriggerFlag ---------------------------------------------------
  std::bitset<NumOfSegTFlag> trigger_flag;
  {
    static const Int_t k_device = gUnpacker.get_device_id("TFlag");
    static const Int_t k_tdc    = gUnpacker.get_data_id("TFlag", "tdc");
    static const Int_t tdc_id   = gHist.getSequentialID( kTriggerFlag, 0, kTDC );
    static const Int_t hit_id   = gHist.getSequentialID( kTriggerFlag, 0, kHitPat );
    for( Int_t seg=0; seg<NumOfSegTFlag; ++seg ){
      for( Int_t m=0, n=gUnpacker.get_entries( k_device, 0, seg, 0, k_tdc );
           m<n; ++m ){
	auto tdc = gUnpacker.get( k_device, 0, seg, 0, k_tdc, m );
	if( tdc>0 ){
	  trigger_flag.set( seg );
	  hptr_array[tdc_id+seg]->Fill( tdc );
	}
      }
      if( trigger_flag[seg] ) hptr_array[hit_id]->Fill( seg );
    }
    if( !( trigger_flag[trigger::kSpillEnd] |
	   trigger_flag[trigger::kLevel1OR] ) |
        !( trigger_flag[trigger::kL1SpillOn] |
	   trigger_flag[trigger::kL1SpillOff] |
           trigger_flag[trigger::kSpillEnd] ) ){
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

#if FLAG_DAQ
  // DAQ -------------------------------------------------------------
  {
    //___ node id
    static const Int_t k_eb = gUnpacker.get_fe_id( "k18eb" );
    std::vector<Int_t> vme_fe_id;
    std::vector<Int_t> hul_fe_id;
    std::vector<Int_t> ea0c_fe_id;
    for( auto&& c : gUnpacker.get_root()->get_child_list() ){
      if( !c.second )
	continue;
      TString n = c.second->get_name();
      auto id = c.second->get_id();
      if( n.Contains( "vme" ) )
	vme_fe_id.push_back( id );
      if( n.Contains( "hul" ) )
	hul_fe_id.push_back( id );
      if( n.Contains( "easiroc" ) )
	ea0c_fe_id.push_back( id );
    }

    //___ sequential id
    static const Int_t eb_hid   = gHist.getSequentialID( kDAQ, kEB, kHitPat );
    static const Int_t vme_hid  = gHist.getSequentialID( kDAQ, kVME, kHitPat2D );
    static const Int_t hul_hid  = gHist.getSequentialID( kDAQ, kHUL, kHitPat2D );
    static const Int_t ea0c_hid = gHist.getSequentialID( kDAQ, kEASIROC, kHitPat2D );

    { //___ EB
      auto data_size = gUnpacker.get_node_header( k_eb, DAQNode::k_data_size );
      hptr_array[eb_hid]->Fill( data_size );
    }

    { //___ VME
      for( Int_t i=0, n=vme_fe_id.size(); i<n; ++i ){
	auto data_size = gUnpacker.get_node_header( vme_fe_id[i], DAQNode::k_data_size );
        hptr_array[vme_hid]->Fill( i, data_size );
      }
    }

    { // EASIROC & VMEEASIROC node
      for( Int_t i=0, n=ea0c_fe_id.size(); i<n; ++i ){
        auto data_size = gUnpacker.get_node_header( ea0c_fe_id[i], DAQNode::k_data_size );
        hptr_array[ea0c_hid]->Fill( i, data_size );
      }
    }

    { //___ HUL node
      for( Int_t i=0, n=hul_fe_id.size(); i<n; ++i ){
        auto data_size = gUnpacker.get_node_header( hul_fe_id[i], DAQNode::k_data_size );
        hptr_array[hul_hid]->Fill( i, data_size );
      }
    }

    // { //___ Misc node
    //   auto h = dynamic_cast<TH2*>( hptr_array[misc_id] );
    // }
  }

#endif

  if(
     trigger_flag[trigger::kSpillEnd]
    )
    return 0;

  // MsT -----------------------------------------------------------
  {
    static const Int_t k_device = gUnpacker.get_device_id("MsT");
    static const Int_t k_tof    = gUnpacker.get_plane_id("MsT", "TOF");
    static const Int_t k_sch    = gUnpacker.get_plane_id("MsT", "SCH");
    static const Int_t k_tag    = gUnpacker.get_plane_id("MsT", "tag");
    static const Int_t k_ch     = 0;
    static const Int_t k_tdc    = 0;
    static const Int_t k_n_flag = 7;

    static const Int_t toft_id     = gHist.getSequentialID(kMsT, 0, kTDC);
    static const Int_t scht_id     = gHist.getSequentialID(kMsT, 0, kTDC, NumOfSegTOF*2 +1);
    static const Int_t toft_2d_id  = gHist.getSequentialID(kMsT, 0, kTDC2D);

    static const Int_t tofhit_id   = gHist.getSequentialID(kMsT, 0, kHitPat, 0);
    static const Int_t schhit_id   = gHist.getSequentialID(kMsT, 0, kHitPat, 1);
    static const Int_t flag_id     = gHist.getSequentialID(kMsT, 0, kHitPat, 2);

    // TOF
    for(Int_t seg=0; seg<NumOfSegTOF; ++seg){
      Int_t nhit = gUnpacker.get_entries(k_device, k_tof, seg, k_ch, k_tdc);
      for(Int_t m = 0; m<nhit; ++m){
	hptr_array[tofhit_id]->Fill(seg);

	UInt_t tdc = gUnpacker.get(k_device, k_tof, seg, k_ch, k_tdc, m);
	if(tdc!=0){
	  hptr_array[toft_id + seg]->Fill(tdc);
	  hptr_array[toft_2d_id]->Fill(seg, tdc);
	}
      }
    }// TOF

    // SCH
    for(Int_t seg=0; seg<NumOfSegSCH; ++seg){
      Int_t nhit = gUnpacker.get_entries(k_device, k_sch, seg, k_ch, k_tdc);
      if(nhit!=0){
	hptr_array[schhit_id]->Fill(seg);
      }
      for(Int_t m = 0; m<nhit; ++m){
	UInt_t tdc = gUnpacker.get(k_device, k_sch, seg, k_ch, k_tdc, m);
	if(tdc!=0){
	  hptr_array[scht_id + seg]->Fill(tdc);
	}
      }
    }// SCH

    // FLAG
    for(Int_t i=0; i<k_n_flag; ++i){
      Int_t nhit = gUnpacker.get_entries(k_device, k_tag, 0, k_ch, i);
      if(nhit!=0){
	Int_t flag = gUnpacker.get(k_device, k_tag, 0, k_ch, i, 0);
	if(flag) hptr_array[flag_id]->Fill(i);
      }
    }// FLAG

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }// MsT

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BH1 -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("BH1");
    static const Int_t k_u        = 0; // up
    static const Int_t k_d        = 1; // down
    static const Int_t k_adc      = gUnpacker.get_data_id("BH1", "adc");
    static const Int_t k_tdc      = gUnpacker.get_data_id("BH1", "fpga_leading");

    // TDC gate range
    static const UInt_t tdc_min = gUser.GetParameter("BH1_TDC", 0);
    static const UInt_t tdc_max = gUser.GetParameter("BH1_TDC", 1);

    // Up PMT
    Int_t bh1a_id   = gHist.getSequentialID(kBH1, 0, kADC);
    Int_t bh1t_id   = gHist.getSequentialID(kBH1, 0, kTDC);
    Int_t bh1awt_id = gHist.getSequentialID(kBH1, 0, kADCwTDC);
    for(Int_t seg=0; seg<NumOfSegBH1; ++seg){
      // ADC
      Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc);
      if(nhit!=0){
	UInt_t adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	hptr_array[bh1a_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      for(Int_t m = 0; m<nhit; ++m){
	UInt_t tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc, m);
	if(tdc!=0){
	  hptr_array[bh1t_id + seg]->Fill(tdc);
	  // ADC wTDC
	  if( tdc>tdc_min && tdc<tdc_max && gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc)>0 ){
	    UInt_t adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	    hptr_array[bh1awt_id + seg]->Fill( adc );
	  }
	}
      }
    }

    // Down PMT
    bh1a_id   = gHist.getSequentialID(kBH1, 0, kADC, NumOfSegBH1+1);
    bh1t_id   = gHist.getSequentialID(kBH1, 0, kTDC, NumOfSegBH1+1);
    bh1awt_id = gHist.getSequentialID(kBH1, 0, kADCwTDC, NumOfSegBH1+1);
    for(Int_t seg=0; seg<NumOfSegBH1; ++seg){
      // ADC
      Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc);
      if(nhit!=0){
	UInt_t adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	hptr_array[bh1a_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      for(Int_t m = 0; m<nhit; ++m){
	UInt_t tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc, m);
	if( tdc!=0 ){
	  hptr_array[bh1t_id + seg]->Fill(tdc);
	  // ADC w/TDC
	  if( tdc>tdc_min && tdc<tdc_max && gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc)>0 ){
	    UInt_t adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	    hptr_array[bh1awt_id + seg]->Fill(adc);
	  }
	}
      }
    }

    // Hit pattern && multiplicity
    static const Int_t bh1hit_id = gHist.getSequentialID(kBH1, 0, kHitPat);
    static const Int_t bh1mul_id = gHist.getSequentialID(kBH1, 0, kMulti);
    Int_t multiplicity  = 0;
    for(Int_t seg=0; seg<NumOfSegBH1; ++seg){
      Int_t nhit_bh1u = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      Int_t nhit_bh1d = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      for( Int_t mu=0; mu<nhit_bh1u; ++mu ){
        for( Int_t md=0; md<nhit_bh1d; ++md ){
          UInt_t tdc_u = gUnpacker.get(k_device, 0, seg, k_u, k_tdc, mu);
          UInt_t tdc_d = gUnpacker.get(k_device, 0, seg, k_d, k_tdc, md);
          // TDC range
          if( tdc_min < tdc_u && tdc_u < tdc_max &&
              tdc_min < tdc_d && tdc_d < tdc_max ){
            hptr_array[bh1hit_id]->Fill(seg); // CHitPat
            ++multiplicity;
          }
        } //md
      }// mu
    }// for(seg)
    hptr_array[bh1mul_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }// BH1

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BFT -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device  = gUnpacker.get_device_id("BFT");
    static const Int_t k_uplane  = gUnpacker.get_plane_id("BFT", "upstream");
    static const Int_t k_dplane  = gUnpacker.get_plane_id("BFT", "downstream");
    static const Int_t k_leading = gUnpacker.get_data_id("BFT", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("BFT", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("BFT_TDC", 0);
    static const Int_t tdc_max = gUser.GetParameter("BFT_TDC", 1);

    // sequential id
    static const Int_t bft_tu_id    = gHist.getSequentialID(kBFT, 0, kTDC,      1);
    static const Int_t bft_td_id    = gHist.getSequentialID(kBFT, 0, kTDC,      2);
    static const Int_t bft_ctu_id   = gHist.getSequentialID(kBFT, 0, kTDC,     11);
    static const Int_t bft_ctd_id   = gHist.getSequentialID(kBFT, 0, kTDC,     12);
    static const Int_t bft_totu_id  = gHist.getSequentialID(kBFT, 0, kADC,      1);
    static const Int_t bft_totd_id  = gHist.getSequentialID(kBFT, 0, kADC,      2);
    static const Int_t bft_ctotu_id = gHist.getSequentialID(kBFT, 0, kADC,     11);
    static const Int_t bft_ctotd_id = gHist.getSequentialID(kBFT, 0, kADC,     12);
    static const Int_t bft_hitu_id  = gHist.getSequentialID(kBFT, 0, kHitPat,   1);
    static const Int_t bft_hitd_id  = gHist.getSequentialID(kBFT, 0, kHitPat,   2);
    static const Int_t bft_chitu_id = gHist.getSequentialID(kBFT, 0, kHitPat,  11);
    static const Int_t bft_chitd_id = gHist.getSequentialID(kBFT, 0, kHitPat,  12);
    static const Int_t bft_mul_id   = gHist.getSequentialID(kBFT, 0, kMulti,    1);
    static const Int_t bft_cmul_id  = gHist.getSequentialID(kBFT, 0, kMulti,   11);

    static const Int_t bft_ctu_2d_id = gHist.getSequentialID(kBFT, 0, kTDC2D,   1);
    static const Int_t bft_ctd_2d_id = gHist.getSequentialID(kBFT, 0, kTDC2D,   2);
    static const Int_t bft_ctotu_2d_id = gHist.getSequentialID(kBFT, 0, kADC2D, 1);
    static const Int_t bft_ctotd_2d_id = gHist.getSequentialID(kBFT, 0, kADC2D, 2);

    Int_t multiplicity  = 0; // includes both u and d planes.
    Int_t cmultiplicity = 0; // includes both u and d planes.
    Int_t tdc_prev      = 0;
    for(Int_t i = 0; i<NumOfSegBFT; ++i){
      Int_t nhit_u = gUnpacker.get_entries(k_device, k_uplane, 0, i, k_leading);
      Int_t nhit_d = gUnpacker.get_entries(k_device, k_dplane, 0, i, k_leading);

      // u plane
      tdc_prev = 0;
      for(Int_t m = 0; m<nhit_u; ++m){
	Int_t tdc = gUnpacker.get(k_device, k_uplane, 0, i, k_leading, m);
	Int_t tdc_t = gUnpacker.get(k_device, k_uplane, 0, i, k_trailing, m);
	Int_t tot = tdc - tdc_t;
	hptr_array[bft_tu_id]->Fill(tdc);
	hptr_array[bft_totu_id]->Fill(tot);
	if(tdc_min < tdc && tdc < tdc_max){
	  ++multiplicity;
	  hptr_array[bft_hitu_id]->Fill(i);
	}
	if(tdc_prev==tdc) continue;
	tdc_prev = tdc;
	if(tot==0) continue;
	hptr_array[bft_ctu_id]->Fill(tdc);
	hptr_array[bft_ctotu_id]->Fill(tot);
	hptr_array[bft_ctu_2d_id]->Fill(i, tdc);
	hptr_array[bft_ctotu_2d_id]->Fill(i, tot);
	if(tdc_min < tdc && tdc < tdc_max){
	  ++cmultiplicity;
	  hptr_array[bft_chitu_id]->Fill(i);
	}
      }

      // d plane
      tdc_prev = 0;
      for(Int_t m = 0; m<nhit_d; ++m){
	Int_t tdc = gUnpacker.get(k_device, k_dplane, 0, i, k_leading, m);
	Int_t tdc_t = gUnpacker.get(k_device, k_dplane, 0, i, k_trailing, m);
	Int_t tot = tdc - tdc_t;
	hptr_array[bft_td_id]->Fill(tdc);
	hptr_array[bft_totd_id]->Fill(tot);
	if(tdc_min < tdc && tdc < tdc_max){
	  ++multiplicity;
	  hptr_array[bft_hitd_id]->Fill(i);
	}
	if(tdc_prev==tdc) continue;
	tdc_prev = tdc;
	if(tot==0) continue;
	hptr_array[bft_ctd_id]->Fill(tdc);
	hptr_array[bft_ctotd_id]->Fill(tot);
	hptr_array[bft_ctd_2d_id]->Fill(i, tdc);
	hptr_array[bft_ctotd_2d_id]->Fill(i, tot);
	if(tdc_min < tdc && tdc < tdc_max){
	  ++cmultiplicity;
	  hptr_array[bft_chitd_id]->Fill(i);
	}
      }
    }
    hptr_array[bft_mul_id]->Fill(multiplicity);
    hptr_array[bft_cmul_id]->Fill(cmultiplicity);

#if 0
    // Debug, dump data relating this detector
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
    static const Int_t k_device   = gUnpacker.get_device_id("BC4");
    static const Int_t k_leading  = gUnpacker.get_data_id("BC4", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("BC4", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("BC4_TDC", 0);
    static const Int_t tdc_max = gUser.GetParameter("BC4_TDC", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("BC4_TOT", 0);
    // static const Int_t tot_max = gUser.GetParameter("BC4_TOT", 1);

    // sequential id
    static const Int_t bc4t_id    = gHist.getSequentialID(kBC4, 0, kTDC);
    static const Int_t bc4tot_id  = gHist.getSequentialID(kBC4, 0, kADC);
    static const Int_t bc4t1st_id = gHist.getSequentialID(kBC4, 0, kTDC2D);
    static const Int_t bc4hit_id  = gHist.getSequentialID(kBC4, 0, kHitPat);
    static const Int_t bc4mul_id  = gHist.getSequentialID(kBC4, 0, kMulti);
    static const Int_t bc4mulwt_id
      = gHist.getSequentialID(kBC4, 0, kMulti, 1+NumOfLayersBC4);

    static const Int_t bc4t_ctot_id    = gHist.getSequentialID(kBC4, 0, kTDC,    1+kTOTcutOffset);
    static const Int_t bc4tot_ctot_id  = gHist.getSequentialID(kBC4, 0, kADC,    1+kTOTcutOffset);
    static const Int_t bc4t1st_ctot_id = gHist.getSequentialID(kBC4, 0, kTDC2D,  1+kTOTcutOffset);
    static const Int_t bc4hit_ctot_id  = gHist.getSequentialID(kBC4, 0, kHitPat, 1+kTOTcutOffset);
    static const Int_t bc4mul_ctot_id  = gHist.getSequentialID(kBC4, 0, kMulti,  1+kTOTcutOffset);
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

  // BH2 -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("BH2");
    static const Int_t k_u      = 0; // up
    static const Int_t k_d      = 1; // down
    static const Int_t k_adc    = gUnpacker.get_data_id("BH2", "adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("BH2", "fpga_leading");
    static const Int_t k_mt     = gUnpacker.get_data_id("BH2", "fpga_meantime");

    // TDC gate range
    static const UInt_t tdc_min = gUser.GetParameter("BH2_TDC", 0);
    static const UInt_t tdc_max = gUser.GetParameter("BH2_TDC", 1);

    // UP
    Int_t bh2a_id   = gHist.getSequentialID(kBH2, 0, kADC);
    Int_t bh2t_id   = gHist.getSequentialID(kBH2, 0, kTDC);
    Int_t bh2awt_id = gHist.getSequentialID(kBH2, 0, kADCwTDC);
    for(Int_t seg=0; seg<NumOfSegBH2; ++seg){
      // ADC
      Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc);
      if(nhit != 0){
	UInt_t adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	hptr_array[bh2a_id + seg]->Fill(adc);
      }
      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      for(Int_t m = 0; m<nhit; ++m){
	UInt_t tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc, m);
	if(tdc!=0){
	  hptr_array[bh2t_id + seg]->Fill(tdc);
	  // ADC w/TDC
	  if( tdc_min<tdc && tdc<tdc_max && gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc)>0){
	    UInt_t adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	    hptr_array[bh2awt_id + seg]->Fill(adc);
	  }
	}
      }
    }

    // DOWN
    bh2a_id   = gHist.getSequentialID(kBH2, 0, kADC,     NumOfSegBH2+1);
    bh2t_id   = gHist.getSequentialID(kBH2, 0, kTDC,     NumOfSegBH2+1);
    bh2awt_id = gHist.getSequentialID(kBH2, 0, kADCwTDC, NumOfSegBH2+1);
    for(Int_t seg=0; seg<NumOfSegBH2; ++seg){
      // ADC
      Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc);
      if(nhit != 0){
	UInt_t adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	hptr_array[bh2a_id + seg]->Fill(adc);
      }
      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      for(Int_t m = 0; m<nhit; ++m){
	UInt_t tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc, m);
	if( tdc!=0 ){
	  hptr_array[bh2t_id + seg]->Fill(tdc);
	  // ADC w/TDC
	  if( tdc_min<tdc && tdc<tdc_max && gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc)>0){
	    UInt_t adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	    hptr_array[bh2awt_id + seg]->Fill( adc );
	  }
	}
      }
    }

    // Hit pattern &&  Multiplicity
    static const Int_t bh2hit_id = gHist.getSequentialID(kBH2, 0, kHitPat);
    static const Int_t bh2mul_id = gHist.getSequentialID(kBH2, 0, kMulti);
    Int_t multiplicity = 0;
    for(Int_t seg=0; seg<NumOfSegBH2; ++seg){
      Int_t nhit_u = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      // Int_t nhit_d = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      for( Int_t mu=0; mu<nhit_u; ++mu ){
	UInt_t tdc_u = gUnpacker.get(k_device, 0, seg, k_u, k_tdc, mu);
	if( tdc_min < tdc_u && tdc_u < tdc_max ){
	  hptr_array[bh2hit_id]->Fill( seg );
	  ++multiplicity;
	}
      }
    }
    hptr_array[bh2mul_id]->Fill(multiplicity);

    // Mean Timer
    bh2t_id   = gHist.getSequentialID(kBH2, 0, kBH2MT, 1);
    for(Int_t seg=0; seg<NumOfSegBH2; ++seg){
      Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_mt);
      for(Int_t m = 0; m<nhit; ++m){
	UInt_t mt = gUnpacker.get(k_device, 0, seg, k_u, k_mt, m);
	if(mt!=0){
	  hptr_array[bh2t_id + seg]->Fill(mt);
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

  // SCH -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("SCH");
    static const Int_t k_leading  = gUnpacker.get_data_id("SCH", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("SCH", "trailing");
    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("SCH_TDC", 0);
    static const Int_t tdc_max = gUser.GetParameter("SCH_TDC", 1);
    // sequential id
    static const Int_t tdc_id    = gHist.getSequentialID(kSCH, 0, kTDC, 1);
    static const Int_t tot_id    = gHist.getSequentialID(kSCH, 0, kADC, 1);
    static const Int_t tdcall_id = gHist.getSequentialID(kSCH, 0, kTDC, NumOfSegSCH+1);
    static const Int_t totall_id = gHist.getSequentialID(kSCH, 0, kADC, NumOfSegSCH+1);
    static const Int_t hit_id    = gHist.getSequentialID(kSCH, 0, kHitPat, 1);
    static const Int_t mul_id    = gHist.getSequentialID(kSCH, 0, kMulti, 1);
    static const Int_t tdc2d_id  = gHist.getSequentialID(kSCH, 0, kTDC2D, 1);
    static const Int_t tot2d_id  = gHist.getSequentialID(kSCH, 0, kADC2D, 1);

    Int_t multiplicity = 0;
    for( Int_t i=0; i<NumOfSegSCH; ++i ){
      Int_t nhit = gUnpacker.get_entries(k_device, 0, i, 0, k_leading);
      Bool_t is_in_gate = false;
      for(Int_t m = 0; m<nhit; ++m){
	Int_t tdc      = gUnpacker.get(k_device, 0, i, 0, k_leading,  m);
	Int_t trailing = gUnpacker.get(k_device, 0, i, 0, k_trailing, m);
	Int_t tot      = tdc - trailing;
	hptr_array[tdc_id +i]->Fill( tdc );
	hptr_array[tdcall_id]->Fill( tdc );
	hptr_array[tot_id +i]->Fill( tot );
	hptr_array[totall_id]->Fill( tot );
	hptr_array[tdc2d_id]->Fill( i, tdc );
	hptr_array[tot2d_id]->Fill( i, tot );
	if( tdc_min<tdc && tdc<tdc_max ){
	  is_in_gate = true;
	}
      }
      if( is_in_gate ){
	++multiplicity;
	hptr_array[hit_id]->Fill( i );
      }
    }
    hptr_array[mul_id]->Fill( multiplicity );

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
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

  //------------------------------------------------------------------
  // TOF
  //------------------------------------------------------------------
  {
    // data typep
    static const Int_t k_device = gUnpacker.get_device_id("TOF");
    static const Int_t k_u      = 0; // up
    static const Int_t k_d      = 1; // down
    static const Int_t k_adc    = gUnpacker.get_data_id("TOF","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("TOF","fpga_leading");

    // TDC gate range
    static const UInt_t tdc_min = gUser.GetParameter("TOF_TDC", 0);
    static const UInt_t tdc_max = gUser.GetParameter("TOF_TDC", 1);

    // sequential id
    Int_t tofa_id   = gHist.getSequentialID(kTOF, 0, kADC);
    Int_t toft_id   = gHist.getSequentialID(kTOF, 0, kTDC);
    Int_t tofawt_id = gHist.getSequentialID(kTOF, 0, kADCwTDC);
    for(Int_t seg = 0; seg<NumOfSegTOF; ++seg){
      // ADC
      Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc);
      if( nhit!=0 ){
	UInt_t adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	hptr_array[tofa_id + seg]->Fill( adc );
      }
      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      Bool_t is_in_gate = false;
      for(Int_t m = 0; m<nhit; ++m){
	UInt_t tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc, m);
	if(tdc!=0){
	  hptr_array[toft_id + seg]->Fill(tdc);
	  // ADC wTDC
	  if( tdc_min<tdc && tdc<tdc_max ) is_in_gate = true;
	}

	if(is_in_gate){
	  if( gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc)>0 ){
	    UInt_t adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	    hptr_array[tofawt_id + seg]->Fill( adc );
	  }
	}
      }
    }

    // Down PMT
    tofa_id   = gHist.getSequentialID(kTOF, 0, kADC, NumOfSegTOF+1);
    toft_id   = gHist.getSequentialID(kTOF, 0, kTDC, NumOfSegTOF+1);
    tofawt_id = gHist.getSequentialID(kTOF, 0, kADCwTDC, NumOfSegTOF+1);

    for(Int_t seg = 0; seg<NumOfSegTOF; ++seg){
      // ADC
      Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc);
      if(nhit != 0){
	UInt_t adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	hptr_array[tofa_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      Bool_t is_in_gate = false;
      for(Int_t m = 0; m<nhit; ++m){
	UInt_t tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc, m);
	if(tdc != 0){
	  hptr_array[toft_id + seg]->Fill(tdc);
	  // ADC w/TDC
	  if( tdc_min<tdc && tdc<tdc_max ) is_in_gate = true;

	  if(is_in_gate){
	    if( gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc)>0 ){
	      UInt_t adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	      hptr_array[tofawt_id + seg]->Fill( adc );
	    }
	  }
	}
      }
    }

    // Hit pattern && multiplicity
    static const Int_t tofhit_id = gHist.getSequentialID(kTOF, 0, kHitPat);
    static const Int_t tofmul_id = gHist.getSequentialID(kTOF, 0, kMulti);
    Int_t multiplicity = 0;
    for(Int_t seg=0; seg<NumOfSegTOF; ++seg){
      Int_t nhit_tofu = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      Int_t nhit_tofd = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      for( Int_t mu=0; mu<nhit_tofu; ++mu ){
        for( Int_t md=0; md<nhit_tofd; ++md ){
          UInt_t tdc_u = gUnpacker.get(k_device, 0, seg, k_u, k_tdc, mu);
          UInt_t tdc_d = gUnpacker.get(k_device, 0, seg, k_d, k_tdc, md);
          if( tdc_min < tdc_u && tdc_u < tdc_max &&
              tdc_min < tdc_d && tdc_d < tdc_max ){
            hptr_array[tofhit_id]->Fill(seg);
            ++multiplicity;
          }
        }
      }
    }
    hptr_array[tofmul_id]->Fill(multiplicity);

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
    static const Int_t k_u      = 0; // up
    static const Int_t k_tdc    = gUnpacker.get_data_id("LAC","tdc");
    static const Int_t lachit_id = gHist.getSequentialID(kLAC, 0, kHitPat);
    static const Int_t lacmul_id = gHist.getSequentialID(kLAC, 0, kMulti);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("LAC_TDC", 0);
    static const Int_t tdc_max = gUser.GetParameter("LAC_TDC", 1);

    Int_t lact_id   = gHist.getSequentialID(kLAC, 0, kTDC);
    Int_t multiplicity = 0;
    for(Int_t seg = 0; seg<NumOfSegLAC; ++seg){
      Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      Bool_t is_in_gate = false;
      for(Int_t m = 0; m<nhit; ++m){
	Int_t tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc, m);
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


  // WC for E42 test-----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("WC");
    static const Int_t k_u      = 0; // up
    static const Int_t k_d      = 1; // down
    static const Int_t k_sum    = 2; // sum
    static const Int_t k_adc    = gUnpacker.get_data_id("WC", "adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("WC", "tdc");
    // TDC gate range
    static const UInt_t tdc_min = gUser.GetParameter("WC_TDC", 0);
    static const UInt_t tdc_max = gUser.GetParameter("WC_TDC", 1);
    // UP
    Int_t wca_id   = gHist.getSequentialID(kWC, 0, kADC);
    Int_t wct_id   = gHist.getSequentialID(kWC, 0, kTDC);
    Int_t wcawt_id = gHist.getSequentialID(kWC, 0, kADCwTDC);    // UP
    for( Int_t seg=0; seg<NumOfSegWC; ++seg ){
      // ADC
      Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc);
      if(nhit != 0){
	UInt_t adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	hptr_array[wca_id + seg]->Fill(adc);
      }
      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      for(Int_t m = 0; m<nhit; ++m){
      	UInt_t tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc, m);
      	if(tdc!=0){
      	  hptr_array[wct_id + seg]->Fill(tdc);
      	  // ADC w/TDC
      	  if( tdc_min<tdc && tdc<tdc_max &&
              gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc)>0 ){
      	    UInt_t adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
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
      Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc);
      if(nhit != 0){
	UInt_t adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	hptr_array[wca_id + seg]->Fill(adc);
      }
      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      for(Int_t m = 0; m<nhit; ++m){
      	UInt_t tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc, m);
      	if( tdc!=0 ){
      	  hptr_array[wct_id + seg]->Fill(tdc);
      	  // ADC w/TDC
      	  if( tdc_min<tdc && tdc<tdc_max &&
              gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc)>0 ){
      	    UInt_t adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
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
      if( nhit != 0 ){
	UInt_t adc = gUnpacker.get(k_device, 0, seg, k_sum, k_adc);
	hptr_array[wca_id + seg]->Fill(adc);
      }
      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_sum, k_tdc);
      Bool_t is_in_gate = false;
      for( Int_t m = 0; m<nhit; ++m ){
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
    static const Int_t k_device_bft  = gUnpacker.get_device_id("BFT");
    static const Int_t k_device_bh2  = gUnpacker.get_device_id("BH2");
    static const Int_t k_device_sch  = gUnpacker.get_device_id("SCH");
    static const Int_t k_device_tof  = gUnpacker.get_device_id("TOF");
    static const Int_t k_device_bc3  = gUnpacker.get_device_id("BC3");
    static const Int_t k_device_bc4  = gUnpacker.get_device_id("BC4");
    static const Int_t k_device_sdc1 = gUnpacker.get_device_id("SDC1");
    // static const Int_t k_device_sdc2 = gUnpacker.get_device_id("SDC2");
    static const Int_t k_device_sdc3 = gUnpacker.get_device_id("SDC3");
    static const Int_t k_device_sdc4 = gUnpacker.get_device_id("SDC4");

    // sequential id
    Int_t cor_id= gHist.getSequentialID(kCorrelation, 0, 0, 1);
    Int_t mtx2d_id= gHist.getSequentialID(kCorrelation, 1, 0, 1);
    Int_t mtx3d_id= gHist.getSequentialID(kCorrelation, 2, 0, 1);

    // BH1 vs BFT
    TH2* hcor_bh1bft = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t seg1 = 0; seg1<NumOfSegBH1; ++seg1){
      for(Int_t seg2 = 0; seg2<NumOfSegBFT; ++seg2){
	Int_t nhitBH1 = gUnpacker.get_entries(k_device_bh1, 0, seg1, 0, 1);
	Int_t nhitBFT = gUnpacker.get_entries(k_device_bft, 0, 0, seg2, 0);
	if( nhitBH1 == 0 || nhitBFT == 0 ) continue;
	Bool_t hitBFT = false;
	for( Int_t m = 0; m<nhitBFT; ++m ){
	  Int_t tdc = gUnpacker.get(k_device_bft, 0, 0, seg2, 0, m);
	  if( !hitBFT && tdc > 0 ) hitBFT = true;
	}
	Int_t tdcBH1 = gUnpacker.get(k_device_bh1, 0, seg1, 0, 1);
	Bool_t hitBH1 = ( tdcBH1 > 0 );
	if( hitBFT && hitBH1 ){
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

    //TOF TDC gate range
    static const UInt_t tof_tdc_min = gUser.GetParameter("TOF_TDC", 0);
    static const UInt_t tof_tdc_max = gUser.GetParameter("TOF_TDC", 1);

    //SCH TDC gate range
    static const UInt_t sch_tdc_min = gUser.GetParameter("SCH_TDC", 0);
    static const UInt_t sch_tdc_max = gUser.GetParameter("SCH_TDC", 1);

    // TOF vs SCH ( x BH2 )
    auto hcor_tofsch = dynamic_cast<TH2*>( hptr_array[cor_id++] );
    auto hmtx2d1 = dynamic_cast<TH2*>( hptr_array[mtx2d_id] );
    auto hmtx2d2 = dynamic_cast<TH2*>( hptr_array[mtx2d_id+1] );
    for( Int_t seg1 = 0; seg1<NumOfSegSCH; ++seg1 ){
      for( Int_t seg2 = 0; seg2<NumOfSegTOF; ++seg2 ){
	Int_t hitSCH = gUnpacker.get_entries( k_device_sch, 0, seg1, 0, 0 );
	Int_t hitTOF = gUnpacker.get_entries( k_device_tof, 0, seg2, 0, 1 );
        for( Int_t m1=0; m1<hitSCH; ++m1 ){
          auto tdcSCH = gUnpacker.get( k_device_sch, 0, seg1, 0, 0, m1 );
          if( tdcSCH == 0 ) continue;
          for( Int_t m2=0; m2<hitTOF; ++m2 ){
            auto tdcTOF = gUnpacker.get( k_device_tof, 0, seg2, 0, 1, m2 );
            if( tof_tdc_min < tdcTOF && tdcTOF < tof_tdc_max &&
                sch_tdc_min < tdcSCH && tdcSCH < sch_tdc_max ){
              hcor_tofsch->Fill( seg1, seg2 );
              if( trigger_flag[trigger::kTrigAPS] )
                hmtx2d1->Fill( seg1, seg2 );
              if( trigger_flag[trigger::kTrigBPS] )
                hmtx2d2->Fill( seg1, seg2 );
              for( Int_t seg3=0; seg3<NumOfSegBH2; ++seg3 ){
                auto nhBh2 = gUnpacker.get_entries( k_device_bh2, 0, seg3, 0, 1 );
                if( nhBh2 == 0 ) continue;
                auto tdcBH2 = gUnpacker.get( k_device_bh2, 0, seg3, 0, 1 );
                if( tdcBH2 == 0 ) continue;
                if( trigger_flag[trigger::kTrigAPS] ||
                    trigger_flag[trigger::kTrigBPS] ){
                  auto hmtx3d  = dynamic_cast<TH2*>( hptr_array[mtx3d_id+seg3] );
                  hmtx3d->Fill( seg1, seg2 );
                }
              }
            }
          }
        }
      }
    }

    // BC3 vs BC4
    auto hcor_bc3bc4 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t wire1 = 0; wire1<NumOfWireBC3; ++wire1){
      for(Int_t wire2 = 0; wire2<NumOfWireBC4; ++wire2){
	Int_t hitBC3 = gUnpacker.get_entries(k_device_bc3, 0, 0, wire1, 0);
	Int_t hitBC4 = gUnpacker.get_entries(k_device_bc4, 5, 0, wire2, 0);
	if(hitBC3 == 0 || hitBC4 == 0)continue;
	hcor_bc3bc4->Fill(wire1, wire2);
      }
    }

    // SDC3 vs SDC1
    auto hcor_sdc1sdc3 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
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
    for( Int_t seg=0; seg<NumOfSegTOF; ++seg ){
      for( Int_t wire=0; wire<NumOfWireSDC4X; ++wire ){
	Int_t hitTOF  = gUnpacker.get_entries(k_device_tof, 0, seg, 0, 1);
	Int_t hitSDC4 = gUnpacker.get_entries(k_device_sdc4, 2, 0, wire, 0);
	if( hitTOF == 0 || hitSDC4 == 0 ) continue;
	hcor_tofsdc4->Fill( wire, seg );
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

    static const Int_t k_u      = 0; // up
    static const Int_t k_d      = 1; // down
    static const Int_t k_tdc    = gUnpacker.get_data_id("BH1", "fpga_leading");

    // HodoParam
    static const Int_t cid_bh1  = 1;
    static const Int_t cid_bh2  = 2;
    static const Int_t plid     = 0;

    // Sequential ID
    static const Int_t btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);

    static const auto& hodoMan = HodoParamMan::GetInstance();

    // BH2
    Double_t t0  = -999;
    Double_t ofs = 0;
    for( Int_t seg=0; seg<NumOfSegBH2; ++seg ){
      Int_t nhitu = gUnpacker.get_entries(k_d_bh2, 0, seg, k_u, k_tdc );
      // Int_t nhitd = gUnpacker.get_entries(k_d_bh2, 0, seg, k_d, k_tdc );
      if( nhitu != 0 /* && nhitd != 0 */ ){
        Int_t tdcu = gUnpacker.get(k_d_bh2, 0, seg, k_u, k_tdc);
        // Int_t tdcd = gUnpacker.get(k_d_bh2, 0, seg, k_d, k_tdc);
        if( tdcu != 0 /* && tdcd != 0 */ ){
          Double_t bh2ut; //, bh2dt;
          hodoMan.GetTime(cid_bh2, plid, seg, k_u, tdcu, bh2ut);
          // hodoMan.GetTime(cid_bh2, plid, seg, k_d, tdcd, bh2dt);
          if( TMath::Abs( t0 ) > TMath::Abs( bh2ut ) ){
	    hodoMan.GetTime(cid_bh2, plid, seg, 2, 0, ofs);
            t0 = bh2ut;
          }
        }//if(tdc)
      }// if(nhit)
    }
    // BH1
    for(Int_t seg = 0; seg<NumOfSegBH1; ++seg){
      Int_t nhitu = gUnpacker.get_entries(k_d_bh1, 0, seg, k_u, k_tdc);
      Int_t nhitd = gUnpacker.get_entries(k_d_bh1, 0, seg, k_d, k_tdc);
      if( nhitu != 0 &&  nhitd != 0 ){
	Int_t tdcu = gUnpacker.get(k_d_bh1, 0, seg, k_u, k_tdc);
	Int_t tdcd = gUnpacker.get(k_d_bh1, 0, seg, k_d, k_tdc);
	if(tdcu != 0 && tdcd != 0){
	  Double_t bh1tu, bh1td;
	  hodoMan.GetTime(cid_bh1, plid, seg, k_u, tdcu, bh1tu);
	  hodoMan.GetTime(cid_bh1, plid, seg, k_d, tdcd, bh1td);
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
    static const Int_t k_d_bh1  = gUnpacker.get_device_id( "BH1" );
    static const Int_t k_d_bh2  = gUnpacker.get_device_id( "BH2" );

    static const Int_t k_u      = 0; // up
    static const Int_t k_d      = 1; // down
    static const Int_t k_tdc    = gUnpacker.get_data_id("BH1", "fpga_leading");

    // Sequential ID
    static const Int_t btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);

    // BH2
    Int_t    multiplicity = 0;
    Double_t t0  = -999;
    Double_t ofs = 0;
    Int_t seg = 2;
    Int_t nhitu = gUnpacker.get_entries(k_d_bh2, 0, seg, k_u, k_tdc);
    // Int_t nhitd = gUnpacker.get_entries(k_d_bh2, 0, seg, k_d, k_tdc);
    if( nhitu != 0 /* && nhitd != 0 */ ){
      Int_t tdcu = gUnpacker.get(k_d_bh2, 0, seg, k_u, k_tdc);
      // Int_t tdcd = gUnpacker.get(k_d_bh2, 0, seg, k_d, k_tdc);
      if( tdcu != 0 /* && tdcd != 0 */ ){
        ++multiplicity;
        t0 = tdcu;//(Double_t)(tdcu+tdcd)/2.;
      }//if(tdc)
    }// if(nhit)

    if( multiplicity == 1 ){
      seg = 5;
      // BH1
      Int_t nhitu = gUnpacker.get_entries(k_d_bh1, 0, seg, k_u, k_tdc);
      Int_t nhitd = gUnpacker.get_entries(k_d_bh1, 0, seg, k_d, k_tdc);
      if(nhitu != 0 &&  nhitd != 0){
	Int_t tdcu = gUnpacker.get(k_d_bh1, 0, seg, k_u, k_tdc);
	Int_t tdcd = gUnpacker.get(k_d_bh1, 0, seg, k_d, k_tdc);
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

    // sequential id
    /*
    static const Int_t baca_id   = gHist.getSequentialID(kBAC, 0, kADC,     2);
    static const Int_t bact_id   = gHist.getSequentialID(kBAC, 0, kTDC,     2);
    static const Int_t bacawt_id = gHist.getSequentialID(kBAC, 0, kADCwTDC, 2);
    static const Int_t bach_id   = gHist.getSequentialID(kBAC, 0, kHitPat,  1);
    static const Int_t bacm_id   = gHist.getSequentialID(kBAC, 0, kMulti,   1);
    */

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

    hptr_array[bacm_id]->Fill( multiplicity );

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//BAC

    // FAC -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("FAC");
    static const Int_t k_adc    = gUnpacker.get_data_id("FAC","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("FAC","tdc");

    // sequential id
    static const Int_t baca_id   = gHist.getSequentialID(kFAC, 0, kADC,     1);
    static const Int_t bact_id   = gHist.getSequentialID(kFAC, 0, kTDC,     1);
    static const Int_t bacawt_id = gHist.getSequentialID(kFAC, 0, kADCwTDC, 1);
    static const Int_t bach_id   = gHist.getSequentialID(kFAC, 0, kHitPat,  1);
    static const Int_t bacm_id   = gHist.getSequentialID(kFAC, 0, kMulti,   1);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("FAC_TDC", 0);
    static const Int_t tdc_max = gUser.GetParameter("FAC_TDC", 1);

    Int_t multiplicity = 0;
    for(Int_t seg = 0; seg<NumOfSegFAC; ++seg){
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

    hptr_array[bacm_id]->Fill( multiplicity );

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//FAC

  // PVAC -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("PVAC");
    static const Int_t k_adc    = gUnpacker.get_data_id("PVAC","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("PVAC","tdc");

    // sequential id
    static const Int_t baca_id   = gHist.getSequentialID(kPVAC, 0, kADC,     1);
    static const Int_t bact_id   = gHist.getSequentialID(kPVAC, 0, kTDC,     1);
    static const Int_t bacawt_id = gHist.getSequentialID(kPVAC, 0, kADCwTDC, 1);
    static const Int_t bach_id   = gHist.getSequentialID(kPVAC, 0, kHitPat,  1);
    static const Int_t bacm_id   = gHist.getSequentialID(kPVAC, 0, kMulti,   1);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("PVAC_TDC", 0);
    static const Int_t tdc_max = gUser.GetParameter("PVAC_TDC", 1);

    Int_t multiplicity = 0;
    for(Int_t seg = 0; seg<NumOfSegPVAC; ++seg){
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

    hptr_array[bacm_id]->Fill( multiplicity );

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//PVAC

  //------------------------------------------------------------------
  // Hyperball-X'
  //------------------------------------------------------------------

  // Ge --------------------------------------------------------------
  {
    static const Int_t tfa_min = gUser.GetParameter("TFA_TDC", 0);
    static const Int_t tfa_max = gUser.GetParameter("TFA_TDC", 1);
    static const Int_t crm_min = gUser.GetParameter("CRM_TDC", 0);
    static const Int_t crm_max = gUser.GetParameter("CRM_TDC", 1);

    // data type
    static const Int_t k_device = gUnpacker.get_device_id("Ge");
    static const Int_t k_adc    = gUnpacker.get_data_id("Ge","adc");
    static const Int_t k_tfa    = gUnpacker.get_data_id("Ge","tfa_leading");
    static const Int_t k_crm    = gUnpacker.get_data_id("Ge","crm_leading");
    static const Int_t k_rst    = gUnpacker.get_data_id("Ge","reset_time");
    static const Int_t k_flag_device = gUnpacker.get_device_id("HBXTFlag");
    static const Int_t k_flag_tdc    = gUnpacker.get_data_id("HBXTFlag","tdc");

    // sequential id
    // sequential hist
    static const Int_t ge_flag_hitpat_id    = gHist.getSequentialID(kGe, 0, kFlagHitPat);
    static const Int_t ge_flag_tdc_id    = gHist.getSequentialID(kGe, 0, kFlagTDC);

    static const Int_t ge_adc_id    = gHist.getSequentialID(kGe, 0, kADC);
    static const Int_t ge_adc_wt_id = gHist.getSequentialID(kGe, 0, kADCwTDC);
    static const Int_t ge_adc_wc_id = gHist.getSequentialID(kGe, 0, kADCwTDC, NumOfSegGe+1);

    static const Int_t ge_tfa_id    = gHist.getSequentialID(kGe, 0, kTFA);
    static const Int_t ge_crm_id    = gHist.getSequentialID(kGe, 0, kCRM);
    static const Int_t ge_rst_id    = gHist.getSequentialID(kGe, 0, kRST);

    // hitpat hist id
    static const Int_t ge_hitpat_id = gHist.getSequentialID(kGe, 0, kHitPat);

    // 2d hist id
    static const Int_t ge_adc2d_id = gHist.getSequentialID(kGe, 0, kADC2D);
    static const Int_t ge_tfa2d_id = gHist.getSequentialID(kGe, 0, kTFA2D);
    static const Int_t ge_crm2d_id = gHist.getSequentialID(kGe, 0, kCRM2D);
    static const Int_t ge_rst2d_id = gHist.getSequentialID(kGe, 0, kRST2D);

    static const Int_t ge_tfa_adc_id = gHist.getSequentialID(kGe, 0, kTFA_ADC);
    static const Int_t ge_rst_adc_id = gHist.getSequentialID(kGe, 0, kRST_ADC);
    static const Int_t ge_tfa_crm_id = gHist.getSequentialID(kGe, 0, kTFA_CRM);

    // HBX TriggerFlag ---------------------------------------------------
    for(Int_t seg = 0; seg<NumOfSegHbxTrig; ++seg){
      Int_t nhit_flag = gUnpacker.get_entries(k_flag_device, 0, seg, 0, k_flag_tdc);
      if( nhit_flag>0 ){
	Int_t tdc = gUnpacker.get( k_flag_device, 0, seg, 0, k_flag_tdc );
	if( tdc>0 ){
	  hptr_array[ge_flag_tdc_id+seg]->Fill( tdc );
	  hptr_array[ge_flag_hitpat_id]->Fill( seg );
	}
      }
    }

    for(Int_t seg = 0; seg<NumOfSegGe; ++seg){
      // ADC
      Int_t nhit_adc = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      Int_t adc = -9999;
      if(nhit_adc != 0){
	adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[ge_adc_id + seg]->Fill(adc);
	hptr_array[ge_adc2d_id]->Fill(seg, adc);

	if(115 < adc && adc < 7500){
	  hptr_array[ge_hitpat_id]->Fill(seg);
	}

      }

      // TFA
      Int_t nhit_tfa = gUnpacker.get_entries(k_device, 0, seg, 0, k_tfa);
      Int_t tfa_first = -9999;
      Int_t tfaflag = 0;
      if(nhit_tfa != 0){
	tfa_first = gUnpacker.get(k_device, 0, seg, 0, k_tfa, 0);
	if(adc >= 0) hptr_array[ge_tfa_adc_id + seg]->Fill(tfa_first, adc);
	for(Int_t m = 0; m<nhit_tfa; ++m){
	  Int_t tfa = gUnpacker.get(k_device, 0, seg, 0, k_tfa, m);
	  hptr_array[ge_tfa_id + seg]->Fill(tfa);
	  hptr_array[ge_tfa2d_id]->Fill(seg, tfa);
	  if( tfa_min < tfa && tfa < tfa_max ) tfaflag = 1;
	}
	//ADCwTFA
	if(tfaflag == 1) hptr_array[ge_adc_wt_id + seg]->Fill(adc);
      }

      // CRM
      Int_t nhit_crm = gUnpacker.get_entries(k_device, 0, seg, 0, k_crm);
      Int_t crmflag = 0;
      if(nhit_crm != 0){
	Int_t crm_first = gUnpacker.get(k_device, 0, seg, 0, k_crm, 0);
	if(tfa_first > 0) hptr_array[ge_tfa_crm_id + seg]->Fill(tfa_first, crm_first);
	for(Int_t m = 0; m<nhit_crm; ++m){
	  Int_t crm = gUnpacker.get(k_device, 0, seg, 0, k_crm, m);
	  hptr_array[ge_crm_id + seg]->Fill(crm);
	  hptr_array[ge_crm2d_id]->Fill(seg, crm);
	  if( crm_min < crm && crm < crm_max ) crmflag = 1;
	}
	//ADCwCRM
	if(crmflag == 1) hptr_array[ge_adc_wc_id + seg]->Fill(adc);
      }

      // RST
      Int_t nhit_rst = gUnpacker.get_entries(k_device, 0, seg, 0, k_rst);
      if(nhit_rst != 0){
	Int_t rst = gUnpacker.get(k_device, 0, seg, 0, k_rst);
	hptr_array[ge_rst_id + seg]->Fill(rst);
	hptr_array[ge_rst2d_id]->Fill(seg, rst);
	if(adc >= 0) hptr_array[ge_rst_adc_id + seg]->Fill(rst, adc);
      }
    }
#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//Ge


  // BGO --------------------------------------------------------------
  {
    // data typep
    static const Int_t k_device = gUnpacker.get_device_id("BGO");
    static const Int_t k_tdc    = gUnpacker.get_data_id("BGO","leading");

    // sequential id
    // sequential hist
    static const Int_t bgo_tdc_id = gHist.getSequentialID(kBGO, 0, kTDC);
    static const Int_t bgo_tdc2d_id  = gHist.getSequentialID(kBGO, 0, kTDC2D);
    static const Int_t bgo_hit_id    = gHist.getSequentialID(kBGO, 0, kHitPat);

    for(Int_t seg = 0; seg<NumOfSegBGO; ++seg){

      TH2* htdc2d = dynamic_cast<TH2*>(hptr_array[bgo_tdc2d_id]);

      // TDC
      Int_t nhit_tdc = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      for(Int_t m = 0; m<nhit_tdc; ++m){
	Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
	hptr_array[bgo_tdc_id + seg]->Fill(tdc);
	htdc2d->Fill(seg, tdc);
      }

      // HitPat
      if(nhit_tdc != 0){
	hptr_array[bgo_hit_id]->Fill(seg);
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

 return 0;
} //process_event()

} //analyzer
