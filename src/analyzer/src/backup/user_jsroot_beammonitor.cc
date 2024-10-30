// -*- C++ -*-

// Author: Shuhei Hayakawa

#include <iostream>
#include <string>
#include <vector>

#include <TAxis.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TGFileBrowser.h>
#include <TGraph.h>
#include <TH1.h>
#include <TLegend.h>
#include <TMath.h>
#include <TPaveText.h>
#include <TStyle.h>
#include <TString.h>

#include <THttpServer.h>
#include <TKey.h>
#include <TSystem.h>
#include <TTimeStamp.h>

#include <DAQNode.hh>
#include <filesystem_util.hh>
#include <Unpacker.hh>
#include <UnpackerManager.hh>

#include "user_analyzer.hh"

#include "ConfMan.hh"
#include "DetectorID.hh"
#include "DCDriftParamMan.hh"
#include "DCGeomMan.hh"
#include "DCTdcCalibMan.hh"
#include "HistMaker.hh"
#include "Updater.hh"
#include "UserParamMan.hh"

#include "HttpServer.hh"

namespace analyzer
{
using namespace hddaq::unpacker;
using namespace hddaq;

namespace
{
auto& gHttp = HttpServer::GetInstance();
auto& gUser = UserParamMan::GetInstance();
}

std::vector<TH1*> hptr_array;
enum eBeam { kKin, kPiin, kBH2, nBeam };
enum eDAQ  { kDAQEff, kL2Eff, kDuty, nDAQ };
enum eTOT  { kBFT_U, kBFT_D, kSCH, nTOT };

TString sBeam[nBeam] = { "K-in", "#pi-in", "BH2" };
TString sDAQ[nDAQ]   = { "DAQ-Eff", "L2-Eff", "Duty" };
TString sTOT[nTOT]   = { "BFT-U", "BFT-D", "SCH" };

TH1D* htot_bftu;
TH1D* htot_bftd;
TH1D* htot_sch;

TGraph  *g_beam[nBeam];
TGraph  *g_daq[nDAQ];
TGraph  *g_tot[nTOT];
TGraph  *g_reset[NumOfSegGe];
TLegend *leg_beam;
TLegend *leg_daq;
TLegend *leg_tot;
TLegend *leg_reset;
Color_t  col_beam[nBeam] = { kGreen, kBlue, kBlack };
Color_t  col_daq[nDAQ]   = { kRed, kOrange+1, kBlue };
Color_t  col_tot[nTOT]   = { kGreen, kGreen+1, kRed };
Color_t  col_reset[NumOfSegGe] = { kGreen, kGreen+1, kGreen+2, kGreen+3,
                                   kBlue, kBlue+1, kBlue+2, kBlue+3,
                                   kRed, kRed+1, kRed+2, kRed+3,
                                   kOrange, kOrange+1, kOrange+2, kOrange+3 };

//_____________________________________________________________________________
Double_t
GetTOT( TH1* h )
{
  if( !h || h->GetEntries()<10 )
    return 0.;
  TCanvas c;
  c.cd();
  TF1 f("f", "gaus", -10., 100.);
  Double_t p = h->GetBinCenter( h->GetMaximumBin() );
  if( p < 30. ) p = 60.;
  Double_t w = 10.;
  h->Fit( "f", "Q", "", p-w, p+w );
  for( Int_t ifit=0; ifit<3; ++ifit ){
    h->Fit( "f", "Q", "", p-w, p+w );
    p = f.GetParameter( 1 );
    w = f.GetParameter( 2 ) * 1.;
  }
  return f.GetParameter(1);
}

//____________________________________________________________________________
Int_t
process_begin( const std::vector<std::string>& argv )
{
  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  gConfMan.InitializeParameter<UserParamMan>("USER");
  if( !gConfMan.IsGood() ) return -1;
  // unpacker and all the parameter managers are initialized at this stage

  htot_bftu = new TH1D( "BFT_TOT_U", "BFT_TOT_U", 200, -50, 150 );
  htot_bftd = new TH1D( "BFT_TOT_D", "BFT_TOT_D", 200, -50, 150 );
  htot_sch  = new TH1D( "SCH_TOT", "SCH_TOT", 200, -50, 150 );

  TCanvas *c = new TCanvas( "beam_monitor", "beam_monitor" );
  c->Divide( 2, 2 );

  gHttp.SetPort( 9091 );
  gHttp.Open();
  gHttp.Register( c );

  Double_t legX = 0.13;
  Double_t legY = 0.13;
  Double_t legW = 0.14;
  Double_t legH = 0.26;

  // Beam Monitor
  {
    leg_beam = new TLegend( legX, legY, legX+legW+0.1, legY+legH+0.1 );
    leg_beam->SetTextSize(0.05);
    leg_beam->SetFillColor(0);
    leg_beam->SetBorderSize(4);
    for( Int_t i=0; i<nBeam; ++i ){
      c->cd(1)->SetGrid();
      gPad->SetTicky(2);
      g_beam[i] = new TGraph;
      g_beam[i]->SetName( "Beam" );
      g_beam[i]->SetTitle( "Beam" );
      g_beam[i]->SetMarkerStyle( 8 );
      g_beam[i]->SetMarkerSize( 1.5 );
      g_beam[i]->SetMarkerColor( col_beam[i] );
      g_beam[i]->SetLineWidth( 3 );
      g_beam[i]->SetLineColor( col_beam[i] );
      if(i==0) g_beam[i]->Draw( "AL" );
      else     g_beam[i]->Draw( "L" );
      g_beam[i]->SetPoint( 0, 0, 0 );
      leg_beam->AddEntry( g_beam[i], sBeam[i], "P" );
    }
    leg_beam->Draw();
  }

  // DAQ Monitor
  {
    leg_daq = new TLegend( legX, legY, legX+legW+0.1, legY+legH+0.1 );
    leg_daq->SetTextSize(0.05);
    leg_daq->SetFillColor(0);
    leg_daq->SetBorderSize(4);
    for( Int_t i=0; i<nDAQ; ++i ){
      c->cd(2)->SetGrid();
      gPad->SetTicky(2);
      g_daq[i] = new TGraph;
      g_daq[i]->SetTitle( "DAQ" );
      g_daq[i]->SetMarkerStyle( 8 );
      g_daq[i]->SetMarkerSize( 1.5 );
      g_daq[i]->SetMarkerColor( col_daq[i] );
      g_daq[i]->SetLineWidth( 3 );
      g_daq[i]->SetLineColor( col_daq[i] );
      if(i==0) g_daq[i]->Draw( "AL" );
      else     g_daq[i]->Draw( "L" );
      g_daq[i]->SetPoint( 0, 0, 0 );
      leg_daq->AddEntry( g_daq[i], sDAQ[i], "P" );
    }
    leg_daq->Draw();
  }

  // TOT Monitor
  {
    leg_tot = new TLegend( legX, legY, legX+legW, legY+legH );
    leg_tot->SetTextSize(0.05);
    leg_tot->SetFillColor(0);
    leg_tot->SetBorderSize(4);
    const Double_t vTOT[nTOT] = {
				 gUser.GetParameter( "BFT_TOTREF", 0 ),
				 gUser.GetParameter( "BFT_TOTREF", 1 ),
				 gUser.GetParameter( "SCH_TOTREF" ),
    };
    for( Int_t i=0; i<nTOT; ++i ){
      c->cd(3);
      gPad->SetGrid();
      gPad->SetTicky(2);
      g_tot[i] = new TGraph();
      g_tot[i]->SetTitle("TOT");
      g_tot[i]->SetName( sTOT[i] );
      g_tot[i]->SetMarkerStyle(8);
      g_tot[i]->SetMarkerSize(1.5);
      g_tot[i]->SetMarkerColor(col_tot[i]);
      g_tot[i]->SetLineWidth(3);
      g_tot[i]->SetLineColor(col_tot[i]);
      if(i==0) g_tot[i]->Draw("AL");
      else     g_tot[i]->Draw("L");
      g_tot[i]->SetPoint(0,0,0);
      leg_tot->AddEntry( g_tot[i], sTOT[i]+Form("  %.1f", vTOT[i]), "P" );
    }
    leg_tot->Draw();
  }

  // Reset Monitor
  {
    leg_reset = new TLegend( legX, legY, legX+legW+0.05, legY+legH+0.1 );
    leg_reset->SetTextSize( 0.05 );
    leg_reset->SetFillColor( 0 );
    leg_reset->SetBorderSize( 4 );
    for( Int_t i=0; i<NumOfSegGe; ++i ){
      TString name = Form( "Reset-%02d", i+1 );
      c->cd(4);
      gPad->SetGrid();
      gPad->SetTicky(2);
      g_reset[i] = new TGraph;
      g_reset[i]->SetTitle("Reset");
      g_reset[i]->SetName( name );
      g_reset[i]->SetMarkerStyle( 8 );
      g_reset[i]->SetMarkerSize( 1.5 );
      g_reset[i]->SetMarkerColor( col_reset[i] );
      g_reset[i]->SetLineWidth( 3 );
      g_reset[i]->SetLineColor( col_reset[i] );
      g_reset[i]->GetYaxis()->SetRangeUser( 0, 1000 );
      if(i==0) g_reset[i]->Draw( "AL" );
      else     g_reset[i]->Draw( "L" );
      g_reset[i]->SetPoint( 0, 0, 0 );
      leg_reset->AddEntry( g_reset[i], name, "P" );
    }
    leg_reset->SetNColumns(2);
    leg_reset->Draw();
  }

  gROOT->SetStyle( "Modern" );
  gStyle->SetTitleW( .4 );
  gStyle->SetTitleH( .11 );

  return 0;
}

//____________________________________________________________________________
Int_t
process_end()
{
  return 0;
}

//____________________________________________________________________________
Int_t
process_event( void )
{
  static UnpackerManager& gUnpacker = GUnpacker::get_instance();
  // static Int_t run_number = gUnpacker.get_root()->get_run_number();

  //___ Trigger Flag
  std::bitset<NumOfSegTFlag> trigger_flag;
  {
    static const auto k_device = gUnpacker.get_device_id( "TFlag" );
    static const auto k_tdc    = gUnpacker.get_data_id( "TFlag", "tdc" );
    for( Int_t seg=0; seg<NumOfSegTFlag; ++seg ){
      for( Int_t i=0, n=gUnpacker.get_entries( k_device, 0, seg, 0, k_tdc );
	   i<n; ++i ){
	auto tdc = gUnpacker.get( k_device, 0, seg, 0, k_tdc, i );
	if( tdc>0 ){
	  trigger_flag.set(seg);
	}
      }
    }
  }
  if( !trigger_flag[trigger::kSpillOnEnd] &&
      !trigger_flag[trigger::kL1SpillOn] ){
    return 0;
  }

  static const Int_t scaler_id = gUnpacker.get_device_id( "Scaler" );

  // Spill Increment
  static Int_t spill = 0;
  Bool_t spill_inc = false;
  {
    static const Int_t module_id  = 0;
    static const Int_t channel_id = 0;
    static Int_t clock     = 0;
    static Int_t clock_pre = 0;
    Int_t hit = gUnpacker.get_entries( scaler_id, module_id, 0, channel_id, 0 );
    if(hit>0){
      clock = gUnpacker.get( scaler_id, module_id, 0, channel_id, 0 );
      if( clock<clock_pre ) spill_inc = true;
    }
    clock_pre = clock;
  }

  // Beam Monitor
  {
    static const Int_t module_id[nBeam]  = {  0,  0,  0 };
    static const Int_t channel_id[nBeam] = { 39, 40, 17 };
    static Double_t beam[nBeam]     = {};
    static Double_t beam_pre[nBeam] = {};
    for( Int_t i=0; i<nBeam; ++i ){
      auto hit = gUnpacker.get_entries( scaler_id, module_id[i], 0,
                                        channel_id[i], 0 );
      if( hit == 0 ) continue;
      beam[i] = gUnpacker.get( scaler_id, module_id[i], 0, channel_id[i], 0 );
    }
    if( spill_inc ){
      for( Int_t i=0; i<nBeam; ++i ){
	g_beam[i]->SetPoint( spill, spill, beam_pre[i] );
	g_beam[i]->GetYaxis()->SetRangeUser( 0, 1.2e6 );
	g_beam[i]->GetXaxis()->SetLimits( spill-90, spill+10 );
      }
      Double_t kpi = beam_pre[kKin] / beam_pre[kPiin];
      leg_beam->SetHeader( Form("K/pi Ratio : %5.3lf", kpi ) );
    }
    for(Int_t i=0; i<nBeam; ++i) beam_pre[i] = beam[i];
  }

  // DAQ Monitor
  {
    enum eVal { kL1Req, kL1Acc, kL2Acc, kRealTime, kLiveTime, nVal };

    static const Int_t module_id[nVal]  = {  0,  0,  0,  0,  0 };
    static const Int_t channel_id[nVal] = {  3,  4,  8,  1,  2 };
    static Double_t val[nVal]     = {};
    static Double_t val_pre[nVal] = {};

    for(Int_t i=0; i<nVal; ++i){
      auto hit = gUnpacker.get_entries( scaler_id, module_id[i], 0,
                                        channel_id[i], 0 );
      if( hit==0 ) continue;
      val[i] = static_cast<Double_t>( gUnpacker.get( scaler_id, module_id[i], 0,
                                                     channel_id[i], 0 ) );
    }
    if( spill_inc ){
      Double_t daq_eff = val_pre[kL1Acc]/val_pre[kL1Req];
      Double_t l2_eff  = val_pre[kL2Acc]/val_pre[kL1Acc];
      Double_t real    = val_pre[kRealTime];
      Double_t live    = val_pre[kLiveTime];
      Double_t duty    = daq_eff/(1.-daq_eff)*(real/live-1.);
      if( TMath::IsNaN(daq_eff) )
	daq_eff = 1.;
      if( TMath::IsNaN(l2_eff) )
	l2_eff = 1.;
      if( duty>1. || TMath::IsNaN(duty) )
	duty = 1.;
      g_daq[kDAQEff]->SetPoint(spill, spill, daq_eff);
      g_daq[kDAQEff]->GetYaxis()->SetRangeUser(0, 1.05);
      g_daq[kDAQEff]->GetXaxis()->SetLimits(spill-90, spill+10);
      g_daq[kL2Eff]->SetPoint(spill, spill, l2_eff);
      g_daq[kDuty]->SetPoint(spill, spill, duty);
      leg_daq->SetHeader( Form("DAQ Eff. : %.3lf", daq_eff));
    }
    for( Int_t i=0; i<nVal; ++i ){
      val_pre[i] = val[i];
    }
  }

  // TOT Monitor
  {
    { // BFT
      static const Int_t k_device   = gUnpacker.get_device_id( "BFT" );
      static const Int_t k_uplane   = gUnpacker.get_plane_id( "BFT", "upstream" );
      static const Int_t k_dplane   = gUnpacker.get_plane_id( "BFT", "downstream" );
      static const Int_t k_leading  = gUnpacker.get_data_id( "BFT", "leading" );
      static const Int_t k_trailing = gUnpacker.get_data_id( "BFT", "trailing" );
      // static const int tdc_min = gUser.GetParameter("BFT_TDC", 0);
      // static const int tdc_max = gUser.GetParameter("BFT_TDC", 1);
      Int_t tdc_prev = 0;
      for( Int_t i=0; i<NumOfSegBFT; ++i ){
        Int_t nhit_u = gUnpacker.get_entries( k_device, k_uplane, 0, i, k_leading );
        Int_t nhit_d = gUnpacker.get_entries( k_device, k_dplane, 0, i, k_leading );
        // u plane
        tdc_prev = 0;
        for( Int_t m=0; m<nhit_u; ++m ){
          Int_t tdc = gUnpacker.get( k_device, k_uplane, 0, i, k_leading, m );
          Int_t tdc_t = gUnpacker.get( k_device, k_uplane, 0, i, k_trailing, m );
          Int_t tot = tdc - tdc_t;
          if( tdc_prev == tdc ) continue;
          tdc_prev = tdc;
          if( tot <= 0 ) continue;
          // if(tdc_min < tdc && tdc < tdc_max)
          htot_bftu->Fill( tot );
        }
        // d plane
        tdc_prev = 0;
        for( Int_t m = 0; m<nhit_d; ++m ){
          Int_t tdc = gUnpacker.get( k_device, k_dplane, 0, i, k_leading, m );
          Int_t tdc_t = gUnpacker.get( k_device, k_dplane, 0, i, k_trailing, m );
          Int_t tot = tdc - tdc_t;
          if( tdc_prev == tdc ) continue;
          tdc_prev = tdc;
          if( tot<=0 ) continue;
          // if(tdc_min < tdc && tdc < tdc_max)
          htot_bftd->Fill( tot );
        }
      }
    }
    { // SCH
      static const Int_t k_device   = gUnpacker.get_device_id( "SCH" );
      static const Int_t k_leading  = gUnpacker.get_data_id( "SCH", "leading" );
      static const Int_t k_trailing = gUnpacker.get_data_id( "SCH", "trailing" );
      // static const Int_t tdc_min = gUser.GetParameter( "SCH_TDC", 0 );
      // static const Int_t tdc_max = gUser.GetParameter( "SCH_TDC", 1 );
      for( Int_t i=0; i<NumOfSegSCH; ++i ){
	Int_t nhit = gUnpacker.get_entries( k_device, 0, i, 0, k_leading );
	for( Int_t m=0; m<nhit; ++m ){
	  Int_t tdc      = gUnpacker.get( k_device, 0, i, 0, k_leading,  m );
	  Int_t trailing = gUnpacker.get( k_device, 0, i, 0, k_trailing, m );
	  Int_t tot      = tdc - trailing;
	  //if( tdc_min<tdc && tdc<tdc_max )
          htot_sch->Fill( tot );
	}
      }
    }
    static const Int_t nspill = 20;
    if( spill_inc && spill%nspill == 0 ){
      static Double_t val[nTOT]     = {};
      static Double_t val_pre[nTOT] = {};
      val[kBFT_U] = GetTOT( htot_bftu );
      val[kBFT_D] = GetTOT( htot_bftd );
      val[kSCH] = GetTOT( htot_sch );
      static const Double_t vTOT[nTOT] = {
					  gUser.GetParameter( "BFT_TOTREF", 0 ),
					  gUser.GetParameter( "BFT_TOTREF", 1 ),
					  gUser.GetParameter( "SCH_TOTREF" ),
      };
      for( Int_t i=0; i<nTOT; ++i ){
	g_tot[i]->SetPoint( spill/nspill, spill/nspill, val_pre[i] - vTOT[i]);
	g_tot[i]->GetYaxis()->SetRangeUser( -5, 5 );
	g_tot[i]->GetXaxis()->SetLimits(spill/nspill-90, spill/nspill+10);
      }
      for( Int_t i=0; i<nTOT; ++i ) val_pre[i] = val[i];
      htot_bftu->Reset();
      htot_bftd->Reset();
      htot_sch->Reset();
    }
  }

  // Reset Monitor
  {
    static Double_t val[NumOfSegGe]     = {};
    static Double_t val_pre[NumOfSegGe] = {};
    // Double_t max_val = 0;
    for(Int_t i=0; i<NumOfSegGe; ++i){
      auto hit = gUnpacker.get_entries( scaler_id, 2, 0, i+32, 0 );
      if( hit == 0 ) continue;
      val[i] = static_cast<Double_t>( gUnpacker.get( scaler_id, 2, 0, i+32, 0 ) );
      // max_val = TMath::Max( max_val, val_pre[i] );
    }
    if( spill_inc ){
      for(Int_t i=0; i<NumOfSegGe; ++i){
        g_reset[i]->SetPoint(spill, spill, val_pre[i]);
        // g_reset[i]->GetYaxis()->SetRangeUser( 0, max_val*1.1 );
	g_reset[i]->GetYaxis()->SetRangeUser( 0, 1000 );
        g_reset[i]->GetXaxis()->SetLimits( spill-90, spill+10 );
      }
    }
    for( Int_t i=0; i<NumOfSegGe; ++i ){
      val_pre[i] = val[i];
    }
  }

  if( spill_inc ) spill++;

  return 0;
}

}
