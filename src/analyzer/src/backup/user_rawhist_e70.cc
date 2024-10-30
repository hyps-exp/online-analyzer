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
#include <TMath.h>
#include <TStyle.h>

#include <DAQNode.hh>
#include <filesystem_util.hh>
#include <Unpacker.hh>
#include <UnpackerManager.hh>

#include "Controller.hh"
#include "user_analyzer.hh"

#include "ConfMan.hh"
#include "DCDriftParamMan.hh"
#include "DCGeomMan.hh"
#include "DCTdcCalibMan.hh"
#include "DetectorID.hh"
#include "GuiPs.hh"
#include "HistMaker.hh"
#include "HodoParamMan.hh"
#include "HodoPHCMan.hh"
#include "MacroBuilder.hh"
#include "MatrixParamMan.hh"
#include "MsTParamMan.hh"
#include "ProcInfo.hh"
#include "PsMaker.hh"
#include "SsdAnalyzer.hh"
#include "TpcPadHelper.hh"
#include "UserParamMan.hh"

#define DEBUG      0
#define FLAG_DAQ   1
#define TIME_STAMP 0

namespace
{
using hddaq::unpacker::GUnpacker;
using hddaq::unpacker::DAQNode;
const auto& gUnpacker = GUnpacker::get_instance();
auto& gHist     = HistMaker::getInstance();
const auto& gMatrix   = MatrixParamMan::GetInstance();
auto& gTpcPad   = TpcPadHelper::GetInstance();
const auto& gUser     = UserParamMan::GetInstance();
std::vector<TH1*> hptr_array;
Bool_t flag_event_cut = false;
Int_t event_cut_factor = 1; // for fast semi-online analysis
}

namespace analyzer
{

//____________________________________________________________________________
Int_t
process_begin(const std::vector<std::string>& argv)
{
  auto& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  gConfMan.InitializeParameter<HodoParamMan>("HDPRM");
  gConfMan.InitializeParameter<HodoPHCMan>("HDPHC");
  gConfMan.InitializeParameter<DCGeomMan>("DCGEO");
  gConfMan.InitializeParameter<DCTdcCalibMan>("DCTDC");
  gConfMan.InitializeParameter<DCDriftParamMan>("DCDRFT");
  gConfMan.InitializeParameter<MatrixParamMan>("MATRIX2D1",
					       "MATRIX2D2",
					       "MATRIX3D");
  gConfMan.InitializeParameter<TpcPadHelper>("TPCPAD");
  gConfMan.InitializeParameter<UserParamMan>("USER");
  if (!gConfMan.IsGood()) return -1;
  // unpacker and all the parameter managers are initialized at this stage

  if (argv.size()==4) {
    Int_t factor = std::strtod(argv[3].c_str(), NULL);
    if (factor!=0) event_cut_factor = std::abs(factor);
    flag_event_cut = true;
    std::cout << "#D Event cut flag on : factor="
	      << event_cut_factor << std::endl;
  }

  // Make tabs
  hddaq::gui::Controller& gCon = hddaq::gui::Controller::getInstance();
  TGFileBrowser *tab_hist  = gCon.makeFileBrowser("Hist");
  TGFileBrowser *tab_macro = gCon.makeFileBrowser("Macro");
  TGFileBrowser *tab_misc   = gCon.makeFileBrowser("Misc");

  // Add macros to the Macro tab
  //tab_macro->Add(hoge());
  tab_macro->Add(macro::Get("clear_all_canvas"));
  tab_macro->Add(macro::Get("clear_canvas"));
  tab_macro->Add(macro::Get("split22"));
  tab_macro->Add(macro::Get("split32"));
  tab_macro->Add(macro::Get("split33"));
  tab_macro->Add(macro::Get("dispBH1"));
  tab_macro->Add(macro::Get("dispBFT"));
  tab_macro->Add(macro::Get("dispBC3"));
  tab_macro->Add(macro::Get("dispBC4"));
  tab_macro->Add(macro::Get("dispBAC"));
  tab_macro->Add(macro::Get("dispBH2"));
  tab_macro->Add(macro::Get("dispSDC1"));
  tab_macro->Add(macro::Get("dispSDC2"));
  tab_macro->Add(macro::Get("dispSDC3"));
  tab_macro->Add(macro::Get("dispSDC4"));
  tab_macro->Add(macro::Get("dispSDC5"));
  tab_macro->Add(macro::Get("dispSDCout_hitpat"));
  tab_macro->Add(macro::Get("dispTOF"));
  tab_macro->Add(macro::Get("dispAC1"));
  //  tab_macro->Add(macro::Get("dispLAC"));
  tab_macro->Add(macro::Get("dispWC"));
  tab_macro->Add(macro::Get("dispMatrix"));
  tab_macro->Add(macro::Get("dispTriggerFlag"));
  tab_macro->Add(macro::Get("dispBeamHitPat"));
  tab_macro->Add(macro::Get("dispHitPat"));
  tab_macro->Add(macro::Get("dispCorrelation"));
  tab_macro->Add(macro::Get("effBcOut"));
  tab_macro->Add(macro::Get("effSdcInOut"));
  tab_macro->Add(macro::Get("effAC1WC"));
  tab_macro->Add(macro::Get("effALL"));
  tab_macro->Add(macro::Get("dispBH2Fit"));
  tab_macro->Add(macro::Get("dispDAQ"));
  //  tab_macro->Add(macro::Get("dispAcEfficiency"));


  // Add histograms to the Hist tab
  tab_hist->Add(gHist.createBH1());
  tab_hist->Add(gHist.createBFT());
  tab_hist->Add(gHist.createBC3());
  tab_hist->Add(gHist.createBC4());
  tab_hist->Add(gHist.createBAC());
  tab_hist->Add(gHist.createBH2());
  tab_hist->Add(gHist.createSDC1());
  tab_hist->Add(gHist.createSDC2());
  tab_hist->Add(gHist.createSDC3());
  tab_hist->Add(gHist.createSDC4());
  tab_hist->Add(gHist.createSDC5());
  tab_hist->Add(gHist.createTOF());
  tab_hist->Add(gHist.createAC1());
  //  tab_hist->Add(gHist.createLAC());
  tab_hist->Add(gHist.createWC());
  // tab_hist->Add(gHist.createTPC());
  tab_hist->Add(gHist.createCorrelation());
  tab_hist->Add(gHist.createTriggerFlag());
  // tab_hist->Add(gHist.createMsT());
#if FLAG_DAQ
  tab_hist->Add(gHist.createDAQ());
#endif
#if TIME_STAMP
  tab_hist->Add(gHist.createTimeStamp(false));
#endif
  tab_hist->Add(gHist.createDCEff());

  //misc tab
  tab_misc->Add(gHist.createBTOF());
  // tab_misc->Add(macro::Get("dispTPC"));
  //___ Matrix ___
  // gMatrix.Print2D1();
  // gMatrix.Print2D2();
  // gMatrix.Print3D();
  tab_misc->Add(gHist.createMatrix());
  tab_misc->Add(gHist.createE72E90());
  tab_misc->Add(gHist.createSAC3());
  tab_misc->Add(gHist.createSFV());
  tab_misc->Add(gHist.createTF_TF());
  tab_misc->Add(gHist.createTF_GN1());
  tab_misc->Add(gHist.createTF_GN2());
  tab_misc->Add(macro::Get("dispE72E90"));
  tab_misc->Add(macro::Get("dispE72E90Eff"));
  tab_misc->Add(macro::Get("dispSAC3"));
  tab_misc->Add(macro::Get("dispSFV"));
  tab_misc->Add(macro::Get("dispTF_TF"));
  tab_misc->Add(macro::Get("dispTF_GN1"));
  tab_misc->Add(macro::Get("dispTF_GN2"));

  // Set histogram pointers to the vector sequentially.
  // This vector contains both TH1 and TH2.
  // Then you need to do down cast when you use TH2.
  if (0 != gHist.setHistPtr(hptr_array)) { return -1; }

  // Users don't have to touch this section (Make Ps tab),
  // but the file path should be changed.
  // ----------------------------------------------------------
  auto& gPsMaker = PsMaker::getInstance();
  std::vector<TString> detList;
  std::vector<TString> optList;
  gHist.getListOfPsFiles(detList);
  gPsMaker.getListOfOption(optList);

  auto& gPsTab = hddaq::gui::GuiPs::getInstance();
  gPsTab.setFilename(Form("%s/PSFile/pro/default.ps", std::getenv("HOME")));
  gPsTab.initialize(optList, detList);
  // ----------------------------------------------------------

  gStyle->SetOptStat(1110);
  gStyle->SetTitleW(.400);
  gStyle->SetTitleH(.100);
  gStyle->SetStatW(.320);
  gStyle->SetStatH(.250);

  return 0;
}

//____________________________________________________________________________
Int_t
process_end()
{
  hptr_array.clear();
  return 0;
}

//____________________________________________________________________________
Int_t
process_event()
{
#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  const Int_t event_number = gUnpacker.get_event_number();
  if (flag_event_cut && event_number%event_cut_factor!=0)
    return 0;

  // TriggerFlag ---------------------------------------------------
  std::bitset<NumOfSegTFlag> trigger_flag;
  {
    static const Int_t k_device = gUnpacker.get_device_id("TFlag");
    static const Int_t k_tdc    = gUnpacker.get_data_id("TFlag", "tdc");
    static const Int_t tdc_id   = gHist.getSequentialID(kTriggerFlag, 0, kTDC);
    static const Int_t hit_id   = gHist.getSequentialID(kTriggerFlag, 0, kHitPat);
    for(Int_t seg=0; seg<NumOfSegTFlag; ++seg) {
      for(Int_t m=0, n=gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
	  m<n; ++m) {
	auto tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
	if (tdc>0) {
	  trigger_flag.set(seg);
	  hptr_array[tdc_id+seg]->Fill(tdc);
	}
      }
      if (trigger_flag[seg]) hptr_array[hit_id]->Fill(seg);
    }

    Bool_t l1_flag =
      trigger_flag[trigger::kL1SpillOn] ||
      trigger_flag[trigger::kL1SpillOff] ||
      trigger_flag[trigger::kSpillOnEnd] ||
      trigger_flag[trigger::kSpillOffEnd];
    if(!l1_flag)
      hddaq::cerr << "#W Trigger flag is missing : "
		  << trigger_flag << std::endl;

#if 0
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

#if TIME_STAMP
  // TimeStamp --------------------------------------------------------
  {
    static const auto hist_id = gHist.getSequentialID(kTimeStamp, 0, kTDC);
    Int_t i = 0;
    for(auto&& c : gUnpacker.get_root()->get_child_list()) {
      if (!c.second)
	continue;
      auto t = gUnpacker.get_node_header(c.second->get_id(),
					 DAQNode::k_unix_time);
      hptr_array[hist_id+i]->Fill(t);
      ++i;
    }
  }
#endif
#if FLAG_DAQ
  { ///// DAQ
    //___ node id
    static const Int_t k_eb = gUnpacker.get_fe_id("k18eb");
    std::vector<Int_t> vme_fe_id;
    std::vector<Int_t> hul_fe_id;
    std::vector<Int_t> ea0c_fe_id;
    std::vector<Int_t> vea0c_fe_id;
    for(auto&& c : gUnpacker.get_root()->get_child_list()) {
      if (!c.second) continue;
      TString n = c.second->get_name();
      auto id = c.second->get_id();
      if (n.Contains("vme"))
	vme_fe_id.push_back(id);
      if (n.Contains("hul"))
	hul_fe_id.push_back(id);
      if (n.Contains("easiroc"))
	ea0c_fe_id.push_back(id);
      if (n.Contains("aft"))
	vea0c_fe_id.push_back(id);
    }

    //___ sequential id
    static const Int_t eb_hid = gHist.getSequentialID(kDAQ, kEB, kHitPat);
    static const Int_t vme_hid = gHist.getSequentialID(kDAQ, kVME, kHitPat2D);
    static const Int_t hul_hid = gHist.getSequentialID(kDAQ, kHUL, kHitPat2D);
    static const Int_t ea0c_hid = gHist.getSequentialID(kDAQ, kEASIROC, kHitPat2D);
    static const Int_t vea0c_hid = gHist.getSequentialID(kDAQ, kVMEEASIROC, kHitPat2D);
    static const Int_t hulof_hid = gHist.getSequentialID(kDAQ, kHULOverflow, kHitPat2D);
    Int_t multihit_hid = gHist.getSequentialID(kDAQ, 0, kMultiHitTdc);

    { //___ EB
      auto data_size = gUnpacker.get_node_header(k_eb, DAQNode::k_data_size);
      hptr_array[eb_hid]->Fill(data_size);
    }

    { //___ VME
      for(Int_t i=0, n=vme_fe_id.size(); i<n; ++i) {
	auto data_size = gUnpacker.get_node_header(vme_fe_id[i], DAQNode::k_data_size);
	hptr_array[vme_hid]->Fill(i, data_size);
      }
    }

    { // EASIROC
      for(Int_t i=0, n=ea0c_fe_id.size(); i<n; ++i) {
	auto data_size = gUnpacker.get_node_header(ea0c_fe_id[i], DAQNode::k_data_size);
	hptr_array[ea0c_hid]->Fill(i, data_size);
      }
    }

    { //___ HUL node
      for(Int_t i=0, n=hul_fe_id.size(); i<n; ++i) {
	auto data_size = gUnpacker.get_node_header(hul_fe_id[i], DAQNode::k_data_size);
	hptr_array[hul_hid]->Fill(i, data_size);
      }
    }

    { //___ VMEEASIROC node
      for(Int_t i=0, n=vea0c_fe_id.size(); i<n; ++i) {
	auto data_size = gUnpacker.get_node_header(vea0c_fe_id[i], DAQNode::k_data_size);
	hptr_array[vea0c_hid]->Fill(i, data_size);
      }
    }

    { //___ MultiHitTdc
      { // BC3
	static const Int_t k_device   = gUnpacker.get_device_id("BC3");
	static const Int_t k_leading  = gUnpacker.get_data_id("BC3", "leading");
	for(Int_t l=0; l<NumOfLayersBC3; ++l) {
	  for(Int_t w=0; w<NumOfWireBC3; ++w) {
	    Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	    hptr_array[multihit_hid]->Fill(w, nhit_l);
	  }
	  ++multihit_hid;
	}
      }
      { // BC4
	static const Int_t k_device   = gUnpacker.get_device_id("BC4");
	static const Int_t k_leading  = gUnpacker.get_data_id("BC4", "leading");
	for(Int_t l=0; l<NumOfLayersBC4; ++l) {
	  for(Int_t w=0; w<NumOfWireBC4; ++w) {
	    Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	    hptr_array[multihit_hid]->Fill(w, nhit_l);
	  }
	  ++multihit_hid;
	}
      }
      { // SDC1
	static const Int_t k_device   = gUnpacker.get_device_id("SDC1");
	static const Int_t k_leading  = gUnpacker.get_data_id("SDC1", "leading");
	for(Int_t l=0; l<NumOfLayersSDC1; ++l) {
	  for(Int_t w=0; w<NumOfWireSDC1; ++w) {
	    Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	    hptr_array[multihit_hid]->Fill(w, nhit_l);
	  }
	  ++multihit_hid;
	}
      }
      { // SDC2
	static const Int_t k_device   = gUnpacker.get_device_id("SDC2");
	static const Int_t k_leading  = gUnpacker.get_data_id("SDC2", "leading");
	for(Int_t l=0; l<NumOfLayersSDC2; ++l) {
	  for(Int_t w=0; w<NumOfWireSDC2; ++w) {
	    Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	    hptr_array[multihit_hid]->Fill(w, nhit_l);
	  }
	  ++multihit_hid;
	}
      }
      { // BH2MTLR
	static const auto device_id = gUnpacker.get_device_id("BH2MTLR");
	static const auto tdc_id = gUnpacker.get_data_id("BH2MTLR", "tdc");
	for(Int_t seg=0; seg<NumOfSegBH2; ++seg) {
	  Int_t nhit_l = gUnpacker.get_entries(device_id, 0, seg, 0, tdc_id);
	  hptr_array[multihit_hid]->Fill(seg, nhit_l);
	}
	++multihit_hid;
      }

      // { // HUL node overflow
      // 	for(Int_t i=0, n=hul_fe_id.size(); i<n; ++i) {
      // 	  auto overflow = gUnpacker.get_node_header(hul_fe_id[i], DAQNode::);
      // 	  hptr_array[hul_hid]->Fill(i, overflow);
      // 	}
      // }

    }
  }

#endif

  if (trigger_flag[trigger::kSpillOnEnd] || trigger_flag[trigger::kSpillOffEnd])
    return 0;


  std::vector<Int_t> hitseg_bh1;
  { ///// BH1
    static const auto device_id = gUnpacker.get_device_id("BH1");
    static const auto adc_id = gUnpacker.get_data_id("BH1", "adc");
    static const auto tdc_id = gUnpacker.get_data_id("BH1", "tdc");
    static const auto tdc_min = gUser.GetParameter("TdcBH1", 0);
    static const auto tdc_max = gUser.GetParameter("TdcBH1", 1);
    static const auto adc_hid = gHist.getSequentialID(kBH1, 0, kADC);
    static const auto tdc_hid = gHist.getSequentialID(kBH1, 0, kTDC);
    static const auto awt_hid = gHist.getSequentialID(kBH1, 0, kADCwTDC);
    static const auto hit_hid = gHist.getSequentialID(kBH1, 0, kHitPat);
    static const auto mul_hid = gHist.getSequentialID(kBH1, 0, kMulti);
    std::vector<std::vector<Int_t>> hit_flag(NumOfSegBH1);
    UInt_t multiplicity = 0;
    for(Int_t seg=0; seg<NumOfSegBH1; ++seg) {
      hit_flag[seg].resize(kUorD);
      for(Int_t ud=0; ud<kUorD; ++ud) {
	hit_flag[seg][ud] = 0;
	///// ADC
	UInt_t adc = 0;
	auto nhit = gUnpacker.get_entries(device_id, 0, seg, ud, adc_id);
	if (nhit != 0) {
	  adc = gUnpacker.get(device_id, 0, seg, ud, adc_id);
	  hptr_array[adc_hid + seg + ud*NumOfSegBH1]->Fill(adc);
	}
	///// TDC
	for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, seg, ud, tdc_id);
	    m<n; ++m) {
	  auto tdc = gUnpacker.get(device_id, 0, seg, ud, tdc_id, m);
	  if (tdc != 0) {
	    hptr_array[tdc_hid + seg + ud*NumOfSegBH1]->Fill(tdc);
	    // ADC wTDC
	    if (tdc > tdc_min && tdc < tdc_max && adc > 0) {
	      hit_flag[seg][ud] = 1;
	    }
	  }
	}
	if (hit_flag[seg][ud] == 1) {
	  hptr_array[awt_hid + seg + ud*NumOfSegBH1]->Fill(adc);
	}
      }
      if (hit_flag[seg][kU] == 1 && hit_flag[seg][kD] == 1) {
	++multiplicity;
	hptr_array[hit_hid]->Fill(seg);
	hitseg_bh1.push_back(seg);
      }
    }
    hptr_array[mul_hid]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }// BH1

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  std::vector<Int_t> hitseg_bftu;
  { ///// BFT
    static const auto device_id = gUnpacker.get_device_id("BFT");
    static const auto leading_id = gUnpacker.get_data_id("BFT", "leading");
    static const auto trailing_id = gUnpacker.get_data_id("BFT", "trailing");
    static const auto tdc_min = gUser.GetParameter("TdcBFT", 0);
    static const auto tdc_max = gUser.GetParameter("TdcBFT", 1);
    static const auto tdc_hid = gHist.getSequentialID(kBFT, 0, kTDC);
    static const auto tot_hid = gHist.getSequentialID(kBFT, 0, kADC);
    static const auto hit_hid = gHist.getSequentialID(kBFT, 0, kHitPat);
    static const auto mul_hid = gHist.getSequentialID(kBFT, 0, kMulti);
    static const auto tdc_2d_hid = gHist.getSequentialID(kBFT, 0, kTDC2D);
    static const auto tot_2d_hid = gHist.getSequentialID(kBFT, 0, kADC2D);
    UInt_t multiplicity = 0; // includes both u and d planes.
    for(Int_t i=0; i<NumOfSegBFT; ++i) {
      for(Int_t ud=0; ud<kUorD; ++ud) {
	UInt_t tdc_prev = 0;
	Bool_t is_in_range = false;
	for(Int_t m=0, n=gUnpacker.get_entries(device_id, ud, i, 0, leading_id);
	    m<n; ++m) {
	  auto tdc = gUnpacker.get(device_id, ud, i, 0, leading_id, m);
	  auto tdc_t = gUnpacker.get(device_id, ud, i, 0, trailing_id, m);
	  auto tot = tdc - tdc_t;
	  if (tdc_prev == tdc || tdc <= 0 || tot <= 0)
	    continue;
	  tdc_prev = tdc;
	  hptr_array[tdc_hid + ud]->Fill(tdc);
	  hptr_array[tot_hid + ud]->Fill(tot);
	  if (tdc_min < tdc && tdc < tdc_max) {
	    is_in_range = true;
	  }
	  hptr_array[tdc_2d_hid + ud]->Fill(i, tdc);
	  hptr_array[tot_2d_hid + ud]->Fill(i, tot);
	}
	if (is_in_range) {
	  ++multiplicity;
	  hptr_array[hit_hid + ud]->Fill(i);
	  hitseg_bftu.push_back(i);
	}
      }
    }
    hptr_array[mul_hid]->Fill(multiplicity);

#if 0
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BC3 -------------------------------------------------------------
  //for HULMHTDC
  std::vector< std::vector<Int_t> > BC3HitCont(6);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("BC3");
    static const Int_t k_leading  = gUnpacker.get_data_id("BC3", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("BC3", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcBC3", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcBC3", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("MinTotBcOut", 0);
    // static const Int_t tot_max = gUser.GetParameter("MinTotBcOut", 1);

    // sequential id
    static const Int_t bc3t_id    = gHist.getSequentialID(kBC3, 0, kTDC);
    static const Int_t bc3tot_id  = gHist.getSequentialID(kBC3, 0, kADC);
    static const Int_t bc3t1st_id = gHist.getSequentialID(kBC3, 0, kTDC2D);
    static const Int_t bc3hit_id  = gHist.getSequentialID(kBC3, 0, kHitPat);
    static const Int_t bc3mul_id  = gHist.getSequentialID(kBC3, 0, kMulti);
    static const Int_t bc3mulwt_id
      = gHist.getSequentialID(kBC3, 0, kMulti, 1+NumOfLayersBC3);

    static const Int_t bc3t_wide_id    = gHist.getSequentialID(kBC3, 0, kTDC,    11);
    static const Int_t bc3t_ctot_id    = gHist.getSequentialID(kBC3, 0, kTDC,    1+kTOTcutOffset);
    static const Int_t bc3tot_ctot_id  = gHist.getSequentialID(kBC3, 0, kADC,    1+kTOTcutOffset);
    static const Int_t bc3tot2D_id     = gHist.getSequentialID(kBC3, 0, kADC2D,  1);
    static const Int_t bc3t1st_ctot_id = gHist.getSequentialID(kBC3, 0, kTDC2D,  1+kTOTcutOffset);
    static const Int_t bc3t2D_id       = gHist.getSequentialID(kBC3, 0, kTDC2D,  21+kTOTcutOffset);
    static const Int_t bc3t2D_ctot_id  = gHist.getSequentialID(kBC3, 0, kTDC2D,  31+kTOTcutOffset);
    static const Int_t bc3hit_ctot_id  = gHist.getSequentialID(kBC3, 0, kHitPat, 1+kTOTcutOffset);
    static const Int_t bc3mul_ctot_id  = gHist.getSequentialID(kBC3, 0, kMulti,  1+kTOTcutOffset);
    static const Int_t bc3mulwt_ctot_id
      = gHist.getSequentialID(kBC3, 0, kMulti, 1+NumOfLayersBC3 + kTOTcutOffset);
    static const Int_t bc3self_corr_id  = gHist.getSequentialID(kBC3, kSelfCorr, 0, 1);


    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersBC3; ++l) {
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      for(Int_t w=0; w<NumOfWireBC3; ++w) {
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if (nhit_l == 0) continue;

	Int_t hit_l_max = 0;
	Int_t hit_t_max = 0;

	if (nhit_l != 0) {
	  hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
	}
	if (nhit_t != 0) {
	  hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
	}

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[bc3hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for(Int_t m = 0; m<nhit_l; ++m) {
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[bc3t_id + l]->Fill(tdc);
	  hptr_array[bc3t_wide_id + l]->Fill(tdc); //TDCwide
	  if (tdc1st<tdc) tdc1st = tdc;

	  // tdc 2D
	  hptr_array[bc3t2D_id + l]->Fill(w,tdc);

	  // Drift time check
	  if (tdc_min < tdc && tdc < tdc_max) {
	    flag_hit_wt = true;
	  }
	}

	if (tdc1st!=0) hptr_array[bc3t1st_id +l]->Fill(tdc1st);
	if (flag_hit_wt) {
	  ++multiplicity_wt;
	  hptr_array[bc3hit_id + l]->Fill(w);
	}

	tdc1st = 0;
	if (nhit_l == nhit_t && hit_l_max > hit_t_max) {
	  ++multiplicity_ctot;
	  for(Int_t m = 0; m<nhit_l; ++m) {
	    tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	    tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
	    tot = tdc - tdc_t;
	    hptr_array[bc3tot_id+l]->Fill(tot);
     	    hptr_array[bc3tot2D_id+l]->Fill(w,tot); //2D
	    if (tot < tot_min) continue;
	    hptr_array[bc3t_ctot_id + l]->Fill(tdc);
	    hptr_array[bc3t2D_ctot_id + l]->Fill(w,tdc); //2D
	    hptr_array[bc3tot_ctot_id+l]->Fill(tot);
	    if (tdc1st<tdc) tdc1st = tdc;
	    if (tdc_min < tdc && tdc < tdc_max) {
	      flag_hit_wt_ctot = true;
	    }
	  }
	}

	if (tdc1st!=0) hptr_array[bc3t1st_ctot_id +l]->Fill(tdc1st);
	if (flag_hit_wt_ctot) {
	  ++multiplicity_wt_ctot;
	  hptr_array[bc3hit_ctot_id + l]->Fill(w);
	  BC3HitCont[l].push_back(w);
	}
      }

      hptr_array[bc3mul_id + l]->Fill(multiplicity);
      hptr_array[bc3mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[bc3mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[bc3mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }


    for(Int_t s=0; s<NumOfDimBC3 ;s++) {
      Int_t corr=2*s;
      for(UInt_t i=0; i<BC3HitCont[corr].size() ;i++) {
	for(UInt_t j=0; j<BC3HitCont[corr+1].size() ;j++) {
	  hptr_array[bc3self_corr_id + s]->Fill(BC3HitCont[corr][i],BC3HitCont[corr+1][j]);
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

  // BC4 -------------------------------------------------------------
  //for HULMHTDC
  std::vector< std::vector<Int_t> > BC4HitCont(6);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("BC4");
    static const Int_t k_leading  = gUnpacker.get_data_id("BC4", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("BC4", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcBC4", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcBC4", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("MinTotBcOut", 0);
    // static const Int_t tot_max = gUser.GetParameter("MinTotBcOut", 1);

    // sequential id
    static const Int_t bc4t_id    = gHist.getSequentialID(kBC4, 0, kTDC);
    static const Int_t bc4tot_id  = gHist.getSequentialID(kBC4, 0, kADC);
    static const Int_t bc4t1st_id = gHist.getSequentialID(kBC4, 0, kTDC2D);
    static const Int_t bc4hit_id  = gHist.getSequentialID(kBC4, 0, kHitPat);
    static const Int_t bc4mul_id  = gHist.getSequentialID(kBC4, 0, kMulti);
    static const Int_t bc4mulwt_id
      = gHist.getSequentialID(kBC4, 0, kMulti, 1+NumOfLayersBC4);

    static const Int_t bc4t_wide_id    = gHist.getSequentialID(kBC4, 0, kTDC, 11);
    static const Int_t bc4t_ctot_id    = gHist.getSequentialID(kBC4, 0, kTDC,    1+kTOTcutOffset);
    static const Int_t bc4tot_ctot_id  = gHist.getSequentialID(kBC4, 0, kADC,    1+kTOTcutOffset);
    static const Int_t bc4tot2D_id     = gHist.getSequentialID(kBC4, 0, kADC2D,  1);
    static const Int_t bc4t1st_ctot_id = gHist.getSequentialID(kBC4, 0, kTDC2D,  1+kTOTcutOffset);
    static const Int_t bc4t2D_id       = gHist.getSequentialID(kBC4, 0, kTDC2D,  21+kTOTcutOffset);
    static const Int_t bc4t2D_ctot_id  = gHist.getSequentialID(kBC4, 0, kTDC2D,  31+kTOTcutOffset);
    static const Int_t bc4hit_ctot_id  = gHist.getSequentialID(kBC4, 0, kHitPat, 1+kTOTcutOffset);
    static const Int_t bc4mul_ctot_id  = gHist.getSequentialID(kBC4, 0, kMulti,  1+kTOTcutOffset);
    static const Int_t bc4mulwt_ctot_id
      = gHist.getSequentialID(kBC4, 0, kMulti, 1+NumOfLayersBC4 + kTOTcutOffset);
    static const Int_t bc4self_corr_id  = gHist.getSequentialID(kBC4, kSelfCorr, 0, 1);


    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersBC4; ++l) {
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      for(Int_t w=0; w<NumOfWireBC4; ++w) {
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if (nhit_l == 0) continue;

	Int_t hit_l_max = 0;
	Int_t hit_t_max = 0;

	if (nhit_l != 0) {
	  hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
	}
	if (nhit_t != 0) {
	  hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
	}

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[bc4hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for(Int_t m = 0; m<nhit_l; ++m) {
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[bc4t_id + l]->Fill(tdc);
	  hptr_array[bc4t_wide_id + l]->Fill(tdc); //TDCwide
	  if (tdc1st<tdc) tdc1st = tdc;

	  // tdc 2D
	  hptr_array[bc4t2D_id + l]->Fill(w,tdc);

	  // Drift time check
	  if (tdc_min < tdc && tdc < tdc_max) {
	    flag_hit_wt = true;
	  }
	}

	if (tdc1st!=0) hptr_array[bc4t1st_id +l]->Fill(tdc1st);
	if (flag_hit_wt) {
	  ++multiplicity_wt;
	  hptr_array[bc4hit_id + l]->Fill(w);
	}

	tdc1st = 0;
	if (nhit_l == nhit_t && hit_l_max > hit_t_max) {
	  ++multiplicity_ctot;
	  for(Int_t m = 0; m<nhit_l; ++m) {
	    tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	    tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
	    tot = tdc - tdc_t;
	    hptr_array[bc4tot_id+l]->Fill(tot);
	    hptr_array[bc4tot2D_id+l]->Fill(w,tot); //2D
	    if (tot < tot_min) continue;
	    hptr_array[bc4t_ctot_id + l]->Fill(tdc);
	    hptr_array[bc4t2D_ctot_id + l]->Fill(w,tdc); //2D
	    hptr_array[bc4tot_ctot_id+l]->Fill(tot);
	    if (tdc1st<tdc) tdc1st = tdc;
	    if (tdc_min < tdc && tdc < tdc_max) {
	      flag_hit_wt_ctot = true;
	    }
	  }
	}

	if (tdc1st!=0) hptr_array[bc4t1st_ctot_id +l]->Fill(tdc1st);
	if (flag_hit_wt_ctot) {
	  ++multiplicity_wt_ctot;
	  hptr_array[bc4hit_ctot_id + l]->Fill(w);
	  BC4HitCont[l].push_back(w);
	}
      }

      hptr_array[bc4mul_id + l]->Fill(multiplicity);
      hptr_array[bc4mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[bc4mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[bc4mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }


    for(Int_t s=0; s<NumOfDimBC4 ;s++) {
      Int_t corr=2*s;
      for(UInt_t i=0; i<BC4HitCont[corr].size() ;i++) {
	for(UInt_t j=0; j<BC4HitCont[corr+1].size() ;j++) {
	  hptr_array[bc4self_corr_id + s]->Fill(BC4HitCont[corr][i],BC4HitCont[corr+1][j]);
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

  std::vector<Int_t> hitseg_bh2;
  { ///// BH2
    static const auto device_id = gUnpacker.get_device_id("BH2");
    static const auto adc_id = gUnpacker.get_data_id("BH2", "adc");
    static const auto tdc_id = gUnpacker.get_data_id("BH2", "tdc");
    static const auto tdc_min = gUser.GetParameter("TdcBH2", 0);
    static const auto tdc_max = gUser.GetParameter("TdcBH2", 1);
    static const auto adc_hid = gHist.getSequentialID(kBH2, 0, kADC);
    static const auto tdc_hid = gHist.getSequentialID(kBH2, 0, kTDC);
    static const auto awt_hid = gHist.getSequentialID(kBH2, 0, kADCwTDC);
    static const auto hit_hid = gHist.getSequentialID(kBH2, 0, kHitPat);
    static const auto mul_hid = gHist.getSequentialID(kBH2, 0, kMulti);
    std::vector<std::vector<Int_t>> hit_flag(NumOfSegBH2);
    UInt_t multiplicity = 0;
    for(Int_t seg=0; seg<NumOfSegBH2; ++seg) {
      hit_flag[seg].resize(kUorD);
      for(Int_t ud=0; ud<2; ++ud) {
	auto nhit = gUnpacker.get_entries(device_id, 0, seg, ud, adc_id);
	UInt_t adc = 0;
	if (nhit != 0) {
	  adc = gUnpacker.get(device_id, 0, seg, ud, adc_id);
	  hptr_array[adc_hid + seg + ud*NumOfSegBH2]->Fill(adc);
	}
	// TDC
	for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, seg, ud, tdc_id);
	    m<n; ++m) {
	  auto tdc = gUnpacker.get(device_id, 0, seg, ud, tdc_id, m);
	  if (tdc != 0) {
	    hptr_array[tdc_hid + seg + ud*NumOfSegBH2]->Fill(tdc);
	    if (tdc_min < tdc && tdc < tdc_max && adc > 0) {
	      hit_flag[seg][ud] = 1;
	    }
	  }
	}
	if (hit_flag[seg][ud] == 1) {
	  hptr_array[awt_hid + seg + ud*NumOfSegBH2]->Fill(adc);
	}
      }
      if (hit_flag[seg][kU] == 1 && hit_flag[seg][kD] == 1) {
	++multiplicity;
	hptr_array[hit_hid]->Fill(seg);
	hitseg_bh2.push_back(seg);
      }
    }
    hptr_array[mul_hid]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

  { // BH2MTLR
    static const auto device_id = gUnpacker.get_device_id("BH2MTLR");
    static const auto tdc_id = gUnpacker.get_data_id("BH2MTLR", "tdc");
    static const auto tdc_hid = gHist.getSequentialID(kBH2, 0, kTDC, 20);
    for(Int_t seg=0; seg<NumOfSegBH2; ++seg) {
      for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, seg, 0, tdc_id);
	  m<n; ++m) {
	auto tdc = gUnpacker.get(device_id, 0, seg, 0, tdc_id, m);
	if (tdc != 0) hptr_array[tdc_hid + seg]->Fill(tdc);
      }
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // SDC1 ------------------------------------------------------------
  std::vector< std::vector<Int_t> > SDC1HitCont(6);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("SDC1");
    static const Int_t k_leading  = gUnpacker.get_data_id("SDC1", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("SDC1", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcSDC1", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcSDC1", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("MinTotSDC1", 0);
    // static const Int_t tot_max = gUser.GetParameter("MinTotSDC1", 1);

    // sequential id
    static const Int_t sdc1t_id    = gHist.getSequentialID(kSDC1, 0, kTDC);
    static const Int_t sdc1tot_id  = gHist.getSequentialID(kSDC1, 0, kADC);
    static const Int_t sdc1t1st_id = gHist.getSequentialID(kSDC1, 0, kTDC2D);
    static const Int_t sdc1hit_id  = gHist.getSequentialID(kSDC1, 0, kHitPat);
    static const Int_t sdc1mul_id  = gHist.getSequentialID(kSDC1, 0, kMulti);
    static const Int_t sdc1mulwt_id
      = gHist.getSequentialID(kSDC1, 0, kMulti, 1+NumOfLayersSDC1);

    static const Int_t sdc1t_wide_id    = gHist.getSequentialID(kSDC1, 0, kTDC,    11);
    static const Int_t sdc1t_ctot_id    = gHist.getSequentialID(kSDC1, 0, kTDC,    1+kTOTcutOffset);
    static const Int_t sdc1tot_ctot_id  = gHist.getSequentialID(kSDC1, 0, kADC,    1+kTOTcutOffset);
    static const Int_t sdc1t1st_ctot_id = gHist.getSequentialID(kSDC1, 0, kTDC2D,  1+kTOTcutOffset);
    static const Int_t sdc1t2D_id       = gHist.getSequentialID(kSDC1, 0, kTDC2D,  21+kTOTcutOffset);
    static const Int_t sdc1t2D_ctot_id  = gHist.getSequentialID(kSDC1, 0, kTDC2D,  31+kTOTcutOffset);
    static const Int_t sdc1hit_ctot_id  = gHist.getSequentialID(kSDC1, 0, kHitPat, 1+kTOTcutOffset);
    static const Int_t sdc1mul_ctot_id  = gHist.getSequentialID(kSDC1, 0, kMulti,  1+kTOTcutOffset);
    static const Int_t sdc1mulwt_ctot_id
      = gHist.getSequentialID(kSDC1, 0, kMulti, 1+NumOfLayersSDC1 + kTOTcutOffset);
    static const Int_t sdc1self_corr_id  = gHist.getSequentialID(kSDC1, kSelfCorr, 0, 1);


    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersSDC1; ++l) {
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      for(Int_t w=0; w<NumOfWireSDC1; ++w) {
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if (nhit_l == 0) continue;

	Int_t hit_l_max = 0;
	Int_t hit_t_max = 0;

	if (nhit_l != 0) {
	  hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
	}
	if (nhit_t != 0) {
	  hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
	}

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[sdc1hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for(Int_t m = 0; m<nhit_l; ++m) {
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[sdc1t_id + l]->Fill(tdc);
	  hptr_array[sdc1t_wide_id + l]->Fill(tdc); //TDCwide
	  if (tdc1st<tdc) tdc1st = tdc;

	  // tdc 2D
	  hptr_array[sdc1t2D_id + l]->Fill(w,tdc);

	  // Drift time check
	  if (tdc_min < tdc && tdc < tdc_max) {
	    flag_hit_wt = true;
	  }
	}

	if (tdc1st!=0) hptr_array[sdc1t1st_id +l]->Fill(tdc1st);
	if (flag_hit_wt) {
	  ++multiplicity_wt;
	  hptr_array[sdc1hit_id + l]->Fill(w);
	}

	tdc1st = 0;
	if (nhit_l == nhit_t && hit_l_max > hit_t_max) {
	  ++multiplicity_ctot;
	  for(Int_t m = 0; m<nhit_l; ++m) {
	    tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	    tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
	    tot = tdc - tdc_t;
	    hptr_array[sdc1tot_id+l]->Fill(tot);
	    if (tot < tot_min) continue;
	    hptr_array[sdc1t_ctot_id + l]->Fill(tdc);
	    hptr_array[sdc1t2D_ctot_id + l]->Fill(w,tdc); //2D
	    hptr_array[sdc1tot_ctot_id+l]->Fill(tot);
	    if (tdc1st<tdc) tdc1st = tdc;
	    if (tdc_min < tdc && tdc < tdc_max) {
	      flag_hit_wt_ctot = true;
	    }
	  }
	}

	if (tdc1st!=0) hptr_array[sdc1t1st_ctot_id +l]->Fill(tdc1st);
	if (flag_hit_wt_ctot) {
	  ++multiplicity_wt_ctot;
	  hptr_array[sdc1hit_ctot_id + l]->Fill(w);
	  SDC1HitCont[l].push_back(w);
	}
      }

      hptr_array[sdc1mul_id + l]->Fill(multiplicity);
      hptr_array[sdc1mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[sdc1mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[sdc1mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }


    for(Int_t s=0; s<NumOfDimSDC1 ;s++) {
      Int_t corr=2*s;
      for(UInt_t i=0; i<SDC1HitCont[corr].size() ;i++) {
	for(UInt_t j=0; j<SDC1HitCont[corr+1].size() ;j++) {
	  hptr_array[sdc1self_corr_id + s]->Fill(SDC1HitCont[corr][i],SDC1HitCont[corr+1][j]);
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

  // SDC2 ------------------------------------------------------------

  std::vector< std::vector<Int_t> > SDC2HitCont(4);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("SDC2");
    static const Int_t k_leading  = gUnpacker.get_data_id("SDC2", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("SDC2", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcSDC2", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcSDC2", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("MinTotSDC2", 0);
    // static const Int_t tot_max = gUser.GetParameter("MinTotSDC2", 1);

    // sequential id
    static const Int_t sdc2t_id    = gHist.getSequentialID(kSDC2, 0, kTDC);
    static const Int_t sdc2tot_id  = gHist.getSequentialID(kSDC2, 0, kADC);
    static const Int_t sdc2t1st_id = gHist.getSequentialID(kSDC2, 0, kTDC2D);
    static const Int_t sdc2hit_id  = gHist.getSequentialID(kSDC2, 0, kHitPat);
    static const Int_t sdc2mul_id  = gHist.getSequentialID(kSDC2, 0, kMulti);
    static const Int_t sdc2mulwt_id
      = gHist.getSequentialID(kSDC2, 0, kMulti, 1+NumOfLayersSDC2);

    static const Int_t sdc2t_wide_id    = gHist.getSequentialID(kSDC2, 0, kTDC,    11);
    static const Int_t sdc2mul2D_id     = gHist.getSequentialID(kSDC2, 0, kMulti2D);
    static const Int_t sdc2t_ctot_id    = gHist.getSequentialID(kSDC2, 0, kTDC,    1+kTOTcutOffset);
    static const Int_t sdc2tot_ctot_id  = gHist.getSequentialID(kSDC2, 0, kADC,    1+kTOTcutOffset);
    static const Int_t sdc2t1st_ctot_id = gHist.getSequentialID(kSDC2, 0, kTDC2D,  1+kTOTcutOffset);
    static const Int_t sdc2t2D_id       = gHist.getSequentialID(kSDC2, 0, kTDC2D,  21+kTOTcutOffset);
    static const Int_t sdc2t2D_ctot_id  = gHist.getSequentialID(kSDC2, 0, kTDC2D,  31+kTOTcutOffset);
    static const Int_t sdc2hit_ctot_id  = gHist.getSequentialID(kSDC2, 0, kHitPat, 1+kTOTcutOffset);
    static const Int_t sdc2mul_ctot_id  = gHist.getSequentialID(kSDC2, 0, kMulti,  1+kTOTcutOffset);
    static const Int_t sdc2mulwt_ctot_id
      = gHist.getSequentialID(kSDC2, 0, kMulti, 1+NumOfLayersSDC2 + kTOTcutOffset);
    static const Int_t sdc2self_corr_id  = gHist.getSequentialID(kSDC2, kSelfCorr, 0, 1);


    // TDC & HitPat & Multi & Multi2D
    std::vector<int>multiplicity2D(4);
    multiplicity2D = {0, 0, 0, 0};
    for(Int_t l=0; l<NumOfLayersSDC2; ++l) {
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      for(Int_t w=0; w<NumOfWireSDC2; ++w) {
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if (nhit_l == 0) continue;

	Int_t hit_l_max = 0;
	Int_t hit_t_max = 0;

	if (nhit_l != 0) {
	  hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
	}
	if (nhit_t != 0) {
	  hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
	}

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[sdc2hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for(Int_t m = 0; m<nhit_l; ++m) {
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[sdc2t_id + l]->Fill(tdc);
	  hptr_array[sdc2t_wide_id + l]->Fill(tdc); //TDCwide
	  if (tdc1st<tdc) tdc1st = tdc;

	  // tdc 2D
	  hptr_array[sdc2t2D_id + l]->Fill(w,tdc);

	  // Drift time check
	  if (tdc_min < tdc && tdc < tdc_max) {
	    flag_hit_wt = true;
	  }
	}

	if (tdc1st!=0) hptr_array[sdc2t1st_id +l]->Fill(tdc1st);
	if (flag_hit_wt) {
	  ++multiplicity_wt;
	  hptr_array[sdc2hit_id + l]->Fill(w);
	}

	tdc1st = 0;
	if (nhit_l == nhit_t && hit_l_max > hit_t_max) {
	  ++multiplicity_ctot;
	  for(Int_t m = 0; m<nhit_l; ++m) {
	    tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	    tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
	    tot = tdc - tdc_t;
	    hptr_array[sdc2tot_id+l]->Fill(tot);
	    if (tot < tot_min) continue;
	    hptr_array[sdc2t_ctot_id + l]->Fill(tdc);
	    hptr_array[sdc2t2D_ctot_id + l]->Fill(w,tdc); //2D
	    hptr_array[sdc2tot_ctot_id+l]->Fill(tot);
	    if (tdc1st<tdc) tdc1st = tdc;
	    if (tdc_min < tdc && tdc < tdc_max) {
	      flag_hit_wt_ctot = true;
	    }
	  }
	}

	if (tdc1st!=0) hptr_array[sdc2t1st_ctot_id +l]->Fill(tdc1st);
	if (flag_hit_wt_ctot) {
	  ++multiplicity_wt_ctot;
	  hptr_array[sdc2hit_ctot_id + l]->Fill(w);
	  SDC2HitCont[l].push_back(w);
	}
      }

      multiplicity2D[l] = multiplicity;

      hptr_array[sdc2mul_id + l]->Fill(multiplicity);
      hptr_array[sdc2mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[sdc2mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[sdc2mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }

    int cor_id = 0;
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i) {
      for(Int_t j = 0; j<NumOfLayersSDC2; ++j) {
	if (i >= j) continue;
	hptr_array[sdc2mul2D_id + cor_id]->Fill(multiplicity2D[i], multiplicity2D[j]);
	cor_id++;
      }
    }




    for(Int_t s=0; s<NumOfDimSDC2 ;s++) {
      Int_t corr=2*s;
      for(UInt_t i=0; i<SDC2HitCont[corr].size() ;i++) {
	for(UInt_t j=0; j<SDC2HitCont[corr+1].size() ;j++) {
	  hptr_array[sdc2self_corr_id + s]->Fill(SDC2HitCont[corr][i],SDC2HitCont[corr+1][j]);
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



  //------------------------------------------------------------------
  // SDC3
  //------------------------------------------------------------------
  std::vector< std::vector<Int_t> > SDC3HitCont(4);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("SDC3");
    static const Int_t k_leading  = gUnpacker.get_data_id("SDC3", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("SDC3", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcSDC3", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcSDC3", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("MinTotSDC3", 0);
    // static const Int_t tot_max = gUser.GetParameter("MinTotSDC3", 1);

    // sequential id
    static const Int_t sdc3t_id    = gHist.getSequentialID(kSDC3, 0, kTDC);
    static const Int_t sdc3tot_id  = gHist.getSequentialID(kSDC3, 0, kADC);
    static const Int_t sdc3t1st_id = gHist.getSequentialID(kSDC3, 0, kTDC2D);
    static const Int_t sdc3hit_id  = gHist.getSequentialID(kSDC3, 0, kHitPat);
    static const Int_t sdc3mul_id  = gHist.getSequentialID(kSDC3, 0, kMulti);
    static const Int_t sdc3mulwt_id
      = gHist.getSequentialID(kSDC3, 0, kMulti, 1+NumOfLayersSDC3);

    static const Int_t sdc3t_ctot_id    = gHist.getSequentialID(kSDC3, 0, kTDC,    11);
    static const Int_t sdc3tot_ctot_id  = gHist.getSequentialID(kSDC3, 0, kADC,    11);
    static const Int_t sdc3t1st_ctot_id = gHist.getSequentialID(kSDC3, 0, kTDC2D,  11);
    static const Int_t sdc3t2D_id       = gHist.getSequentialID(kSDC3, 0, kTDC2D,  21);
    static const Int_t sdc3t2D_ctot_id  = gHist.getSequentialID(kSDC3, 0, kTDC2D,  31);
    static const Int_t sdc3hit_ctot_id  = gHist.getSequentialID(kSDC3, 0, kHitPat, 11);
    static const Int_t sdc3mul_ctot_id  = gHist.getSequentialID(kSDC3, 0, kMulti,  11);
    static const Int_t sdc3mulwt_ctot_id
      = gHist.getSequentialID(kSDC3, 0, kMulti, 1+NumOfLayersSDC3 + 10);
    static const Int_t sdc3self_corr_id  = gHist.getSequentialID(kSDC3, kSelfCorr, 0, 1);


    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersSDC3; ++l) {
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      for(Int_t w=0; w<NumOfWireSDC3; ++w) {
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if (nhit_l == 0) continue;

	Int_t hit_l_max = 0;
	Int_t hit_t_max = 0;

	if (nhit_l != 0) {
	  hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
	}
	if (nhit_t != 0) {
	  hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
	}

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[sdc3hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for(Int_t m = 0; m<nhit_l; ++m) {
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[sdc3t_id + l]->Fill(tdc);
	  if (tdc1st<tdc) tdc1st = tdc;

	  // tdc 2D
	  hptr_array[sdc3t2D_id + l]->Fill(w,tdc);

	  // Drift time check
	  if (tdc_min < tdc && tdc < tdc_max) {
	    flag_hit_wt = true;
	  }
	}

	if (tdc1st!=0) hptr_array[sdc3t1st_id +l]->Fill(tdc1st);
	if (flag_hit_wt) {
	  ++multiplicity_wt;
	  hptr_array[sdc3hit_id + l]->Fill(w);
	}

	tdc1st = 0;
	if (nhit_l == nhit_t && hit_l_max > hit_t_max) {
	  ++multiplicity_ctot;
	  for(Int_t m = 0; m<nhit_l; ++m) {
	    tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	    tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
	    tot = tdc - tdc_t;
	    hptr_array[sdc3tot_id+l]->Fill(tot);
	    if (tot < tot_min) continue;
	    hptr_array[sdc3t_ctot_id + l]->Fill(tdc);
	    hptr_array[sdc3t2D_ctot_id + l]->Fill(w,tdc); //2D
	    hptr_array[sdc3tot_ctot_id+l]->Fill(tot);
	    if (tdc1st<tdc) tdc1st = tdc;
	    if (tdc_min < tdc && tdc < tdc_max) {
	      flag_hit_wt_ctot = true;
	    }
	  }
	}

	if (tdc1st!=0) hptr_array[sdc3t1st_ctot_id +l]->Fill(tdc1st);
	if (flag_hit_wt_ctot) {
	  ++multiplicity_wt_ctot;
	  hptr_array[sdc3hit_ctot_id + l]->Fill(w);
	  SDC3HitCont[l].push_back(w);
	}
      }

      hptr_array[sdc3mul_id + l]->Fill(multiplicity);
      hptr_array[sdc3mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[sdc3mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[sdc3mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }


    for(Int_t s=0; s<NumOfDimSDC3 ;s++) {
      Int_t corr=2*s;
      for(UInt_t i=0; i<SDC3HitCont[corr].size() ;i++) {
	for(UInt_t j=0; j<SDC3HitCont[corr+1].size() ;j++) {
	  hptr_array[sdc3self_corr_id + s]->Fill(SDC3HitCont[corr][i],SDC3HitCont[corr+1][j]);
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

  //------------------------------------------------------------------
  // SDC4
  //------------------------------------------------------------------
  std::vector< std::vector<Int_t> > SDC4HitCont(4);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("SDC4");
    static const Int_t k_leading  = gUnpacker.get_data_id("SDC4", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("SDC4", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcSDC4", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcSDC4", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("MinTotSDC4", 0);
    // static const Int_t tot_max = gUser.GetParameter("MinTotSDC4", 1);

    // sequential id
    static const Int_t sdc4t_id    = gHist.getSequentialID(kSDC4, 0, kTDC);
    static const Int_t sdc4tot_id  = gHist.getSequentialID(kSDC4, 0, kADC);
    static const Int_t sdc4t1st_id = gHist.getSequentialID(kSDC4, 0, kTDC2D);
    static const Int_t sdc4hit_id  = gHist.getSequentialID(kSDC4, 0, kHitPat);
    static const Int_t sdc4mul_id  = gHist.getSequentialID(kSDC4, 0, kMulti);
    static const Int_t sdc4mulwt_id
      = gHist.getSequentialID(kSDC4, 0, kMulti, 1+NumOfLayersSDC4);

    static const Int_t sdc4t_ctot_id    = gHist.getSequentialID(kSDC4, 0, kTDC,    11);
    static const Int_t sdc4tot_ctot_id  = gHist.getSequentialID(kSDC4, 0, kADC,    11);
    static const Int_t sdc4t1st_ctot_id = gHist.getSequentialID(kSDC4, 0, kTDC2D,  11);
    static const Int_t sdc4t2D_id       = gHist.getSequentialID(kSDC4, 0, kTDC2D,  21);
    static const Int_t sdc4t2D_ctot_id  = gHist.getSequentialID(kSDC4, 0, kTDC2D,  31);
    static const Int_t sdc4hit_ctot_id  = gHist.getSequentialID(kSDC4, 0, kHitPat, 11);
    static const Int_t sdc4mul_ctot_id  = gHist.getSequentialID(kSDC4, 0, kMulti,  11);
    static const Int_t sdc4mulwt_ctot_id
      = gHist.getSequentialID(kSDC4, 0, kMulti, 1+NumOfLayersSDC4 + 10);
    static const Int_t sdc4self_corr_id  = gHist.getSequentialID(kSDC4, kSelfCorr, 0, 1);

    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersSDC4; ++l) {
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      for(Int_t w=0; w<NumOfWireSDC4; ++w) {
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if (nhit_l == 0) continue;

	Int_t hit_l_max = 0;
	Int_t hit_t_max = 0;

	if (nhit_l != 0) {
	  hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
	}
	if (nhit_t != 0) {
	  hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
	}

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[sdc4hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for(Int_t m = 0; m<nhit_l; ++m) {
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[sdc4t_id + l]->Fill(tdc);
	  if (tdc1st<tdc) tdc1st = tdc;

	  // tdc 2D
	  hptr_array[sdc4t2D_id + l]->Fill(w,tdc);

	  // Drift time check
	  if (tdc_min < tdc && tdc < tdc_max) {
	    flag_hit_wt = true;
	  }
	}

	if (tdc1st!=0) hptr_array[sdc4t1st_id +l]->Fill(tdc1st);
	if (flag_hit_wt) {
	  ++multiplicity_wt;
	  hptr_array[sdc4hit_id + l]->Fill(w);
	}

	tdc1st = 0;
	if (nhit_l == nhit_t && hit_l_max > hit_t_max) {
	  ++multiplicity_ctot;
	  for(Int_t m = 0; m<nhit_l; ++m) {
	    tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	    tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
	    tot = tdc - tdc_t;
	    hptr_array[sdc4tot_id+l]->Fill(tot);
	    if (tot < tot_min) continue;
	    hptr_array[sdc4t_ctot_id + l]->Fill(tdc);
	    hptr_array[sdc4t2D_ctot_id + l]->Fill(w,tdc); //2D
	    hptr_array[sdc4tot_ctot_id+l]->Fill(tot);
	    if (tdc1st<tdc) tdc1st = tdc;
	    if (tdc_min < tdc && tdc < tdc_max) {
	      flag_hit_wt_ctot = true;
	    }
	  }
	}

	if (tdc1st!=0) hptr_array[sdc4t1st_ctot_id +l]->Fill(tdc1st);
	if (flag_hit_wt_ctot) {
	  ++multiplicity_wt_ctot;
	  hptr_array[sdc4hit_ctot_id + l]->Fill(w);
	  SDC4HitCont[l].push_back(w);
	}
      }

      hptr_array[sdc4mul_id + l]->Fill(multiplicity);
      hptr_array[sdc4mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[sdc4mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[sdc4mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }


    for(Int_t s=0; s<NumOfDimSDC4 ;s++) {
      Int_t corr=2*s;
      for(UInt_t i=0; i<SDC4HitCont[corr].size() ;i++) {
	for(UInt_t j=0; j<SDC4HitCont[corr+1].size() ;j++) {
	  hptr_array[sdc4self_corr_id + s]->Fill(SDC4HitCont[corr][i],SDC4HitCont[corr+1][j]);
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

  //------------------------------------------------------------------
  // SDC5
  //------------------------------------------------------------------
  std::vector< std::vector<Int_t> > SDC5HitCont(4);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("SDC5");
    static const Int_t k_leading  = gUnpacker.get_data_id("SDC5", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("SDC5", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcSDC5", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcSDC5", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("MinTotSDC5", 0);
    // static const Int_t tot_max = gUser.GetParameter("MinTotSDC5", 1);


    // sequential id
    static const Int_t sdc5t_id    = gHist.getSequentialID(kSDC5, 0, kTDC);
    static const Int_t sdc5tot_id  = gHist.getSequentialID(kSDC5, 0, kADC);
    static const Int_t sdc5t1st_id = gHist.getSequentialID(kSDC5, 0, kTDC2D);
    static const Int_t sdc5hit_id  = gHist.getSequentialID(kSDC5, 0, kHitPat);
    static const Int_t sdc5mul_id  = gHist.getSequentialID(kSDC5, 0, kMulti);
    static const Int_t sdc5mulwt_id
      = gHist.getSequentialID(kSDC5, 0, kMulti, 1+NumOfLayersSDC5);

    static const Int_t sdc5t_ctot_id    = gHist.getSequentialID(kSDC5, 0, kTDC,    11);
    static const Int_t sdc5tot_ctot_id  = gHist.getSequentialID(kSDC5, 0, kADC,    11);
    static const Int_t sdc5t1st_ctot_id = gHist.getSequentialID(kSDC5, 0, kTDC2D,  11);
    static const Int_t sdc5t2D_id       = gHist.getSequentialID(kSDC5, 0, kTDC2D,  21);
    static const Int_t sdc5t2D_ctot_id  = gHist.getSequentialID(kSDC5, 0, kTDC2D,  31);
    static const Int_t sdc5hit_ctot_id  = gHist.getSequentialID(kSDC5, 0, kHitPat, 11);
    static const Int_t sdc5mul_ctot_id  = gHist.getSequentialID(kSDC5, 0, kMulti,  11);
    static const Int_t sdc5mulwt_ctot_id
      = gHist.getSequentialID(kSDC5, 0, kMulti, 1+NumOfLayersSDC5 + 10);
    static const Int_t sdc5self_corr_id  = gHist.getSequentialID(kSDC5, kSelfCorr, 0, 1);

    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersSDC5; ++l) {
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      Int_t sdc5_nwire = 0;
      if (l==0 || l==1)
	sdc5_nwire = NumOfWireSDC5Y;
      if (l==2 || l==3)
	sdc5_nwire = NumOfWireSDC5X;

      for(Int_t w=0; w<sdc5_nwire; ++w) {
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if (nhit_l == 0) continue;

	Int_t hit_l_max = 0;
	Int_t hit_t_max = 0;

	if (nhit_l != 0) {
	  hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
	}
	if (nhit_t != 0) {
	  hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
	}

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[sdc5hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for(Int_t m = 0; m<nhit_l; ++m) {
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[sdc5t_id + l]->Fill(tdc);
	  if (tdc1st<tdc) tdc1st = tdc;

	  // tdc 2D
	  hptr_array[sdc5t2D_id + l]->Fill(w,tdc);

	  // Drift time check
	  if (tdc_min < tdc && tdc < tdc_max) {
	    flag_hit_wt = true;
	  }
	}

	if (tdc1st!=0) hptr_array[sdc5t1st_id +l]->Fill(tdc1st);
	if (flag_hit_wt) {
	  ++multiplicity_wt;
	  hptr_array[sdc5hit_id + l]->Fill(w);
	}

	tdc1st = 0;
	if (nhit_l == nhit_t && hit_l_max > hit_t_max) {
	  ++multiplicity_ctot;
	  for(Int_t m = 0; m<nhit_l; ++m) {
	    tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	    tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
	    tot = tdc - tdc_t;
	    hptr_array[sdc5tot_id+l]->Fill(tot);
	    if (tot < tot_min) continue;
	    hptr_array[sdc5t_ctot_id + l]->Fill(tdc);
	    hptr_array[sdc5t2D_ctot_id + l]->Fill(w,tdc); //2D
	    hptr_array[sdc5tot_ctot_id+l]->Fill(tot);
	    if (tdc1st<tdc) tdc1st = tdc;
	    if (tdc_min < tdc && tdc < tdc_max) {
	      flag_hit_wt_ctot = true;
	    }
	  }
	}

	if (tdc1st!=0) hptr_array[sdc5t1st_ctot_id +l]->Fill(tdc1st);
	if (flag_hit_wt_ctot) {
	  ++multiplicity_wt_ctot;
	  hptr_array[sdc5hit_ctot_id + l]->Fill(w);
	  SDC5HitCont[l].push_back(w);
	}
      }

      hptr_array[sdc5mul_id + l]->Fill(multiplicity);
      hptr_array[sdc5mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[sdc5mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[sdc5mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }

    for(Int_t s=0; s<NumOfDimSDC5 ;s++) {
      Int_t corr=2*s;
      for(UInt_t i=0; i<SDC5HitCont[corr].size() ;i++) {
	for(UInt_t j=0; j<SDC5HitCont[corr+1].size() ;j++) {
	  hptr_array[sdc5self_corr_id + s]->Fill(SDC5HitCont[corr][i],SDC5HitCont[corr+1][j]);
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

#if 0
  //------------------------------------------------------------------
  // SDC4_e42
  //------------------------------------------------------------------
  std::vector< std::vector<Int_t> > SDC4HitCont(4);
  {
    // data type
    static const Int_t k_device   = gUnpacker.get_device_id("SDC4");
    static const Int_t k_leading  = gUnpacker.get_data_id("SDC4", "leading");
    static const Int_t k_trailing = gUnpacker.get_data_id("SDC4", "trailing");

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcSDC4", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcSDC4", 1);
    // TOT gate range
    static const Int_t tot_min = gUser.GetParameter("MinTotSDC4", 0);
    static const Int_t tot_max = gUser.GetParameter("MinTotSDC4", 1);


    // sequential id
    static const Int_t sdc4t_id    = gHist.getSequentialID(kSDC4, 0, kTDC);
    static const Int_t sdc4tot_id  = gHist.getSequentialID(kSDC4, 0, kADC);
    static const Int_t sdc4t1st_id = gHist.getSequentialID(kSDC4, 0, kTDC2D);
    static const Int_t sdc4hit_id  = gHist.getSequentialID(kSDC4, 0, kHitPat);
    static const Int_t sdc4mul_id  = gHist.getSequentialID(kSDC4, 0, kMulti);
    static const Int_t sdc4mulwt_id
      = gHist.getSequentialID(kSDC4, 0, kMulti, 1+NumOfLayersSDC4);

    static const Int_t sdc4t_ctot_id    = gHist.getSequentialID(kSDC4, 0, kTDC,    11);
    static const Int_t sdc4tot_ctot_id  = gHist.getSequentialID(kSDC4, 0, kADC,    11);
    static const Int_t sdc4t1st_ctot_id = gHist.getSequentialID(kSDC4, 0, kTDC2D,  11);
    static const Int_t sdc4hit_ctot_id  = gHist.getSequentialID(kSDC4, 0, kHitPat, 11);
    static const Int_t sdc4mul_ctot_id  = gHist.getSequentialID(kSDC4, 0, kMulti,  11);
    static const Int_t sdc4mulwt_ctot_id
      = gHist.getSequentialID(kSDC4, 0, kMulti, 1+NumOfLayersSDC4 + 10);
    static const Int_t sdc4self_corr_id  = gHist.getSequentialID(kSDC4, kSelfCorr, 0, 1);

    // TDC & HitPat & Multi
    for(Int_t l=0; l<NumOfLayersSDC4; ++l) {
      Int_t tdc                  = 0;
      Int_t tdc_t                = 0;
      Int_t tot                  = 0;
      Int_t tdc1st               = 0;
      Int_t multiplicity         = 0;
      Int_t multiplicity_wt      = 0;
      Int_t multiplicity_ctot    = 0;
      Int_t multiplicity_wt_ctot = 0;
      Int_t sdc4_nwire = 0;
      if (l==0 || l==1)
	sdc4_nwire = NumOfWireSDC4Y;
      if (l==2 || l==3)
	sdc4_nwire = NumOfWireSDC4X;

      for(Int_t w=0; w<sdc4_nwire; ++w) {
	Int_t nhit_l = gUnpacker.get_entries(k_device, l, 0, w, k_leading);
	Int_t nhit_t = gUnpacker.get_entries(k_device, l, 0, w, k_trailing);
	if (nhit_l == 0) continue;

	Int_t hit_l_max = 0;
	Int_t hit_t_max = 0;

	if (nhit_l != 0) {
	  hit_l_max = gUnpacker.get(k_device, l, 0, w, k_leading,  nhit_l - 1);
	}
	if (nhit_t != 0) {
	  hit_t_max = gUnpacker.get(k_device, l, 0, w, k_trailing, nhit_t - 1);
	}

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[sdc4hit_id + l]->Fill(w, nhit);

	Bool_t flag_hit_wt = false;
	Bool_t flag_hit_wt_ctot = false;
	for(Int_t m = 0; m<nhit_l; ++m) {
	  tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	  hptr_array[sdc4t_id + l]->Fill(tdc);
	  if (tdc1st<tdc) tdc1st = tdc;

	  // Drift time check
	  if (tdc_min < tdc && tdc < tdc_max) {
	    flag_hit_wt = true;
	  }
	}

	if (tdc1st!=0) hptr_array[sdc4t1st_id +l]->Fill(tdc1st);
	if (flag_hit_wt) {
	  ++multiplicity_wt;
	  hptr_array[sdc4hit_id + l]->Fill(w);
	}

	tdc1st = 0;
	if (nhit_l == nhit_t && hit_l_max > hit_t_max) {
	  ++multiplicity_ctot;
	  for(Int_t m = 0; m<nhit_l; ++m) {
	    tdc = gUnpacker.get(k_device, l, 0, w, k_leading, m);
	    tdc_t = gUnpacker.get(k_device, l, 0, w, k_trailing, m);
	    tot = tdc - tdc_t;
	    hptr_array[sdc4tot_id+l]->Fill(tot);
	    if (tot < tot_min || tot >tot_max) continue;
	    hptr_array[sdc4t_ctot_id + l]->Fill(tdc);
	    hptr_array[sdc4tot_ctot_id+l]->Fill(tot);
	    if (tdc1st<tdc) tdc1st = tdc;
	    if (tdc_min < tdc && tdc < tdc_max) {
	      flag_hit_wt_ctot = true;
	    }
	  }
	}

	if (tdc1st!=0) hptr_array[sdc4t1st_ctot_id +l]->Fill(tdc1st);
	if (flag_hit_wt_ctot) {
	  ++multiplicity_wt_ctot;
	  hptr_array[sdc4hit_ctot_id + l]->Fill(w);
	  SDC4HitCont[l].push_back(w);
	}
      }

      hptr_array[sdc4mul_id + l]->Fill(multiplicity);
      hptr_array[sdc4mulwt_id + l]->Fill(multiplicity_wt);
      hptr_array[sdc4mul_ctot_id   + l]->Fill(multiplicity_ctot);
      hptr_array[sdc4mulwt_ctot_id + l]->Fill(multiplicity_wt_ctot);
    }

    for(Int_t s=0; s<NumOfDimSDC4 ;s++) {
      Int_t corr=2*s;
      for(UInt_t i=0; i<SDC4HitCont[corr].size() ;i++) {
	for(UInt_t j=0; j<SDC4HitCont[corr+1].size() ;j++) {
	  hptr_array[sdc4self_corr_id + s]->Fill(SDC4HitCont[corr][i],SDC4HitCont[corr+1][j]);
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

#endif

  std::vector<Int_t> hitseg_tof;
  { ///// TOF
    static const auto device_id = gUnpacker.get_device_id("TOF");
    static const auto adc_id = gUnpacker.get_data_id("TOF", "adc");
    static const auto tdc_id = gUnpacker.get_data_id("TOF", "tdc");
    static const auto tdc_min = gUser.GetParameter("TdcTOF", 0);
    static const auto tdc_max = gUser.GetParameter("TdcTOF", 1);
    static const auto adc_hid = gHist.getSequentialID(kTOF, 0, kADC);
    static const auto tdc_hid = gHist.getSequentialID(kTOF, 0, kTDC);
    static const auto awt_hid = gHist.getSequentialID(kTOF, 0, kADCwTDC);
    static const auto hit_hid = gHist.getSequentialID(kTOF, 0, kHitPat);
    static const auto mul_hid = gHist.getSequentialID(kTOF, 0, kMulti);
    std::vector<std::vector<Int_t>> hit_flag(NumOfSegTOF);
    Int_t multiplicity = 0;
    for(Int_t seg=0; seg<NumOfSegTOF; ++seg) {
      hit_flag[seg].resize(kUorD);
      for(Int_t ud=0; ud<kUorD; ++ud) {
	hit_flag[seg][ud] = 0;
	// ADC
	UInt_t adc = 0;
	auto nhit = gUnpacker.get_entries(device_id, 0, seg, ud, adc_id);
	if (nhit != 0) {
	  adc = gUnpacker.get(device_id, 0, seg, ud, adc_id);
	  hptr_array[adc_hid + ud*NumOfSegTOF + seg]->Fill(adc);
	}
	// TDC
	for(Int_t m=0, n=gUnpacker.get_entries(device_id, 0, seg, ud, tdc_id);
	    m<n; ++m) {
	  auto tdc = gUnpacker.get(device_id, 0, seg, ud, tdc_id, m);
	  if (tdc != 0) {
	    hptr_array[tdc_hid + ud*NumOfSegTOF + seg]->Fill(tdc);
	    if (tdc_min<tdc && tdc<tdc_max && adc > 0) {
	      hit_flag[seg][ud] = 1;
	    }
	  }
	}
	if (hit_flag[seg][ud] == 1) {
	  hptr_array[awt_hid + ud*NumOfSegTOF + seg]->Fill(adc);
	}
      }
      if (hit_flag[seg][kU] == 1 && hit_flag[seg][kD] == 1) {
	++multiplicity;
	hptr_array[hit_hid]->Fill(seg);
	hitseg_tof.push_back(seg);
      }
    }
    hptr_array[mul_hid]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif


  //------------------------------------------------------------------
  //AC1
  //------------------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("AC1");
    static const Int_t k_adc    = gUnpacker.get_data_id("AC1","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("AC1","tdc");

    static const Int_t ac1a_id   = gHist.getSequentialID(kAC1, 0, kADC);
    static const Int_t ac1t_id   = gHist.getSequentialID(kAC1, 0, kTDC);
    static const Int_t ac1awt_id  = gHist.getSequentialID(kAC1, 0, kADCwTDC);
    static const Int_t ac1hit_id  = gHist.getSequentialID(kAC1, 0, kHitPat);
    static const Int_t ac1mul_id  = gHist.getSequentialID(kAC1, 0, kMulti);
    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcAC1", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcAC1", 1);

    Int_t multiplicity = 0;

    for(Int_t seg = 0; seg<NumOfSegAC1; ++seg) {
      // ADC
      if(seg>NumOfSegAC1-5 && seg<NumOfSegAC1-1){
        Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
        if (nhit_a!=0) {
	  Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	  hptr_array[ac1a_id + seg-NumOfSegAC1+4 ]->Fill(adc);
        }
      }
      if(seg>NumOfSegAC1-2){
        Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg-1, 0, k_adc);
        if (nhit_a!=0) {
	  Int_t adc = gUnpacker.get(k_device, 0, seg-1, 0, k_adc);
	  hptr_array[ac1a_id + seg-NumOfSegAC1+4 ]->Fill(adc);
        }
      }

      // INDIVISUAL TDC
      Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      Bool_t is_in_gate = false;

      for(Int_t m = 0; m<nhit_t; ++m) {
	Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
	hptr_array[ac1t_id + seg]->Fill(tdc);

	if (tdc_min < tdc && tdc < tdc_max) {
	  is_in_gate = true;
	}// tdc range is ok
      }// for tdc

      if (is_in_gate) {
        if(seg>NumOfSegAC1-5 && seg<NumOfSegAC1-1){
	  if (gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0) {
	    Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	    hptr_array[ac1awt_id + seg-NumOfSegAC1+4 ]->Fill(adc);
	  }
        }
      	else{
	  if(seg>NumOfSegAC1-2){
	    if (gUnpacker.get_entries(k_device, 0, seg-1, 0, k_adc)>0) {
	      Int_t adc = gUnpacker.get(k_device, 0, seg-1, 0, k_adc);
	      hptr_array[ac1awt_id + seg-NumOfSegAC1+4 ]->Fill(adc);
	    }
	  }
	  else{
	    	++multiplicity;
      	        hptr_array[ac1hit_id]->Fill(seg);
	  }
	}
      }
    }
    hptr_array[ac1mul_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//AC

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

//   //------------------------------------------------------------------
//   //LAC
//   //------------------------------------------------------------------
//   {
//     // data typep
//     static const Int_t k_device = gUnpacker.get_device_id("LAC");
//     static const Int_t k_u      = 0; // up
//     static const Int_t k_tdc    = gUnpacker.get_data_id("LAC","tdc");
//     static const Int_t lachit_id = gHist.getSequentialID(kLAC, 0, kHitPat);
//     static const Int_t lacmul_id = gHist.getSequentialID(kLAC, 0, kMulti);

//     // TDC gate range
//     static const Int_t tdc_min = gUser.GetParameter("TdcLAC", 0);
//     static const Int_t tdc_max = gUser.GetParameter("TdcLAC", 1);

//     Int_t lact_id   = gHist.getSequentialID(kLAC, 0, kTDC);
//     Int_t multiplicity = 0;
//     for(Int_t seg = 0; seg<NumOfSegLAC; ++seg) {
//       Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
//       Bool_t is_in_gate = false;
//       for(Int_t m = 0; m<nhit; ++m) {
// 	Int_t tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc, m);
// 	hptr_array[lact_id + seg]->Fill(tdc);

// 	if (tdc_min < tdc && tdc < tdc_max) is_in_gate = true;
//       }

//       // Hit pattern && multiplicity
//       if (is_in_gate) {
// 	hptr_array[lachit_id]->Fill(seg);
// 	++multiplicity;
//       }
//     }

//     hptr_array[lacmul_id]->Fill(multiplicity);

// #if 0
//     // Debug, dump data relating this detector
//     gUnpacker.dump_data_device(k_device);
// #endif
//   }

// #if DEBUG
//   std::cout << __FILE__ << " " << __LINE__ << std::endl;
// #endif


  // SAC3  -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("SAC3");
    static const Int_t k_adc    = gUnpacker.get_data_id("SAC3","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("SAC3","tdc");

    static const Int_t a_id   = gHist.getSequentialID(kSAC3, 0, kADC);
    static const Int_t t_id   = gHist.getSequentialID(kSAC3, 0, kTDC);
    static const Int_t awt_id = gHist.getSequentialID(kSAC3, 0, kADCwTDC);
//    static const Int_t h_id   = gHist.getSequentialID(kE90SAC, 0, kHitPat);
//    static const Int_t m6_id   = gHist.getSequentialID(kE90SAC, 0, kMulti, 1);
//    static const Int_t m8_id   = gHist.getSequentialID(kE90SAC, 0, kMulti, 2);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcSAC3", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcSAC3", 1);

  //  Int_t multiplicity[2] = {0, 0};
    for(Int_t seg = 0; seg<NumOfSegSAC3; ++seg) {
      // ADC
      Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      if (nhit_a!=0) {
	Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[a_id + seg]->Fill(adc);
      }
      // TDC
      Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      Bool_t is_in_gate = false;

      for(Int_t m = 0; m<nhit_t; ++m) {
        Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
        hptr_array[t_id + seg]->Fill(tdc);

        if (tdc_min < tdc && tdc < tdc_max) {
          is_in_gate = true;
        }// tdc range is ok
      }// for(m)

      if (is_in_gate) {
        // ADC w/TDC
	// SAC3 segment 1 is dummy, only segment 0 is used.
        if (gUnpacker.get_entries(k_device, 0, 0, 0, k_adc)>0) {
          Int_t adc = gUnpacker.get(k_device, 0, 0, 0, k_adc);
          hptr_array[awt_id + seg]->Fill(adc);
        }
        //hptr_array[h_id]->Fill(seg);
        //hptr_array[e72para_id]->Fill(e72parasite::kE90SAC1 + seg);
        //++multiplicity[seg];
      }// flag is OK
    }

  //  hptr_array[m6_id]->Fill(multiplicity[0]);
  //  hptr_array[m8_id]->Fill(multiplicity[1]);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//SAC3

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif


  // SFV  -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("SFV");
    //static const Int_t k_adc    = gUnpacker.get_data_id("SFV","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("SFV","tdc");

//    static const Int_t a_id   = gHist.getSequentialID(kSFV, 0, kADC, 1);
    static const Int_t SFVt_id   = gHist.getSequentialID(kSFV, 0, kTDC);
//    static const Int_t awt_id = gHist.getSequentialID(kE90SAC, 0, kADCwTDC);
    static const Int_t SFVhit_id   = gHist.getSequentialID(kSFV, 0, kHitPat);
    static const Int_t SFVmul_id   = gHist.getSequentialID(kSFV, 0, kMulti);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcSFV", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcSFV", 1);

    Int_t multiplicity = 0;

    for(Int_t seg = 0; seg<NumOfSegSFV; ++seg) {
  //    // ADC
  //    Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
  //    if (nhit_a!=0) {
  //      Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
  //      hptr_array[a_id + seg]->Fill(adc);
  //    }
      // TDC
      Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      Bool_t is_in_gate = false;

      for(Int_t m = 0; m<nhit_t; ++m) {
        Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
        hptr_array[SFVt_id + seg]->Fill(tdc);

        if (tdc_min < tdc && tdc < tdc_max) {
          is_in_gate = true;
        }// tdc range is ok
      }// for(m)

      if (is_in_gate) {
       if(seg<NumOfSegSFV-1){
  //      // ADC w/TDC
  //      if (gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0) {
  //        Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
  //        hptr_array[awt_id + seg]->Fill(adc);
  //      }
        hptr_array[SFVhit_id]->Fill(seg);
        ++multiplicity;
       }
      }// flag is OK
    }

    hptr_array[SFVmul_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//SFV

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif


  // WC for E42 test-----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("WC");
    static const Int_t k_u      = 0; // up
    static const Int_t k_d      = 1; // down
    static const Int_t k_sum    = 2; // sum
    static const Int_t k_adc    = gUnpacker.get_data_id("WC", "adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("WC", "tdc");
    // TDC gate range
    static const UInt_t tdc_min = gUser.GetParameter("TdcWC", 0);
    static const UInt_t tdc_max = gUser.GetParameter("TdcWC", 1);
    // UP
    Int_t wca_id   = gHist.getSequentialID(kWC, 0, kADC);
    Int_t wct_id   = gHist.getSequentialID(kWC, 0, kTDC);
    Int_t wcawt_id = gHist.getSequentialID(kWC, 0, kADCwTDC);    // UP
    for(Int_t seg=0; seg<NumOfSegWC; ++seg) {
      // ADC
      Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc);
      if (nhit != 0) {
	UInt_t adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	hptr_array[wca_id + seg]->Fill(adc);
      }
      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      for(Int_t m = 0; m<nhit; ++m) {
	UInt_t tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc, m);
	if (tdc!=0) {
	  hptr_array[wct_id + seg]->Fill(tdc);
	  // ADC w/TDC
	  if (tdc_min<tdc && tdc<tdc_max &&
	      gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc)>0) {
	    UInt_t adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	    hptr_array[wcawt_id + seg]->Fill(adc);
	  }
	}
      }
    }

    // DOWN
    wca_id   = gHist.getSequentialID(kWC, 0, kADC,     NumOfSegWC+1);
    wct_id   = gHist.getSequentialID(kWC, 0, kTDC,     NumOfSegWC+1);
    wcawt_id = gHist.getSequentialID(kWC, 0, kADCwTDC, NumOfSegWC+1);    // Down
    for(Int_t seg=0; seg<NumOfSegWC; ++seg) {
      // ADC
      Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc);
      if (nhit != 0) {
	UInt_t adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	hptr_array[wca_id + seg]->Fill(adc);
      }
      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      for(Int_t m = 0; m<nhit; ++m) {
	UInt_t tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc, m);
	if (tdc!=0) {
 	  hptr_array[wct_id + seg]->Fill(tdc);
	  // ADC w/TDC
	  if (tdc_min<tdc && tdc<tdc_max &&
	      gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc)>0) {
	    UInt_t adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	    hptr_array[wcawt_id + seg]->Fill(adc);
	  }
	}
      }
    }

    // SUM
    wca_id   = gHist.getSequentialID(kWC, 0, kADC,     NumOfSegWC*2+1);
    wct_id   = gHist.getSequentialID(kWC, 0, kTDC,     NumOfSegWC*2+1);
    wcawt_id = gHist.getSequentialID(kWC, 0, kADCwTDC, NumOfSegWC*2+1);    // Sum
    Int_t multi = 0;
    for(Int_t seg=0; seg<NumOfSegWC; ++seg) {
      // ADC
      Int_t nhit = gUnpacker.get_entries(k_device, 0, seg, k_sum, k_adc);
      if (nhit != 0) {
	UInt_t adc = gUnpacker.get(k_device, 0, seg, k_sum, k_adc);
	hptr_array[wca_id + seg]->Fill(adc);
      }
      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_sum, k_tdc);
      Bool_t is_in_gate = false;
      for(Int_t m = 0; m<nhit; ++m) {
	UInt_t tdc = gUnpacker.get(k_device, 0, seg, k_sum, k_tdc, m);
	if (tdc!=0) {
	  hptr_array[wct_id + seg]->Fill(tdc);
	  // ADC w/TDC
	  if (tdc_min<tdc && tdc<tdc_max &&
	      gUnpacker.get_entries(k_device, 0, seg, k_sum, k_adc)>0) {
	    is_in_gate = true;
	    UInt_t adc = gUnpacker.get(k_device, 0, seg, k_sum, k_adc);
	    hptr_array[wcawt_id + seg]->Fill(adc);
	  }
	}
      }
      if (is_in_gate) {
	hptr_array[gHist.getSequentialID(kWC, 0, kHitPat)]->Fill(seg);
	++multi;
      }
    }
    hptr_array[gHist.getSequentialID(kWC, 0, kMulti)]->Fill(multi);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

  // Correlation (2D histograms) -------------------------------------
  {
    // data type
    static const Int_t k_device_bh1  = gUnpacker.get_device_id("BH1");
    static const Int_t k_device_bh2  = gUnpacker.get_device_id("BH2");
    static const Int_t k_device_bc3  = gUnpacker.get_device_id("BC3");
    static const Int_t k_device_bc4  = gUnpacker.get_device_id("BC4");
    static const Int_t k_device_sdc1 = gUnpacker.get_device_id("SDC1");
    // static const Int_t k_device_sdc2 = gUnpacker.get_device_id("SDC2");
    static const Int_t k_device_sdc3 = gUnpacker.get_device_id("SDC3");
    static const Int_t k_device_sdc4 = gUnpacker.get_device_id("SDC4");
    static const Int_t k_device_tof = gUnpacker.get_device_id("TOF");
    static const Int_t k_device_ac1 = gUnpacker.get_device_id("AC1");
    static const Int_t k_device_wc = gUnpacker.get_device_id("WC");

    // sequential id
    Int_t cor_id = gHist.getSequentialID(kCorrelation, 0, 0, 1);
    Int_t mtx2d_id = gHist.getSequentialID(kCorrelation, 1, 0, 1);
    Int_t mtx3d_id = gHist.getSequentialID(kCorrelation, 2, 0, 1);

    // BH1 vs BFT
    TH2* hcor_bh1bft = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t seg1 = 0; seg1<NumOfSegBH1; ++seg1) {
      for(const auto& seg2: hitseg_bftu) {
	Int_t nhitBH1 = gUnpacker.get_entries(k_device_bh1, 0, seg1, 0, 1);
	if (nhitBH1 == 0) continue;
	Int_t tdcBH1 = gUnpacker.get(k_device_bh1, 0, seg1, 0, 1);
	Bool_t hitBH1 = (tdcBH1 > 0);
	if (hitBH1) {
	  hcor_bh1bft->Fill(seg1, seg2);
	}
      }
    }

    // BH1 vs BH2
    TH2* hcor_bh1bh2 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t seg1 = 0; seg1<NumOfSegBH1; ++seg1) {
      for(Int_t seg2 = 0; seg2<NumOfSegBH2; ++seg2) {
	Int_t hitBH1 = gUnpacker.get_entries(k_device_bh1, 0, seg1, 0, 1);
	Int_t hitBH2 = gUnpacker.get_entries(k_device_bh2, 0, seg2, 0, 1);
	if (hitBH1 == 0 || hitBH2 == 0)continue;
	Int_t tdcBH1 = gUnpacker.get(k_device_bh1, 0, seg1, 0, 1);
	Int_t tdcBH2 = gUnpacker.get(k_device_bh2, 0, seg2, 0, 1);
	if (tdcBH1 != 0 && tdcBH2 != 0) {
	  hcor_bh1bh2->Fill(seg1, seg2);
	}
      }
    }

    // BC3 vs BC4
    TH2* hcor_bc3bc4 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t wire1 = 0; wire1<NumOfWireBC3; ++wire1) {
      for(Int_t wire2 = 0; wire2<NumOfWireBC4; ++wire2) {
	Int_t hitBC3 = gUnpacker.get_entries(k_device_bc3, 0, 0, wire1, 0);
	Int_t hitBC4 = gUnpacker.get_entries(k_device_bc4, 5, 0, wire2, 0);
	if (hitBC3 == 0 || hitBC4 == 0)continue;
	hcor_bc3bc4->Fill(wire1, wire2);
      }
    }

    // SDC3 vs SDC1
    TH2* hcor_sdc1sdc3 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t wire1 = 0; wire1<NumOfWireSDC1; ++wire1) {
      for(Int_t wire3 = 0; wire3<NumOfWireSDC3; ++wire3) {
	Int_t hitSDC1 = gUnpacker.get_entries(k_device_sdc1, 0, 0, wire1, 0);
	Int_t hitSDC3 = gUnpacker.get_entries(k_device_sdc3, 0, 0, wire3, 0);
	if (hitSDC1 == 0 || hitSDC3 == 0) continue;
	hcor_sdc1sdc3->Fill(wire1, wire3);
      }
    }

    // SDC3 vs SDC4
    TH2* hcor_sdc3sdc4 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t wire3 = 0; wire3<NumOfWireSDC3; ++wire3) {
      for(Int_t wire4 = 0; wire4<NumOfWireSDC4; ++wire4) {
	Int_t hitSDC3 = gUnpacker.get_entries(k_device_sdc3, 0, 0, wire3, 0);
	Int_t hitSDC4 = gUnpacker.get_entries(k_device_sdc4, 0, 0, wire4, 0);
	if (hitSDC3 == 0 || hitSDC4 == 0) continue;
	hcor_sdc3sdc4->Fill(wire3, wire4);
      }
    }

    // TOF vs SDC4
    TH2* hcor_tofsdc4 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(const auto& seg_tof: hitseg_tof) {
      for(Int_t wire=0; wire<NumOfWireSDC4X; ++wire) {
	Int_t hitSDC4 = gUnpacker.get_entries(k_device_sdc4, 2, 0, wire, 0);
	if (hitSDC4 == 0) continue;
	hcor_tofsdc4->Fill(wire, seg_tof);
      }
    }

    // AC1 vs TOF
    TH2* hcor_ac1tof = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t seg1 = 0; seg1<NumOfSegAC1-2; ++seg1) {
      for(Int_t seg2 = 0; seg2<NumOfSegTOF; ++seg2) {
	Int_t hitAC1 = gUnpacker.get_entries(k_device_ac1, 0, seg1, 0, 1);
	Int_t hitTOF = gUnpacker.get_entries(k_device_tof, 0, seg2, 0, 1);
	if (hitAC1 == 0 || hitTOF == 0)continue;
	Int_t tdcac1 = gUnpacker.get(k_device_ac1, 0, seg1, 0, 1);
	Int_t tdctof = gUnpacker.get(k_device_tof, 0, seg2, 0, 1);
	if (tdcac1 != 0 && tdctof != 0) {
	  hcor_ac1tof->Fill(seg1, seg2);
	}
      }
    }

    // WC vs TOF
    TH2* hcor_wctof = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(Int_t seg1 = 0; seg1<NumOfSegWC; ++seg1) {
      for(Int_t seg2 = 0; seg2<NumOfSegTOF; ++seg2) {
	Int_t hitWC = gUnpacker.get_entries(k_device_wc, 0, seg1, 0, 1);
	Int_t hitTOF = gUnpacker.get_entries(k_device_tof, 0, seg2, 0, 1);
	if (hitWC == 0 || hitTOF == 0)continue;
	Int_t tdcwc = gUnpacker.get(k_device_wc, 0, seg1, 0, 1);
	Int_t tdctof = gUnpacker.get(k_device_tof, 0, seg2, 0, 1);
	if (tdcwc != 0 && tdctof != 0) {
	  hcor_wctof->Fill(seg1, seg2);
	}
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
    static const Int_t k_d_bh1  = gUnpacker.get_device_id("BH1");
    static const Int_t k_d_bh2  = gUnpacker.get_device_id("BH2");
    static const Int_t k_tdc    = gUnpacker.get_data_id("BH1", "tdc");
    // HodoParam
    static const Int_t cid_bh1  = 1;
    static const Int_t cid_bh2  = 2;
    static const Int_t plid     = 0;
    // Sequential ID
    static const Int_t btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);
    static const auto& hodoMan = HodoParamMan::GetInstance();
    // TDC gate range
    static const UInt_t tdc_min_bh1 = gUser.GetParameter("TdcBH1", 0);
    static const UInt_t tdc_max_bh1 = gUser.GetParameter("TdcBH1", 1);
    static const UInt_t tdc_min_bh2 = gUser.GetParameter("TdcBH2", 0);
    static const UInt_t tdc_max_bh2 = gUser.GetParameter("TdcBH2", 1);
    // BH2
    Double_t t0  = 1e10;
    Double_t ofs = 0;
    for(Int_t seg=0; seg<NumOfSegBH2; ++seg) {
      Int_t nhitu = gUnpacker.get_entries(k_d_bh2, 0, seg, kU, k_tdc);
      Int_t nhitd = gUnpacker.get_entries(k_d_bh2, 0, seg, kD, k_tdc);
      for(Int_t mu=0; mu<nhitu; ++mu) {
	auto tdcu = gUnpacker.get(k_d_bh2, 0, seg, kU, k_tdc, mu);
	if (tdcu < tdc_min_bh2 || tdc_max_bh2 < tdcu) continue;
	for(Int_t md=0; md<nhitd; ++md) {
	  auto tdcd = gUnpacker.get(k_d_bh2, 0, seg, kD, k_tdc, md);
	  if (tdcd < tdc_min_bh2 || tdc_max_bh2 < tdcd) continue;
	  Double_t bh2ut, bh2dt;
	  hodoMan.GetTime(cid_bh2, plid, seg, kU, tdcu, bh2ut);
	  hodoMan.GetTime(cid_bh2, plid, seg, kD, tdcd, bh2dt);
	  Double_t bh2mt = (bh2ut + bh2dt)/2.;
	  if (TMath::Abs(t0) > TMath::Abs(bh2mt)) {
	    hodoMan.GetTime(cid_bh2, plid, seg, 2, 0, ofs);
	    t0 = bh2ut;
	  }
	}
      }
    }
    // BH1
    for(Int_t seg=0; seg<NumOfSegBH1; ++seg) {
      Int_t nhitu = gUnpacker.get_entries(k_d_bh1, 0, seg, kU, k_tdc);
      Int_t nhitd = gUnpacker.get_entries(k_d_bh1, 0, seg, kD, k_tdc);
      for(Int_t mu=0; mu<nhitu; ++mu) {
	auto tdcu = gUnpacker.get(k_d_bh1, 0, seg, kU, k_tdc, mu);
	if (tdcu < tdc_min_bh1 || tdc_max_bh1 < tdcu) continue;
	for(Int_t md=0; md<nhitd; ++md) {
	  auto tdcd = gUnpacker.get(k_d_bh1, 0, seg, kD, k_tdc, md);
	  if (tdcd < tdc_min_bh1 || tdc_max_bh1 < tdcd) continue;
	  Double_t bh1tu, bh1td;
	  hodoMan.GetTime(cid_bh1, plid, seg, kU, tdcu, bh1tu);
	  hodoMan.GetTime(cid_bh1, plid, seg, kD, tdcd, bh1td);
	  Double_t mt = (bh1tu+bh1td)/2.;
	  Double_t btof = mt-(t0+ofs);
	  hptr_array[btof_id]->Fill(btof);
	}// if (tdc)
      }// if (nhit)
    }// for(seg)
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  //------------------------------------------------------------------
  // BH1-6_BH2-3
  //------------------------------------------------------------------
  {
    // Unpacker
    static const Int_t k_d_bh1  = gUnpacker.get_device_id("BH1");
    static const Int_t k_d_bh2  = gUnpacker.get_device_id("BH2");
    static const Int_t k_tdc    = gUnpacker.get_data_id("BH1", "tdc");
    // Sequential ID
    static const Int_t btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);
    // BH2
    Int_t    multiplicity = 0;
    Double_t t0  = 1e10;
    Double_t ofs = 0;
    Int_t seg = 3;
    Int_t nhitu = gUnpacker.get_entries(k_d_bh2, 0, seg, kU, k_tdc);
    Int_t nhitd = gUnpacker.get_entries(k_d_bh2, 0, seg, kD, k_tdc);
    if (nhitu != 0 && nhitd != 0) {
      Int_t tdcu = gUnpacker.get(k_d_bh2, 0, seg, kU, k_tdc);
      Int_t tdcd = gUnpacker.get(k_d_bh2, 0, seg, kD, k_tdc);
      if (tdcu != 0 && tdcd != 0) {
	++multiplicity;
	t0 = (Double_t)(tdcu+tdcd)/2.;
      }//if (tdc)
    }// if (nhit)
    if (multiplicity == 1) {
      seg = 5;
      // BH1
      Int_t nhitu = gUnpacker.get_entries(k_d_bh1, 0, seg, kU, k_tdc);
      Int_t nhitd = gUnpacker.get_entries(k_d_bh1, 0, seg, kD, k_tdc);
      if (nhitu != 0 &&  nhitd != 0) {
	Int_t tdcu = gUnpacker.get(k_d_bh1, 0, seg, kU, k_tdc);
	Int_t tdcd = gUnpacker.get(k_d_bh1, 0, seg, kD, k_tdc);
	if (tdcu != 0 && tdcd != 0) {
	  Double_t mt = (Double_t)(tdcu+tdcd)/2.;
	  Double_t btof = mt-(t0+ofs);
	  hptr_array[btof_id +1]->Fill(btof);
	}// if (tdc)
      }// if (nhit)
    }
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // BAC -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("BAC");
    static const Int_t k_adc    = gUnpacker.get_data_id("BAC","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("BAC","tdc");

    // sequential id
    /*
      static const Int_t baca_id   = gHist.getSequentialID(kBAC, 0, kADC,     2);
      static const Int_t bact_id   = gHist.getSequentialID(kBAC, 0, kTDC,     2);
      static const Int_t bacawt_id = gHist.getSequentialID(kBAC, 0, kADCwTDC, 2);
      static const Int_t bach_id   = gHist.getSequentialID(kBAC, 0, kHitPat,  1);
      static const Int_t bacm_id   = gHist.getSequentialID(kBAC, 0, kMulti,   1);
    */

    static const Int_t baca_id   = gHist.getSequentialID(kBAC, 0, kADC);
    static const Int_t bact_id   = gHist.getSequentialID(kBAC, 0, kTDC);
    static const Int_t bacawt_id = gHist.getSequentialID(kBAC, 0, kADCwTDC);
    static const Int_t bach_id   = gHist.getSequentialID(kBAC, 0, kHitPat);
    static const Int_t bacm_id   = gHist.getSequentialID(kBAC, 0, kMulti);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcBAC", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcBAC", 1);

    Int_t multiplicity = 0;
    for(Int_t seg = 0; seg<NumOfSegBAC; ++seg) {
      // ADC
      Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      if (nhit_a!=0) {
	Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[baca_id + seg]->Fill(adc);
      }
      // TDC
      Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      Bool_t is_in_gate = false;

      for(Int_t m = 0; m<nhit_t; ++m) {
	Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
	hptr_array[bact_id + seg]->Fill(tdc);

	if (tdc_min < tdc && tdc < tdc_max) {
	  is_in_gate = true;
	}// tdc range is ok
      }// for(m)

      if (is_in_gate) {
	// ADC w/TDC
	if (gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0) {
	  Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	  hptr_array[bacawt_id + seg]->Fill(adc);
	}
	hptr_array[bach_id]->Fill(seg);
	++multiplicity;
      }// flag is OK
    }

    hptr_array[bacm_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//BAC


  // TF_TF  -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("TF_TF");
    static const Int_t k_adc    = gUnpacker.get_data_id("TF_TF","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("TF_TF","tdc");

    static const Int_t a_id   = gHist.getSequentialID(kTF_TF, 0, kADC);
    static const Int_t t_id   = gHist.getSequentialID(kTF_TF, 0, kTDC);
    static const Int_t awt_id = gHist.getSequentialID(kTF_TF, 0, kADCwTDC);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcTF_TF", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcTF_TF", 1);

    for(Int_t seg = 0; seg<NumOfSegTF_TF; ++seg) {
      // ADC
      Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      if (nhit_a!=0) {
	Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[a_id + seg]->Fill(adc);
      }
      // TDC
      Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      Bool_t is_in_gate = false;

      for(Int_t m = 0; m<nhit_t; ++m) {
        Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
        hptr_array[t_id + seg]->Fill(tdc);

        if (tdc_min < tdc && tdc < tdc_max) {
          is_in_gate = true;
        }// tdc range is ok
      }// for(m)
    if (is_in_gate) {
        // ADC w/TDC
        if (gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0) {
          Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
          hptr_array[awt_id + seg]->Fill(adc);
        }
        //hptr_array[h_id]->Fill(seg);
        //hptr_array[e72para_id]->Fill(e72parasite::kE90SAC1 + seg);
        //++multiplicity[seg];
      }// flag is OK
    }



#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//TF_TF

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif


  // TF_GN1  -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("TF_GN1");
    //static const Int_t k_adc    = gUnpacker.get_data_id("TF_GN1","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("TF_GN1","tdc");

    //static const Int_t a_id   = gHist.getSequentialID(kTF_GN1, 0, kADC);
    static const Int_t t_id   = gHist.getSequentialID(kTF_GN1, 0, kTDC);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcTF_GN1", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcTF_GN1", 1);

    for(Int_t seg = 0; seg<NumOfSegTF_GN1; ++seg) {
      // ADC
    //  Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
    //  if (nhit_a!=0) {
    //    Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
    //    hptr_array[a_id + seg]->Fill(adc);
    //  }
      // TDC
      Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      Bool_t is_in_gate = false;

      for(Int_t m = 0; m<nhit_t; ++m) {
        Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
        hptr_array[t_id + seg]->Fill(tdc);

        if (tdc_min < tdc && tdc < tdc_max) {
          is_in_gate = true;
        }// tdc range is ok
      }// for(m)

    }


#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//TF_GN1

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif


  // TF_GN2  -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("TF_GN2");
    //static const Int_t k_adc    = gUnpacker.get_data_id("TF_GN2","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("TF_GN2","tdc");

    //static const Int_t a_id   = gHist.getSequentialID(kTF_GN2, 0, kADC);
    static const Int_t t_id   = gHist.getSequentialID(kTF_GN2, 0, kTDC);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcTF_GN2", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcTF_GN2", 1);

    for(Int_t seg = 0; seg<NumOfSegTF_GN2; ++seg) {
      // ADC
    //  Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
    //  if (nhit_a!=0) {
    //    Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
    //    hptr_array[a_id + seg]->Fill(adc);
    //  }
      // TDC
      Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      Bool_t is_in_gate = false;

      for(Int_t m = 0; m<nhit_t; ++m) {
        Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
        hptr_array[t_id + seg]->Fill(tdc);

        if (tdc_min < tdc && tdc < tdc_max) {
          is_in_gate = true;
        }// tdc range is ok
      }// for(m)

    }


#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//TF_GN2

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif


  // ======================== E72E90 ==============================
#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // E72Parasite -----------------------------------------------------------
  static const Int_t e72para_id = gHist.getSequentialID(kE72Parasite, 0, kHitPat);
  Bool_t is_T1_fired = false, is_T2_fired = false;
  Bool_t is_BH2_fired = false;
  // T1 -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("T1");
    static const Int_t k_tdc    = gUnpacker.get_data_id("T1","tdc");

    // sequential id
    static const Int_t t_id   = gHist.getSequentialID(kT1, 0, kTDC);
    static const Int_t m_id   = gHist.getSequentialID(kT1, 0, kMulti);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcT1", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcT1", 1);

    Int_t multiplicity = 0;
    Int_t seg = 0;
    // TDC
    auto nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);

    for(Int_t m = 0; m<nhit_t; ++m) {
      Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
      hptr_array[t_id + seg]->Fill(tdc);
      if (tdc_min < tdc && tdc < tdc_max) {
	is_T1_fired = true;
      }// tdc range is ok
    }// for(m)
    if (is_T1_fired) {
      ++multiplicity;
    }
    hptr_array[m_id]->Fill(multiplicity);
  }//T1
  // T2 -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("T2");
    static const Int_t k_tdc    = gUnpacker.get_data_id("T2","tdc");

    // sequential id
    static const Int_t t_id   = gHist.getSequentialID(kT2, 0, kTDC);
    static const Int_t m_id   = gHist.getSequentialID(kT2, 0, kMulti);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcT2", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcT2", 1);

    Int_t multiplicity = 0;
    Int_t seg = 0;
    // TDC
    auto nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);

    for(Int_t m = 0; m<nhit_t; ++m) {
      Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
      hptr_array[t_id + seg]->Fill(tdc);
      if (tdc_min < tdc && tdc < tdc_max) {
	is_T2_fired = true;
      }// tdc range is ok
    }// for(m)
    if (is_T2_fired) {
      ++multiplicity;
    }
    hptr_array[m_id]->Fill(multiplicity);
  }//T2

  // E42BH2  -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("E42BH2");
    static const Int_t k_adc    = gUnpacker.get_data_id("E42BH2","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("E42BH2","tdc");

    static const Int_t a_id   = gHist.getSequentialID(kE42BH2, 0, kADC);
    static const Int_t t_id   = gHist.getSequentialID(kE42BH2, 0, kTDC);
    static const Int_t awt_id = gHist.getSequentialID(kE42BH2, 0, kADCwTDC);
    static const Int_t h_id   = gHist.getSequentialID(kE42BH2, 0, kHitPat);
    static const Int_t m_id   = gHist.getSequentialID(kE42BH2, 0, kMulti);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcE42BH2", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcE42BH2", 1);

    std::vector<std::vector<Int_t>> hit_flag(NumOfSegE42BH2);
    for(Int_t seg = 0; seg<NumOfSegE42BH2; ++seg) {
      hit_flag[seg].resize(3);
      for(Int_t ud=0; ud<3; ++ud) {
	hit_flag[seg][ud] = 0;
      }
    }
    Int_t multiplicity = 0;
    Int_t seg = 4;
    for(Int_t ud=0; ud<2; ++ud) {
      // ADC
      UInt_t adc = 0;
      Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, ud, k_adc);
      if (nhit_a!=0) {
	adc = gUnpacker.get(k_device, 0, seg, ud, k_adc);
	hptr_array[a_id + ud]->Fill(adc);
      }
      // TDC
      Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, ud, k_tdc);

      for(Int_t m = 0; m<nhit_t; ++m) {
	Int_t tdc = gUnpacker.get(k_device, 0, seg, ud, k_tdc, m);
	hptr_array[t_id + ud]->Fill(tdc);

	if (tdc_min < tdc && tdc < tdc_max && adc > 0) {
	  hit_flag[seg][ud] = 1;
	}// tdc range is ok
      }// for(m)

      if (hit_flag[seg][ud] == 1) {
	// ADC w/TDC
	hptr_array[awt_id + ud]->Fill(adc);
      }
    }//ud

    for(Int_t seg = 0; seg<NumOfSegE42BH2; ++seg) {
      Int_t ud=2;
      // TDC
      Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, ud, k_tdc);

      for(Int_t m = 0; m<nhit_t; ++m) {
	Int_t tdc = gUnpacker.get(k_device, 0, seg, ud, k_tdc, m);
	hptr_array[t_id + seg + 2]->Fill(tdc);

	if (tdc_min < tdc && tdc < tdc_max) {
	  hit_flag[seg][ud] = 1;
	}// tdc range is ok
      }// for(m)

      if (hit_flag[seg][ud] == 1) {
	hptr_array[h_id]->Fill(seg);
	++multiplicity;
	if( seg>=2 && seg<=5) is_BH2_fired = true;
      }// flag is OK
    }//seg

    hptr_array[m_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//E42BH2

  // apply T1 & T2 & BH2[seg3-6] cut
#if 0
  if (!(is_T1_fired && is_T2_fired && is_BH2_fired)) return 0;
  hptr_array[e72para_id]->Fill(e72parasite::kT1);
  hptr_array[e72para_id]->Fill(e72parasite::kT2);
  hptr_array[e72para_id]->Fill(e72parasite::kE42BH2);
#endif

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif
  // E72BAC -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("E72BAC");
    static const Int_t k_adc    = gUnpacker.get_data_id("E72BAC","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("E72BAC","tdc");

    // sequential id
    static const Int_t a_id   = gHist.getSequentialID(kE72BAC, 0, kADC);
    static const Int_t t_id   = gHist.getSequentialID(kE72BAC, 0, kTDC);
    static const Int_t awt_id = gHist.getSequentialID(kE72BAC, 0, kADCwTDC);
    static const Int_t m_id   = gHist.getSequentialID(kE72BAC, 0, kMulti);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcE72BAC", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcE72BAC", 1);

    Int_t multiplicity = 0;
    Int_t seg = 0;
    // ADC
    auto nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
    if (nhit_a!=0) {
      Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
      hptr_array[a_id + seg]->Fill(adc);
    }
    // TDC
    auto nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
    Bool_t is_in_gate = false;

    for(Int_t m = 0; m<nhit_t; ++m) {
      Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
      hptr_array[t_id + seg]->Fill(tdc);

      if (tdc_min < tdc && tdc < tdc_max) {
	is_in_gate = true;
      }// tdc range is ok
    }// for(m)

    if (is_in_gate) {
      // ADC w/TDC
      if (gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0) {
	Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[awt_id + seg]->Fill(adc);
      }
      hptr_array[e72para_id]->Fill(e72parasite::kE72BAC);
      ++multiplicity;
    }// flag is OK
    hptr_array[m_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//E72BAC

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // E90SAC  -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("E90SAC");
    static const Int_t k_adc    = gUnpacker.get_data_id("E90SAC","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("E90SAC","tdc");

    static const Int_t a_id   = gHist.getSequentialID(kE90SAC, 0, kADC);
    static const Int_t t_id   = gHist.getSequentialID(kE90SAC, 0, kTDC);
    static const Int_t awt_id = gHist.getSequentialID(kE90SAC, 0, kADCwTDC);
    static const Int_t h_id   = gHist.getSequentialID(kE90SAC, 0, kHitPat);
    static const Int_t m6_id   = gHist.getSequentialID(kE90SAC, 0, kMulti, 1);
    static const Int_t m8_id   = gHist.getSequentialID(kE90SAC, 0, kMulti, 2);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcE90SAC", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcE90SAC", 1);

    Int_t multiplicity[2] = {0, 0};
    for(Int_t seg = 0; seg<NumOfSegE90SAC; ++seg) {
      // ADC
      Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      if (nhit_a!=0) {
	Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[a_id + seg]->Fill(adc);
      }
      // TDC
      Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      Bool_t is_in_gate = false;

      for(Int_t m = 0; m<nhit_t; ++m) {
	Int_t tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc, m);
	hptr_array[t_id + seg]->Fill(tdc);

	if (tdc_min < tdc && tdc < tdc_max) {
	  is_in_gate = true;
	}// tdc range is ok
      }// for(m)

      if (is_in_gate) {
	// ADC w/TDC
	if (gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0) {
	  Int_t adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	  hptr_array[awt_id + seg]->Fill(adc);
	}
	hptr_array[h_id]->Fill(seg);
	hptr_array[e72para_id]->Fill(e72parasite::kE90SAC1 + seg);
	++multiplicity[seg];
      }// flag is OK
    }

    hptr_array[m6_id]->Fill(multiplicity[0]);
    hptr_array[m8_id]->Fill(multiplicity[1]);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//E90SAC

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // E72KVC  -----------------------------------------------------------
  {
    // data type
    static const Int_t k_device = gUnpacker.get_device_id("E72KVC");
    static const Int_t k_adc    = gUnpacker.get_data_id("E72KVC","adc");
    static const Int_t k_tdc    = gUnpacker.get_data_id("E72KVC","tdc");

    static const Int_t a_id   = gHist.getSequentialID(kE72KVC, 0, kADC);
    static const Int_t t_id   = gHist.getSequentialID(kE72KVC, 0, kTDC);
    static const Int_t awt_id = gHist.getSequentialID(kE72KVC, 0, kADCwTDC);
    static const Int_t h_id   = gHist.getSequentialID(kE72KVC, 0, kHitPat);
    static const Int_t m_id   = gHist.getSequentialID(kE72KVC, 0, kMulti);
    static const Int_t h2_id   = gHist.getSequentialID(kE72KVC, 0, kHitPat, 2);
    static const Int_t m2_id   = gHist.getSequentialID(kE72KVC, 0, kMulti, 2);

    // TDC gate range
    static const Int_t tdc_min = gUser.GetParameter("TdcE72KVC", 0);
    static const Int_t tdc_max = gUser.GetParameter("TdcE72KVC", 1);

    std::vector<std::vector<Int_t>> hit_flag(NumOfSegE72KVC);
    Int_t multiplicity = 0, multiplicity_sum = 0;
    for(Int_t seg = 0; seg<NumOfSegE72KVC; ++seg) {
      hit_flag[seg].resize(3);
      for(Int_t ud=0; ud<3; ++ud) {
	hit_flag[seg][ud] = 0;
	// ADC
	UInt_t adc = 0;
	Int_t nhit_a = gUnpacker.get_entries(k_device, 0, seg, ud, k_adc);
	if (nhit_a!=0) {
	  adc = gUnpacker.get(k_device, 0, seg, ud, k_adc);
	  hptr_array[a_id + seg + ud*NumOfSegE72KVC]->Fill(adc);
	}
	// TDC
	Int_t nhit_t = gUnpacker.get_entries(k_device, 0, seg, ud, k_tdc);

	for(Int_t m = 0; m<nhit_t; ++m) {
	  Int_t tdc = gUnpacker.get(k_device, 0, seg, ud, k_tdc, m);
	  hptr_array[t_id + seg + ud*NumOfSegE72KVC]->Fill(tdc);

	  if (tdc_min < tdc && tdc < tdc_max && adc > 0) {
	    hit_flag[seg][ud] = 1;
	  }// tdc range is ok
	}// for(m)

	if (hit_flag[seg][ud] == 1) {
	  // ADC w/TDC
	  hptr_array[awt_id + seg +ud*NumOfSegE72KVC]->Fill(adc);
	}
      }//ud
      if (hit_flag[seg][kU] == 1 && hit_flag[seg][kD] == 1) {
	hptr_array[h_id]->Fill(seg);
	++multiplicity;
      }// flag is OK
      if (hit_flag[seg][2] == 1) {
	hptr_array[h2_id]->Fill(seg);
	++multiplicity_sum;
      }// flag is OK
    }//seg

    hptr_array[m_id]->Fill(multiplicity);
    hptr_array[m2_id]->Fill(multiplicity_sum);

    if (multiplicity) hptr_array[e72para_id]->Fill(e72parasite::kE72KVC);
    if (multiplicity_sum) hptr_array[e72para_id]->Fill(e72parasite::kE72KVCSUM);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//E72KVC
#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif
  return 0;
} //process_event()

} //analyzer
