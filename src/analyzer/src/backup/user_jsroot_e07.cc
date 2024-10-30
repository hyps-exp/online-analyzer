// -*- C++ -*-

// Author: Tomonori Takahashi

#include <iomanip>
#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <TCanvas.h>
#include <TGFileBrowser.h>
#include <TH1.h>
#include <TH2.h>
#include <THttpServer.h>
#include <TKey.h>
#include <TMath.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TText.h>
#include <TTimeStamp.h>

#include <user_analyzer.hh>
#include <Unpacker.hh>
#include <UnpackerManager.hh>
#include <DAQNode.hh>
#include <filesystem_util.hh>

#include "Controller.hh"
#include "Updater.hh"

#include "ConfMan.hh"
#include "DetectorID.hh"
#include "DCDriftParamMan.hh"
#include "DCGeomMan.hh"
#include "DCTdcCalibMan.hh"
#include "EMCParamMan.hh"
#include "HistMaker.hh"
#include "HodoParamMan.hh"
#include "HodoPHCMan.hh"
#include "HttpServer.hh"
#include "MacroBuilder.hh"
#include "MatrixParamMan.hh"
#include "MsTParamMan.hh"
#include "PsMaker.hh"
#include "SsdAnalyzer.hh"
#include "UserParamMan.hh"

#define DEBUG    0
#define FLAG_DAQ 1

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  namespace
  {
    std::vector<TH1*> hptr_array;
    EMCParamMan& gEMC  = EMCParamMan::GetInstance();
    HistMaker&   gHist = HistMaker::getInstance();
    HttpServer&  gHttp = HttpServer::GetInstance();
    const UserParamMan& gUser = UserParamMan::GetInstance();
    const Double_t emc_x_offset = 500000 - 303300;
    const Double_t emc_y_offset = 500000 + 164000;
    TText text;
    TText end;
    const TString comment_txt("/data1/e07_2017/misc/comment.txt");
  }

//____________________________________________________________________________
int
process_begin( const std::vector<std::string>& argv )
{
  // gROOT->SetBatch(kTRUE);
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
  gConfMan.InitializeParameter<EMCParamMan>("EMC");
  gConfMan.InitializeParameter<HodoParamMan>("HDPRM");
  gConfMan.InitializeParameter<DCGeomMan>("DCGEOM");
  gConfMan.InitializeParameter<DCTdcCalibMan>("TDCCALIB");
  gConfMan.InitializeParameter<DCDriftParamMan>("DRFTPM");
  gConfMan.InitializeParameter<HodoParamMan>("HDPRM");
  gConfMan.InitializeParameter<HodoPHCMan>("HDPHC");
  gConfMan.InitializeParameter<UserParamMan>("USER");
  if( !gConfMan.IsGood() ) return -1;
  // unpacker and all the parameter managers are initialized at this stage

  gHttp.SetPort(9090);
  gHttp.Open();
  gHttp.Register(gHist.createBH1());
  gHttp.Register(gHist.createBFT());
  gHttp.Register(gHist.createBC3());
  gHttp.Register(gHist.createBC4());
  gHttp.Register(gHist.createBH2());
  gHttp.Register(gHist.createBAC());
  gHttp.Register(gHist.createFBH());
  gHttp.Register(gHist.createPVAC());
  gHttp.Register(gHist.createFAC());
  gHttp.Register(gHist.createSCH());
  gHttp.Register(gHist.createSDC1());
  gHttp.Register(gHist.createSDC2());
  gHttp.Register(gHist.createSDC3());
  gHttp.Register(gHist.createTOF());
  gHttp.Register(gHist.createMsT());
  gHttp.Register(gHist.createMtx3D());
  gHttp.Register(gHist.createTriggerFlag());
  gHttp.Register(gHist.createCorrelation());
  gHttp.Register(gHist.createDAQ(false));
  gHttp.Register(gHist.createTimeStamp(false));
  gHttp.Register(gHist.createDCEff());
  gHttp.Register(gHist.createEMC());
  gHttp.Register(gHist.createSSDT());
  gHttp.Register(gHist.createSSD1());
  gHttp.Register(gHist.createSSD2());
  {
    int btof_id = gHist.getUniqueID(kMisc, 0, kTDC);
    gHttp.Register( gHist.createTH1(btof_id, "BTOF",
				    300, -10, 5,
				    "BTOF [ns]", "") );
  }

  if(0 != gHist.setHistPtr(hptr_array)){ return -1; }

  // Macro for HttpServer
  gHttp.Register(http::BH1ADC());
  gHttp.Register(http::BH1TDC());
  gHttp.Register(http::BFT());
  gHttp.Register(http::BH2());
  gHttp.Register(http::ACs());
  gHttp.Register(http::FBHTDC());
  gHttp.Register(http::FBHTOT());
  gHttp.Register(http::FBHHitMulti());
  gHttp.Register(http::SCHTDC());
  gHttp.Register(http::SCHTOT());
  gHttp.Register(http::SCHHitMulti());
  gHttp.Register(http::TOFADC());
  gHttp.Register(http::TOFTDC());
  gHttp.Register(http::MsTTDC());
  gHttp.Register(http::BC3TDC());
  gHttp.Register(http::BC3HitMulti());
  gHttp.Register(http::BC4TDC());
  gHttp.Register(http::BC4HitMulti());
  gHttp.Register(http::SDC1TDC());
  gHttp.Register(http::SDC1HitMulti());
  gHttp.Register(http::SDC2TDC());
  gHttp.Register(http::SDC2HitMulti());
  gHttp.Register(http::SDC3TDC());
  gHttp.Register(http::SDC3HitMulti());
  gHttp.Register(http::BcOutEfficiency());
  gHttp.Register(http::SdcInOutEfficiency());
  gHttp.Register(http::HitPattern());

  gHttp.Register(http::EMC());
  {
    ((TCanvas*)gROOT->FindObject("EMC"))->cd();
    text.SetNDC();
    text.SetTextSize(0.040);
    text.Draw();
    end.SetNDC();
    end.SetTextSize(0.040);
    end.Draw();
  }
  gHttp.Register(http::SSD1ADCTDC());
  gHttp.Register(http::SSD2ADCTDC());
  gHttp.Register(http::SSD1HitMulti());
  gHttp.Register(http::SSD2HitMulti());
  gHttp.Register(http::SSD1CHitMulti());
  gHttp.Register(http::SSD2CHitMulti());
  gHttp.Register(http::SSD1dETime());
  gHttp.Register(http::SSD2dETime());
  gHttp.Register(http::SSD12Chisqr());

  gHttp.Register(http::SSDEfficiency());

  for( Int_t i=0, n=hptr_array.size(); i<n; ++i ){
    hptr_array[i]->SetDirectory(0);
  }

  return 0;
}

//____________________________________________________________________________
int
process_end( void )
{
  hptr_array.clear();
  return 0;
}

//____________________________________________________________________________
int
process_event( void )
{
  static UnpackerManager& gUnpacker = GUnpacker::get_instance();
  static HistMaker&       gHist     = HistMaker::getInstance();
  static MatrixParamMan&  gMatrix   = MatrixParamMan::GetInstance();
  static MsTParamMan&     gMsT      = MsTParamMan::GetInstance();

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  static const Int_t nspill = gEMC.NSpill();
  static Int_t spill  = 0;
  static Int_t rspill = 0;
  static int run_number = -1;
  if( run_number != gUnpacker.get_root()->get_run_number() ){
    for( Int_t i=0, n=hptr_array.size(); i<n; ++i ){
      if( i==gHist.getSequentialID(kEMC, 0, kXYpos) )
	continue;
      hptr_array[i]->Reset();
    }
    run_number = gUnpacker.get_root()->get_run_number();

    std::ifstream ifs( comment_txt );
    TString line;
    while( ifs.good() && line.ReadLine(ifs) ){
      if( line.IsNull() || line[0]=='#' ) continue;
      std::istringstream iss( line.Data() );
      TString buf[5];
      iss >> buf[0] >> buf[1] >> buf[2] >> buf[3] >> buf[4];
      TString num = buf[4];
      num.ReplaceAll("]","");
      line.Remove(0,40);
      if( run_number == num.Atoi() ){
	static const int xypos_id = gHist.getSequentialID(kEMC, 0, kXYpos);
      	hptr_array[xypos_id]->SetTitle( line );
	break;
      }
    }
  }

  // TriggerFlag ---------------------------------------------------
  std::bitset<NumOfSegTFlag> trigger_flag;
  bool matrix2d_flag = false;
  bool matrix3d_flag = false;
  {
    static const int k_device = gUnpacker.get_device_id("TFlag");
    static const int k_tdc    = gUnpacker.get_data_id("TFlag", "tdc");

    static const int tf_tdc_id = gHist.getSequentialID( kTriggerFlag, 0, kTDC );
    static const int tf_hit_id = gHist.getSequentialID( kTriggerFlag, 0, kHitPat );
    for( int seg=0; seg<NumOfSegTFlag; ++seg ){
      int nhit = gUnpacker.get_entries( k_device, 0, seg, 0, k_tdc );
      if( nhit>0 ){
	int tdc = gUnpacker.get( k_device, 0, seg, 0, k_tdc );
	if( tdc>0 ){
	  trigger_flag.set(seg);
	  hptr_array[tf_tdc_id+seg]->Fill( tdc );
	  hptr_array[tf_hit_id]->Fill( seg );
	  if( seg==8 ) matrix2d_flag = true;
	  if( seg==9 ) matrix3d_flag = true;
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

  // TimeStamp --------------------------------------------------------
  {
    static const int k_device = gUnpacker.get_device_id("VME-RM");
    static const int k_time   = gUnpacker.get_data_id("VME-RM", "time");
    static const int hist_id  = gHist.getSequentialID(kTimeStamp, 0, kTDC);
    int time0 = 0;
    for( int i=0; i<NumOfVmeRm; ++i ){
      int nhit = gUnpacker.get_entries( k_device, i, 0, 0, k_time );
      if( nhit>0 ){
	int time = gUnpacker.get( k_device, i, 0, 0, k_time );
	if( i==0 ) time0 = time;
	TH1* h = dynamic_cast<TH1*>(hptr_array[hist_id +i]);
	h->Fill( time - time0 );
      }
    }
  }

  if( trigger_flag[SpillEndFlag] ) return 0;

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
    int bh1a_id   = gHist.getSequentialID(kBH1, 0, kADC);
    int bh1t_id   = gHist.getSequentialID(kBH1, 0, kTDC);
    int bh1awt_id = gHist.getSequentialID(kBH1, 0, kADCwTDC);
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
	if(tdc!=0){
	  hptr_array[bh1t_id + seg]->Fill(tdc);
	  // ADC wTDC
	  if( gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc)>0 ){
	    unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	    hptr_array[bh1awt_id + seg]->Fill( adc );
	  }
	}
      }
    }

    // Down PMT
    bh1a_id   = gHist.getSequentialID(kBH1, 0, kADC, NumOfSegBH1+1);
    bh1t_id   = gHist.getSequentialID(kBH1, 0, kTDC, NumOfSegBH1+1);
    bh1awt_id = gHist.getSequentialID(kBH1, 0, kADCwTDC, NumOfSegBH1+1);
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
	if( tdc!=0 ){
	  hptr_array[bh1t_id + seg]->Fill(tdc);
	  // ADC w/TDC
	  if( gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc)>0 ){
	    unsigned int adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	    hptr_array[bh1awt_id + seg]->Fill(adc);
	  }
	}
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
    static const int tdc_min = gUser.GetParameter("BFT_TDC", 0);
    static const int tdc_max = gUser.GetParameter("BFT_TDC", 1);

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
    static const int tdc_min = gUser.GetParameter("BC3_TDC", 0);
    static const int tdc_max = gUser.GetParameter("BC3_TDC", 1);

    // sequential id
    static const int bc3t_id    = gHist.getSequentialID(kBC3, 0, kTDC);
    static const int bc3t1st_id = gHist.getSequentialID(kBC3, 0, kTDC2D);
    static const int bc3hit_id  = gHist.getSequentialID(kBC3, 0, kHitPat);
    static const int bc3mul_id  = gHist.getSequentialID(kBC3, 0, kMulti);
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
	bool flag_hit_wt = false;
	int  tdc1st = 0;
	for(int m = 0; m<nhit; ++m){
	  int tdc = gUnpacker.get(k_device, l, 0, w, k_tdc, m);
	  hptr_array[bc3t_id + l]->Fill(tdc);
	  if( tdc1st<tdc ) tdc1st = tdc;
	  // Drift time check
	  if(tdc_min < tdc && tdc < tdc_max){
	    hptr_array[bc3hit_id + l]->Fill(w);
	    flag_hit_wt = true;
	  }
	}

	if( tdc1st!=0 ) hptr_array[bc3t1st_id + l]->Fill(tdc1st);
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
    static const int tdc_min = gUser.GetParameter("BC4_TDC", 0);
    static const int tdc_max = gUser.GetParameter("BC4_TDC", 1);

    // sequential id
    static const int bc4t_id    = gHist.getSequentialID(kBC4, 0, kTDC);
    static const int bc4t1st_id = gHist.getSequentialID(kBC4, 0, kTDC2D);
    static const int bc4hit_id  = gHist.getSequentialID(kBC4, 0, kHitPat);
    static const int bc4mul_id  = gHist.getSequentialID(kBC4, 0, kMulti);
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

	bool flag_hit_wt = false;
	int  tdc1st = 0;
	for(int m = 0; m<nhit; ++m){
	  int tdc = gUnpacker.get(k_device, l, 0, w, k_tdc, m);
	  hptr_array[bc4t_id + l]->Fill(tdc);
	  if( tdc1st<tdc ) tdc1st = tdc;
	  // Drift time check
	  if(tdc_min < tdc && tdc < tdc_max){
	    hptr_array[bc4hit_id + l]->Fill(w);
	    flag_hit_wt = true;
	  }
	}

	if( tdc1st!=0 ) hptr_array[bc4t1st_id + l]->Fill(tdc1st);
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

  // BH2 -----------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("BH2");
    static const int k_u      = 0; // up
    static const int k_d      = 1; // down
    static const int k_adc    = gUnpacker.get_data_id("BH2", "adc");
    static const int k_tdc    = gUnpacker.get_data_id("BH2", "tdc");

    // UP
    int bh2a_id   = gHist.getSequentialID(kBH2, 0, kADC);
    int bh2t_id   = gHist.getSequentialID(kBH2, 0, kTDC);
    int bh2awt_id = gHist.getSequentialID(kBH2, 0, kADCwTDC);
    for(int seg=0; seg<NumOfSegBH2; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc);
      if(nhit != 0){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	hptr_array[bh2a_id + seg]->Fill(adc);
      }
      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      if( nhit!=0 ){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	if( tdc!=0 ){
	  hptr_array[bh2t_id + seg]->Fill(tdc);
	  // ADC w/TDC
	  if( gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc)>0 ){
	    unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	    hptr_array[bh2awt_id + seg]->Fill(adc);
	  }
	}
      }
    }

    // DOWN
    bh2a_id   = gHist.getSequentialID(kBH2, 0, kADC, NumOfSegBH2+1);
    bh2t_id   = gHist.getSequentialID(kBH2, 0, kTDC, NumOfSegBH2+1);
    bh2awt_id = gHist.getSequentialID(kBH2, 0, kADCwTDC, NumOfSegBH2+1);
    for(int seg=0; seg<NumOfSegBH2; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc);
      if(nhit != 0){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	hptr_array[bh2a_id + seg]->Fill(adc);
      }
      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      if( nhit!=0 ){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
	if( tdc!=0 ){
	  hptr_array[bh2t_id + seg]->Fill(tdc);
	  // ADC w/TDC
	  if( gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc)>0 ){
	    unsigned int adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	    hptr_array[bh2awt_id + seg]->Fill( adc );
	  }
	}
      }
    }

    // Hit pattern &&  Multiplicity
    static const int bh2hit_id = gHist.getSequentialID(kBH2, 0, kHitPat);
    static const int bh2mul_id = gHist.getSequentialID(kBH2, 0, kMulti);
    int multiplicity = 0;
    for(int seg=0; seg<NumOfSegBH2; ++seg){
      int nhit_u = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      int nhit_d = gUnpacker.get_entries(k_device, 0, seg, k_d, k_tdc);
      // AND
      if( nhit_u!=0 && nhit_d!=0 ){
	unsigned int tdc_u = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	unsigned int tdc_d = gUnpacker.get(k_device, 0, seg, k_d, k_tdc);
	// TDC AND
	if( tdc_u!=0 && tdc_d!=0 ){
	  hptr_array[bh2hit_id]->Fill( seg );
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

  // BAC ---------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("BAC");
    static const int k_adc    = gUnpacker.get_data_id("BAC","adc");
    static const int k_tdc    = gUnpacker.get_data_id("BAC","tdc");

    // sequential id
    static const int baca_id   = gHist.getSequentialID(kBAC, 0, kADC,     1);
    static const int bact_id   = gHist.getSequentialID(kBAC, 0, kTDC,     1);
    static const int bacawt_id = gHist.getSequentialID(kBAC, 0, kADCwTDC, 1);
    static const int bach_id   = gHist.getSequentialID(kBAC, 0, kHitPat,  1);
    static const int bacm_id   = gHist.getSequentialID(kBAC, 0, kMulti,   1);

    int multiplicity = 0;
    for(int seg = 0; seg<NumOfSegBAC; ++seg){
      // ADC
      int nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      if( nhit_a!=0 ){
	int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[baca_id + seg]->Fill( adc );
      }
      // TDC
      int nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      if( nhit_t!=0 ){
	int tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc);
	if( tdc!=0 ){
	  hptr_array[bact_id + seg]->Fill( tdc );
	  // ADC w/TDC
	  if( gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0 ){
	    int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	    hptr_array[bacawt_id + seg]->Fill( adc );
	  }
	  hptr_array[bach_id]->Fill(seg);
	  ++multiplicity;
	}
      }
    }

    hptr_array[bacm_id]->Fill( multiplicity );

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // FBH -----------------------------------------------------------
  {
    // data type
    static const int k_device   = gUnpacker.get_device_id("FBH");
    static const int k_leading  = gUnpacker.get_data_id("FBH", "leading");
    static const int k_trailing = gUnpacker.get_data_id("FBH", "trailing");

    static const int k_u = 0;
    static const int k_d = 1;

    // TDC gate range
    static const int tdc_min = gUser.GetParameter("FBH_TDC", 0);
    static const int tdc_max = gUser.GetParameter("FBH_TDC", 1);

    // sequential id
    static const int fbh_tdc_u_id = gHist.getSequentialID(kFBH, 0, kTDC,    1);
    static const int fbh_tot_u_id = gHist.getSequentialID(kFBH, 0, kADC,    1);
    static const int fbh_tdc_d_id = gHist.getSequentialID(kFBH, 0, kTDC,    NumOfSegFBH +1);
    static const int fbh_tot_d_id = gHist.getSequentialID(kFBH, 0, kADC,    NumOfSegFBH +1);
    static const int fbh_t_all_id   = gHist.getSequentialID(kFBH, 0, kTDC, NumOfSegFBH*2+1);
    static const int fbh_tot_all_id = gHist.getSequentialID(kFBH, 0, kADC, NumOfSegFBH*2+1);
    static const int fbh_hit_id   = gHist.getSequentialID(kFBH, 0, kHitPat, 1);
    static const int fbh_mul_id   = gHist.getSequentialID(kFBH, 0, kMulti,  1);

    static const int fbh_t_2d_id   = gHist.getSequentialID(kFBH, 0, kTDC2D, 1);
    static const int fbh_tot_2d_id = gHist.getSequentialID(kFBH, 0, kADC2D, 1);

    int multiplicity  = 0;
    int hit_flag_u[NumOfSegFBH] = {};
    int hit_flag_d[NumOfSegFBH] = {};
    for( int i=0; i<NumOfSegFBH; ++i ){
      int nhit_u = gUnpacker.get_entries(k_device, 0, i, k_u, k_leading);
      int nhit_d = gUnpacker.get_entries(k_device, 0, i, k_d, k_leading);
      for(int m=0; m<nhit_u; ++m){
	int tdc_u      = gUnpacker.get(k_device, 0, i, k_u, k_leading,  m);
	int trailing_u = gUnpacker.get(k_device, 0, i, k_u, k_trailing, m);
	int tot_u      = tdc_u - trailing_u;
	hptr_array[fbh_tdc_u_id +i]->Fill(tdc_u);
	hptr_array[fbh_tot_u_id +i]->Fill(tot_u);
	hptr_array[fbh_t_all_id]->Fill( tdc_u );
	hptr_array[fbh_tot_all_id]->Fill( tot_u );
	hptr_array[fbh_t_2d_id]->Fill(i, tdc_u);
	hptr_array[fbh_tot_2d_id]->Fill(i, tot_u);
	if( tdc_min<tdc_u && tdc_u<tdc_max ){
	  hit_flag_u[i] = true;
	}
      }
      for(int m=0; m<nhit_d; ++m){
	int tdc_d      = gUnpacker.get(k_device, 0, i, k_d, k_leading,  m);
	int trailing_d = gUnpacker.get(k_device, 0, i, k_d, k_trailing, m);
	int tot_d      = tdc_d - trailing_d;
	hptr_array[fbh_tdc_d_id +i]->Fill(tdc_d);
	hptr_array[fbh_tot_d_id +i]->Fill(tot_d);
	hptr_array[fbh_t_all_id]->Fill( tdc_d );
	hptr_array[fbh_tot_all_id]->Fill( tot_d );
	hptr_array[fbh_t_2d_id]->Fill(i +NumOfSegFBH, tdc_d);
	hptr_array[fbh_tot_2d_id]->Fill(i +NumOfSegFBH, tot_d);
	if( tdc_min<tdc_d && tdc_d<tdc_max ){
	  hit_flag_d[i] = true;
	}
      }
      if( hit_flag_u[i] && hit_flag_d[i] ){
	hptr_array[fbh_hit_id]->Fill( i );
	++multiplicity;
      }
    }

    hptr_array[fbh_mul_id]->Fill( multiplicity );

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // SSD _______________________________________________________________________
  {
    SsdAnalyzer ssdAna;
    ssdAna.Calculate();
    static const Double_t MinDe     = 2500.;
    static const Double_t MaxChisqr =  300.;

    {
      static const int id = gHist.getSequentialID(kSSDT, 0, kTDC, 1);
      for(int seg=0; seg<NumOfSegSSDT; ++seg){
	int tdc = ssdAna.GetSSDT(seg);
	if(tdc>0) hptr_array[id+seg]->Fill( tdc );
      }
    }

    // SSD1 ---------------------------------------------------------
    {
      // sequential id
      static const int adc_id = gHist.getSequentialID(kSSD1, 0, kADC2D,  1);
      static const int tdc_id = gHist.getSequentialID(kSSD1, 0, kTDC2D,  1);
      static const int hit_id = gHist.getSequentialID(kSSD1, 0, kHitPat, 1);
      static const int mul_id = gHist.getSequentialID(kSSD1, 0, kMulti,  1);
      static const int de_id = gHist.getSequentialID(kSSD1, 0, kDeltaE, 1);
      static const int ct_id = gHist.getSequentialID(kSSD1, 0, kCTime, 1);
      static const int de2d_id = gHist.getSequentialID(kSSD1, 0, kDeltaE2D, 1);
      static const int ct2d_id = gHist.getSequentialID(kSSD1, 0, kCTime2D, 1);
      static const int chi2_id = gHist.getSequentialID(kSSD1, 0, kChisqr, 1);
      bool chit_flag[NumOfLayersSSD1][NumOfSegSSD1];
      for(int l=0; l<NumOfLayersSSD1; ++l){
	int  multiplicity = 0;
	int cmultiplicity = 0;
	for(int seg=0; seg<NumOfSegSSD1; ++seg){
	  chit_flag[l][seg] = false;
	  // ADC
	  Double_t adc = ssdAna.GetAdc(l,seg);
	  Double_t tdc = ssdAna.GetTdc(l,seg);
	  if( adc>=0 && tdc>=0){
	    hptr_array[adc_id +l]->Fill( seg, adc );
	    hptr_array[tdc_id +l]->Fill( seg, tdc );
	    hptr_array[hit_id +2*l]->Fill( seg );
	    multiplicity++;
	    Double_t de     = ssdAna.GetDe(l,seg);
	    Double_t chisqr = ssdAna.GetChisqr(l,seg);
	    Double_t time   = ssdAna.GetTime(l,seg);
	    chit_flag[l][seg] = ( de>MinDe && chisqr<MaxChisqr );
	    if( chit_flag[l][seg] ){
	      hptr_array[hit_id +2*l+1]->Fill( seg );
	      hptr_array[de_id +l]->Fill( de );
	      hptr_array[ct_id +l]->Fill( time );
	      hptr_array[de2d_id +l]->Fill( seg, de );
	      hptr_array[ct2d_id +l]->Fill( seg, time );
	      hptr_array[chi2_id +l]->Fill( chisqr );
	      cmultiplicity++;
	    }
	  }
	}//for(seg)
	hptr_array[mul_id +2*l]->Fill( multiplicity );
	hptr_array[mul_id +2*l+1]->Fill( cmultiplicity );
      }//for(l)
      // Correlation XY
      static const int hit2d_id = gHist.getSequentialID(kSSD1, 0, kHitPat2D, 1);
      for(int x_seg=0; x_seg<NumOfSegSSD1; ++x_seg){
	if( !chit_flag[1][x_seg] ) continue;
	for(int y_seg=0; y_seg<NumOfSegSSD1; ++y_seg){
	  if( !chit_flag[0][y_seg] ) continue;
	  hptr_array[hit2d_id]->Fill( x_seg, y_seg );
	}
      }
      for(int x_seg=0; x_seg<NumOfSegSSD1; ++x_seg){
	if( !chit_flag[3][x_seg] ) continue;
	for(int y_seg=0; y_seg<NumOfSegSSD1; ++y_seg){
	  if( !chit_flag[2][y_seg] ) continue;
	  hptr_array[hit2d_id+1]->Fill( x_seg, y_seg );
	}
      }
    }

    // SSD1 ---------------------------------------------------------
    {
      static const int adc_id = gHist.getSequentialID(kSSD2, 0, kADC2D,  1);
      static const int tdc_id = gHist.getSequentialID(kSSD2, 0, kTDC2D,  1);
      static const int hit_id = gHist.getSequentialID(kSSD2, 0, kHitPat, 1);
      static const int mul_id = gHist.getSequentialID(kSSD2, 0, kMulti,  1);
      static const int de_id = gHist.getSequentialID(kSSD2, 0, kDeltaE, 1);
      static const int ct_id = gHist.getSequentialID(kSSD2, 0, kCTime, 1);
      static const int de2d_id = gHist.getSequentialID(kSSD2, 0, kDeltaE2D, 1);
      static const int ct2d_id = gHist.getSequentialID(kSSD2, 0, kCTime2D, 1);
      static const int chi2_id = gHist.getSequentialID(kSSD2, 0, kChisqr, 1);
      bool chit_flag[NumOfLayersSSD2][NumOfSegSSD2];
      for(int l=0; l<NumOfLayersSSD2; ++l){
	int  multiplicity = 0;
	int cmultiplicity = 0;
	for(int seg=0; seg<NumOfSegSSD2; ++seg){
	  chit_flag[l][seg] = false;
	  // ADC
	  Double_t adc = ssdAna.GetAdc(l+NumOfLayersSSD1,seg);
	  Double_t tdc = ssdAna.GetTdc(l+NumOfLayersSSD1,seg);
	  if( adc>=0 && tdc>=0){
	    hptr_array[adc_id +l]->Fill( seg, adc );
	    hptr_array[tdc_id +l]->Fill( seg, tdc );
	    hptr_array[hit_id +2*l]->Fill( seg );
	    multiplicity++;
	    Double_t de     = ssdAna.GetDe(l+NumOfLayersSSD1,seg);
	    Double_t chisqr = ssdAna.GetChisqr(l+NumOfLayersSSD1,seg);
	    Double_t time   = ssdAna.GetTime(l+NumOfLayersSSD1,seg);
	    chit_flag[l][seg] = ( de>MinDe && chisqr<MaxChisqr );
	    if( chit_flag[l][seg] ){
	      hptr_array[hit_id +2*l+1]->Fill( seg );
	      hptr_array[de_id +l]->Fill( de );
	      hptr_array[ct_id +l]->Fill( time );
	      hptr_array[de2d_id +l]->Fill( seg, de );
	      hptr_array[ct2d_id +l]->Fill( seg, time );
	      hptr_array[chi2_id +l]->Fill( chisqr );
	      cmultiplicity++;
	    }
	  }
	}//for(seg)
	hptr_array[mul_id +2*l]->Fill( multiplicity );
	hptr_array[mul_id +2*l+1]->Fill( cmultiplicity );
      }//for(l)
      // Correlation XY
      static const int hit2d_id = gHist.getSequentialID(kSSD2, 0, kHitPat2D, 1);
      for(int x_seg=0; x_seg<NumOfSegSSD2; ++x_seg){
	if( !chit_flag[0][x_seg] ) continue;
	for(int y_seg=0; y_seg<NumOfSegSSD2; ++y_seg){
	  if( !chit_flag[1][y_seg] ) continue;
	  hptr_array[hit2d_id]->Fill( x_seg, y_seg );
	}
      }
      for(int x_seg=0; x_seg<NumOfSegSSD2; ++x_seg){
	if( !chit_flag[2][x_seg] ) continue;
	for(int y_seg=0; y_seg<NumOfSegSSD2; ++y_seg){
	  if( !chit_flag[3][y_seg] ) continue;
	  hptr_array[hit2d_id+1]->Fill( x_seg, y_seg );
	}
      }
    }
  }
#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // PVAC ---------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("PVAC");
    static const int k_adc    = gUnpacker.get_data_id("PVAC","adc");
    static const int k_tdc    = gUnpacker.get_data_id("PVAC","tdc");

    // sequential id
    static const int pvaca_id   = gHist.getSequentialID(kPVAC, 0, kADC,     1);
    static const int pvact_id   = gHist.getSequentialID(kPVAC, 0, kTDC,     1);
    static const int pvacawt_id = gHist.getSequentialID(kPVAC, 0, kADCwTDC, 1);
    static const int pvach_id   = gHist.getSequentialID(kPVAC, 0, kHitPat,  1);
    static const int pvacm_id   = gHist.getSequentialID(kPVAC, 0, kMulti,   1);

    int multiplicity = 0;
    for(int seg = 0; seg<NumOfSegPVAC; ++seg){
      // ADC
      int nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      if(nhit_a != 0){
	int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[pvaca_id + seg]->Fill(adc, nhit_a);
      }
      // TDC
      int nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      if( nhit_t!=0 ){
	int tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc);
	if( tdc!=0 ){
	  hptr_array[pvact_id + seg]->Fill( tdc );
	  // ADC w/TDC
	  if( gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0 ){
	    int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	    hptr_array[pvacawt_id + seg]->Fill( adc );
	  }
	  hptr_array[pvach_id]->Fill( seg );
	  ++multiplicity;
	}
      }
    }
    hptr_array[pvacm_id]->Fill( multiplicity );

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // FAC ---------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("FAC");
    static const int k_adc    = gUnpacker.get_data_id("FAC","adc");
    static const int k_tdc    = gUnpacker.get_data_id("FAC","tdc");

    // sequential id
    static const int faca_id = gHist.getSequentialID(kFAC, 0, kADC,       1);
    static const int fact_id = gHist.getSequentialID(kFAC, 0, kTDC,       1);
    static const int facawt_id = gHist.getSequentialID(kFAC, 0, kADCwTDC, 1);
    static const int fach_id = gHist.getSequentialID(kFAC, 0, kHitPat,    1);
    static const int facm_id = gHist.getSequentialID(kFAC, 0, kMulti,     1);

    int multiplicity = 0;
    for(int seg = 0; seg<NumOfSegFAC; ++seg){
      // ADC
      int nhit_a = gUnpacker.get_entries(k_device, 0, seg, 0, k_adc);
      if( nhit_a!=0 ){
	int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	hptr_array[faca_id + seg]->Fill( adc );
      }
      // TDC
      int nhit_t = gUnpacker.get_entries(k_device, 0, seg, 0, k_tdc);
      if( nhit_t!=0 ){
	int tdc = gUnpacker.get(k_device, 0, seg, 0, k_tdc);
	if( tdc!=0 ){
	  hptr_array[fact_id + seg]->Fill(tdc, nhit_t);
	  // ADC w/TDC
	  if( gUnpacker.get_entries(k_device, 0, seg, 0, k_adc)>0 ){
	    int adc = gUnpacker.get(k_device, 0, seg, 0, k_adc);
	    hptr_array[facawt_id + seg]->Fill( adc );
	  }
	  hptr_array[fach_id]->Fill( seg );
	  ++multiplicity;
	}
      }
    }

    hptr_array[facm_id]->Fill( multiplicity );

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // SCH -----------------------------------------------------------
  {
    // data type
    static const int k_device   = gUnpacker.get_device_id("SCH");
    static const int k_leading  = gUnpacker.get_data_id("SCH", "leading");
    static const int k_trailing = gUnpacker.get_data_id("SCH", "trailing");

    // TDC gate range
    static const int tdc_min = gUser.GetParameter("SCH_TDC", 0);
    static const int tdc_max = gUser.GetParameter("SCH_TDC", 1);

    // sequential id
    static const int sch_t_id    = gHist.getSequentialID(kSCH, 0, kTDC,      1);
    static const int sch_tot_id  = gHist.getSequentialID(kSCH, 0, kADC,      1);
    static const int sch_t_all_id   = gHist.getSequentialID(kSCH, 0, kTDC, NumOfSegSCH+1);
    static const int sch_tot_all_id = gHist.getSequentialID(kSCH, 0, kADC, NumOfSegSCH+1);
    static const int sch_hit_id  = gHist.getSequentialID(kSCH, 0, kHitPat,   1);
    static const int sch_mul_id  = gHist.getSequentialID(kSCH, 0, kMulti,    1);

    static const int sch_t_2d_id   = gHist.getSequentialID(kSCH, 0, kTDC2D,  1);
    static const int sch_tot_2d_id = gHist.getSequentialID(kSCH, 0, kADC2D,  1);

    int multiplicity  = 0;
    for( int i=0; i<NumOfSegSCH; ++i ){
      int nhit = gUnpacker.get_entries(k_device, 0, i, 0, k_leading);

      for(int m = 0; m<nhit; ++m){
	int tdc      = gUnpacker.get(k_device, 0, i, 0, k_leading,  m);
	int trailing = gUnpacker.get(k_device, 0, i, 0, k_trailing, m);
	int tot      = tdc - trailing;
	hptr_array[sch_t_id +i]->Fill(tdc);
	hptr_array[sch_t_all_id]->Fill(tdc);
	hptr_array[sch_tot_id +i]->Fill(tot);
	hptr_array[sch_tot_all_id]->Fill(tot);
	hptr_array[sch_t_2d_id]->Fill(i, tdc);
	hptr_array[sch_tot_2d_id]->Fill(i, tot);
	if( tdc_min<tdc && tdc<tdc_max ){
	  ++multiplicity;
	  hptr_array[sch_hit_id]->Fill(i);
	}
      }
    }
    hptr_array[sch_mul_id]->Fill(multiplicity);

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  // SDC1 ------------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("SDC1");
    static const int k_tdc    = 0;

    // TDC gate range
    static const int tdc_min = gUser.GetParameter("SDC1_TDC", 0);
    static const int tdc_max = gUser.GetParameter("SDC1_TDC", 1);

    // sequential id
    static const int sdc1t_id    = gHist.getSequentialID(kSDC1, 0, kTDC, 1);
    static const int sdc1t1st_id = gHist.getSequentialID(kSDC1, 0, kTDC2D, 1);
    static const int sdc1hit_id  = gHist.getSequentialID(kSDC1, 0, kHitPat, 1);
    static const int sdc1mul_id  = gHist.getSequentialID(kSDC1, 0, kMulti, 1);
    static const int sdc1mulwt_id
      = gHist.getSequentialID(kSDC1, 0, kMulti, 1+NumOfLayersSDC1);

    // TDC & HitPat & Multi
    for(int l = 0; l<NumOfLayersSDC1; ++l){
      int multiplicity    = 0;
      int multiplicity_wt = 0;
      for(int w = 0; w<NumOfWireSDC1; ++w){
	int nhit = gUnpacker.get_entries(k_device, l, 0, w, k_tdc);
	if( nhit==0 ) continue;

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[sdc1hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	int  tdc1st = 0;
	for( int m=0; m<nhit; ++m ){
	  int tdc = gUnpacker.get(k_device, l, 0, w, k_tdc, m);
	  hptr_array[sdc1t_id + l]->Fill(tdc);
	  if( tdc1st<tdc ) tdc1st = tdc;

	  // Drift time check
	  if( tdc_min<tdc && tdc<tdc_max ){
	    flag_hit_wt = true;
	  }
	}

	if( tdc1st!=0 ) hptr_array[sdc1t1st_id + l]->Fill(tdc1st);
	if( flag_hit_wt ){
	  ++multiplicity_wt;
	  hptr_array[sdc1hit_id + l]->Fill( w );
	}
      }

      hptr_array[sdc1mul_id + l]->Fill(multiplicity);
      hptr_array[sdc1mulwt_id + l]->Fill(multiplicity_wt);
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device,0);
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
    static const int tdc_min = gUser.GetParameter("SDC2_TDC", 0);
    static const int tdc_max = gUser.GetParameter("SDC2_TDC", 1);

    // sequential id
    static const int sdc2t_id    = gHist.getSequentialID(kSDC2, 0, kTDC);
    static const int sdc2t1st_id = gHist.getSequentialID(kSDC2, 0, kTDC2D);
    static const int sdc2hit_id  = gHist.getSequentialID(kSDC2, 0, kHitPat);
    static const int sdc2mul_id  = gHist.getSequentialID(kSDC2, 0, kMulti);
    static const int sdc2mulwt_id
      = gHist.getSequentialID(kSDC2, 0, kMulti, 1+NumOfLayersSDC2);

    // TDC & HitPat & Multi
    for(int l=0; l<NumOfLayersSDC2; ++l){
      int multiplicity    = 0;
      int multiplicity_wt = 0;
      for( int w=0; w<NumOfWireSDC2; ++w ){
	int nhit = gUnpacker.get_entries(k_device, l, 0, w, k_tdc);
	if( nhit == 0 ) continue;

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[sdc2hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	int  tdc1st = 0;
	for( int m = 0; m<nhit; ++m ){
	  int tdc = gUnpacker.get(k_device, l, 0, w, k_tdc, m);
	  hptr_array[sdc2t_id + l]->Fill(tdc);
	  if( tdc1st<tdc ) tdc1st = tdc;

	  // Drift time check
	  if( tdc_min < tdc && tdc < tdc_max ){
	    flag_hit_wt = true;
	  }
	}

	if( tdc1st!=0 ) hptr_array[sdc2t1st_id +l]->Fill( tdc1st );
	if( flag_hit_wt ){
	  ++multiplicity_wt;
	  hptr_array[sdc2hit_id + l]->Fill( w );
	}
      }

      hptr_array[sdc2mul_id + l]->Fill(multiplicity);
      hptr_array[sdc2mulwt_id + l]->Fill(multiplicity_wt);
    }

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device,0);
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
    static const int tdc_min = gUser.GetParameter("SDC3_TDC", 0);
    static const int tdc_max = gUser.GetParameter("SDC3_TDC", 1);

    // sequential id
    static const int sdc3t_id    = gHist.getSequentialID(kSDC3, 0, kTDC);
    static const int sdc3t1st_id = gHist.getSequentialID(kSDC3, 0, kTDC2D);
    static const int sdc3hit_id  = gHist.getSequentialID(kSDC3, 0, kHitPat);
    static const int sdc3mul_id  = gHist.getSequentialID(kSDC3, 0, kMulti);
    static const int sdc3mulwt_id
      = gHist.getSequentialID(kSDC3, 0, kMulti, 1+NumOfLayersSDC3);

    // TDC & HitPat & Multi
    for(int l=0; l<NumOfLayersSDC3; ++l){
      int multiplicity    = 0;
      int multiplicity_wt = 0;
      int sdc3_nwire = 0;
      if( l==0 || l==1 )
	sdc3_nwire = NumOfWireSDC3Y;
      if( l==2 || l==3 )
	sdc3_nwire = NumOfWireSDC3X;

      for( int w=0 ; w<sdc3_nwire; ++w ){
	int nhit = gUnpacker.get_entries(k_device, l, 0, w, k_tdc);
	if( nhit == 0 ) continue;

	// This wire fired at least one times.
	++multiplicity;
	// hptr_array[sdc3hit_id + l]->Fill(w, nhit);

	bool flag_hit_wt = false;
	int  tdc1st = 0;
	for( int m = 0; m<nhit; ++m ){
	  int tdc = gUnpacker.get(k_device, l, 0, w, k_tdc, m);
	  hptr_array[sdc3t_id + l]->Fill(tdc);
	  if( tdc1st<tdc ) tdc1st = tdc;

	  // Drift time check
	  if( tdc_min < tdc && tdc < tdc_max ){
	    flag_hit_wt = true;
	  }
	}

	if( tdc1st!=0 ) hptr_array[sdc3t1st_id +l]->Fill( tdc1st );
	if( flag_hit_wt ){
	  ++multiplicity_wt;
	  hptr_array[sdc3hit_id + l]->Fill( w );
	}
      }

      hptr_array[sdc3mul_id +l]->Fill( multiplicity );
      hptr_array[sdc3mulwt_id +l]->Fill( multiplicity_wt );
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
    int tofa_id   = gHist.getSequentialID(kTOF, 0, kADC);
    int toft_id   = gHist.getSequentialID(kTOF, 0, kTDC);
    int tofawt_id = gHist.getSequentialID(kTOF, 0, kADCwTDC);
    for(int seg = 0; seg<NumOfSegTOF; ++seg){
      // ADC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc);
      if( nhit!=0 ){
	unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	hptr_array[tofa_id + seg]->Fill( adc );
      }
      // TDC
      nhit = gUnpacker.get_entries(k_device, 0, seg, k_u, k_tdc);
      if(nhit != 0){
	int tdc = gUnpacker.get(k_device, 0, seg, k_u, k_tdc);
	if( tdc!=0 ){
	  hptr_array[toft_id + seg]->Fill(tdc);
	  // ADC w/TDC
	  if( gUnpacker.get_entries(k_device, 0, seg, k_u, k_adc)>0 ){
	    unsigned int adc = gUnpacker.get(k_device, 0, seg, k_u, k_adc);
	    hptr_array[tofawt_id + seg]->Fill( adc );
	  }
	}
      }
    }

    // Down PMT
    tofa_id   = gHist.getSequentialID(kTOF, 0, kADC, NumOfSegTOF+1);
    toft_id   = gHist.getSequentialID(kTOF, 0, kTDC, NumOfSegTOF+1);
    tofawt_id = gHist.getSequentialID(kTOF, 0, kADCwTDC, NumOfSegTOF+1);

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
	if(tdc != 0){
	  hptr_array[toft_id + seg]->Fill(tdc);
	  // ADC w/TDC
	  if( gUnpacker.get_entries(k_device, 0, seg, k_d, k_adc)>0 ){
	    unsigned int adc = gUnpacker.get(k_device, 0, seg, k_d, k_adc);
	    hptr_array[tofawt_id + seg]->Fill( adc );
	  }
	}
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
	if(tdc_u!=0 && tdc_d!=0){
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

  // MsT -----------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("MsT");
    // sequential id
    int tdc_id    = gHist.getSequentialID(kMsT, 0, kTDC);
    int tdc2d_id  = gHist.getSequentialID(kMsT, 0, kTDC2D);
    int tof_hp_id = gHist.getSequentialID(kMsT, 0, kHitPat, 0);
    int sch_hp_id = gHist.getSequentialID(kMsT, 0, kHitPat, 1);
    int flag_id   = gHist.getSequentialID(kMsT, 0, kHitPat, 2);
    int flag2d_id = gHist.getSequentialID(kMsT, 0, kHitPat2D, 0);

    // Flag
    int hul_flag  = -1;
    int soft_flag =  1;
    {
      int nhit = gUnpacker.get_entries(k_device, 2, 0, 0, 1);
      if( nhit>0 ){
	int flag = gUnpacker.get(k_device, 2, 0, 0, 1);
	if( std::bitset<3>(flag).count() == 1 ){
	  hptr_array[flag_id]->Fill( flag );
	  hul_flag = TMath::Log2(flag); // 1,2,4 -> 0,1,2
	} else {
	  std::cerr << "#W Invalid MsTFlag : " << flag << std::endl;
	}
      }
    }

    int tof_nhits = 0;
    for(int seg=0; seg<NumOfSegTOF; ++seg){
      // TDC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, 0, 1);
      if( nhit<=0 ) continue;
      int tdc = gUnpacker.get(k_device, 0, seg, 0, 1);
      if( tdc<=0 ) continue;
      hptr_array[tdc_id +seg]->Fill( tdc );
      hptr_array[tdc2d_id]->Fill( seg, tdc );
      if( hul_flag==0 )
	hptr_array[tdc_id +NumOfSegTOF +seg]->Fill( tdc );
      // HitPat
      hptr_array[tof_hp_id]->Fill(seg);
      for( int seg2=0; seg2<NumOfSegSCH; ++seg2 ){
	int nhit2 = gUnpacker.get_entries(k_device, 1, seg2, 0, 1);
	if( nhit2<=0 ) continue;
	int hit2 = gUnpacker.get(k_device, 1, seg2, 0, 1);
	if( hit2<=0 ) continue;
	if( hit2>0 && soft_flag!=0 ){
	  if( gMsT.IsAccept( seg, seg2, tdc ) ){
	    soft_flag = 0;
	  }
	}
      }
      tof_nhits++;
    }

    if( !trigger_flag[7] || tof_nhits==0 )
      soft_flag = 2;

    hptr_array[flag2d_id]->Fill( hul_flag, soft_flag );

    for(int seg=0; seg<NumOfSegSCH; ++seg){
      // HitPat
      int nhit = gUnpacker.get_entries(k_device, 1, seg, 0, 1);
      if( nhit!=0 ){
	int flag = gUnpacker.get(k_device, 1, seg, 0, 1);
	if( flag ) hptr_array[sch_hp_id]->Fill(seg);
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

  // Mtx3D --------------------------------------------------------
  {
    // data type
    static const int k_device = gUnpacker.get_device_id("Mtx3D");
    static const int k_tof    = gUnpacker.get_data_id("Mtx3D", "ttof");
    static const int k_fbh    = gUnpacker.get_data_id("Mtx3D", "tfbh");
    static const int k_sch    = gUnpacker.get_data_id("Mtx3D", "tch");

    // sequential id
    static const int tof_tdc_id = gHist.getSequentialID(kMtx3D, kHulTOF, kTDC);
    static const int fbh_tdc_id = gHist.getSequentialID(kMtx3D, kHulFBH, kTDC);
    static const int sch_tdc_id = gHist.getSequentialID(kMtx3D, kHulSCH, kTDC);
    static const int tof_fbh_id = gHist.getSequentialID(kMtx3D, kHulTOFxFBH, kHitPat2D);
    static const int tof_sch_id = gHist.getSequentialID(kMtx3D, kHulTOFxSCH, kHitPat2D);
    static const int fbh_sch_id = gHist.getSequentialID(kMtx3D, kHulFBHxSCH, kHitPat2D);
    static const int mtx2d_id   = gHist.getSequentialID(kMtx3D, kHul2DHitPat,kHitPat2D);
    static const int mtx3d_id   = gHist.getSequentialID(kMtx3D, kHul3DHitPat,kHitPat2D);
    static const int flag2d_id  = gHist.getSequentialID(kMtx3D, kHul2D, kHitPat2D);
    static const int flag3d_id  = gHist.getSequentialID(kMtx3D, kHul3D, kHitPat2D);

    // TOF
    for(int seg=0; seg<NumOfSegTOF; ++seg){
      // TDC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, 0, k_tof);
      if( nhit!=0 ){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, 0, k_tof);
	hptr_array[tof_tdc_id +seg]->Fill( tdc );
      }
    }

    // FBH
    for(int seg=0; seg<NumOfSegClusteredFBH; ++seg){
      // TDC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, 0, k_fbh);
      if( nhit!=0 ){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, 0, k_fbh);
	hptr_array[fbh_tdc_id +seg]->Fill( tdc );
      }
    }

    // SCH
    for(int seg=0; seg<NumOfSegSCH; ++seg){
      // TDC
      int nhit = gUnpacker.get_entries(k_device, 0, seg, 0, k_sch);
      if( nhit!=0 ){
	unsigned int tdc = gUnpacker.get(k_device, 0, seg, 0, k_sch);
	hptr_array[sch_tdc_id +seg]->Fill( tdc );
      }
    }

    // TOF x FBH
    bool fl_fbh_hit = false;
    for(int i_tof=0; i_tof<NumOfSegTOF; ++i_tof){
      int nhit_tof = gUnpacker.get_entries(k_device, 0, i_tof, 0, k_tof);
      if(nhit_tof == 0) continue;

      for(int i_fbh=0; i_fbh<NumOfSegClusteredFBH; ++i_fbh){
	int nhit_fbh = gUnpacker.get_entries(k_device, 0, i_fbh, 0, k_fbh);
	if(nhit_fbh != 0) hptr_array[tof_fbh_id]->Fill(i_fbh, i_tof);
	if(nhit_fbh != 0) fl_fbh_hit = true;
      }// for(FBH)
    }// for(TOF)

    // TOF x SCH
    bool hul2d_flag = false;
    bool hul3d_flag = false;
    bool fbh_coin   = false;
    for(int i_tof=0; i_tof<NumOfSegTOF; ++i_tof){
      int nhit_tof = gUnpacker.get_entries(k_device, 0, i_tof, 0, k_tof);
      if(nhit_tof == 0) continue;

      for(int i_sch=0; i_sch<NumOfSegSCH; ++i_sch){
	int nhit_sch = gUnpacker.get_entries(k_device, 0, i_sch, 0, k_sch);
	if(nhit_sch != 0){
	  hptr_array[tof_sch_id]->Fill(i_sch, i_tof);

	  if(fl_fbh_hit) hptr_array[mtx2d_id]->Fill(i_sch, i_tof);
	  // x FBH
	  for(int i_fbh=0; i_fbh<NumOfSegClusteredFBH; ++i_fbh){
	    if( !hul3d_flag ) hul3d_flag = gMatrix.IsAccept( i_tof, i_sch, i_fbh );
	    int nhit_fbh = gUnpacker.get_entries(k_device, 0, i_fbh, 0, k_fbh);
	    if( nhit_fbh>0 ) hptr_array[mtx3d_id+i_fbh]->Fill(i_sch, i_tof);
	    if( !fbh_coin ){
	      if( nhit_fbh>0 ) fbh_coin = true;
	    }
	  }// for(FBH)
	  if( !hul2d_flag ) hul2d_flag = ( gMatrix.IsAccept( i_tof, i_sch ) && fbh_coin );
	}
      }// for(SCH)
    }// for(TOF)

    hptr_array[flag2d_id]->Fill( matrix2d_flag, hul2d_flag );
    hptr_array[flag3d_id]->Fill( matrix3d_flag, hul3d_flag );

    // FBH x SCH
    for(int i_fbh=0; i_fbh<NumOfSegClusteredFBH; ++i_fbh){
      int nhit_fbh = gUnpacker.get_entries(k_device, 0, i_fbh, 0, k_fbh);
      if(nhit_fbh == 0) continue;

      for(int i_sch=0; i_sch<NumOfSegSCH; ++i_sch){
	int nhit_sch = gUnpacker.get_entries(k_device, 0, i_sch, 0, k_sch);
	if(nhit_sch != 0) hptr_array[fbh_sch_id]->Fill(i_sch, i_fbh);
      }// for(SCH)
    }// for(FBH)

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
    static const int k_device_bh1  = gUnpacker.get_device_id("BH1");
    static const int k_device_bh2  = gUnpacker.get_device_id("BH2");
    static const int k_device_sch  = gUnpacker.get_device_id("SCH");
    static const int k_device_tof  = gUnpacker.get_device_id("TOF");
    static const int k_device_bc3  = gUnpacker.get_device_id("BC3");
    static const int k_device_bc4  = gUnpacker.get_device_id("BC4");
    static const int k_device_sdc1 = gUnpacker.get_device_id("SDC1");
    static const int k_device_sdc2 = gUnpacker.get_device_id("SDC2");

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

    // TOF vs SCH
    TH2* hcor_tofsch = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(int seg1 = 0; seg1<NumOfSegSCH; ++seg1){
      for(int seg2 = 0; seg2<NumOfSegTOF; ++seg2){
	int hitSCH = gUnpacker.get_entries(k_device_sch, 0, seg1, 0, 1);
	int hitTOF = gUnpacker.get_entries(k_device_tof, 0, seg2, 0, 1);
	if(hitTOF == 0 || hitSCH == 0) continue;
	int tdcSCH = gUnpacker.get(k_device_sch, 0, seg1, 0, 1);
	int tdcTOF = gUnpacker.get(k_device_tof, 0, seg2, 0, 1);
	if(tdcTOF != 0 && tdcSCH != 0){
	  hcor_tofsch->Fill(seg1, seg2);
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

    // SDC2 vs SDC1
    TH2* hcor_sdc1sdc2 = dynamic_cast<TH2*>(hptr_array[cor_id++]);
    for(int wire1 = 0; wire1<NumOfWireSDC1; ++wire1){
      for(int wire2 = 0; wire2<NumOfWireSDC2; ++wire2){
	int hitSDC1 = gUnpacker.get_entries(k_device_sdc1, 0, 0, wire1, 0);
	int hitSDC2 = gUnpacker.get_entries(k_device_sdc2, 0, 0, wire2, 0);
	if( hitSDC1 == 0 || hitSDC2 == 0 ) continue;
	hcor_sdc1sdc2->Fill( wire1, wire2 );
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
    static const int cid_bh1  = 1;
    static const int cid_bh2  = 2;
    static const int plid     = 0;

    // Sequential ID
    static const int btof_id  = gHist.getSequentialID(kMisc, 0, kTDC);

    // BH2
    double t0  = -999;
    double ofs = 0;
    int    seg = 0;
    int nhitu = gUnpacker.get_entries(k_d_bh2, 0, seg, k_u, k_tdc);
    int nhitd = gUnpacker.get_entries(k_d_bh2, 0, seg, k_d, k_tdc);
    if( nhitu != 0 && nhitd != 0 ){
      int tdcu = gUnpacker.get(k_d_bh2, 0, seg, k_u, k_tdc);
      int tdcd = gUnpacker.get(k_d_bh2, 0, seg, k_d, k_tdc);
      if( tdcu != 0 && tdcd != 0 ){
	HodoParamMan& hodoMan = HodoParamMan::GetInstance();
	double bh2ut, bh2dt;
	hodoMan.GetTime(cid_bh2, plid, seg, k_u, tdcu, bh2ut);
	hodoMan.GetTime(cid_bh2, plid, seg, k_d, tdcd, bh2dt);
	t0 = (bh2ut+bh2dt)/2;
      }//if(tdc)
    }// if(nhit)

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

  // EMC -----------------------------------------------------------
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
      //if( spill > pos2spill ){
      if( // pos2spill == nspill ||
	  pos2spill == 1 ){
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
	TTimeStamp stamp;
	stamp.Add( -TTimeStamp::GetZoneOffset() );
	stamp.Add( Int_t(rspill*5.52) );
	ss << std::setw(4) << rspill << " : "
	   << rhour << "h " << rmin << "m " << rsec << "s";
	text.SetText( 0.120, 0.860, ss.str().c_str() );
	end.SetText( 0.120, 0.800, "End Time : "+TString(stamp.AsString("s")) );
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

  // Efficiency
  http::UpdateBcOutEfficiency();
  http::UpdateSdcInOutEfficiency();
  http::UpdateSSDEfficiency();

  return 0;
}

}
