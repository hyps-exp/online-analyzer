// -*- C++ -*-

// Author: Tomonori Takahashi
// Change 2017/09/ S.Hoshino

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

#include "Controller.hh"

#include "user_analyzer.hh"
#include "Unpacker.hh"
#include "UnpackerManager.hh"
#include "DAQNode.hh"
#include "filesystem_util.hh"
#include "ConfMan.hh"
#include "HistMaker.hh"
#include "DetectorID.hh"
#include "PsMaker.hh"
#include "GuiPs.hh"
#include "MacroBuilder.hh"
#include "SsdAnalyzer.hh"
#include "UserParamMan.hh"
#include "HodoParamMan.hh"
#include "MatrixParamMan.hh"
#include "MsTParamMan.hh"
#include "ProcInfo.hh"

#define DEBUG    0
#define FLAG_DAQ 1

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  namespace
  {
    std::vector<TH1*> hptr_array;
    bool flag_event_cut = false;
    int event_cut_factor = 1; // for fast semi-online analysis
  }

//____________________________________________________________________________
int
process_begin( const std::vector<std::string>& argv )
{
  ConfMan& gConfMan = ConfMan::getInstance();
  gConfMan.initialize(argv);
  gConfMan.initializeHodoParamMan();
  gConfMan.initializeHodoPHCMan();
  gConfMan.initializeDCGeomMan();
  gConfMan.initializeDCTdcCalibMan();
  gConfMan.initializeDCDriftParamMan();
  gConfMan.initializeMatrixParamMan();
  gConfMan.initializeMsTParamMan();
  gConfMan.initializeUserParamMan();
  if(!gConfMan.isGood()) return -1;
  // unpacker and all the parameter managers are initialized at this stage

  if( argv.size()==4 ){
    int factor = std::strtod( argv[3].c_str(), NULL );
    if( factor!=0 ) event_cut_factor = std::abs( factor );
    flag_event_cut = true;
    std::cout << "#D Event cut flag on : factor="
	      << event_cut_factor << std::endl;
  }

  // Make tabs
  hddaq::gui::Controller& gCon = hddaq::gui::Controller::getInstance();
  TGFileBrowser *tab_hist  = gCon.makeFileBrowser("Hist");
  TGFileBrowser *tab_macro = gCon.makeFileBrowser("Macro");
  TGFileBrowser *tab_e40   = gCon.makeFileBrowser("E40");

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
  tab_macro->Add(macro::Get("dispSFT"));
  tab_macro->Add(macro::Get("dispSDC1"));
  tab_macro->Add(macro::Get("dispSAC"));
  tab_macro->Add(macro::Get("dispSCH"));
  tab_macro->Add(macro::Get("dispFBT1"));
  tab_macro->Add(macro::Get("dispSDC2"));
  tab_macro->Add(macro::Get("dispSDC3"));
  tab_macro->Add(macro::Get("dispFBT2"));
  tab_macro->Add(macro::Get("dispTOF"));
  tab_macro->Add(macro::Get("dispTOF_HT"));
  tab_macro->Add(macro::Get("dispLC"));
  tab_macro->Add(macro::Get("dispHitPat"));
  tab_macro->Add(macro::Get("effBcOut"));
  tab_macro->Add(macro::Get("effSdcInOut"));
  tab_macro->Add(macro::Get("dispDAQ"));
  // tab_macro->Add(macro::Get("auto_monitor_all"));

  // Add histograms to the Hist tab
  HistMaker& gHist = HistMaker::getInstance();
  tab_hist->Add(gHist.createBH1());
  tab_hist->Add(gHist.createBFT());
  tab_hist->Add(gHist.createBC3());
  tab_hist->Add(gHist.createBC4());
  tab_hist->Add(gHist.createBH2());
  tab_hist->Add(gHist.createSFT());
  tab_hist->Add(gHist.createSDC1());
  tab_hist->Add(gHist.createSAC());
  tab_hist->Add(gHist.createSCH());
  tab_hist->Add(gHist.createFBT1());
  tab_hist->Add(gHist.createSDC2());
  tab_hist->Add(gHist.createSDC3());
  tab_hist->Add(gHist.createFBT2());
  tab_hist->Add(gHist.createTOF());
  tab_hist->Add(gHist.createTOF_HT());
  tab_hist->Add(gHist.createLC());
  tab_hist->Add(gHist.createCorrelation());
  tab_hist->Add(gHist.createTriggerFlag());
  tab_hist->Add(gHist.createMsT());
  tab_hist->Add(gHist.createDAQ(false));
  tab_hist->Add(gHist.createTimeStamp(false));
  tab_hist->Add(gHist.createDCEff());

  //e40 tab
  int btof_id = gHist.getUniqueID(kMisc, 0, kTDC);
  tab_e40->Add(gHist.createTH1(btof_id, "BTOF",
                               300, -10, 5,
                               "BTOF [ns]", ""
                               ));

  tab_e40->Add(gHist.createTH1(btof_id + 1, "BH1-6_BH2-4",
                               400, 400, 600,
                               "[ch]", ""
                               ));
  // Matrix pattern
  // Mtx2D
  {
    int mtx2d_id = gHist.getUniqueID(kMisc, kHul2D, kHitPat2D);
    gHist.createTH2(mtx2d_id, "Mtx2D pattern",
                    NumOfSegSCH,   0, NumOfSegSCH,
                    NumOfSegTOF+1, 0, NumOfSegTOF+1,
                    "SCH seg", "TOF seg"
                    );
  }// Mtx2D

  // Mtx3D
  //  {
  //    int mtx3d_id = gHist.getUniqueID(kMisc, kHul3D, kHitPat2D);
  //    for(int i = 0; i<NumOfSegClusteredFBH; ++i){
  //      gHist.createTH2(mtx3d_id+i, Form("Mtx3D pattern_FBH%d",i),
  //                      NumOfSegSCH,   0, NumOfSegSCH,
  //                      NumOfSegTOF+1, 0, NumOfSegTOF+1,
  //                      "SCH seg", "TOF seg"
  //                      );
  //    }// for(i)
  //  }// Mtx3D

  // Set histogram pointers to the vector sequentially.
  // This vector contains both TH1 and TH2.
  // Then you need to do down cast when you use TH2.
  if(0 != gHist.setHistPtr(hptr_array)){ return -1; }

  // Users don't have to touch this section (Make Ps tab),
  // but the file path should be changed.
  // ----------------------------------------------------------
  PsMaker& gPsMaker = PsMaker::getInstance();
  std::vector<std::string> detList;
  std::vector<std::string> optList;
  gHist.getListOfPsFiles(detList);
  gPsMaker.getListOfOption(optList);

  hddaq::gui::GuiPs& gPsTab = hddaq::gui::GuiPs::getInstance();
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
  static UnpackerManager& gUnpacker = GUnpacker::get_instance();
  static HistMaker&       gHist     = HistMaker::getInstance();
//  static MatrixParamMan&  gMatrix   = MatrixParamMan::GetInstance();
//  static MsTParamMan&     gMsT      = MsTParamMan::GetInstance();

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  const int event_number = gUnpacker.get_event_number();
  if( flag_event_cut && event_number%event_cut_factor!=0 )
    return 0;

  // TriggerFlag ---------------------------------------------------
  std::bitset<NumOfSegTFlag> trigger_flag;
  //  bool matrix2d_flag = false;
  //  bool matrix3d_flag = false;
  {
    static const int k_device = gUnpacker.get_device_id("TFlag");
    static const int k_tdc    = gUnpacker.get_data_id("TFlag", "tdc");

    static const int tf_tdc_id = gHist.getSequentialID( kTriggerFlag, 0, kTDC );
    static const int tf_hit_id = gHist.getSequentialID( kTriggerFlag, 0, kHitPat );
    for( int seg=0; seg<NumOfSegTFlag; ++seg ){
      int nhit = gUnpacker.get_entries( k_device, 0, seg, 0, k_tdc );
      if( nhit>0 ){
	int tdc = gUnpacker.get( k_device, 0, seg, 0, k_tdc );
	if( tdc>0 ){
	  trigger_flag.set(seg);
	  hptr_array[tf_tdc_id+seg]->Fill( tdc );
	  hptr_array[tf_hit_id]->Fill( seg );
	  //	  if( seg==8 ) matrix2d_flag = true;
	  //	  if( seg==9 ) matrix3d_flag = true;
	}
      }
    }// for(seg)

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // TimeStamp --------------------------------------------------------
  {
    static const int k_device = gUnpacker.get_device_id("VME-RM");
    static const int k_time   = gUnpacker.get_data_id("VME-RM", "time");
    static const int hist_id  = gHist.getSequentialID(kTimeStamp, 0, kTDC);
    int time0 = 0;
    for( int i=0; i<NumOfVmeRm; ++i ){
      int nhit = gUnpacker.get_entries( k_device, i, 0, 0, k_time );
      if( nhit>0 ){
	int time = gUnpacker.get( k_device, i, 0, 0, k_time );
	if( i==0 ) time0 = time;
	TH1* h = dynamic_cast<TH1*>(hptr_array[hist_id +i]);
	h->Fill( time - time0 );
      }
    }
  }

#if FLAG_DAQ
  // DAQ -------------------------------------------------------------
  {
    // node id
    static const int k_eb      = gUnpacker.get_fe_id("k18eb");
    static const int k_vme     = gUnpacker.get_fe_id("vme01");
    static const int k_clite   = gUnpacker.get_fe_id("clite1");
    static const int k_easiroc = gUnpacker.get_fe_id("easiroc0");

    // sequential id
    static const int eb_id      = gHist.getSequentialID(kDAQ, kEB, kHitPat);
    static const int vme_id     = gHist.getSequentialID(kDAQ, kVME, kHitPat2D);
    static const int clite_id   = gHist.getSequentialID(kDAQ, kCLite, kHitPat2D);
    static const int easiroc_id = gHist.getSequentialID(kDAQ, kEASIROC, kHitPat2D);
//    static const int misc_id    = gHist.getSequentialID(kDAQ, kMiscNode, kHitPat2D);

    { // EB
      int data_size = gUnpacker.get_node_header(k_eb, DAQNode::k_data_size);
      hptr_array[eb_id]->Fill(data_size);
    }

    { // VME node
      TH2* h = dynamic_cast<TH2*>(hptr_array[vme_id]);
      for( int i=0; i<10; ++i ){
	if( i==1 || i==5 ) continue;
	int node_id = k_vme+i;
	int data_size = gUnpacker.get_node_header( node_id, DAQNode::k_data_size);
	h->Fill( i, data_size );
      }
    }

    { // CLite node
      TH2* h = dynamic_cast<TH2*>(hptr_array[clite_id]);
      for(int i = 0; i<14; ++i){
	int data_size = gUnpacker.get_node_header(k_clite+i, DAQNode::k_data_size);
	h->Fill( i, data_size );
      }
    }

    { // EASIROC node
      TH2* h = dynamic_cast<TH2*>(hptr_array[easiroc_id]);
      for(int i = 0; i<20; ++i){
	int data_size = gUnpacker.get_node_header(k_easiroc+i, DAQNode::k_data_size);
	h->Fill( i, data_size );
      }
    }

    { // Misc node
//      TH2* h = dynamic_cast<TH2*>(hptr_array[misc_id]);

    }


  }

#endif

  if( trigger_flag[SpillEndFlag] ) return 0;

  // MsT -----------------------------------------------------------
  {
    static const int k_device = gUnpacker.get_device_id("MsT");
    static const int k_tof    = gUnpacker.get_plane_id("MsT", "TOF");
    static const int k_sch    = gUnpacker.get_plane_id("MsT", "SCH");
    static const int k_tag    = gUnpacker.get_plane_id("MsT", "tag");
    static const int k_ch     = 0;
    static const int k_tdc    = 0;
    static const int k_n_flag = 7;

    static const int toft_id     = gHist.getSequentialID(kMsT, 0, kTDC);
    static const int scht_id     = gHist.getSequentialID(kMsT, 0, kTDC, NumOfSegTOF*2 +1);
    static const int toft_2d_id  = gHist.getSequentialID(kMsT, 0, kTDC2D);

    static const int tofhit_id   = gHist.getSequentialID(kMsT, 0, kHitPat, 0);
    static const int schhit_id   = gHist.getSequentialID(kMsT, 0, kHitPat, 1);
    static const int flag_id     = gHist.getSequentialID(kMsT, 0, kHitPat, 2);

    // TOF
    for(int seg=0; seg<NumOfSegTOF; ++seg){
      int nhit = gUnpacker.get_entries(k_device, k_tof, seg, k_ch, k_tdc);
      for(int m = 0; m<nhit; ++m){
	hptr_array[tofhit_id]->Fill(seg);

	unsigned int tdc = gUnpacker.get(k_device, k_tof, seg, k_ch, k_tdc, m);
	if(tdc!=0){
	  hptr_array[toft_id + seg]->Fill(tdc);
	  hptr_array[toft_2d_id]->Fill(seg, tdc);
	}
      }
    }// TOF

    // SCH
    for(int seg=0; seg<NumOfSegSCH; ++seg){
      int nhit = gUnpacker.get_entries(k_device, k_sch, seg, k_ch, k_tdc);
      if(nhit!=0){
	hptr_array[schhit_id]->Fill(seg);
      }
      for(int m = 0; m<nhit; ++m){
	unsigned int tdc = gUnpacker.get(k_device, k_sch, seg, k_ch, k_tdc, m);
	if(tdc!=0){
	  hptr_array[scht_id + seg]->Fill(tdc);
	}
      }
    }// SCH

    // FLAG
    for(int i=0; i<k_n_flag; ++i){
      int nhit = gUnpacker.get_entries(k_device, k_tag, 0, k_ch, i);
      if(nhit!=0){
	int flag = gUnpacker.get(k_device, k_tag, 0, k_ch, i, 0);
	if(flag) hptr_array[flag_id]->Fill(i);
      }
    }// FLAG
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BH1 -----------------------------------------------------------
  {
    // data type
    static const int k_device   = gUnpacker.get_device_id("BH1");
    static const int k_u        = 0; // up
    static const int k_d        = 1; // down
    static const int k_adc      = gUnpacker.get_data_id("BH1", "adc");
    static const int k_tdc      = gUnpacker.get_data_id("BH1", "tdc");

    // TDC gate range
    static const unsigned int tdc_min
      = UserParamMan::getInstance().getParameter("BH1_TDC", 0);
    static const unsigned int tdc_max
      = UserParamMan::getInstance().getParameter("BH1_TDC", 1);

    // Up PMT
    int bh1a_id   = gHist.getSequentialID(kBH1, 0, kADC);
    int bh1t_id   = gHist.getSequentialID(kBH1, 0, kTDC);
    int bh1awt_id = gHist.getSequentialID(kBH1, 0, kADCwTDC);
    for(int seg=0; seg<NumOfSegBH1; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc);
      if(nhit!=0){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	hptr_array[bh1a_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      if(nhit!=0){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	if(tdc!=0){
	  hptr_array[bh1t_id + seg]->Fill(tdc);
	  // ADC wTDC
	  if( gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc)>0 ){
	    unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	    hptr_array[bh1awt_id + seg]->Fill( adc );
	  }
	}
      }
    }

    // Down PMT
    bh1a_id   = gHist.getSequentialID(kBH1, 0, kADC, NumOfSegBH1+1);
    bh1t_id   = gHist.getSequentialID(kBH1, 0, kTDC, NumOfSegBH1+1);
    bh1awt_id = gHist.getSequentialID(kBH1, 0, kADCwTDC, NumOfSegBH1+1);
    for(int seg=0; seg<NumOfSegBH1; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc);
      if(nhit!=0){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	hptr_array[bh1a_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      if(nhit!=0){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
	if( tdc!=0 ){
	  hptr_array[bh1t_id + seg]->Fill(tdc);
	  // ADC w/TDC
	  if( gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc)>0 ){
	    unsigned int adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	    hptr_array[bh1awt_id + seg]->Fill(adc);
	  }
	}
      }
    }

    // Hit pattern && multiplicity
    static const int bh1hit_id = gHist.getSequentialID(kBH1, 0, kHitPat);
    static const int bh1mul_id = gHist.getSequentialID(kBH1, 0, kMulti);
    int multiplicity  = 0;
    int cmultiplicity = 0;
    for(int seg=0; seg<NumOfSegBH1; ++seg){
      int nhit_bh1u = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      int nhit_bh1d = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      // AND
      if(nhit_bh1u!=0 && nhit_bh1d!=0){
	unsigned int tdc_u = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	unsigned int tdc_d = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
	// TDC AND
	if(tdc_u != 0 && tdc_d != 0){
	  hptr_array[bh1hit_id]->Fill(seg);
	  ++multiplicity;
	  // TDC range
	  if(true
	     && tdc_min < tdc_u && tdc_u < tdc_max
	     && tdc_min < tdc_d && tdc_d < tdc_max
	     ){
	    hptr_array[bh1hit_id+1]->Fill(seg); // CHitPat
	    ++cmultiplicity;
	  }// TDC range OK
	}// TDC AND
      }// AND
    }// for(seg)

    hptr_array[bh1mul_id]->Fill(multiplicity);
    hptr_array[bh1mul_id+1]->Fill(cmultiplicity); // CMulti

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }// BH1

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BH1 (FPGA) -------------------------------------------------------
  {
    // data type
    static const int k_device   = gUnpacker.get_device_id("BH1");
    static const int k_u        = 0; // up
    static const int k_d        = 1; // down
    static const int k_tdc      = gUnpacker.get_data_id("BH1", "fpga_leading");

    // Up PMT
    int bh1t_id   = gHist.getSequentialID(kBH1, 0, kTDC, NumOfSegBH1*2+1);
    for(int seg=0; seg<NumOfSegBH1; ++seg){
      // TDC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      for(int m = 0; m<nhit; ++m){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc, m);
	if(tdc!=0){
	  hptr_array[bh1t_id + seg]->Fill(tdc);
	}
      }
    }

    // Down PMT
    bh1t_id   = gHist.getSequentialID(kBH1, 0, kTDC, NumOfSegBH1*2 + NumOfSegBH1+1);
    for(int seg=0; seg<NumOfSegBH1; ++seg){
      // TDC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      for(int m = 0; m<nhit; ++m){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc, m);
	if( tdc!=0 ){
	  hptr_array[bh1t_id + seg]->Fill(tdc);
	}
      }
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }// BH1(FPGA)

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BFT -----------------------------------------------------------
  {
    // data type
    static const int k_device  = gUnpacker.get_device_id("BFT");
    static const int k_uplane  = gUnpacker.get_plane_id("BFT", "upstream");
    static const int k_dplane  = gUnpacker.get_plane_id("BFT", "downstream");
    static const int k_leading = gUnpacker.get_data_id("BFT", "leading");
    static const int k_trailing = gUnpacker.get_data_id("BFT", "trailing");

    // TDC gate range
    UserParamMan& gPar = UserParamMan::getInstance();
    static const int tdc_min = gPar.getParameter("BFT_TDC", 0);
    static const int tdc_max = gPar.getParameter("BFT_TDC", 1);

    // sequential id
    static const int bft_tu_id    = gHist.getSequentialID(kBFT, 0, kTDC,      1);
    static const int bft_td_id    = gHist.getSequentialID(kBFT, 0, kTDC,      2);
    static const int bft_ctu_id   = gHist.getSequentialID(kBFT, 0, kTDC,     11);
    static const int bft_ctd_id   = gHist.getSequentialID(kBFT, 0, kTDC,     12);
    static const int bft_totu_id  = gHist.getSequentialID(kBFT, 0, kADC,      1);
    static const int bft_totd_id  = gHist.getSequentialID(kBFT, 0, kADC,      2);
    static const int bft_ctotu_id = gHist.getSequentialID(kBFT, 0, kADC,     11);
    static const int bft_ctotd_id = gHist.getSequentialID(kBFT, 0, kADC,     12);
    static const int bft_hitu_id  = gHist.getSequentialID(kBFT, 0, kHitPat,   1);
    static const int bft_hitd_id  = gHist.getSequentialID(kBFT, 0, kHitPat,   2);
    static const int bft_chitu_id = gHist.getSequentialID(kBFT, 0, kHitPat,  11);
    static const int bft_chitd_id = gHist.getSequentialID(kBFT, 0, kHitPat,  12);
    static const int bft_mul_id   = gHist.getSequentialID(kBFT, 0, kMulti,    1);
    static const int bft_cmul_id  = gHist.getSequentialID(kBFT, 0, kMulti,   11);

    static const int bft_ctu_2d_id = gHist.getSequentialID(kBFT, 0, kTDC2D,   1);
    static const int bft_ctd_2d_id = gHist.getSequentialID(kBFT, 0, kTDC2D,   2);
    static const int bft_ctotu_2d_id = gHist.getSequentialID(kBFT, 0, kADC2D, 1);
    static const int bft_ctotd_2d_id = gHist.getSequentialID(kBFT, 0, kADC2D, 2);

    int multiplicity  = 0; // includes both u and d planes.
    int cmultiplicity = 0; // includes both u and d planes.
    int tdc_prev      = 0;
    for(int i = 0; i<NumOfSegBFT; ++i){
      int nhit_u = gUnpacker.get_entries(k_device, k_uplane, 0, i, k_leading);
      int nhit_d = gUnpacker.get_entries(k_device, k_dplane, 0, i, k_leading);

      // u plane
      tdc_prev = 0;
      for(int m = 0; m<nhit_u; ++m){
	int tdc = gUnpacker.get(k_device, k_uplane, 0, i, k_leading, m);
	int tdc_t = gUnpacker.get(k_device, k_uplane, 0, i, k_trailing, m);
	int tot = tdc - tdc_t;
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
      for(int m = 0; m<nhit_d; ++m){
	int tdc = gUnpacker.get(k_device, k_dplane, 0, i, k_leading, m);
	int tdc_t = gUnpacker.get(k_device, k_dplane, 0, i, k_trailing, m);
	int tot = tdc - tdc_t;
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
  {
    // data type
    static const int k_device  = gUnpacker.get_device_id("BC3");
    static const int k_tdc     = 0;

    // TDC gate range
    static const int tdc_min
      = UserParamMan::getInstance().getParameter("BC3_TDC", 0);
    static const int tdc_max
      = UserParamMan::getInstance().getParameter("BC3_TDC", 1);

    // sequential id
    static const int bc3t_id    = gHist.getSequentialID(kBC3, 0, kTDC);
    static const int bc3t1st_id = gHist.getSequentialID(kBC3, 0, kTDC2D);
    static const int bc3hit_id  = gHist.getSequentialID(kBC3, 0, kHitPat);
    static const int bc3mul_id  = gHist.getSequentialID(kBC3, 0, kMulti);
    static const int bc3mulwt_id
      = gHist.getSequentialID(kBC3, 0, kMulti, 1+NumOfLayersBC3);

    // TDC & HitPat & Multi
    for(int l = 0; l<NumOfLayersBC3; ++l){

      int multiplicity    = 0;
      int multiplicity_wt = 0;
      for(int w = 0; w<NumOfWireBC3; ++w){
	int nhit = gUnpacker.get_entries(k_device, l, 0, w, k_tdc);
	if(nhit == 0){continue;}

	// This wire fired at least one times.
	++multiplicity;
//	hptr_array[bc3hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	int  tdc1st = 0;
	for(int m = 0; m<nhit; ++m){
       	  hptr_array[bc3hit_id + l]->Fill(w);
	  int tdc = gUnpacker.get(k_device, l, 0, w, k_tdc, m);
	  hptr_array[bc3t_id + l]->Fill(tdc);
	  if( tdc1st<tdc ) tdc1st = tdc;
	  // Drift time check
	  if(tdc_min < tdc && tdc < tdc_max){
	    flag_hit_wt = true;
	  }
	}

	if( tdc1st!=0 ) hptr_array[bc3t1st_id + l]->Fill(tdc1st);
	if(flag_hit_wt){ ++multiplicity_wt; }
      }

      hptr_array[bc3mul_id + l]->Fill(multiplicity);
      hptr_array[bc3mulwt_id + l]->Fill(multiplicity_wt);
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
  {
    // data type
    static const int k_device  = gUnpacker.get_device_id("BC4");
    static const int k_tdc = 0;

    // TDC gate range
    static const int tdc_min
      = UserParamMan::getInstance().getParameter("BC4_TDC", 0);
    static const int tdc_max
      = UserParamMan::getInstance().getParameter("BC4_TDC", 1);

    // sequential id
    static const int bc4t_id    = gHist.getSequentialID(kBC4, 0, kTDC);
    static const int bc4t1st_id = gHist.getSequentialID(kBC4, 0, kTDC2D);
    static const int bc4hit_id  = gHist.getSequentialID(kBC4, 0, kHitPat);
    static const int bc4mul_id  = gHist.getSequentialID(kBC4, 0, kMulti);
    static const int bc4mulwt_id
      = gHist.getSequentialID(kBC4, 0, kMulti, 1+NumOfLayersBC4);

    // TDC & HitPat & Multi
    for(int l = 0; l<NumOfLayersBC4; ++l){

      int multiplicity    = 0;
      int multiplicity_wt = 0;
      for(int w = 0; w<NumOfWireBC4; ++w){
	int nhit = gUnpacker.get_entries(k_device, l, 0, w, k_tdc);
	if(nhit == 0){continue;}

	// This wire fired at least one times.
	++multiplicity;
//	hptr_array[bc4hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	int  tdc1st = 0;
	for(int m = 0; m<nhit; ++m){
	  hptr_array[bc4hit_id + l]->Fill(w);
	  int tdc = gUnpacker.get(k_device, l, 0, w, k_tdc, m);
	  hptr_array[bc4t_id + l]->Fill(tdc);
	  if( tdc1st<tdc ) tdc1st = tdc;
	  // Drift time check
	  if(tdc_min < tdc && tdc < tdc_max){
	    flag_hit_wt = true;
	  }
	}

	if( tdc1st!=0 ) hptr_array[bc4t1st_id + l]->Fill(tdc1st);
	if(flag_hit_wt){ ++multiplicity_wt; }
      }

      hptr_array[bc4mul_id + l]->Fill(multiplicity);
      hptr_array[bc4mulwt_id + l]->Fill(multiplicity_wt);
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
    static const int k_device = gUnpacker.get_device_id("BH2");
    static const int k_u      = 0; // up
    static const int k_d      = 1; // down
    static const int k_adc    = gUnpacker.get_data_id("BH2", "adc");
    static const int k_tdc    = gUnpacker.get_data_id("BH2", "tdc");

    // TDC gate range
    static const unsigned int tdc_min
      = UserParamMan::getInstance().getParameter("BH2_TDC", 0);
    static const unsigned int tdc_max
      = UserParamMan::getInstance().getParameter("BH2_TDC", 1);

    // UP
    int bh2a_id   = gHist.getSequentialID(kBH2, 0, kADC);
    int bh2t_id   = gHist.getSequentialID(kBH2, 0, kTDC);
    int bh2awt_id = gHist.getSequentialID(kBH2, 0, kADCwTDC);
    for(int seg=0; seg<NumOfSegBH2; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc);
      if(nhit != 0){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	hptr_array[bh2a_id + seg]->Fill(adc);
      }
      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      if( nhit!=0 ){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	if( tdc!=0 ){
	  hptr_array[bh2t_id + seg]->Fill(tdc);
	  // ADC w/TDC
	  if( tdc_min < tdc && tdc < tdc_max ){
	    unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	    hptr_array[bh2awt_id + seg]->Fill(adc);
	  }
	}
      }
    }

    // DOWN
    bh2a_id   = gHist.getSequentialID(kBH2, 0, kADC, NumOfSegBH2+1);
    bh2t_id   = gHist.getSequentialID(kBH2, 0, kTDC, NumOfSegBH2+1);
    bh2awt_id = gHist.getSequentialID(kBH2, 0, kADCwTDC, NumOfSegBH2+1);
    for(int seg=0; seg<NumOfSegBH2; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc);
      if(nhit != 0){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	hptr_array[bh2a_id + seg]->Fill(adc);
      }
      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      if( nhit!=0 ){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
	if( tdc!=0 ){
	  hptr_array[bh2t_id + seg]->Fill(tdc);
	  // ADC w/TDC
	  if( tdc_min < tdc && tdc < tdc_max ){
	    unsigned int adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	    hptr_array[bh2awt_id + seg]->Fill( adc );
	  }
	}
      }
    }

    // Hit pattern &&  Multiplicity
    static const int bh2hit_id = gHist.getSequentialID(kBH2, 0, kHitPat);
    static const int bh2mul_id = gHist.getSequentialID(kBH2, 0, kMulti);
    int multiplicity = 0;
    for(int seg=0; seg<NumOfSegBH2; ++seg){
      int nhit_u = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      int nhit_d = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      // AND
      if( nhit_u!=0 && nhit_d!=0 ){
	unsigned int tdc_u = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	unsigned int tdc_d = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
	// TDC AND
	if( tdc_u!=0 && tdc_d!=0 ){
	  hptr_array[bh2hit_id]->Fill( seg );
	  ++multiplicity;
	}
      }
    }

    hptr_array[bh2mul_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BH2 (FPGA) -------------------------------------------------------
  {
    // data type
    static const int k_device   = gUnpacker.get_device_id("BH2");
    static const int k_u        = 0; // up
    static const int k_d        = 1; // down
    static const int k_tdc      = gUnpacker.get_data_id("BH2", "fpga_leading");

    // Up PMT
    int bh1t_id   = gHist.getSequentialID(kBH2, 0, kTDC, NumOfSegBH2*2+1);
    for(int seg=0; seg<NumOfSegBH2; ++seg){
      // TDC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      for(int m = 0; m<nhit; ++m){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc, m);
	if(tdc!=0){
	  hptr_array[bh1t_id + seg]->Fill(tdc);
	}
      }
    }

    // Down PMT
    bh1t_id   = gHist.getSequentialID(kBH2, 0, kTDC, NumOfSegBH2*2 + NumOfSegBH2+1);
    for(int seg=0; seg<NumOfSegBH2; ++seg){
      // TDC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      for(int m = 0; m<nhit; ++m){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc, m);
	if( tdc!=0 ){
	  hptr_array[bh1t_id + seg]->Fill(tdc);
	}
      }
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }// BH2(FPGA)

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif


  // SCH -----------------------------------------------------------
  {
    // data type
    static const int k_device   = gUnpacker.get_device_id("SCH");
    static const int k_leading  = gUnpacker.get_data_id("SCH", "leading");
    static const int k_trailing = gUnpacker.get_data_id("SCH", "trailing");

    // TDC gate range
    UserParamMan& gPar = UserParamMan::getInstance();
    static const int tdc_min = gPar.getParameter("SCH_TDC", 0);
    static const int tdc_max = gPar.getParameter("SCH_TDC", 1);

    // sequential id
    static const int sch_t_id    = gHist.getSequentialID(kSCH, 0, kTDC,      1);
    static const int sch_tot_id  = gHist.getSequentialID(kSCH, 0, kADC,      1);
    static const int sch_t_all_id   = gHist.getSequentialID(kSCH, 0, kTDC, NumOfSegSCH+1);
    static const int sch_tot_all_id = gHist.getSequentialID(kSCH, 0, kADC, NumOfSegSCH+1);
    static const int sch_hit_id  = gHist.getSequentialID(kSCH, 0, kHitPat,   1);
    static const int sch_mul_id  = gHist.getSequentialID(kSCH, 0, kMulti,    1);

    static const int sch_t_2d_id   = gHist.getSequentialID(kSCH, 0, kTDC2D,  1);
    static const int sch_tot_2d_id = gHist.getSequentialID(kSCH, 0, kADC2D,  1);

    int multiplicity  = 0;
    for( int i=0; i<NumOfSegSCH; ++i ){
      int nhit = gUnpacker.get_entries(k_device, 0, i, 0, k_leading);

      for(int m = 0; m<nhit; ++m){
	int tdc      = gUnpacker.get(k_device, 0, i, 0, k_leading,  m);
	int trailing = gUnpacker.get(k_device, 0, i, 0, k_trailing, m);
	int tot      = tdc - trailing;
	hptr_array[sch_t_id +i]->Fill(tdc);
	hptr_array[sch_t_all_id]->Fill(tdc);
	hptr_array[sch_tot_id +i]->Fill(tot);
	hptr_array[sch_tot_all_id]->Fill(tot);
	hptr_array[sch_t_2d_id]->Fill(i, tdc);
	hptr_array[sch_tot_2d_id]->Fill(i, tot);
	if( tdc_min<tdc && tdc<tdc_max ){
	  ++multiplicity;
	  hptr_array[sch_hit_id]->Fill(i);
	}
      }
    }
    hptr_array[sch_mul_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // SDC1 ------------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("SDC1");
    static const int k_tdc    = 0;

    // TDC gate range
    static const int tdc_min
      = UserParamMan::getInstance().getParameter("SDC1_TDC", 0);
    static const int tdc_max
      = UserParamMan::getInstance().getParameter("SDC1_TDC", 1);

    // sequential id
    static const int sdc1t_id    = gHist.getSequentialID(kSDC1, 0, kTDC, 1);
    static const int sdc1t1st_id = gHist.getSequentialID(kSDC1, 0, kTDC2D, 1);
    static const int sdc1hit_id  = gHist.getSequentialID(kSDC1, 0, kHitPat, 1);
    static const int sdc1mul_id  = gHist.getSequentialID(kSDC1, 0, kMulti, 1);
    static const int sdc1mulwt_id
      = gHist.getSequentialID(kSDC1, 0, kMulti, 1+NumOfLayersSDC1);

    // TDC & HitPat & Multi
    for(int l = 0; l<NumOfLayersSDC1; ++l){
      int multiplicity    = 0;
      int multiplicity_wt = 0;
      for(int w = 0; w<NumOfWireSDC1; ++w){
	int nhit = gUnpacker.get_entries(k_device, l, 0, w, k_tdc);
	if( nhit==0 ) continue;

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[sdc1hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	int  tdc1st = 0;
	for( int m=0; m<nhit; ++m ){
	  int tdc = gUnpacker.get(k_device, l, 0, w, k_tdc, m);
	  hptr_array[sdc1t_id + l]->Fill(tdc);
	  if( tdc1st<tdc ) tdc1st = tdc;

	  // Drift time check
	  if( tdc_min<tdc && tdc<tdc_max ){
	    flag_hit_wt = true;
	  }
	}

	if( tdc1st!=0 ) hptr_array[sdc1t1st_id + l]->Fill(tdc1st);
	if( flag_hit_wt ){
	  ++multiplicity_wt;
	  hptr_array[sdc1hit_id + l]->Fill( w );
	}
      }

      hptr_array[sdc1mul_id + l]->Fill(multiplicity);
      hptr_array[sdc1mulwt_id + l]->Fill(multiplicity_wt);
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device,0);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

//  // SDC2 ------------------------------------------------------------
//  {
//    // data type
//    static const int k_device = gUnpacker.get_device_id("SDC2");
//    static const int k_tdc    = 0;
//
//    // TDC gate range
//    static const int tdc_min
//      = UserParamMan::getInstance().getParameter("SDC2_TDC", 0);
//    static const int tdc_max
//      = UserParamMan::getInstance().getParameter("SDC2_TDC", 1);
//
//    // sequential id
//    static const int sdc2t_id    = gHist.getSequentialID(kSDC2, 0, kTDC);
//    static const int sdc2t1st_id = gHist.getSequentialID(kSDC2, 0, kTDC2D);
//    static const int sdc2hit_id  = gHist.getSequentialID(kSDC2, 0, kHitPat);
//    static const int sdc2mul_id  = gHist.getSequentialID(kSDC2, 0, kMulti);
//    static const int sdc2mulwt_id
//      = gHist.getSequentialID(kSDC2, 0, kMulti, 1+NumOfLayersSDC2);
//
//    // TDC & HitPat & Multi
//    for(int l=0; l<NumOfLayersSDC2; ++l){
//      int multiplicity    = 0;
//      int multiplicity_wt = 0;
//      for( int w=0; w<NumOfWireSDC2; ++w ){
//	int nhit = gUnpacker.get_entries(k_device, l, 0, w, k_tdc);
//	if( nhit == 0 ) continue;
//
//	// This wire fired at least one times.
//	++multiplicity;
//	// hptr_array[sdc2hit_id + l]->Fill(w, nhit);
//
//	bool flag_hit_wt = false;
//	int  tdc1st = 0;
//	for( int m = 0; m<nhit; ++m ){
//	  int tdc = gUnpacker.get(k_device, l, 0, w, k_tdc, m);
//	  hptr_array[sdc2t_id + l]->Fill(tdc);
//	  if( tdc1st<tdc ) tdc1st = tdc;
//
//	  // Drift time check
//	  if( tdc_min < tdc && tdc < tdc_max ){
//	    flag_hit_wt = true;
//	  }
//	}
//
//	if( tdc1st!=0 ) hptr_array[sdc2t1st_id +l]->Fill( tdc1st );
//	if( flag_hit_wt ){
//	  ++multiplicity_wt;
//	  hptr_array[sdc2hit_id + l]->Fill( w );
//	}
//      }
//
//      hptr_array[sdc2mul_id + l]->Fill(multiplicity);
//      hptr_array[sdc2mulwt_id + l]->Fill(multiplicity_wt);
//    }
//
//#if 0
//    // Debug, dump data relating this detector
//    gUnpacker.dump_data_device(k_device);
//#endif
//  }
//
//#if DEBUG
//  std::cout << __FILE__ << " " << __LINE__ << std::endl;
//#endif

//  // SDC3 -------------------------------------------------------------
//  {
//    // data type
//    static const int k_device = gUnpacker.get_device_id("SDC3");
//    static const int k_tdc    = 0;
//
//    // TDC gate range
//    static const int tdc_min
//      = UserParamMan::getInstance().getParameter("SDC3_TDC", 0);
//    static const int tdc_max
//      = UserParamMan::getInstance().getParameter("SDC3_TDC", 1);
//
//    // sequential id
//    static const int sdc3t_id    = gHist.getSequentialID(kSDC3, 0, kTDC);
//    static const int sdc3t1st_id = gHist.getSequentialID(kSDC3, 0, kTDC2D);
//    static const int sdc3hit_id  = gHist.getSequentialID(kSDC3, 0, kHitPat);
//    static const int sdc3mul_id  = gHist.getSequentialID(kSDC3, 0, kMulti);
//    static const int sdc3mulwt_id
//      = gHist.getSequentialID(kSDC3, 0, kMulti, 1+NumOfLayersSDC3);
//
//    // TDC & HitPat & Multi
//    for(int l=0; l<NumOfLayersSDC3; ++l){
//      int multiplicity    = 0;
//      int multiplicity_wt = 0;
//      int sdc3_nwire = 0;
//      if( l==0 || l==1 )
//	sdc3_nwire = NumOfWireSDC3Y;
//      if( l==2 || l==3 )
//	sdc3_nwire = NumOfWireSDC3X;
//
//      for( int w=0 ; w<sdc3_nwire; ++w ){
//	int nhit = gUnpacker.get_entries(k_device, l, 0, w, k_tdc);
//	if( nhit == 0 ) continue;
//	//	std::cout << "w " << w << ", nhit " << nhit  << std::endl;
//
//	// This wire fired at least one times.
//	++multiplicity;
//	// hptr_array[sdc3hit_id + l]->Fill(w, nhit);
//
//	bool flag_hit_wt = false;
//	int  tdc1st = 0;
//	for( int m = 0; m<nhit; ++m ){
//	  int tdc = gUnpacker.get(k_device, l, 0, w, k_tdc, m);
//	  hptr_array[sdc3t_id + l]->Fill(tdc);
//	  if( tdc1st<tdc ) tdc1st = tdc;
//
//	  // Drift time check
//	  if( tdc_min < tdc && tdc < tdc_max ){
//	    flag_hit_wt = true;
//	  }
//	}
//
//	if( tdc1st!=0 ) hptr_array[sdc3t1st_id +l]->Fill( tdc1st );
//	if( flag_hit_wt ){
//	  ++multiplicity_wt;
//	  hptr_array[sdc3hit_id + l]->Fill( w );
//	}
//      }
//
//      hptr_array[sdc3mul_id +l]->Fill( multiplicity );
//      hptr_array[sdc3mulwt_id +l]->Fill( multiplicity_wt );
//    }
//
//#if 0
//    // Debug, dump data relating this detector
//    gUnpacker.dump_data_device(k_device);
//#endif
//  }
//
//#if DEBUG
//  std::cout << __FILE__ << " " << __LINE__ << std::endl;
//#endif

  //------------------------------------------------------------------
  // SDC2
  //------------------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("SDC2");
    static const int k_leading = gUnpacker.get_data_id("SDC2", "leading");
    static const int k_trailing = gUnpacker.get_data_id("SDC2", "trailing");

    // TDC gate range
    static const int tdc_min
      = UserParamMan::getInstance().getParameter("SDC2_TDC", 0);
    static const int tdc_max
      = UserParamMan::getInstance().getParameter("SDC2_TDC", 1);
    // TOT gate range
    static const int tot_min
      = UserParamMan::getInstance().getParameter("SDC2_TOT", 0);
//    static const int tot_max
//      = UserParamMan::getInstance().getParameter("SDC2_TOT", 1);


    // sequential id
    static const int sdc2t_id    = gHist.getSequentialID(kSDC2, 0, kTDC);
    static const int sdc2tot_id  = gHist.getSequentialID(kSDC2, 0, kADC);
    static const int sdc2t1st_id = gHist.getSequentialID(kSDC2, 0, kTDC2D);
    static const int sdc2hit_id  = gHist.getSequentialID(kSDC2, 0, kHitPat);
    static const int sdc2mul_id  = gHist.getSequentialID(kSDC2, 0, kMulti);
    static const int sdc2mulwt_id
      = gHist.getSequentialID(kSDC2, 0, kMulti, 1+NumOfLayersSDC2);

    static const int sdc2t_ctot_id    = gHist.getSequentialID(kSDC2, 0, kTDC, 11);
    static const int sdc2tot_ctot_id  = gHist.getSequentialID(kSDC2, 0, kADC, 11);
    static const int sdc2t1st_ctot_id = gHist.getSequentialID(kSDC2, 0, kTDC2D, 11);
    static const int sdc2hit_ctot_id  = gHist.getSequentialID(kSDC2, 0, kHitPat, 11);
    static const int sdc2mul_ctot_id  = gHist.getSequentialID(kSDC2, 0, kMulti, 11);
    static const int sdc2mulwt_ctot_id
      = gHist.getSequentialID(kSDC2, 0, kMulti, 1+NumOfLayersSDC2 + 10);

    // TDC & HitPat & Multi
    for(int l=0; l<NumOfLayersSDC2; ++l){
      int tdc = 0; 
      int tdc_t = 0; 
      int tot = 0; 
      int tdc1st = 0;
      int multiplicity    = 0;
      int multiplicity_wt = 0;
      int multiplicity_ctot    = 0;
      int multiplicity_wt_ctot = 0;
      for( int w=0; w<NumOfWireSDC2; ++w ){
	int nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	int nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if( nhit_l == 0 ) continue;

        int hit_l_max = 0;
        int hit_t_max = 0;

        if(nhit_l != 0){
          hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
        }
        if(nhit_t != 0){
          hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
        }

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[sdc2hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	bool flag_hit_wt_ctot = false;
	for( int m = 0; m<nhit_l; ++m ){
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
          for(int m = 0; m<nhit_l; ++m){
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

        }
      }

      hptr_array[sdc2mul_id + l]->Fill(multiplicity);
      hptr_array[sdc2mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[sdc2mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[sdc2mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
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
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("SDC3");
    static const int k_leading = gUnpacker.get_data_id("SDC3", "leading");
    static const int k_trailing = gUnpacker.get_data_id("SDC3", "trailing");

    // TDC gate range
    static const int tdc_min
      = UserParamMan::getInstance().getParameter("SDC3_TDC", 0);
    static const int tdc_max
      = UserParamMan::getInstance().getParameter("SDC3_TDC", 1);
    // TOT gate range
    static const int tot_min
      = UserParamMan::getInstance().getParameter("SDC3_TOT", 0);
    static const int tot_max
      = UserParamMan::getInstance().getParameter("SDC3_TOT", 1);


    // sequential id
    static const int sdc3t_id    = gHist.getSequentialID(kSDC3, 0, kTDC);
    static const int sdc3tot_id  = gHist.getSequentialID(kSDC3, 0, kADC);
    static const int sdc3t1st_id = gHist.getSequentialID(kSDC3, 0, kTDC2D);
    static const int sdc3hit_id  = gHist.getSequentialID(kSDC3, 0, kHitPat);
    static const int sdc3mul_id  = gHist.getSequentialID(kSDC3, 0, kMulti);
    static const int sdc3mulwt_id
      = gHist.getSequentialID(kSDC3, 0, kMulti, 1+NumOfLayersSDC3);

    static const int sdc3t_ctot_id    = gHist.getSequentialID(kSDC3, 0, kTDC, 11);
    static const int sdc3tot_ctot_id  = gHist.getSequentialID(kSDC3, 0, kADC, 11);
    static const int sdc3t1st_ctot_id = gHist.getSequentialID(kSDC3, 0, kTDC2D, 11);
    static const int sdc3hit_ctot_id  = gHist.getSequentialID(kSDC3, 0, kHitPat, 11);
    static const int sdc3mul_ctot_id  = gHist.getSequentialID(kSDC3, 0, kMulti, 11);
    static const int sdc3mulwt_ctot_id
      = gHist.getSequentialID(kSDC3, 0, kMulti, 1+NumOfLayersSDC3 + 10);

    // TDC & HitPat & Multi
    for(int l=0; l<NumOfLayersSDC3; ++l){
      int tdc = 0; 
      int tdc_t = 0; 
      int tot = 0; 
      int tdc1st = 0;
      int multiplicity    = 0;
      int multiplicity_wt = 0;
      int multiplicity_ctot    = 0;
      int multiplicity_wt_ctot = 0;
      int sdc3_nwire = 0;
      if( l==0 || l==1 )
	sdc3_nwire = NumOfWireSDC3Y;
      if( l==2 || l==3 )
	sdc3_nwire = NumOfWireSDC3X;

      for( int w=0; w<sdc3_nwire; ++w ){
	int nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	int nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if( nhit_l == 0 ) continue;

        int hit_l_max = 0;
        int hit_t_max = 0;

        if(nhit_l != 0){
          hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
        }
        if(nhit_t != 0){
          hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
        }

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[sdc3hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	bool flag_hit_wt_ctot = false;
	for( int m = 0; m<nhit_l; ++m ){
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
          for(int m = 0; m<nhit_l; ++m){
            tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
            tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
            tot = tdc - tdc_t;
            hptr_array[sdc3tot_id+l]->Fill(tot);
            if(tot < tot_min || tot >tot_max) continue;
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

        }
      }

      hptr_array[sdc3mul_id + l]->Fill(multiplicity);
      hptr_array[sdc3mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[sdc3mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[sdc3mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
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
    static const int k_device = gUnpacker.get_device_id("TOF");
    static const int k_u      = 0; // up
    static const int k_d      = 1; // down
    static const int k_adc    = gUnpacker.get_data_id("TOF","adc");
    static const int k_tdc    = gUnpacker.get_data_id("TOF","tdc");

    // sequential id
    int tofa_id   = gHist.getSequentialID(kTOF, 0, kADC);
    int toft_id   = gHist.getSequentialID(kTOF, 0, kTDC);
    int tofawt_id = gHist.getSequentialID(kTOF, 0, kADCwTDC);
    for(int seg = 0; seg<NumOfSegTOF; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc);
      if( nhit!=0 ){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	hptr_array[tofa_id + seg]->Fill( adc );
      }
      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      if(nhit != 0){
	int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	if( tdc!=0 ){
	  hptr_array[toft_id + seg]->Fill(tdc);
	  // ADC w/TDC
	  if( gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc)>0 ){
	    unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	    hptr_array[tofawt_id + seg]->Fill( adc );
	  }
	}
      }
    }

    // Down PMT
    tofa_id   = gHist.getSequentialID(kTOF, 0, kADC, NumOfSegTOF+1);
    toft_id   = gHist.getSequentialID(kTOF, 0, kTDC, NumOfSegTOF+1);
    tofawt_id = gHist.getSequentialID(kTOF, 0, kADCwTDC, NumOfSegTOF+1);

    for(int seg = 0; seg<NumOfSegTOF; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc);
      if(nhit != 0){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	hptr_array[tofa_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      if(nhit != 0){
	int tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
	if(tdc != 0){
	  hptr_array[toft_id + seg]->Fill(tdc);
	  // ADC w/TDC
	  if( gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc)>0 ){
	    unsigned int adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	    hptr_array[tofawt_id + seg]->Fill( adc );
	  }
	}
      }
    }

    // Hit pattern && multiplicity
    static const int tofhit_id = gHist.getSequentialID(kTOF, 0, kHitPat);
    static const int tofmul_id = gHist.getSequentialID(kTOF, 0, kMulti);
    int multiplicity = 0;
    for(int seg=0; seg<NumOfSegTOF; ++seg){
      int nhit_tofu = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      int nhit_tofd = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      // AND
      if(nhit_tofu!=0 && nhit_tofd!=0){
	unsigned int tdc_u = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	unsigned int tdc_d = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
	// TDC AND
	if(tdc_u!=0 && tdc_d!=0){
	  hptr_array[tofhit_id]->Fill(seg);
	  ++multiplicity;
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
  // TOF_HT
  //------------------------------------------------------------------
  {
    // data typep
    static const int k_device = gUnpacker.get_device_id("TOF_HT");
    static const int k_u      = 0; // up
//    static const int k_d      = 1; // down
//    static const int k_adc    = gUnpacker.get_data_id("TOF_HT","adc");
    static const int k_tdc    = gUnpacker.get_data_id("TOF_HT","tdc");

    // sequential id
//    int tofa_id   = gHist.getSequentialID(kTOF_HT, 0, kADC);
    int toft_id   = gHist.getSequentialID(kTOF_HT, 0, kTDC);
//    int tofawt_id = gHist.getSequentialID(kTOF_HT, 0, kADCwTDC);
    for(int seg = 0; seg<NumOfSegTOF_HT; ++seg){
 //     // ADC
 //     int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc);
 //     if( nhit!=0 ){
 //       unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
 //       hptr_array[tofa_id + seg]->Fill( adc );
 //     }
      // TDC
//      nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      if(nhit != 0){
	int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	if( tdc!=0 ){
	  hptr_array[toft_id + seg]->Fill(tdc);
//	  // ADC w/TDC
//	  if( gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc)>0 ){
//	    unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
//	    hptr_array[tofawt_id + seg]->Fill( adc );
//	  }
	}
      }
    }

 //   // Down PMT
 //   tofa_id   = gHist.getSequentialID(kTOF_HT, 0, kADC, NumOfSegTOF_HT+1);
 //   toft_id   = gHist.getSequentialID(kTOF_HT, 0, kTDC, NumOfSegTOF_HT+1);
 //   tofawt_id = gHist.getSequentialID(kTOF_HT, 0, kADCwTDC, NumOfSegTOF_HT+1);

 //   for(int seg = 0; seg<NumOfSegTOF_HT; ++seg){
 //     // ADC
 //     int nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc);
 //     if(nhit != 0){
 //       unsigned int adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
 //       hptr_array[tofa_id + seg]->Fill(adc);
 //     }

 //     // TDC
 //     nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
 //     if(nhit != 0){
 //       int tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
 //       if(tdc != 0){
 //         hptr_array[toft_id + seg]->Fill(tdc);
 //         // ADC w/TDC
 //         if( gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc)>0 ){
 //           unsigned int adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
 //           hptr_array[tofawt_id + seg]->Fill( adc );
 //         }
 //       }
 //     }
 //   }

    // Hit pattern && multiplicity
    static const int tofhit_id = gHist.getSequentialID(kTOF_HT, 0, kHitPat);
    static const int tofmul_id = gHist.getSequentialID(kTOF_HT, 0, kMulti);
    int multiplicity = 0;
    for(int seg=0; seg<NumOfSegTOF_HT; ++seg){
      int nhit_tofu = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
//      int nhit_tofd = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      // AND
//      if(nhit_tofu!=0 && nhit_tofd!=0){
      if(nhit_tofu!=0){
	unsigned int tdc_u = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
//	unsigned int tdc_d = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
	// TDC AND
//	if(tdc_u!=0 && tdc_d!=0){
	if(tdc_u!=0){
	  hptr_array[tofhit_id]->Fill(seg);
	  ++multiplicity;
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
  //LC
  //------------------------------------------------------------------
  {
    // data typep
    static const int k_device = gUnpacker.get_device_id("LC");
    static const int k_u      = 0; // up
//    static const int k_d      = 1; // down
//    static const int k_adc    = gUnpacker.get_data_id("LC","adc");
    static const int k_tdc    = gUnpacker.get_data_id("LC","tdc");

    // sequential id
//    int lca_id   = gHist.getSequentialID(kLC, 0, kADC);
    int lct_id   = gHist.getSequentialID(kLC, 0, kTDC);
    for(int seg = 0; seg<NumOfSegLC; ++seg){
//      // ADC
//      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc);
//      if( nhit!=0 ){
//	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
//	hptr_array[lca_id + seg]->Fill( adc );
//      }
      // TDC
//      nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      if(nhit != 0){
	int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	if( tdc!=0 ){
	  hptr_array[lct_id + seg]->Fill(tdc);
	}
      }
    }

    // Down PMT
//    lca_id   = gHist.getSequentialID(kLC, 0, kADC, NumOfSegLC+1);
//    lct_id   = gHist.getSequentialID(kLC, 0, kTDC, NumOfSegLC+1);
//
//    for(int seg = 0; seg<NumOfSegLC; ++seg){
//      // ADC
//      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc);
//      if(nhit != 0){
//	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
//	hptr_array[lca_id + seg]->Fill(adc);
//      }

      // TDC
//      nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
//      if(nhit != 0){
//        int tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
//        if(tdc != 0){
//          hptr_array[lct_id + seg]->Fill(tdc);
//        }
//      }
//    }

    // Hit pattern && multiplicity
    static const int lchit_id = gHist.getSequentialID(kLC, 0, kHitPat);
    static const int lcmul_id = gHist.getSequentialID(kLC, 0, kMulti);
    int multiplicity = 0;
    for(int seg=0; seg<NumOfSegLC; ++seg){
      int nhit_lcu = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
//      int nhit_lcd = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      // AND
//      if(nhit_lcu!=0 && nhit_lcd!=0){
      if(nhit_lcu!=0){
	unsigned int tdc_u = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
//	unsigned int tdc_d = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
	// TDC AND
//	if(tdc_u!=0 && tdc_d!=0){
	if(tdc_u!=0){
	  hptr_array[lchit_id]->Fill(seg);
	  ++multiplicity;
	}
      }
    }

    hptr_array[lcmul_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif


  // Correlation (2D histograms) -------------------------------------
  {
    // data typep
    static const int k_device_bh1  = gUnpacker.get_device_id("BH1");
    static const int k_device_bh2  = gUnpacker.get_device_id("BH2");
    static const int k_device_sch  = gUnpacker.get_device_id("SCH");
    static const int k_device_tof  = gUnpacker.get_device_id("TOF");
    static const int k_device_bc3  = gUnpacker.get_device_id("BC3");
    static const int k_device_bc4  = gUnpacker.get_device_id("BC4");
    static const int k_device_sdc1 = gUnpacker.get_device_id("SDC1");
    static const int k_device_sdc2 = gUnpacker.get_device_id("SDC2");

    // sequential id
    int cor_id= gHist.getSequentialID(kCorrelation, 0, 0, 1);

    // BH1 vs BH2
    TH2* hcor_bh1bh2 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(int seg1 = 0; seg1<NumOfSegBH1; ++seg1){
      for(int seg2 = 0; seg2<NumOfSegBH2; ++seg2){
	int hitBH1 = gUnpacker.get_entries(k_device_bh1, 0, seg1, 0, 1);
	int hitBH2 = gUnpacker.get_entries(k_device_bh2, 0, seg2, 0, 1);
	if(hitBH1 == 0 || hitBH2 == 0)continue;
	int tdcBH1 = gUnpacker.get(k_device_bh1, 0, seg1, 0, 1);
	int tdcBH2 = gUnpacker.get(k_device_bh2, 0, seg2, 0, 1);
	if(tdcBH1 != 0 && tdcBH2 != 0){
	  hcor_bh1bh2->Fill(seg1, seg2);
	}
      }
    }

    // TOF vs SCH
    TH2* hcor_tofsch = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(int seg1 = 0; seg1<NumOfSegSCH; ++seg1){
      for(int seg2 = 0; seg2<NumOfSegTOF; ++seg2){
	int hitSCH = gUnpacker.get_entries(k_device_sch, 0, seg1, 0, 1);
	int hitTOF = gUnpacker.get_entries(k_device_tof, 0, seg2, 0, 1);
	if(hitTOF == 0 || hitSCH == 0) continue;
	int tdcSCH = gUnpacker.get(k_device_sch, 0, seg1, 0, 1);
	int tdcTOF = gUnpacker.get(k_device_tof, 0, seg2, 0, 1);
	if(tdcTOF != 0 && tdcSCH != 0){
	  hcor_tofsch->Fill(seg1, seg2);
	}
      }
    }

    // BC3 vs BC4
    TH2* hcor_bc3bc4 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(int wire1 = 0; wire1<NumOfWireBC3; ++wire1){
      for(int wire2 = 0; wire2<NumOfWireBC4; ++wire2){
	int hitBC3 = gUnpacker.get_entries(k_device_bc3, 0, 0, wire1, 0);
	int hitBC4 = gUnpacker.get_entries(k_device_bc4, 5, 0, wire2, 0);
	if(hitBC3 == 0 || hitBC4 == 0)continue;
	hcor_bc3bc4->Fill(wire1, wire2);
      }
    }

    // SDC2 vs SDC1
    TH2* hcor_sdc1sdc2 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(int wire1 = 0; wire1<NumOfWireSDC1; ++wire1){
      for(int wire2 = 0; wire2<NumOfWireSDC2; ++wire2){
	int hitSDC1 = gUnpacker.get_entries(k_device_sdc1, 0, 0, wire1, 0);
	int hitSDC2 = gUnpacker.get_entries(k_device_sdc2, 0, 0, wire2, 0);
	if( hitSDC1 == 0 || hitSDC2 == 0 ) continue;
	hcor_sdc1sdc2->Fill( wire1, wire2 );
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
    static const int k_d_bh1  = gUnpacker.get_device_id("BH1");
    static const int k_d_bh2  = gUnpacker.get_device_id("BH2");

    static const int k_u      = 0; // up
    static const int k_d      = 1; // down
    static const int k_tdc    = gUnpacker.get_data_id("BH1", "tdc");

    // HodoParam
    static const int cid_bh1  = 1;
    static const int cid_bh2  = 2;
    static const int plid     = 0;

    // Sequential ID
    static const int btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);

    // BH2
    double t0  = -999;
    double ofs = 0;
    int    seg = 0;
    int nhitu = gUnpacker.get_entries(k_d_bh2, 0, seg, k_u, k_tdc);
    int nhitd = gUnpacker.get_entries(k_d_bh2, 0, seg, k_d, k_tdc);
    if( nhitu != 0 && nhitd != 0 ){
      int tdcu = gUnpacker.get(k_d_bh2, 0, seg, k_u, k_tdc);
      int tdcd = gUnpacker.get(k_d_bh2, 0, seg, k_d, k_tdc);
      if( tdcu != 0 && tdcd != 0 ){
	HodoParamMan& hodoMan = HodoParamMan::GetInstance();
	double bh2ut, bh2dt;
	hodoMan.GetTime(cid_bh2, plid, seg, k_u, tdcu, bh2ut);
	hodoMan.GetTime(cid_bh2, plid, seg, k_d, tdcd, bh2dt);
	t0 = (bh2ut+bh2dt)/2;
      }//if(tdc)
    }// if(nhit)

    // BH1
    for(int seg = 0; seg<NumOfSegBH1; ++seg){
      int nhitu = gUnpacker.get_entries(k_d_bh1, 0, seg, k_u, k_tdc);
      int nhitd = gUnpacker.get_entries(k_d_bh1, 0, seg, k_d, k_tdc);
      if(nhitu != 0 &&  nhitd != 0){
	int tdcu = gUnpacker.get(k_d_bh1, 0, seg, k_u, k_tdc);
	int tdcd = gUnpacker.get(k_d_bh1, 0, seg, k_d, k_tdc);
	if(tdcu != 0 && tdcd != 0){
	  HodoParamMan& hodoMan = HodoParamMan::GetInstance();
	  double bh1tu, bh1td;
	  hodoMan.GetTime(cid_bh1, plid, seg, k_u, tdcu, bh1tu);
	  hodoMan.GetTime(cid_bh1, plid, seg, k_d, tdcd, bh1td);
	  double mt = (bh1tu+bh1td)/2.;
	  double btof = mt-(t0+ofs);
	  hptr_array[btof_id]->Fill(btof);
	}// if(tdc)
      }// if(nhit)
    }// for(seg)
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  //------------------------------------------------------------------
  // BH1-6_BH2-4
  //------------------------------------------------------------------
  {
    // Unpacker
    static const int k_d_bh1  = gUnpacker.get_device_id("BH1");
    static const int k_d_bh2  = gUnpacker.get_device_id("BH2");

    static const int k_u      = 0; // up
    static const int k_d      = 1; // down
    static const int k_tdc    = gUnpacker.get_data_id("BH1", "tdc");

    // Sequential ID
    static const int btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);

    // BH2
    int    multiplicity = 0;
    double t0  = -999;
    double ofs = 0;
    int seg = 4;
    int nhitu = gUnpacker.get_entries(k_d_bh2, 0, seg, k_u, k_tdc);
    int nhitd = gUnpacker.get_entries(k_d_bh2, 0, seg, k_d, k_tdc);
    if( nhitu != 0 && nhitd != 0 ){
      int tdcu = gUnpacker.get(k_d_bh2, 0, seg, k_u, k_tdc);
      int tdcd = gUnpacker.get(k_d_bh2, 0, seg, k_d, k_tdc);
      if( tdcu != 0 && tdcd != 0 ){
        ++multiplicity;
        t0 = (double)(tdcu+tdcd)/2.;
      }//if(tdc)
    }// if(nhit)

    if( multiplicity == 1 ){
	seg = 6;
	    // BH1
	    int nhitu = gUnpacker.get_entries(k_d_bh1, 0, seg, k_u, k_tdc);
	    int nhitd = gUnpacker.get_entries(k_d_bh1, 0, seg, k_d, k_tdc);
	    if(nhitu != 0 &&  nhitd != 0){
		    int tdcu = gUnpacker.get(k_d_bh1, 0, seg, k_u, k_tdc);
		    int tdcd = gUnpacker.get(k_d_bh1, 0, seg, k_d, k_tdc);
		    if(tdcu != 0 && tdcd != 0){
			    double mt = (double)(tdcu+tdcd)/2.; 
			    double btof = mt-(t0+ofs);
			    hptr_array[btof_id +1]->Fill(btof);
		    }// if(tdc)
	    }// if(nhit)
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  //------------------------------------------------------------------
  // SFT
  //------------------------------------------------------------------
  {
    // data type
    static const int k_device  = gUnpacker.get_device_id("SFT");
    static const int k_leading = gUnpacker.get_data_id("SFT", "leading");
    static const int k_trailing = gUnpacker.get_data_id("SFT", "trailing");

    // TDC gate range
    UserParamMan& gPar = UserParamMan::getInstance();
    static const int tdc_min = gPar.getParameter("SFT_TDC", 0);
    static const int tdc_max = gPar.getParameter("SFT_TDC", 1);

    // SequentialID
    int sft_t_id    = gHist.getSequentialID(kSFT, 0, kTDC,     1);
    int sft_ct_id   = gHist.getSequentialID(kSFT, 0, kTDC,    11);
    int sft_tot_id  = gHist.getSequentialID(kSFT, 0, kADC,     1);
    int sft_ctot_id = gHist.getSequentialID(kSFT, 0, kADC,    11);
    int sft_hit_id  = gHist.getSequentialID(kSFT, 0, kHitPat,  1);
    int sft_chit_id = gHist.getSequentialID(kSFT, 0, kHitPat, 11);
    int sft_mul_id   = gHist.getSequentialID(kSFT, 0, kMulti,   1);
    int sft_cmul_id  = gHist.getSequentialID(kSFT, 0, kMulti,  11);

    int sft_ct_2d_id = gHist.getSequentialID(kSFT, 0, kTDC2D,   1);
    int sft_ctot_2d_id = gHist.getSequentialID(kSFT, 0, kADC2D, 1);

    int multiplicity[4] ; // includes each layers.
    int cmultiplicity[4]; // includes each layers.

    for(int l=0; l<NumOfLayersSFT; ++l){
      int sft_fiber = 0;
      multiplicity[l]  = 0; // includes each layers.
      cmultiplicity[l] = 0; // includes each layers.
      int tdc_prev      = 0;

      if( l==0 || l==1 ){
	// X & X'
	sft_fiber = NumOfSegSFT_X;
      }else{
	// V & U
	sft_fiber = NumOfSegSFT_UV;
      }

      for(int i = 0; i<sft_fiber; ++i){

        int nhit_l = gUnpacker.get_entries(k_device, l, 0, i, k_leading);
        int nhit_t = gUnpacker.get_entries(k_device, l, 0, i, k_trailing);

        int hit_l_max = 0;
        int hit_t_max = 0;

        if(nhit_l != 0){
          hit_l_max = gUnpacker.get(k_device, l, 0, i, k_leading,  nhit_l - 1);
        }
        if(nhit_t != 0){
          hit_t_max = gUnpacker.get(k_device, l, 0, i, k_trailing, nhit_t - 1);
        }

        for(int m = 0; m<nhit_l; ++m){
          int tdc = gUnpacker.get(k_device, l, 0, i, k_leading, m);
          hptr_array[sft_t_id+l]->Fill(tdc);
          if(tdc_min < tdc && tdc < tdc_max){
            ++multiplicity[l];
            hptr_array[sft_hit_id+l]->Fill(i);
          }
          if(tdc_prev==tdc) continue;
          tdc_prev = tdc;
            hptr_array[sft_ct_id+l]->Fill(tdc);
            hptr_array[sft_ct_2d_id+l]->Fill(i, tdc);
          if(tdc_min < tdc && tdc < tdc_max){
            ++cmultiplicity[l];
            hptr_array[sft_chit_id+l]->Fill(i);
          }
        }
        if(nhit_l == nhit_t && hit_l_max > hit_t_max){
          tdc_prev = 0;
          for(int m = 0; m<nhit_l; ++m){
            int tdc = gUnpacker.get(k_device, l, 0, i, k_leading, m);
            int tdc_t = gUnpacker.get(k_device, l, 0, i, k_trailing, m);
            int tot = tdc - tdc_t;
            hptr_array[sft_tot_id+l]->Fill(tot);
            if(tdc_prev==tdc) continue;
            tdc_prev = tdc;
            if(tot==0) continue;
            hptr_array[sft_ctot_id+l]->Fill(tot);
            hptr_array[sft_ctot_2d_id+l]->Fill(i, tot);
          }
        }
     }
   }
   // X & X'
      hptr_array[sft_mul_id]->Fill(multiplicity[0] + multiplicity[1]);
      hptr_array[sft_cmul_id]->Fill(cmultiplicity[0] + cmultiplicity[1]);
   // V 
      hptr_array[sft_mul_id+1]->Fill(multiplicity[2]);
      hptr_array[sft_cmul_id+1]->Fill(cmultiplicity[2]);
   // U
      hptr_array[sft_mul_id+2]->Fill(multiplicity[3]);
      hptr_array[sft_cmul_id+2]->Fill(cmultiplicity[3]);
#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//SFT

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // SAC -----------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("SAC");
    static const int k_adc    = gUnpacker.get_data_id("SAC","adc");
    static const int k_tdc    = gUnpacker.get_data_id("SAC","tdc");

    // sequential id
    static const int saca_id   = gHist.getSequentialID(kSAC, 0, kADC,     1);
    static const int sact_id   = gHist.getSequentialID(kSAC, 0, kTDC,     1);
    static const int sacawt_id = gHist.getSequentialID(kSAC, 0, kADCwTDC, 1);
    static const int sach_id   = gHist.getSequentialID(kSAC, 0, kHitPat,  1);
    static const int sacm_id   = gHist.getSequentialID(kSAC, 0, kMulti,   1);

    int multiplicity = 0;
    for(int seg = 0; seg<NumOfRoomsSAC; ++seg){
      // ADC
      int nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      if( nhit_a!=0 ){
	int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[saca_id + seg]->Fill( adc );
      }
      // TDC
      int nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      if( nhit_t!=0 ){
	int tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc);
	if( tdc!=0 ){
	  hptr_array[sact_id + seg]->Fill( tdc );
	  // ADC w/TDC
	  if( gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0 ){
	    int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	    hptr_array[sacawt_id + seg]->Fill( adc );
	  }
	}
	hptr_array[sach_id]->Fill(seg);
	++multiplicity;
      }
    }

    hptr_array[sacm_id]->Fill( multiplicity );

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//SAC

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // FBT1 -----------------------------------------------------------
  {
    // data type
    static const int k_device   = gUnpacker.get_device_id("FBT1");
    static const int k_leading  = gUnpacker.get_data_id("FBT1", "leading");
    static const int k_trailing = gUnpacker.get_data_id("FBT1", "trailing");

    // TDC gate range
    UserParamMan& gPar = UserParamMan::getInstance();
    static const int tdc_min = gPar.getParameter("FBT1_TDC", 0);
    static const int tdc_max = gPar.getParameter("FBT1_TDC", 1);


    // sequential id
    for( int l=0; l<NumOfLayersFBT; ++l ){
      for( int ud=0; ud<NumOfUDStructureFBT; ++ud ){
	//          int fbt1_tdc_id     = gHist.getSequentialID(kFBT1, l, kTDC,    1+ ud*FBTOffset);
	//          int fbt1_tot_id     = gHist.getSequentialID(kFBT1, l, kADC,    1+ ud*FBTOffset);
	int fbt1_t_all_id   = gHist.getSequentialID(kFBT1, l, kTDC,
						    NumOfSegFBT1+1+ ud*FBTOffset);
	int fbt1_tot_all_id = gHist.getSequentialID(kFBT1, l, kADC,
						    NumOfSegFBT1+1+ ud*FBTOffset);
	int fbt1_hit_id     = gHist.getSequentialID(kFBT1, l, kHitPat, 1+ ud*FBTOffset);
	int fbt1_mul_id     = gHist.getSequentialID(kFBT1, l, kMulti,  1+ ud*FBTOffset);

	int fbt1_t_2d_id   = gHist.getSequentialID(kFBT1, l, kTDC2D, 1+ ud*FBTOffset);
	int fbt1_tot_2d_id = gHist.getSequentialID(kFBT1, l, kADC2D, 1+ ud*FBTOffset);

	int multiplicity  = 0;

	for( int seg=0; seg<NumOfSegFBT1; ++seg ){
	  int nhit = gUnpacker.get_entries(k_device, l, seg, ud, k_leading);

	  for(int m = 0; m<nhit; ++m){
	    int tdc      = gUnpacker.get(k_device, l, seg, ud, k_leading,  m);
	    int trailing = gUnpacker.get(k_device, l, seg, ud, k_trailing, m);
	    int tot      = tdc - trailing;
	    //                          hptr_array[fbt1_tdc_id +i]->Fill(tdc);
	    //                          hptr_array[fbt1_tot_id +i]->Fill(tot);
	    hptr_array[fbt1_t_all_id]->Fill(tdc);
	    hptr_array[fbt1_tot_all_id]->Fill(tot);
	    hptr_array[fbt1_t_2d_id]->Fill(seg, tdc);
	    hptr_array[fbt1_tot_2d_id]->Fill(seg, tot);
	    if( tdc_min<tdc && tdc<tdc_max ){
	      ++multiplicity;
	      hptr_array[fbt1_hit_id]->Fill(seg);
	    }
	  }
	}
	hptr_array[fbt1_mul_id]->Fill(multiplicity);
      }
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//FBT1

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif


  // FBT2 -----------------------------------------------------------
  {
    // data type
    static const int k_device   = gUnpacker.get_device_id("FBT2");
    static const int k_leading  = gUnpacker.get_data_id("FBT2", "leading");
    static const int k_trailing = gUnpacker.get_data_id("FBT2", "trailing");

    // TDC gate range
    UserParamMan& gPar = UserParamMan::getInstance();
    static const int tdc_min = gPar.getParameter("FBT2_TDC", 0);
    static const int tdc_max = gPar.getParameter("FBT2_TDC", 1);

    // sequential id
    for( int l=0; l<NumOfLayersFBT; ++l ){
      for( int ud=0; ud<NumOfUDStructureFBT; ++ud ){
	// int fbt1_tdc_id     = gHist.getSequentialID(kFBT2, l, kTDC,    1+ ud*FBTOffset);
	// int fbt1_tot_id     = gHist.getSequentialID(kFBT2, l, kADC,    1+ ud*FBTOffset);
	int fbt2_t_all_id   = gHist.getSequentialID(kFBT2, l, kTDC,
						    NumOfSegFBT2+1+ ud*FBTOffset);
	int fbt2_tot_all_id = gHist.getSequentialID(kFBT2, l, kADC,
						    NumOfSegFBT2+1+ ud*FBTOffset);
	int fbt2_hit_id     = gHist.getSequentialID(kFBT2, l, kHitPat, 1+ ud*FBTOffset);
	int fbt2_mul_id     = gHist.getSequentialID(kFBT2, l, kMulti,  1+ ud*FBTOffset);

	int fbt2_t_2d_id   = gHist.getSequentialID(kFBT2, l, kTDC2D, 1+ ud*FBTOffset);
	int fbt2_tot_2d_id = gHist.getSequentialID(kFBT2, l, kADC2D, 1+ ud*FBTOffset);

	int multiplicity  = 0;

	for( int seg=0; seg<NumOfSegFBT2; ++seg ){
	  int nhit = gUnpacker.get_entries(k_device, l, seg, ud, k_leading);

	  for(int m = 0; m<nhit; ++m){
	    int tdc      = gUnpacker.get(k_device, l, seg, ud, k_leading,  m);
	    int trailing = gUnpacker.get(k_device, l, seg, ud, k_trailing, m);
	    int tot      = tdc - trailing;
	    //                          hptr_array[fbt2_tdc_id +i]->Fill(tdc);
	    //                          hptr_array[fbt2_tot_id +i]->Fill(tot);
	    hptr_array[fbt2_t_all_id]->Fill(tdc);
	    hptr_array[fbt2_tot_all_id]->Fill(tot);
	    hptr_array[fbt2_t_2d_id]->Fill(seg, tdc);
	    hptr_array[fbt2_tot_2d_id]->Fill(seg, tot);
	    if( tdc_min<tdc && tdc<tdc_max ){
	      ++multiplicity;
	      hptr_array[fbt2_hit_id]->Fill(seg);
	    }
	  }
	}

	hptr_array[fbt2_mul_id]->Fill(multiplicity);
      }
    }

#if 0
          // Debug, dump data relating this detector
          gUnpacker.dump_data_device(k_device);
#endif
  }//FBT2

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  return 0;
} //process_event()

} //analyzer
