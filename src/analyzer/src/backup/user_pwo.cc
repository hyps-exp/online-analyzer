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

#include "Controller.hh"

#include "user_analyzer.hh"
#include "UnpackerManager.hh"
#include "filesystem_util.hh"
#include "ConfMan.hh"
#include "UserParamMan.hh"
#include "HistMaker.hh"
#include "DetectorID.hh"
#include "PsMaker.hh"
#include "GuiPs.hh"
#include "MacroBuilder.hh"

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  std::vector<TH1*> hptr_array;

  static const int NofPWO    = 238;
  static const int NofPWOBox = 22;
  static const int NofUnit[NofPWOBox] =
    {
      6, 9, 6, 14, 14, 21, 14, 14, 6, 9, 6,
      6, 9, 6, 14, 14, 21, 14, 14, 6, 9, 6
    };
  static const int SegPWOBox[NofPWOBox] =
    {
      4, 0, 0, 4, 0, 0, 1, 5, 2, 1, 6,
      1, 2, 5, 6, 2, 1, 3, 7, 7, 3, 3,
    };

//____________________________________________________________________________
int
process_begin(const std::vector<std::string>& argv)
{
  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  if( !gConfMan.IsGood() ) return -1;
  // unpacker and all the parameter managers are initialized at this stage

  // Make tabs
  hddaq::gui::Controller& gCon = hddaq::gui::Controller::getInstance();
  TGFileBrowser *tab_hist  = gCon.makeFileBrowser("Hist");
  TGFileBrowser *tab_macro = gCon.makeFileBrowser("Macro");

  // Add macros to the Macro tab
  //  tab_macro->Add(hoge());
  tab_macro->Add(clear_canvas());

  // Add histograms to the Hist tab
  HistMaker& gHist = HistMaker::getInstance();
  // PWO ADC
  {
    TList *dir_adc = new TList();
    dir_adc->SetName("ADC");
    int base_id = gHist.getUniqueID(DetIdPWO, 0, kADC);
    int index = 0;
    for(int i=0; i<NofPWOBox; ++i){
      std::string box_type;
      switch(NofUnit[i]){
      case 21: box_type = "B"; break;
      case 14: box_type = "E"; break;
      case  9: box_type = "C"; break;
      case  6: box_type = "L"; break;
      default: std::cerr<<"########## crazyyyyyyyyy!!!!!!"<<std::endl; std::exit(-1);
      }
      TList *pwo_adc = new TList();
      pwo_adc->SetName(Form("PWO_Box_%02d  %s type",i+1, box_type.c_str()));
      for(int j=0; j<NofUnit[i];j++){
	std::string title;
	if(i<NofPWOBox/2)  title = Form("PWO U-%03d ADC", index+1);
	if(i>=NofPWOBox/2) title = Form("PWO D-%03d ADC", index+1-NofPWO/2);
	pwo_adc->Add(gHist.createTH1(base_id +index, title.c_str(), 0x1000, 0, 0x1000));
	index++;
      }
      dir_adc->Add(pwo_adc);
    }
    dir_adc->Add(gHist.createTH2(base_id +index++, "PWO_ADC_UP%ch",
    				 NofPWO/2, 0, NofPWO/2, 0x1000, 0, 0x1000, "channel", ""));
    dir_adc->Add(gHist.createTH2(base_id +index++, "PWO_ADC_DOWN%ch",
    				 NofPWO/2, 0, NofPWO/2, 0x1000, 0, 0x1000, "channel", ""));
    tab_hist->Add(dir_adc);
  }
  // PWO TDC
  {
    TList *dir_tdc = new TList();
    dir_tdc->SetName("TDC");
    int base_id = gHist.getUniqueID(DetIdPWO, 0, kTDC);
    int index = 0;
    for(int i=0; i<NofPWOBox; ++i){
      std::string box_type;
      switch(NofUnit[i]){
      case 21: box_type = "B"; break;
      case 14: box_type = "E"; break;
      case  9: box_type = "C"; break;
      case  6: box_type = "L"; break;
      default: std::cerr<<"########## crazyyyyyyyyy!!!!!!"<<std::endl; std::exit(-1);
      }
      TList *pwo_tdc = new TList();
      pwo_tdc->SetName(Form("PWO_Box_%02d  %s type",i+1, box_type.c_str()));
      for(int j=0; j<NofUnit[i];j++){
	std::string title;
	if(i<NofPWOBox/2)  title = Form("PWO U-%03d TDC", index+1);
	if(i>=NofPWOBox/2) title = Form("PWO D-%03d TDC", index+1-NofPWO/2);
	pwo_tdc->Add(gHist.createTH1(base_id +index, title.c_str(), 0x2400, 0, 0x2400));
	index++;
      }
      dir_tdc->Add(pwo_tdc);
    }
    dir_tdc->Add(gHist.createTH2(base_id +index++, "PWO_TDC_UP%ch",
    				 NofPWO/2, 0, NofPWO/2, 0x2400, 0, 0x2400, "channel", ""));
    dir_tdc->Add(gHist.createTH2(base_id +index++, "PWO_TDC_DOWN%ch",
    				 NofPWO/2, 0, NofPWO/2, 0x2400, 0, 0x2400, "channel", ""));
    tab_hist->Add(dir_tdc);
  }

  // Set histogram pointers to the vector sequentially.
  // This vector contains both TH1 and TH2.
  // Then you need to do down cast when you use TH2.
  if(0 != gHist.setHistPtr(hptr_array)){return -1;}

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

  // PWO -----------------------------------------------------------
  {
    static const int pwo_adc = gHist.getSequentialID(DetIdPWO, 0, kADC);
    static const int pwo_tdc = gHist.getSequentialID(DetIdPWO, 0, kTDC);

    ////////// ADC
    int index = 0;
    int plane = -1;
    TH2* hist_u = (TH2*)hptr_array[pwo_adc+NofPWO];
    TH2* hist_d = (TH2*)hptr_array[pwo_adc+NofPWO +1];
    for(int i=0; i<NofPWOBox; ++i){
      {
	switch(NofUnit[i]){
	case 21: plane = 0; break;
	case 14: plane = 1; break;
	case  9: plane = 2; break;
	case  6: plane = 3; break;
	default: std::cerr<<"########## crazyyyyyyyyy!!!!!!"<<std::endl; std::exit(-1);
	}
	for(int j=0; j<NofUnit[i];j++){
	  TH1* hist = hptr_array[pwo_adc+index];
	  int nhits = gUnpacker.get_entries(28, plane, SegPWOBox[i], j, 0);
	  if(nhits==0){ index++; continue; }
	  unsigned int val = gUnpacker.get(28, plane, SegPWOBox[i], j, 0);
	  // std::cout<<"# gUnpacker.get(28,"<<plane<<","<<SegPWOBox[i]<<","<<j
	  // 	   <<",0)  index: "<<index<<"  val:"<<val<<std::endl;
	  if(hist){ hist->Fill(val); }//std::cout<<"1";}
	  if(hist_u && index<NofPWO/2){  hist_u->Fill(index, val); }//std::cout<<"u";}
	  if(hist_d && index>=NofPWO/2){ hist_d->Fill(index-NofPWO/2, val); }//std::cout<<"d"; }
	  //std::cout<<"\n";
	  index++;
	}
      }
    }
    ////////// TDC
    {
      int index = 0;
      int plane = -1;
      TH2* hist_u = (TH2*)hptr_array[pwo_tdc+NofPWO];
      TH2* hist_d = (TH2*)hptr_array[pwo_tdc+NofPWO +1];
      for(int i=0; i<NofPWOBox; ++i){
	{
	  switch(NofUnit[i]){
	  case 21: plane = 0; break;
	  case 14: plane = 1; break;
	  case  9: plane = 2; break;
	  case  6: plane = 3; break;
	  default: std::cerr<<"########## crazyyyyyyyyy!!!!!!"<<std::endl; std::exit(-1);
	  }
	  for(int j=0; j<NofUnit[i];j++){
	    TH1* hist = hptr_array[pwo_tdc+index];
	    int nhits = gUnpacker.get_entries(28, plane, SegPWOBox[i], j, 1);
	    if(nhits==0){ index++; continue; }
	    unsigned int val = gUnpacker.get(28, plane, SegPWOBox[i], j, 1);
	    // std::cout<<"# gUnpacker.get(28,"<<plane<<","<<SegPWOBox[i]<<","<<j
	    // 	   <<",0)  index: "<<index<<"  val:"<<val<<std::endl;
	    if(hist){ hist->Fill(val); }//std::cout<<"1";}
	    if(hist_u && index<NofPWO/2){  hist_u->Fill(index, val); }//std::cout<<"u";}
	    if(hist_d && index>=NofPWO/2){ hist_d->Fill(index-NofPWO/2, val); }//std::cout<<"d"; }
	    index++;
	  }
	}
      }
    }
  }

  return 0;
}

}
