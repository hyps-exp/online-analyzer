// Author: Tomonori Takahashi

#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

#include <TGFileBrowser.h>
#include <TH1.h>
#include <TString.h>
#include <TStyle.h>
#include <TROOT.h>
#include <THttpServer.h>

#include "user_analyzer.hh"
#include "Controller.hh"
#include "ConfMan.hh"
#include "HistMaker.hh"
#include "MacroBuilder.hh"
#include "DetectorID.hh"
#include "HttpServer.hh"

#include <DAQNode.hh>
#include <filesystem_util.hh>
#include <Unpacker.hh>
#include <UnpackerManager.hh>

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  namespace{
    const int n_ch_tof = 24;
    std::vector<TH1*> hptr_array;
  }

//____________________________________________________________________________
int
process_begin(const std::vector<std::string>& argv)
{
  gStyle->SetOptStat(1110);
  gStyle->SetTitleW(.4);
  gStyle->SetTitleH(.1);
  // gStyle->SetStatW(.42);
  // gStyle->SetStatH(.35);
  gStyle->SetStatW(.32);
  gStyle->SetStatH(.25);
  gStyle->SetPalette(55);

  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  //  GUnpacker::get_instance().set_decode_mode(false);

//  hddaq::gui::Controller& gCon = hddaq::gui::Controller::getInstance();
//  TGFileBrowser *tab_hist  = gCon.makeFileBrowser("Hist");
//  TGFileBrowser *tab_macro = gCon.makeFileBrowser("Macro");

  HttpServer& gHttp = HttpServer::GetInstance();
  HistMaker& gHist  = HistMaker::getInstance();
  gHttp.SetPort(9090);
  gHttp.Open();
  gHttp.Register(gHist.createMsT());

  if(0 != gHist.setHistPtr(hptr_array)){ return -1; }

  // Add macros to the Macro tab
//  tab_macro->Add(macro::Get("clear_all_canvas"));
//  tab_macro->Add(macro::Get("clear_canvas"));

  // mst-tof ________________________________________
  const int mst_tof_id = gHist.getSequentialID( kMsT, 0, kTDC);
  for(int i = 0; i<n_ch_tof; ++i){
    hptr_array[mst_tof_id + i]->Rebin(10);
  }// for(i)

  for( Int_t i=0, n=hptr_array.size(); i<n; ++i ){
    hptr_array[i]->SetDirectory(0);
  }

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
process_event()
{
  static UnpackerManager& gUnpacker = GUnpacker::get_instance();
  HistMaker & gHist = HistMaker::getInstance();

  static int run_number = -1;
  if( run_number != gUnpacker.get_root()->get_run_number() ){
    for( Int_t i=0, n=hptr_array.size(); i<n; ++i ){
      hptr_array[i]->Reset();
    }
    run_number = gUnpacker.get_root()->get_run_number();
  }

  // trigger flag
  bool pipi_flag = false;
  const int tdc_min = 900;
  const int tdc_max = 1000;
  {
    static const int k_device = gUnpacker.get_device_id("TFlag");
    static const int k_tdc    = gUnpacker.get_data_id("TFlag", "tdc");

    int nhit = gUnpacker.get_entries( k_device, 0, trigger::kBeamPiPS, 0, k_tdc );
    for(int m = 0; m<nhit; ++m){
      int tdc = gUnpacker.get( k_device, 0, trigger::kBeamPiPS, 0, k_tdc, m );
      if(tdc_min < tdc && tdc < tdc_max) pipi_flag = true;
    }// for(m)
  }

  if(!pipi_flag) return 0;

  // MsT TOF
  {
    static const int k_device = gUnpacker.get_device_id("MsT");
    static const int k_tof    = gUnpacker.get_plane_id("MsT", "TOF");
    static const int k_tdc    = 0;

    static const int mst_tof_id = gHist.getSequentialID( kMsT, 0, kTDC);

    // TOF
    for(int i = 0; i<n_ch_tof; ++i){
      int nhit = gUnpacker.get_entries( k_device, k_tof, i, 0, k_tdc );
      for(int m = 0; m<nhit; ++m){
	int tdc = gUnpacker.get( k_device, k_tof, i, 0, k_tdc, m );
	hptr_array[mst_tof_id + i]->Fill(tdc);
      }// for(m)
    }// for(i)
  }

  return 0;
}

}
