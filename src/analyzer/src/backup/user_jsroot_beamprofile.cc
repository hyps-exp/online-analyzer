// -*- C++ -*-

#include <iomanip>
#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TH1.h>
#include <TH2.h>
#include <THttpServer.h>
#include <TKey.h>
#include <TMath.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TText.h>
#include <TTimeStamp.h>

#include <user_analyzer.hh>
#include <Unpacker.hh>
#include <UnpackerManager.hh>
#include <DAQNode.hh>
#include <filesystem_util.hh>

#include "BH2Filter.hh"
#include "BH2Hit.hh"
#include "ConfMan.hh"
#include "DetectorID.hh"
#include "DCAnalyzer.hh"
#include "DCAnalyzerOld.hh"
#include "DCDriftParamMan.hh"
#include "DCGeomMan.hh"
#include "DCHit.hh"
#include "DCLocalTrack.hh"
#include "DCTdcCalibMan.hh"
#include "EMCParamMan.hh"
#include "EventAnalyzer.hh"
#include "FiberCluster.hh"
#include "FiberHit.hh"
#include "HistMaker.hh"
#include "HodoAnalyzer.hh"
#include "HodoParamMan.hh"
#include "HodoPHCMan.hh"
#include "HodoRawHit.hh"
#include "HttpServer.hh"
#include "MacroBuilder.hh"
#include "MatrixParamMan.hh"
#include "MsTParamMan.hh"
#include "PsMaker.hh"
#include "RawData.hh"
#include "UserParamMan.hh"

namespace analyzer
{
using namespace hddaq::unpacker;
using namespace hddaq;

namespace
{
std::vector<TH1*> hptr_array;
const auto& gUnpacker  = GUnpacker::get_instance();
// auto&       gBH2Filter = BH2Filter::GetInstance();
auto&       gHist      = HistMaker::getInstance();
auto&       gHttp      = HttpServer::GetInstance();
const auto& gUser      = UserParamMan::GetInstance();
const auto& gGeom      = DCGeomMan::GetInstance();
TCanvas* c1 = nullptr;
std::vector<TGraph*> g_array;
}

//____________________________________________________________________________
int
process_begin( const std::vector<std::string>& argv )
{
  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  // gConfMan.InitializeParameter<BH2Filter>("BH2FLT");
  gConfMan.InitializeParameter<DCGeomMan>("DCGEO");
  gConfMan.InitializeParameter<DCTdcCalibMan>("DCTDC");
  gConfMan.InitializeParameter<DCDriftParamMan>("DCDRFT");
  gConfMan.InitializeParameter<HodoParamMan>("HDPRM");
  gConfMan.InitializeParameter<HodoPHCMan>("HDPHC");
  gConfMan.InitializeParameter<UserParamMan>("USER");
  if( !gConfMan.IsGood() ) return -1;
  // unpacker and all the parameter managers are initialized at this stage

  // gBH2Filter.Print();

  gHttp.SetPort(9090);
  gHttp.Open();
  //BcOut
  gHttp.Register(gHist.createBH1());
  gHttp.Register(gHist.createBFT());
  gHttp.Register(gHist.createBC3());
  gHttp.Register(gHist.createBC4());
  gHttp.Register(gHist.createBH2());
  // gHttp.Register(gHist.createBH2_E42());
  gHttp.Register(gHist.createBeamProfileE42());

  if(0 != gHist.setHistPtr(hptr_array)){ return -1; }

  // Macro for HttpServer
  gHttp.Register(http::HitPatternBeam());
  for( ParticleType ip : { kKaon, kPion, kAll } ){
    gHttp.Register(http::BeamProfileX(ip));
    gHttp.Register(http::BeamProfileY(ip));
    gHttp.Register(http::BeamProfileXY(ip));
  }
  gHttp.Register(http::BeamProfileFF());
  for( Int_t i=0, n=hptr_array.size(); i<n; ++i ){
    TString name = hptr_array[i]->GetName();
    if( name.Contains("Kaon") )
      hptr_array[i]->SetLineColor( kGreen + 2 );
    else if( name.Contains("Pion") )
      hptr_array[i]->SetLineColor( kBlue + 2 );
    else
      hptr_array[i]->SetLineColor( kBlack );
    hptr_array[i]->SetLineWidth(2);
    hptr_array[i]->SetDirectory(0);
    // hptr_array[i]->SetStats(0);
  }

  // Graph
  c1 = new TCanvas("BeamEnvelope", "BeamEnvelope");
  c1->Divide(2, 2);
  g_array.push_back(new TGraphErrors);
  g_array.push_back(new TGraphErrors);
  g_array.push_back(new TGraphErrors);
  g_array.push_back(new TGraphErrors);
  g_array.push_back(new TGraphErrors);
  g_array.push_back(new TGraphErrors);
  g_array.push_back(new TGraphErrors);
  g_array.push_back(new TGraphErrors);
  g_array[0]->SetNameTitle("BeamProfile X Mean",
                           "BeamProfile X Mean");
  g_array[1]->SetNameTitle("BeamProfile X Fit Mean",
                           "BeamProfile X Fit Mean");
  g_array[2]->SetNameTitle("BeamProfile X RMS",
                           "BeamProfile X RMS");
  g_array[3]->SetNameTitle("BeamProfile X Fit Sigma",
                           "BeamProfile X Fit Sigma");
  g_array[4]->SetNameTitle("BeamProfile Y Mean",
                           "BeamProfile Y Mean");
  g_array[5]->SetNameTitle("BeamProfile Y Fit Mean",
                           "BeamProfile Y Fit Mean");
  g_array[6]->SetNameTitle("BeamProfile Y RMS",
                           "BeamProfile Y RMS");
  g_array[7]->SetNameTitle("BeamProfile Y Fit Sigma",
                           "BeamProfile Y Fit Sigma");
  for (Int_t i=0, n=g_array.size(); i<n; ++i) {
    g_array[i]->SetMarkerStyle(8);
    g_array[i]->SetMarkerSize(2);
    g_array[i]->SetLineWidth(2);
    if (i%2 == 0){
      g_array[i]->SetMarkerColor(kBlack);
      g_array[i]->SetLineColor(kBlack);
    }else{
      g_array[i]->SetMarkerColor(kRed + 1);
      g_array[i]->SetLineColor(kRed + 1);
    }
  }

  c1->cd(1); g_array[1]->Draw("ALP");// g_array[0]->Draw("LP");
  c1->cd(2); g_array[3]->Draw("ALP");// g_array[2]->Draw("LP");
  c1->cd(3); g_array[5]->Draw("ALP");// g_array[4]->Draw("LP");
  c1->cd(4); g_array[7]->Draw("ALP");// g_array[6]->Draw("LP");

  gHttp.Register(c1);

  // gStyle->SetOptStat(1110);
  // gStyle->SetTitleW(.4);
  // gStyle->SetTitleH(.4);
  // gStyle->SetStatW(.42);
  // gStyle->SetStatH(.3);

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
  static const Double_t MinTotBC3 = gUser.GetParameter("BC3_TOT");
  static const Double_t MinTotBC4 = gUser.GetParameter("BC4_TOT");

  RawData rawData;
  rawData.DecodeHits();
  HodoAnalyzer hodoAna;
  hodoAna.DecodeBH2Hits(&rawData);
  DCAnalyzer dcAna;
  dcAna.DecodeBcOutHits(&rawData);

  static Int_t run_number = -1;
  if( run_number != gUnpacker.get_root()->get_run_number() ){
    for( Int_t i=0, n=hptr_array.size(); i<n; ++i ){
      hptr_array[i]->Reset();
    }
    run_number = gUnpacker.get_root()->get_run_number();
  }

  // BH1 -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("BH1");
    static const Int_t k_u      = 0; // up
    static const Int_t k_d      = 1; // down
    static const Int_t k_tdc    = gUnpacker.get_data_id("BH1", "fpga_leading");
    // TDC gate range
    static const UInt_t tdc_min = gUser.GetParameter("BH1_TDC", 0);
    static const UInt_t tdc_max = gUser.GetParameter("BH1_TDC", 1);
    // Hit pattern && multiplicity
    static const Int_t bh1hit_id = gHist.getSequentialID(kBH1, 0, kHitPat);
    for (Int_t seg=0; seg<NumOfSegBH1; ++seg) {
      Int_t nhit_bh1u = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      Int_t nhit_bh1d = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      for ( Int_t mu=0; mu<nhit_bh1u; ++mu ) {
        for ( Int_t md=0; md<nhit_bh1d; ++md ) {
          UInt_t tdc_u = gUnpacker.get(k_device, 0, seg, k_u, k_tdc, mu);
          UInt_t tdc_d = gUnpacker.get(k_device, 0, seg, k_d, k_tdc, md);
          if (tdc_u != 0 && tdc_d != 0) {
            if (tdc_min < tdc_u && tdc_u < tdc_max &&
                tdc_min < tdc_d && tdc_d < tdc_max ){
              hptr_array[bh1hit_id]->Fill(seg);
            }
          }
        }
      }
    }
  }// BH1

  // BFT -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("BFT");
    static const Int_t k_uplane = gUnpacker.get_plane_id("BFT", "upstream");
    static const Int_t k_dplane = gUnpacker.get_plane_id("BFT", "downstream");
    static const Int_t k_leading = gUnpacker.get_data_id("BFT", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("BFT", "trailing");
    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("BFT_TDC", 0);
    static const Int_t tdc_max = gUser.GetParameter("BFT_TDC", 1);
    // sequential id
    static const Int_t bft_hitu_id  = gHist.getSequentialID(kBFT, 0, kHitPat,   1);
    static const Int_t bft_hitd_id  = gHist.getSequentialID(kBFT, 0, kHitPat,   2);

    Int_t tdc_prev      = 0;
    for (Int_t i = 0; i<NumOfSegBFT; ++i) {
      Int_t nhit_u = gUnpacker.get_entries(k_device, k_uplane, 0, i, k_leading);
      Int_t nhit_d = gUnpacker.get_entries(k_device, k_dplane, 0, i, k_leading);
      // u plane
      tdc_prev = 0;
      for(Int_t m = 0; m<nhit_u; ++m){
	Int_t tdc = gUnpacker.get(k_device, k_uplane, 0, i, k_leading, m);
	Int_t tdc_t = gUnpacker.get(k_device, k_uplane, 0, i, k_trailing, m);
	Int_t tot = tdc - tdc_t;
	if (tdc_prev == tdc) continue;
	tdc_prev = tdc;
	if( tot <= 0 ) continue;
	if (tdc_min < tdc && tdc < tdc_max) {
	  hptr_array[bft_hitu_id]->Fill(i);
	}
      }
      // d plane
      tdc_prev = 0;
      for(Int_t m = 0; m<nhit_d; ++m){
	Int_t tdc = gUnpacker.get(k_device, k_dplane, 0, i, k_leading, m);
	Int_t tdc_t = gUnpacker.get(k_device, k_dplane, 0, i, k_trailing, m);
	Int_t tot = tdc - tdc_t;
	if (tdc_prev == tdc) continue;
	tdc_prev = tdc;
	if (tot <= 0) continue;
	if(tdc_min < tdc && tdc < tdc_max){
	  hptr_array[bft_hitd_id]->Fill(i);
	}
      }
    }
  }

  // BC3 -------------------------------------------------------------
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
    // sequential id
    static const Int_t bc3hit_id  = gHist.getSequentialID(kBC3, 0, kHitPat);

    for (Int_t l=0; l<NumOfLayersBC3; ++l) {
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      for (Int_t w=0; w<NumOfWireBC3; ++w) {
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if (nhit_l == 0) continue;
        Int_t hit_l_max = 0;
        Int_t hit_t_max = 0;
        if (nhit_l != 0) {
          hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
        }
        if (nhit_t != 0) {
          hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
        }
	// Bool_t flag_hit_wt = false;
	// for (Int_t m = 0; m<nhit_l; ++m) {
	//   tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	//   if (tdc1st < tdc) tdc1st = tdc;
	//   if (tdc_min < tdc && tdc < tdc_max) {
	//     flag_hit_wt = true;
	//   }
	// }
	// if (flag_hit_wt) {
	//   hptr_array[bc3hit_id + l]->Fill( w );
	// }
	Bool_t flag_hit_wt_ctot = false;
	tdc1st = 0;
        if (nhit_l == nhit_t && hit_l_max > hit_t_max) {
          for(Int_t m = 0; m<nhit_l; ++m){
            tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
            tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
            tot = tdc - tdc_t;
            if (tot < tot_min) continue;
	    if (tdc1st < tdc) tdc1st = tdc;
	    if (tdc_min < tdc && tdc < tdc_max) {
	      flag_hit_wt_ctot = true;
	    }
          }
        }
	if (flag_hit_wt_ctot) {
	  hptr_array[bc3hit_id + l]->Fill(w);
        }
      }
    }
  }

  // BC4 -------------------------------------------------------------
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
    // sequential id
    static const Int_t bc4hit_id  = gHist.getSequentialID(kBC4, 0, kHitPat);

    for (Int_t l=0; l<NumOfLayersBC4; ++l) {
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      for (Int_t w=0; w<NumOfWireBC4; ++w) {
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if (nhit_l == 0) continue;
        Int_t hit_l_max = 0;
        Int_t hit_t_max = 0;
        if (nhit_l != 0) {
          hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
        }
        if (nhit_t != 0) {
          hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
        }
	// Bool_t flag_hit_wt = false;
	// for (Int_t m = 0; m<nhit_l; ++m) {
	//   tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	//   if (tdc1st < tdc) tdc1st = tdc;
	//   if (tdc_min < tdc && tdc < tdc_max) {
	//     flag_hit_wt = true;
	//   }
	// }
	// if (flag_hit_wt) {
	//   hptr_array[bc4hit_id + l]->Fill( w );
	// }
	Bool_t flag_hit_wt_ctot = false;
	tdc1st = 0;
        if (nhit_l == nhit_t && hit_l_max > hit_t_max) {
          for(Int_t m = 0; m<nhit_l; ++m){
            tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
            tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
            tot = tdc - tdc_t;
            if (tot < tot_min) continue;
	    if (tdc1st < tdc) tdc1st = tdc;
	    if (tdc_min < tdc && tdc < tdc_max) {
	      flag_hit_wt_ctot = true;
	    }
          }
        }
	if (flag_hit_wt_ctot) {
	  hptr_array[bc4hit_id + l]->Fill(w);
        }
      }
    }
  }

  // BH2 -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("BH2");
    static const Int_t k_u      = 0; // up
    // static const Int_t k_d      = 1; // down
    static const Int_t k_tdc    = gUnpacker.get_data_id("BH2", "fpga_leading");
    // TDC gate range
    static const UInt_t tdc_min = gUser.GetParameter("BH2_TDC", 0);
    static const UInt_t tdc_max = gUser.GetParameter("BH2_TDC", 1);
    // Hit pattern &&  Multiplicity
    static const Int_t bh2hit_id = gHist.getSequentialID(kBH2, 0, kHitPat);
    for (Int_t seg=0; seg<NumOfSegBH2; ++seg) {
      Int_t nhit_u = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      // Int_t nhit_d = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      // AND
      if( nhit_u!=0 /* && nhit_d!=0 */ ){
	UInt_t tdc_u = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	// UInt_t tdc_d = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
	// TDC AND
	if( tdc_u!=0 /* && tdc_d!=0 */ ){
	  hptr_array[bh2hit_id]->Fill(seg);
	}
      }
    }
  }

  // Trigger Flag
  Bool_t trig_flag[NParticleType];
  trig_flag[kKaon] = false;
  trig_flag[kPion] = true;
  trig_flag[kAll] = true;
#if 0
  {
    static const Int_t k_device = gUnpacker.get_device_id("TFlag");
    static const Int_t k_tdc    = gUnpacker.get_data_id("TFlag", "tdc");
    Int_t nhit_k = gUnpacker.get_entries( k_device, 0, trigger::kBeamTOF, 0, k_tdc );
    Int_t nhit_pi = gUnpacker.get_entries( k_device, 0, trigger::kBeamPi, 0, k_tdc );
    for( Int_t m = 0; m<nhit_k; ++m ){
      UInt_t tdc = gUnpacker.get( k_device, 0, trigger::kBeamTOF, 0, k_tdc, m );
      //if(tflag_tdc_min < tdc && tdc < tflag_tdc_max) pipi_flag = true;
      if(tdc!=0) trig_flag[kKaon] = true;//K- beam trigger
    }
    for( Int_t m = 0; m<nhit_pi; ++m ){
      UInt_t tdc = gUnpacker.get( k_device, 0, trigger::kBeamPi, 0, k_tdc, m );
      //if(tflag_tdc_min < tdc && tdc < tflag_tdc_max) pipi_flag = true;
      if(tdc!=0) trig_flag[kPion] = true;//pi- beam trigger
    }
  }
#endif

#if 0
  Int_t nhBh2 = HodoAna->GetNHitsBH2();
  Int_t segBh2 = -1.;
  Double_t bh2mt = -9999.;
  for( Int_t i=0; i<nhBh2; ++i ){
    const BH2Hit* const hit = HodoAna->GetHitBH2(i);
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
#endif

  dcAna.TotCutBC3( MinTotBC3 );
  dcAna.TotCutBC4( MinTotBC4 );
  // dcAna.TotCutBC3( 10 );
  // dcAna.TotCutBC4( 10 );

  Double_t bcout_nhits = 0;
  {
    // const std::vector<Double_t>& xmin = gBH2Filter.GetXmin( segBh2 );
    // const std::vector<Double_t>& xmax = gBH2Filter.GetXmax( segBh2 );
    for( Int_t l=0; l<NumOfLayersBcOut; ++l ){
      const DCHitContainer& cont = dcAna.GetBcOutHC(l+1);
      Int_t nhOut = cont.size();
      bcout_nhits += (Double_t)nhOut;
#if 0
      for( Int_t i=0; i<nhOut; ++i ){
	const DCHit* const hit = cont.at(i);
	Double_t pos   = hit->GetWirePosition();
	const Int_t bh2raw_id = gHist.getSequentialID(kMisc, ip, kHitPat2D, 1);
	// static const Int_t bh2cut_id = gHist.getSequentialID(kMisc, 1, kHitPat2D);
	// hptr_array[bh2raw_id+l]->Fill( pos, segBh2 );
	// if( xmin.at(l) < pos && pos < xmax.at(l) ){
	//   hptr_array[bh2cut_id+l]->Fill( pos, segBh2 );
	// }
      }
#endif
    }
  }
  if( (bcout_nhits/NumOfLayersBcOut) > 10 ){
    cout << "#D too much nhits: " << bcout_nhits << std::endl;
    return 0;
  }

  // event.ApplyBH2Filter();

  dcAna.TrackSearchBcOut();

  for( ParticleType ip : { kKaon, kPion, kAll } ){
    if( !trig_flag[ip] )
      continue;

    // BcOutTracking
    const Int_t nt_id = gHist.getSequentialID( kMisc, ip, kMulti, 1 );
    const Int_t chisqr_id = gHist.getSequentialID( kMisc, ip, kChisqr, 1 );
    const Int_t bh2hit_id = gHist.getSequentialID( kMisc, ip, kHitPat2D, 1 );
    const Int_t xpos_id = gHist.getSequentialID(kMisc, ip, kHitPat);
    const Int_t ypos_id = gHist.getSequentialID(kMisc, ip, kHitPat,
						NProfile*2+1);
    const Int_t xypos_id = gHist.getSequentialID(kMisc, ip, kHitPat,
						 NProfile*4+1);
    const Int_t uxpos_id = gHist.getSequentialID(kMisc, ip, kHitPat,
						 NProfile*5+1);
    const Int_t vypos_id = gHist.getSequentialID(kMisc, ip, kHitPat,
						 NProfile*5+2);
    const Double_t zBH2 = gGeom.GetLocalZ("BH2");
    Int_t ntBcOut = dcAna.GetNtracksBcOut();
    Int_t cntBcOut = 0;
    for( Int_t i=0; i<ntBcOut; ++i ){
      const DCLocalTrack* const track = dcAna.GetTrackBcOut(i);
      const Double_t chisqr = track->GetChiSquare();
      hptr_array[chisqr_id]->Fill( chisqr );
      if( !track || chisqr>10. ) continue;
      cntBcOut++;
      // hptr_array[bh2hit_id]->Fill( track->GetX(zBH2), segBh2 );
      for( Int_t j=0; j<NProfile; ++j ){
	const Double_t z = FFfromVO + Profiles[j];
	const Double_t x = track->GetX(z);
	const Double_t y = track->GetY(z);
	hptr_array[xpos_id+j]->Fill(x);
	hptr_array[ypos_id+j]->Fill(y);
	hptr_array[xpos_id+j+NProfile]->Fill(x);
	hptr_array[ypos_id+j+NProfile]->Fill(y);
	hptr_array[xypos_id+j]->Fill(x, y);
      }
      hptr_array[uxpos_id]->Fill( track->GetX0(), track->GetU0() );
      hptr_array[vypos_id]->Fill( track->GetY0(), track->GetV0() );
    }
    // hptr_array[nt_id]->Fill( ntBcOut );
    hptr_array[nt_id]->Fill( cntBcOut );

    if( gUnpacker.get_counter()%100 == 0 )
      http::UpdateBeamProfile(ip);
  }
  // }

  if( gUnpacker.get_counter()%100 == 0 ){
    for (Int_t i=0, n=g_array.size(); i<n; ++i) {
      g_array[i]->Set(0);
      g_array[i]->GetXaxis()->SetLimits(Profiles[0]-100,
                                        Profiles[NProfile-1]+100);
    }
    // g_array[0]->GetYaxis()->SetRangeUser(-20, 20);
    // g_array[1]->GetYaxis()->SetRangeUser(-20, 20);
    // g_array[2]->GetYaxis()->SetRangeUser(0, 40);
    // g_array[3]->GetYaxis()->SetRangeUser(0, 40);
    // g_array[4]->GetYaxis()->SetRangeUser(-20, 20);
    // g_array[5]->GetYaxis()->SetRangeUser(-20, 20);
    // g_array[6]->GetYaxis()->SetRangeUser(0, 40);
    // g_array[7]->GetYaxis()->SetRangeUser(0, 40);
    for (Int_t j=0; j<NProfile; ++j) {
      g_array[0]->SetPoint(j, Profiles[j], http::GetEnvelopeXMean()[j]);
      g_array[1]->SetPoint(j, Profiles[j], http::GetEnvelopeXfitMean()[j]);
      g_array[2]->SetPoint(j, Profiles[j], http::GetEnvelopeXRMS()[j]);
      g_array[3]->SetPoint(j, Profiles[j], http::GetEnvelopeXfitSigma()[j]);
      g_array[4]->SetPoint(j, Profiles[j], http::GetEnvelopeYMean()[j]);
      g_array[5]->SetPoint(j, Profiles[j], http::GetEnvelopeYfitMean()[j]);
      g_array[6]->SetPoint(j, Profiles[j], http::GetEnvelopeYRMS()[j]);
      g_array[7]->SetPoint(j, Profiles[j], http::GetEnvelopeYfitSigma()[j]);
    }
    for (Int_t i=0, n=g_array.size(); i<n; ++i) {
      g_array[i]->GetYaxis()->UnZoom();
      // c1->cd(i + 1)->UnZoomed();
    }
    c1->Update();
  }

  return 0;
}

}
