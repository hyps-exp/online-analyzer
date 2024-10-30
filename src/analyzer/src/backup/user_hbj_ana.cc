// Author: Tomonori Takahashi

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

#include "Controller.hh"

#include "user_analyzer.hh"
#include "UnpackerManager.hh"
#include "DAQNode.hh"
#include "filesystem_util.hh"
#include "ConfMan.hh"
#include "HistMaker.hh"
#include "DetectorID.hh"
#include "PsMaker.hh"
#include "GuiPs.hh"
#include "MacroBuilder.hh"

#include "UserParamMan.hh"
#include "HodoParamMan.hh"

#include "GeAdcCalibMan.hh"

#define DEBUG 0

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  std::vector<TH1*> hptr_array;

//____________________________________________________________________________
int
process_begin(const std::vector<std::string>& argv)
{
  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  gConfMan.InitializeHodoParamMan();
  gConfMan.InitializeHodoPHCMan();
  // gConfMan.InitializeDCGeomMan();
  // gConfMan.InitializeDCTdcCalibMan();
  // gConfMan.InitializeDCDriftParamMan();
  gConfMan.InitializeUserParamMan();
  gConfMan.InitializeGeAdcCalibMan();
  if( !gConfMan.IsGood() ) return -1;
  // unpacker and all the parameter managers are initialized at this stage

  // Make tabs
  hddaq::gui::Controller& gCon = hddaq::gui::Controller::getInstance();
  TGFileBrowser *tab_macro = gCon.makeFileBrowser("Macro");
  TGFileBrowser *tab_hist  = gCon.makeFileBrowser("Hist");

  // Add macros to the Macro tab
  tab_macro->Add(clear_all_canvas());
  tab_macro->Add(clear_canvas());
  tab_macro->Add(split22());
  tab_macro->Add(split32());
  tab_macro->Add(split33());
  tab_macro->Add(dispGeAdc());
  tab_macro->Add(dispGeTdc());
  tab_macro->Add(dispPWOTdc());
  tab_macro->Add(dispGeAdc_60Co());
  tab_macro->Add(dispGeAdc_LSO());
  // tab_macro->Add(dispGeAdc_60Co_1170());
  // tab_macro->Add(dispGeAdc_60Co_1330());

  // Add histograms to the Hist tab
  HistMaker& gHist = HistMaker::getInstance();
  tab_hist->Add(gHist.createGe());
  tab_hist->Add(gHist.createPWO());
  tab_hist->Add(gHist.createTriggerFlag());

  // Set histogram pointers to the vector sequentially.
  // This vector contains both TH1 and TH2.
  // Then you need to do down cast when you use TH2.
  if(0 != gHist.setHistPtr(hptr_array)){return -1;}

  // Users don't have to touch this section (Make Ps tab),
  // but the file path should be changed.
  // ----------------------------------------------------------
  PsMaker& gPsMaker = PsMaker::getInstance();
  std::vector<std::string> detList;
  std::vector<std::string> optList;
  gHist.getListOfPsFiles(detList);
  gPsMaker.getListOfOption(optList);

  hddaq::gui::GuiPs& gPsTab = hddaq::gui::GuiPs::getInstance();
  gPsTab.setFilename("/home/sks/PSFile/e13_2015/hbjana.ps");
  gPsTab.initialize(optList, detList);
  // ----------------------------------------------------------

  gStyle->SetOptStat(1110);
  gStyle->SetTitleW(.4);
  gStyle->SetTitleH(.1);
  gStyle->SetStatW(.42);
  gStyle->SetStatH(.35);

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
  static UnpackerManager& gUnpacker = GUnpacker::get_instance();
  static HistMaker&       gHist     = HistMaker::getInstance();

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // TriggerFlag ---------------------------------------------------
  bool scaler_flag = false;
  {
    static const int k_device = gUnpacker.get_device_id("Misc");
    static const int k_tdc    = gUnpacker.get_data_id("Misc", "tdc");

    static const int tf_tdc_id = gHist.getSequentialID(kTriggerFlag, 0, kTDC);
    static const int tf_hit_id = gHist.getSequentialID(kTriggerFlag, 0, kHitPat);
    for(int seg = 0; seg<NumOfSegMisc; ++seg){
      int nhit = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      if(nhit != 0){
	int tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc);
	if(tdc != 0){
	  hptr_array[tf_tdc_id+seg]->Fill(tdc);
	  hptr_array[tf_hit_id]->Fill(seg);
	  if(seg==SegIdScalerTrigger) scaler_flag = true;
	}
      }
    }// for(seg)
#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  //------------------------------------------------------------------
  // Hyperball-J
  //------------------------------------------------------------------

  // Ge --------------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("Ge");
    static const int k_adc    = gUnpacker.get_data_id("Ge","adc");
    static const int k_crm    = gUnpacker.get_data_id("Ge","crm");
    static const int k_tfa    = gUnpacker.get_data_id("Ge","tfa");
    static const int k_pur    = gUnpacker.get_data_id("Ge","pur");
    static const int k_rst    = gUnpacker.get_data_id("Ge","reset");

    // sequential id
    // sequential hist
    static const int ge_adc_id = gHist.getSequentialID(kGe, 0, kADC);
    static const int ge_crm_id = gHist.getSequentialID(kGe, 0, kCRM);
    static const int ge_tfa_id = gHist.getSequentialID(kGe, 0, kTFA);
    static const int ge_pur_id = gHist.getSequentialID(kGe, 0, kPUR);
    static const int ge_rst_id = gHist.getSequentialID(kGe, 0, kRST);

    // sum hist id
    static const int ge_adcsum_id
      = gHist.getSequentialID(kGe, 0, kADC, NumOfSegGe +1);
    static const int ge_adcsum_calib_id
      = gHist.getSequentialID(kGe, 0, kADC, NumOfSegGe +2);

    // hitpat hist id
    static const int ge_hitpat_id = gHist.getSequentialID(kGe, 0, kHitPat);

    // 2d hist id
    static const int ge_adc2d_id = gHist.getSequentialID(kGe, 0, kADC2D);
    static const int ge_crm2d_id = gHist.getSequentialID(kGe, 0, kCRM2D);
    static const int ge_tfa2d_id = gHist.getSequentialID(kGe, 0, kTFA2D);
    static const int ge_pur2d_id = gHist.getSequentialID(kGe, 0, kPUR2D);
    static const int ge_rst2d_id = gHist.getSequentialID(kGe, 0, kRST2D);

    for(int seg = 0; seg<NumOfSegGe; ++seg){
      // ADC
      int nhit_adc = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      if(nhit_adc != 0){
	int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[ge_adc_id + seg]->Fill(adc);
	hptr_array[ge_adc2d_id]->Fill(seg, adc);
	hptr_array[ge_adcsum_id]->Fill(adc);

	if(115 < adc && adc < 7500){
	  hptr_array[ge_hitpat_id]->Fill(seg);
	}

	GeAdcCalibMan& gGeAMan = GeAdcCalibMan::GetInstance();
	double energy;
	gGeAMan.CalibAdc(seg, adc, energy);
	if(energy > 100) hptr_array[ge_adcsum_calib_id]->Fill(energy);
      }

      // CRM
      int nhit_crm = gUnpacker.get_entries(k_device, 0, seg, 0, k_crm);
      if(nhit_crm != 0){
	for(int m = 0; m<nhit_crm; ++m){
	  int crm = gUnpacker.get(k_device, 0, seg, 0, k_crm, m);
	  hptr_array[ge_crm_id + seg]->Fill(crm);
	  hptr_array[ge_crm2d_id]->Fill(seg, crm);
	}
      }

      // TFA
      int nhit_tfa = gUnpacker.get_entries(k_device, 0, seg, 0, k_tfa);
      if(nhit_tfa != 0){
	for(int m = 0; m<nhit_tfa; ++m){
	  int tfa = gUnpacker.get(k_device, 0, seg, 0, k_tfa, m);
	  hptr_array[ge_tfa_id + seg]->Fill(tfa);
	  hptr_array[ge_tfa2d_id]->Fill(seg, tfa);
	}
      }

      // PUR
      int nhit_pur = gUnpacker.get_entries(k_device, 0, seg, 0, k_pur);
      if(nhit_pur != 0){
	for(int m = 0; m<nhit_pur; ++m){
	  int pur = gUnpacker.get(k_device, 0, seg, 0, k_pur, m);
	  hptr_array[ge_pur_id + seg]->Fill(pur);
	  hptr_array[ge_pur2d_id]->Fill(seg, pur);
	}
      }

      // RST
      int nhit_rst = gUnpacker.get_entries(k_device, 0, seg, 0, k_rst);
      if(nhit_rst != 0){
	for(int m = 0; m<nhit_rst; ++m){
	  int rst = gUnpacker.get(k_device, 0, seg, 0, k_rst, m);
	  hptr_array[ge_rst_id + seg]->Fill(rst);
	  hptr_array[ge_rst2d_id]->Fill(seg, rst);
	}
      }
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

  // PWO -------------------------------------------------------------
  {
    // data typep
    static const int k_device = gUnpacker.get_device_id("PWO");
    static const int k_adc    = gUnpacker.get_data_id("PWO","adc");
    static const int k_tdc    = gUnpacker.get_data_id("PWO","tdc");

    // sequential id
    // sequential hist
    static const int pwo_adc2d_id  = gHist.getSequentialID(kPWO, 0, kADC2D);
    static const int pwo_tdcsum_id = gHist.getSequentialID(kPWO, 0, kTDC);
    static const int pwo_tdc2d_id  = gHist.getSequentialID(kPWO, 0, kTDC2D);
    static const int pwo_hit_id    = gHist.getSequentialID(kPWO, 0, kHitPat);
    static const int pwo_mul_id    = gHist.getSequentialID(kPWO, 0, kMulti);

    int plane=0;
    for(int box = 0; box<NumOfBoxPWO; ++box){
      switch(NumOfUnitPWO[box]){
      case 21: plane = 0; break;
      case 14: plane = 1; break;
      case  9: plane = 2; break;
      case  6: plane = 3; break;
      default: std::cerr << "#E unknown PWO box type" << std::endl;
      }

      TH2* hadc2d = dynamic_cast<TH2*>(hptr_array[pwo_adc2d_id + box]);
      TH2* htdc2d = dynamic_cast<TH2*>(hptr_array[pwo_tdc2d_id + box]);

      int Multiplicity = 0;
      for(int unit = 0; unit<NumOfUnitPWO[box]; ++unit){
	// ADC
	int nhit_adc = gUnpacker.get_entries(k_device, plane, SegIdPWO[box], unit, k_adc);
	if(nhit_adc != 0){
	  int adc = gUnpacker.get(k_device, plane, SegIdPWO[box], unit, k_adc);
	  hadc2d->Fill(unit, adc);
	}

	// TDC
	int nhit_tdc = gUnpacker.get_entries(k_device, plane, SegIdPWO[box], unit, k_tdc);
	for(int m = 0; m<nhit_tdc; ++m){
	  int tdc = gUnpacker.get(k_device, plane, SegIdPWO[box], unit, k_tdc, m);
	  hptr_array[pwo_tdcsum_id+box]->Fill(tdc);
	  htdc2d->Fill(unit, tdc);
	}

	// HitPat
	if(nhit_tdc != 0){
	  hptr_array[pwo_hit_id+box]->Fill(unit);
	  ++Multiplicity;
	}

      }// for(unit)

      hptr_array[pwo_mul_id+box]->Fill(Multiplicity);
    }// for(box)

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  return 0;
}

}
