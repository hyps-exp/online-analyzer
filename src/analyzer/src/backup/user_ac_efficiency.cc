// -*- C++ -*-

#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <TGFileBrowser.h>
#include <TH1.h>
#include <TH2.h>
#include <TStyle.h>
#include <TString.h>

#include <DAQNode.hh>
#include <filesystem_util.hh>
#include <Unpacker.hh>
#include <UnpackerManager.hh>

#include "Controller.hh"
#include "user_analyzer.hh"

#include "ConfMan.hh"
#include "HistMaker.hh"
#include "DetectorID.hh"
#include "GuiPs.hh"
#include "HistMaker.hh"
#include "HodoParamMan.hh"
#include "HodoPHCMan.hh"
#include "MacroBuilder.hh"
#include "UserParamMan.hh"

namespace
{
using hddaq::unpacker::GUnpacker;
using hddaq::unpacker::DAQNode;
const auto& gUnpacker = GUnpacker::get_instance();
      auto& gHist     = HistMaker::getInstance();
const auto& gUser     = UserParamMan::GetInstance();
std::vector<TH1*> hptr_array;

static const double
btof_pi_min = 0., btof_pi_max = 1.,
btof_k_min = -1., btof_k_max = -2.;
}

namespace analyzer
{

//____________________________________________________________________________
int
process_begin(const std::vector<std::string>& argv)
{
  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  gConfMan.InitializeParameter<HodoParamMan>("HDPRM");
  gConfMan.InitializeParameter<HodoPHCMan>("HDPHC");
  gConfMan.InitializeParameter<UserParamMan>("USER");
  if( !gConfMan.IsGood() ) return -1;
  // unpacker and all the parameter managers are initialized at this stage

  // Make tabs
  hddaq::gui::Controller& gCon = hddaq::gui::Controller::getInstance();
  TGFileBrowser *tab_macro = gCon.makeFileBrowser("Macro");

  // Add macros to the Macro tab
  tab_macro->Add(macro::Get("clear_all_canvas"));
  tab_macro->Add(macro::Get("clear_canvas"));
  tab_macro->Add(macro::Get("split22"));
  tab_macro->Add(macro::Get("split32"));
  tab_macro->Add(macro::Get("split33"));
  tab_macro->Add(macro::Get("dispAcEfficiency"));

  // Add histograms to the Hist tab
  HistMaker& gHist = HistMaker::getInstance();

  // Set histogram pointers to the vector sequentially.
  // This vector contains both TH1 and TH2.
  // Then you need to do down cast when you use TH2.
  if(0 != gHist.setHistPtr(hptr_array)){return -1;}

  gStyle->SetOptStat(1110);
  gStyle->SetTitleW(.400);
  gStyle->SetTitleH(.100);
  gStyle->SetStatW(.320);
  gStyle->SetStatH(.250);

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

  //------------------------------------------------------------------
  // BTOF
  //------------------------------------------------------------------
  // Unpacker
  static const Int_t k_d_bh1  = gUnpacker.get_device_id("BH1");
  static const Int_t k_d_bh2  = gUnpacker.get_device_id("BH2");

  Int_t k_u      = 0; // up
  Int_t k_d      = 1; // down
  Int_t k_tdc    = gUnpacker.get_data_id("BH1", "fpga_leading");

  // HodoParam
  static const Int_t cid_bh1  = 1;
  static const Int_t cid_bh2  = 2;
  static const Int_t plid     = 0;

  // Sequential ID
  static const Int_t btof_id  = gHist.getSequentialID(kMisc, 0, kTDC, 1);
  static const Int_t btof_pi_id  = gHist.getSequentialID(kMisc, 0, kTDC, 2);
  static const Int_t btof_k_id  = gHist.getSequentialID(kMisc, 0, kTDC, 3);

  // BH2
  Double_t t0  = -999;
  Double_t ofs = 0;
  Int_t    seg = 0;
  Int_t nhit = gUnpacker.get_entries(k_d_bh2, 0, seg, 0, k_tdc);
  if( nhit != 0 ){
    Int_t tdc = gUnpacker.get(k_d_bh2, 0, seg, 0, k_tdc);
    if( tdc != 0 ){
      HodoParamMan& hodoMan = HodoParamMan::GetInstance();
      Double_t bh2t;
      hodoMan.GetTime(cid_bh2, plid, seg, 0, tdc, bh2t);
      t0 = bh2t;
    }//if(tdc)
  }// if(nhit)

  // BH1
  int pi_flag=0, k_flag=0;
  for(Int_t seg = 0; seg<NumOfSegBH1; ++seg){
    Int_t nhitu = gUnpacker.get_entries(k_d_bh1, 0, seg, k_u, k_tdc);
    Int_t nhitd = gUnpacker.get_entries(k_d_bh1, 0, seg, k_d, k_tdc);
    if(nhitu != 0 &&  nhitd != 0){
      Int_t tdcu = gUnpacker.get(k_d_bh1, 0, seg, k_u, k_tdc);
      Int_t tdcd = gUnpacker.get(k_d_bh1, 0, seg, k_d, k_tdc);
      if(tdcu != 0 && tdcd != 0){
	HodoParamMan& hodoMan = HodoParamMan::GetInstance();
	Double_t bh1tu, bh1td;
	hodoMan.GetTime(cid_bh1, plid, seg, k_u, tdcu, bh1tu);
	hodoMan.GetTime(cid_bh1, plid, seg, k_d, tdcd, bh1td);
	Double_t mt = (bh1tu+bh1td)/2.;
	Double_t btof = mt-(t0+ofs);
	hptr_array[btof_id]->Fill(btof);
	if(btof_pi_min<btof && btof<btof_pi_max){
	  hptr_array[btof_pi_id]->Fill(btof);
	  pi_flag=1;
	}
	if(btof_k_min<btof && btof<btof_k_max){
	  hptr_array[btof_k_id]->Fill(btof);
	  k_flag=1;
	}
      }// if(tdc)
    }// if(nhit)
  }// for(seg)

  // -----------------------------------------------------------BTOF


  // BAC -----------------------------------------------------------

  // data type
  Int_t k_device = gUnpacker.get_device_id("BAC");
  Int_t k_adc    = gUnpacker.get_data_id("BAC","adc");
  k_tdc    = gUnpacker.get_data_id("BAC","tdc");

  // sequential id
  static const Int_t baca_id   = gHist.getSequentialID(kBAC, 0, kADC,     1);
  static const Int_t baca_pi_id= gHist.getSequentialID(kBAC, 0, kADC, NumOfSegBAC+1);
  static const Int_t baca_k_id = gHist.getSequentialID(kBAC, 0, kADC, NumOfSegBAC*2+1);
  static const Int_t bact_id   = gHist.getSequentialID(kBAC, 0, kTDC,     1);
  static const Int_t bacawt_id = gHist.getSequentialID(kBAC, 0, kADCwTDC, 1);
  static const Int_t bach_pi_id= gHist.getSequentialID(kBAC, 0, kHitPat, NumOfSegBAC+1);
  static const Int_t bach_k_id = gHist.getSequentialID(kBAC, 0, kHitPat, NumOfSegBAC*2+1);

  // TDC gate range
  Int_t tdc_min = gUser.GetParameter("BAC_TDC", 0);
  Int_t tdc_max = gUser.GetParameter("BAC_TDC", 1);

  for(Int_t seg = 0; seg<NumOfSegBAC; ++seg){
    // ADC
    Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
    if( nhit_a!=0 ){
      Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
      hptr_array[baca_id + seg]->Fill( adc );
      if(pi_flag) hptr_array[baca_pi_id + seg]->Fill( adc );
      if(k_flag) hptr_array[baca_k_id + seg]->Fill( adc );
    }
    // TDC
    Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
    Bool_t is_in_gate = false;

    for(Int_t m = 0; m<nhit_t; ++m){
      Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
      hptr_array[bact_id + seg]->Fill( tdc );

      if(tdc_min < tdc && tdc < tdc_max){
	is_in_gate = true;
      }// tdc range is ok
    }// for(m)

    // ADC w/TDC

    if( is_in_gate ){
      if( gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0 ){
	Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[bacawt_id + seg]->Fill( adc );
      }
      hptr_array[bach_pi_id + seg]->Fill( 0 );
      if(pi_flag) hptr_array[bach_pi_id + seg]->Fill( 1 );
    }// flag is OK
    if( !is_in_gate ){
      hptr_array[bach_k_id + seg]->Fill( 0 );
      if(k_flag) hptr_array[bach_k_id + seg]->Fill( 1 );
    }
  }//for(seg)

  // -----------------------------------------------------------BAC


  // PVAC -----------------------------------------------------------

  // data type
  k_device = gUnpacker.get_device_id("PVAC");
  k_adc    = gUnpacker.get_data_id("PVAC","adc");
  k_tdc    = gUnpacker.get_data_id("PVAC","tdc");

  // sequential id
  static const Int_t pvaca_id   = gHist.getSequentialID(kPVAC, 0, kADC,     1);
  static const Int_t pvaca_pi_id= gHist.getSequentialID(kPVAC, 0, kADC, NumOfSegPVAC+1);
  static const Int_t pvaca_k_id = gHist.getSequentialID(kPVAC, 0, kADC, NumOfSegPVAC*2+1);
  static const Int_t pvact_id   = gHist.getSequentialID(kPVAC, 0, kTDC,     1);
  static const Int_t pvacawt_id = gHist.getSequentialID(kPVAC, 0, kADCwTDC, 1);
  static const Int_t pvach_pi_id= gHist.getSequentialID(kPVAC, 0, kHitPat,  2);
  static const Int_t pvach_k_id = gHist.getSequentialID(kPVAC, 0, kHitPat,  3);

  // TDC gate range
  tdc_min = gUser.GetParameter("PVAC_TDC", 0);
  tdc_max = gUser.GetParameter("PVAC_TDC", 1);

  for(Int_t seg = 0; seg<NumOfSegPVAC; ++seg){
    // ADC
    Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
    if( nhit_a!=0 ){
      Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
      hptr_array[pvaca_id + seg]->Fill( adc );
      if(pi_flag) hptr_array[pvaca_pi_id + seg]->Fill( adc );
      if(k_flag) hptr_array[pvaca_k_id + seg]->Fill( adc );
    }
    // TDC
    Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
    Bool_t is_in_gate = false;

    for(Int_t m = 0; m<nhit_t; ++m){
      Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
      hptr_array[pvact_id + seg]->Fill( tdc );

      if(tdc_min < tdc && tdc < tdc_max){
	is_in_gate = true;
      }// tdc range is ok
    }// for(m)

    // ADC w/TDC

    if( is_in_gate ){
      if( gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0 ){
	Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[pvacawt_id + seg]->Fill( adc );
      }
      hptr_array[pvach_pi_id + seg]->Fill( 0 );
      if(pi_flag) hptr_array[pvach_pi_id + seg]->Fill( 1 );
    }// flag is OK
    if( !is_in_gate ){
      hptr_array[pvach_k_id + seg]->Fill( 0 );
      if(k_flag) hptr_array[pvach_k_id + seg]->Fill( 1 );
    }
  }//for(seg)

  // -----------------------------------------------------------PVAC


  // FAC -----------------------------------------------------------

  // data type
  k_device = gUnpacker.get_device_id("FAC");
  k_adc    = gUnpacker.get_data_id("FAC","adc");
  k_tdc    = gUnpacker.get_data_id("FAC","tdc");

  // sequential id
  static const Int_t faca_id   = gHist.getSequentialID(kFAC, 0, kADC,     1);
  static const Int_t faca_pi_id= gHist.getSequentialID(kFAC, 0, kADC,     2);
  static const Int_t faca_k_id = gHist.getSequentialID(kFAC, 0, kADC,     3);
  static const Int_t fact_id   = gHist.getSequentialID(kFAC, 0, kTDC,     1);
  static const Int_t facawt_id = gHist.getSequentialID(kFAC, 0, kADCwTDC, 1);
  static const Int_t fach_pi_id= gHist.getSequentialID(kFAC, 0, kHitPat,  2);
  static const Int_t fach_k_id = gHist.getSequentialID(kFAC, 0, kHitPat,  3);

  // TDC gate range
  tdc_min = gUser.GetParameter("FAC_TDC", 0);
  tdc_max = gUser.GetParameter("FAC_TDC", 1);

  for(Int_t seg = 0; seg<NumOfSegFAC; ++seg){
    // ADC
    Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
    if( nhit_a!=0 ){
      Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
      hptr_array[faca_id + seg]->Fill( adc );
      if(pi_flag) hptr_array[faca_pi_id + seg]->Fill( adc );
      if(k_flag) hptr_array[faca_k_id + seg]->Fill( adc );
    }
    // TDC
    Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
    Bool_t is_in_gate = false;

    for(Int_t m = 0; m<nhit_t; ++m){
      Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
      hptr_array[fact_id + seg]->Fill( tdc );

      if(tdc_min < tdc && tdc < tdc_max){
	is_in_gate = true;
      }// tdc range is ok
    }// for(m)

    // ADC w/TDC

    if( is_in_gate ){
      if( gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0 ){
	Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[facawt_id + seg]->Fill( adc );
      }
      hptr_array[fach_pi_id + seg]->Fill( 0 );
      if(pi_flag) hptr_array[fach_pi_id + seg]->Fill( 1 );
    }// flag is OK
    if( !is_in_gate ){
      hptr_array[fach_k_id + seg]->Fill( 0 );
      if(k_flag) hptr_array[fach_k_id + seg]->Fill( 1 );
    }
  }//for(seg)

  // -----------------------------------------------------------FAC




  return 0;
}

}
