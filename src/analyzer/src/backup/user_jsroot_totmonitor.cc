// -*- C++ -*-

// Author: Shuhei Hayakawa

#include <iostream>
#include <string>
#include <vector>

#include <TAxis.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TGFileBrowser.h>
#include <TH1.h>
#include <TH2.h>
#include <TGraph.h>
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
#include "HodoParamMan.hh"
#include "Updater.hh"
#include "UserParamMan.hh"

#include "HttpServer.hh"

#define CFT 0

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  namespace
  {
    HttpServer&   gHttp = HttpServer::GetInstance();
    UserParamMan& gUser = UserParamMan::GetInstance();
    HistMaker&    gHist = HistMaker::getInstance();
    std::vector<TH1*> hptr_array;
    enum eBeam { kBFT_U, kBFT_D, kSFT_U, kSFT_V, kSFT_X, kSFT_XP,
		 kSCH_1_16, kSCH_17_64,
		 kFHT1_UX1, kFHT1_DX1, kFHT1_UX2, kFHT1_DX2,
		 kFHT2_UX1, kFHT2_DX1, kFHT2_UX2, kFHT2_DX2,
#if CFT
		 kCFT_UV1, kCFT_PHI1, kCFT_UV2, kCFT_PHI2,
		 kCFT_UV3, kCFT_PHI3, kCFT_UV4, kCFT_PHI4,
#endif
		 nTOT };
    enum eDAQ  { kDAQEff, kL2Eff, kDuty, nDAQ };
    TString sTOT[nTOT] = { "BFT-U", "BFT-D",
			   "SFT-U", "SFT-V", "SFT-X", "SFT-XP",
			   "SCH-1-16", "SCH-17-64",
			   "FHT1-UX1", "FHT1-DX1", "FHT1-UX2", "FHT1-DX2",
			   "FHT2-UX1", "FHT2-DX1", "FHT2-UX2", "FHT2-DX2"
#if CFT
		 ,"CFT-UV1", "CFT-PHI1", "CFT-UV2", "CFT-PHI2",
		 "CFT-UV3", "CFT-PHI3", "CFT-UV4", "CFT-PHI4"
#endif
		 };
    Double_t vTOT[nTOT] = { 41.0, 41.0, 65.0, 65.0, 67.5, 67.5, 42.0, 54.0,
			    58.0, 60.0, 58.0, 60.0, 60.0, 60.0, 60.0, 60.0 };
    TString sDAQ[nDAQ] = { "DAQ-Eff", "L2-Eff", "Duty" };

    // Double_t t0base[NumOfSegBH2] = {
    //   // SigmaP
    //   353400, 356300, 353800, 352000, 353200, 354100, 354600, 354900
    //   // SigmaM
    //   // 353800, 356700, 354200, 351300, 353500, 354300, 354800, 355200
    // };

    TGraph  *g_tot[nTOT];
    TGraph  *g_daq[nDAQ];
    TGraph  *g_t0[NumOfSegBH2];
    TLegend *leg_tot;
    TLegend *leg_daq;
    TLegend *leg_t0;
    Color_t  col_tot[] = { kGreen, kGreen+1,
			   kBlue, kBlue+1, kBlue+2, kBlue+3,
			   kRed, kRed+1,
			   kOrange, kOrange+1, kOrange+2, kOrange+3,
			   kOrange+4, kOrange+5, kOrange+6, kOrange+7,
			   kCyan, kCyan+1, kCyan+2, kCyan+3,
			   kCyan+4, kCyan+5, kCyan+6, kCyan+7 };
    Color_t  col_daq[nDAQ] = { kRed, kOrange+1, kBlue };
    Color_t  col_t0[NumOfSegBH2] = { kBlack, kRed+1, kGreen+1, kBlue+1,
				     kMagenta+1, kCyan+1, kOrange+1,
				     kSpring+1 };

    Double_t
    GetTOT( TH1* h )
    {
      if( !h || h->GetEntries()<10 )
	return 0.;
      TCanvas c;
      c.cd();
      TF1 f("f", "gaus", -10., 100.);
      Double_t p = h->GetBinCenter(h->GetMaximumBin());
      if( p < 30. ) p = 60.;
      Double_t w = 10.;
      h->Fit("f", "Q", "", p-w, p+w );
      for( Int_t ifit=0; ifit<3; ++ifit ){
	h->Fit("f", "Q", "", p-w, p+w );
	p = f.GetParameter(1);
	w = f.GetParameter(2) * 1.;
      }
      return f.GetParameter(1);
    }

    Double_t
    GetT0( TH1* h )
    {
      if( !h || h->GetEntries()<10 )
	return 0.;
      TCanvas c;
      c.cd();
      TF1 f("f", "gaus", 350000., 360000.);
      Double_t p = h->GetBinCenter(h->GetMaximumBin());
      Double_t w = 500.;
      h->Fit("f", "Q", "", p-w, p+w );
      for( Int_t ifit=0; ifit<3; ++ifit ){
	h->Fit("f", "Q", "", p-2*w, p+2*w );
	p = f.GetParameter(1);
	w = f.GetParameter(2);
      }
      return f.GetParameter(1);
    }
  }

//____________________________________________________________________________
Int_t
process_begin( const std::vector<std::string>& argv )
{
  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  gConfMan.InitializeParameter<DCGeomMan>("DCGEOM");
  gConfMan.InitializeParameter<DCTdcCalibMan>("TDCCALIB");
  gConfMan.InitializeParameter<DCDriftParamMan>("DRFTPM");
  gConfMan.InitializeParameter<HodoParamMan>("HDPRM");
  gConfMan.InitializeParameter<UserParamMan>("USER");
  if( !gConfMan.IsGood() ) return -1;
  // unpacker and all the parameter managers are initialized at this stage

  std::vector<TCanvas*> carray;
  carray.push_back( new TCanvas("t0_monitor", "t0_monitor") );
  carray.push_back( new TCanvas("tot_monitor", "tot_monitor") );
  carray.push_back( new TCanvas("daq_monitor", "daq_monitor") );

  gHttp.SetPort(9091);
  gHttp.Open();
  gHttp.Register(gHist.createBH2());
  gHttp.Register(gHist.createBFT());
  gHttp.Register(gHist.createSFT());
  gHttp.Register(gHist.createSCH());
  gHttp.Register(gHist.createFHT1());
  gHttp.Register(gHist.createFHT2());
#if CFT
  gHttp.Register(gHist.createCFT());
#endif
  for( auto&& c : carray )
    gHttp.Register(c);

  Double_t legX = 0.13;
  Double_t legY = 0.13;
  Double_t legW = 0.14;
  Double_t legH = 0.26;

  // T0 Monitor
  {
    leg_t0 = new TLegend( legX, legY, legX+legW, legY+legH+0.3 );
    leg_t0->SetTextSize(0.05);
    leg_t0->SetFillColor(0);
    leg_t0->SetBorderSize(4);
    for( Int_t i=0; i<NumOfSegBH2; ++i ){
      carray[0]->cd();
      gPad->SetGrid();
      gPad->SetTicky(2);
      g_t0[i] = new TGraph();
      g_t0[i]->SetTitle("T0 Monitor");
      g_t0[i]->SetName( Form("BH2-%d", i+1) );
      g_t0[i]->SetMarkerStyle(8);
      g_t0[i]->SetMarkerSize(1.5);
      g_t0[i]->SetMarkerColor(col_t0[i]);
      g_t0[i]->SetLineWidth(3);
      g_t0[i]->SetLineColor(col_t0[i]);
      if(i==0) g_t0[i]->Draw("AL");
      else     g_t0[i]->Draw("L");
      g_t0[i]->SetPoint(0,0,0);
      leg_t0->AddEntry( g_t0[i], Form("BH2-%d", i+1), "P" );
    }
    leg_t0->SetNColumns(2);
    leg_t0->Draw();
  }

  // TOT Monitor
  {
    leg_tot = new TLegend( legX, legY, legX+legW+0.1, legY+legH+0.3 );
    leg_tot->SetTextSize(0.05);
    leg_tot->SetFillColor(0);
    leg_tot->SetBorderSize(4);
    for( Int_t i=0; i<nTOT; ++i ){
      carray[1]->cd();
      gPad->SetGrid();
      gPad->SetTicky(2);
      g_tot[i] = new TGraph();
      g_tot[i]->SetTitle("TOT Monitor");
      g_tot[i]->SetName( sTOT[i] );
      g_tot[i]->SetMarkerStyle(8);
      g_tot[i]->SetMarkerSize(1.5);
      g_tot[i]->SetMarkerColor(col_tot[i]);
      g_tot[i]->SetLineWidth(3);
      g_tot[i]->SetLineColor(col_tot[i]);
      if(i==0) g_tot[i]->Draw("AL");
      else     g_tot[i]->Draw("L");
      g_tot[i]->SetPoint(0,0,0);
      leg_tot->AddEntry( g_tot[i], sTOT[i], "P" );
    }
    leg_tot->SetNColumns(2);
    leg_tot->Draw();
  }

  if( gHist.setHistPtr(hptr_array) != 0 ) return -1;

  // DAQ Monitor
  {
    leg_daq = new TLegend( legX, legY, legX+legW, legY+legH );
    leg_daq->SetTextSize(0.05);
    leg_daq->SetFillColor(0);
    leg_daq->SetBorderSize(4);
    for( Int_t i=0; i<nDAQ; ++i ){
      carray[2]->cd();
      gPad->SetGrid();
      gPad->SetTicky(2);
      g_daq[i] = new TGraph();
      g_daq[i]->SetTitle("DAQ Monitor");
      g_daq[i]->SetMarkerStyle(8);
      g_daq[i]->SetMarkerSize(1.5);
      g_daq[i]->SetMarkerColor(col_daq[i]);
      g_daq[i]->SetLineWidth(3);
      g_daq[i]->SetLineColor(col_daq[i]);
      if(i==0) g_daq[i]->Draw("AL");
      else     g_daq[i]->Draw("L");
      g_daq[i]->SetPoint(0,0,0);
      leg_daq->AddEntry( g_daq[i], sDAQ[i], "P" );
    }
    leg_daq->Draw();
  }

  gROOT->SetStyle("Modern");
  gStyle->SetTitleW(.4);
  gStyle->SetTitleH(.11);

  for( Int_t i=0, n=hptr_array.size(); i<n; ++i ){
    hptr_array[i]->SetDirectory(0);
  }

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

  static const Int_t scaler_id = gUnpacker.get_device_id("Scaler");

  // Spill Increment
  static Int_t spill = 0;
  static Int_t spill_inc = 0;
  static const Int_t nspill = 100;
  {
    static const Int_t module_id  =  0;
    static const Int_t channel_id = 50;

    static Int_t clock     = 0;
    static Int_t clock_pre = 0;
    Int_t hit = gUnpacker.get_entries( scaler_id, module_id, 0, channel_id, 0 );
    if(hit>0){
      clock = gUnpacker.get( scaler_id, module_id, 0, channel_id, 0 );
      if( clock<clock_pre ) spill_inc++;
    }
    clock_pre = clock;
  }

  // T0 Monitor
  {
    static const int k_device = gUnpacker.get_device_id("BH2");
    static const int k_d      = 1; // down
    static const int k_tdc    = gUnpacker.get_data_id("BH2", "fpga_leading");
    // static const unsigned int tdc_min = gUser.GetParameter("BH2_TDC_FPGA", 0);
    // static const unsigned int tdc_max = gUser.GetParameter("BH2_TDC_FPGA", 1);
    int bh2t_id   = gHist.getSequentialID(kBH2, 0, kTDC, NumOfSegBH2+1);
    for(int seg=0; seg<NumOfSegBH2; ++seg){
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      for(int m = 0; m<nhit; ++m){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc, m);
	if( tdc!=0 )
	  hptr_array[bh2t_id + seg]->Fill(tdc);
      }
    }
    if( spill_inc == nspill ){
      static Double_t val[NumOfSegBH2]     = {};
      static Double_t val_pre[NumOfSegBH2] = {};
      static HodoParamMan& hodoMan = HodoParamMan::GetInstance();
      for(Int_t i=0; i<NumOfSegBH2; ++i){
	val[i] = GetT0( hptr_array[bh2t_id + i] );
	Double_t t;
	hodoMan.GetTime(2, 0, i, 1, val_pre[i], t);
	std::cout << i << " " << val_pre[i] << " " << t << std::endl;
	g_t0[i]->SetPoint(spill, spill, t*1e3);
	g_t0[i]->GetYaxis()->SetRangeUser(-1000, 1000);
	g_t0[i]->GetXaxis()->SetLimits(spill-90, spill+10);
      }
      for(Int_t i=0; i<NumOfSegBH2; ++i) val_pre[i] = val[i];
    }
  }

  // TOT Monitor
  {
    // sequential id
    static const int bft_ctotu_id = gHist.getSequentialID(kBFT, 0, kADC, 11);
    static const int bft_ctotd_id = gHist.getSequentialID(kBFT, 0, kADC, 12);
    static const int sft_ctot_id  = gHist.getSequentialID(kSFT, 0, kADC, 11);
    static const int sch_tot_id   = gHist.getSequentialID(kSCH, 0, kADC,  1);
    static const int fht1_tot_id[NumOfLayersFHT*NumOfUDStructureFHT] = {
      gHist.getSequentialID(kFHT1, 0, kADC, NumOfSegFHT1+1),
      gHist.getSequentialID(kFHT1, 0, kADC, NumOfSegFHT1+1+FHTOffset),
      gHist.getSequentialID(kFHT1, 1, kADC, NumOfSegFHT1+1),
      gHist.getSequentialID(kFHT1, 1, kADC, NumOfSegFHT1+1+FHTOffset)
    };
    static const int fht2_tot_id[NumOfLayersFHT*NumOfUDStructureFHT] = {
      gHist.getSequentialID(kFHT2, 0, kADC, NumOfSegFHT2+1),
      gHist.getSequentialID(kFHT2, 0, kADC, NumOfSegFHT2+1+FHTOffset),
      gHist.getSequentialID(kFHT2, 1, kADC, NumOfSegFHT2+1),
      gHist.getSequentialID(kFHT2, 1, kADC, NumOfSegFHT2+1+FHTOffset)
    };
#if CFT
    // static const int cft_ctot_id = gHist.getSequentialID(kCFT, 0, kADC, 11);
    static const int cft_lg_id = gHist.getSequentialID(kCFT, 0, kLowGain, 1);
#endif
    // BFT _____________________________________________________________________
    {
      // data type
      static const int k_device   = gUnpacker.get_device_id("BFT");
      static const int k_uplane   = gUnpacker.get_plane_id("BFT", "upstream");
      static const int k_dplane   = gUnpacker.get_plane_id("BFT", "downstream");
      static const int k_leading  = gUnpacker.get_data_id("BFT", "leading");
      static const int k_trailing = gUnpacker.get_data_id("BFT", "trailing");
      // static const int tdc_min    = gUser.GetParameter("BFT_TDC", 0);
      // static const int tdc_max    = gUser.GetParameter("BFT_TDC", 1);

      int tdc_prev = 0;
      for(int i = 0; i<NumOfSegBFT; ++i){
	int nhit_u = gUnpacker.get_entries(k_device, k_uplane, 0, i, k_leading);
	int nhit_d = gUnpacker.get_entries(k_device, k_dplane, 0, i, k_leading);
	// u plane
	tdc_prev = 0;
	for(int m = 0; m<nhit_u; ++m){
	  int tdc = gUnpacker.get(k_device, k_uplane, 0, i, k_leading, m);
	  int tdc_t = gUnpacker.get(k_device, k_uplane, 0, i, k_trailing, m);
	  int tot = tdc - tdc_t;
	  if(tdc_prev==tdc) continue;
	  tdc_prev = tdc;
	  if(tot<=0) continue;
	  // if(tdc_min < tdc && tdc < tdc_max)
	  hptr_array[bft_ctotu_id]->Fill(tot);
	}
	// d plane
	tdc_prev = 0;
	for(int m = 0; m<nhit_d; ++m){
	  int tdc = gUnpacker.get(k_device, k_dplane, 0, i, k_leading, m);
	  int tdc_t = gUnpacker.get(k_device, k_dplane, 0, i, k_trailing, m);
	  int tot = tdc - tdc_t;
	  if(tdc_prev==tdc) continue;
	  tdc_prev = tdc;
	  if(tot<=0) continue;
	  // if(tdc_min < tdc && tdc < tdc_max)
	  hptr_array[bft_ctotd_id]->Fill(tot);
	}
      }
    }
    // SFT _____________________________________________________________________
    {
      // data type
      static const int k_device   = gUnpacker.get_device_id("SFT");
      static const int k_leading  = gUnpacker.get_data_id("SFT", "leading");
      static const int k_trailing = gUnpacker.get_data_id("SFT", "trailing");
      // static const int tdc_min    = gUser.GetParameter("SFT_TDC", 0);
      // static const int tdc_max    = gUser.GetParameter("SFT_TDC", 1);

      for(int l=0; l<NumOfPlaneSFT; ++l){
	int tdc_prev = 0;
	for(int i = 0; i<NumOfSegSFT[l]; ++i){
	  int nhit_l = gUnpacker.get_entries(k_device, l, 0, i, k_leading);
	  int nhit_t = gUnpacker.get_entries(k_device, l, 0, i, k_trailing);
	  int hit_l_max = 0;
	  int hit_t_max = 0;
	  if(nhit_l != 0)
	    hit_l_max = gUnpacker.get(k_device, l, 0, i, k_leading,  nhit_l - 1);
	  if(nhit_t != 0)
	    hit_t_max = gUnpacker.get(k_device, l, 0, i, k_trailing, nhit_t - 1);
	  if(nhit_l == nhit_t && hit_l_max > hit_t_max){
	    tdc_prev = 0;
	    for(int m = 0; m<nhit_l; ++m){
	      int tdc = gUnpacker.get(k_device, l, 0, i, k_leading, m);
	      int tdc_t = gUnpacker.get(k_device, l, 0, i, k_trailing, m);
	      int tot = tdc - tdc_t;
	      if(tdc_prev==tdc) continue;
	      tdc_prev = tdc;
	      if(tot<=0) continue;
	      //if(tdc_min < tdc && tdc < tdc_max)
	      hptr_array[sft_ctot_id+l]->Fill(tot);
	    }
	  }
	}
      }
    }
    // SCH _____________________________________________________________________
    {
      static const int k_device   = gUnpacker.get_device_id("SCH");
      static const int k_leading  = gUnpacker.get_data_id("SCH", "leading");
      static const int k_trailing = gUnpacker.get_data_id("SCH", "trailing");
      // static const int tdc_min = gUser.GetParameter("SCH_TDC", 0);
      // static const int tdc_max = gUser.GetParameter("SCH_TDC", 1);
      for( int i=0; i<NumOfSegSCH; ++i ){
	int nhit = gUnpacker.get_entries(k_device, 0, i, 0, k_leading);
	for(int m = 0; m<nhit; ++m){
	  int tdc      = gUnpacker.get(k_device, 0, i, 0, k_leading,  m);
	  int trailing = gUnpacker.get(k_device, 0, i, 0, k_trailing, m);
	  int tot      = tdc - trailing;
	  //if( tdc_min<tdc && tdc<tdc_max ){
	  if( i < 16 )
	    hptr_array[sch_tot_id]->Fill(tot);
	  else
	    hptr_array[sch_tot_id+1]->Fill(tot);
	  //}
	}
      }
    }
    // FHT1 ____________________________________________________________________
    {
      static const int k_device   = gUnpacker.get_device_id("FHT1");
      static const int k_leading  = gUnpacker.get_data_id("FHT1", "leading");
      static const int k_trailing = gUnpacker.get_data_id("FHT1", "trailing");
      // static const int tdc_min    = gUser.GetParameter("FHT1_TDC", 0);
      // static const int tdc_max    = gUser.GetParameter("FHT1_TDC", 1);
      for( int l=0; l<NumOfLayersFHT; ++l ){
	for( int ud=0; ud<NumOfUDStructureFHT; ++ud ){
	  for( int seg=0; seg<NumOfSegFHT1; ++seg ){
	    int nhit_l = gUnpacker.get_entries(k_device, l, seg, ud, k_leading);
	    std::vector<int> vtdc;
	    for(int m = 0; m<nhit_l; ++m){
	      int tdc = gUnpacker.get(k_device, l, seg, ud, k_leading,  m);
	      vtdc.push_back(tdc);
	    }
	    int nhit_t = gUnpacker.get_entries(k_device, l, seg, ud, k_trailing);
	    for(int m = 0; m<nhit_t; ++m){
	      int trailing = gUnpacker.get(k_device, l, seg, ud, k_trailing, m);
	      if( nhit_l == nhit_t ){
		int tdc = vtdc[m];
		int tot = tdc - trailing;
		// if( tdc_min<tdc && tdc<tdc_max )
		if( tot > 0 )
		  hptr_array[fht1_tot_id[l*2+ud]]->Fill(tot);
	      }
	    }
	  }
	}
      }
    }
    // FHT2 ____________________________________________________________________
    {
      static const int k_device   = gUnpacker.get_device_id("FHT2");
      static const int k_leading  = gUnpacker.get_data_id("FHT2", "leading");
      static const int k_trailing = gUnpacker.get_data_id("FHT2", "trailing");
      // static const int tdc_min    = gUser.GetParameter("FHT2_TDC", 0);
      // static const int tdc_max    = gUser.GetParameter("FHT2_TDC", 1);
      for( int l=0; l<NumOfLayersFHT; ++l ){
	for( int ud=0; ud<NumOfUDStructureFHT; ++ud ){
	  for( int seg=0; seg<NumOfSegFHT2; ++seg ){
	    int nhit_l = gUnpacker.get_entries(k_device, l, seg, ud, k_leading);
	    std::vector<int> vtdc;
	    for(int m = 0; m<nhit_l; ++m){
	      int tdc = gUnpacker.get(k_device, l, seg, ud, k_leading,  m);
	      vtdc.push_back(tdc);
	    }
	    int nhit_t = gUnpacker.get_entries(k_device, l, seg, ud, k_trailing);
	    for(int m = 0; m<nhit_t; ++m){
	      int trailing = gUnpacker.get(k_device, l, seg, ud, k_trailing, m);
	      if( nhit_l == nhit_t ){
		int tdc = vtdc[m];
		int tot = tdc - trailing;
		// if( tdc_min<tdc && tdc<tdc_max )
		if( tot > 0 )
		  hptr_array[fht2_tot_id[l*2+ud]]->Fill(tot);
	      }
	    }
	  }
	}
      }
    }
#if CFT
    // CFT _____________________________________________________________________
    {
      static const int k_device   = gUnpacker.get_device_id("CFT");
      // static const int k_leading  = gUnpacker.get_data_id("CFT" , "leading");
      // static const int k_trailing = gUnpacker.get_data_id("CFT", "trailing");
      static const int k_lowgain  = gUnpacker.get_data_id("CFT", "lowgain");
      // static const int tdc_min = gUser.GetParameter("CFT_TDC", 0);
      // static const int tdc_max = gUser.GetParameter("CFT_TDC", 1);
      for(int l=0; l<NumOfLayersCFT; ++l){
	for(int i = 0; i<NumOfSegCFT[l]; ++i){
	  // int nhit_l = gUnpacker.get_entries(k_device, l, i, 0, k_leading );
	  // int nhit_t = gUnpacker.get_entries(k_device, l, i, 0, k_trailing );
	  // int hit_l_max = 0;
	  // int hit_t_max = 0;
	  // if(nhit_l==0)
	  //   continue;
	  // if(nhit_l != 0)
	  //   hit_l_max = gUnpacker.get(k_device, l, i, 0, k_leading,  nhit_l - 1);
	  // if(nhit_t != 0)
	  //   hit_t_max = gUnpacker.get(k_device, l, i, 0, k_trailing, nhit_t - 1);
	  // if(nhit_l == nhit_t && hit_l_max > hit_t_max){
	  //   for(int m = 0; m<nhit_l; ++m){
	  //     int tdc = gUnpacker.get(k_device, l, i, 0, k_leading, m );
	  //     int tot = tdc - gUnpacker.get(k_device, l, i, 0, k_trailing, m );
	  //     if(tdc_min < tdc && tdc < tdc_max){
	  // 	hptr_array[cft_ctot_id+l]->Fill(tot);
	  // 	if(m!= nhit_l-1 )continue;
	  //     }
	  //   }
	  // }
	  int nhit_lg = gUnpacker.get_entries(k_device, l, i, 0, k_lowgain);
	  if(nhit_lg==0)continue;
	  int adc_lg = gUnpacker.get(k_device, l, i, 0, k_lowgain, 0);
	  hptr_array[cft_lg_id+l]->Fill(adc_lg);
	}
      }
    }
#endif
    if( spill_inc == nspill ){
      static Double_t val[nTOT]     = {};
      static Double_t val_pre[nTOT] = {};
      val[kBFT_U] = GetTOT( hptr_array[bft_ctotu_id] );
      val[kBFT_D] = GetTOT( hptr_array[bft_ctotd_id] );
      val[kSFT_U] = GetTOT( hptr_array[sft_ctot_id] );
      val[kSFT_V] = GetTOT( hptr_array[sft_ctot_id+1] );
      val[kSFT_X] = GetTOT( hptr_array[sft_ctot_id+2] );
      val[kSFT_XP] = GetTOT( hptr_array[sft_ctot_id+3] );
      val[kSCH_1_16] = GetTOT( hptr_array[sch_tot_id] );
      val[kSCH_17_64] = GetTOT( hptr_array[sch_tot_id+1] );
      val[kFHT1_UX1] = GetTOT( hptr_array[fht1_tot_id[0]] );
      val[kFHT1_DX1] = GetTOT( hptr_array[fht1_tot_id[1]] );
      val[kFHT1_UX2] = GetTOT( hptr_array[fht1_tot_id[2]] );
      val[kFHT1_DX2] = GetTOT( hptr_array[fht1_tot_id[3]] );
      val[kFHT2_UX1] = GetTOT( hptr_array[fht2_tot_id[0]] );
      val[kFHT2_DX1] = GetTOT( hptr_array[fht2_tot_id[1]] );
      val[kFHT2_UX2] = GetTOT( hptr_array[fht2_tot_id[2]] );
      val[kFHT2_DX2] = GetTOT( hptr_array[fht2_tot_id[3]] );
#if CFT
      // val[kCFT_UV1] = GetTOT( hptr_array[cft_ctot_id] );
      // val[kCFT_PHI1] = GetTOT( hptr_array[cft_ctot_id+1] );
      // val[kCFT_UV2] = GetTOT( hptr_array[cft_ctot_id+2] );
      // val[kCFT_PHI2] = GetTOT( hptr_array[cft_ctot_id+3] );
      // val[kCFT_UV3] = GetTOT( hptr_array[cft_ctot_id+4] );
      // val[kCFT_PHI3] = GetTOT( hptr_array[cft_ctot_id+5] );
      // val[kCFT_UV4] = GetTOT( hptr_array[cft_ctot_id+6] );
      // val[kCFT_PHI4] = GetTOT( hptr_array[cft_ctot_id+7] );
      val[kCFT_UV1] = GetTOT( hptr_array[cft_lg_id] );
      val[kCFT_PHI1] = GetTOT( hptr_array[cft_lg_id+1] );
      val[kCFT_UV2] = GetTOT( hptr_array[cft_lg_id+2] );
      val[kCFT_PHI2] = GetTOT( hptr_array[cft_lg_id+3] );
      val[kCFT_UV3] = GetTOT( hptr_array[cft_lg_id+4] );
      val[kCFT_PHI3] = GetTOT( hptr_array[cft_lg_id+5] );
      val[kCFT_UV4] = GetTOT( hptr_array[cft_lg_id+6] );
      val[kCFT_PHI4] = GetTOT( hptr_array[cft_lg_id+7] );
#endif
      for(Int_t i=0; i<nTOT; ++i){
	// std::cout << sTOT[i] << " " << val_pre[i] << std::endl;
	if( val_pre[i] > 200 ) val_pre[i] *= 0.05;
	g_tot[i]->SetPoint(spill, spill, val_pre[i] - vTOT[i]);
	g_tot[i]->GetYaxis()->SetRangeUser(-6, 6);
	g_tot[i]->GetXaxis()->SetLimits(spill-90, spill+10);
      }
      // Double_t kpi_ratio = beam_pre[kKbeam]/beam_pre[kPibeam];
      // leg_beam->SetHeader(Form("  K/pi : %.3lf", kpi_ratio));
      // Double_t intensity = beam_pre[kBeam];
      // leg_beam->SetHeader( Form("  Rate : %.3lf M",
      // 				intensity * 1.e-6 ) );
      for(Int_t i=0; i<nTOT; ++i) val_pre[i] = val[i];
      for( Int_t i=0, n=hptr_array.size(); i<n; ++i ){
	hptr_array[i]->Reset();
      }
    }
  }

  // DAQ Monitor
  static Int_t    spill_pre = 0;
  {
    enum eVal { kL1Req, kL1Acc, kL2Acc, kRealTime, kLiveTime, nVal };

    static const Int_t module_id[nVal]  = {  0,  0,  0,  0,  0 };
    static const Int_t channel_id[nVal] = { 55, 56, 62, 53, 54 };
    static Double_t val[nVal]     = {};
    static Double_t val_pre[nVal] = {};
    static Double_t val_sum[nVal] = {};
    for(Int_t i=0; i<nVal; ++i){
      Int_t hit = gUnpacker.get_entries( scaler_id, module_id[i], 0, channel_id[i], 0 );
      if( hit==0 ) continue;
      val[i] = (Double_t)gUnpacker.get( scaler_id, module_id[i], 0, channel_id[i], 0 );
    }
    if( spill_inc != spill_pre ){
      for( Int_t i=0; i<nVal; ++i ){
	val_sum[i] += val_pre[i];
      }
    }
    if( spill_inc == nspill ){
      Double_t daq_eff = val_sum[kL1Acc]/val_sum[kL1Req];
      Double_t l2_eff  = val_sum[kL2Acc]/val_sum[kL1Acc];
      Double_t real    = val_sum[kRealTime];
      Double_t live    = val_sum[kLiveTime];
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
      leg_daq->SetHeader( Form("  DAQ Eff. : %.3lf", daq_eff));
      for( Int_t i=0; i<nVal; ++i ){
#if 0
	std::cout << std::setw(2) << i << " " << std::setw(10) << val_pre[i] << " "
		  << std::setw(10) << (Double_t)(val_sum[i])/spill_inc << std::endl;
#endif
	val_sum[i] = 0;
      }
    }
    for( Int_t i=0; i<nVal; ++i ){
      val_pre[i] = val[i];
    }
  }

  if( spill_inc == nspill ){
    spill++;
    spill_inc = 0;
  }
  spill_pre = spill_inc;
  return 0;
}

}
