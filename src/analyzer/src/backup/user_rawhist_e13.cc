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
#define FLAG_DAQ 1

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
  gConfMan.InitializeDCGeomMan();
  gConfMan.InitializeDCTdcCalibMan();
  gConfMan.InitializeDCDriftParamMan();
  gConfMan.InitializeUserParamMan();
  gConfMan.InitializeGeAdcCalibMan();
  if( !gConfMan.IsGood() ) return -1;
  // unpacker and all the parameter managers are initialized at this stage

  // Make tabs
  hddaq::gui::Controller& gCon = hddaq::gui::Controller::getInstance();
  TGFileBrowser *tab_hist  = gCon.makeFileBrowser("Hist");
  TGFileBrowser *tab_macro = gCon.makeFileBrowser("Macro");
  TGFileBrowser *tab_btof  = gCon.makeFileBrowser("BTOF");

  // Add macros to the Macro tab
  //tab_macro->Add(hoge());
  tab_macro->Add(clear_all_canvas());
  tab_macro->Add(clear_canvas());
  tab_macro->Add(split22());
  tab_macro->Add(split32());
  tab_macro->Add(split33());
  tab_macro->Add(dispBH1());
  tab_macro->Add(dispBFT());
  tab_macro->Add(dispBH2());
  tab_macro->Add(dispACs_SFV());
  tab_macro->Add(dispSP0Adc());
  tab_macro->Add(dispSP0Tdc());
  tab_macro->Add(dispTOF());
  tab_macro->Add(dispLC());
  tab_macro->Add(dispGeAdc());
  tab_macro->Add(dispGeTdc());
  tab_macro->Add(dispPWOTdc());
  tab_macro->Add(dispBC3());
  tab_macro->Add(dispBC4());
  tab_macro->Add(dispSDC2());
  tab_macro->Add(dispHDC());
  tab_macro->Add(dispSDC3());
  tab_macro->Add(dispSDC4());
  tab_macro->Add(dispHitPat());
  tab_macro->Add(effBcOut());
  tab_macro->Add(effSdcIn());
  tab_macro->Add(effSdcOut());
  tab_macro->Add(auto_monitor_all());

  // Add histograms to the Hist tab
  HistMaker& gHist = HistMaker::getInstance();
  tab_hist->Add(gHist.createBH1());
  tab_hist->Add(gHist.createBFT());
  tab_hist->Add(gHist.createBC3());
  tab_hist->Add(gHist.createBC4());
  tab_hist->Add(gHist.createBMW(false));
  tab_hist->Add(gHist.createBH2());
  tab_hist->Add(gHist.createBAC_SAC());
  tab_hist->Add(gHist.createSDC2());
  tab_hist->Add(gHist.createHDC());
  tab_hist->Add(gHist.createSP0());
  tab_hist->Add(gHist.createSDC3());
  tab_hist->Add(gHist.createSDC4());
  tab_hist->Add(gHist.createTOF());
  tab_hist->Add(gHist.createTOFMT());
  tab_hist->Add(gHist.createSFV_SAC3());
  tab_hist->Add(gHist.createLC());
  tab_hist->Add(gHist.createGe());
  tab_hist->Add(gHist.createPWO());
  tab_hist->Add(gHist.createTriggerFlag());
  tab_hist->Add(gHist.createCorrelation());
  tab_hist->Add(gHist.createDAQ(false));

  // Add extra histogram
  int btof_id = gHist.getUniqueID(kMisc, 0, kTDC);
  tab_btof->Add(gHist.createTH1(btof_id, "BTOF",
				300, -10, 5,
				"BTOF [ns]", ""
				));

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
  gPsTab.setFilename("/home/sks/PSFile/e13_2015/default.ps");
  gPsTab.initialize(optList, detList);
  // ----------------------------------------------------------

  gStyle->SetOptStat(1110);
  gStyle->SetTitleW(.4);
  gStyle->SetTitleH(.1);
  //  gStyle->SetStatW(.42);
  //  gStyle->SetStatH(.35);
  gStyle->SetStatW(.32);
  gStyle->SetStatH(.25);

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

#if FLAG_DAQ
  // DAQ -------------------------------------------------------------
  {
    // node id
    static const int k_eb      = gUnpacker.get_fe_id("skseb");
    static const int k_vme     = gUnpacker.get_fe_id("vme01");
    static const int k_copper  = gUnpacker.get_fe_id("clite1");
    static const int k_easiroc = gUnpacker.get_fe_id("easiroc0");

    // sequential id
    static const int eb_id      = gHist.getSequentialID(kDAQ, kEB, kHitPat);
    static const int vme_id     = gHist.getSequentialID(kDAQ, kVME, kHitPat2D);
    static const int copper_id  = gHist.getSequentialID(kDAQ, kCopper, kHitPat2D);
    static const int easiroc_id = gHist.getSequentialID(kDAQ, kEASIROC, kHitPat2D);
    static const int tko_id     = gHist.getSequentialID(kDAQ, kTKO, kHitPat2D);

    {
      // EB
      int data_size = gUnpacker.get_node_header(k_eb, DAQNode::k_data_size);
      hptr_array[eb_id]->Fill(data_size);
    }

    {
      // VME node
      TH2* h = dynamic_cast<TH2*>(hptr_array[vme_id]);
      for(int i = 0; i<6; ++i){
	if(i == 1){continue;}
	int data_size = gUnpacker.get_node_header(k_vme+i, DAQNode::k_data_size);
	h->Fill(i+1, data_size);
      }
    }

    {
      // Copper node
      TH2* h = dynamic_cast<TH2*>(hptr_array[copper_id]);
      for(int i = 0; i<14; ++i){
	int data_size = gUnpacker.get_node_header(k_copper+i, DAQNode::k_data_size);
	h->Fill(i+1, data_size);
      }
    }

    {
      // EASIROC node
      TH2* h = dynamic_cast<TH2*>(hptr_array[easiroc_id]);
      for(int i = 0; i<10; ++i){
	int data_size = gUnpacker.get_node_header(k_easiroc+i, DAQNode::k_data_size);
	h->Fill(i+1, data_size);
      }
    }

    {
      // TKO box
      static const int addr[] = {0x10000000, 0x10200000, 0x10400000, 0x10600000,
				 0x10800000, 0x10a00000};

      for(int smp = 0; smp<6; ++smp){
	TH2* h = dynamic_cast<TH2*>(hptr_array[tko_id+smp]);
	for(int ma = 0; ma<24; ++ma){
	  int nhit = gUnpacker.get_fe_info(k_vme, addr[smp], ma);
	  h->Fill(ma, nhit);
	}
      }
    }

  }

#endif

  if(scaler_flag) return 0;

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BH1 -----------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("BH1");
    static const int k_u      = 0; // up
    static const int k_d      = 1; // down
    static const int k_adc    = gUnpacker.get_data_id("BH1", "adc");
    static const int k_tdc    = gUnpacker.get_data_id("BH1", "tdc");

    // Up PMT
    int bh1a_id = gHist.getSequentialID(kBH1, 0, kADC);
    int bh1t_id = gHist.getSequentialID(kBH1, 0, kTDC);
    for(int seg=0; seg<NumOfSegBH1; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc);
      if(nhit!=0){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	hptr_array[bh1a_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      if(nhit!=0){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	if(tdc!=0){ hptr_array[bh1t_id + seg]->Fill(tdc); }
      }
    }

    // Down PMT
    bh1a_id = gHist.getSequentialID(kBH1, 0, kADC, NumOfSegBH1+1);
    bh1t_id = gHist.getSequentialID(kBH1, 0, kTDC, NumOfSegBH1+1);
    for(int seg=0; seg<NumOfSegBH1; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc);
      if(nhit!=0){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	hptr_array[bh1a_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      if(nhit!=0){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
	if(tdc!=0){ hptr_array[bh1t_id + seg]->Fill(tdc); }
      }
    }

    // Hit pattern && multiplicity
    static const int bh1hit_id = gHist.getSequentialID(kBH1, 0, kHitPat);
    static const int bh1mul_id = gHist.getSequentialID(kBH1, 0, kMulti);
    int multiplicity = 0;
    for(int seg=0; seg<NumOfSegBH1; ++seg){
      int nhit_bh1u = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      int nhit_bh1d = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      // AND
      if(nhit_bh1u!=0 && nhit_bh1d!=0){
	unsigned int tdc_u = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	unsigned int tdc_d = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
	// TDC AND
	if(tdc_u != 0 && tdc_d != 0){
	  hptr_array[bh1hit_id]->Fill(seg);
	  ++multiplicity;
	}
      }
    }

    hptr_array[bh1mul_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BFT -----------------------------------------------------------
  {
    // data type
    static const int k_device  = gUnpacker.get_device_id("BFT");
    static const int k_uplane  = gUnpacker.get_plane_id("BFT", "upstream");
    static const int k_dplane  = gUnpacker.get_plane_id("BFT", "downstream");
    static const int k_leading = gUnpacker.get_data_id("BFT", "leading");
    static const int k_trailing = gUnpacker.get_data_id("BFT", "trailing");

    // TDC gate range
    UserParamMan& gPar = UserParamMan::getInstance();
    static const int tdc_min = gPar.getParameter("BFT_TDC", 0);
    static const int tdc_max = gPar.getParameter("BFT_TDC", 1);

    // sequential id
    static const int bft_tu_id    = gHist.getSequentialID(kBFT, 0, kTDC,      1);
    static const int bft_td_id    = gHist.getSequentialID(kBFT, 0, kTDC,      2);
    static const int bft_ctu_id   = gHist.getSequentialID(kBFT, 0, kTDC,     11);
    static const int bft_ctd_id   = gHist.getSequentialID(kBFT, 0, kTDC,     12);
    static const int bft_totu_id  = gHist.getSequentialID(kBFT, 0, kADC,      1);
    static const int bft_totd_id  = gHist.getSequentialID(kBFT, 0, kADC,      2);
    static const int bft_ctotu_id = gHist.getSequentialID(kBFT, 0, kADC,     11);
    static const int bft_ctotd_id = gHist.getSequentialID(kBFT, 0, kADC,     12);
    static const int bft_hitu_id  = gHist.getSequentialID(kBFT, 0, kHitPat,   1);
    static const int bft_hitd_id  = gHist.getSequentialID(kBFT, 0, kHitPat,   2);
    static const int bft_chitu_id = gHist.getSequentialID(kBFT, 0, kHitPat,  11);
    static const int bft_chitd_id = gHist.getSequentialID(kBFT, 0, kHitPat,  12);
    static const int bft_mul_id   = gHist.getSequentialID(kBFT, 0, kMulti,    1);
    static const int bft_cmul_id  = gHist.getSequentialID(kBFT, 0, kMulti,   11);

    static const int bft_ctu_2d_id = gHist.getSequentialID(kBFT, 0, kTDC2D,   1);
    static const int bft_ctd_2d_id = gHist.getSequentialID(kBFT, 0, kTDC2D,   2);
    static const int bft_ctotu_2d_id = gHist.getSequentialID(kBFT, 0, kADC2D, 1);
    static const int bft_ctotd_2d_id = gHist.getSequentialID(kBFT, 0, kADC2D, 2);

    int multiplicity  = 0; // includes both u and d planes.
    int cmultiplicity = 0; // includes both u and d planes.
    int tdc_prev      = 0;
    for(int i = 0; i<NumOfSegBFT; ++i){
      int nhit_u = gUnpacker.get_entries(k_device, k_uplane, 0, i, k_leading);
      int nhit_d = gUnpacker.get_entries(k_device, k_dplane, 0, i, k_leading);

      // u plane
      tdc_prev = 0;
      for(int m = 0; m<nhit_u; ++m){
	int tdc = gUnpacker.get(k_device, k_uplane, 0, i, k_leading, m);
	int tdc_t = gUnpacker.get(k_device, k_uplane, 0, i, k_trailing, m);
	int tot = tdc - tdc_t;
	hptr_array[bft_tu_id]->Fill(tdc);
	hptr_array[bft_totu_id]->Fill(tot);
	if(tdc_min < tdc && tdc < tdc_max){
	  ++multiplicity;
	  hptr_array[bft_hitu_id]->Fill(i);
	}
	if(tdc_prev==tdc) continue;
	tdc_prev = tdc;
	if(tot==0) continue;
	hptr_array[bft_ctu_id]->Fill(tdc);
	hptr_array[bft_ctotu_id]->Fill(tot);
	hptr_array[bft_ctu_2d_id]->Fill(i, tdc);
	hptr_array[bft_ctotu_2d_id]->Fill(i, tot);
	if(tdc_min < tdc && tdc < tdc_max){
	  ++cmultiplicity;
	  hptr_array[bft_chitu_id]->Fill(i);
	}
      }

      // d plane
      tdc_prev = 0;
      for(int m = 0; m<nhit_d; ++m){
	int tdc = gUnpacker.get(k_device, k_dplane, 0, i, k_leading, m);
	int tdc_t = gUnpacker.get(k_device, k_dplane, 0, i, k_trailing, m);
	int tot = tdc - tdc_t;
	hptr_array[bft_td_id]->Fill(tdc);
	hptr_array[bft_totd_id]->Fill(tot);
	if(tdc_min < tdc && tdc < tdc_max){
	  ++multiplicity;
	  hptr_array[bft_hitd_id]->Fill(i);
	}
	if(tdc_prev==tdc) continue;
	tdc_prev = tdc;
	if(tot==0) continue;
	hptr_array[bft_ctd_id]->Fill(tdc);
	hptr_array[bft_ctotd_id]->Fill(tot);
	hptr_array[bft_ctd_2d_id]->Fill(i, tdc);
	hptr_array[bft_ctotd_2d_id]->Fill(i, tot);
	if(tdc_min < tdc && tdc < tdc_max){
	  ++cmultiplicity;
	  hptr_array[bft_chitd_id]->Fill(i);
	}
      }
    }
    hptr_array[bft_mul_id]->Fill(multiplicity);
    hptr_array[bft_cmul_id]->Fill(cmultiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BC3 -------------------------------------------------------------
  {
    // data type
    static const int k_device  = gUnpacker.get_device_id("BC3");
    static const int k_tdc     = 0;

    // TDC gate range
    static const int tdc_min
      = UserParamMan::getInstance().getParameter("BC3_TDC", 0);
    static const int tdc_max
      = UserParamMan::getInstance().getParameter("BC3_TDC", 1);

    // sequential id
    static const int bc3t_id   = gHist.getSequentialID(kBC3, 0, kTDC);
    static const int bc3hit_id = gHist.getSequentialID(kBC3, 0, kHitPat);
    static const int bc3mul_id = gHist.getSequentialID(kBC3, 0, kMulti);
    static const int bc3mulwt_id
      = gHist.getSequentialID(kBC3, 0, kMulti, 1+NumOfLayersBC3);

    // TDC & HitPat & Multi
    for(int l = 0; l<NumOfLayersBC3; ++l){

      int multiplicity    = 0;
      int multiplicity_wt = 0;
      for(int w = 0; w<NumOfWireBC3; ++w){
	int nhit = gUnpacker.get_entries(k_device, l, 0, w, k_tdc);
	if(nhit == 0){continue;}

	// This wire fired at least one times.
	++multiplicity;
	hptr_array[bc3hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	for(int m = 0; m<nhit; ++m){
	  int tdc = gUnpacker.get(k_device, l, 0, w, k_tdc, m);
	  hptr_array[bc3t_id + l]->Fill(tdc);

	  // Drift time check
	  if(tdc_min < tdc && tdc < tdc_max){
	    flag_hit_wt = true;
	  }
	}

	if(flag_hit_wt){ ++multiplicity_wt; }
      }

      hptr_array[bc3mul_id + l]->Fill(multiplicity);
      hptr_array[bc3mulwt_id + l]->Fill(multiplicity_wt);
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BC4 -------------------------------------------------------------
  {
    // data type
    static const int k_device  = gUnpacker.get_device_id("BC4");
    static const int k_tdc = 0;

    // TDC gate range
    static const int tdc_min
      = UserParamMan::getInstance().getParameter("BC4_TDC", 0);
    static const int tdc_max
      = UserParamMan::getInstance().getParameter("BC4_TDC", 1);

    // sequential id
    static const int bc4t_id   = gHist.getSequentialID(kBC4, 0, kTDC);
    static const int bc4hit_id = gHist.getSequentialID(kBC4, 0, kHitPat);
    static const int bc4mul_id = gHist.getSequentialID(kBC4, 0, kMulti);
    static const int bc4mulwt_id
      = gHist.getSequentialID(kBC4, 0, kMulti, 1+NumOfLayersBC4);

    // TDC & HitPat & Multi
    for(int l = 0; l<NumOfLayersBC4; ++l){

      int multiplicity    = 0;
      int multiplicity_wt = 0;
      for(int w = 0; w<NumOfWireBC4; ++w){
	int nhit = gUnpacker.get_entries(k_device, l, 0, w, k_tdc);
	if(nhit == 0){continue;}

	// This wire fired at least one times.
	++multiplicity;
	hptr_array[bc4hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	for(int m = 0; m<nhit; ++m){
	  int tdc = gUnpacker.get(k_device, l, 0, w, k_tdc, m);
	  hptr_array[bc4t_id + l]->Fill(tdc);

	  // Drift time check
	  if(tdc_min < tdc && tdc < tdc_max){
	    flag_hit_wt = true;
	  }
	}

	if(flag_hit_wt){ ++multiplicity_wt; }
      }

      hptr_array[bc4mul_id + l]->Fill(multiplicity);
      hptr_array[bc4mulwt_id + l]->Fill(multiplicity_wt);
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BMW -------------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("BMW");
    static const int k_adc    = gUnpacker.get_data_id("BMW", "adc");
    static const int k_tdc    = gUnpacker.get_data_id("BMW", "tdc");

    // sequential id
    static const int bmwa_id = gHist.getSequentialID(kBMW, 0, kADC);
    static const int bmwt_id = gHist.getSequentialID(kBMW, 0, kTDC);

    // ADC
    int nhit_a = gUnpacker.get_entries(k_device, 0, 0, 0, k_adc);
    if(nhit_a != 0){
      int adc = gUnpacker.get(k_device, 0, 0, 0, k_adc);
      hptr_array[bmwa_id]->Fill(adc, nhit_a);
    }

    // TDC
    int nhit_t = gUnpacker.get_entries(k_device, 0, 0, 0, k_tdc);
    if(nhit_t != 0){
      int tdc = gUnpacker.get(k_device, 0, 0, 0, k_tdc);
      if(tdc != 0){ hptr_array[bmwt_id]->Fill(tdc, nhit_t); }
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BH2 -----------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("BH2");
    static const int k_u      = 0; // up
    //    static const int k_d   = 1; // down
    static const int k_adc    = gUnpacker.get_data_id("BH2", "adc");
    static const int k_tdc    = gUnpacker.get_data_id("BH2", "tdc");

    // sequential id
    static const int bh2a_id = gHist.getSequentialID(kBH2, 0, kADC);
    static const int bh2t_id = gHist.getSequentialID(kBH2, 0, kTDC);
    for(int seg=0; seg<NumOfSegBH2; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc);
      if(nhit != 0){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	hptr_array[bh2a_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      if(nhit != 0){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	if(tdc != 0){ hptr_array[bh2t_id + seg]->Fill(tdc); }
      }
    }

    // Hit pattern &&  Multiplicity
    static const int bh2hit_id = gHist.getSequentialID(kBH2, 0, kHitPat);
    static const int bh2mul_id = gHist.getSequentialID(kBH2, 0, kMulti);
    int multiplicity = 0;
    for(int seg=0; seg<NumOfSegBH2; ++seg){
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);

      if(nhit != 0){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	// TDC
	if(tdc != 0){
	  hptr_array[bh2hit_id]->Fill(seg);
	  ++multiplicity;
	}
      }
    }

    hptr_array[bh2mul_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BAC_SAC ---------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("BAC");
    static const int k_adc    = gUnpacker.get_data_id("BAC","adc");
    static const int k_tdc    = gUnpacker.get_data_id("BAC","tdc");

    // sequential id
    static const int baca_id = gHist.getSequentialID(kBAC_SAC, 0, kADC, 1);
    static const int bact_id = gHist.getSequentialID(kBAC_SAC, 0, kTDC, 1);

    for(int seg = 0; seg<NumOfSegBAC_SAC; ++seg){
      // ADC
      int nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      if(nhit_a != 0){
	int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[baca_id + seg]->Fill(adc, nhit_a);
      }

      // TDC
      int nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      if(nhit_t != 0){
	int tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc);
	if(tdc != 0){ hptr_array[bact_id + seg]->Fill(tdc, nhit_t); }
      }
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // SDC2 ------------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("SDC2");
    static const int k_tdc    = 0;

    // TDC gate range
    static const int tdc_min
      = UserParamMan::getInstance().getParameter("SDC2_TDC", 0);
    static const int tdc_max
      = UserParamMan::getInstance().getParameter("SDC2_TDC", 1);

    // sequential id
    static const int sdc2t_id   = gHist.getSequentialID(kSDC2, 0, kTDC, 1);
    static const int sdc2hit_id = gHist.getSequentialID(kSDC2, 0, kHitPat, 1);
    static const int sdc2mul_id = gHist.getSequentialID(kSDC2, 0, kMulti, 1);
    static const int sdc2mulwt_id
      = gHist.getSequentialID(kSDC2, 0, kMulti, 1+NumOfLayersSDC2);

    // TDC & HitPat & Multi
    for(int l = 0; l<NumOfLayersSDC2; ++l){

      int multiplicity    = 0;
      int multiplicity_wt = 0;
      for(int w = 0; w<NumOfWireSDC2; ++w){
	int nhit = gUnpacker.get_entries(k_device, l, 0, w, k_tdc);
	if(nhit == 0){continue;}

	// This wire fired at least one times.
	++multiplicity;
	hptr_array[sdc2hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	for(int m = 0; m<nhit; ++m){
	  int tdc = gUnpacker.get(k_device, l, 0, w, k_tdc, m);
	  hptr_array[sdc2t_id + l]->Fill(tdc);

	  // Drift time check
	  if(tdc_min < tdc && tdc < tdc_max){
	    flag_hit_wt = true;
	  }
	}

	if(flag_hit_wt){ ++multiplicity_wt; }
      }

      hptr_array[sdc2mul_id + l]->Fill(multiplicity);
      hptr_array[sdc2mulwt_id + l]->Fill(multiplicity_wt);
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // HDC ------------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("HDC");
    static const int k_tdc    = 0;

    // TDC gate range
    static const int tdc_min
      = UserParamMan::getInstance().getParameter("HDC_TDC", 0);
    static const int tdc_max
      = UserParamMan::getInstance().getParameter("HDC_TDC", 1);

    // sequential id
    static const int hdct_id   = gHist.getSequentialID(kHDC, 0, kTDC, 1);
    static const int hdchit_id = gHist.getSequentialID(kHDC, 0, kHitPat, 1);
    static const int hdcmul_id = gHist.getSequentialID(kHDC, 0, kMulti, 1);
    static const int hdcmulwt_id
      = gHist.getSequentialID(kHDC, 0, kMulti, 1+NumOfLayersHDC);

    // TDC & HitPat & Multi
    for(int l = 0; l<NumOfLayersHDC; ++l){

      int multiplicity    = 0;
      int multiplicity_wt = 0;
      for(int w = 0; w<NumOfWireHDC; ++w){
	int nhit = gUnpacker.get_entries(k_device, l, 0, w, k_tdc);
	if(nhit == 0){continue;}

	// This wire fired at least one times.
	++multiplicity;
	hptr_array[hdchit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	for(int m = 0; m<nhit; ++m){
	  int tdc = gUnpacker.get(k_device, l, 0, w, k_tdc, m);
	  hptr_array[hdct_id + l]->Fill(tdc);

	  // Drift time check
	  if(tdc_min < tdc && tdc < tdc_max){
	    flag_hit_wt = true;
	  }
	}

	if(flag_hit_wt){ ++multiplicity_wt; }
      }

      hptr_array[hdcmul_id + l]->Fill(multiplicity);
      hptr_array[hdcmulwt_id + l]->Fill(multiplicity_wt);
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device, 1);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // SP0 -----------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("SP0");
    static const int k_u      = 0; // up
    static const int k_d      = 1; // down
    static const int k_adc    = gUnpacker.get_data_id("SP0","adc");
    static const int k_tdc    = gUnpacker.get_data_id("SP0","tdc");

    for(int l = 0; l<NumOfLayersSP0; ++l){
      int sp0a_id  = gHist.getSequentialID(kSP0, kSP0_L1+l, kADC);
      int sp0t_id  = gHist.getSequentialID(kSP0, kSP0_L1+l, kTDC);
      int sp0hu_id = gHist.getSequentialID(kSP0, kSP0_L1+l, kHitPat, 1);
      int sp0hd_id = gHist.getSequentialID(kSP0, kSP0_L1+l, kHitPat, 2);

      for(int seg=0; seg<NumOfSegSP0; ++seg){
	// ADC
	int nhit = gUnpacker.get_entries(k_device, l, seg, k_u, k_adc);
	if(nhit != 0){
	  unsigned int adc = gUnpacker.get(k_device, l, seg, k_u, k_adc);
	  hptr_array[sp0a_id + seg]->Fill(adc);
	}

	// TDC
	nhit = gUnpacker.get_entries(k_device, l, seg, k_u, k_tdc);
	if(nhit != 0){
	  unsigned int tdc = gUnpacker.get(k_device, l, seg, k_u, k_tdc);
	  if(tdc != 0){
	    hptr_array[sp0t_id + seg]->Fill(tdc);
	    hptr_array[sp0hu_id]->Fill(seg);
	  }
	}
      }

      // Down PMT
      sp0a_id = gHist.getSequentialID(kSP0, kSP0_L1+l, kADC, NumOfSegSP0+1);
      sp0t_id = gHist.getSequentialID(kSP0, kSP0_L1+l, kTDC, NumOfSegSP0+1);

      for(int seg=0; seg<NumOfSegSP0; ++seg){
	// ADC
	int nhit = gUnpacker.get_entries(k_device, l, seg, k_d, k_adc);
	if(nhit != 0){
	  unsigned int adc = gUnpacker.get(k_device, l, seg, k_d, k_adc);
	  hptr_array[sp0a_id + seg]->Fill(adc);
	}

	// TDC
	nhit = gUnpacker.get_entries(k_device, l, seg, k_d, k_tdc);
	if(nhit != 0){
	  unsigned int tdc = gUnpacker.get(k_device, l, seg, k_d, k_tdc);
	  if(tdc != 0){
	    hptr_array[sp0t_id + seg]->Fill(tdc);
	    hptr_array[sp0hd_id]->Fill(seg);
	  }
	}
      }
    }
#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // SDC3 ------------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("SDC3");
    static const int k_tdc    = 0;

    // TDC gate range
    static const int tdc_min
      = UserParamMan::getInstance().getParameter("SDC3_TDC", 0);
    static const int tdc_max
      = UserParamMan::getInstance().getParameter("SDC3_TDC", 1);

    // sequential id
    static const int sdc3t_id   = gHist.getSequentialID(kSDC3, 0, kTDC);
    static const int sdc3hit_id = gHist.getSequentialID(kSDC3, 0, kHitPat);
    static const int sdc3mul_id = gHist.getSequentialID(kSDC3, 0, kMulti);
    static const int sdc3mulwt_id
      = gHist.getSequentialID(kSDC3, 0, kMulti, 1+NumOfLayersSDC3);

    // TDC & HitPat & Multi
    for(int l = 0; l<NumOfLayersSDC3; ++l){

      int multiplicity    = 0;
      int multiplicity_wt = 0;

      int num_of_wire_SDC3 = 0;
      if(l ==1 || l ==4){
	num_of_wire_SDC3 = NumOfWireSDC3x;
      }else{
	num_of_wire_SDC3 = NumOfWireSDC3uv;
      }

      for(int w = 0; w<num_of_wire_SDC3; ++w){
	int nhit = gUnpacker.get_entries(k_device, l, 0, w, k_tdc);
	if(nhit == 0){continue;}

	// This wire fired at least one times.
	++multiplicity;
	hptr_array[sdc3hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	for(int m = 0; m<nhit; ++m){
	  int tdc = gUnpacker.get(k_device, l, 0, w, k_tdc, m);
	  hptr_array[sdc3t_id + l]->Fill(tdc);

	  // Drift time check
	  if(tdc_min < tdc && tdc < tdc_max){
	    flag_hit_wt = true;
	  }
	}

	if(flag_hit_wt){ ++multiplicity_wt; }
      }

      hptr_array[sdc3mul_id + l]->Fill(multiplicity);
      hptr_array[sdc3mulwt_id + l]->Fill(multiplicity_wt);
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // SDC4 ------------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("SDC4");
    static const int k_tdc    = 0;

    // TDC gate range
    static const int tdc_min
      = UserParamMan::getInstance().getParameter("SDC4_TDC", 0);
    static const int tdc_max
      = UserParamMan::getInstance().getParameter("SDC4_TDC", 1);

    // sequential id
    static const int sdc4t_id   = gHist.getSequentialID(kSDC4, 0, kTDC);
    static const int sdc4hit_id = gHist.getSequentialID(kSDC4, 0, kHitPat);
    static const int sdc4mul_id = gHist.getSequentialID(kSDC4, 0, kMulti);
    static const int sdc4mulwt_id
      = gHist.getSequentialID(kSDC4, 0, kMulti, 1+NumOfLayersSDC4);

    // TDC & HitPat & Multi
    for(int l = 0; l<NumOfLayersSDC4; ++l){

      int multiplicity    = 0;
      int multiplicity_wt = 0;

      int num_of_wire_SDC4 = 0;
      if(l ==1 || l ==4){
	num_of_wire_SDC4 = NumOfWireSDC4x;
      }else{
	num_of_wire_SDC4 = NumOfWireSDC4uv;
      }

      for(int w = 0; w<num_of_wire_SDC4; ++w){
	int nhit = gUnpacker.get_entries(k_device, l, 0, w, k_tdc);
	if(nhit == 0){continue;}

	// This wire fired at least one times.
	++multiplicity;
	hptr_array[sdc4hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	for(int m = 0; m<nhit; ++m){
	  int tdc = gUnpacker.get(k_device, l, 0, w, k_tdc, m);
	  hptr_array[sdc4t_id + l]->Fill(tdc);

	  // Drift time check
	  if(tdc_min < tdc && tdc < tdc_max){
	    flag_hit_wt = true;
	  }
	}

	if(flag_hit_wt){ ++multiplicity_wt; }
      }

      hptr_array[sdc4mul_id + l]->Fill(multiplicity);
      hptr_array[sdc4mulwt_id + l]->Fill(multiplicity_wt);
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // TOF -----------------------------------------------------------
  {
    // data typep
    static const int k_device = gUnpacker.get_device_id("TOF");
    static const int k_u      = 0; // up
    static const int k_d      = 1; // down
    static const int k_adc    = gUnpacker.get_data_id("TOF","adc");
    static const int k_tdc    = gUnpacker.get_data_id("TOF","tdc");

    // sequential id
    int tofa_id = gHist.getSequentialID(kTOF, 0, kADC);
    int toft_id = gHist.getSequentialID(kTOF, 0, kTDC);
    for(int seg = 0; seg<NumOfSegTOF; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc);
      if(nhit != 0){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	hptr_array[tofa_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      if(nhit != 0){
	int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	if(tdc != 0){ hptr_array[toft_id + seg]->Fill(tdc); }
      }
    }

    // Down PMT
    tofa_id = gHist.getSequentialID(kTOF, 0, kADC, NumOfSegTOF+1);
    toft_id = gHist.getSequentialID(kTOF, 0, kTDC, NumOfSegTOF+1);

    for(int seg = 0; seg<NumOfSegTOF; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc);
      if(nhit != 0){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	hptr_array[tofa_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      if(nhit != 0){
	int tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
	if(tdc != 0){ hptr_array[toft_id + seg]->Fill(tdc); }
      }
    }

    // Hit pattern && multiplicity
    static const int tofhit_id = gHist.getSequentialID(kTOF, 0, kHitPat);
    static const int tofmul_id = gHist.getSequentialID(kTOF, 0, kMulti);
    int multiplicity = 0;
    for(int seg=0; seg<NumOfSegTOF; ++seg){
      int nhit_tofu = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      int nhit_tofd = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      // AND
      if(nhit_tofu!=0 && nhit_tofd!=0){
	unsigned int tdc_u = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	unsigned int tdc_d = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
	// TDC AND
	if(tdc_u != 0 && tdc_d != 0){
	  hptr_array[tofhit_id]->Fill(seg);
	  ++multiplicity;
	}
      }
    }

    hptr_array[tofmul_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // TOFMT -----------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("TOFMT");
    static const int k_u      = 0; // up
    //    static const int k_d   = 1; // down
    //    static const int k_adc = 0;
    static const int k_tdc    = gUnpacker.get_data_id("TOFMT","tdc");

    // sequential id
    static const int tofmtt_id = gHist.getSequentialID(kTOFMT, 0, kTDC);
    for(int seg=0; seg<NumOfSegTOF; ++seg){
      // TDC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      if(nhit != 0){
	int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	if(tdc != 0){ hptr_array[tofmtt_id + seg]->Fill(tdc); }
      }
    }

    // Hit pattern && multiplicity
    static const int tofmthit_id = gHist.getSequentialID(kTOFMT, 0, kHitPat);
    static const int tofmtmul_id = gHist.getSequentialID(kTOFMT, 0, kMulti);
    int multiplicity = 0;
    for(int seg=0; seg<NumOfSegTOF; ++seg){
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      // AND
      if(nhit != 0){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	// TDC AND
	if(tdc != 0){
	  hptr_array[tofmthit_id]->Fill(seg);
	  ++multiplicity;
	}
      }
    }
    hptr_array[tofmtmul_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

  // SFV_SAC3 --------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("SFV");
    static const int k_adc    = gUnpacker.get_data_id("SFV","adc");
    static const int k_tdc    = gUnpacker.get_data_id("SFV","tdc");

    // sequential id
    static const int sfva_id = gHist.getSequentialID(kSFV_SAC3, 0, kADC);
    static const int sfvt_id = gHist.getSequentialID(kSFV_SAC3, 0, kTDC);

    static const int NofLoop = 7; // SFV 6 seg + SAC3 1 seg
    for(int seg = 0; seg<NofLoop; ++seg){
      // ADC
      int nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      if(nhit_a != 0){
	int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[sfva_id + seg]->Fill(adc, nhit_a);
      }

      // TDC
      int nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      if(nhit_t != 0){
	int tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc);
	if(tdc != 0){ hptr_array[sfvt_id + seg]->Fill(tdc, nhit_t); }
      }
    }

    // Hit pattern
    static const int sfvhit_id = gHist.getSequentialID(kSFV_SAC3, 0, kHitPat);
    for(int seg = 0; seg<NofLoop; ++seg){
      int nhit = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      if(nhit != 0){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc);
	if(tdc != 0){
	  hptr_array[sfvhit_id]->Fill(seg);
	}
      }
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // LC -----------------------------------------------------------
  {
    // data typep
    static const int k_device = gUnpacker.get_device_id("LC");
    static const int k_u      = 0; // up
    static const int k_d      = 1; // down
    static const int k_adc    = gUnpacker.get_data_id("LC","adc");
    static const int k_tdc    = gUnpacker.get_data_id("LC","tdc");

    // sequential id
    int lca_id = gHist.getSequentialID(kLC, 0, kADC);
    int lct_id = gHist.getSequentialID(kLC, 0, kTDC);
    for(int seg = 0; seg<NumOfSegLC; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc);
      if(nhit != 0){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	hptr_array[lca_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      if(nhit != 0){
	int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	if(tdc != 0){ hptr_array[lct_id + seg]->Fill(tdc); }
      }
    }

    // Down PMT
    lca_id = gHist.getSequentialID(kLC, 0, kADC, NumOfSegLC+1);
    lct_id = gHist.getSequentialID(kLC, 0, kTDC, NumOfSegLC+1);

    for(int seg = 0; seg<NumOfSegLC; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc);
      if(nhit != 0){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	hptr_array[lca_id + seg]->Fill(adc);
      }

      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      if(nhit != 0){
	int tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
	if(tdc != 0){ hptr_array[lct_id + seg]->Fill(tdc); }
      }
    }

    // Hit pattern && multiplicity
    static const int lchit_id = gHist.getSequentialID(kLC, 0, kHitPat);
    static const int lcmul_id = gHist.getSequentialID(kLC, 0, kMulti);
    int multiplicity = 0;
    for(int seg=0; seg<NumOfSegLC; ++seg){
      int nhit_lcu = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      int nhit_lcd = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      // AND
      if(nhit_lcu!=0 && nhit_lcd!=0){
	unsigned int tdc_u = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	unsigned int tdc_d = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
	// TDC AND
	if(tdc_u != 0 && tdc_d != 0){
	  hptr_array[lchit_id]->Fill(seg);
	  ++multiplicity;
	}
      }
    }

    hptr_array[lcmul_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // Correlation (2D histograms) -------------------------------------
  {
    // data typep
    static const int k_device_bh1 = gUnpacker.get_device_id("BH1");
    static const int k_device_bh2 = gUnpacker.get_device_id("BH2");
    static const int k_device_tof = gUnpacker.get_device_id("TOF");
    static const int k_device_lc  = gUnpacker.get_device_id("LC");
    static const int k_device_bc3 = gUnpacker.get_device_id("BC3");
    static const int k_device_bc4 = gUnpacker.get_device_id("BC4");
    static const int k_device_sdc2= gUnpacker.get_device_id("SDC2");
    static const int k_device_hdc = gUnpacker.get_device_id("HDC");

    // sequential id
    int cor_id= gHist.getSequentialID(kCorrelation, 0, 0, 1);

    // BH1 vs BH2
    TH2* hcor_bh1bh2 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(int seg1 = 0; seg1<NumOfSegBH1; ++seg1){
      for(int seg2 = 0; seg2<NumOfSegBH2; ++seg2){
	int hitBH1 = gUnpacker.get_entries(k_device_bh1, 0, seg1, 0, 1);
	int hitBH2 = gUnpacker.get_entries(k_device_bh2, 0, seg2, 0, 1);
	if(hitBH1 == 0 || hitBH2 == 0)continue;
	int tdcBH1 = gUnpacker.get(k_device_bh1, 0, seg1, 0, 1);
	int tdcBH2 = gUnpacker.get(k_device_bh2, 0, seg2, 0, 1);
	if(tdcBH1 != 0 && tdcBH2 != 0){
	  hcor_bh1bh2->Fill(seg1, seg2);
	}
      }
    }

    // TOF vs LC
    TH2* hcor_toflc = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(int seg1 = 0; seg1<NumOfSegTOF; ++seg1){
      for(int seg2 = 0; seg2<NumOfSegLC; ++seg2){
	int hitTOF = gUnpacker.get_entries(k_device_tof, 0, seg1, 0, 1);
	int hitLC  = gUnpacker.get_entries(k_device_lc,  0, seg2, 0, 1);
	if(hitTOF == 0 || hitLC == 0)continue;
	int tdcTOF = gUnpacker.get(k_device_tof, 0, seg1, 0, 1);
	int tdcLC  = gUnpacker.get(k_device_lc,  0, seg2, 0, 1);
	if(tdcTOF != 0 && tdcLC != 0){
	  hcor_toflc->Fill(seg1, seg2);
	}
      }
    }

    // BC3 vs BC4
    TH2* hcor_bc3bc4 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(int wire1 = 0; wire1<NumOfWireBC3; ++wire1){
      for(int wire2 = 0; wire2<NumOfWireBC4; ++wire2){
	int hitBC3 = gUnpacker.get_entries(k_device_bc3, 0, 0, wire1, 0);
	int hitBC4 = gUnpacker.get_entries(k_device_bc4, 5, 0, wire2, 0);
	if(hitBC3 == 0 || hitBC4 == 0)continue;
	hcor_bc3bc4->Fill(wire1, wire2);
      }
    }

    // SDC2 vs HDC
    TH2* hcor_sdc2hdc = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(int wire1 = 0; wire1<NumOfWireSDC2; ++wire1){
      for(int wire2 = 0; wire2<NumOfWireHDC; ++wire2){
	int hitSDC2 = gUnpacker.get_entries(k_device_sdc2, 0, 0, wire1, 0);
	int hitHDC  = gUnpacker.get_entries(k_device_hdc,  3, 0, wire2, 0);
	if(hitSDC2 == 0 || hitHDC == 0)continue;
	hcor_sdc2hdc->Fill(wire1, wire2);
      }
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  //------------------------------------------------------------------
  // BTOF
  //------------------------------------------------------------------
  {
    // Unpacker
    static const int k_d_bh1  = gUnpacker.get_device_id("BH1");
    static const int k_d_bh2  = gUnpacker.get_device_id("BH2");
    static const int k_u      = 0; // up
    static const int k_d      = 1; // down
    static const int k_tdc    = gUnpacker.get_data_id("BH1", "tdc");

    // HodoParam
    static const int cid_bh1  = 3;
    static const int cid_bh2  = 4;
    static const int plid     = 0;

    // Sequential ID
    static const int btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);

    // BH2
    double t0  = -999;
    double ofs = 0;
    for(int seg = 0; seg<NumOfSegBH2; ++seg){
      int nhit = gUnpacker.get_entries(k_d_bh2, 0, seg, k_u, k_tdc);
      if(nhit != 0){
	int tdc = gUnpacker.get(k_d_bh2, 0, seg, k_u, k_tdc);
	if(tdc != 0){
	  HodoParamMan& hodoMan = HodoParamMan::GetInstance();
	  double bh2t=-999;
	  hodoMan.GetTime(cid_bh2, plid, seg, k_u, tdc, bh2t);
	  if(fabs(t0) > fabs(bh2t)){
	    hodoMan.GetTime(cid_bh2, plid, seg, 2, 0, ofs);
	    t0 = bh2t;
	  }
	}//if(tdc)
      }// if(nhit)
    }// for(seg)

    // BH1
    for(int seg = 0; seg<NumOfSegBH1; ++seg){
      int nhitu = gUnpacker.get_entries(k_d_bh1, 0, seg, k_u, k_tdc);
      int nhitd = gUnpacker.get_entries(k_d_bh1, 0, seg, k_d, k_tdc);
      if(nhitu != 0 &&  nhitd != 0){
	int tdcu = gUnpacker.get(k_d_bh1, 0, seg, k_u, k_tdc);
	int tdcd = gUnpacker.get(k_d_bh1, 0, seg, k_d, k_tdc);
	if(tdcu != 0 && tdcd != 0){
	  HodoParamMan& hodoMan = HodoParamMan::GetInstance();
	  double bh1tu, bh1td;
	  hodoMan.GetTime(cid_bh1, plid, seg, k_u, tdcu, bh1tu);
	  hodoMan.GetTime(cid_bh1, plid, seg, k_d, tdcd, bh1td);
	  double mt = (bh1tu+bh1td)/2.;
	  double btof = mt-(t0+ofs);
	  hptr_array[btof_id]->Fill(btof);
	}// if(tdc)
      }// if(nhit)
    }// for(seg)
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  //------------------------------------------------------------------
  // Hyperball-J
  //------------------------------------------------------------------

  // Ge --------------------------------------------------------------
  {
    // data typep
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

    static const int ge_multi_crm_id = gHist.getSequentialID(kGe, 0, kMulti);
    static const int ge_multi_tfa_id = gHist.getSequentialID(kGe, 0, kMulti) + NumOfSegGe;

    // sum hist id
    static const
      int ge_adcsum_id = gHist.getSequentialID(kGe, 0, kADC, NumOfSegGe +1);
    static const
      int ge_adcsum_calib_id = gHist.getSequentialID(kGe, 0, kADC, NumOfSegGe +2);

    // hitpat hist id
    static const
      int ge_hitpat_id = gHist.getSequentialID(kGe, 0, kHitPat);

    // 2d hist id
    static const
      int ge_adc2d_id = gHist.getSequentialID(kGe, 0, kADC2D);
    static const
      int ge_crm2d_id = gHist.getSequentialID(kGe, 0, kCRM2D);
    static const
      int ge_tfa2d_id = gHist.getSequentialID(kGe, 0, kTFA2D);
    static const
      int ge_pur2d_id = gHist.getSequentialID(kGe, 0, kPUR2D);
    static const
      int ge_rst2d_id = gHist.getSequentialID(kGe, 0, kRST2D);

    for(int seg = 0; seg<NumOfSegGe; ++seg){
      // ADC
      int nhit_adc = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      if(nhit_adc != 0){
	int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[ge_adc_id + seg]->Fill(adc);
	hptr_array[ge_adc2d_id]->Fill(seg, adc);
	hptr_array[ge_adcsum_id]->Fill(adc);

	if(115 < adc && adc < 7500 && seg != 13){
	  hptr_array[ge_hitpat_id]->Fill(seg);
	}

	GeAdcCalibMan& gGeAMan = GeAdcCalibMan::GetInstance();
	double energy;
	gGeAMan.CalibAdc(seg, adc, energy);
	if(energy > 100) hptr_array[ge_adcsum_calib_id]->Fill(energy);
      }

      // CRM
      int nhit_crm = gUnpacker.get_entries(k_device, 0, seg, 0, k_crm);
      hptr_array[ge_multi_crm_id + seg]->Fill(nhit_crm);
      if(nhit_crm != 0){
	for(int m = 0; m<nhit_crm; ++m){
	  int crm = gUnpacker.get(k_device, 0, seg, 0, k_crm, m);
	  hptr_array[ge_crm_id + seg]->Fill(crm);
	  hptr_array[ge_crm2d_id]->Fill(seg, crm);
	}
      }

      // TFA
      int nhit_tfa = gUnpacker.get_entries(k_device, 0, seg, 0, k_tfa);
      hptr_array[ge_multi_tfa_id + seg]->Fill(nhit_tfa);
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
