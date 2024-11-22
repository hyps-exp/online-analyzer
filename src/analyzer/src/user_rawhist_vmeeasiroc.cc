// -*- C++ -*-

// Author: Tomonori Takahashi
// Change 2017/09/ S.Hoshino

#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

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
#include "DetectorID.hh"
#include "DCDriftParamMan.hh"
#include "DCGeomMan.hh"
#include "DCTdcCalibMan.hh"
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

#define DEBUG    0
#define FLAG_DAQ 1

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  namespace
  {
    const UserParamMan& gUser = UserParamMan::GetInstance();
    std::vector<TH1*> hptr_array;
    bool flag_event_cut = false;
    int event_cut_factor = 1; // for fast semi-online analysis
  }

//____________________________________________________________________________
int
process_begin( const std::vector<std::string>& argv )
{
  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  gConfMan.InitializeParameter<UserParamMan>("USER");
  if( !gConfMan.IsGood() ) return -1;
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

  // Add macros to the Macro tab
  //tab_macro->Add(hoge());
  tab_macro->Add(macro::Get("clear_all_canvas"));
  tab_macro->Add(macro::Get("clear_canvas"));
  tab_macro->Add(macro::Get("split22"));
  tab_macro->Add(macro::Get("split32"));
  tab_macro->Add(macro::Get("split33"));
  // tab_macro->Add(macro::Get("dispVMEEASIROC1D"));
  // tab_macro->Add(macro::Get("dispVMEEASIROC2D"));
  tab_macro->Add(macro::Get("dispVMEEASIROC2D_TDC_TOT"));
  tab_macro->Add(macro::Get("dispVMEEASIROC2D_HG_Multi"));
  tab_macro->Add(macro::Get("dispVMEEASIROC_AwT"));
  tab_macro->Add(macro::Get("dispDAQ"));
  // tab_macro->Add(macro::Get("dispVMEEASIROC_2DHG"));
  // tab_macro->Add(macro::Get("auto_monitor_all"));

  // Add histograms to the Hist tab
  HistMaker& gHist = HistMaker::getInstance();
  tab_hist->Add(gHist.createVMEEASIROC());

  // set histogram pointers to the vector sequentially.
  // This vector contains both TH1 and TH2.
  // Then you need to do down cast when you use TH2.
  if(0 != gHist.setHistPtr(hptr_array)){ return -1; }

  // Users don't have to touch this section (Make Ps tab),
  // but the file path should be changed.
  // ----------------------------------------------------------
  PsMaker& gPsMaker = PsMaker::getInstance();
  std::vector<TString> detList;
  std::vector<TString> optList;
  gHist.getListOfPsFiles(detList);
  gPsMaker.getListOfOption(optList);

  hddaq::gui::GuiPs& gPsTab = hddaq::gui::GuiPs::getInstance();
  gPsTab.setFilename(Form("%s/PSFile/pro/default_vmeeasiroc.ps", std::getenv("HOME")));
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

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  const int event_number = gUnpacker.get_event_number();
  if( flag_event_cut && event_number%event_cut_factor!=0 )
    return 0;

 // DAQ -------------------------------------------------------------
 // {
 //   // node id
 //   static const int k_eb      = gUnpacker.get_fe_id("k18eb");
 //   static const int k_vme     = gUnpacker.get_fe_id("vme03");
 //   static const int k_vmeeasiroc = gUnpacker.get_fe_id("vmeeasiroc0");

 //   // sequential id
 //   static const int eb_id      = gHist.getSequentialID(kDAQ, kEB, kHitPat);
 //   static const int vme_id     = gHist.getSequentialID(kDAQ, kVME, kHitPat2D);
 //   static const int vmeeasiroc_id = gHist.getSequentialID(kDAQ, kVMEEASIROC, kHitPat2D);

 //   { // EB
 //     int data_size = gUnpacker.get_node_header(k_eb, DAQNode::k_data_size);
 //     hptr_array[eb_id]->Fill(data_size);
 //   }

 //   { // VME node
 //     TH2* h = dynamic_cast<TH2*>(hptr_array[vme_id]);
 //     for( int i=0; i<10; ++i ){
 // 	if( i==1 || i==5 ) continue;
 // 	int node_id = k_vme+i;
 // 	int data_size = gUnpacker.get_node_header( node_id, DAQNode::k_data_size);
 // 	h->Fill( i, data_size );
 //     }
 //   }

 //   { // VMEEASIROC node
 //     TH2* h = dynamic_cast<TH2*>(hptr_array[easiroc_id]);
 //     for(int i = 0; i<20; ++i){
 // 	int data_size = gUnpacker.get_node_header(k_easiroc+i, DAQNode::k_data_size);
 // 	h->Fill( i, data_size );
 //     }
 //   }

 // }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

//  if( trigger_flag[SpillEndFlag] ) return 0;

  //------------------------------------------------------------------
  // VMEEASIROC
  //------------------------------------------------------------------
  {
    // data type
    static const int k_device   = gUnpacker.get_device_id("VMEEASIROC");
    static const int k_leading  = gUnpacker.get_data_id("VMEEASIROC" , "leading");
    static const int k_trailing = gUnpacker.get_data_id("VMEEASIROC" , "trailing");
    static const int k_highgain = gUnpacker.get_data_id("VMEEASIROC" , "highgain");
    static const int k_lowgain  = gUnpacker.get_data_id("VMEEASIROC", "lowgain");

    // SequentialID
    int vmeeasiroc_t_2d_id        = gHist.getSequentialID(kVMEEASIROC, 0, kTDC2D,          0);
    int vmeeasiroc_tot_2d_id      = gHist.getSequentialID(kVMEEASIROC, 0, kTOT2D,          0);
    int vmeeasiroc_hg_2d_id       = gHist.getSequentialID(kVMEEASIROC, 0, kHighGain,      10);
    int vmeeasiroc_chg_2d_id      = gHist.getSequentialID(kVMEEASIROC, 0, kHighGain,     100);
    int vmeeasiroc_lg_2d_id       = gHist.getSequentialID(kVMEEASIROC, 0, kLowGain,       10);
    int vmeeasiroc_clg_2d_id      = gHist.getSequentialID(kVMEEASIROC, 0, kLowGain,      100);
    int vmeeasiroc_multihit_2d_id = gHist.getSequentialID(kVMEEASIROC, 0, kMultiHitTdc,   20);
    int vmeeasiroc_hit_id         = gHist.getSequentialID(kVMEEASIROC, 0, kHitPat,         0);
    // int vmeeasiroc_hgvstot_2d_id  = gHist.getSequentialID(kVMEEASIROC, 0, kHighGainvsTOT, 30);

    // TDC gate range
    static const int tdc_min = gUser.GetParameter("TdcVMEEASIROC", 0);
    static const int tdc_max = gUser.GetParameter("TdcVMEEASIROC", 1);

    for(int l=0; l<NumOfPlaneVMEEASIROC; ++l){
      // Int_t plane = PlaneIdOfVMEEASIROC[l]-1; // 0 origin
      Int_t plane = PlaneIdOfVMEEASIROC[l];
      // Int_t tdc1st               = 0;
      // Int_t multiplicity         = 0;
      // Int_t multiplicity_wt      = 0;
      // Int_t multiplicity_ctot    = 0;
      // Int_t multiplicity_wt_ctot = 0;

      for(int seg = 0; seg<NumOfSegVMEEASIROC; ++seg){

	{ // tdc
	  int nhit_l = gUnpacker.get_entries(k_device, plane, seg, 0, k_leading );
	  bool flag_hit_wt = false;
	  if( nhit_l!=0 ) hptr_array[vmeeasiroc_hit_id+l]->Fill(seg);
	  for(int m = 0; m<nhit_l; ++m){
	    int tdc = gUnpacker.get(k_device, plane, seg, 0, k_leading, m);
	    hptr_array[vmeeasiroc_t_2d_id+l]->Fill(seg, tdc);
	    if(tdc_min < tdc && tdc < tdc_max){ // w/ TDC cut
	      flag_hit_wt = true;
	    }
	  }
	  if(flag_hit_wt){
	    // highgain w/ TDC cut
	    int nhit_hg = gUnpacker.get_entries(k_device, plane, seg, 0, k_highgain);
	    if( nhit_hg != 0 ){
	      int adc_hg = gUnpacker.get(k_device, plane, seg, 0, k_highgain);
	      hptr_array[vmeeasiroc_chg_2d_id+l]->Fill(seg, adc_hg);
	    }
	    // lowgain w/ TDC cut
	    int nhit_lg = gUnpacker.get_entries(k_device, plane, seg, 0, k_lowgain);
	    if( nhit_lg != 0 ){
	      int adc_lg = gUnpacker.get(k_device, plane, seg, 0, k_lowgain);
	      hptr_array[vmeeasiroc_clg_2d_id+l]->Fill(seg, adc_lg);
	    }
	  }
	}

	{ // tot
	  int nhit_l = gUnpacker.get_entries(k_device, plane, seg, 0, k_leading );
	  int nhit_t = gUnpacker.get_entries(k_device, plane, seg, 0, k_trailing );
	  Int_t hit_l_max = 0;
	  Int_t hit_t_max = 0;
	  if(nhit_l != 0) hit_l_max = gUnpacker.get(k_device, plane, seg, 0, k_leading,  nhit_l - 1);
	  if(nhit_t != 0) hit_t_max = gUnpacker.get(k_device, plane, seg, 0, k_trailing, nhit_t - 1);
	  // tdc1st = 0;
	  if(nhit_l == nhit_t && hit_l_max > hit_t_max){
	    for(Int_t m = 0; m<nhit_l; ++m){
	      int tdc   = gUnpacker.get(k_device, plane, seg, 0, k_leading, m);
	      int tdc_t = gUnpacker.get(k_device, plane, seg, 0, k_trailing, m);
	      int tot = tdc - tdc_t;
	      hptr_array[vmeeasiroc_tot_2d_id+l]->Fill(seg, tot);
	      // if(tot < tot_min) continue;
	      // hptr_array[sdc3t_ctot_id + l]->Fill(tdc);
	      // hptr_array[sdc3tot_ctot_id+l]->Fill(tot);
	      // if( tdc1st<tdc ) tdc1st = tdc;
	      // if( tdc_min < tdc && tdc < tdc_max ){
	      //   flag_hit_wt_ctot = true;
	      // }
	    }
	  }
	}

      	{ // High gain
	  int nhit_hg = gUnpacker.get_entries(k_device, plane, seg, 0, k_highgain);
	  for(int m = 0; m<nhit_hg; ++m){
	    int adc_hg = gUnpacker.get(k_device, plane, seg, 0, k_highgain, m);
	    hptr_array[vmeeasiroc_hg_2d_id+l]->Fill(seg, adc_hg);
	  }
	}

	{ // Low gain
	  int nhit_lg = gUnpacker.get_entries(k_device, plane, seg, 0, k_lowgain );
	  for(int m = 0; m<nhit_lg; ++m){
	    int adc_lg = gUnpacker.get(k_device, plane, seg, 0, k_lowgain, m);
	    hptr_array[vmeeasiroc_lg_2d_id+l]->Fill(seg, adc_lg);
	  }
	}

	{ // multi hit
	  int nhit_l = gUnpacker.get_entries(k_device, plane, seg, 0, k_leading );
	  hptr_array[vmeeasiroc_multihit_2d_id+l]->Fill(seg, nhit_l);
	}

      	// { // adc vs tot
	//   int nhit_hg = gUnpacker.get_entries(k_device, plane, seg, 0, k_highgain);
	//   int nhit_l  = gUnpacker.get_entries(k_device, plane, seg, 0, k_leading );
	//   int nhit_t  = gUnpacker.get_entries(k_device, plane, seg, 0, k_trailing );
	//   Int_t hit_l_max = 0;
	//   Int_t hit_t_max = 0;
	//   if(nhit_l != 0) hit_l_max = gUnpacker.get(k_device, plane, seg, 0, k_leading,  nhit_l - 1);
	//   if(nhit_t != 0) hit_t_max = gUnpacker.get(k_device, plane, seg, 0, k_trailing, nhit_t - 1);
	//   // tdc1st = 0;
	//   if(nhit_l == nhit_t && hit_l_max > hit_t_max){
	//     if(nhit_hg == nhit_l){
	//       for(Int_t m = 0; m<nhit_l; ++m){
	// 	int adc_hg = gUnpacker.get(k_device, plane, seg, 0, k_highgain, m);
	// 	int tdc    = gUnpacker.get(k_device, plane, seg, 0, k_leading, m);
	// 	int tdc_t  = gUnpacker.get(k_device, plane, seg, 0, k_trailing, m);
	// 	int tot = tdc - tdc_t;
	// 	hptr_array[vmeeasiroc_hgvstot_2d_id+l*NumOfSegVMEEASIROC+seg]->Fill(adc_hg, tot);
	// 	// if(tot < tot_min) continue;
	// 	// hptr_array[sdc3t_ctot_id + l]->Fill(tdc);
	// 	// hptr_array[sdc3tot_ctot_id+l]->Fill(tot);
	// 	// if( tdc1st<tdc ) tdc1st = tdc;
	// 	// if( tdc_min < tdc && tdc < tdc_max ){
	// 	//   flag_hit_wt_ctot = true;
	// 	// }
	//       }
	//     }
	//   }
	// }
      }
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//VMEEASIROC

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  return 0;
} //process_event()

} //analyzer
