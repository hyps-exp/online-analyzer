// Author: Tomonori Takahashi

#include <iostream>
#include <string>
#include <vector>

#include <TGFileBrowser.h>
#include <TH1.h>
#include <TH2.h>
#include <TStyle.h>
#include <TString.h>

#include <Unpacker.hh>
#include <UnpackerManager.hh>

#include "Controller.hh"
#include "user_analyzer.hh"

#include "ConfMan.hh"
#include "DCAnalyzerOld.hh"
#include "DCDriftParamMan.hh"
#include "DCGeomMan.hh"
#include "DCTdcCalibMan.hh"
#include "DetectorID.hh"
#include "HistMaker.hh"
#include "MacroBuilder.hh"
#include "UserParamMan.hh"

namespace analyzer
{
  namespace
  {
    using namespace hddaq::unpacker;
    using namespace hddaq;

    std::vector<TH1*> hptr_array;

    const double dist_FF = 1200.;

    enum HistName
      {
	FF_m600, FF_m300, FF_0, FF_300, FF_600,
	NHist
      };
    const double FF_plus[] =
      {
	-600., -300., 0, 300., 600.
      };
  }

//____________________________________________________________________________
int
process_begin(const std::vector<std::string>& argv)
{
  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  gConfMan.InitializeParameter<DCGeomMan>("DCGEOM");
  gConfMan.InitializeParameter<DCTdcCalibMan>("TDCCALIB");
  gConfMan.InitializeParameter<DCDriftParamMan>("DRFTPM");
  gConfMan.InitializeParameter<UserParamMan>("USER");
  if( !gConfMan.IsGood() ) return -1;
  // unpacker and all the parameter managers are initialized at this stage

  // Make tabs
  hddaq::gui::Controller& gCon = hddaq::gui::Controller::getInstance();
  TGFileBrowser *tab_hist  = gCon.makeFileBrowser("Hist");
  TGFileBrowser *tab_macro = gCon.makeFileBrowser("Macro");

  // Add macros to the Macro tab
  tab_macro->Add(macro::Get("clear_all_canvas"));
  tab_macro->Add(macro::Get("clear_canvas"));
  tab_macro->Add(macro::Get("split22"));
  tab_macro->Add(macro::Get("split32"));
  tab_macro->Add(macro::Get("split33"));
  tab_macro->Add(macro::Get("dispBeamProfile_e42"));
  tab_macro->Add(macro::Get("dispBcOutFF"));
  //  tab_macro->Add(macro::Get("dispSSD1Profile"));

  // Add histograms to the Hist tab
  HistMaker& gHist = HistMaker::getInstance();
  //BcOut
  {
    TList *sub_dir = new TList;
    const char* nameSubDir = "BcOut";
    sub_dir->SetName(nameSubDir);
    int unique_id = gHist.getUniqueID(kMisc, 0, kHitPat);
    // Profile X
    for(int i = 0; i<NHist; ++i){
      char* title = Form("%s FF %d_X", nameSubDir, (int)FF_plus[i]);
      sub_dir->Add(gHist.createTH1(unique_id++, title,
				    400,-200,200,
				    "x position [mm]", ""));
    }
    // Profile Y
    for(int i = 0; i<NHist; ++i){
      char* title = Form("%s FF %d_Y", nameSubDir, (int)FF_plus[i]);
      sub_dir->Add(gHist.createTH1(unique_id++, title,
				    200,-100,100,
				    "y position [mm]", ""));
    }
    tab_hist->Add(sub_dir);
    // Profile XY
    for(int i = 0; i<NHist; ++i){
      char* title = Form("%s FF %d_XY", nameSubDir, (int)FF_plus[i]);
      sub_dir->Add(gHist.createTH2(unique_id++, title,
				   400,-200,200, 200,-100,100,
				   "x position [mm]", "y position [mm]"));
    }
    tab_hist->Add(sub_dir);

    sub_dir->Add(gHist.createTH2(unique_id++, "XU",
				 400,-200,200, 2000,-1,1,
				 "x0 [mm]", "u"));
    tab_hist->Add(sub_dir);
  }

  // Set histogram pointers to the vector sequentially.
  // This vector contains both TH1 and TH2.
  // Then you need to do down cast when you use TH2.
  if(0 != gHist.setHistPtr(hptr_array)){return -1;}

  gStyle->SetOptStat(1110);
  gStyle->SetTitleW(.400);
  gStyle->SetTitleH(.100);
  gStyle->SetStatW(.150);
  gStyle->SetStatH(.180);

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
  static UnpackerManager& gUnpacker = GUnpacker::get_instance();
  static DCGeomMan&       gGeom     = DCGeomMan::GetInstance();

  /////////// BcOut
  {
    DCRHC BcOutAna(DetIdBcOut);
    bool BcOutTrack = BcOutAna.TrackSearch(9);

    if(BcOutTrack){
      static const int xpos_id = gHist.getSequentialID(kMisc, 0, kHitPat);
      static const int ypos_id = gHist.getSequentialID(kMisc, 0, kHitPat, NHist+1);
      static const int xypos_id = gHist.getSequentialID(kMisc, 0, kHitPat, NHist*2+1);
      static const int xu_id = gHist.getSequentialID(kMisc, 0, kHitPat, NHist*3+1);

      for(int i = 0; i<NHist; ++i){
	double xpos = BcOutAna.GetPosX(dist_FF+FF_plus[i]);
	double ypos = BcOutAna.GetPosY(dist_FF+FF_plus[i]);
	hptr_array[xpos_id+i]->Fill(xpos);
	hptr_array[ypos_id+i]->Fill(ypos);
	hptr_array[xypos_id+i]->Fill(xpos, ypos);
      }
      double x0 = BcOutAna.GetX0();
      double u0 = BcOutAna.GetU();
      hptr_array[xu_id]->Fill(x0, u0);
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif


#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  return 0;
}

}
