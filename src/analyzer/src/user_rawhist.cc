// Author: Tomonori Takahashi

#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <TGFileBrowser.h>
#include <TH1.h>
#include <TH2.h>

#include "Controller.hh"

#include "user_analyzer.hh"
#include "UnpackerManager.hh"
#include "filesystem_util.hh"
#include "ConfMan.hh"
#include "UserParamMan.hh"
#include "HistMaker.hh"
#include "DetectorID.hh"
#include "PsMaker.hh"
#include "GuiPs.hh"
#include "MacroBuilder.hh"

#define DEBUG 0

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  std::vector<TH1*> hptr_array;
  
//____________________________________________________________________________
int
process_begin(const std::vector<std::string>& argv)
{
  ConfMan& gConfMan = ConfMan::getInstance();
  gConfMan.initialize(argv);
  gConfMan.initializeHodoParamMan();
  gConfMan.initializeHodoPHCMan();
  gConfMan.initializeDCGeomMan();
  gConfMan.initializeDCTdcCalibMan();
  gConfMan.initializeDCDriftParamMan();
  gConfMan.initializeUserParamMan();
  if(!gConfMan.isGood()){return -1;}
  // unpacker and all the parameter managers are initialized at this stage

  // Make tabs
  hddaq::gui::Controller& gCon = hddaq::gui::Controller::getInstance();
  TGFileBrowser *tab_hist  = gCon.makeFileBrowser("Hist");
  TGFileBrowser *tab_macro = gCon.makeFileBrowser("Macro");

  // Add macros to the Macro tab
  tab_macro->Add(hoge());
  tab_macro->Add(clear_canvas());
  tab_macro->Add(split22());
  tab_macro->Add(split32());
  tab_macro->Add(split33());
  tab_macro->Add(dispBH1());
  tab_macro->Add(dispBH2());
  tab_macro->Add(dispACs_SFV());
  tab_macro->Add(dispTOF());
  tab_macro->Add(dispLC());
  tab_macro->Add(dispBC3());
  tab_macro->Add(dispBC4());
  tab_macro->Add(dispSDC2());
  tab_macro->Add(dispHDC());
  tab_macro->Add(dispSDC3());
  tab_macro->Add(dispSDC4());
  tab_macro->Add(dispHitPat());

  // Add histograms to the Hist tab
  HistMaker& gHist = HistMaker::getInstance();
  tab_hist->Add(gHist.createBH1());
  tab_hist->Add(gHist.createBFT());
  tab_hist->Add(gHist.createBC3());
  tab_hist->Add(gHist.createBC4());
  tab_hist->Add(gHist.createBMW(false));
  tab_hist->Add(gHist.createBH2());
  tab_hist->Add(gHist.createBAC_SAC());
  tab_hist->Add(gHist.createSDC2());
  tab_hist->Add(gHist.createHDC());
  tab_hist->Add(gHist.createSP0());
  tab_hist->Add(gHist.createSDC3());
  tab_hist->Add(gHist.createSDC4());
  tab_hist->Add(gHist.createTOF());
  tab_hist->Add(gHist.createTOFMT());
  tab_hist->Add(gHist.createSFV_SAC3());
  tab_hist->Add(gHist.createLC());
  tab_hist->Add(gHist.createGe());
  tab_hist->Add(gHist.createPWO());
  tab_hist->Add(gHist.createTriggerFlag(false));
  tab_hist->Add(gHist.createCorrelation());

  // Set histogram pointers to the vector sequentially.
  // This vector contains both TH1 and TH2.
  // Then you need to do down cast when you use TH2.
  if(0 != gHist.setHistPtr(hptr_array)){return -1;}

  // Users don't have to touch this section (Make Ps tab),
  // but the file path should be changed.
  // ----------------------------------------------------------
  PsMaker& gPsMaker = PsMaker::getInstance();
  std::vector<std::string> detList;
  std::vector<std::string> optList;
  gHist.getListOfPsFiles(detList);
  gPsMaker.getListOfOption(optList);
  
  hddaq::gui::GuiPs& gPsTab = hddaq::gui::GuiPs::getInstance();
  gPsTab.setFilename("/home/sks/PSFile/tmp/default.ps");
  gPsTab.initialize(optList, detList);
  // ----------------------------------------------------------
  
  return 0;
}

//____________________________________________________________________________
int
process_end()
{
  hptr_array.clear();
  return 0;
}

//____________________________________________________________________________
int
process_event()
{
  static UnpackerManager& gUnpacker = GUnpacker::get_instance();
  static HistMaker&       gHist     = HistMaker::getInstance();

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif
  
  // BH1 -----------------------------------------------------------
  {
    // data type
    static const int k_u   = 0; // up
    static const int k_d   = 1; // down
    static const int k_adc = 0;
    static const int k_tdc = 1;

    // Up PMT
    int bh1a_id = gHist.getSequentialID(kBH1, 0, kADC);
    int bh1t_id = gHist.getSequentialID(kBH1, 0, kTDC);
    for(int seg=0; seg<NumOfSegBH1; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(DetIdBH1, 0, seg, k_u, k_adc);
      if(nhit!=0){
	unsigned int adc = gUnpacker.get(DetIdBH1, 0, seg, k_u, k_adc);
	hptr_array[bh1a_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(DetIdBH1, 0, seg, k_u, k_tdc);
      if(nhit!=0){
	unsigned int tdc = gUnpacker.get(DetIdBH1, 0, seg, k_u, k_tdc);
	if(tdc!=0){ hptr_array[bh1t_id + seg]->Fill(tdc); }
      }
    }

    // Down PMT
    bh1a_id = gHist.getSequentialID(kBH1, 0, kADC, NumOfSegBH1+1);
    bh1t_id = gHist.getSequentialID(kBH1, 0, kTDC, NumOfSegBH1+1);
    for(int seg=0; seg<NumOfSegBH1; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(DetIdBH1, 0, seg, k_d, k_adc);
      if(nhit!=0){
	unsigned int adc = gUnpacker.get(DetIdBH1, 0, seg, k_d, k_adc);
	hptr_array[bh1a_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(DetIdBH1, 0, seg, k_d, k_tdc);
      if(nhit!=0){
	unsigned int tdc = gUnpacker.get(DetIdBH1, 0, seg, k_d, k_tdc);
	if(tdc!=0){ hptr_array[bh1t_id + seg]->Fill(tdc); }
      }
    }

    // Hit pattern && multiplicity
    static const int bh1hit_id = gHist.getSequentialID(kBH1, 0, kHitPat);
    static const int bh1mul_id = gHist.getSequentialID(kBH1, 0, kMulti);
    int multiplicity = 0;
    for(int seg=0; seg<NumOfSegBH1; ++seg){
      int nhit_bh1u = gUnpacker.get_entries(DetIdBH1, 0, seg, k_u, k_tdc);
      int nhit_bh1d = gUnpacker.get_entries(DetIdBH1, 0, seg, k_d, k_tdc);
      // AND
      if(nhit_bh1u!=0 && nhit_bh1d!=0){
	unsigned int tdc_u = gUnpacker.get(DetIdBH1, 0, seg, k_u, k_tdc);
	unsigned int tdc_d = gUnpacker.get(DetIdBH1, 0, seg, k_d, k_tdc);
	// TDC AND
	if(tdc_u != 0 && tdc_d != 0){
	  hptr_array[bh1hit_id]->Fill(seg);
	  ++multiplicity;
	}
      }
    }

    hptr_array[bh1mul_id]->Fill(multiplicity);
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BFT -----------------------------------------------------------
  {
    // data type
    static const int k_uplane   = 0; // up
    static const int k_dplane   = 1; // down
    static const int k_leading  = 0;
    //    static const int k_trailing = 1;

    // TDC gate range
    UserParamMan& gPar = UserParamMan::getInstance();
    static const int tdc_min = gPar.getParameter("BFT_TDC", 0);
    static const int tdc_max = gPar.getParameter("BFT_TDC", 1);

    // sequential id
    static const int bft_tu_id = gHist.getSequentialID(kBFT, 0, kTDC, 1);
    static const int bft_td_id = gHist.getSequentialID(kBFT, 0, kTDC, 2);
    static const int bft_hitu_id = gHist.getSequentialID(kBFT, 0, kHitPat, 1);
    static const int bft_hitd_id = gHist.getSequentialID(kBFT, 0, kHitPat, 2);
    static const int bft_mul_id  = gHist.getSequentialID(kBFT, 0, kMulti, 1);

    int multiplicity = 0; // includes both u and d planes.
    for(int i = 0; i<NumOfSegBFT; ++i){
      int nhit_u = gUnpacker.get_entries(DetIdBFT, k_uplane, 0, i, k_leading);
      int nhit_d = gUnpacker.get_entries(DetIdBFT, k_dplane, 0, i, k_leading);

      // u plane
      for(int m = 0; m<nhit_u; ++m){
	int tdc = gUnpacker.get(DetIdBFT, k_uplane, 0, i, k_leading, m);
	hptr_array[bft_tu_id]->Fill(tdc);
	if(tdc_min < tdc && tdc < tdc_max){
	  ++multiplicity;
	  hptr_array[bft_hitu_id]->Fill(i);
	}
      }

      // d plane
      for(int m = 0; m<nhit_d; ++m){
	int tdc = gUnpacker.get(DetIdBFT, k_dplane, 0, i, k_leading, m);
	hptr_array[bft_td_id]->Fill(tdc);
	if(tdc_min < tdc && tdc < tdc_max){
	  ++multiplicity;
	  hptr_array[bft_hitd_id]->Fill(i);
	}
      }
    }

    hptr_array[bft_mul_id]->Fill(multiplicity);
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BC3 -------------------------------------------------------------
  {
    // data type
    static const int k_tdc = 0;        

    // TDC gate range
    static const int tdc_min 
      = UserParamMan::getInstance().getParameter("BC3_TDC", 0);
    static const int tdc_max 
      = UserParamMan::getInstance().getParameter("BC3_TDC", 1);

    // sequential id
    static const int bc3t_id   = gHist.getSequentialID(kBC3, 0, kTDC);
    static const int bc3hit_id = gHist.getSequentialID(kBC3, 0, kHitPat);
    static const int bc3mul_id = gHist.getSequentialID(kBC3, 0, kMulti);
    static const int bc3mulwt_id 
      = gHist.getSequentialID(kBC3, 0, kMulti, 1+NumOfLayersBC3);

    // TDC & HitPat & Multi
    for(int l = 0; l<NumOfLayersBC3; ++l){

      int multiplicity    = 0;
      int multiplicity_wt = 0;
      for(int w = 0; w<NumOfWireBC3; ++w){
	int nhit = gUnpacker.get_entries(DetIdBC3, l, 0, w, k_tdc);
	if(nhit == 0){continue;}
	
	// This wire fired at least one times.
	++multiplicity;
	hptr_array[bc3hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	for(int m = 0; m<nhit; ++m){
	  int tdc = gUnpacker.get(DetIdBC3, l, 0, w, k_tdc, m);
	  hptr_array[bc3t_id + l]->Fill(tdc);
	  
	  // Drift time check
	  if(tdc_min < tdc && tdc < tdc_max){
	    flag_hit_wt = true;
	  }
	}

	if(flag_hit_wt){ ++multiplicity_wt; }
      }
      
      hptr_array[bc3mul_id + l]->Fill(multiplicity);
      hptr_array[bc3mulwt_id + l]->Fill(multiplicity_wt);
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BC4 -------------------------------------------------------------
  {
    // data type
    static const int k_tdc = 0;
    
    // TDC gate range
    static const int tdc_min 
      = UserParamMan::getInstance().getParameter("BC4_TDC", 0);
    static const int tdc_max 
      = UserParamMan::getInstance().getParameter("BC4_TDC", 1);

    // sequential id
    static const int bc4t_id   = gHist.getSequentialID(kBC4, 0, kTDC);
    static const int bc4hit_id = gHist.getSequentialID(kBC4, 0, kHitPat);
    static const int bc4mul_id = gHist.getSequentialID(kBC4, 0, kMulti);
    static const int bc4mulwt_id 
      = gHist.getSequentialID(kBC4, 0, kMulti, 1+NumOfLayersBC4);
    
    // TDC & HitPat & Multi
    for(int l = 0; l<NumOfLayersBC4; ++l){

      int multiplicity    = 0;
      int multiplicity_wt = 0;
      for(int w = 0; w<NumOfWireBC4; ++w){
	int nhit = gUnpacker.get_entries(DetIdBC4, l, 0, w, k_tdc);
	if(nhit == 0){continue;}
	
	// This wire fired at least one times.
	++multiplicity;
	hptr_array[bc4hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	for(int m = 0; m<nhit; ++m){
	  int tdc = gUnpacker.get(DetIdBC4, l, 0, w, k_tdc, m);
	  hptr_array[bc4t_id + l]->Fill(tdc);
	  
	  // Drift time check
	  if(tdc_min < tdc && tdc < tdc_max){
	    flag_hit_wt = true;
	  }
	}

	if(flag_hit_wt){ ++multiplicity_wt; }
      }
      
      hptr_array[bc4mul_id + l]->Fill(multiplicity);
      hptr_array[bc4mulwt_id + l]->Fill(multiplicity_wt);
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BMW -------------------------------------------------------------
  {
    // data type
    static const int k_adc   = 0;
    static const int k_tdc   = 1;

    // sequential id
    static const int bmwa_id = gHist.getSequentialID(kBMW, 0, kADC);
    static const int bmwt_id = gHist.getSequentialID(kBMW, 0, kTDC);

    // ADC
    int nhit_a = gUnpacker.get_entries(DetIdBMW, 0, 0, 0, k_adc);
    if(nhit_a != 0){
      int adc = gUnpacker.get(DetIdBMW, 0, 0, 0, k_adc);
      hptr_array[bmwa_id]->Fill(adc, nhit_a);
    }

    // TDC
    int nhit_t = gUnpacker.get_entries(DetIdBMW, 0, 0, 0, k_tdc);
    if(nhit_t != 0){
      int tdc = gUnpacker.get(DetIdBMW, 0, 0, 0, k_tdc);
      if(tdc != 0){ hptr_array[bmwt_id]->Fill(tdc, nhit_t); }
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BH2 -----------------------------------------------------------
  {
    // data type
    static const int k_u   = 0; // up
    //    static const int k_d   = 1; // down
    static const int k_adc = 0;
    static const int k_tdc = 1;

    // sequential id
    static const int bh2a_id = gHist.getSequentialID(kBH2, 0, kADC);
    static const int bh2t_id = gHist.getSequentialID(kBH2, 0, kTDC);    
    for(int seg=0; seg<NumOfSegBH2; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(DetIdBH2, 0, seg, k_u, k_adc);
      if(nhit != 0){
	unsigned int adc = gUnpacker.get(DetIdBH2, 0, seg, k_u, k_adc);
	hptr_array[bh2a_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(DetIdBH2, 0, seg, k_u, k_tdc);
      if(nhit != 0){
	unsigned int tdc = gUnpacker.get(DetIdBH2, 0, seg, k_u, k_tdc);
	if(tdc != 0){ hptr_array[bh2t_id + seg]->Fill(tdc); }
      }
    }

    // Hit pattern &&  Multiplicity
    static const int bh2hit_id = gHist.getSequentialID(kBH2, 0, kHitPat);
    static const int bh2mul_id = gHist.getSequentialID(kBH2, 0, kMulti);
    int multiplicity = 0;
    for(int seg=0; seg<NumOfSegBH2; ++seg){
      int nhit = gUnpacker.get_entries(DetIdBH2, 0, seg, k_u, k_tdc);

      if(nhit != 0){
	unsigned int tdc = gUnpacker.get(DetIdBH2, 0, seg, k_u, k_tdc);
	// TDC
	if(tdc != 0){
	  hptr_array[bh2hit_id]->Fill(seg);
	  ++multiplicity;
	}
      }
    }

    hptr_array[bh2mul_id]->Fill(multiplicity);    
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BAC_SAC ---------------------------------------------------------
  {
    // data type
    static const int k_adc = 0;
    static const int k_tdc = 1;

    // sequential id
    static const int baca_id = gHist.getSequentialID(kBAC_SAC, 0, kADC, 1);
    static const int bact_id = gHist.getSequentialID(kBAC_SAC, 0, kTDC, 1);

    for(int seg = 0; seg<NumOfSegBAC; ++seg){
      // ADC
      int nhit_a = gUnpacker.get_entries(DetIdBAC, 0, seg, 0, k_adc);
      if(nhit_a != 0){
	int adc = gUnpacker.get(DetIdBAC, 0, seg, 0, k_adc);
	hptr_array[baca_id + seg]->Fill(adc, nhit_a);
      }

      // TDC
      int nhit_t = gUnpacker.get_entries(DetIdBAC, 0, seg, 0, k_tdc);
      if(nhit_t != 0){
	int tdc = gUnpacker.get(DetIdBAC, 0, seg, 0, k_tdc);
	if(tdc != 0){ hptr_array[bact_id + seg]->Fill(tdc, nhit_t); }
      }
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // SDC2 ------------------------------------------------------------
  {
    // data type
    static const int k_tdc = 0;
    
    // TDC gate range
    static const int tdc_min 
      = UserParamMan::getInstance().getParameter("SDC2_TDC", 0);
    static const int tdc_max 
      = UserParamMan::getInstance().getParameter("SDC2_TDC", 1);
   
    // sequential id
    static const int sdc2t_id   = gHist.getSequentialID(kSDC2, 0, kTDC, 1);
    static const int sdc2hit_id = gHist.getSequentialID(kSDC2, 0, kHitPat, 1);
    static const int sdc2mul_id = gHist.getSequentialID(kSDC2, 0, kMulti, 1);
    static const int sdc2mulwt_id 
      = gHist.getSequentialID(kSDC2, 0, kMulti, 1+NumOfLayersSDC2);
    
    // TDC & HitPat & Multi
    for(int l = 0; l<NumOfLayersSDC2; ++l){

      int multiplicity    = 0;
      int multiplicity_wt = 0;
      for(int w = 0; w<NumOfWireSDC2; ++w){
	int nhit = gUnpacker.get_entries(DetIdSDC2, l, 0, w, k_tdc);
	if(nhit == 0){continue;}
	
	// This wire fired at least one times.
	++multiplicity;
	hptr_array[sdc2hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	for(int m = 0; m<nhit; ++m){
	  int tdc = gUnpacker.get(DetIdSDC2, l, 0, w, k_tdc, m);
	  hptr_array[sdc2t_id + l]->Fill(tdc);
	  
	  // Drift time check
	  if(tdc_min < tdc && tdc < tdc_max){
	    flag_hit_wt = true;
	  }
	}

	if(flag_hit_wt){ ++multiplicity_wt; }
      }
      
      hptr_array[sdc2mul_id + l]->Fill(multiplicity);
      hptr_array[sdc2mulwt_id + l]->Fill(multiplicity_wt);
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // HDC ------------------------------------------------------------
  {
    // data type
    static const int k_tdc = 0;
    
    // TDC gate range
    static const int tdc_min 
      = UserParamMan::getInstance().getParameter("HDC_TDC", 0);
    static const int tdc_max 
      = UserParamMan::getInstance().getParameter("HDC_TDC", 1);
    
    // sequential id
    static const int hdct_id   = gHist.getSequentialID(kHDC, 0, kTDC, 1);
    static const int hdchit_id = gHist.getSequentialID(kHDC, 0, kHitPat, 1);
    static const int hdcmul_id = gHist.getSequentialID(kHDC, 0, kMulti, 1);
    static const int hdcmulwt_id 
      = gHist.getSequentialID(kHDC, 0, kMulti, 1+NumOfLayersHDC);
    
    // TDC & HitPat & Multi
    for(int l = 0; l<NumOfLayersHDC; ++l){

      int multiplicity    = 0;
      int multiplicity_wt = 0;
      for(int w = 0; w<NumOfWireHDC; ++w){
	int nhit = gUnpacker.get_entries(DetIdHDC, l, 0, w, k_tdc);
	if(nhit == 0){continue;}

	// This wire fired at least one times.
	++multiplicity;
	hptr_array[hdchit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	for(int m = 0; m<nhit; ++m){
	  int tdc = gUnpacker.get(DetIdHDC, l, 0, w, k_tdc, m);
	  hptr_array[hdct_id + l]->Fill(tdc);
	  
	  // Drift time check
	  if(tdc_min < tdc && tdc < tdc_max){
	    flag_hit_wt = true;
	  }
	}

	if(flag_hit_wt){ ++multiplicity_wt; }
      }
      
      hptr_array[hdcmul_id + l]->Fill(multiplicity);
      hptr_array[hdcmulwt_id + l]->Fill(multiplicity_wt);
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // SP0 -----------------------------------------------------------
  {
    // data type
    static const int k_u   = 0; // up
    static const int k_d   = 1; // down
    static const int k_adc = 0;
    static const int k_tdc = 1;

    for(int l = 0; l<NumOfLayersSP0; ++l){
      int sp0a_id = gHist.getSequentialID(kSP0, kSP0_L1+l, kADC);
      int sp0t_id = gHist.getSequentialID(kSP0, kSP0_L1+l, kTDC);

      for(int seg=0; seg<NumOfSegSP0; ++seg){
	// ADC
	int nhit = gUnpacker.get_entries(DetIdSP0, 0, seg, k_u, k_adc);
	if(nhit != 0){
	  unsigned int adc = gUnpacker.get(DetIdSP0, 0, seg, k_u, k_adc);
	  hptr_array[sp0a_id + seg]->Fill(adc);
	}

	// TDC
	nhit = gUnpacker.get_entries(DetIdSP0, 0, seg, k_u, k_tdc);
	if(nhit != 0){
	  unsigned int tdc = gUnpacker.get(DetIdSP0, 0, seg, k_u, k_tdc);
	  if(tdc != 0){ hptr_array[sp0t_id + seg]->Fill(tdc); }
	}
      }

      // Down PMT
      sp0a_id = gHist.getSequentialID(kSP0, kSP0_L1+l, kADC, NumOfSegSP0+1);
      sp0t_id = gHist.getSequentialID(kSP0, kSP0_L1+l, kTDC, NumOfSegSP0+1);
    
      for(int seg=0; seg<NumOfSegSP0; ++seg){
	// ADC
	int nhit = gUnpacker.get_entries(DetIdSP0, 0, seg, k_d, k_adc);
	if(nhit != 0){
	  unsigned int adc = gUnpacker.get(DetIdSP0, 0, seg, k_d, k_adc);
	  hptr_array[sp0a_id + seg]->Fill(adc);
	}

	// TDC
	nhit = gUnpacker.get_entries(DetIdSP0, 0, seg, k_d, k_tdc);
	if(nhit != 0){
	  unsigned int tdc = gUnpacker.get(DetIdSP0, 0, seg, k_d, k_tdc);
	  if(tdc != 0){ hptr_array[sp0t_id + seg]->Fill(tdc); }
	}
      }
    }

  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // SDC3 ------------------------------------------------------------
  {
    // data type
    static const int k_tdc = 0;
    
    // TDC gate range
    static const int tdc_min 
      = UserParamMan::getInstance().getParameter("SDC3_TDC", 0);
    static const int tdc_max 
      = UserParamMan::getInstance().getParameter("SDC3_TDC", 1);

    // sequential id
    static const int sdc3t_id   = gHist.getSequentialID(kSDC3, 0, kTDC);
    static const int sdc3hit_id = gHist.getSequentialID(kSDC3, 0, kHitPat);
    static const int sdc3mul_id = gHist.getSequentialID(kSDC3, 0, kMulti);
    static const int sdc3mulwt_id 
      = gHist.getSequentialID(kSDC3, 0, kMulti, 1+NumOfLayersSDC3);
    
    // TDC & HitPat & Multi
    for(int l = 0; l<NumOfLayersSDC3; ++l){

      int multiplicity    = 0;
      int multiplicity_wt = 0;
      
      int num_of_wire_SDC3 = 0;
      if(l ==1 || l ==4){
	num_of_wire_SDC3 = NumOfWireSDC3x;
      }else{
	num_of_wire_SDC3 = NumOfWireSDC3uv;
      }
      
      for(int w = 0; w<num_of_wire_SDC3; ++w){
	int nhit = gUnpacker.get_entries(DetIdSDC3, l, 0, w, k_tdc);
	if(nhit == 0){continue;}
	
	// This wire fired at least one times.
	++multiplicity;
	hptr_array[sdc3hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	for(int m = 0; m<nhit; ++m){
	  int tdc = gUnpacker.get(DetIdSDC3, l, 0, w, k_tdc, m);
	  hptr_array[sdc3t_id + l]->Fill(tdc);
	  
	  // Drift time check
	  if(tdc_min < tdc && tdc < tdc_max){
	    flag_hit_wt = true;
	  }
	}

	if(flag_hit_wt){ ++multiplicity_wt; }
      }
      
      hptr_array[sdc3mul_id + l]->Fill(multiplicity);
      hptr_array[sdc3mulwt_id + l]->Fill(multiplicity_wt);
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // SDC4 ------------------------------------------------------------
  {
    // data type
    static const int k_tdc = 0;
    
    // TDC gate range
    static const int tdc_min 
      = UserParamMan::getInstance().getParameter("SDC4_TDC", 0);
    static const int tdc_max 
      = UserParamMan::getInstance().getParameter("SDC4_TDC", 1);

    // sequential id
    static const int sdc4t_id   = gHist.getSequentialID(kSDC4, 0, kTDC);
    static const int sdc4hit_id = gHist.getSequentialID(kSDC4, 0, kHitPat);
    static const int sdc4mul_id = gHist.getSequentialID(kSDC4, 0, kMulti);
    static const int sdc4mulwt_id 
      = gHist.getSequentialID(kSDC4, 0, kMulti, 1+NumOfLayersSDC4);
    
    // TDC & HitPat & Multi
    for(int l = 0; l<NumOfLayersSDC4; ++l){

      int multiplicity    = 0;
      int multiplicity_wt = 0;
      
      int num_of_wire_SDC4 = 0;
      if(l ==1 || l ==4){
	num_of_wire_SDC4 = NumOfWireSDC4x;
      }else{
	num_of_wire_SDC4 = NumOfWireSDC4uv;
      }
      
      for(int w = 0; w<num_of_wire_SDC4; ++w){
	int nhit = gUnpacker.get_entries(DetIdSDC4, l, 0, w, k_tdc);
	if(nhit == 0){continue;}
	
	// This wire fired at least one times.
	++multiplicity;
	hptr_array[sdc4hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	for(int m = 0; m<nhit; ++m){
	  int tdc = gUnpacker.get(DetIdSDC4, l, 0, w, k_tdc, m);
	  hptr_array[sdc4t_id + l]->Fill(tdc);
	  
	  // Drift time check
	  if(tdc_min < tdc && tdc < tdc_max){
	    flag_hit_wt = true;
	  }
	}

	if(flag_hit_wt){ ++multiplicity_wt; }
      }
      
      hptr_array[sdc4mul_id + l]->Fill(multiplicity);
      hptr_array[sdc4mulwt_id + l]->Fill(multiplicity_wt);
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // TOF -----------------------------------------------------------
  {
    // data typep
    static const int k_u   = 0; // up
    static const int k_d   = 1; // down
    static const int k_adc = 0;
    static const int k_tdc = 1;

    // sequential id
    int tofa_id = gHist.getSequentialID(kTOF, 0, kADC);
    int toft_id = gHist.getSequentialID(kTOF, 0, kTDC);
    for(int seg = 0; seg<NumOfSegTOF; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(DetIdTOF, 0, seg, k_u, k_adc);
      if(nhit != 0){
	unsigned int adc = gUnpacker.get(DetIdTOF, 0, seg, k_u, k_adc);
	hptr_array[tofa_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(DetIdTOF, 0, seg, k_u, k_tdc);
      if(nhit != 0){
	int tdc = gUnpacker.get(DetIdTOF, 0, seg, k_u, k_tdc);
	if(tdc != 0){ hptr_array[toft_id + seg]->Fill(tdc); }
      }
    }

    // Down PMT
    tofa_id = gHist.getSequentialID(kTOF, 0, kADC, NumOfSegTOF+1);
    toft_id = gHist.getSequentialID(kTOF, 0, kTDC, NumOfSegTOF+1);
    
    for(int seg = 0; seg<NumOfSegTOF; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(DetIdTOF, 0, seg, k_d, k_adc);
      if(nhit != 0){
	unsigned int adc = gUnpacker.get(DetIdTOF, 0, seg, k_d, k_adc);
	hptr_array[tofa_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(DetIdTOF, 0, seg, k_d, k_tdc);
      if(nhit != 0){
	int tdc = gUnpacker.get(DetIdTOF, 0, seg, k_d, k_tdc);
	if(tdc != 0){ hptr_array[toft_id + seg]->Fill(tdc); }
      }
    }

    // Hit pattern && multiplicity
    static const int tofhit_id = gHist.getSequentialID(kTOF, 0, kHitPat);
    static const int tofmul_id = gHist.getSequentialID(kTOF, 0, kMulti);
    int multiplicity = 0;
    for(int seg=0; seg<NumOfSegTOF; ++seg){
      int nhit_tofu = gUnpacker.get_entries(DetIdTOF, 0, seg, k_u, k_tdc);
      int nhit_tofd = gUnpacker.get_entries(DetIdTOF, 0, seg, k_d, k_tdc);
      // AND
      if(nhit_tofu!=0 && nhit_tofd!=0){
	unsigned int tdc_u = gUnpacker.get(DetIdTOF, 0, seg, k_u, k_tdc);
	unsigned int tdc_d = gUnpacker.get(DetIdTOF, 0, seg, k_d, k_tdc);
	// TDC AND
	if(tdc_u != 0 && tdc_d != 0){
	  hptr_array[tofhit_id]->Fill(seg);
	  ++multiplicity;
	}
      }
    }

    hptr_array[tofmul_id]->Fill(multiplicity);
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // TOFMT -----------------------------------------------------------
  {
    // data type
    static const int k_u   = 0; // up
    //    static const int k_d   = 1; // down
    //    static const int k_adc = 0;
    static const int k_tdc = 1;

    // sequential id
    static const int tofmtt_id = gHist.getSequentialID(kTOFMT, 0, kTDC);
    for(int seg=0; seg<NumOfSegTOF; ++seg){
      // TDC
      int nhit = gUnpacker.get_entries(DetIdTOFMT, 0, seg, k_u, k_tdc);
      if(nhit != 0){
	int tdc = gUnpacker.get(DetIdTOFMT, 0, seg, k_u, k_tdc);
	if(tdc != 0){ hptr_array[tofmtt_id + seg]->Fill(tdc); }
      }
    }

    // Hit pattern && multiplicity
    static const int tofmthit_id = gHist.getSequentialID(kTOFMT, 0, kHitPat);
    static const int tofmtmul_id = gHist.getSequentialID(kTOFMT, 0, kMulti);
    int multiplicity = 0;
    for(int seg=0; seg<NumOfSegTOF; ++seg){
      int nhit = gUnpacker.get_entries(DetIdTOFMT, 0, seg, k_u, k_tdc);
      // AND
      if(nhit != 0){
	unsigned int tdc = gUnpacker.get(DetIdTOFMT, 0, seg, k_u, k_tdc);
	// TDC AND
	if(tdc != 0){
	  hptr_array[tofmthit_id]->Fill(seg);
	  ++multiplicity;
	}
      }
    }
    hptr_array[tofmtmul_id]->Fill(multiplicity);
  }

  // SFV_SAC3 --------------------------------------------------------
  {
    // data type
    static const int k_adc = 0;
    static const int k_tdc = 1;

    // sequential id
    static const int sfva_id = gHist.getSequentialID(kSFV_SAC3, 0, kADC);
    static const int sfvt_id = gHist.getSequentialID(kSFV_SAC3, 0, kTDC);

    static const int NofLoop = 7; // SFV 6 seg + SAC3 1 seg
    for(int seg = 0; seg<NofLoop; ++seg){
      // ADC
      int nhit_a = gUnpacker.get_entries(DetIdSFV, 0, seg, 0, k_adc);
      if(nhit_a != 0){
	int adc = gUnpacker.get(DetIdSFV, 0, seg, 0, k_adc);
	hptr_array[sfva_id + seg]->Fill(adc, nhit_a);
      }

      // TDC
      int nhit_t = gUnpacker.get_entries(DetIdSFV, 0, seg, 0, k_tdc);
      if(nhit_t != 0){
	int tdc = gUnpacker.get(DetIdSFV, 0, seg, 0, k_tdc);
	if(tdc != 0){ hptr_array[sfvt_id + seg]->Fill(tdc, nhit_t); }
      }
    }

    // Hit pattern
    static const int sfvhit_id = gHist.getSequentialID(kSFV_SAC3, 0, kHitPat);
    for(int seg = 0; seg<NofLoop; ++seg){
      int nhit = gUnpacker.get_entries(DetIdSFV, 0, seg, 0, k_tdc);
      if(nhit != 0){
	unsigned int tdc = gUnpacker.get(DetIdSFV, 0, seg, 0, k_tdc);
	if(tdc != 0){
	  hptr_array[sfvhit_id]->Fill(seg);
	}
      }
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // LC -----------------------------------------------------------
  {
    // data typep
    static const int k_u   = 0; // up
    static const int k_d   = 1; // down
    static const int k_adc = 0;
    static const int k_tdc = 1;

    // sequential id
    int lca_id = gHist.getSequentialID(kLC, 0, kADC);
    int lct_id = gHist.getSequentialID(kLC, 0, kTDC);
    for(int seg = 0; seg<NumOfSegLC; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(DetIdLC, 0, seg, k_u, k_adc);
      if(nhit != 0){
	unsigned int adc = gUnpacker.get(DetIdLC, 0, seg, k_u, k_adc);
	hptr_array[lca_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(DetIdLC, 0, seg, k_u, k_tdc);
      if(nhit != 0){
	int tdc = gUnpacker.get(DetIdLC, 0, seg, k_u, k_tdc);
	if(tdc != 0){ hptr_array[lct_id + seg]->Fill(tdc); }
      }
    }

    // Down PMT
    lca_id = gHist.getSequentialID(kLC, 0, kADC, NumOfSegLC+1);
    lct_id = gHist.getSequentialID(kLC, 0, kTDC, NumOfSegLC+1);
    
    for(int seg = 0; seg<NumOfSegLC; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(DetIdLC, 0, seg, k_d, k_adc);
      if(nhit != 0){
	unsigned int adc = gUnpacker.get(DetIdLC, 0, seg, k_d, k_adc);
	hptr_array[lca_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(DetIdLC, 0, seg, k_d, k_tdc);
      if(nhit != 0){
	int tdc = gUnpacker.get(DetIdLC, 0, seg, k_d, k_tdc);
	if(tdc != 0){ hptr_array[lct_id + seg]->Fill(tdc); }
      }
    }

    // Hit pattern && multiplicity
    static const int lchit_id = gHist.getSequentialID(kLC, 0, kHitPat);
    static const int lcmul_id = gHist.getSequentialID(kLC, 0, kMulti);
    int multiplicity = 0;
    for(int seg=0; seg<NumOfSegLC; ++seg){
      int nhit_lcu = gUnpacker.get_entries(DetIdLC, 0, seg, k_u, k_tdc);
      int nhit_lcd = gUnpacker.get_entries(DetIdLC, 0, seg, k_d, k_tdc);
      // AND
      if(nhit_lcu!=0 && nhit_lcd!=0){
	unsigned int tdc_u = gUnpacker.get(DetIdLC, 0, seg, k_u, k_tdc);
	unsigned int tdc_d = gUnpacker.get(DetIdLC, 0, seg, k_d, k_tdc);
	// TDC AND
	if(tdc_u != 0 && tdc_d != 0){
	  hptr_array[lchit_id]->Fill(seg);
	  ++multiplicity;
	}
      }
    }

    hptr_array[lcmul_id]->Fill(multiplicity);
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // Correlation (2D histograms) -------------------------------------
  {
    // sequential id
    int cor_id= gHist.getSequentialID(kCorrelation, 0, 0, 1);

    // BH1 vs BH2
    TH2* hcor_bh1bh2 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(int seg1 = 0; seg1<NumOfSegBH1; ++seg1){
      for(int seg2 = 0; seg2<NumOfSegBH2; ++seg2){
	int hitBH1 = gUnpacker.get_entries(DetIdBH1, 0, seg1, 0, 1);
	int hitBH2 = gUnpacker.get_entries(DetIdBH2, 0, seg2, 0, 1);
	if(hitBH1 == 0 || hitBH2 == 0)continue;
	int tdcBH1 = gUnpacker.get(DetIdBH1, 0, seg1, 0, 1);
	int tdcBH2 = gUnpacker.get(DetIdBH2, 0, seg2, 0, 1);
	if(tdcBH1 != 0 && tdcBH2 != 0){
	  hcor_bh1bh2->Fill(seg1, seg2);
	}
      }
    }

    // TOF vs LC
    TH2* hcor_toflc = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(int seg1 = 0; seg1<NumOfSegTOF; ++seg1){
      for(int seg2 = 0; seg2<NumOfSegLC; ++seg2){
	int hitTOF = gUnpacker.get_entries(DetIdTOF, 0, seg1, 0, 1);
	int hitLC  = gUnpacker.get_entries(DetIdLC, 0, seg2, 0, 1);
	if(hitTOF == 0 || hitLC == 0)continue;
	int tdcTOF = gUnpacker.get(DetIdTOF, 0, seg1, 0, 1);
	int tdcLC  = gUnpacker.get(DetIdLC,  0, seg2, 0, 1);
	if(tdcTOF != 0 && tdcLC != 0){
	  hcor_toflc->Fill(seg1, seg2);
	}
      }
    }

    // BC3 vs BC4
    TH2* hcor_bc3bc4 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(int wire1 = 0; wire1<NumOfWireBC3; ++wire1){
      for(int wire2 = 0; wire2<NumOfWireBC4; ++wire2){
	int hitBC3 = gUnpacker.get_entries(DetIdBC3, 0, 0, wire1, 0);
	int hitBC4 = gUnpacker.get_entries(DetIdBC4, 5, 0, wire2, 0);
	if(hitBC3 == 0 || hitBC4 == 0)continue;
	hcor_bc3bc4->Fill(wire1, wire2);
      }
    }

    // SDC2 vs HDC
    TH2* hcor_sdc2hdc = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(int wire1 = 0; wire1<NumOfWireSDC2; ++wire1){
      for(int wire2 = 0; wire2<NumOfWireHDC; ++wire2){
	int hitSDC2 = gUnpacker.get_entries(DetIdSDC2, 0, 0, wire1, 0);
	int hitHDC  = gUnpacker.get_entries(DetIdHDC,  3, 0, wire2, 0);
	if(hitSDC2 == 0 || hitHDC == 0)continue;
	hcor_sdc2hdc->Fill(wire1, wire2);
      }
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  //------------------------------------------------------------------
  // Hyperball-J
  //------------------------------------------------------------------

  // Ge --------------------------------------------------------------
  {
    // data typep
    static const int k_adc = 0;
    static const int k_crm = 1;
    static const int k_tfa = 2;
    static const int k_pur = 3;
    static const int k_rst = 4;

    // sequential id
    // sequential hist
    static const int ge_adc_id = gHist.getSequentialID(kGe, 0, kADC);
    static const int ge_crm_id = gHist.getSequentialID(kGe, 0, kCRM);
    static const int ge_tfa_id = gHist.getSequentialID(kGe, 0, kTFA);
    static const int ge_pur_id = gHist.getSequentialID(kGe, 0, kPUR);
    static const int ge_rst_id = gHist.getSequentialID(kGe, 0, kRST);

    // 2d hist id
    static const
      int ge_adc2d_id = gHist.getSequentialID(kGe, 0, kADC, NumOfSegGe +1);
    static const
      int ge_crm2d_id = gHist.getSequentialID(kGe, 0, kCRM, NumOfSegGe +1);
    static const
      int ge_tfa2d_id = gHist.getSequentialID(kGe, 0, kTFA, NumOfSegGe +1);
    static const
      int ge_pur2d_id = gHist.getSequentialID(kGe, 0, kPUR, NumOfSegGe +1);
    static const
      int ge_rst2d_id = gHist.getSequentialID(kGe, 0, kRST, NumOfSegGe +1);
    
    // sum hist id
    static const
      int ge_adcsum_id = gHist.getSequentialID(kGe, 0, kADC, NumOfSegGe +2);

    for(int seg = 0; seg<NumOfSegGe; ++seg){
      // ADC
      int nhit_adc = gUnpacker.get_entries(DetIdGe, 0, seg, 0, k_adc);
      if(nhit_adc != 0){
	int adc = gUnpacker.get(DetIdGe, 0, seg, 0, k_adc);
	hptr_array[ge_adc_id + seg]->Fill(adc);
	hptr_array[ge_adc2d_id]->Fill(seg, adc);
	hptr_array[ge_adcsum_id]->Fill(adc);
      }

      // CRM
      int nhit_crm = gUnpacker.get_entries(DetIdGe, 0, seg, 0, k_crm);
      if(nhit_crm != 0){
	for(int m = 0; m<nhit_crm; ++m){
	  int crm = gUnpacker.get(DetIdGe, 0, seg, 0, k_crm, m);
	  hptr_array[ge_crm_id + seg]->Fill(crm);
	  hptr_array[ge_crm2d_id]->Fill(seg, crm);
	}
      }

      // TFA
      int nhit_tfa = gUnpacker.get_entries(DetIdGe, 0, seg, 0, k_tfa);
      if(nhit_tfa != 0){
	for(int m = 0; m<nhit_tfa; ++m){
	  int tfa = gUnpacker.get(DetIdGe, 0, seg, 0, k_tfa, m);
	  hptr_array[ge_tfa_id + seg]->Fill(tfa);
	  hptr_array[ge_tfa2d_id]->Fill(seg, tfa);	
	}
      }

      // PUR
      int nhit_pur = gUnpacker.get_entries(DetIdGe, 0, seg, 0, k_pur);
      if(nhit_pur != 0){
	for(int m = 0; m<nhit_pur; ++m){
	  int pur = gUnpacker.get(DetIdGe, 0, seg, 0, k_pur, m);
	  hptr_array[ge_pur_id + seg]->Fill(pur);
	  hptr_array[ge_pur2d_id]->Fill(seg, pur);
	}
      }

      // RST
      int nhit_rst = gUnpacker.get_entries(DetIdGe, 0, seg, 0, k_rst);
      if(nhit_rst != 0){
	for(int m = 0; m<nhit_rst; ++m){
	  int rst = gUnpacker.get(DetIdGe, 0, seg, 0, k_rst, m);
	  hptr_array[ge_rst_id + seg]->Fill(rst);
	  hptr_array[ge_rst2d_id]->Fill(seg, rst);
	}
      }
    }

  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  return 0;
}

}
