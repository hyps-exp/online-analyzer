// Author: Tomonori Takahashi

#include <iostream>
#include <string>
#include <vector>

#include <TGFileBrowser.h>
#include <TH1.h>
#include <TH2.h>
#include <TStyle.h>
#include <TString.h>

#include "Controller.hh"
#include "user_analyzer.hh"
#include "UnpackerManager.hh"
#include "ConfMan.hh"
#include "DCAnalyzer.hh"
#include "DetectorID.hh"
#include "HistMaker.hh"
#include "MacroBuilder.hh"

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  std::vector<TH1*> hptr_array;
  
  enum HistName
    {
      FF_n500, FF_n300, FF_0, FF_300, FF_500,
      //FF_n100, FF_100,
      size_HistName
    };
  static const double FF_plus[] =
    {
      -500., -300., 0, 300., 500.
      //-100., 100.,
    };

//____________________________________________________________________________
int
process_begin(const std::vector<std::string>& argv)
{
  ConfMan& gConfMan = ConfMan::getInstance();
  gConfMan.initialize(argv);
  gConfMan.initializeDCGeomMan();
  gConfMan.initializeDCTdcCalibMan();
  gConfMan.initializeDCDriftParamMan();
  if(!gConfMan.isGood()){return -1;}
  // unpacker and all the parameter managers are initialized at this stage

  // Make tabs
  hddaq::gui::Controller& gCon = hddaq::gui::Controller::getInstance();
  TGFileBrowser *tab_hist  = gCon.makeFileBrowser("Hist");
  TGFileBrowser *tab_macro = gCon.makeFileBrowser("Macro");

  // Add macros to the Macro tab
  tab_macro->Add(clear_canvas());
  tab_macro->Add(split22());
  tab_macro->Add(split32());
  tab_macro->Add(split33());
  tab_macro->Add(dispBeamProfile_e07());

  // Add histograms to the Hist tab
  HistMaker& gHist = HistMaker::getInstance();
  int unique_id = gHist.getUniqueID(kMisc, 0, kHitPat);
  // Profile X
  for(int i = 0; i<size_HistName; ++i){
    char* title = Form("FF %d_X", (int)FF_plus[i]);
    tab_hist->Add(gHist.createTH1(unique_id++, title,
				  400,-200,200,
				  "x position [mm]", ""));    
  }

  // Profile Y
  for(int i = 0; i<size_HistName; ++i){
    char* title = Form("FF %d_Y", (int)FF_plus[i]);
    tab_hist->Add(gHist.createTH1(unique_id++, title,
				  200,-100,100,
				  "y position [mm]", ""));    
  }

  // Set histogram pointers to the vector sequentially.
  // This vector contains both TH1 and TH2.
  // Then you need to do down cast when you use TH2.
  if(0 != gHist.setHistPtr(hptr_array)){return -1;}

  gStyle->SetOptStat(1110);
  gStyle->SetTitleW(.4);
  gStyle->SetTitleH(.1);
  gStyle->SetStatW(.42);
  gStyle->SetStatH(.3);

  return 0;
}

//____________________________________________________________________________
int
process_end()
{
  return 0;
}

//____________________________________________________________________________
int
process_event()
{
  static HistMaker& gHist = HistMaker::getInstance();

  static const double dist_FF = 1200.;

  // Decode Hits & Search Track
  DCRHC BcOutAna(DetIdBcOut);
  bool BcOutTrack = BcOutAna.TrackSearch(9);

  if(BcOutTrack){
    static const int xpos_id = gHist.getSequentialID(kMisc, 0, kHitPat);
    static const int ypos_id = gHist.getSequentialID(kMisc, 0, kHitPat, size_HistName+1);
    
    for(int i = 0; i<size_HistName; ++i){
      hptr_array[xpos_id+i]->Fill(BcOutAna.GetPosX(dist_FF+FF_plus[i]));
      hptr_array[ypos_id+i]->Fill(BcOutAna.GetPosY(dist_FF+FF_plus[i]));
    }
  }
  
  return 0;
}

}
