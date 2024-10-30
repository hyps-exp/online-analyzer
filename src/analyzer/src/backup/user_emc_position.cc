// -*- C++ -*-

// Author: Shuhei Hayakawa

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <TAxis.h>
#include <TCanvas.h>
#include <TGFileBrowser.h>
#include <TH1.h>
#include <TH2.h>
#include <TStyle.h>
#include <TString.h>
#include <TSystem.h>
#include <TText.h>

#include "Controller.hh"
#include "EMCParamMan.hh"
#include "user_analyzer.hh"
#include "Unpacker.hh"
#include "UnpackerManager.hh"
#include "ConfMan.hh"
#include "DetectorID.hh"
#include "HistMaker.hh"
#include "Updater.hh"

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  namespace
  {
    std::vector<TH1*> hptr_array;
    EMCParamMan& gEMC = EMCParamMan::GetInstance();
    const double emc_x_offset = 500000 - 303300;
    const double emc_y_offset = 500000 + 164000;
    TText text;
  }

//____________________________________________________________________________
int
process_begin(const std::vector<std::string>& argv)
{
  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  gConfMan.InitializeEMCParamMan();
  if( !gConfMan.IsGood() ) return -1;
  // unpacker and all the parameter managers are initialized at this stage

  // Make tabs
  hddaq::gui::Controller& gCon = hddaq::gui::Controller::getInstance();
  TGFileBrowser *tab_emc  = gCon.makeFileBrowser("EMC");
  HistMaker& gHist = HistMaker::getInstance();
  tab_emc->Add(gHist.createEMC());
  if(0 != gHist.setHistPtr(hptr_array)){ return -1; }

  gStyle->SetOptStat(10);
  gStyle->SetStatX(.900);
  gStyle->SetStatY(.900);
  gStyle->SetStatW(.250);
  gStyle->SetStatH(.200);
  gStyle->SetTitleW(.400);
  gStyle->SetTitleH(.090);

  TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
  c->cd();
  hptr_array[gHist.getSequentialID(kEMC, 0, kXYpos)]->Draw("colz");
  c->Update();

  gui::Updater::getInstance().setUpdateMode(gui::Updater::k_seconds);
  gui::Updater::getInstance().setInterval(2);

  text.SetNDC();
  text.SetTextSize(0.040);
  text.Draw();

  return 0;
}

//____________________________________________________________________________
int
process_end()
{
  return 0;
}

//____________________________________________________________________________
int
process_event( void )
{
  static UnpackerManager& gUnpacker = GUnpacker::get_instance();
  static HistMaker&       gHist     = HistMaker::getInstance();

  // EMC -----------------------------------------------------------
  static const int nspill = gEMC.NSpill();
  static int spill  = 0;
  static int rspill = 0;
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("EMC");
    static const int k_xpos   = gUnpacker.get_data_id("EMC", "xpos");
    static const int k_ypos   = gUnpacker.get_data_id("EMC", "ypos");
    // sequential id
    static const int xpos_id  = gHist.getSequentialID(kEMC, 0, kXpos);
    static const int ypos_id  = gHist.getSequentialID(kEMC, 0, kYpos);
    static const int xypos_id = gHist.getSequentialID(kEMC, 0, kXYpos);

    for(int seg=0; seg<NumOfSegEMC; ++seg){
      double xpos = 0;
      double ypos = 0;
      // Xpos
      int xpos_nhit = gUnpacker.get_entries(k_device, 0, 0, 0, k_xpos);
      if(xpos_nhit != 0){
	xpos = gUnpacker.get(k_device, 0, 0, 0, k_xpos);
	xpos = 500. - ( xpos / 1000. );
	hptr_array[xpos_id + seg]->Fill(xpos);
      }
      // Ypos
      int ypos_nhit = gUnpacker.get_entries(k_device, 0, 0, 0, k_ypos);
      if(ypos_nhit != 0){
	ypos = gUnpacker.get(k_device, 0, 0, 0, k_ypos);
	ypos = 500. - ( ypos / 1000. );
	hptr_array[ypos_id + seg]->Fill(ypos);
      }
      // XYpos
      if(xpos_nhit !=0 && ypos_nhit != 0){
	hptr_array[xypos_id + seg]->Fill(xpos, ypos);
      }

      xpos = gUnpacker.get(k_device, 0, 0, 0, k_xpos)
	- emc_x_offset;
      ypos = gUnpacker.get(k_device, 0, 0, 0, k_ypos)
	- emc_y_offset;
      double pos2spill = gEMC.Pos2Spill( xpos, ypos );
      // if( spill>pos2spill || ( pos2spill>spill && spill==1 ) ){
      if( spill==1 ){
	hptr_array[xypos_id + seg]->Reset();
      }
      spill = pos2spill;
      rspill = nspill - spill;

      if( spill>=0 ){
	std::stringstream ss;
	ss << "Spill# " << std::setw(4) << spill << "/" << nspill;
	int rsec  = int(rspill*5.52);
	int rhour = rsec/3600;
	int rmin  = rsec/60 - rhour*60;
	rsec = rsec%60; ss << " -> ";
	ss << std::setw(4) << rspill << " : "
	   << rhour << "h " << rmin << "m " << rsec << "s" << std::endl;
	text.SetText( 0.500, 0.930, ss.str().c_str() );
      }
    }
  }

  // // TriggerFlag ---------------------------------------------------
  // std::bitset<NumOfSegTFlag> trigger_flag;
  // {
  //   static const int k_device = gUnpacker.get_device_id("TFlag");
  //   static const int k_tdc    = gUnpacker.get_data_id("TFlag", "tdc");
  //   for( int seg=0; seg<NumOfSegTFlag; ++seg ){
  //     int nhit = gUnpacker.get_entries( k_device, 0, seg, 0, k_tdc );
  //     if( nhit>0 ){
  // 	int tdc = gUnpacker.get( k_device, 0, seg, 0, k_tdc );
  // 	if( tdc>0 ) trigger_flag.set(seg);
  //     }
  //   }
  // }

  // if( trigger_flag[SegIdScalerTrigger] ){
  //   ((TCanvas*)gROOT->FindObject("c5"))->Print("/tmp/c1.png");
  // }

  return 0;
}

}
