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

//#include "BH2Filter.hh"
#include "BH2Hit.hh"
#include "ConfMan.hh"
//#include "DCAnalyzer.hh"
#include "DCAnalyzerOld.hh"
#include "DCDriftParamMan.hh"
#include "DCGeomMan.hh"
//#include "DCHit.hh"
//#include "DCLocalTrack.hh"
#include "DCTdcCalibMan.hh"
#include "DetectorID.hh"
//#include "EventAnalyzer.hh"
#include "HistMaker.hh"
#include "HodoAnalyzer.hh"
#include "HodoParamMan.hh"
#include "HodoPHCMan.hh"
#include "MacroBuilder.hh"
//#include "RawData.hh"
#include "UserParamMan.hh"

#define BH2FILTER 0

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  namespace
  {
    // const UnpackerManager& gUnpacker  = GUnpacker::get_instance();
    // const HistMaker&       gHist      = HistMaker::getInstance();
    // const DCGeomMan&       gGeom      = DCGeomMan::GetInstance();
    //       BH2Filter&       gBH2Filter = BH2Filter::GetInstance();
    const UserParamMan& gUser = UserParamMan::GetInstance();

    const Double_t dist_FF = 1200.;
    const Double_t dist_D1 = 2100.62;

    std::vector<TH1*> hptr_array;

    // enum HistName
    //   {
    // 	// Inside Target
    // 	i_Zn495, i_Zn338, i_Zn167_5, i_Z0, i_Z150, i_Z240,
    // 	// Detector positions
    // 	i_SFT, i_EG, i_DC1, i_SAC,
    // 	NHist
    //   };
    // static const Double_t TgtOrg_plus[] =
    //   {
    // 	// Inside Target
    // 	-495., -338., -167.5, 0, 150., 240.,
    // 	// Detector
    // 	350., 540., 733., 866.
    //   };

    // static const TString posName[] =
    //   {
    // 	// Inside Target
    // 	"Tgt Z=-495", "Tgt Z=-338", "Tgt Z=-167.5", "Tgt Z=0",
    // 	"Tgt Z=150",  "Tgt Z=240",
    // 	// Detectors
    // 	"E03 TGT", "End guard", "DC1", "KbAC"
    //   };
    // enum HistName
    // {
    //   FF_000, FF_200, FF_400, FF_550, FF_800, FF_1000,
    //   NHist
    // };
    // static const double FF_plus[NHist] =
    //   {
    // 	0., 200., 400, 550., 800., 1000.
    //   };


    enum HistName_FF
    {
      FF_m400, FF_m200, FF_0, FF_200, FF_400, FF_600, FF_800,
      NHist_FF
    };
    static const double FF_plus[NHist_FF] =
      {
	-400., -200., 0, 200., 400., 600., 800.
      };
    enum HistName_D1
    {
      D1_400, D1_700, D1_1000, D1_1300, D1_1600, D1_1900,
      NHist_D1
    };
    static const double D1_plus[NHist_D1] =
      {
	400., 700., 1000., 1300., 1600., 1900.
      };
    //static Double_t FF_plus = 0;
  }

//____________________________________________________________________________
Int_t
process_begin( const std::vector<std::string>& argv )
{
  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  //gConfMan.InitializeParameter<BH2Filter>("BH2FLT");
  gConfMan.InitializeParameter<DCGeomMan>("DCGEO");
  //gConfMan.InitializeParameter<DCTdcCalibMan>("TDCCALIB");
  gConfMan.InitializeParameter<DCTdcCalibMan>("DCTDC");
  //gConfMan.InitializeParameter<DCDriftParamMan>("DRFTPM");
  gConfMan.InitializeParameter<DCDriftParamMan>("DCDRFT");
  gConfMan.InitializeParameter<HodoParamMan>("HDPRM");
  gConfMan.InitializeParameter<HodoPHCMan>("HDPHC");
  gConfMan.InitializeParameter<UserParamMan>("USER");
  if( !gConfMan.IsGood() ) return -1;
  // unpacker and all the parameter managers are initialized at this stage

  //gBH2Filter.Print();
  // gBH2Filter.SetVerbose();

  //FF_plus = UserParamMan::GetInstance().GetParameter("FF_PLUS", 0);
  //std::cout << "#D : FF+" << FF_plus << std::endl;

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
  tab_macro->Add(macro::Get("dispBeamProfile_e40"));
  tab_macro->Add(macro::Get("dispBcOutFF"));
  tab_macro->Add(macro::Get("dispBH2Filter"));
  {
    TList *sub_dir = new TList;
    const char* nameSubDir = "y2020";
    sub_dir->SetName(nameSubDir);
    sub_dir->Add(macro::Get("dispBAC1_y2020"));
    //sub_dir->Add(macro::Get("dispBAC2_y2020"));
    //sub_dir->Add(macro::Get("dispPVAC_y2020"));
    //sub_dir->Add(macro::Get("dispFAC_y2020"));
    sub_dir->Add(macro::Get("dispE70targeting"));
    tab_macro->Add(sub_dir);
  }
  tab_macro->Add(macro::Get("dispKtune202006"));
  tab_macro->Add(macro::Get("dispHitPat_y2020"));
  tab_macro->Add(macro::Get("dispHitPat_y2020kaon"));
  tab_macro->Add(macro::Get("dispBeamProfile_e70"));
  tab_macro->Add(macro::Get("dispBeamProfile_e70kaon"));
  tab_macro->Add(macro::Get("dispS2sProfile_e70"));
  tab_macro->Add(macro::Get("dispS2sProfile_e70kaon"));
  tab_macro->Add(macro::Get("dispSdcOutResidual"));

  // Add histograms to the Hist tab
  HistMaker& gHist = HistMaker::getInstance();
  //tab_hist->Add(gHist.createBH2());
  //BcOut
  {
    TList *sub_dir = new TList;
    const char* nameSubDir = "BcOut";
    sub_dir->SetName(nameSubDir);
    Int_t unique_id = gHist.getUniqueID(kMisc, 0, kHitPat);

     // Profile X
    for(int i = 0; i<NHist_FF; ++i){
      char* title = Form("%s FF %d_X", nameSubDir, (int)FF_plus[i]);
      sub_dir->Add(gHist.createTH1(unique_id++, title,
    				    400,-200,200,
    				    "x position [mm]", ""));
    }
    // Profile Y
    for(int i = 0; i<NHist_FF; ++i){
      char* title = Form("%s FF %d_Y", nameSubDir, (int)FF_plus[i]);
      sub_dir->Add(gHist.createTH1(unique_id++, title,
    				    200,-100,100,
    				    "y position [mm]", ""));
    }
    tab_hist->Add(sub_dir);
    // Profile XY
    for(int i = 0; i<NHist_FF; ++i){
      char* title = Form("%s FF %d_XY", nameSubDir, (int)FF_plus[i]);
      sub_dir->Add(gHist.createTH2(unique_id++, title,
    				   400,-200,200, 200,-100,100,
    				   "x position [mm]", "y position [mm]"));
    }
    tab_hist->Add(sub_dir);


     // Profile X kaon
    for(int i = 0; i<NHist_FF; ++i){
      char* title = Form("%s FF %d_X kaon", nameSubDir, (int)FF_plus[i]);
      sub_dir->Add(gHist.createTH1(unique_id++, title,
    				    400,-200,200,
    				    "x position [mm]", ""));
    }
    // Profile Y kaon
    for(int i = 0; i<NHist_FF; ++i){
      char* title = Form("%s FF %d_Y kaon", nameSubDir, (int)FF_plus[i]);
      sub_dir->Add(gHist.createTH1(unique_id++, title,
    				    200,-100,100,
    				    "y position [mm]", ""));
    }


    // // Profile X
    // for(Int_t i = 0; i<NHist; ++i){
    //   char* title = i==i_Z0?
    // 	Form("%s_X %s (FF+%d)", nameSubDir, posName[i].Data(), (Int_t)FF_plus[i]) :
    // 	Form("%s_X %s", nameSubDir, posName[i].Data());
    //   sub_dir->Add(gHist.createTH1(unique_id++, title,
    // 				    400,-200,200,
    // 				    "x position [mm]", ""));
    // }
    // // Profile Y
    // for(Int_t i = 0; i<NHist; ++i){
    //   char* title = i==i_Z0?
    // 	Form("%s_Y %s (FF+%d)", nameSubDir, posName[i].Data(), (Int_t)FF_plus[i]) :
    // 	Form("%s_Y %s", nameSubDir, posName[i].Data());
    //   sub_dir->Add(gHist.createTH1(unique_id++, title,
    // 				   200,-100,100,
    // 				   "y position [mm]", ""));
    // }
    // tab_hist->Add(sub_dir);
    // // Profile XY
    // for(Int_t i = 0; i<NHist; ++i){
    //   char* title = i==i_Z0?
    // 	Form("%s_XY %s (FF+%d)", nameSubDir, posName[i].Data(), (Int_t)FF_plus[i]) :
    // 	Form("%s_YY %s", nameSubDir, posName[i].Data());
    //   sub_dir->Add(gHist.createTH2(unique_id++, title,
    // 				   400,-200,200, 200,-100,100,
    // 				   "x position [mm]", "y position [mm]"));
    // }

    // // Profile X Kaon
    // for(Int_t i = 0; i<NHist; ++i){
    //   char* title = i==i_Z0?
    // 	Form("BcOut K_X %s (FF+%d)", posName[i].Data(), (Int_t)FF_plus[i]) :
    // 	Form("BcOut K_X %s", posName[i].Data());
    //   sub_dir->Add(gHist.createTH1(unique_id++, title,
    // 				    400,-200,200,
    // 				    "x position [mm]", ""));
    // }
    // // Profile Y Kaon
    // for(Int_t i = 0; i<NHist; ++i){
    //   char* title = i==i_Z0?
    // 	Form("BcOut K_Y %s (FF+%d)", posName[i].Data(), (Int_t)FF_plus[i]) :
    // 	Form("BcOut K_Y %s", posName[i].Data());
    //   sub_dir->Add(gHist.createTH1(unique_id++, title,
    // 				   200,-100,100,
    // 				   "y position [mm]", ""));
    // }

    // BH2 Filter
#if !BH2FILTER
    // sub_dir->Add( gHist.createTH2( gHist.getUniqueID(kMisc, 2, kHitPat2D, 1),
    // 				   "BcOut_BH2_RAW",
    // 				   400,-200,200, NumOfSegBH2+1, 0., (Double_t)NumOfSegBH2+1.,
    // 				   "x position [mm]", "Segment" ) );
    sub_dir->Add( gHist.createTH1( gHist.getUniqueID(kMisc, 0, kMulti, 1),
    				   "BcOut_NTracks_RAW", 10, 0., 10. ) );
    sub_dir->Add( gHist.createTH1( gHist.getUniqueID(kMisc, 0, kChisqr, 1),
    				   "BcOut_Chisqr_RAW", 100, 0., 20. ) );
#endif
    // sub_dir->Add( gHist.createTH2( gHist.getUniqueID(kMisc, 2, kHitPat2D, 2),
    // 				   "BcOut_BH2",
    // 				   400,-200,200, NumOfSegBH2+1, 0., (Double_t)NumOfSegBH2+1.,
    // 				   "x position [mm]", "Segment" ) );
    sub_dir->Add( gHist.createTH1( gHist.getUniqueID(kMisc, 0, kMulti, 2),
    				   "BcOut_NTracks", 10, 0., 10. ) );
    sub_dir->Add( gHist.createTH1( gHist.getUniqueID(kMisc, 0, kChisqr, 2),
    				   "BcOut_Chisqr", 100, 0., 20. ) );
    tab_hist->Add(sub_dir);
  }


  //SdcOut
  {
    TList *top_dir = new TList;
    const char* nameDetector = "SdcOut";
    top_dir->SetName(nameDetector);

    {
      Int_t unique_id = gHist.getUniqueID(kMisc, 0, kHitPat, 100);
      // Profile X
      for(int i = 0; i<NHist_D1; ++i){
	char* title = Form("%s D1 %d_X", nameDetector, (int)D1_plus[i]);
	top_dir->Add(gHist.createTH1(unique_id++, title,
				     1000,-500,500,
				     "x position [mm]", ""));
      }
      // Profile Y
      for(int i = 0; i<NHist_D1; ++i){
	char* title = Form("%s D1 %d_Y", nameDetector, (int)D1_plus[i]);
	top_dir->Add(gHist.createTH1(unique_id++, title,
				     1000,-500,500,
				     "y position [mm]", ""));
      }
      tab_hist->Add(top_dir);
      // Profile XY
      for(int i = 0; i<NHist_D1; ++i){
	char* title = Form("%s D1 %d_XY", nameDetector, (int)D1_plus[i]);
	top_dir->Add(gHist.createTH2(unique_id++, title,
				     250,-500,500, 2500,-500,500,
				     "x position [mm]", "y position [mm]"));
      }
      tab_hist->Add(top_dir);


      // Profile X kaon
      for(int i = 0; i<NHist_D1; ++i){
	char* title = Form("%s D1 %d_X kaon", nameDetector, (int)D1_plus[i]);
	top_dir->Add(gHist.createTH1(unique_id++, title,
				     1000,-500,500,
				     "x position [mm]", ""));
      }
      // Profile Y kaon
      for(int i = 0; i<NHist_D1; ++i){
	char* title = Form("%s D1 %d_Y kaon", nameDetector, (int)D1_plus[i]);
	top_dir->Add(gHist.createTH1(unique_id++, title,
				     1000,-500,500,
				     "y position [mm]", ""));
      }
    }

    TList *sub_dir = new TList;
    const char* nameSubDir = "Residual";
    sub_dir->SetName(nameSubDir);
    {
      Int_t unique_id = gHist.getUniqueID(kMisc, 0, kHitPat, 200);
      for(int i = 0; i < NumOfLayersSdcOut; i++){
	char* title = Form("%s layer%d %s", nameDetector, i, nameSubDir);
	sub_dir->Add(gHist.createTH1(unique_id++, title,
				     200, -1, 1,
				     "Residual [mm]", ""));
      }
    }
    top_dir->Add(sub_dir);
  }

  {
    TList *sub_dir = new TList;
    const char* nameSubDir = "BH2Filter";
    sub_dir->SetName(nameSubDir);
    Int_t unique_id_wobh2 = gHist.getUniqueID(kMisc, 0, kHitPat2D);
    Int_t unique_id_wbh2  = gHist.getUniqueID(kMisc, 1, kHitPat2D);
    const Char_t* name_layer[] = { "BC3-x0", "BC3-x1", "BC3-v0", "BC3-v1", "BC3-u0", "BC3-u1",
				   "BC4-u0", "BC4-u1", "BC4-v0", "BC4-v1", "BC4-x0", "BC4-x1" };
    for( Int_t l=0; l<NumOfLayersBcOut; ++l ){
      sub_dir->Add( gHist.createTH2( unique_id_wobh2+l,
				     Form("%s_%s_RAW", nameSubDir, name_layer[l]),
				     100, -150., 150.,
				     NumOfSegBH2, 0., (Double_t)NumOfSegBH2,
				     "Wire Position [mm]", "Segment" ) );
    }
    for( Int_t l=0; l<NumOfLayersBcOut; ++l ){
      sub_dir->Add( gHist.createTH2( unique_id_wbh2+l,
				     Form("%s_%s_CUT", nameSubDir, name_layer[l]),
				     100, -150., 150.,
				     NumOfSegBH2, 0., (Double_t)NumOfSegBH2,
				     "Wire Position [mm]", "Segment" ) );
    }
    tab_hist->Add(sub_dir);
  }

  tab_hist->Add(gHist.createTriggerFlag());
  tab_hist->Add(gHist.createBH1());
  tab_hist->Add(gHist.createBFT());
  tab_hist->Add(gHist.createBC3());
  tab_hist->Add(gHist.createBC4());
  tab_hist->Add(gHist.createBH2());
  tab_hist->Add(gHist.createBAC());
  //tab_hist->Add(gHist.createPVAC());
  //tab_hist->Add(gHist.createFAC());
  int btof_id = gHist.getUniqueID(kMisc, 0, kTDC);
  tab_hist->Add(gHist.createTH1(btof_id, "BTOF",
                               800, -20, 20,
                               "BTOF [ns]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+1, "BTOFpi",
                               800, -20, 20,
                               "BTOF [ns]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+2, "BTOFk",
                               800, -20, 20,
                               "BTOF [ns]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+3, "BTOFp",
                               800, -20, 20,
                               "BTOF [ns]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+4, "BAC1eff [pi]",
                               6, 0, 6,
                               "Pattern", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+5, "BAC1eff [k]",
                               6, 0, 6,
                               "Pattern", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+6, "BAC1eff [p]",
                               6, 0, 6,
                               "Pattern", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+7, "BAC1 ADC",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+8, "BAC1 ADC wTDC",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+9, "BAC1 ADC [pi]",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+10, "BAC1 ADC [k]",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+11, "BAC1 ADC [p]",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+12, "X at E70TGT (FF+400)",
                               300, -150, 150,
                               "x [mm]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+13, "Y at E70TGT (FF+400)",
                               200, -50, 50,
                               "y [mm]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+14, "X at E70TGT [pi]",
                               300, -150, 150,
                               "x [mm]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+15, "Y at E70TGT [pi]",
                               200, -50, 50,
                               "y [mm]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+16, "X at E70TGT [k]",
                               300, -150, 150,
                               "x [mm]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+17, "Y at E70TGT [k]",
                               200, -50, 50,
                               "y [mm]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+18, "X at E70TGT [p]",
                               300, -150, 150,
                               "x [mm]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+19, "Y at E70TGT [p]",
                                200, -50, 50,
				"y [mm]", ""
				));

  tab_hist->Add(gHist.createTH2(btof_id+20, "XY at E70TGT [pi]",
                                150, -150, 150, 100, -50, 50,
				"x [mm]", "y [mm]"
				));
  tab_hist->Add(gHist.createTH2(btof_id+21, "XY at E70TGT [k]",
                                150, -150, 150, 100, -50, 50,
				"x [mm]", "y [mm]"
				));
  tab_hist->Add(gHist.createTH2(btof_id+22, "XY at E70TGT [p]",
                                150, -150, 150, 100, -50, 50,
				"x [mm]", "y [mm]"
				));




 tab_hist->Add(gHist.createTH1(btof_id+23, "BAC2eff [pi]",
                               6, 0, 6,
                               "Pattern", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+24, "BAC2eff [k]",
                               6, 0, 6,
                               "Pattern", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+25, "BAC2eff [p]",
                               6, 0, 6,
                               "Pattern", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+26, "BAC2 ADC",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+27, "BAC2 ADC wTDC",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+28, "BAC2 ADC [pi]",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+29, "BAC2 ADC [k]",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+30, "BAC2 ADC [p]",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));




  tab_hist->Add(gHist.createTH1(btof_id+23+8, "BAC1eff wBAC2 [pi]",
                               6, 0, 6,
                               "Pattern", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+24+8, "BAC1eff wBAC2 [k]",
                               6, 0, 6,
                               "Pattern", ""
                               ));
/*  tab_hist->Add(gHist.createTH1(btof_id+23+8, "PVACeff [pi]",
                               6, 0, 6,
                               "Pattern", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+24+8, "PVACeff [k]",
                               6, 0, 6,
                               "Pattern", ""
                               ));
*/
  tab_hist->Add(gHist.createTH1(btof_id+25+8, "PVACeff [p]",
                               6, 0, 6,
                               "Pattern", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+26+8, "PVAC ADC",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+27+8, "PVAC ADC wTDC",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+28+8, "PVAC ADC [pi]",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+29+8, "PVAC ADC [k]",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+30+8, "PVAC ADC [p]",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));




  tab_hist->Add(gHist.createTH1(btof_id+23+16, "FACeff [pi]",
                               6, 0, 6,
                               "Pattern", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+24+16, "FACeff [k]",
                               6, 0, 6,
                               "Pattern", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+25+16, "FACeff [p]",
                               6, 0, 6,
                               "Pattern", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+26+16, "FAC ADC",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+27+16, "FAC ADC wTDC",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+28+16, "FAC ADC [pi]",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+29+16, "FAC ADC [k]",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+30+16, "FAC ADC [p]",
                               200, 0, 2000,
                               "ADC [ch]", ""
                               ));


  tab_hist->Add(gHist.createTH1(btof_id+30+16+1, "BH1 chitpat kaon",
                               11, 0, 11,
                               "segment", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+30+16+2, "BFT U hitpat kaon",
                               160, 0, 160,
                               "segment", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+30+16+3, "BFT D hitpat kaon",
                               160, 0, 160,
                               "segment", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+30+16+4, "BC3x0 hitpat kaon",
                               64, 0, 64,
                               "segment", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+30+16+5, "BC4x0 hitpat kaon",
                               64, 0, 64,
                               "segment", ""
                               ));
  tab_hist->Add(gHist.createTH1(btof_id+30+16+6, "BH2 hitpat kaon",
                               8, 0, 8,
                               "segment", ""
                               ));



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
Int_t
process_end( void )
{
  return 0;
}

//____________________________________________________________________________
Int_t
process_event( void )
{
  //EventAnalyzer event;
  //event.DecodeRawData();
  //event.DecodeDCAnalyzer();
  //event.DecodeHodoAnalyzer();

  //const HodoAnalyzer* const hodoAna = event.GetHodoAnalyzer();
  //const DCAnalyzer* const dcAna = event.GetDCAnalyzer();

  static HistMaker& gHist = HistMaker::getInstance();
  static UnpackerManager& gUnpacker = GUnpacker::get_instance();
  static DCGeomMan&       gGeom     = DCGeomMan::GetInstance();

  int pi_flag = 0;
  int k_flag = 0;
  int p_flag = 0;
  int nhbh1_flag = 0;
  int nhbh2_flag = 0;
  double btof = -999;
  Int_t segBh2 = -1.;
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

    // TDC gate range
    static const UInt_t tdc_min_bh1 = gUser.GetParameter("TdcBH1", 0);
    static const UInt_t tdc_max_bh1 = gUser.GetParameter("TdcBH1", 1);
    static const UInt_t tdc_min_bh2 = gUser.GetParameter("TdcBH2", 0);
    static const UInt_t tdc_max_bh2 = gUser.GetParameter("TdcBH2", 1);


    // BH2
    //    double t0  = -999;
    double t0  = 1e10;
    double ofs = 0;
    //    double ofs_bh2seg[8] = {+0.15,
    //			    +0.05,
    //			    +0.15,
    //			    0.00,
    //			    -0.40,
    //			    -1.05,
    //			    -0.90,
    //			    -0.85};
    double ofs_bh2seg[8] = {0.00,
			    0.00,
			    0.00,
			    0.00,
			    0.00,
			    0.00,
			    0.00,
			    0.00};
    //  double ofs_bh1seg[11] = {1.20,
    double ofs_bh1seg[11] = {0.0,
			     0.00,
			     0.00,
			     0.00,
			     0.00,
			     0.00,
			     0.00,
			     0.00};

    //int seg = 0;
    // for(int seg=0;seg<NumOfSegBH2; ++seg){
    //   int nhitu = gUnpacker.get_entries(k_d_bh2, 0, seg, k_u, k_tdc);
    //   int nhitd = gUnpacker.get_entries(k_d_bh2, 0, seg, k_d, k_tdc);
    //   //if( nhitu != 0 ){
    //   for(int multi=0; multi<nhitu; multi++){
    // 	int tdcu = gUnpacker.get(k_d_bh2, 0, seg, k_u, k_tdc, multi);

    // 	//int tdcd = gUnpacker.get(k_d_bh2, 0, seg, k_d, k_tdc);
    // 	if( tdcu != 0 ){
    // 	  HodoParamMan& hodoMan = HodoParamMan::GetInstance();
    // 	  double bh2ut;
    // 	  hodoMan.GetTime(cid_bh2, plid, seg, k_u, tdcu, bh2ut);
    // 	  //hodoMan.GetTime(cid_bh2, plid, seg, k_d, tdcd, bh2dt);
    // 	  if( fabs(bh2ut)<fabs(t0) ){
    // 	    t0 = (bh2ut);
    // 	    ofs=ofs_bh2seg[seg];
    // 	    segBh2=seg;
    // 	  }
    // 	  if( -10 < bh2ut && bh2ut < 10 ){
    // 	    nhbh2_flag++;
    // 	  }
    // 	}//if(tdc)
    //   }// if(nhit)
    // }
    for(Int_t seg=0; seg<NumOfSegBH2; ++seg) {
      Int_t nhitu = gUnpacker.get_entries(k_d_bh2, 0, seg, kU, k_tdc);
      Int_t nhitd = gUnpacker.get_entries(k_d_bh2, 0, seg, kD, k_tdc);
      for(Int_t mu=0; mu<nhitu; ++mu) {
	auto tdcu = gUnpacker.get(k_d_bh2, 0, seg, kU, k_tdc, mu);
	if (tdcu < tdc_min_bh2 || tdc_max_bh2 < tdcu) continue;
	for(Int_t md=0; md<nhitd; ++md) {
	  auto tdcd = gUnpacker.get(k_d_bh2, 0, seg, kD, k_tdc, md);
	  if (tdcd < tdc_min_bh2 || tdc_max_bh2 < tdcd) continue;
	  HodoParamMan& hodoMan = HodoParamMan::GetInstance();
	  Double_t bh2ut, bh2dt;
	  hodoMan.GetTime(cid_bh2, plid, seg, kU, tdcu, bh2ut);
	  hodoMan.GetTime(cid_bh2, plid, seg, kD, tdcd, bh2dt);
	  Double_t bh2mt = (bh2ut + bh2dt)/2.;
	  if (TMath::Abs(t0) > TMath::Abs(bh2mt)) {
	    hodoMan.GetTime(cid_bh2, plid, seg, 2, 0, ofs);
	    //t0 = bh2ut;
	    t0 = bh2mt;
	    segBh2=seg;
	  }
	  if( -10 < bh2mt && bh2mt < 10 ){
	  //if( -500 < bh2mt && bh2mt < 500 ){
	    nhbh2_flag++;
     	  }
	}
      }
    }






    // BH1
    for(int seg = 0; seg<NumOfSegBH1; ++seg){
      int nhitu = gUnpacker.get_entries(k_d_bh1, 0, seg, k_u, k_tdc);
      int nhitd = gUnpacker.get_entries(k_d_bh1, 0, seg, k_d, k_tdc);
      //if(nhitu != 0 &&  nhitd != 0){
      for(int m1=0; m1<nhitu; m1++){
	for(int m2=0; m2<nhitd; m2++){
	  int tdcu = gUnpacker.get(k_d_bh1, 0, seg, k_u, k_tdc, m1);
	  int tdcd = gUnpacker.get(k_d_bh1, 0, seg, k_d, k_tdc, m2);
	  if(tdcu != 0 && tdcd != 0 ){
	    HodoParamMan& hodoMan = HodoParamMan::GetInstance();
	    double bh1tu, bh1td;
	    hodoMan.GetTime(cid_bh1, plid, seg, k_u, tdcu, bh1tu);
	    hodoMan.GetTime(cid_bh1, plid, seg, k_d, tdcd, bh1td);
	    if( fabs(bh1tu-bh1td)<5 ){
	      double mt = (bh1tu+bh1td)/2.;
	      mt = mt + ofs_bh1seg[seg];
	      if( fabs(mt)<10 ){
	      //if( fabs(mt)<1000 ){
		//double btof = mt-(t0+ofs)-0.07;
		//		btof = mt-(t0+ofs)-0.07-0.1;
		btof = mt-(t0+ofs);
		//hptr_array[btof_id]->Fill(btof);
		nhbh1_flag++;
	      }
	    }
	  }// if(tdc)
	}// for(m2)
      }// for(m1)
    }// for(seg)

    if( nhbh1_flag>0 && nhbh2_flag>0 && nhbh1_flag<3 && nhbh2_flag<3 ){
      hptr_array[btof_id]->Fill(btof);
    }
    if( nhbh1_flag>0 && nhbh2_flag>0 && nhbh1_flag<3 && nhbh2_flag<3 && -0.2<btof && btof<0.8 ){
      pi_flag=1;
      hptr_array[btof_id+1]->Fill(btof);
      hptr_array[btof_id+4]->Fill(0); //BAC1eff
      hptr_array[btof_id+4+19]->Fill(0); //BAC2eff

      static const int k_device = gUnpacker.get_device_id("BAC");
      static const int k_tdcbac2 = gUnpacker.get_data_id("BAC","tdc");
      int nhit_t2 = gUnpacker.get_entries(k_device, 0, 1, 0, k_tdcbac2);
      for(int m = 0; m<nhit_t2; ++m){
        int tdcbac2 = gUnpacker.get(k_device, 0, 1, 0, k_tdcbac2, m);
        static const int tdc_min = 720;
        static const int tdc_max = 760;
        if(tdc_min < tdcbac2 && tdcbac2 < tdc_max){
          hptr_array[btof_id+4+19+8]->Fill(0); //PVACeff->BAC1/BAC2 eff
        }
      }

      hptr_array[btof_id+4+19+16]->Fill(0); //PVAC2eff
    }
    if( nhbh1_flag>0 && nhbh2_flag>0 && nhbh1_flag<3 && nhbh2_flag<3 && -2.0<btof && btof<-0.7 ){  // 1.8GeV/c
    //if( nhbh1_flag>0 && nhbh2_flag>0 && nhbh1_flag<3 && nhbh2_flag<3 && -2.8<btof && btof<-1.8 ){  // 1.4GeV/c
      //if( nhbh1_flag>0 && nhbh2_flag>0 && nhbh1_flag<3 && nhbh2_flag<3 && -3.5<btof && btof<-2.5 ){  // 1.2GeV/c
      //if( nhbh1_flag>0 && nhbh2_flag>0 && nhbh1_flag<3 && nhbh2_flag<3 && -8<btof && btof<-4 ){  // 0.8GeV/c
      k_flag=1;
      hptr_array[btof_id+2]->Fill(btof);
      hptr_array[btof_id+5]->Fill(0); //BAC1eff
      hptr_array[btof_id+5+19]->Fill(0); //BAC2eff
      static const int k_device = gUnpacker.get_device_id("BAC");
      static const int k_tdcbac2 = gUnpacker.get_data_id("BAC","tdc");
      int nhit_t2 = gUnpacker.get_entries(k_device, 0, 1, 0, k_tdcbac2);
      for(int m = 0; m<nhit_t2; ++m){
        int tdcbac2 = gUnpacker.get(k_device, 0, 1, 0, k_tdcbac2, m);
        static const int tdc_min = 720;
        static const int tdc_max = 760;
        if(tdc_min > tdcbac2 || tdcbac2 > tdc_max){
          hptr_array[btof_id+5+19+8]->Fill(0); //PVACeff->BAC1/BAC2 eff
        }
      }
      hptr_array[btof_id+5+19+16]->Fill(0); //FACeff
    }
    // if( nhbh1_flag>0 && nhbh2_flag>0 && nhbh1_flag<3 && nhbh2_flag<3 && -4.7<btof && btof<-3.7 ){ // 1.8GeV/c
      //if( nhbh1_flag>0 && nhbh2_flag>0 && nhbh1_flag<3 && nhbh2_flag<3 && -4.7-2.8<btof && btof<-3.7-2.8 ){  // 1.4GeV/c
      //if( nhbh1_flag>0 && nhbh2_flag>0 && nhbh1_flag<3 && nhbh2_flag<3 && -4.7-5.0<btof && btof<-3.7-5.0 ){  // 1.2GeV/c
      if( nhbh1_flag>0 && nhbh2_flag>0 && nhbh1_flag<3 && nhbh2_flag<3 && -4.7-10.0<btof && btof<-3.7-1.3 ){  // 1.0GeV/c
      p_flag=1;
      hptr_array[btof_id+3]->Fill(btof);
      hptr_array[btof_id+6]->Fill(0); //BAC1eff
      hptr_array[btof_id+6+19]->Fill(0); //BAC2eff
      hptr_array[btof_id+6+19+8]->Fill(0); //PVACeff
      hptr_array[btof_id+6+19+8]->Fill(0); //FACeff
    }

  } // btof
  //-------------------------------------------------------------------



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
  }


  // BH1 -----------------------------------------------------------
  {
    // data type
    static const int k_device   = gUnpacker.get_device_id("BH1");
    static const int k_u        = 0; // up
    static const int k_d        = 1; // down
    static const int k_adc      = gUnpacker.get_data_id("BH1", "adc");
    static const int k_tdc      = gUnpacker.get_data_id("BH1", "tdc");

    // TDC gate range
    static const unsigned int tdc_min = gUser.GetParameter("TdcBH1", 0);
    static const unsigned int tdc_max = gUser.GetParameter("TdcBH1", 1);

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
      for(int m = 0; m<nhit; ++m){
  	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc, m);
  	if(tdc!=0){
  	  hptr_array[bh1t_id + seg]->Fill(tdc);
  	  // ADC wTDC_FPGA
  	  if( tdc>tdc_min && tdc<tdc_max && gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc)>0 ){
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
      for(int m = 0; m<nhit; ++m){
  	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc, m);
  	if( tdc!=0 ){
  	  hptr_array[bh1t_id + seg]->Fill(tdc);
  	  // ADC w/TDC_FPGA
  	  if( tdc>tdc_min && tdc<tdc_max && gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc)>0 ){
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
      //if(nhit_bh1u!=0 && nhit_bh1d!=0){
      for(int mu=0; mu<nhit_bh1u; mu++){
	for(int md=0; md<nhit_bh1d; md++){
	  UInt_t tdc_u = gUnpacker.get(k_device, 0, seg, k_u, k_tdc, mu);
	  UInt_t tdc_d = gUnpacker.get(k_device, 0, seg, k_d, k_tdc, md);
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
	      if(k_flag==1){
		static const int btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);
		hptr_array[btof_id+30+16+1]->Fill(seg);
	      }
	    }// TDC range OK
	  }// TDC AND
	}
      }// AND
    }// for(seg)

  hptr_array[bh1mul_id]->Fill(multiplicity);
  hptr_array[bh1mul_id+1]->Fill(cmultiplicity); // CMulti

  }// BH1

  std::vector<Int_t> hitseg_bftu;
  // // BFT -----------------------------------------------------------
  {
    static const auto device_id = gUnpacker.get_device_id("BFT");
    static const auto leading_id = gUnpacker.get_data_id("BFT", "leading");
    static const auto trailing_id = gUnpacker.get_data_id("BFT", "trailing");
    static const auto tdc_min = gUser.GetParameter("TdcBFT", 0);
    static const auto tdc_max = gUser.GetParameter("TdcBFT", 1);
    static const auto tdc_hid = gHist.getSequentialID(kBFT, 0, kTDC);
    static const auto tot_hid = gHist.getSequentialID(kBFT, 0, kADC);
    static const auto hit_hid = gHist.getSequentialID(kBFT, 0, kHitPat);
    static const auto mul_hid = gHist.getSequentialID(kBFT, 0, kMulti);
    static const auto tdc_2d_hid = gHist.getSequentialID(kBFT, 0, kTDC2D);
    static const auto tot_2d_hid = gHist.getSequentialID(kBFT, 0, kADC2D);
    UInt_t multiplicity = 0; // includes both u and d planes.
    for(Int_t i=0; i<NumOfSegBFT; ++i) {
      for(Int_t ud=0; ud<kUorD; ++ud) {
	UInt_t tdc_prev = 0;
	Bool_t is_in_range = false;
	for(Int_t m=0, n=gUnpacker.get_entries(device_id, ud, i, 0, leading_id);
	    m<n; ++m) {
	  auto tdc = gUnpacker.get(device_id, ud, i, 0, leading_id, m);
	  auto tdc_t = gUnpacker.get(device_id, ud, i, 0, trailing_id, m);
	  auto tot = tdc - tdc_t;
	  if (tdc_prev == tdc || tdc <= 0 || tot <= 0)
	    continue;
	  tdc_prev = tdc;
	  hptr_array[tdc_hid + ud]->Fill(tdc);
	  hptr_array[tot_hid + ud]->Fill(tot);
	  if (tdc_min < tdc && tdc < tdc_max) {
	    is_in_range = true;
	  }
	  hptr_array[tdc_2d_hid + ud]->Fill(i, tdc);
	  hptr_array[tot_2d_hid + ud]->Fill(i, tot);
	}
	if (is_in_range) {
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


  // BC3 -------------------------------------------------------------
  //for HULMHTDC
  std::vector< std::vector<int> > BC3HitCont(6);
  {
    // data type
    static const int k_device   = gUnpacker.get_device_id("BC3");
    static const int k_leading  = gUnpacker.get_data_id("BC3", "leading");
    static const int k_trailing = gUnpacker.get_data_id("BC3", "trailing");

    // TDC gate range
    // static const int tdc_min = gUser.GetParameter("BC3_TDC", 0);
    // static const int tdc_max = gUser.GetParameter("BC3_TDC", 1);
    static const Int_t tdc_min = gUser.GetParameter("TdcBC3", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcBC3", 1);
    // TOT gate range
    //static const int tot_min = gUser.GetParameter("BC3_TOT", 0);
    static const Int_t tot_min = gUser.GetParameter("MinTotBcOut", 0);

    // static const int tot_max = gUser.GetParameter("BC3_TOT", 1);

    // sequential id
    static const int bc3t_id    = gHist.getSequentialID(kBC3, 0, kTDC);
    static const int bc3tot_id  = gHist.getSequentialID(kBC3, 0, kADC);
    static const int bc3t1st_id = gHist.getSequentialID(kBC3, 0, kTDC2D);
    static const int bc3hit_id  = gHist.getSequentialID(kBC3, 0, kHitPat);
    static const int bc3mul_id  = gHist.getSequentialID(kBC3, 0, kMulti);
    static const int bc3mulwt_id
      = gHist.getSequentialID(kBC3, 0, kMulti, 1+NumOfLayersBC3);

    static const int bc3t_ctot_id    = gHist.getSequentialID(kBC3, 0, kTDC,    1+kTOTcutOffset);
    static const int bc3tot_ctot_id  = gHist.getSequentialID(kBC3, 0, kADC,    1+kTOTcutOffset);
    static const int bc3t1st_ctot_id = gHist.getSequentialID(kBC3, 0, kTDC2D,  1+kTOTcutOffset);
    static const int bc3hit_ctot_id  = gHist.getSequentialID(kBC3, 0, kHitPat, 1+kTOTcutOffset);
    static const int bc3mul_ctot_id  = gHist.getSequentialID(kBC3, 0, kMulti,  1+kTOTcutOffset);
    static const int bc3mulwt_ctot_id
      = gHist.getSequentialID(kBC3, 0, kMulti, 1+NumOfLayersBC3 + kTOTcutOffset);
    static const int bc3self_corr_id  = gHist.getSequentialID(kBC3, kSelfCorr, 0, 1);


    // TDC & HitPat & Multi
    for(int l=0; l<NumOfLayersBC3; ++l){
      int tdc                  = 0;
      int tdc_t                = 0;
      int tot                  = 0;
      int tdc1st               = 0;
      int multiplicity         = 0;
      int multiplicity_wt      = 0;
      int multiplicity_ctot    = 0;
      int multiplicity_wt_ctot = 0;
      for( int w=0; w<NumOfWireBC3; ++w ){
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
  	// hptr_array[bc3hit_id + l]->Fill(w, nhit);

  	bool flag_hit_wt = false;
  	bool flag_hit_wt_ctot = false;
  	for( int m = 0; m<nhit_l; ++m ){
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
	  if( k_flag==1 && l==0 ){
	    static const int btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);
	    hptr_array[btof_id+30+16+4]->Fill( w );
	  }
  	}

  	tdc1st = 0;
        if(nhit_l == nhit_t && hit_l_max > hit_t_max){
  	  ++multiplicity_ctot;
          for(int m = 0; m<nhit_l; ++m){
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


    for(int s=0; s<NumOfDimBC3 ;s++){
      int corr=2*s;
      for(unsigned int i=0; i<BC3HitCont[corr].size() ;i++){
        for(unsigned int j=0; j<BC3HitCont[corr+1].size() ;j++){
          hptr_array[bc3self_corr_id + s]->Fill(BC3HitCont[corr][i],BC3HitCont[corr+1][j]);
        }
      }
    }

  }


  // // BC4 -------------------------------------------------------------
  // //for HULMHTDC
  std::vector< std::vector<int> > BC4HitCont(6);
  {
    // data type
    static const int k_device   = gUnpacker.get_device_id("BC4");
    static const int k_leading  = gUnpacker.get_data_id("BC4", "leading");
    static const int k_trailing = gUnpacker.get_data_id("BC4", "trailing");

    // TDC gate range
    // static const int tdc_min = gUser.GetParameter("BC4_TDC", 0);
    // static const int tdc_max = gUser.GetParameter("BC4_TDC", 1);
    static const Int_t tdc_min = gUser.GetParameter("TdcBC4", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcBC4", 1);
    // TOT gate range
    //    static const int tot_min = gUser.GetParameter("BC4_TOT", 0);
    // static const int tot_max = gUser.GetParameter("BC4_TOT", 1);
    static const Int_t tot_min = gUser.GetParameter("MinTotBcOut", 0);

    // sequential id
    static const int bc4t_id    = gHist.getSequentialID(kBC4, 0, kTDC);
    static const int bc4tot_id  = gHist.getSequentialID(kBC4, 0, kADC);
    static const int bc4t1st_id = gHist.getSequentialID(kBC4, 0, kTDC2D);
    static const int bc4hit_id  = gHist.getSequentialID(kBC4, 0, kHitPat);
    static const int bc4mul_id  = gHist.getSequentialID(kBC4, 0, kMulti);
    static const int bc4mulwt_id
      = gHist.getSequentialID(kBC4, 0, kMulti, 1+NumOfLayersBC4);

    static const int bc4t_ctot_id    = gHist.getSequentialID(kBC4, 0, kTDC,    1+kTOTcutOffset);
    static const int bc4tot_ctot_id  = gHist.getSequentialID(kBC4, 0, kADC,    1+kTOTcutOffset);
    static const int bc4t1st_ctot_id = gHist.getSequentialID(kBC4, 0, kTDC2D,  1+kTOTcutOffset);
    static const int bc4hit_ctot_id  = gHist.getSequentialID(kBC4, 0, kHitPat, 1+kTOTcutOffset);
    static const int bc4mul_ctot_id  = gHist.getSequentialID(kBC4, 0, kMulti,  1+kTOTcutOffset);
    static const int bc4mulwt_ctot_id
      = gHist.getSequentialID(kBC4, 0, kMulti, 1+NumOfLayersBC4 + kTOTcutOffset);
    static const int bc4self_corr_id  = gHist.getSequentialID(kBC4, kSelfCorr, 0, 1);


    // TDC & HitPat & Multi
    for(int l=0; l<NumOfLayersBC4; ++l){
      int tdc                  = 0;
      int tdc_t                = 0;
      int tot                  = 0;
      int tdc1st               = 0;
      int multiplicity         = 0;
      int multiplicity_wt      = 0;
      int multiplicity_ctot    = 0;
      int multiplicity_wt_ctot = 0;
      for( int w=0; w<NumOfWireBC4; ++w ){
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
  	// hptr_array[bc4hit_id + l]->Fill(w, nhit);

  	bool flag_hit_wt = false;
  	bool flag_hit_wt_ctot = false;
  	for( int m = 0; m<nhit_l; ++m ){
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
	  if( k_flag==1 && l==4 ){
	    static const int btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);
	    hptr_array[btof_id+30+16+5]->Fill( w );
	  }
  	}

  	tdc1st = 0;
        if(nhit_l == nhit_t && hit_l_max > hit_t_max){
  	  ++multiplicity_ctot;
          for(int m = 0; m<nhit_l; ++m){
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


    for(int s=0; s<NumOfDimBC4 ;s++){
      int corr=2*s;
      for(unsigned int i=0; i<BC4HitCont[corr].size() ;i++){
        for(unsigned int j=0; j<BC4HitCont[corr+1].size() ;j++){
          hptr_array[bc4self_corr_id + s]->Fill(BC4HitCont[corr][i],BC4HitCont[corr+1][j]);
        }
      }
    }
  }


  // BH2 -----------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("BH2");
    static const int k_u      = 0; // up
    static const int k_d      = 1; // down
    static const int k_adc    = gUnpacker.get_data_id("BH2", "adc");
    //    static const int k_tdc    = gUnpacker.get_data_id("BH2", "leading");
    static const int k_tdc    = gUnpacker.get_data_id("BH2", "tdc");
    //    static const int k_mt     = gUnpacker.get_data_id("BH2", "fpga_meantime");

    // TDC gate range
    static const unsigned int tdc_min = gUser.GetParameter("TdcBH2", 0);
    static const unsigned int tdc_max = gUser.GetParameter("TdcBH2", 1);

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
      for(int m = 0; m<nhit; ++m){
  	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc, m);
  	if(tdc!=0){
  	  hptr_array[bh2t_id + seg]->Fill(tdc);
  	  // ADC w/TDC_FPGA
  	  if( tdc_min<tdc && tdc<tdc_max && gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc)>0){
  	    unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
  	    hptr_array[bh2awt_id + seg]->Fill(adc);
  	  }
  	}
      }
    }

    // DOWN
    bh2a_id   = gHist.getSequentialID(kBH2, 0, kADC,     NumOfSegBH2+1);
    bh2t_id   = gHist.getSequentialID(kBH2, 0, kTDC,     NumOfSegBH2+1);
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
      for(int m = 0; m<nhit; ++m){
  	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc, m);
  	if( tdc!=0 ){
  	  hptr_array[bh2t_id + seg]->Fill(tdc);
  	  // ADC w/TDC_FPGA
  	  if( tdc_min<tdc && tdc<tdc_max && gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc)>0){
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
      //int nhit_d = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      // AND
      //if( nhit_u!=0 ){
      for(int m=0; m<nhit_u;m++){
  	unsigned int tdc_u = gUnpacker.get(k_device, 0, seg, k_u, k_tdc, m);
  	//unsigned int tdc_d = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
  	// TDC AND
  	if( tdc_u!=0 ){
  	  //hptr_array[bh2hit_id]->Fill( seg );
  	  ++multiplicity;
	  if(true
	     && tdc_min < tdc_u && tdc_u < tdc_max
	     ){
	    //hptr_array[bh2hit_id+1]->Fill(seg); // CHitPat
	    hptr_array[bh2hit_id]->Fill(seg); // CHitPat
	    //++cmultiplicity;
	    if(k_flag==1){
	      static const int btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);
	      hptr_array[btof_id+30+16+6]->Fill(seg);
	    }
	  }// TDC range OK
  	}
      }
    }

    hptr_array[bh2mul_id]->Fill(multiplicity);

    // Mean Timer
    bh2t_id   = gHist.getSequentialID(kBH2, 0, kBH2MT, 1);
    // for(int seg=0; seg<NumOfSegBH2; ++seg){
    //   int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_mt);
    //   for(int m = 0; m<nhit; ++m){
    // 	unsigned int mt = gUnpacker.get(k_device, 0, seg, k_u, k_mt, m);
    // 	if(mt!=0){
    // 	  hptr_array[bh2t_id + seg]->Fill(mt);
    // 	}
    //   }
    // }
  }

  // BAC1 -----------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("BAC");
    static const int k_adc    = gUnpacker.get_data_id("BAC","adc");
    static const int k_tdc    = gUnpacker.get_data_id("BAC","tdc");

    // sequential id
    static const int baca_id   = gHist.getSequentialID(kBAC, 0, kADC,     1);
    static const int bact_id   = gHist.getSequentialID(kBAC, 0, kTDC,     1);
    static const int bacawt_id = gHist.getSequentialID(kBAC, 0, kADCwTDC, 1);
    static const int bach_id   = gHist.getSequentialID(kBAC, 0, kHitPat,  1);
    static const int bacm_id   = gHist.getSequentialID(kBAC, 0, kMulti,   1);
    static const int btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);
    // TDC gate range
    //static const int tdc_min = gUser.GetParameter("BAC_TDC", 0);
    //static const int tdc_max = gUser.GetParameter("BAC_TDC", 1);
    static const int tdc_min = 720;
    static const int tdc_max = 760;

    int multiplicity = 0;
    //for(int seg = 0; seg<NumOfSegBAC; ++seg){
    for(int seg = 0; seg<1; ++seg){
      // ADC
      int nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      if( nhit_a!=0 ){
  	int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
  	hptr_array[baca_id + seg]->Fill( adc );
  	if(nhbh1_flag==1&&nhbh2_flag==1){
  	  hptr_array[btof_id+7]->Fill(adc);
  	  if(pi_flag){
  	    hptr_array[btof_id+9]->Fill(adc);
  	  }
  	  if(k_flag){
  	    hptr_array[btof_id+10]->Fill(adc);
  	  }
  	  if(p_flag){
  	    hptr_array[btof_id+11]->Fill(adc);
  	  }
  	}
      }
      // TDC
      int nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      bool flag_t = false;

      for(int m = 0; m<nhit_t; ++m){
      	int tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
      	hptr_array[bact_id + seg]->Fill( tdc );

      	if(tdc_min < tdc && tdc < tdc_max){
      	  flag_t = true;
      	}// tdc range is ok
      }// for(m)

      if( flag_t ){
      	// ADC w/TDC
      	if( gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0 ){
      	  int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
      	  hptr_array[bacawt_id + seg]->Fill( adc );
      	  if(nhbh1_flag==1&&nhbh2_flag==1){
      	    hptr_array[btof_id+8]->Fill(adc);
      	  }
      	}
      	hptr_array[bach_id]->Fill(seg);
      	++multiplicity;

      	if( pi_flag==1 ){
      	  hptr_array[btof_id+4]->Fill(1); //BACeff

          static const int k_device2 = gUnpacker.get_device_id("BAC");
          static const int k_tdcbac2 = gUnpacker.get_data_id("BAC","tdc");
          int nhit_t2 = gUnpacker.get_entries(k_device, 0, 1, 0, k_tdcbac2);
          for(int m = 0; m<nhit_t2; ++m){
            int tdcbac2 = gUnpacker.get(k_device, 0, 1, 0, k_tdcbac2, m);
            static const int tdc2_min = 720;
            static const int tdc2_max = 760;
            if(tdc_min < tdcbac2 && tdcbac2 < tdc_max){
              hptr_array[btof_id+4+19+8]->Fill(1); //PVACeff->BAC1/BAC2 eff
            }
          }
      	}
      	if( k_flag==1 ){
      	  hptr_array[btof_id+5]->Fill(1); //BACeff

          static const int k_device2 = gUnpacker.get_device_id("BAC");
          static const int k_tdcbac2 = gUnpacker.get_data_id("BAC","tdc");
          int nhit_t2 = gUnpacker.get_entries(k_device, 0, 1, 0, k_tdcbac2);
          for(int m = 0; m<nhit_t2; ++m){
            int tdcbac2 = gUnpacker.get(k_device, 0, 1, 0, k_tdcbac2, m);
            static const int tdc2_min = 720;
            static const int tdc2_max = 760;
            if(tdc_min > tdcbac2 || tdcbac2 > tdc_max){
              hptr_array[btof_id+5+19+8]->Fill(1); //PVACeff->BAC1/BAC2 eff
            }
          }
      	}
      	if( p_flag==1 ){
      	  hptr_array[btof_id+6]->Fill(1); //BACeff
      	}

      }// flag is OK
    }

    hptr_array[bacm_id]->Fill( multiplicity );

  }//BAC1
  //---------------------------------------------------------------


  // BAC2 -----------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("BAC");
    static const int k_adc    = gUnpacker.get_data_id("BAC","adc");
    static const int k_tdc    = gUnpacker.get_data_id("BAC","tdc");

    // sequential id
    static const int baca_id   = gHist.getSequentialID(kBAC, 0, kADC,     1);
    static const int bact_id   = gHist.getSequentialID(kBAC, 0, kTDC,     1);
    static const int bacawt_id = gHist.getSequentialID(kBAC, 0, kADCwTDC, 1);
    static const int bach_id   = gHist.getSequentialID(kBAC, 0, kHitPat,  1);
    static const int bacm_id   = gHist.getSequentialID(kBAC, 0, kMulti,   1);
    static const int btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);
    // TDC gate range
    //static const int tdc_min = gUser.GetParameter("BAC_TDC", 0);
    //static const int tdc_max = gUser.GetParameter("BAC_TDC", 1);
    static const int tdc_min = 720;
    static const int tdc_max = 760;

    int multiplicity = 0;
    //for(int seg = 0; seg<NumOfSegBAC; ++seg){
    for(int seg = 1; seg<2; ++seg){
      // ADC
      int nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      if( nhit_a!=0 ){
  	int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
  	hptr_array[baca_id + seg]->Fill( adc );
  	if(nhbh1_flag==1&&nhbh2_flag==1){
  	  hptr_array[btof_id+7+19]->Fill(adc);
  	  if(pi_flag){
  	    hptr_array[btof_id+9+19]->Fill(adc);
  	  }
  	  if(k_flag){
  	    hptr_array[btof_id+10+19]->Fill(adc);
  	  }
  	  if(p_flag){
  	    hptr_array[btof_id+11+19]->Fill(adc);
  	  }
  	}
      }
      // TDC
      int nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      bool flag_t = false;

      for(int m = 0; m<nhit_t; ++m){
      	int tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
      	hptr_array[bact_id + seg]->Fill( tdc );

      	if(tdc_min < tdc && tdc < tdc_max){
      	  flag_t = true;
      	}// tdc range is ok
      }// for(m)

      if( flag_t ){
      	// ADC w/TDC
      	if( gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0 ){
      	  int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
      	  hptr_array[bacawt_id + seg]->Fill( adc );
      	  if(nhbh1_flag==1&&nhbh2_flag==1){
      	    hptr_array[btof_id+8+19]->Fill(adc);
      	  }
      	}
      	hptr_array[bach_id]->Fill(seg);
      	++multiplicity;

      	if( pi_flag==1 ){
      	  hptr_array[btof_id+4+19]->Fill(1); //BACeff
      	}
      	if( k_flag==1 ){
      	  hptr_array[btof_id+5+19]->Fill(1); //BACeff
      	}
      	if( p_flag==1 ){
      	  hptr_array[btof_id+6+19]->Fill(1); //BACeff
      	}

      }// flag is OK
    }

    hptr_array[bacm_id]->Fill( multiplicity );

  }//BAC2
  //---------------------------------------------------------------



  // PVAC -----------------------------------------------------------
  // {
  //   // data type
  //   static const int k_device = gUnpacker.get_device_id("PVAC");
  //   static const int k_adc    = gUnpacker.get_data_id("PVAC","adc");
  //   static const int k_tdc    = gUnpacker.get_data_id("PVAC","tdc");

  //   // sequential id
  //   static const int baca_id   = gHist.getSequentialID(kPVAC, 0, kADC,     1);
  //   static const int bact_id   = gHist.getSequentialID(kPVAC, 0, kTDC,     1);
  //   static const int bacawt_id = gHist.getSequentialID(kPVAC, 0, kADCwTDC, 1);
  //   static const int bach_id   = gHist.getSequentialID(kPVAC, 0, kHitPat,  1);
  //   static const int bacm_id   = gHist.getSequentialID(kPVAC, 0, kMulti,   1);
  //   static const int btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);
  //   // TDC gate range
  //   //static const int tdc_min = gUser.GetParameter("BAC_TDC", 0);
  //   //static const int tdc_max = gUser.GetParameter("BAC_TDC", 1);
  //   static const int tdc_min = 1130;
  //   static const int tdc_max = 1170;

  //   int multiplicity = 0;
  //   //for(int seg = 0; seg<NumOfSegBAC; ++seg){
  //   for(int seg = 0; seg<1; ++seg){
  //     // ADC
  //     int nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
  //     if( nhit_a!=0 ){
  // 	int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
  // 	hptr_array[baca_id + seg]->Fill( adc );
  // 	if(nhbh1_flag==1&&nhbh2_flag==1){
  // 	  hptr_array[btof_id+7+19+8]->Fill(adc);
  // 	  if(pi_flag){
  // 	    hptr_array[btof_id+9+19+8]->Fill(adc);
  // 	  }
  // 	  if(k_flag){
  // 	    hptr_array[btof_id+10+19+8]->Fill(adc);
  // 	  }
  // 	  if(p_flag){
  // 	    hptr_array[btof_id+11+19+8]->Fill(adc);
  // 	  }
  // 	}
  //     }
  //     // TDC
  //     int nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
  //     bool flag_t = false;

  //     for(int m = 0; m<nhit_t; ++m){
  //     	int tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
  //     	hptr_array[bact_id + seg]->Fill( tdc );

  //     	if(tdc_min < tdc && tdc < tdc_max){
  //     	  flag_t = true;
  //     	}// tdc range is ok
  //     }// for(m)

  //     if( flag_t ){
  //     	// ADC w/TDC
  //     	if( gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0 ){
  //     	  int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
  //     	  hptr_array[bacawt_id + seg]->Fill( adc );
  //     	  if(nhbh1_flag==1&&nhbh2_flag==1){
  //     	    hptr_array[btof_id+8+19+8]->Fill(adc);
  //     	  }
  //     	}
  //     	hptr_array[bach_id]->Fill(seg);
  //     	++multiplicity;

  //     	if( pi_flag==1 ){
  //     	  hptr_array[btof_id+4+19+8]->Fill(1); //BACeff
  //     	}
  //     	if( k_flag==1 ){
  //     	  hptr_array[btof_id+5+19+8]->Fill(1); //BACeff
  //     	}
  //     	if( p_flag==1 ){
  //     	  hptr_array[btof_id+6+19+8]->Fill(1); //BACeff
  //     	}

  //     }// flag is OK
  //   }

  //   hptr_array[bacm_id]->Fill( multiplicity );

  // }//PVAC
  //---------------------------------------------------------------



  // FAC -----------------------------------------------------------
  // {
  //   // data type
  //   static const int k_device = gUnpacker.get_device_id("FAC");
  //   static const int k_adc    = gUnpacker.get_data_id("FAC","adc");
  //   static const int k_tdc    = gUnpacker.get_data_id("FAC","tdc");

  //   // sequential id
  //   static const int baca_id   = gHist.getSequentialID(kFAC, 0, kADC,     1);
  //   static const int bact_id   = gHist.getSequentialID(kFAC, 0, kTDC,     1);
  //   static const int bacawt_id = gHist.getSequentialID(kFAC, 0, kADCwTDC, 1);
  //   static const int bach_id   = gHist.getSequentialID(kFAC, 0, kHitPat,  1);
  //   static const int bacm_id   = gHist.getSequentialID(kFAC, 0, kMulti,   1);
  //   static const int btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);
  //   // TDC gate range
  //   //static const int tdc_min = gUser.GetParameter("BAC_TDC", 0);
  //   //static const int tdc_max = gUser.GetParameter("BAC_TDC", 1);
  //   static const int tdc_min = 1130;
  //   static const int tdc_max = 1170;

  //   int multiplicity = 0;
  //   //for(int seg = 0; seg<NumOfSegBAC; ++seg){
  //   for(int seg = 0; seg<1; ++seg){
  //     // ADC
  //     int nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
  //     if( nhit_a!=0 ){
  // 	int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
  // 	hptr_array[baca_id + seg]->Fill( adc );
  // 	if(nhbh1_flag==1&&nhbh2_flag==1){
  // 	  hptr_array[btof_id+7+19+16]->Fill(adc);
  // 	  if(pi_flag){
  // 	    hptr_array[btof_id+9+19+16]->Fill(adc);
  // 	  }
  // 	  if(k_flag){
  // 	    hptr_array[btof_id+10+19+16]->Fill(adc);
  // 	  }
  // 	  if(p_flag){
  // 	    hptr_array[btof_id+11+19+16]->Fill(adc);
  // 	  }
  // 	}
  //     }
  //     // TDC
  //     int nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
  //     bool flag_t = false;

  //     for(int m = 0; m<nhit_t; ++m){
  //     	int tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
  //     	hptr_array[bact_id + seg]->Fill( tdc );

  //     	if(tdc_min < tdc && tdc < tdc_max){
  //     	  flag_t = true;
  //     	}// tdc range is ok
  //     }// for(m)

  //     if( flag_t ){
  //     	// ADC w/TDC
  //     	if( gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0 ){
  //     	  int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
  //     	  hptr_array[bacawt_id + seg]->Fill( adc );
  //     	  if(nhbh1_flag==1&&nhbh2_flag==1){
  //     	    hptr_array[btof_id+8+19+16]->Fill(adc);
  //     	  }
  //     	}
  //     	hptr_array[bach_id]->Fill(seg);
  //     	++multiplicity;

  //     	if( pi_flag==1 ){
  //     	  hptr_array[btof_id+4+19+16]->Fill(1); //BACeff
  //     	}
  //     	if( k_flag==1 ){
  //     	  hptr_array[btof_id+5+19+16]->Fill(1); //BACeff
  //     	}
  //     	if( p_flag==1 ){
  //     	  hptr_array[btof_id+6+19+16]->Fill(1); //BACeff
  //     	}

  //     }// flag is OK
  //   }

  //   hptr_array[bacm_id]->Fill( multiplicity );

  // }//PVAC
  //---------------------------------------------------------------



  ///////////////////////////////
  // original user_beamprofile //
  ///////////////////////////////
  /*
  Int_t nhBh2 = hodoAna->GetNHitsBH2();
  Int_t segBh2 = -1.;
  Double_t bh2mt = -9999.;
  for( Int_t i=0; i<nhBh2; ++i ){
    const BH2Hit* const hit = hodoAna->GetHitBH2(i);
    Int_t seg   = hit->SegmentId();
    Double_t mt = hit->MeanTime();
    if( TMath::Abs( mt ) < TMath::Abs( bh2mt ) ){
      bh2mt = mt;
      segBh2 = seg;
    }
    // if( nhBh2 == 1 ) segBh2 = seg;
    //static const Int_t hit_id = gHist.getSequentialID(kBH2, 0, kHitPat);
    //hptr_array[hit_id]->Fill( seg );
  }


  // BH2 % BcOut Hit
  //  if( segBh2 >= 0 && TMath::Abs( bh2mt ) < 0.1 ){
  {
    const std::vector<Double_t>& xmin = gBH2Filter.GetXmin( segBh2 );
    const std::vector<Double_t>& xmax = gBH2Filter.GetXmax( segBh2 );
    for( Int_t l=0; l<NumOfLayersBcOut; ++l ){
      const DCHitContainer& cont = dcAna->GetBcOutHC(l+1);
      Int_t nhOut = cont.size();
      for( Int_t i=0; i<nhOut; ++i ){
	const DCHit* const hit = cont.at(i);
	Double_t pos   = hit->GetWirePosition();
	static const Int_t bh2raw_id = gHist.getSequentialID(kMisc, 0, kHitPat2D);
	static const Int_t bh2cut_id = gHist.getSequentialID(kMisc, 1, kHitPat2D);
	hptr_array[bh2raw_id+l]->Fill( pos, segBh2 );
	if( xmin.at(l) < pos && pos < xmax.at(l) ){
	  hptr_array[bh2cut_id+l]->Fill( pos, segBh2 );
	}
      }
    }
  }
  */

  // BcOutTracking
#if !BH2FILTER
  // event.TrackSearchBcOut();
  // {
  //   Int_t ntBcOut = dcAna->GetNtracksBcOut();
  //   hptr_array[gHist.getSequentialID(kMisc, 0, kMulti, 1)]->Fill( ntBcOut );
  //   for( Int_t i=0; i<ntBcOut; ++i ){
  //     const DCLocalTrack* const track = dcAna->GetTrackBcOut(i);
  //     static const Int_t chisqr_id = gHist.getSequentialID(kMisc, 0, kChisqr, 1);
  //     Double_t chisqr = track->GetChiSquare();
  //     hptr_array[chisqr_id]->Fill( chisqr );
  //     if( !track || chisqr>10. ) continue;
  //     static const Int_t bh2raw_id = gHist.getSequentialID(kMisc, 2, kHitPat2D, 1);
  //     static const Double_t zBH2 = gGeom.GetLocalZ("BH2");
  //     hptr_array[bh2raw_id]->Fill( track->GetX(zBH2), segBh2 );
  //   }
  // }
#endif



 /////////// BcOut
  // {
  //   DCRHC BcOutAna(DetIdBcOut);
  //   bool BcOutTrack = BcOutAna.TrackSearch(9);

  //   if(BcOutTrack){
  //     //static const int xpos_id = gHist.getSequentialID(kMisc, 0, kHitPat);
  //     //static const int ypos_id = gHist.getSequentialID(kMisc, 0, kHitPat, NHist+1);
  //     //static const int xypos_id = gHist.getSequentialID(kMisc, 0, kHitPat, NHist*2+1);

  //     for(int i = 0; i<NHist; ++i){
  // 	double xpos = BcOutAna.GetPosX(0);
  // 	double ypos = BcOutAna.GetPosY(0);
  // 	//hptr_array[xpos_id+i]->Fill(xpos);
  // 	//hptr_array[ypos_id+i]->Fill(ypos);
  // 	//hptr_array[xypos_id+i]->Fill(xpos, ypos);
  //     }
  //   }
  // }

  //event.ApplyBH2Filter();
  // event.TrackSearchBcOut();
  // {
  //   Int_t ntBcOut = dcAna->GetNtracksBcOut();
  //   std::cout << ntBcOut << std::endl;
  //   Int_t cntBcOut = 0;
  //   // hptr_array[gHist.getSequentialID(kMisc, 0, kMulti, 2)]->Fill( ntBcOut );
  //   for( Int_t i=0; i<ntBcOut; ++i ){
  //     const DCLocalTrack* const track = dcAna->GetTrackBcOut(i);
  //     static const Int_t chisqr_id = gHist.getSequentialID(kMisc, 0, kChisqr, 2);
  //     Double_t chisqr = track->GetChiSquare();

  //     hptr_array[chisqr_id]->Fill( chisqr );
  //     if( !track || chisqr>10. ) continue;
  //     cntBcOut++;
  //     // Double_t x0 = track->GetX0(); Double_t y0 = track->GetY0();
  //     // Double_t u0 = track->GetU0(); Double_t v0 = track->GetV0();
  //     for( Int_t j=0; j<NHist; ++j ){
  // 	Double_t z = dist_FF+FF_plus+TgtOrg_plus[j];
  // 	Double_t x = track->GetX(z); Double_t y = track->GetY(z);
  // 	static const Int_t xpos_id   = gHist.getSequentialID(kMisc, 0, kHitPat);
  // 	static const Int_t ypos_id   = gHist.getSequentialID(kMisc, 0, kHitPat, NHist+1);
  // 	static const Int_t xypos_id  = gHist.getSequentialID(kMisc, 0, kHitPat, NHist*2+1);
  // 	static const Int_t kxpos_id   = gHist.getSequentialID(kMisc, 0, kHitPat, NHist*3+1);
  // 	static const Int_t kypos_id   = gHist.getSequentialID(kMisc, 0, kHitPat, NHist*4+1);
  // 	hptr_array[xpos_id+j]->Fill(x);
  // 	hptr_array[ypos_id+j]->Fill(y);
  // 	hptr_array[xypos_id+j]->Fill(x, y);
  // 	if( k_flag ){
  // 	  hptr_array[kxpos_id+j]->Fill(x);
  // 	  hptr_array[kypos_id+j]->Fill(y);
  // 	}
  //     }
  //     static const Int_t bh2cut_id = gHist.getSequentialID(kMisc, 2, kHitPat2D, 2);
  //     static const Double_t zBH2 = gGeom.GetLocalZ("BH2");
  //     hptr_array[bh2cut_id]->Fill( track->GetX(zBH2), segBh2 );

  //     //E03 target pos
  //     {
  // 	if( nhbh1_flag==1 && nhbh2_flag==1 ){
  // 	  static const int btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);
  // 	  Double_t z = dist_FF+650.;
  // 	  Double_t x = track->GetX(z); Double_t y = track->GetY(z);
  // 	  hptr_array[btof_id+12]->Fill(x);
  // 	  hptr_array[btof_id+13]->Fill(y);
  // 	  if( pi_flag ){
  // 	    hptr_array[btof_id+14]->Fill(x);
  // 	    hptr_array[btof_id+15]->Fill(y);
  // 	    hptr_array[btof_id+20]->Fill(x,y);
  // 	  }
  // 	  if( k_flag ){
  // 	    hptr_array[btof_id+16]->Fill(x);
  // 	    hptr_array[btof_id+17]->Fill(y);
  // 	    hptr_array[btof_id+21]->Fill(x,y);
  // 	  }
  // 	  if( p_flag ){
  // 	    hptr_array[btof_id+18]->Fill(x);
  // 	    hptr_array[btof_id+19]->Fill(y);
  // 	    hptr_array[btof_id+22]->Fill(x,y);
  // 	  }
  // 	}
  //     }
  //   }
  //   hptr_array[gHist.getSequentialID(kMisc, 0, kMulti, 2)]->Fill( cntBcOut );
  // }



  /////////// BcOut
  {
    DCRHC BcOutAna(DetIdBcOut);
    bool BcOutTrack = BcOutAna.TrackSearch(9);

    if(BcOutTrack){
      static const int xpos_id = gHist.getSequentialID(kMisc, 0, kHitPat);
      static const int ypos_id = gHist.getSequentialID(kMisc, 0, kHitPat, NHist_FF+1);
      static const int xypos_id = gHist.getSequentialID(kMisc, 0, kHitPat, NHist_FF*2+1);

      for(int i = 0; i<NHist_FF; ++i){
    	double xpos = BcOutAna.GetPosX(dist_FF+FF_plus[i]);
    	double ypos = BcOutAna.GetPosY(dist_FF+FF_plus[i]);
    	hptr_array[xpos_id+i]->Fill(xpos);
    	hptr_array[ypos_id+i]->Fill(ypos);
    	hptr_array[xypos_id+i]->Fill(xpos, ypos);

	if(k_flag==1){
	  hptr_array[xpos_id+i+(NHist_FF*3)]->Fill(xpos);
	  hptr_array[ypos_id+i+(NHist_FF*3)]->Fill(ypos);
	}
      } // for i

      //E70 target pos
      {
  	if( nhbh1_flag==1 && nhbh2_flag==1 ){
  	  static const int btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);
  	  Double_t z = dist_FF+400.;
  	  //Double_t x = track->GetX(z); Double_t y = track->GetY(z);
	  double x = BcOutAna.GetPosX(z);
	  double y = BcOutAna.GetPosY(z);

  	  hptr_array[btof_id+12]->Fill(x);
  	  hptr_array[btof_id+13]->Fill(y);
  	  if( pi_flag ){
  	    hptr_array[btof_id+14]->Fill(x);
  	    hptr_array[btof_id+15]->Fill(y);
  	    hptr_array[btof_id+20]->Fill(x,y);
  	  }
  	  if( k_flag ){
  	    hptr_array[btof_id+16]->Fill(x);
  	    hptr_array[btof_id+17]->Fill(y);
  	    hptr_array[btof_id+21]->Fill(x,y);
  	  }
  	  if( p_flag ){
  	    hptr_array[btof_id+18]->Fill(x);
  	    hptr_array[btof_id+19]->Fill(y);
  	    hptr_array[btof_id+22]->Fill(x,y);
  	  }
  	}
      }


    }
  }

  /////////// SdcOut
  {
    DCRHC SdcOutAna(DetIdSdcOut);
    bool SdcOutTrack = SdcOutAna.TrackSearch(9);

    if(SdcOutTrack){
      static const int residual_id = gHist.getSequentialID(kMisc, 0, kHitPat, 200);
      for( int layer = 0; layer < NumOfLayersSdcOut; layer++ ){
	Double_t res = SdcOutAna.GetResidualSdcOut(layer);
	hptr_array[residual_id+layer]->Fill(res);
      }

      static const int xpos_id = gHist.getSequentialID(kMisc, 0, kHitPat, 100);
      static const int ypos_id = gHist.getSequentialID(kMisc, 0, kHitPat, NHist_D1+100);
      static const int xypos_id = gHist.getSequentialID(kMisc, 0, kHitPat, NHist_D1*2+100);

      for(int i = 0; i<NHist_D1; ++i){
    	double xpos = SdcOutAna.GetPosX(dist_D1+D1_plus[i]);
    	double ypos = SdcOutAna.GetPosY(dist_D1+D1_plus[i]);
    	hptr_array[xpos_id+i]->Fill(xpos);
    	hptr_array[ypos_id+i]->Fill(ypos);
    	hptr_array[xypos_id+i]->Fill(xpos, ypos);

	if(k_flag==1){
	  hptr_array[xpos_id+i+(NHist_D1*3)]->Fill(xpos);
	  hptr_array[ypos_id+i+(NHist_D1*3)]->Fill(ypos);
	}
      } // for i

      // //E70 target pos
      // {
      // 	if( nhbh1_flag==1 && nhbh2_flag==1 ){
      // 	  static const int btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);
      // 	  Double_t z = dist_FF+400.;
      // 	  //Double_t x = track->GetX(z); Double_t y = track->GetY(z);
      // 	  double x = SdcOutAna.GetPosX(z);
      // 	  double y = SdcOutAna.GetPosY(z);

      // 	  hptr_array[btof_id+12]->Fill(x);
      // 	  hptr_array[btof_id+13]->Fill(y);
      // 	  if( pi_flag ){
      // 	    hptr_array[btof_id+14]->Fill(x);
      // 	    hptr_array[btof_id+15]->Fill(y);
      // 	    hptr_array[btof_id+20]->Fill(x,y);
      // 	  }
      // 	  if( k_flag ){
      // 	    hptr_array[btof_id+16]->Fill(x);
      // 	    hptr_array[btof_id+17]->Fill(y);
      // 	    hptr_array[btof_id+21]->Fill(x,y);
      // 	  }
      // 	  if( p_flag ){
      // 	    hptr_array[btof_id+18]->Fill(x);
      // 	    hptr_array[btof_id+19]->Fill(y);
      // 	    hptr_array[btof_id+22]->Fill(x,y);
      // 	  }
      // 	}
      // }


    }
  }



  return 0;
}

}
