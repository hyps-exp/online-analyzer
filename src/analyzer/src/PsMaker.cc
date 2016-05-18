#include"PsMaker.hh"
#include"GuiPs.hh"
#include"HistMaker.hh"
#include"DetectorID.hh"
#include"HistHelper.hh"
#include"UnpackerManager.hh"
#include"Unpacker.hh"

#include<iostream>
#include<algorithm>

#include<TROOT.h>
#include<TList.h>
#include<TIterator.h>
#include<TClass.h>
#include<TCanvas.h>
#include<TPad.h>
#include<TPostScript.h>
#include<TH1.h>
#include<TH2.h>
#include<TText.h>
#include<TString.h>
#include<TStyle.h>

#define CONV_STRING(x) getStr_FromEnum(#x)

static const std::string MyName = "PsMaker::";

using namespace hddaq::gui;

// Constructor --------------------------------------------------------------
PsMaker::PsMaker():
  cps_(NULL), ps_(NULL)
{
  // Initialize option list
  name_option_.resize(sizeOptionList);
  
  name_option_[kExpDataSheet]  = "ExpDataSheet";
  name_option_[kFixXaxis]      = "FixXaxis";
  name_option_[kLogyADC]       = "LogyADC";
  name_option_[kLogyTDC]       = "LogyTDC";
  //  name_option_[kLogyHitPat]    = "LogyHitPat";
  //  name_option_[kLogyMulti]     = "LogyMulti";
}

// Destructor --------------------------------------------------------------
PsMaker::~PsMaker()
{
  
}

// -------------------------------------------------------------------------
// getListOfOption
// -------------------------------------------------------------------------
void PsMaker::getListOfOption(std::vector<std::string>& vec)
{
  PsMaker& g = PsMaker::getInstance();
  std::copy(g.name_option_.begin(), 
	    g.name_option_.end(),
	    back_inserter(vec)
	    );
}

// -------------------------------------------------------------------------
// makePs
// -------------------------------------------------------------------------
void PsMaker::makePs()
{
  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(1110);
  gStyle->SetTitleW(.4);
  gStyle->SetTitleH(.1);
  gStyle->SetStatW(.32);
  gStyle->SetStatH(.25);
  
  // make ps file instance
  const std::string& filename = GuiPs::getFilename();
  ps_  = new TPostScript(filename.c_str(), 112);
  cps_ = new TCanvas("cps","");

  // Make title page with run number
  drawRunNumber();

  // start draw histograms of each detector
  std::vector<std::string> name_detectors;
  HistMaker::getListOfPsFiles(name_detectors);
  const int NofDetector = name_detectors.size();
  for(int i = 0; i<NofDetector; ++i){
    if(GuiPs::isDevOn(i) || GuiPs::isOptOn(kExpDataSheet)){
      create(name_detectors[i]);
    }
  }

  // close
  if(ps_){
    ps_->Close();
    ps_ = NULL;
  }

  if(cps_){
    delete cps_;
    cps_ = NULL;
  }

  gROOT->SetStyle("Classic");
}

// -------------------------------------------------------------------------
// create
// -------------------------------------------------------------------------
void PsMaker::create(std::string& name)
{
  std::vector<int> id_list;
  std::vector<int> par_list(sizeParameterList);
  bool flag_xaxis = false; // case by case in each detector
  bool flag_log   = false; // case by case in each detector

  // BH1 ----------------------------------------------------------------
  if(name == CONV_STRING(kBH1)){
    int base_id = 0;
    // ADC
    par_list[kXdiv] = 3; par_list[kYdiv] = 4;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 2000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);

    // ADC U
    base_id = HistMaker::getUniqueID(kBH1, 0, kADC);
    for(int i = 0; i<NumOfSegBH1; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // ADC D
    base_id = HistMaker::getUniqueID(kBH1, 0, kADC, NumOfSegBH1+1);
    for(int i = 0; i<NumOfSegBH1; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // TDC
    par_list[kXdiv] = 3; par_list[kYdiv] = 4;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 4000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyTDC)  | GuiPs::isOptOn(kExpDataSheet);

    // TDC U
    base_id = HistMaker::getUniqueID(kBH1, 0, kTDC);
    for(int i = 0; i<NumOfSegBH1; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
    
    // TDC D
    base_id = HistMaker::getUniqueID(kBH1, 0, kTDC, NumOfSegBH1+1);
    for(int i = 0; i<NumOfSegBH1; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat and multiplicity
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;

    id_list.push_back(HistMaker::getUniqueID(kBH1, 0, kHitPat));
    id_list.push_back(HistMaker::getUniqueID(kBH1, 0, kMulti));
    drawOneCanvas(id_list, par_list, false, false);
  }

  // BFT ----------------------------------------------------------------
  if(name == CONV_STRING(kBFT)){
    // TDC
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 1000; // for TDC
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);

    id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kTDC,      1));
    id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kTDC,      2));
    id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kTDC,     11));
    id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kTDC,     12));
    drawOneCanvas(id_list, par_list, flag_xaxis, false);

    // TOT
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    par_list[kXrange_min] = -50; par_list[kXrange_max] = 150; // for TOT
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);

    id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kADC,      1));
    id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kADC,      2));
    id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kADC,     11));
    id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kADC,     12));
    drawOneCanvas(id_list, par_list, flag_xaxis, false);

    // HitPat & multi
    par_list[kXdiv] = 3; par_list[kYdiv] = 2;
    id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kHitPat,   1));
    id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kHitPat,   2));
    id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kMulti,    1));
    id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kHitPat,  11));
    id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kHitPat,  12));
    id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kMulti,   11));
    drawOneCanvas(id_list, par_list, flag_xaxis, false);

    // par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    // id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kTDC2D,    1));
    // id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kTDC2D,    2));
    // id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kADC2D,    1));
    // id_list.push_back(HistMaker::getUniqueID(kBFT, 0, kADC2D,    2));
    // drawOneCanvas(id_list, par_list, flag_xaxis, false, "box");
  }

  // BC3 ----------------------------------------------------------------
  if(name == CONV_STRING(kBC3)){
    // For all
    int base_id = 0;
    par_list[kXdiv] = 2; par_list[kYdiv] = 3;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);

    // BC3 TDC
    par_list[kXrange_min] = 256; par_list[kXrange_max] = 1000;
    base_id = HistMaker::getUniqueID(kBC3, 0, kTDC);
    for(int i = 0; i<NumOfLayersBC3; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, false);

    // BC3 HitPat
    base_id = HistMaker::getUniqueID(kBC3, 0, kHitPat);
    for(int i = 0; i<NumOfLayersBC3; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, false, false);

    // BC3 Multi
    base_id = HistMaker::getUniqueID(kBC3, 0, kMulti);
    for(int i = 0; i<NumOfLayersBC3; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, false, false);
  }

  // BC4 ----------------------------------------------------------------
  if(name == CONV_STRING(kBC4)){
    // For all
    int base_id = 0;
    par_list[kXdiv] = 2; par_list[kYdiv] = 3;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);

    // BC4 TDC
    par_list[kXrange_min] = 256; par_list[kXrange_max] = 1000;
    base_id = HistMaker::getUniqueID(kBC4, 0, kTDC);
    for(int i = 0; i<NumOfLayersBC4; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, false);

    // BC4 HitPat
    base_id = HistMaker::getUniqueID(kBC4, 0, kHitPat);
    for(int i = 0; i<NumOfLayersBC4; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, false, false);

    // BC4 Multi
    base_id = HistMaker::getUniqueID(kBC4, 0, kMulti);
    for(int i = 0; i<NumOfLayersBC4; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, false, false);
  }

  // BMW ----------------------------------------------------------------
  if(name == CONV_STRING(kBMW)){
    // For all
    par_list[kXdiv] = 2; par_list[kYdiv] = 1;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 2000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kLogyTDC) | GuiPs::isOptOn(kExpDataSheet);

    // ADC && TDC
    id_list.push_back(HistMaker::getUniqueID(kBMW, 0, kADC));
    id_list.push_back(HistMaker::getUniqueID(kBMW, 0, kTDC));
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  }

  // BH2 ----------------------------------------------------------------
  if(name == CONV_STRING(kBH2)){
    int base_id = 0;
    // ADC
    par_list[kXdiv] = 2; par_list[kYdiv] = 3;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 2000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);

    // ADC D
    base_id = HistMaker::getUniqueID(kBH2, 0, kADC);
    for(int i = 0; i<NumOfSegBH2; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // TDC
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 4000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyTDC)  | GuiPs::isOptOn(kExpDataSheet);

    // TDC D
    base_id = HistMaker::getUniqueID(kBH2, 0, kTDC);
    for(int i = 0; i<NumOfSegBH2; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat and multiplicity
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    id_list.push_back(HistMaker::getUniqueID(kBH2, 0, kHitPat));
    id_list.push_back(HistMaker::getUniqueID(kBH2, 0, kMulti));
    drawOneCanvas(id_list, par_list, false, false);
  }
  
  // BAC ----------------------------------------------------------------
  if(name == CONV_STRING(kBAC)){
    int base_id = 0;

    par_list[kXdiv] = 3; par_list[kYdiv] = 2;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 2000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);

    // ADC
    base_id = HistMaker::getUniqueID(kBAC, 0, kADC);
    for(int i = 0; i<NumOfSegBAC; ++i){id_list.push_back(base_id + i);}
    // HitPat
    base_id = HistMaker::getUniqueID(kBAC, 0, kHitPat);
    id_list.push_back(base_id);
    // TDC
    base_id = HistMaker::getUniqueID(kBAC, 0, kTDC);
    for(int i = 0; i<NumOfSegBAC; ++i){id_list.push_back(base_id + i);}
    // Multi
    base_id = HistMaker::getUniqueID(kBAC, 0, kMulti);
    id_list.push_back(base_id);

    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  }

  // BH2_E07 ----------------------------------------------------------------
  if(name == CONV_STRING(kBH2_E07)){
    int base_id = 0;
    // ADC
    par_list[kXdiv] = 4; par_list[kYdiv] = 2;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 2000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);
    
    // ADC/TDC
    base_id = HistMaker::getUniqueID(kBH2_E07, 0, kADC);
    for(int i = 0; i<NumOfSegBH2_E07*2; ++i){id_list.push_back(base_id + i);}
    base_id = HistMaker::getUniqueID(kBH2_E07, 0, kTDC);
    for(int i = 0; i<NumOfSegBH2_E07*2; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat/Multi
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    base_id = HistMaker::getUniqueID(kBH2_E07, 0, kHitPat);
    id_list.push_back(base_id);
    base_id = HistMaker::getUniqueID(kBH2_E07, 0, kMulti);
    id_list.push_back(base_id);
    drawOneCanvas(id_list, par_list, false, false);
  }

  // BAC_E07 ----------------------------------------------------------------
  if(name == CONV_STRING(kBAC_E07)){
    int base_id = 0;
    // ADC
    par_list[kXdiv] = 3; par_list[kYdiv] = 4;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 2000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);
    
    // ADC/TDC
    base_id = HistMaker::getUniqueID(kBAC_E07, 0, kADC);
    id_list.push_back(base_id + 0);
    id_list.push_back(base_id + 2);
    id_list.push_back(base_id + 3);
    id_list.push_back(base_id + 1);
    id_list.push_back(base_id + 4);
    id_list.push_back(base_id + 5);
    base_id = HistMaker::getUniqueID(kBAC_E07, 0, kTDC);
    id_list.push_back(base_id + 0);
    id_list.push_back(base_id + 2);
    id_list.push_back(base_id + 3);
    id_list.push_back(base_id + 1);
    id_list.push_back(base_id + 4);
    id_list.push_back(base_id + 5);
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat/Multi
    // par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    // base_id = HistMaker::getUniqueID(kBAC_E07, 0, kHitPat);
    // id_list.push_back(base_id);
    // base_id = HistMaker::getUniqueID(kBAC_E07, 0, kMulti);
    // id_list.push_back(base_id);
    // drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  }

  // SSD0 ----------------------------------------------------------------
  if(name == CONV_STRING(kSSD0)){
    int base_id = 0;
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = false;

    // ADC
    base_id = HistMaker::getUniqueID(kSSD0, 0, kADC2D);
    for(int i=0; i<NumOfLayersSSD0; ++i){ id_list.push_back(base_id++); }
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
    // TDC
    base_id = HistMaker::getUniqueID(kSSD0, 0, kTDC2D);
    for(int i=0; i<NumOfLayersSSD0; ++i){ id_list.push_back(base_id++); }
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat/Multi
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    base_id = HistMaker::getUniqueID(kSSD0, 0, kHitPat);
    for(int i=0; i<NumOfLayersSSD0; ++i){ id_list.push_back(base_id++); }
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
    base_id = HistMaker::getUniqueID(kSSD0, 0, kMulti);
    for(int i=0; i<NumOfLayersSSD0; ++i){ id_list.push_back(base_id++); }
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  }

  // SSD1 ----------------------------------------------------------------
  if(name == CONV_STRING(kSSD1)){
    int base_id = 0;
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = false;

    // ADC
    base_id = HistMaker::getUniqueID(kSSD1, 0, kADC2D);
    for(int i=0; i<NumOfLayersSSD1; ++i){ id_list.push_back(base_id++); }
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
    // TDC
    base_id = HistMaker::getUniqueID(kSSD1, 0, kTDC2D);
    for(int i=0; i<NumOfLayersSSD1; ++i){ id_list.push_back(base_id++); }
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat/Multi
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    base_id = HistMaker::getUniqueID(kSSD1, 0, kHitPat);
    for(int i=0; i<NumOfLayersSSD1; ++i){ id_list.push_back(base_id++); }
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
    base_id = HistMaker::getUniqueID(kSSD1, 0, kMulti);
    for(int i=0; i<NumOfLayersSSD1; ++i){ id_list.push_back(base_id++); }
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  }

  // SSD2 ----------------------------------------------------------------
  if(name == CONV_STRING(kSSD2)){
    int base_id = 0;
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = false;

    // ADC
    base_id = HistMaker::getUniqueID(kSSD2, 0, kADC2D);
    for(int i=0; i<NumOfLayersSSD2; ++i){ id_list.push_back(base_id++); }
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
    // TDC
    base_id = HistMaker::getUniqueID(kSSD2, 0, kTDC2D);
    for(int i=0; i<NumOfLayersSSD2; ++i){ id_list.push_back(base_id++); }
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat/Multi
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    base_id = HistMaker::getUniqueID(kSSD2, 0, kHitPat);
    for(int i=0; i<NumOfLayersSSD2; ++i){ id_list.push_back(base_id++); }
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
    base_id = HistMaker::getUniqueID(kSSD2, 0, kMulti);
    for(int i=0; i<NumOfLayersSSD2; ++i){ id_list.push_back(base_id++); }
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  }

  // PVAC ----------------------------------------------------------------
  if(name == CONV_STRING(kPVAC)){
    int base_id = 0;
    // ADC
    par_list[kXdiv] = 3; par_list[kYdiv] = 4;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 2000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);
    
    // ADC/TDC
    base_id = HistMaker::getUniqueID(kPVAC, 0, kADC);
    for(int i = 0; i<NumOfSegPVAC; ++i){id_list.push_back(base_id + i);}
    id_list.push_back(-1);
    base_id = HistMaker::getUniqueID(kPVAC, 0, kTDC);
    for(int i = 0; i<NumOfSegPVAC; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat/Multi
    // par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    // base_id = HistMaker::getUniqueID(kPVAC, 0, kHitPat);
    // id_list.push_back(base_id);
    // base_id = HistMaker::getUniqueID(kPVAC, 0, kMulti);
    // id_list.push_back(base_id);
    // drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  }

  // FAC ----------------------------------------------------------------
  if(name == CONV_STRING(kFAC)){
    int base_id = 0;
    // ADC
    par_list[kXdiv] = 4; par_list[kYdiv] = 4;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 2000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);
    
    // ADC/TDC
    base_id = HistMaker::getUniqueID(kFAC, 0, kADC);
    for(int i = 0; i<NumOfSegFAC; ++i){id_list.push_back(base_id + i);}
    id_list.push_back(-1);
    base_id = HistMaker::getUniqueID(kFAC, 0, kTDC);
    for(int i = 0; i<NumOfSegFAC; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat/Multi
    // par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    // base_id = HistMaker::getUniqueID(kFAC, 0, kHitPat);
    // id_list.push_back(base_id);
    // base_id = HistMaker::getUniqueID(kFAC, 0, kMulti);
    // id_list.push_back(base_id);
    // drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  }

  // SAC1 ----------------------------------------------------------------
  if(name == CONV_STRING(kSAC1)){
    // ADC
    par_list[kXdiv] = 2; par_list[kYdiv] = 1;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 2000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);
    
    // ADC/TDC
    id_list.push_back( HistMaker::getUniqueID(kSAC1, 0, kADC) );
    id_list.push_back( HistMaker::getUniqueID(kSAC1, 0, kTDC) );
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat/Multi
    // par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    // id_list.push_back( HistMaker::getUniqueID(kSAC1, 0, kHitPat) );
    // id_list.push_back( HistMaker::getUniqueID(kSAC1, 0, kMulti) );
    // drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  }

  // SCH ----------------------------------------------------------------
  if(name == CONV_STRING(kSCH)){
    int base_id = 0;
    // TDC
    par_list[kXdiv] = 8; par_list[kYdiv] = 4;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 1000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    base_id    = HistMaker::getUniqueID(kSCH, 0, kTDC);
    for(int i=0; i<NumOfSegSCH/2; ++i){id_list.push_back(base_id +i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
    for(int i=0; i<NumOfSegSCH/2; ++i){id_list.push_back(base_id +i +NumOfSegSCH/2);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // TOT
    par_list[kXdiv] = 8; par_list[kYdiv] = 4;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    base_id = HistMaker::getUniqueID(kSCH, 0, kADC);
    for(int i=0; i<NumOfSegSCH/2; ++i){id_list.push_back(base_id +i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
    for(int i=0; i<NumOfSegSCH/2; ++i){id_list.push_back(base_id +i +NumOfSegSCH/2);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // TDC/TOT 2D, HitPat and Multi
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    base_id = HistMaker::getUniqueID(kSCH, 0, kTDC2D);
    id_list.push_back(base_id);
    base_id = HistMaker::getUniqueID(kSCH, 0, kADC2D);
    id_list.push_back(base_id);
    base_id = HistMaker::getUniqueID(kSCH, 0, kHitPat);
    id_list.push_back(base_id);
    base_id = HistMaker::getUniqueID(kSCH, 0, kMulti);
    id_list.push_back(base_id);
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  }

  // KIC ----------------------------------------------------------------
  if(name == CONV_STRING(kKIC)){
    int base_id = 0;
    // ADC
    par_list[kXdiv] = 4; par_list[kYdiv] = 2;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 0x1000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);
    
    // ADC/TDC
    base_id = HistMaker::getUniqueID(kKIC, 0, kADC);
    for(int i = 0; i<NumOfSegKIC; ++i){id_list.push_back(base_id + i);}
    base_id = HistMaker::getUniqueID(kKIC, 0, kTDC);
    for(int i = 0; i<NumOfSegKIC; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat/Multi
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    base_id = HistMaker::getUniqueID(kKIC, 0, kHitPat);
    id_list.push_back(base_id);
    base_id = HistMaker::getUniqueID(kKIC, 0, kMulti);
    id_list.push_back(base_id);
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  }

  // SDC2 ----------------------------------------------------------------
  if(name == CONV_STRING(kSDC2)){
    // For all
    int base_id = 0;
    par_list[kXdiv] = 2; par_list[kYdiv] = 3;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);

    // SDC2 TDC
    par_list[kXrange_min] = 256; par_list[kXrange_max] = 1000;
    base_id = HistMaker::getUniqueID(kSDC2, 0, kTDC);
    for(int i = 0; i<NumOfLayersSDC2; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, false);

    // SDC2 HitPat
    base_id = HistMaker::getUniqueID(kSDC2, 0, kHitPat);
    for(int i = 0; i<NumOfLayersSDC2; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, false, false);

    // SDC2 Multi
    base_id = HistMaker::getUniqueID(kSDC2, 0, kMulti);
    for(int i = 0; i<NumOfLayersSDC2; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, false, false);
  }

  // HDC ----------------------------------------------------------------
  if(name == CONV_STRING(kHDC)){
    // For all
    int base_id = 0;
    par_list[kXdiv] = 2; par_list[kYdiv] = 3;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);

    // HDC TDC
    par_list[kXrange_min] = 256; par_list[kXrange_max] = 1000;
    base_id = HistMaker::getUniqueID(kHDC, 0, kTDC);
    for(int i = 0; i<NumOfLayersHDC; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, false);

    // HDC HitPat
    base_id = HistMaker::getUniqueID(kHDC, 0, kHitPat);
    for(int i = 0; i<NumOfLayersHDC; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, false, false);

    // HDC Multi
    base_id = HistMaker::getUniqueID(kHDC, 0, kMulti);
    for(int i = 0; i<NumOfLayersHDC; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, false, false);
  }

  // SP0 -----------------------------------------------------------------
  if(name == CONV_STRING(kSP0)){
    int base_id = 0;
    // ADC
    par_list[kXdiv] = 5; par_list[kYdiv] = 4;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 2000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);

    for(int draw = 0; draw<4; ++draw){
      for(int l = 0; l<2; ++l){
	// ADC U
	base_id = HistMaker::getUniqueID(kSP0, kSP0_L1+l+2*draw, kADC);
	for(int i = 0; i<NumOfSegSP0; ++i){id_list.push_back(base_id + i);}

	// ADC D
	base_id = HistMaker::getUniqueID(kSP0, kSP0_L1+l+2*draw, kADC, NumOfSegSP0+1);
	for(int i = 0; i<NumOfSegSP0; ++i){id_list.push_back(base_id + i);}
      }
      drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
    }

    // TDC
    par_list[kXdiv] = 5; par_list[kYdiv] = 4;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 4000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyTDC)  | GuiPs::isOptOn(kExpDataSheet);

    for(int draw = 0; draw<4; ++draw){
      for(int l = 0; l<2; ++l){
	// TDC U
	base_id = HistMaker::getUniqueID(kSP0, kSP0_L1+l+2*draw, kTDC);
	for(int i = 0; i<NumOfSegSP0; ++i){id_list.push_back(base_id + i);}

	// TDC D
	base_id = HistMaker::getUniqueID(kSP0, kSP0_L1+l+2*draw, kTDC, NumOfSegSP0+1);
	for(int i = 0; i<NumOfSegSP0; ++i){id_list.push_back(base_id + i);}
      }
      drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
    }
  }

  // SDC3 ----------------------------------------------------------------
  if(name == CONV_STRING(kSDC3)){
    // For all
    int base_id = 0;
    par_list[kXdiv] = 2; par_list[kYdiv] = 3;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);

    // SDC3 TDC
    par_list[kXrange_min] = 700; par_list[kXrange_max] = 1600;
    base_id = HistMaker::getUniqueID(kSDC3, 0, kTDC);
    for(int i = 0; i<NumOfLayersSDC3; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, false);

    // SDC3 HitPat
    base_id = HistMaker::getUniqueID(kSDC3, 0, kHitPat);
    for(int i = 0; i<NumOfLayersSDC3; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, false, false);

    // SDC3 Multi
    base_id = HistMaker::getUniqueID(kSDC3, 0, kMulti);
    for(int i = 0; i<NumOfLayersSDC3; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, false, false);
  }

  // SDC4 ----------------------------------------------------------------
  if(name == CONV_STRING(kSDC4)){
    // For all
    int base_id = 0;
    par_list[kXdiv] = 2; par_list[kYdiv] = 3;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);

    // SDC4 TDC
    par_list[kXrange_min] = 700; par_list[kXrange_max] = 1600;
    base_id = HistMaker::getUniqueID(kSDC4, 0, kTDC);
    for(int i = 0; i<NumOfLayersSDC4; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, flag_xaxis, false);

    // SDC4 HitPat
    base_id = HistMaker::getUniqueID(kSDC4, 0, kHitPat);
    for(int i = 0; i<NumOfLayersSDC4; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, false, false);

    // SDC4 Multi
    base_id = HistMaker::getUniqueID(kSDC4, 0, kMulti);
    for(int i = 0; i<NumOfLayersSDC4; ++i){id_list.push_back(base_id + i);}
    drawOneCanvas(id_list, par_list, false, false);
  }

  // TOF ----------------------------------------------------------------
  if(name == CONV_STRING(kTOF)){
    int base_id = 0;
    int index   = 0;
    // ADC
    par_list[kXdiv] = 4; par_list[kYdiv] = 4;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 0x1000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);

    // ADC U
    base_id = HistMaker::getUniqueID(kTOF, 0, kADC);
    for(int i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    for(int i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // ADC D
    index = 0;
    base_id = HistMaker::getUniqueID(kTOF, 0, kADC, NumOfSegTOF+1);
    for(int i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    for(int i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // TDC
    par_list[kXdiv] = 4; par_list[kYdiv] = 4;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 4000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyTDC)  | GuiPs::isOptOn(kExpDataSheet);

    // TDC U
    index = 0;
    base_id = HistMaker::getUniqueID(kTOF, 0, kTDC);
    for(int i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    for(int i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // TDC D
    index = 0;
    base_id = HistMaker::getUniqueID(kTOF, 0, kTDC, NumOfSegTOF+1);
    for(int i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    for(int i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat and multiplicity
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;

    id_list.push_back(HistMaker::getUniqueID(kTOF, 0, kHitPat));
    id_list.push_back(HistMaker::getUniqueID(kTOF, 0, kMulti));
    drawOneCanvas(id_list, par_list, false, false);
  }

  // TOFMT --------------------------------------------------------------
  if(name == CONV_STRING(kTOFMT)){
    int base_id = 0;
    int index   = 0;

    // TDC
    par_list[kXdiv] = 4; par_list[kYdiv] = 4;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 4000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyTDC)  | GuiPs::isOptOn(kExpDataSheet);

    // TDC MT
    index = 0;
    base_id = HistMaker::getUniqueID(kTOFMT, 0, kTDC);
    for(int i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    for(int i = 0; i<NumOfSegTOF/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat and multiplicity
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;

    id_list.push_back(HistMaker::getUniqueID(kTOFMT, 0, kHitPat));
    id_list.push_back(HistMaker::getUniqueID(kTOFMT, 0, kMulti));
    drawOneCanvas(id_list, par_list, false, false);
  }

  // LAC ----------------------------------------------------------------
  if(name == CONV_STRING(kLAC)){
    int base_id = 0;
    int index   = 0;
    // ADC
    par_list[kXdiv] = 5; par_list[kYdiv] = 3;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 2000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);
    
    index = 0;
    base_id = HistMaker::getUniqueID(kLAC, 0, kADC);
    for(int i = 0; i<NumOfSegLAC/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    for(int i = 0; i<NumOfSegLAC/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // TDC
    par_list[kXdiv] = 5; par_list[kYdiv] = 3;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 4000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyTDC)  | GuiPs::isOptOn(kExpDataSheet);

    index = 0;
    base_id = HistMaker::getUniqueID(kLAC, 0, kTDC);
    for(int i = 0; i<NumOfSegLAC/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    for(int i = 0; i<NumOfSegLAC/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat and multiplicity
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;

    id_list.push_back(HistMaker::getUniqueID(kLAC, 0, kHitPat));
    id_list.push_back(HistMaker::getUniqueID(kLAC, 0, kMulti));
    drawOneCanvas(id_list, par_list, false, false);
  }

  // LC ----------------------------------------------------------------
  if(name == CONV_STRING(kLC)){
    int base_id = 0;
    int index   = 0;
    // ADC
    par_list[kXdiv] = 4; par_list[kYdiv] = 4;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 2000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);

    // ADC U
    base_id = HistMaker::getUniqueID(kLC, 0, kADC);
    for(int i = 0; i<NumOfSegLC/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    for(int i = 0; i<NumOfSegLC/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // ADC D
    index = 0;
    base_id = HistMaker::getUniqueID(kLC, 0, kADC, NumOfSegLC+1);
    for(int i = 0; i<NumOfSegLC/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    for(int i = 0; i<NumOfSegLC/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // TDC
    par_list[kXdiv] = 4; par_list[kYdiv] = 4;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 4000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyTDC)  | GuiPs::isOptOn(kExpDataSheet);

    // TDC U
    index = 0;
    base_id = HistMaker::getUniqueID(kLC, 0, kTDC);
    for(int i = 0; i<NumOfSegLC/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    for(int i = 0; i<NumOfSegLC/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // TDC D
    index = 0;
    base_id = HistMaker::getUniqueID(kLC, 0, kTDC, NumOfSegLC+1);
    for(int i = 0; i<NumOfSegLC/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    for(int i = 0; i<NumOfSegLC/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat and multiplicity
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;

    id_list.push_back(HistMaker::getUniqueID(kLC, 0, kHitPat));
    id_list.push_back(HistMaker::getUniqueID(kLC, 0, kMulti));
    drawOneCanvas(id_list, par_list, false, false);
  }

  // MsT ----------------------------------------------------------------
  if(name == CONV_STRING(kMsT)){
    int base_id = 0;
    int index   = 0;

    // TDC
    par_list[kXdiv] = 4; par_list[kYdiv] = 4;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 2048;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyTDC)  | GuiPs::isOptOn(kExpDataSheet);

    // TDC U
    index = 0;
    base_id = HistMaker::getUniqueID(kMsT, 0, kTDC);
    for(int i = 0; i<NumOfSegMsT/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
    for(int i = 0; i<NumOfSegMsT/2; ++i){id_list.push_back(base_id + index++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat and Flag
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    base_id = HistMaker::getUniqueID(kMsT, 0, kHitPat, 0);
    id_list.push_back(base_id);
    id_list.push_back(base_id+1);
    id_list.push_back(HistMaker::getUniqueID(kMsT, 0, kTDC2D));
    drawOneCanvas(id_list, par_list, false, false, "colz");
    par_list[kXdiv] = 1; par_list[kYdiv] = 1;
    id_list.push_back(HistMaker::getUniqueID(kMsT, 0, kHitPat2D, 0));
    drawOneCanvas(id_list, par_list, false, false, "text");
  }

  // PWO ----------------------------------------------------------------
  if(name == CONV_STRING(kPWO)){
    const int box_id = 3;
    int base_id = 0;
    // ADC
    par_list[kXdiv] = 4; par_list[kYdiv] = 4;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 0x800;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyADC)  | GuiPs::isOptOn(kExpDataSheet);

    base_id = HistMaker::getUniqueID(kPWO, 0, kADC);
    for(int i=0; i<NumOfUnitPWO[box_id]; ++i){ id_list.push_back(base_id++);}
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // TDC
    par_list[kXdiv] = 4; par_list[kYdiv] = 4;
    par_list[kXrange_min] = 0; par_list[kXrange_max] = 0x1000;
    flag_xaxis = GuiPs::isOptOn(kFixXaxis) | GuiPs::isOptOn(kExpDataSheet);
    flag_log   = GuiPs::isOptOn(kLogyTDC)  | GuiPs::isOptOn(kExpDataSheet);

    base_id = HistMaker::getUniqueID(kPWO, 0, kTDC);
    for(int i=0; i<NumOfUnitPWO[box_id]; ++i){ id_list.push_back(base_id++); }
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);

    // HitPat and multiplicity
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;

    id_list.push_back(HistMaker::getUniqueID(kPWO, 0, kHitPat));
    id_list.push_back(HistMaker::getUniqueID(kPWO, 0, kMulti));
    drawOneCanvas(id_list, par_list, false, false);
  }

  // Correlation ------------------------------------------------------------
  if(name == CONV_STRING(kCorrelation)){
    par_list[kXdiv] = 2; par_list[kYdiv] = 2;
    
    id_list.push_back(HistMaker::getUniqueID(kCorrelation, 0, 0, 1));
    id_list.push_back(HistMaker::getUniqueID(kCorrelation, 0, 0, 2));
    id_list.push_back(HistMaker::getUniqueID(kCorrelation, 0, 0, 3));
    id_list.push_back(HistMaker::getUniqueID(kCorrelation, 0, 0, 4));
    drawOneCanvas(id_list, par_list, false, false, "box");
  }

  // TriggerFlag------------------------------------------------------------
  if(name == CONV_STRING(kTriggerFlag)){
    int base_id = 0;
    par_list[kXdiv] = 1; par_list[kYdiv] = 1;
    //    par_list[kXrange_min] = 0; par_list[kXrange_max] = 0x1000;

    base_id = HistMaker::getUniqueID(kTriggerFlag, 0, kHitPat);
    id_list.push_back(base_id);
    // base_id = HistMaker::getUniqueID(kTriggerFlag, 1, kHitPat);
    // id_list.push_back(base_id); //for(E07)
    drawOneCanvas(id_list, par_list, false, false);
  }

  // EMC ----------------------------------------------------------------
  if(name == CONV_STRING(kEMC)){
    int base_id = 0;
    // XYpos
    par_list[kXdiv] = 1; par_list[kYdiv] = 1;
    base_id = HistMaker::getUniqueID(kEMC, 0, kXYpos);
    for(int i = 0; i<NumOfSegEMC; ++i){ id_list.push_back( base_id++ ); }
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  }

  // TimeStamp ----------------------------------------------------------------
  if(name == CONV_STRING(kTimeStamp)){
    int base_id = 0;
    par_list[kXdiv] = 4; par_list[kYdiv] = 3;
    base_id = HistMaker::getUniqueID(kTimeStamp, 0, kTDC);
    for(int i=0; i<NumOfVmeRm; ++i ){ id_list.push_back( base_id++ ); }
    drawOneCanvas(id_list, par_list, flag_xaxis, flag_log);
  }

}

// -------------------------------------------------------------------------
// drawOneChanvas
// -------------------------------------------------------------------------
void PsMaker::drawOneCanvas(std::vector<int>& id_list,
			    std::vector<int>& par_list,
			    bool flag_xaxis,
			    bool flag_log,
			    const char* optDraw
			    )
{
  static const std::string MyFunc = "drawOneCanvas ";
  // error check
  int NofPad  = par_list[kXdiv]*par_list[kYdiv];
  int NofHist = id_list.size();
  if(NofHist > NofPad){
    std::cerr << "#E: " << MyName << MyFunc
	      << "Num of pad is not enough\n"
	      << " 1st unique ID: " << id_list[0] << std::endl;
    return;
  }
  
  // make new ps page
  ps_->NewPage();
  cps_->Divide(par_list[kXdiv], par_list[kYdiv]);
  for(int i = 0; i<(int)id_list.size(); ++i){
    if(id_list[i]==-1) continue;
    cps_->cd(i + 1);
    if(flag_log){
      // log scale flag
      cps_->GetPad(i + 1)->SetLogy(1);
    }
    
    TH1* h = GHist::get(id_list[i]);
    if(!h){
      std::cerr << "#E: " << MyName << MyFunc
		<< "Pointer is NULL\n"
		<< " Unique ID: " << id_list[i] << std::endl;
      return;
    }

    if(flag_xaxis){
      // xaxis fix flag
      h->GetXaxis()->SetRangeUser(par_list[kXrange_min],
				  par_list[kXrange_max]);
    }

    h->SetLineColor(1);    
    h->Draw(optDraw);
  }

  cps_->Update();
  cps_->cd();
  clearOneCanvas(par_list[kXdiv]*par_list[kYdiv]);

  id_list.clear();
}

// -------------------------------------------------------------------------
// clearOneCanvas
// -------------------------------------------------------------------------
void PsMaker::clearOneCanvas(int npad)
{
  for(int i = 0; i<npad; ++i){
    cps_->GetPad(i + 1)->Close();
  }
  return;
}

// -------------------------------------------------------------------------
// drawRunNumber
// -------------------------------------------------------------------------
void PsMaker::drawRunNumber()
{
  hddaq::unpacker::UnpackerManager& g_unpacker =
    hddaq::unpacker::GUnpacker::get_instance();
  int runno = g_unpacker.get_root()->get_run_number();

  ps_->NewPage();
  cps_->Divide(1,1);
  cps_->cd(1);
  cps_->GetPad(1)->Range(0,0,100,100);
  
  TText text;
  text.SetTextSize(0.2);
  text.SetTextAlign(22);
  text.SetText(50.,50., Form("Run# %d", runno));
  text.Draw();
  
  cps_->Update();
  cps_->cd();
  cps_->GetPad(1)->Close();
}


