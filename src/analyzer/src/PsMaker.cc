// -*- C++ -*-

#include "PsMaker.hh"

#include <iostream>
#include <algorithm>

#include <TROOT.h>
#include <TList.h>
#include <TIterator.h>
#include <TClass.h>
#include <TCanvas.h>
#include <TLatex.h>
#include <TPad.h>
#include <TPostScript.h>
#include <TTimeStamp.h>
#include <TH1.h>
#include <TH2.h>
#include <TText.h>
#include <TString.h>
#include <TStyle.h>

#include <Unpacker.hh>
#include <UnpackerManager.hh>

#include "GuiPs.hh"
#include "HistMaker.hh"
#include "DetectorID.hh"
#include "HistHelper.hh"

#define CONV_STRING(x) getStr_FromEnum(#x)

namespace
{
const TString MyName = "PsMaker::";
using hddaq::gui::GuiPs;
}

//_____________________________________________________________________________
PsMaker::PsMaker( void )
  : m_name_option( sizeOptionList ),
    m_canvas( nullptr ),
    m_ps( nullptr )
{
  m_name_option[kExpDataSheet] = "ExpDataSheet";
  m_name_option[kFixXaxis]     = "FixXaxis";
  m_name_option[kLogyADC]      = "LogyADC";
  m_name_option[kLogyTDC]      = "LogyTDC";
  // m_name_option[kLogyHitPat]   = "LogyHitPat";
  // m_name_option[kLogyMulti]    = "LogyMulti";
  // m_name_option[kAutoSaveAtRunChange] = "AutoSaveAtRunChange";
}

//_____________________________________________________________________________
PsMaker::~PsMaker( void )
{
}

//_____________________________________________________________________________
void
PsMaker::getListOfOption( std::vector<TString>& vec )
{
  PsMaker& g = PsMaker::getInstance();
  std::copy( g.m_name_option.begin(),
             g.m_name_option.end(),
             back_inserter(vec) );
}

//_____________________________________________________________________________
void
PsMaker::makePs( void )
{
  gROOT->SetStyle( "Plain" );
  gStyle->SetOptStat( 1110 );
  gStyle->SetTitleW( .4 );
  gStyle->SetTitleH( .1 );
  gStyle->SetStatW( .32 );
  gStyle->SetStatH( .25 );
  if( m_ps ) delete m_ps;
  if( m_canvas ) delete m_canvas;

  // make ps file instance
  const TString& filename = GuiPs::getFilename();
  m_ps     = new TPostScript( filename, kLandscape );
  m_canvas = new TCanvas( "cps", "", 700, 500 );

  std::cout << std::endl << "PSFile = " << filename
	    << std::endl << std::endl;

  // Make title page with run number
  drawRunNumber();

  // start draw histograms of each detector
  std::vector<TString> name_detectors;
  HistMaker::getListOfPsFiles( name_detectors );
  const Int_t NofDetector = name_detectors.size();
  std::cout << "NofDetector = " << NofDetector << std::endl;
  for( Int_t i=0; i<NofDetector; ++i ){
      std::cout << "NofDetector = " << i << std::endl;
      std::cout << "DetName = " << name_detectors[i] << std::endl;
      // create( name_detectors[i] );
    if( GuiPs::isDevOn( i ) || GuiPs::isOptOn( kExpDataSheet ) ){
      std::cout << "OK!" << std::endl;
      create( name_detectors[i] );
    }
  }
  if( m_ps ){
    m_ps->Close();
    delete m_ps;
    m_ps = nullptr;
  }
  if( m_canvas ){
    delete m_canvas;
    m_canvas = nullptr;
  }
  gROOT->SetStyle("Classic");
}

//_____________________________________________________________________________
void
PsMaker::create( TString& name )
{
  std::vector<Int_t> id_list;
  std::vector<Int_t> par_list( sizeParameterList );
  Bool_t flag_xaxis = false; // case by case in each detector
  Bool_t flag_log   = false; // case by case in each detector

  // // CFT ----------------------------------------------------------------
  // if( name == CONV_STRING(kCFT) ){
  //   Int_t base_id = 0;
  //   // TDC
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
  //   flag_log   = GuiPs::isOptOn(kLogyTDC);
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kTDC, 1);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  //   // TDC_2D
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kTDC2D ,1);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, true, "colz");
  //   // TOT
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
  //   flag_log   = GuiPs::isOptOn(kLogyADC);
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kADC, 1);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  //   // TOT_2D
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kADC2D ,1);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  //   // CTOT
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
  //   flag_log   = GuiPs::isOptOn(kLogyADC);
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kADC, 11);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  //   // CTOT_2D
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kADC2D ,11);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  //   // HighGain
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
  //   flag_log   = GuiPs::isOptOn(kLogyADC);
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kHighGain, 1);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  //   // Pede
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
  //   flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kPede, 1);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  //   // LowGain
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
  //   flag_log   = GuiPs::isOptOn(kLogyADC);
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kLowGain, 1);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  //   // HighGain_2D
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kHighGain, 11);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  //   // Pede_2D
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kPede, 11);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  //   // LowGain_2D
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kLowGain, 11);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  //   // CHighGain
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kHighGain, 21);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  //   // CLowGain
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kLowGain, 21);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  //   // CHighGain_2D
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kHighGain, 31);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  //   // CLowGain_2D
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kLowGain, 31);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  //   // CHighGain_x_TOT
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kHighGain, 41);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  //   // CLowGain_x_TOT
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kLowGain, 41);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  //   // HitPattern
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kHitPat, 1);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  //   // CHitPattern
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kHitPat, 11);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  //   // Multiplicity
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kMulti, 1);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  //   // CMultiplicity
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, 0, kMulti, 11);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  //   // HitPattern Multiplicity
  //   par_list[kXdiv] = 2; par_list[kYdiv] = 2;
  //   id_list.push_back(HistMaker::getUniqueID(kCFT, 0, kMulti,  9));
  //   id_list.push_back(HistMaker::getUniqueID(kCFT, 0, kMulti, 10));
  //   id_list.push_back(HistMaker::getUniqueID(kCFT, 0, kMulti, 19));
  //   id_list.push_back(HistMaker::getUniqueID(kCFT, 0, kMulti, 20));
  //   drawOneCanvas(id_list, par_list, false, false);
  //   // Cluster HighGain 2D
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, kCluster, kHighGain, 10);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id +i+1);}
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  //   // Cluster LowGain 2D
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, kCluster, kLowGain, 10);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id +i+1);}
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  //   // Cluster TDC 2D
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kCFT, kCluster, kTDC2D, 0);
  //   for(Int_t i = 0; i<NumOfLayersCFT; ++i){id_list.push_back(base_id +i+1);}
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  // }

  // // BGO ----------------------------------------------------------------
  // if(name == CONV_STRING(kBGO)){
  //   Int_t base_id = 0;
  //   // FADC  1-12
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 3;
  //   base_id = HistMaker::getUniqueID(kBGO, 0, kFADC);
  //   for(Int_t i = 0; i<12; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, true, "colz");
  //   // FADC  13-24
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 3;
  //   for(Int_t i = 0; i<12; ++i){id_list.push_back(base_id + i + 12);}
  //   drawOneCanvas(id_list, par_list, false, true, "colz");
  //   // TDC  1-12
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 3;
  //   base_id = HistMaker::getUniqueID(kBGO, 0, kTDC);
  //   for(Int_t i = 0; i<12; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  //   // TDC  13-24
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 3;
  //   for(Int_t i = 0; i<12; ++i){id_list.push_back(base_id + i + 12);}
  //   drawOneCanvas(id_list, par_list, false, false);
  //   //Multi & HitPat
  //   par_list[kXdiv] = 2; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kBGO, 0, kHitPat,1);
  //   id_list.push_back(base_id);
  //   base_id = HistMaker::getUniqueID(kBGO, 0, kHitPat, 2);
  //   id_list.push_back(base_id);
  //   base_id = HistMaker::getUniqueID(kBGO, 0, kMulti,1);
  //   id_list.push_back(base_id);
  //   base_id = HistMaker::getUniqueID(kBGO, 0, kMulti, 2);
  //   id_list.push_back(base_id);
  //   drawOneCanvas(id_list, par_list, false, false);

  // }
  // // PiID ----------------------------------------------------------------
  // if(name == CONV_STRING(kPiID)){
  //   Int_t base_id = 0;
  //   // ADC 1-16
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 4;
  //   base_id = HistMaker::getUniqueID(kPiID, 0, kHighGain);
  //   for(Int_t i = 0; i<16; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  //   // ADC 17-32
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 4;
  //   base_id = HistMaker::getUniqueID(kPiID, 0, kHighGain, 16);
  //   for(Int_t i = 0; i<16; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  //   // ADC 1-16
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 4;
  //   base_id = HistMaker::getUniqueID(kPiID, 0, kLowGain);
  //   for(Int_t i = 0; i<16; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  //   // ADC 17-32
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 4;
  //   base_id = HistMaker::getUniqueID(kPiID, 0, kLowGain, 16);
  //   for(Int_t i = 0; i<16; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  //   // TDC 1-16
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 4;
  //   base_id = HistMaker::getUniqueID(kPiID, 0, kTDC);
  //   for(Int_t i = 0; i<16; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  //   // TDC 17-32
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 4;
  //   base_id = HistMaker::getUniqueID(kPiID, 0, kTDC, 16);
  //   for(Int_t i = 0; i<16; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list,par_list, false, false);
  //   // ADC2D
  //   par_list[kXdiv] = 2; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kPiID, 0, kADC2D, 1);
  //   for(Int_t i = 0; i<2; ++i){id_list.push_back(base_id + 10*i);}
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  //   // HitPat and multiplicity
  //   par_list[kXdiv] = 2; par_list[kYdiv] = 2;
  //   base_id = HistMaker::getUniqueID(kPiID, 0, kHitPat);
  //   id_list.push_back(base_id);
  //   id_list.push_back(base_id + 1);
  //   base_id = HistMaker::getUniqueID(kPiID, 0, kMulti);
  //   id_list.push_back(base_id);
  //   id_list.push_back(base_id + 1);
  //   drawOneCanvas(id_list, par_list, false, false);
  // }
  // SAC ----------------------------------------------------------------
  // if( name == CONV_STRING(kSAC) ){
  //   // ADC
  //   par_list[kXdiv] = 2; par_list[kYdiv] = 1;
  //   par_list[kXrange_min] = 0; par_list[kXrange_max] = 2000;
  //   flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
  //   flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);
  //   // ADC/TDC
  //   id_list.push_back( HistMaker::getUniqueID(kSAC, 0, kADC) );
  //   id_list.push_back( HistMaker::getUniqueID(kSAC, 0, kTDC) );
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  //   // HitPat/Multi
  //   // par_list[kXdiv] = 2; par_list[kYdiv] = 2;
  //   // id_list.push_back( HistMaker::getUniqueID(kSAC, 0, kHitPat) );
  //   // id_list.push_back( HistMaker::getUniqueID(kSAC, 0, kMulti) );
  //   // drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  // }
  // SDC1 ----------------------------------------------------------------
  if( name == CONV_STRING(kSDC1) ){
    // For all
    Int_t base_id = 0;
    par_list[kXdiv] = 2; par_list[kYdiv] = 3;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    std::cout << "OK!1" << std::endl;
    // SDC1 TDC
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 1000;
    base_id = HistMaker::getSequentialID(kSDC1, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, false);
    std::cout << "OK!2" << std::endl;
    // SDC1 TOT
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 500;
    base_id = HistMaker::getSequentialID(kSDC1, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, false);
    std::cout << "OK!3" << std::endl;
    // SDC1 HitPat
    base_id = HistMaker::getSequentialID(kSDC1, 0, kHitPat, 0);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, false, false);
    // SDC1 SelfCorr
    // par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    // base_id = HistMaker::getSequentialID(kSDC1, kSelfCorr, 0, 1);
    // for(Int_t i = 0; i<NumOfDimSDC1; ++i){id_list.push_back(base_id + i);}
    // drawOneCanvas(id_list, par_list, false, false, "colz");
    // SDC1 Multi
    par_list[kXdiv] = 2; par_list[kYdiv] = 3;
    base_id = HistMaker::getSequentialID(kSDC1, 0, kMulti, 0);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, false, false);
  }
  // SDC2 ----------------------------------------------------------------
  // if( name == CONV_STRING(kSDC2) ){
  //   // For all
  //   Int_t base_id = 0;
  //   par_list[kXdiv] = 2; par_list[kYdiv] = 2;
  //   flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
  //   // SDC2 TDC
  //   par_list[kXrange_min] = 0; par_list[kXrange_max] = 1000;
  //   base_id = HistMaker::getUniqueID(kSDC2, 0, kTDC);
  //   for(Int_t i = 0; i<NumOfLayersSDC2; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, false);
  //   // SDC2 TOT
  //   par_list[kXrange_min] = 0; par_list[kXrange_max] = 500;
  //   base_id = HistMaker::getUniqueID(kSDC2, 0, kADC);
  //   for(Int_t i = 0; i<NumOfLayersSDC2; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, false);
  //   // SDC2 HitPat
  //   base_id = HistMaker::getUniqueID(kSDC2, 0, kHitPat);
  //   for(Int_t i = 0; i<NumOfLayersSDC2; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  //   // SDC2 SelfCorr
  //   base_id = HistMaker::getUniqueID(kSDC2, kSelfCorr, 0, 1);
  //   for(Int_t i = 0; i<NumOfDimSDC2; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  //   // SDC2 Multi
  //   base_id = HistMaker::getUniqueID(kSDC2, 0, kMulti);
  //   for(Int_t i = 0; i<NumOfLayersSDC2; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  // }
  // // SDC0 ----------------------------------------------------------------
  // if( name == CONV_STRING(kSDC0) ){
  //   // For all
  //   Int_t base_id = 0;
  //   par_list[kXdiv] = 2; par_list[kYdiv] = 3;
  //   flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
  //   // SDC0 TDC
  //   par_list[kXrange_min] = 256; par_list[kXrange_max] = 1000;
  //   base_id = HistMaker::getUniqueID(kSDC0, 0, kTDC);
  //   for(Int_t i = 0; i<NumOfLayersSDC0; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, false);
  //   // SDC0 HitPat
  //   base_id = HistMaker::getUniqueID(kSDC0, 0, kHitPat);
  //   for(Int_t i = 0; i<NumOfLayersSDC0; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  //   // SDC0 Multi
  //   base_id = HistMaker::getUniqueID(kSDC0, 0, kMulti);
  //   for(Int_t i = 0; i<NumOfLayersSDC0; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  // }
  // // SDC3 ----------------------------------------------------------------
  // if(name == CONV_STRING(kSDC3)){
  //   // For all
  //   Int_t base_id = 0;
  //   par_list[kXdiv] = 2; par_list[kYdiv] = 2;
  //   flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
  //   // SDC3 TDC
  //   par_list[kXrange_min] = 0; par_list[kXrange_max] = 1500;
  //   base_id = HistMaker::getUniqueID(kSDC3, 0, kTDC);
  //   for(Int_t i = 0; i<NumOfLayersSDC3; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, false);
  //   // SDC3 TOT
  //   par_list[kXrange_min] = 0; par_list[kXrange_max] = 500;
  //   base_id = HistMaker::getUniqueID(kSDC3, 0, kADC);
  //   for(Int_t i = 0; i<NumOfLayersSDC3; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, false);
  //   // SDC3 HitPat
  //   base_id = HistMaker::getUniqueID(kSDC3, 0, kHitPat);
  //   for(Int_t i = 0; i<NumOfLayersSDC3; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  //   // SDC3 CHitPat
  //   base_id = HistMaker::getUniqueID(kSDC3, 0, kHitPat, 11);
  //   for(Int_t i = 0; i<NumOfLayersSDC3; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  //   // SDC3 SelfCorr
  //   base_id = HistMaker::getUniqueID(kSDC3, kSelfCorr, 0, 1);
  //   for(Int_t i = 0; i<NumOfDimSDC3; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  //   // SDC3 Multi
  //   base_id = HistMaker::getUniqueID(kSDC3, 0, kMulti);
  //   for(Int_t i = 0; i<NumOfLayersSDC3; ++i){id_list.push_back(base_id + i);}
  //   drawOneCanvas(id_list, par_list, false, false);
  // }
  // // BVH ----------------------------------------------------------------
  // if( name == CONV_STRING(kBVH) ){
  //   Int_t base_id = 0;
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 2;
  //   // TDC
  //   par_list[kXrange_min] = 0; par_list[kXrange_max] = 0x1000;
  //   flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
  //   flag_log   = GuiPs::isOptOn(kLogyTDC)  | GuiPs::isOptOn(kExpDataSheet);
  //   base_id = HistMaker::getUniqueID(kBVH, 0, kTDC);
  //   for(Int_t i = 0; i<NumOfSegBVH; ++i){id_list.push_back(base_id + i);}
  //   // HitPat and multiplicity
  //   id_list.push_back(HistMaker::getUniqueID(kBVH, 0, kHitPat));
  //   id_list.push_back(HistMaker::getUniqueID(kBVH, 0, kMulti));
  //   drawOneCanvas(id_list, par_list, false, false);
  // }
  // // TOF ----------------------------------------------------------------
  // if( name == CONV_STRING(kTOF) ){
  //   Int_t base_id = 0;
  //   Int_t index   = 0;
  //   // ADC
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 3;
  //   par_list[kXrange_min] = 0; par_list[kXrange_max] = 0x1000;
  //   flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
  //   flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);
  //   // ADC U
  //   base_id = HistMaker::getUniqueID(kTOF, 0, kADC);
  //   for(Int_t i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  //   for(Int_t i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  //   // ADC D
  //   index = 0;
  //   base_id = HistMaker::getUniqueID(kTOF, 0, kADC, NumOfSegTOF+1);
  //   for(Int_t i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  //   for(Int_t i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  //   // TDC
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 3;
  //   par_list[kXrange_min] = 200000; par_list[kXrange_max] = 420000;
  //   flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
  //   flag_log   = GuiPs::isOptOn(kLogyTDC)  | GuiPs::isOptOn(kExpDataSheet);
  //   // TDC U
  //   index = 0;
  //   base_id = HistMaker::getUniqueID(kTOF, 0, kTDC);
  //   for(Int_t i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  //   for(Int_t i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  //   // TDC D
  //   index = 0;
  //   base_id = HistMaker::getUniqueID(kTOF, 0, kTDC, NumOfSegTOF+1);
  //   for(Int_t i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  //   for(Int_t i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  //   // HitPat and multiplicity
  //   par_list[kXdiv] = 2; par_list[kYdiv] = 2;
  //   id_list.push_back(HistMaker::getUniqueID(kTOF, 0, kHitPat));
  //   id_list.push_back(HistMaker::getUniqueID(kTOF, 0, kMulti));
  //   drawOneCanvas(id_list, par_list, false, false);
  // }

  // // TOF_HT ----------------------------------------------------------------
  // if(name == CONV_STRING(kTOF_HT)){
  //   Int_t base_id = 0;
  //   Int_t index   = 0;
//    // ADC
//    par_list[kXdiv] = 4; par_list[kYdiv] = 3;
//    par_list[kXrange_min] = 0; par_list[kXrange_max] = 0x1000;
//    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
//    flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);
//
//    // ADC U
//    base_id = HistMaker::getUniqueID(kTOF_HT, 0, kADC);
//    for(Int_t i = 0; i<NumOfSegTOF_HT/2; ++i){id_list.push_back(base_id + index++);}
//    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
//
//    for(Int_t i = 0; i<NumOfSegTOF_HT/2; ++i){id_list.push_back(base_id + index++);}
//    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
//
//    // ADC D
//    index = 0;
//    base_id = HistMaker::getUniqueID(kTOF_HT, 0, kADC, NumOfSegTOF_HT+1);
//    for(Int_t i = 0; i<NumOfSegTOF_HT/2; ++i){id_list.push_back(base_id + index++);}
//    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
//
//    for(Int_t i = 0; i<NumOfSegTOF_HT/2; ++i){id_list.push_back(base_id + index++);}
//    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
//
    // // TDC
    // par_list[kXdiv] = 4; par_list[kYdiv] = 4;
    // par_list[kXrange_min] = 0; par_list[kXrange_max] = 4000;
    // flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    // flag_log   = GuiPs::isOptOn(kLogyTDC)  | GuiPs::isOptOn(kExpDataSheet);

    // // TDC U
    // index = 0;
    // base_id = HistMaker::getUniqueID(kTOF_HT, 0, kTDC);
    // for(Int_t i = 0; i<NumOfSegHtTOF; ++i){id_list.push_back(base_id + index++);}
    // drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

//    for(Int_t i = 0; i<NumOfSegTOF_HT/2; ++i){id_list.push_back(base_id + index++);}
//    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

//    // TDC D
//    index = 0;
//    base_id = HistMaker::getUniqueID(kTOF_HT, 0, kTDC, NumOfSegTOF_HT+1);
//    for(Int_t i = 0; i<NumOfSegTOF_HT/2; ++i){id_list.push_back(base_id + index++);}
//    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
//
//    for(Int_t i = 0; i<NumOfSegTOF_HT/2; ++i){id_list.push_back(base_id + index++);}
//    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat and multiplicity
  //   par_list[kXdiv] = 2; par_list[kYdiv] = 2;

  //   id_list.push_back(HistMaker::getUniqueID(kTOF_HT, 0, kHitPat));
  //   id_list.push_back(HistMaker::getUniqueID(kTOF_HT, 0, kMulti));
  //   drawOneCanvas(id_list, par_list, false, false);
  // }

  // // TOFMT --------------------------------------------------------------
  // if(name == CONV_STRING(kTOFMT)){
  //   Int_t base_id = 0;
  //   Int_t index   = 0;

  //   // TDC
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 4;
  //   par_list[kXrange_min] = 0; par_list[kXrange_max] = 4000;
  //   flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
  //   flag_log   = GuiPs::isOptOn(kLogyTDC)  | GuiPs::isOptOn(kExpDataSheet);

  //   // TDC MT
  //   index = 0;
  //   base_id = HistMaker::getUniqueID(kTOFMT, 0, kTDC);
  //   for(Int_t i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

  //   for(Int_t i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

  //   // HitPat and multiplicity
  //   par_list[kXdiv] = 2; par_list[kYdiv] = 2;

  //   id_list.push_back(HistMaker::getUniqueID(kTOFMT, 0, kHitPat));
  //   id_list.push_back(HistMaker::getUniqueID(kTOFMT, 0, kMulti));
  //   drawOneCanvas(id_list, par_list, false, false);
  // }

//   // LAC ----------------------------------------------------------------
//   if( name == CONV_STRING(kLAC) ){
//     Int_t base_id = 0;
//     Int_t index   = 0;
// #if 0
//     // ADC
//     par_list[kXdiv] = 4; par_list[kYdiv] = 4;
//     par_list[kXrange_min] = 0; par_list[kXrange_max] = 2000;
//     flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
//     flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);
//     // ADC
//     base_id = HistMaker::getUniqueID(kLAC, 0, kADC);
//     for(Int_t i = 0; i<NumOfSegLAC/2; ++i){id_list.push_back(base_id + index++);}
//     drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
//     for(Int_t i = 0; i<NumOfSegLAC/2; ++i){id_list.push_back(base_id + index++);}
//     drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
// #endif
//     // TDC
//     par_list[kXdiv] = 4; par_list[kYdiv] = 4;
//     par_list[kXrange_min] = 0; par_list[kXrange_max] = 4000;
//     flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
//     flag_log   = GuiPs::isOptOn(kLogyTDC)  | GuiPs::isOptOn(kExpDataSheet);
//     // TDC
//     index = 0;
//     base_id = HistMaker::getUniqueID(kLAC, 0, kTDC);
//     for(Int_t i = 0; i<NumOfSegLAC/2; ++i){id_list.push_back(base_id + index++);}
//     drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
//     for(Int_t i = 0; i<NumOfSegLAC/2; ++i){id_list.push_back(base_id + index++);}
//     drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
//     // HitPat and multiplicity
//     par_list[kXdiv] = 2; par_list[kYdiv] = 2;
//     id_list.push_back(HistMaker::getUniqueID(kLAC, 0, kHitPat));
//     id_list.push_back(HistMaker::getUniqueID(kLAC, 0, kMulti));
//     drawOneCanvas(id_list, par_list, false, false);
//   }

//   // AC1 ----------------------------------------------------------------
// //   if( name == CONV_STRING(kAC1) ){
// //     Int_t base_id = 0;
// //     Int_t index   = 0;
// // #if 1
// //     // ADC
// //     par_list[kXdiv] = 2; par_list[kYdiv] = 2;
// //     par_list[kXrange_min] = 0; par_list[kXrange_max] = 2000;
// //     flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
// //     flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);
// //     // ADC
// //     base_id = HistMaker::getUniqueID(kAC1, 0, kADC);
// //     for(Int_t i = 0; i<4; ++i){id_list.push_back(base_id + index++);}
// //     drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
// // #endif
// //     // TDC
// //     par_list[kXdiv] = 4; par_list[kYdiv] = 3;
// //     par_list[kXrange_min] = 0; par_list[kXrange_max] = 4000;
// //     flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
// //     flag_log   = GuiPs::isOptOn(kLogyTDC)  | GuiPs::isOptOn(kExpDataSheet);
// //     // TDC
// //     index = 0;
// //     base_id = HistMaker::getUniqueID(kAC1, 0, kTDC);
// //     for(Int_t i = 0; i<NumOfSegAC1/2; ++i){id_list.push_back(base_id + index++);}
// //     drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
// //     for(Int_t i = 0; i<NumOfSegAC1/2; ++i){id_list.push_back(base_id + index++);}
// //     drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
// //     // HitPat and multiplicity
// //     par_list[kXdiv] = 2; par_list[kYdiv] = 2;
// //     id_list.push_back(HistMaker::getUniqueID(kAC1, 0, kHitPat));
// //     id_list.push_back(HistMaker::getUniqueID(kAC1, 0, kMulti));
// //     drawOneCanvas(id_list, par_list, false, false);
// //   }


  // LC ----------------------------------------------------------------
  // if(name == CONV_STRING(kLC)){
  //   Int_t base_id = 0;
  //   Int_t index   = 0;
//    // ADC
//    par_list[kXdiv] = 4; par_list[kYdiv] = 4;
//    par_list[kXrange_min] = 0; par_list[kXrange_max] = 2000;
//    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
//    flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);
//
//    // ADC U
//    base_id = HistMaker::getUniqueID(kLC, 0, kADC);
//    for(Int_t i = 0; i<NumOfSegLC/2; ++i){id_list.push_back(base_id + index++);}
//    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
//
//    for(Int_t i = 0; i<NumOfSegLC/2; ++i){id_list.push_back(base_id + index++);}
//    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
//
//    // ADC D
//    index = 0;
//    base_id = HistMaker::getUniqueID(kLC, 0, kADC, NumOfSegLC+1);
//    for(Int_t i = 0; i<NumOfSegLC/2; ++i){id_list.push_back(base_id + index++);}
//    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
//
//    for(Int_t i = 0; i<NumOfSegLC/2; ++i){id_list.push_back(base_id + index++);}
//    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

//     // TDC
//     par_list[kXdiv] = 4; par_list[kYdiv] = 4;
//     par_list[kXrange_min] = 0; par_list[kXrange_max] = 4000;
//     flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
//     flag_log   = GuiPs::isOptOn(kLogyTDC)  | GuiPs::isOptOn(kExpDataSheet);

//     // TDC U
//     index = 0;
//     base_id = HistMaker::getUniqueID(kLC, 0, kTDC);
//     for(Int_t i = 0; i<NumOfSegLC/2; ++i){id_list.push_back(base_id + index++);}
//     drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

//     for(Int_t i = 0; i<NumOfSegLC/2; ++i){id_list.push_back(base_id + index++);}
//     drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

// //    // TDC D
// //    index = 0;
// //    base_id = HistMaker::getUniqueID(kLC, 0, kTDC, NumOfSegLC+1);
//    for(Int_t i = 0; i<NumOfSegLC/2; ++i){id_list.push_back(base_id + index++);}
//    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
//
//    for(Int_t i = 0; i<NumOfSegLC/2; ++i){id_list.push_back(base_id + index++);}
//    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat and multiplicity
  //   par_list[kXdiv] = 2; par_list[kYdiv] = 2;

  //   id_list.push_back(HistMaker::getUniqueID(kLC, 0, kHitPat));
  //   id_list.push_back(HistMaker::getUniqueID(kLC, 0, kMulti));
  //   drawOneCanvas(id_list, par_list, false, false);
  // }


  // Correlation ------------------------------------------------------------
  // if( name == CONV_STRING(kCorrelation) ){
  //   par_list[kXdiv] = 3; par_list[kYdiv] = 2;
  //   id_list.push_back(HistMaker::getUniqueID(kCorrelation, 0, 0, 1));
  //   id_list.push_back(HistMaker::getUniqueID(kCorrelation, 0, 0, 2));
  //   id_list.push_back(HistMaker::getUniqueID(kCorrelation, 0, 0, 3));
  //   id_list.push_back(HistMaker::getUniqueID(kCorrelation, 0, 0, 4));
  //   id_list.push_back(HistMaker::getUniqueID(kCorrelation, 0, 0, 5));
  //   id_list.push_back(HistMaker::getUniqueID(kCorrelation, 0, 0, 6));
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  //   // par_list[kXdiv] = 2; par_list[kYdiv] = 2;
  //   // id_list.push_back(HistMaker::getUniqueID(kCorrelation, 0, 0, 7));
  //   // id_list.push_back(HistMaker::getUniqueID(kCorrelation, 0, 0, 8));
  //   // id_list.push_back(HistMaker::getUniqueID(kCorrelation, 0, 0, 9));
  //   // id_list.push_back(HistMaker::getUniqueID(kCorrelation, 0, 0, 10));
  //   // drawOneCanvas(id_list, par_list, false, false, "colz");
  // }

  // Correlation catch-------------------------------------------------------
  // if(name == CONV_STRING(kCorrelation_catch)){
  //   par_list[kXdiv] = 2; par_list[kYdiv] = 2;
  //   id_list.push_back(HistMaker::getUniqueID(kCorrelation_catch, 0, 0, 1));
  //   id_list.push_back(HistMaker::getUniqueID(kCorrelation_catch, 0, 0, 2));
  //   id_list.push_back(HistMaker::getUniqueID(kCorrelation_catch, 0, 0, 3));
  //   id_list.push_back(HistMaker::getUniqueID(kCorrelation_catch, 0, 0, 4));
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  // }

  // // TriggerFlag------------------------------------------------------------
  // if( name == CONV_STRING(kTriggerFlag) ){
  //   Int_t base_id = 0;
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 4;
  //   par_list[kXrange_min] = 0; par_list[kXrange_max] = 0x1000;
  //   base_id = HistMaker::getUniqueID( kTriggerFlag, 0, kTDC );
  //   for( Int_t i=0; i<NumOfSegTFlag/2; ++i)
  //     id_list.push_back( base_id+i );
  //   drawOneCanvas( id_list, par_list, flag_xaxis, false );
  //   for( Int_t i=0; i<NumOfSegTFlag/2; ++i)
  //     id_list.push_back( base_id+i+NumOfSegTFlag/2 );
  //   drawOneCanvas( id_list, par_list, flag_xaxis, false );
  //   par_list[kXdiv] = 1; par_list[kYdiv] = 1;
  //   id_list.push_back( HistMaker::getUniqueID( kTriggerFlag, 0, kHitPat ) );
  //   drawOneCanvas( id_list, par_list, false, false );
  // }

  //// EMC ----------------------------------------------------------------
  //if(name == CONV_STRING(kEMC)){
  //  Int_t base_id = 0;
  //  // XYpos
  //  par_list[kXdiv] = 1; par_list[kYdiv] = 1;
  //  base_id = HistMaker::getUniqueID(kEMC, 0, kXYpos);
  //  for(Int_t i = 0; i<NumOfSegEMC; ++i){ id_list.push_back( base_id++ ); }
  //  drawOneCanvas(id_list, par_list, flag_xaxis, flag_log, "box" );
  //}

  // TimeStamp ----------------------------------------------------------------
  // if(name == CONV_STRING(kTimeStamp)){
  //   Int_t base_id = 0;
  //   par_list[kXdiv] = 4; par_list[kYdiv] = 3;
  //   base_id = HistMaker::getUniqueID(kTimeStamp, 0, kTDC);
  //   for(Int_t i=0; i<NumOfVmeRm; ++i ){ id_list.push_back( base_id++ ); }
  //   drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  // }

  // if( name == "DAQ" ){
  //   par_list[kXdiv] = 3; par_list[kYdiv] = 2;
  //   id_list.push_back(HistMaker::getUniqueID( kDAQ, kEB, kHitPat));
  //   id_list.push_back(HistMaker::getUniqueID( kDAQ, kVME, kHitPat2D));
  //   id_list.push_back(HistMaker::getUniqueID( kDAQ, kEASIROC, kHitPat2D));
  //   id_list.push_back(HistMaker::getUniqueID( kDAQ, kHUL, kHitPat2D));
  //   id_list.push_back(HistMaker::getUniqueID( kDAQ, kVMEEASIROC, kHitPat2D));
  //   drawOneCanvas(id_list, par_list, false, false, "colz");
  // }

  // // DC Eff ----------------------------------------------------------------
  // if(name == CONV_STRING(kDCEff)){
  //   drawDCEff();
  // }
}

//_____________________________________________________________________________
void
PsMaker::drawOneCanvas( std::vector<Int_t>& id_list,
                        std::vector<Int_t>& par_list,
                        Bool_t flag_xaxis,
                        Bool_t flag_log,
                        const Option_t* optDraw )
{
  static const TString MyFunc = "drawOneCanvas ";
  // error check
  Int_t NofPad  = par_list[kXdiv]*par_list[kYdiv];
  Int_t NofHist = id_list.size();
  if( NofHist > NofPad ){
    std::cerr << "#E: " << MyName << MyFunc
	      << "Num of pad is not enough "
              << NofHist << "/" << NofPad << std::endl
	      << " 1st unique ID: " << id_list[0] << std::endl;
    return;
  }

  // make new ps page
  m_ps->NewPage();
  m_canvas->Clear();
  m_canvas->Divide( par_list[kXdiv], par_list[kYdiv] );
  for( Int_t i=0, n=id_list.size(); i<n; ++i ){
    if( id_list[i]==-1 ) continue;
    m_canvas->cd( i+1 )->SetGrid();
    auto h = GHist::get( id_list[i] );
    if( !h ){
      std::cerr << "#E: " << MyName << MyFunc
		<< "Pointer is NULL\n"
		<< " Unique ID: " << id_list[i] << std::endl;
      return;
    }
    if( flag_xaxis ){
      // xaxis fix flag
      h->GetXaxis()->SetRangeUser( par_list[kXrange_min],
                                   par_list[kXrange_max] );
    }
    // Rebin
    TString hclass = h->ClassName();
    TString hname  = Form("hclone_%d", id_list[i]);
    if( flag_log ){
      // log scale flag
      if( hclass.Contains("TH1") )
	gPad->SetLogy(1);
      if( hclass.Contains("TH2") )
	gPad->SetLogz(1);
    } else {
      if( hclass.Contains("TH1") )
      h->SetMinimum( 0 );
    }
    if( hclass.Contains("TH2") &&
	h->GetNbinsX() * h->GetNbinsY() > 200000 ){
      if( gROOT->FindObject(hname) )
	gROOT->FindObject(hname)->Delete();
      TH2 *hclone = (TH2*)h->Clone(hname);
      if( h->GetNbinsX() > 200 )
	hclone->RebinX(h->GetNbinsX()/200);
      if( h->GetNbinsY() > 200 )
	hclone->RebinY(h->GetNbinsY()/200);
      hclone->SetLineColor(1);
      hclone->Draw(optDraw);
    } else {
      h->SetLineColor(1);
      h->Draw(optDraw);
    }
  }
  m_canvas->Update();
  m_canvas->cd();
  clearOneCanvas( par_list[kXdiv]*par_list[kYdiv] );
  id_list.clear();
}

//_____________________________________________________________________________
void
PsMaker::clearOneCanvas( Int_t npad )
{
  for(Int_t i = 0; i<npad; ++i){
    m_canvas->GetPad(i + 1)->Close();
  }
  return;
}

//_____________________________________________________________________________
void
PsMaker::drawRunNumber( void )
{
  hddaq::unpacker::UnpackerManager& g_unpacker =
    hddaq::unpacker::GUnpacker::get_instance();
  Int_t runno = g_unpacker.get_root()->get_run_number();
  // if( GuiPs::isOptOn( kAutoSaveAtRunChange ) )
  //   runno -= 1;

  m_ps->NewPage();
  m_canvas->Divide(1,1);
  m_canvas->cd(1);
  m_canvas->GetPad(1)->Range(0,0,100,100);

  TText text;
  text.SetTextSize(0.2);
  text.SetTextAlign(22);
  text.DrawText(50.,65., Form("Run# %d", runno));
  TTimeStamp stamp;
  stamp.Add( -stamp.GetZoneOffset() );
  text.SetTextSize(0.1);
  text.DrawText(50.,40., stamp.AsString("s") );

  m_canvas->Update();
  m_canvas->cd();
  m_canvas->GetPad(1)->Close();
}

//_____________________________________________________________________________
void
PsMaker::drawDCEff( void )
{
  static const TString MyFunc = "drawDCEff ";

  TLatex text;
  text.SetNDC();
  text.SetTextSize( 0.08 );

  // SdcIn
  {
    m_ps->NewPage();
    Int_t x = 4; Int_t y = 3;
    m_canvas->Divide( x, y );
    Int_t sdc1_id = HistMaker::getUniqueID( kSDC1, 0, kMulti );
    for( Int_t i=0; i<NumOfLayersSDC1; ++i ){
      m_canvas->cd( i+1 );
      TH1* h = GHist::get( sdc1_id+i+NumOfLayersSDC1 );
      if( !h ){
	std::cerr << "#E: " << MyName << MyFunc
		  << "Pointer is NULL\n"
		  << " Unique ID: " << sdc1_id+i+NumOfLayersSDC1 << std::endl;
	return;
      }
      h->SetLineColor(1);
      h->Draw();
      Double_t nof0   = h->GetBinContent(1);
      Double_t nofall = h->GetEntries();
      Double_t eff    = 1. - nof0/nofall;
      Double_t xpos   = 0.35;
      Double_t ypos   = 0.5;
      text.SetTextSize( 0.08 );
      text.DrawLatex( xpos, ypos, Form("plane eff. %.2f", eff ) );
    }
    Int_t sdc2_id = HistMaker::getUniqueID( kSDC2, 0, kMulti );
    for( Int_t i=0; i<NumOfLayersSDC2; ++i ){
      m_canvas->cd( i+1+NumOfLayersSDC1+2 );
      TH1* h = GHist::get( sdc2_id+i+NumOfLayersSDC2 );
      if( !h ){
	std::cerr << "#E: " << MyName << MyFunc
		  << "Pointer is NULL\n"
		  << " Unique ID: " << sdc2_id+i+NumOfLayersSDC2 << std::endl;
	return;
      }
      h->SetLineColor(1);
      h->Draw();
      Double_t nof0   = h->GetBinContent(1);
      Double_t nofall = h->GetEntries();
      Double_t eff    = 1. - nof0/nofall;
      Double_t xpos   = 0.35;
      Double_t ypos   = 0.5;
      text.SetTextSize( 0.08 );
      text.DrawLatex( xpos, ypos, Form("plane eff. %.2f", eff ) );
    }
    m_canvas->Update();
    m_canvas->cd();
    clearOneCanvas( x*y );
  }

  // SdcOut
  {
    m_ps->NewPage();
    Int_t x = 4; Int_t y = 3;
    m_canvas->Divide( x, y );
    Int_t sdc3_id = HistMaker::getUniqueID( kSDC3, 0, kMulti );
    for( Int_t i=0; i<NumOfLayersSDC3; ++i ){
      m_canvas->cd( i+1 );
      TH1* h = GHist::get( sdc3_id+i+NumOfLayersSDC3 );
      if( !h ){
	std::cerr << "#E: " << MyName << MyFunc
		  << "Pointer is NULL\n"
		  << " Unique ID: " << sdc3_id+i+NumOfLayersSDC3 << std::endl;
	return;
      }
      h->SetLineColor(1);
      h->Draw();
      Double_t nof0   = h->GetBinContent(1);
      Double_t nofall = h->GetEntries();
      Double_t eff    = 1. - nof0/nofall;
      Double_t xpos   = 0.35;
      Double_t ypos   = 0.5;
      text.SetTextSize( 0.08 );
      text.DrawLatex( xpos, ypos, Form("plane eff. %.2f", eff ) );
    }
    m_canvas->Update();
    m_canvas->cd();
    clearOneCanvas( x*y );
  }


}
