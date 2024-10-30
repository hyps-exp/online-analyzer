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

#include "BH2Filter.hh"
#include "BH2Hit.hh"
#include "ConfMan.hh"
#include "DCAnalyzer.hh"
#include "DCDriftParamMan.hh"
#include "DCGeomMan.hh"
#include "DCHit.hh"
#include "DCLocalTrack.hh"
#include "DCTdcCalibMan.hh"
#include "DetectorID.hh"
#include "EventAnalyzer.hh"
#include "HistMaker.hh"
#include "HodoAnalyzer.hh"
#include "HodoParamMan.hh"
#include "HodoPHCMan.hh"
#include "MacroBuilder.hh"
#include "RawData.hh"
#include "UserParamMan.hh"

#define BH2FILTER 1


namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  namespace
  {
    const UnpackerManager& gUnpacker  = GUnpacker::get_instance();
    const HistMaker&       gHist      = HistMaker::getInstance();
    const DCGeomMan&       gGeom      = DCGeomMan::GetInstance();
          BH2Filter&       gBH2Filter = BH2Filter::GetInstance();

    const Double_t dist_FF = 1200.;

    std::vector<TH1*> hptr_array;

    enum HistName
      {
	// Inside Target
	i_Zn495, i_Zn338, i_Zn167_5, i_Z0, i_Z150, i_Z240,
	// Detector positions
	i_SFT, i_EG, i_DC1, i_SAC,
	NHist
      };
    static const Double_t TgtOrg_plus[] =
      {
	// Inside Target
	-495., -338., -167.5, 0, 150., 240.,
	// Detector
	400., 540., 733., 866.
      };

    static const TString posName[] =
      {
	// Inside Target
	"Tgt Z=-495", "Tgt Z=-338", "Tgt Z=-167.5", "Tgt Z=0",
	"Tgt Z=150",  "Tgt Z=240",
	// Detectors
	"SFT", "End guard", "DC1", "KbAC"
      };

    static Double_t FF_plus = 0;
  }

//____________________________________________________________________________
Int_t
process_begin( const std::vector<std::string>& argv )
{
  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  gConfMan.InitializeParameter<BH2Filter>("BH2FLT");
  gConfMan.InitializeParameter<DCGeomMan>("DCGEOM");
  gConfMan.InitializeParameter<DCTdcCalibMan>("TDCCALIB");
  gConfMan.InitializeParameter<DCDriftParamMan>("DRFTPM");
  gConfMan.InitializeParameter<HodoParamMan>("HDPRM");
  gConfMan.InitializeParameter<HodoPHCMan>("HDPHC");
  gConfMan.InitializeParameter<UserParamMan>("USER");
  if( !gConfMan.IsGood() ) return -1;
  // unpacker and all the parameter managers are initialized at this stage

  gBH2Filter.Print();
  // gBH2Filter.SetVerbose();

  FF_plus = UserParamMan::GetInstance().GetParameter("FF_PLUS", 0);
  std::cout << "#D : FF+" << FF_plus << std::endl;

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

  // Add histograms to the Hist tab
  HistMaker& gHist = HistMaker::getInstance();
  tab_hist->Add(gHist.createBH2());
  //BcOut
  {
    TList *sub_dir = new TList;
    const char* nameSubDir = "BcOut";
    sub_dir->SetName(nameSubDir);
    Int_t unique_id = gHist.getUniqueID(kMisc, 0, kHitPat);
    // Profile X
    for(Int_t i = 0; i<NHist; ++i){
      char* title = i==i_Z0?
	Form("%s_X %s (FF+%d)", nameSubDir, posName[i].Data(), (Int_t)FF_plus) :
	Form("%s_X %s", nameSubDir, posName[i].Data());
      sub_dir->Add(gHist.createTH1(unique_id++, title,
				    400,-200,200,
				    "x position [mm]", ""));
    }
    // Profile Y
    for(Int_t i = 0; i<NHist; ++i){
      char* title = i==i_Z0?
	Form("%s_Y %s (FF+%d)", nameSubDir, posName[i].Data(), (Int_t)FF_plus) :
	Form("%s_Y %s", nameSubDir, posName[i].Data());
      sub_dir->Add(gHist.createTH1(unique_id++, title,
				   200,-100,100,
				   "y position [mm]", ""));
    }
    tab_hist->Add(sub_dir);
    // Profile XY
    for(Int_t i = 0; i<NHist; ++i){
      char* title = i==i_Z0?
	Form("%s_XY %s (FF+%d)", nameSubDir, posName[i].Data(), (Int_t)FF_plus) :
	Form("%s_YY %s", nameSubDir, posName[i].Data());
      sub_dir->Add(gHist.createTH2(unique_id++, title,
				   400,-200,200, 200,-100,100,
				   "x position [mm]", "y position [mm]"));
    }
    // BH2 Filter
#if !BH2FILTER
    sub_dir->Add( gHist.createTH2( gHist.getUniqueID(kMisc, 2, kHitPat2D, 1),
    				   "BcOut_BH2_RAW",
    				   400,-200,200, NumOfSegBH2+1, 0, NumOfSegBH2+1,
    				   "x position [mm]", "Segment" ) );
    sub_dir->Add( gHist.createTH1( gHist.getUniqueID(kMisc, 0, kMulti, 1),
    				   "BcOut_NTracks_RAW", 10, 0., 10. ) );
    sub_dir->Add( gHist.createTH1( gHist.getUniqueID(kMisc, 0, kChisqr, 1),
    				   "BcOut_Chisqr_RAW", 100, 0., 20. ) );
#endif
    sub_dir->Add( gHist.createTH2( gHist.getUniqueID(kMisc, 2, kHitPat2D, 2),
    				   "BcOut_BH2",
    				   400,-200,200, NumOfSegBH2+1, 0, NumOfSegBH2+1,
    				   "x position [mm]", "Segment" ) );
    sub_dir->Add( gHist.createTH1( gHist.getUniqueID(kMisc, 0, kMulti, 2),
    				   "BcOut_NTracks", 10, 0., 10. ) );
    sub_dir->Add( gHist.createTH1( gHist.getUniqueID(kMisc, 0, kChisqr, 2),
    				   "BcOut_Chisqr", 100, 0., 20. ) );
    tab_hist->Add(sub_dir);
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
  EventAnalyzer event;
  event.DecodeRawData();
  event.DecodeDCAnalyzer();
  event.DecodeHodoAnalyzer();

  const HodoAnalyzer* const hodoAna = event.GetHodoAnalyzer();
  const DCAnalyzer* const dcAna = event.GetDCAnalyzer();

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
    static const Int_t hit_id = gHist.getSequentialID(kBH2, 0, kHitPat);
    hptr_array[hit_id]->Fill( seg );
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

  // BcOutTracking
#if !BH2FILTER
  event.TrackSearchBcOut();
  {
    Int_t ntBcOut = dcAna->GetNtracksBcOut();
    hptr_array[gHist.getSequentialID(kMisc, 0, kMulti, 1)]->Fill( ntBcOut );
    for( Int_t i=0; i<ntBcOut; ++i ){
      const DCLocalTrack* const track = dcAna->GetTrackBcOut(i);
      static const Int_t chisqr_id = gHist.getSequentialID(kMisc, 0, kChisqr, 1);
      Double_t chisqr = track->GetChiSquare();
      hptr_array[chisqr_id]->Fill( chisqr );
      if( !track || chisqr>10. ) continue;
      static const Int_t bh2raw_id = gHist.getSequentialID(kMisc, 2, kHitPat2D, 1);
      static const Double_t zBH2 = gGeom.GetLocalZ("BH2");
      hptr_array[bh2raw_id]->Fill( track->GetX(zBH2), segBh2 );
    }
  }
#endif

  event.ApplyBH2Filter();
  event.TrackSearchBcOut();
  {
    Int_t ntBcOut = dcAna->GetNtracksBcOut();
    Int_t cntBcOut = 0;
    // hptr_array[gHist.getSequentialID(kMisc, 0, kMulti, 2)]->Fill( ntBcOut );
    for( Int_t i=0; i<ntBcOut; ++i ){
      const DCLocalTrack* const track = dcAna->GetTrackBcOut(i);
      static const Int_t chisqr_id = gHist.getSequentialID(kMisc, 0, kChisqr, 2);
      Double_t chisqr = track->GetChiSquare();
      hptr_array[chisqr_id]->Fill( chisqr );
      if( !track || chisqr>10. ) continue;
      cntBcOut++;
      // Double_t x0 = track->GetX0(); Double_t y0 = track->GetY0();
      // Double_t u0 = track->GetU0(); Double_t v0 = track->GetV0();
      for( Int_t j=0; j<NHist; ++j ){
	Double_t z = dist_FF+FF_plus+TgtOrg_plus[j];
	Double_t x = track->GetX(z); Double_t y = track->GetY(z);
	static const Int_t xpos_id   = gHist.getSequentialID(kMisc, 0, kHitPat);
	static const Int_t ypos_id   = gHist.getSequentialID(kMisc, 0, kHitPat, NHist+1);
	static const Int_t xypos_id  = gHist.getSequentialID(kMisc, 0, kHitPat, NHist*2+1);
	hptr_array[xpos_id+j]->Fill(x);
	hptr_array[ypos_id+j]->Fill(y);
	hptr_array[xypos_id+j]->Fill(x, y);
      }
      static const Int_t bh2cut_id = gHist.getSequentialID(kMisc, 2, kHitPat2D, 2);
      static const Double_t zBH2 = gGeom.GetLocalZ("BH2");
      hptr_array[bh2cut_id]->Fill( track->GetX(zBH2), segBh2 );
    }
    hptr_array[gHist.getSequentialID(kMisc, 0, kMulti, 2)]->Fill( cntBcOut );
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  return 0;
}

}
