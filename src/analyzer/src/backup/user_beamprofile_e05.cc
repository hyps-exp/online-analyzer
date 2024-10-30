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
#include "DCAnalyzerOld.hh"
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
      FF_200, FF_400,  FF_580, FF_800, FF_1000,
      size_HistName
    };
  static const double FF_plus[] =
    {
      200., 400., 580, 800., 1000.
    };

//____________________________________________________________________________
int
process_begin(const std::vector<std::string>& argv)
{
  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  gConfMan.InitializeDCGeomMan();
  gConfMan.InitializeDCTdcCalibMan();
  gConfMan.InitializeDCDriftParamMan();
  if( !gConfMan.IsGood() ) return -1;
  // unpacker and all the parameter managers are initialized at this stage

  // Make tabs
  hddaq::gui::Controller& gCon = hddaq::gui::Controller::getInstance();
  TGFileBrowser *tab_hist  = gCon.makeFileBrowser("Hist");
  TGFileBrowser *tab_macro = gCon.makeFileBrowser("Macro");

  // Add macros to the Macro tab
  tab_macro->Add(clear_all_canvas());
  tab_macro->Add(clear_canvas());
  tab_macro->Add(split22());
  tab_macro->Add(split32());
  tab_macro->Add(split33());
  tab_macro->Add(dispBeamProfile_e05());

  // Add histograms to the Hist tab
  HistMaker& gHist = HistMaker::getInstance();
  //BcOut
  {
    TList *sub_dir = new TList;
    const char* nameSubDir = "BcOut";
    sub_dir->SetName(nameSubDir);
    int unique_id = gHist.getUniqueID(kMisc, 0, kHitPat);
    // Profile X
    for(int i = 0; i<size_HistName; ++i){
      char* title = Form("%s FF %d_X", nameSubDir, (int)FF_plus[i]);
      sub_dir->Add(gHist.createTH1(unique_id++, title,
				    400,-200,200,
				    "x position [mm]", ""));
    }
    // Profile Y
    for(int i = 0; i<size_HistName; ++i){
      char* title = Form("%s FF %d_Y", nameSubDir, (int)FF_plus[i]);
      sub_dir->Add(gHist.createTH1(unique_id++, title,
				    200,-100,100,
				    "y position [mm]", ""));
    }
    tab_hist->Add(sub_dir);

    unique_id = gHist.getUniqueID(kMisc, 0, kHitPat2D);
    // Profile XY
    for(int i = 0; i<size_HistName; ++i){
      char* title = Form("%s FF %d_XY", nameSubDir, (int)FF_plus[i]);
      sub_dir->Add(gHist.createTH2(unique_id++, title,
				   400,-200,200, 200,-100,100,
				   "x position [mm]", "y position [mm]"));
    }
    tab_hist->Add(sub_dir);
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

  /////////// BcOut
  {
    DCRHC BcOutAna(DetIdBcOut);
    bool BcOutTrack = BcOutAna.TrackSearch(9);

    if(BcOutTrack){
      static const int xpos_id = gHist.getSequentialID(kMisc, 0, kHitPat);
      static const int ypos_id = gHist.getSequentialID(kMisc, 0, kHitPat, size_HistName+1);
      static const int xypos_id = gHist.getSequentialID(kMisc, 0, kHitPat2D);

      for(int i = 0; i<size_HistName; ++i){
	double xpos = BcOutAna.GetPosX(dist_FF+FF_plus[i]);
	double ypos = BcOutAna.GetPosY(dist_FF+FF_plus[i]);
	hptr_array[xpos_id+i]->Fill(xpos);
	hptr_array[ypos_id+i]->Fill(ypos);
	hptr_array[xypos_id+i]->Fill(xpos, ypos);
      }
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  return 0;
}

}
