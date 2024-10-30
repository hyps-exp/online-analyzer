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
//#include "DCAnalyzerOld.hh"
#include "DCAnalyzer.hh"
#include "DCHit.hh"
#include "DCLocalTrack.hh"
#include "EventAnalyzer.hh"
#include "DCDriftParamMan.hh"
#include "DCGeomMan.hh"
#include "DCTdcCalibMan.hh"
#include "DetectorID.hh"
#include "HistMaker.hh"
#include "MacroBuilder.hh"
#include "RawData.hh"
#include "UserParamMan.hh"
#include "HodoPHCMan.hh"
#include "HodoParamMan.hh"




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
    const Int_t NumTrigflag=3;
    const Int_t NumHist=(3*NHist+1);//X, Y, XY, U
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
    gConfMan.InitializeParameter<HodoParamMan>("HDPRM");
    gConfMan.InitializeParameter<HodoPHCMan>("HDPHC");
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
    tab_macro->Add(macro::Get("dispBeamProfile_e42_Kbeam"));
    tab_macro->Add(macro::Get("dispBeamProfile_e42_pibeam"));
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


      sub_dir->Add( gHist.createTH1( gHist.getUniqueID(kMisc, 0, kMulti, 1),
				     "BcOut_NTracks_RAW", 10, 0, 10 ) );
      sub_dir->Add( gHist.createTH1( gHist.getUniqueID(kMisc, 0, kChisqr, 1),
				     "BcOut_Chisqr_RAW", 100, 0, 20 ) );

      TString Trig_flag[3] = {"No flag", "K-Beam", "pi-Beam"};
      const char* Tf_name = NULL;

      for(int itr=0; itr<NumTrigflag; ++itr){
	Tf_name = Trig_flag[itr].Data();
	// Profile X
	for(int i = 0; i<NHist; ++i){
	  const char* title = NULL;
	  title = Form("%s FF %d_X(%s)", nameSubDir, (int)FF_plus[i], Tf_name);
	  sub_dir->Add(gHist.createTH1(unique_id++, title,
				       400,-200,200,
				       "x position [mm]", ""));
	}
	// Profile Y
	for(int i = 0; i<NHist; ++i){
	  const char* title = NULL;
	  title = Form("%s FF %d_Y(%s)", nameSubDir, (int)FF_plus[i], Tf_name);
	  sub_dir->Add(gHist.createTH1(unique_id++, title,
				       200,-100,100,
				       "y position [mm]", ""));
	}
	tab_hist->Add(sub_dir);
	// Profile XY
	for(int i = 0; i<NHist; ++i){
	  const char* title = NULL;
	  title = Form("%s FF %d_XY(%s)", nameSubDir, (int)FF_plus[i], Tf_name);
	  sub_dir->Add(gHist.createTH2(unique_id++, title,
				       400,-200,200, 200,-100,100,
				       "x position [mm]", "y position [mm]"));
	}
	tab_hist->Add(sub_dir);

	const char* title = NULL;
	title = Form("XU(%s)", Tf_name);
	sub_dir->Add(gHist.createTH2(unique_id++, title,
				     400,-200,200, 2000,-1,1,
				     "x0 [mm]", "u"));



	tab_hist->Add(sub_dir);
      }
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
  process_end( void )
  {
    return 0;
  }

  //____________________________________________________________________________
  int
  process_event( void )
  {
    EventAnalyzer event;
    event.DecodeRawData();
    event.DecodeDCAnalyzer();
    const DCAnalyzer* const dcAna = event.GetDCAnalyzer();
    static HistMaker& gHist = HistMaker::getInstance();
    static UnpackerManager& gUnpacker = GUnpacker::get_instance();
    static DCGeomMan&       gGeom     = DCGeomMan::GetInstance();

    // Trigger flag selection

    bool trig_flag[3]={false, false, false};
    trig_flag[0]=true;//no trigger flag
    {
      static const int k_device = gUnpacker.get_device_id("TFlag");
      static const int k_tdc    = gUnpacker.get_data_id("TFlag", "tdc");

      int nhit_K = gUnpacker.get_entries( k_device, 0, trigger::kBeamTOF, 0, k_tdc );
      int nhit_pi = gUnpacker.get_entries( k_device, 0, trigger::kBeamPi, 0, k_tdc );
      for(int m = 0; m<nhit_K; ++m){
	int tdc = gUnpacker.get( k_device, 0, trigger::kBeamTOF, 0, k_tdc, m );
	//if(tflag_tdc_min < tdc && tdc < tflag_tdc_max) pipi_flag = true;
	if(tdc!=0) trig_flag[1] = true;//K- beam trigger
      }
      for(int m = 0; m<nhit_pi; ++m){
	int tdc = gUnpacker.get( k_device, 0, trigger::kBeamPi, 0, k_tdc, m );
	//if(tflag_tdc_min < tdc && tdc < tflag_tdc_max) pipi_flag = true;
	if(tdc!=0) trig_flag[2] = true;//pi- beam trigger
      }
    }



    /////////// BcOut
    event.TrackSearchBcOut();
    {
      Int_t ntBcOut = dcAna->GetNtracksBcOut();
      Int_t cntBcOut = 0;
      hptr_array[gHist.getSequentialID(kMisc, 0, kMulti, 1)]->Fill( ntBcOut );
      // hptr_array[gHist.getSequentialID(kMisc, 0, kMulti, 2)]->Fill( ntBcOut );

      for( Int_t i=0; i<ntBcOut; ++i ){
	const DCLocalTrack* const track = dcAna->GetTrackBcOut(i);
	static const Int_t chisqr_id = gHist.getSequentialID(kMisc, 0, kChisqr, 1);
	Double_t chisqr = track->GetChiSquare();
	hptr_array[chisqr_id]->Fill( chisqr );
	if( !track || chisqr>10. ) continue;
	cntBcOut++;
	// Double_t x0 = track->GetX0(); Double_t y0 = track->GetY0();
	// Double_t u0 = track->GetU0(); Double_t v0 = track->GetV0();
	for(int itr=0; itr<NumTrigflag; ++itr){
	  for( Int_t j=0; j<NHist; ++j ){
	    Double_t z = dist_FF+FF_plus[j];
	    Double_t x = track->GetX(z); Double_t y = track->GetY(z);


	    static const Int_t xpos_id   = gHist.getSequentialID(kMisc, 0, kHitPat);
	    static const Int_t ypos_id   = gHist.getSequentialID(kMisc, 0, kHitPat, NHist+1);
	    static const Int_t xypos_id  = gHist.getSequentialID(kMisc, 0, kHitPat, NHist*2+1);

	    if(trig_flag[itr]){
	      hptr_array[(NumHist*itr)+xpos_id+j]->Fill(x);
	      hptr_array[(NumHist*itr)+ypos_id+j]->Fill(y);
	      hptr_array[(NumHist*itr)+xypos_id+j]->Fill(x, y);
	    }
	  }

	  static const int xu_id = gHist.getSequentialID(kMisc, 0, kHitPat, NHist*3+1);
	  Double_t x0 = track->GetX0();
	  Double_t u0 = track->GetU0();
	  if(trig_flag[itr])
	    hptr_array[(NumHist*itr)+xu_id]->Fill(x0, u0);
	}
      }
      //hptr_array[gHist.getSequentialID(kMisc, 0, kMulti, 2)]->Fill( cntBcOut );
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
