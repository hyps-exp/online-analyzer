// -*- C++ -*-

// Author: Tomonori Takahashi
// Change 2017/09/ S.Hoshino

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
#include "DetectorID.hh"
#include "DCDriftParamMan.hh"
#include "DCGeomMan.hh"
#include "DCTdcCalibMan.hh"
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
#include "UserParamMan.hh"

#include "RawData.hh"
#include "FiberCluster.hh"
#include "HodoAnalyzer.hh"
#include "FiberHit.hh"

#include "AftHelper.hh"

#define DEBUG    0
#define FLAG_DAQ 1

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  namespace
  {
    const UserParamMan& gUser = UserParamMan::GetInstance();
    auto& gAftHelper          = AftHelper::GetInstance();
    std::vector<TH1*> hptr_array;
    bool flag_event_cut = false;
    int event_cut_factor = 1; // for fast semi-online analysis
  }

//____________________________________________________________________________
int
process_begin( const std::vector<std::string>& argv )
{
  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  gConfMan.InitializeParameter<HodoParamMan>("HDPRM");
  gConfMan.InitializeParameter<HodoPHCMan>("HDPHC");
  gConfMan.InitializeParameter<DCGeomMan>("DCGEO");
  gConfMan.InitializeParameter<DCTdcCalibMan>("DCTDC");
  gConfMan.InitializeParameter<DCDriftParamMan>("DCDRFT");
  gConfMan.InitializeParameter<UserParamMan>("USER");
  if( !gConfMan.IsGood() ) return -1;
  // unpacker and all the parameter managers are initialized at this stage

  if( argv.size()==4 ){
    int factor = std::strtod( argv[3].c_str(), NULL );
    if( factor!=0 ) event_cut_factor = std::abs( factor );
    flag_event_cut = true;
    std::cout << "#D Event cut flag on : factor="
	      << event_cut_factor << std::endl;
  }

  // Make tabs
  hddaq::gui::Controller& gCon = hddaq::gui::Controller::getInstance();
  TGFileBrowser *tab_hist  = gCon.makeFileBrowser("Hist");
  TGFileBrowser *tab_macro = gCon.makeFileBrowser("Macro");

  // Add macros to the Macro tab
  //tab_macro->Add(hoge());
  tab_macro->Add(macro::Get("clear_all_canvas"));
  tab_macro->Add(macro::Get("clear_canvas"));
  tab_macro->Add(macro::Get("split22"));
  tab_macro->Add(macro::Get("split32"));
  tab_macro->Add(macro::Get("split33"));
  tab_macro->Add(macro::Get("dispAFT1D"));
  tab_macro->Add(macro::Get("dispAFT2D"));
  tab_macro->Add(macro::Get("dispAFTHitPat"));
  tab_macro->Add(macro::Get("dispAFTTot"));
  tab_macro->Add(macro::Get("effAFT"));
  tab_macro->Add(macro::Get("dispDAQ"));

  // Add histograms to the Hist tab
  HistMaker& gHist = HistMaker::getInstance();
  tab_hist->Add(gHist.createAFT());

  // Set histogram pointers to the vector sequentially.
  // This vector contains both TH1 and TH2.
  // Then you need to do down cast when you use TH2.
  if(0 != gHist.setHistPtr(hptr_array)){ return -1; }

  // Users don't have to touch this section (Make Ps tab),
  // but the file path should be changed.
  // ----------------------------------------------------------
  PsMaker& gPsMaker = PsMaker::getInstance();
  std::vector<TString> detList;
  std::vector<TString> optList;
  gHist.getListOfPsFiles(detList);
  gPsMaker.getListOfOption(optList);

  hddaq::gui::GuiPs& gPsTab = hddaq::gui::GuiPs::getInstance();
  gPsTab.setFilename(Form("%s/PSFile/pro/default_catch.ps", std::getenv("HOME")));
  gPsTab.initialize(optList, detList);
  // ----------------------------------------------------------

  gStyle->SetOptStat(1110);
  gStyle->SetTitleW(.700);
  gStyle->SetTitleH(.100);
  // gStyle->SetTitleW(.400);
  // gStyle->SetTitleH(.100);
  // gStyle->SetStatW(.320);
  gStyle->SetStatW(.200);
  gStyle->SetStatH(.250);

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

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

  const int event_number = gUnpacker.get_event_number();
  if( flag_event_cut && event_number%event_cut_factor!=0 )
    return 0;

  // RawData *rawData;
  // rawData = new RawData;

  // rawData->DecodeHits();

  HodoAnalyzer *hodoAna;
  hodoAna = new HodoAnalyzer;



//  // DAQ -------------------------------------------------------------
//  {
//    // node id
//    static const int k_eb      = gUnpacker.get_fe_id("k18eb");
//    static const int k_vme     = gUnpacker.get_fe_id("vme03");
//    static const int k_vmeeasiroc = gUnpacker.get_fe_id("vmeeasiroc0");
//
//    // sequential id
//    static const int eb_id      = gHist.getSequentialID(kDAQ, kEB, kHitPat);
//    static const int vme_id     = gHist.getSequentialID(kDAQ, kVME, kHitPat2D);
//    static const int vmeeasiroc_id = gHist.getSequentialID(kDAQ, kVMEEASIROC, kHitPat2D);
//
//    { // EB
//      int data_size = gUnpacker.get_node_header(k_eb, DAQNode::k_data_size);
//      hptr_array[eb_id]->Fill(data_size);
//    }
//
//    { // VME node
//      TH2* h = dynamic_cast<TH2*>(hptr_array[vme_id]);
//      for( int i=0; i<10; ++i ){
//	if( i==1 || i==5 ) continue;
//	int node_id = k_vme+i;
//	int data_size = gUnpacker.get_node_header( node_id, DAQNode::k_data_size);
//	h->Fill( i, data_size );
//      }
//    }
//
//    { // VMEEASIROC node
//      TH2* h = dynamic_cast<TH2*>(hptr_array[easiroc_id]);
//      for(int i = 0; i<20; ++i){
//	int data_size = gUnpacker.get_node_header(k_easiroc+i, DAQNode::k_data_size);
//	h->Fill( i, data_size );
//      }
//    }
//
//  }

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif

//  if( trigger_flag[SpillEndFlag] ) return 0;

  //------------------------------------------------------------------
  // AFT
  //------------------------------------------------------------------
  {
    // data type
    static const int k_device   = gUnpacker.get_device_id("AFT");
    static const int k_leading  = gUnpacker.get_data_id("AFT" , "leading");
    static const int k_trailing = gUnpacker.get_data_id("AFT", "trailing");
    static const int k_highgain = gUnpacker.get_data_id("AFT" , "highgain");
    static const int k_lowgain  = gUnpacker.get_data_id("AFT", "lowgain");
    // TDC gate range
    static const int tdc_min = gUser.GetParameter("TdcAFT", 0);
    static const int tdc_max = gUser.GetParameter("TdcAFT", 1);

    // SequentialID
    int aft_t_id    = gHist.getSequentialID(kAFT, 0, kTDC,        1);
    int aft_tot_id  = gHist.getSequentialID(kAFT, 0, kTOT,        1);
    int aft_ctot_id = gHist.getSequentialID(kAFT, 0, kTOT,      101);
    int aft_hg_id   = gHist.getSequentialID(kAFT, 0, kHighGain,   1);
    int aft_chg_id  = gHist.getSequentialID(kAFT, 0, kHighGain, 101);
    int aft_lg_id   = gHist.getSequentialID(kAFT, 0, kLowGain,    1);
    int aft_clg_id  = gHist.getSequentialID(kAFT, 0, kLowGain,  101);
    // int aft_pe_id   = gHist.getSequentialID(kAFT, 0, kPede,       1);

    int aft_t_2d_id    = gHist.getSequentialID(kAFT, 0, kTDC2D,        1);
    int aft_tot_2d_id  = gHist.getSequentialID(kAFT, 0, kTOT2D,        1);
    int aft_ctot_2d_id = gHist.getSequentialID(kAFT, 0, kTOT2D,      101);
    int aft_hg_2d_id   = gHist.getSequentialID(kAFT, 0, kHighGain2D,   1);
    int aft_chg_2d_id  = gHist.getSequentialID(kAFT, 0, kHighGain2D, 101);
    int aft_lg_2d_id   = gHist.getSequentialID(kAFT, 0, kLowGain2D,    1);
    int aft_clg_2d_id  = gHist.getSequentialID(kAFT, 0, kLowGain2D,  101);
    // int aft_pe_2d_id   = gHist.getSequentialID(kAFT, 0, kPede2D,       1);

    int aft_hg_tot_id = gHist.getSequentialID(kAFT, 0, kHighGainXTOT, 1);
    int aft_lg_tot_id = gHist.getSequentialID(kAFT, 0, kLowGainXTOT,  1);

    int aft_hit_id     = gHist.getSequentialID(kAFT, 0, kHitPat,   1);
    int aft_chit_id    = gHist.getSequentialID(kAFT, 0, kHitPat, 101);
    int aft_mul_id     = gHist.getSequentialID(kAFT, 0, kMulti,    1);
    int aft_cmul_id    = gHist.getSequentialID(kAFT, 0, kMulti,  101);

    int aft_cl_tdc_id     = gHist.getSequentialID(kAFT, kCluster, kTDC,        1);
    int aft_cl_tdc2d_id   = gHist.getSequentialID(kAFT, kCluster, kTDC2D,      1);
    int aft_cl_hgadc_id   = gHist.getSequentialID(kAFT, kCluster, kHighGain,   1);
    int aft_cl_hgadc2d_id = gHist.getSequentialID(kAFT, kCluster, kHighGain2D, 1);
    int aft_cl_lgadc_id   = gHist.getSequentialID(kAFT, kCluster, kLowGain,    1);
    int aft_cl_lgadc2d_id = gHist.getSequentialID(kAFT, kCluster, kLowGain2D,  1);

    int cmultiplicity_both[NumOfPlaneAFT];
    for(int l=0; l<NumOfPlaneAFT; ++l){
      int multiplicity = 0;
      int cmultiplicity = 0;
      cmultiplicity_both[l] = 0;
      std::vector<std::vector<bool>> flag_hit_wt(NumOfSegAFT[l%4]);
      for(int seg = 0; seg<NumOfSegAFT[l%4]; ++seg){
	flag_hit_wt[seg].resize(kUorD);
	for(int ud=0; ud<kUorD; ud++){
	  { // highgain
	    int nhit_hg = gUnpacker.get_entries(k_device, l, seg, ud, k_highgain);
	    if(nhit_hg!=0){
	      for(int m = 0; m<nhit_hg; ++m){
		int adc_hg = gUnpacker.get(k_device, l, seg, ud, k_highgain, m);
		hptr_array[aft_hg_id+ud*NumOfPlaneAFT+l]->Fill(adc_hg);
		hptr_array[aft_hg_2d_id+ud*NumOfPlaneAFT+l]->Fill(seg, adc_hg);

		// double pedeAFT = 0.;
		// double gainAFT = 100.;
		// // pedeAFT = hman->GetPedestal(DetIdAFT,layer,i,0);
		// // gainAFT = hman->GetGain(DetIdAFT,layer,i,0);
		// double adc_pe = ((double)adc_hg - pedeAFT)/gainAFT;
		// hptr_array[aft_pe_id+ud*NumOfPlaneAFT+l]->Fill(adc_pe);
		// hptr_array[aft_pe_2d_id+ud*NumOfPlaneAFT+l]->Fill(seg, adc_pe);
	      }
	    }
	  }

	  { // lowgain
	    int nhit_lg = gUnpacker.get_entries(k_device, l, seg, ud, k_lowgain );
	    if(nhit_lg!=0){
	      for(int m = 0; m<nhit_lg; ++m){
		int adc_lg = gUnpacker.get(k_device, l, seg, ud, k_lowgain, m);
		hptr_array[aft_lg_id+ud*NumOfPlaneAFT+l]->Fill(adc_lg);
		hptr_array[aft_lg_2d_id+ud*NumOfPlaneAFT+l]->Fill(seg, adc_lg);
	      }
	    }
	  }

	  { // TDC
	    int nhit_l = gUnpacker.get_entries(k_device, l, seg, ud, k_leading );
	    flag_hit_wt[seg][ud] = false;
	    if(nhit_l!=0){
	      ++multiplicity;
	      for(int m = 0; m<nhit_l; ++m){
		// TDC & hitpat
		int tdc = gUnpacker.get(k_device, l, seg, ud, k_leading, m);
		hptr_array[aft_t_id+ud*NumOfPlaneAFT+l]->Fill(tdc);
		hptr_array[aft_t_2d_id+ud*NumOfPlaneAFT+l]->Fill(seg, tdc);
		hptr_array[aft_hit_id+ud*NumOfPlaneAFT+l]->Fill(seg);
		if(tdc_min < tdc && tdc < tdc_max){ // w/ TDC cut
		  flag_hit_wt[seg][ud] = true;
		  hptr_array[aft_chit_id+ud*NumOfPlaneAFT+l]->Fill(seg);
		}
	      }
	      if(flag_hit_wt[seg][ud]){
		++cmultiplicity;
		// highgain w/ TDC cut
		int nhit_hg = gUnpacker.get_entries(k_device, l, seg, ud, k_highgain);
		if( nhit_hg != 0 ){
		  int adc_hg = gUnpacker.get(k_device, l, seg, ud, k_highgain, 0);
		  hptr_array[aft_chg_id+ud*NumOfPlaneAFT+l]->Fill(adc_hg);
		  hptr_array[aft_chg_2d_id+ud*NumOfPlaneAFT+l]->Fill(seg, adc_hg);
		}
		// lowgain w/ TDC cut
		int nhit_lg = gUnpacker.get_entries(k_device, l, seg, ud, k_lowgain);
		if( nhit_lg != 0 ){
		  int adc_lg = gUnpacker.get(k_device, l, seg, ud, k_lowgain, 0);
		  hptr_array[aft_clg_id+ud*NumOfPlaneAFT+l]->Fill(adc_lg);
		  hptr_array[aft_clg_2d_id+ud*NumOfPlaneAFT+l]->Fill(seg, adc_lg);
		}
	      }
	    }
	  }

	  { // TOT
	    int nhit_l = gUnpacker.get_entries(k_device, l, seg, ud, k_leading );
	    int nhit_t = gUnpacker.get_entries(k_device, l, seg, ud, k_trailing );
	    int hit_l_max = 0;
	    int hit_t_max = 0;
	    if(nhit_l != 0) hit_l_max = gUnpacker.get(k_device, l, seg, ud, k_leading,  nhit_l - 1);
	    if(nhit_t != 0) hit_t_max = gUnpacker.get(k_device, l, seg, ud, k_trailing, nhit_t - 1);
	    if(nhit_l == nhit_t && hit_l_max > hit_t_max){
	      for(int m = 0; m<nhit_l; ++m){
		int tdc   = gUnpacker.get(k_device, l, seg, ud, k_leading, m);
		int tdc_t = gUnpacker.get(k_device, l, seg, ud, k_trailing, m);
		int tot   = tdc - tdc_t;
		hptr_array[aft_tot_id+ud*NumOfPlaneAFT+l]->Fill(tot);
		hptr_array[aft_tot_2d_id+ud*NumOfPlaneAFT+l]->Fill(seg, tot);
		if(tdc_min < tdc && tdc < tdc_max){
		  // tot w/ TDC cut
		  hptr_array[aft_ctot_id+ud*NumOfPlaneAFT+l]->Fill(tot);
		  hptr_array[aft_ctot_2d_id+ud*NumOfPlaneAFT+l]->Fill(seg, tot);
		  if( m == nhit_l-1 ){
		    // highgain x TOT1st w/ TDC cut
		    int nhit_hg = gUnpacker.get_entries(k_device, l, seg, ud, k_highgain);
		    if( nhit_hg != 0 ){
		      int adc_hg = gUnpacker.get(k_device, l, seg, ud, k_highgain, 0);
		      hptr_array[aft_hg_tot_id+ud*NumOfPlaneAFT+l]->Fill(adc_hg, tot);
		    }
		    // lowgain x TOT1st w/ TDC cut
		    int nhit_lg = gUnpacker.get_entries(k_device, l, seg, ud, k_lowgain);
		    if( nhit_lg != 0 ){
		      int adc_lg = gUnpacker.get(k_device, l, seg, ud, k_lowgain, 0);
		      hptr_array[aft_lg_tot_id+ud*NumOfPlaneAFT+l]->Fill(adc_lg, tot);
		    }
		  }
		}
	      } // for in multihit
	    }
	  }

	  hptr_array[aft_mul_id+ud*NumOfPlaneAFT+l]->Fill(multiplicity);
	  hptr_array[aft_cmul_id+ud*NumOfPlaneAFT+l]->Fill(cmultiplicity);
	} // for in Up or Down

	// hitpat & multiplicity of hit on both ends
	if( flag_hit_wt[seg][kU] && flag_hit_wt[seg][kD] ){
	  ++cmultiplicity_both[l];
	  hptr_array[aft_chit_id+kUorD*NumOfPlaneAFT+l]->Fill(seg);
	  double posx = gAftHelper.GetX( l, seg );
	  double posz = gAftHelper.GetZ( l, seg );
	  if( l%4 == 0 || l%4 == 1 ) hptr_array[aft_chit_id+(kUorD+1)*NumOfPlaneAFT+0]->Fill(posz, posx);
	  if( l%4 == 2 || l%4 == 3 ) hptr_array[aft_chit_id+(kUorD+1)*NumOfPlaneAFT+1]->Fill(posz, posx);
	}

      } // for in NumOfSegAFT
      hptr_array[aft_cmul_id+kUorD*NumOfPlaneAFT+l]->Fill(cmultiplicity_both[l]);
      if( l%2==1 ){
	int cmultiplicity_both_XY = cmultiplicity_both[l-1] + cmultiplicity_both[l];
	hptr_array[aft_cmul_id+(kUorD+1)*NumOfPlaneAFT+l/2]->Fill(cmultiplicity_both_XY);
      }
    } // for in NumOfPlaneAFT


  //   //clustering analysis
  // hodoAna->DecodeAFTHits(rawData);
  // // Fiber Cluster
  // for(int p = 0; p<NumOfPlaneAFT; ++p){
  //   //int nhits = hodoAna->GetNHitsAFT(p);
  //   //std::cout<<"plane= "<<p<<",NHits="<<nhits<<std::endl;

  //   //hodoAna->TimeCutAFT(p, -30, 30); // CATCH@J-PARC
  //   hodoAna->AdcCutAFT(p, 0, 4000); // CATCH@J-PARC
  //   //hodoAna->AdcCutAFT(p, 50, 4000); // CATCH@J-PARC  for proton
  //   //hodoAna->WidthCutAFT(p, 60, 300); // pp scattering
  //   //hodoAna->WidthCutAFT(p, 30, 300); // cosmic ray

  //   int ncl = hodoAna->GetNClustersAFT(p);
  //   //std::cout<<"plane= "<<p<<",NClusters="<<ncl<<std::endl;
  //   for(int i=0; i<ncl; ++i){
  //     FiberCluster *cl = hodoAna->GetClusterAFT(p,i);
  //     if(!cl) continue;
  //     //      double size  = cl->ClusterSize();
  //     //double seg = cl->MeanSeg();
  //     //double ctime = cl->CMeanTime();
  //     //double width = -cl->minWidth();
  //     //double width = cl->Width();
  //     //double sumADC= cl->SumAdcLow();
  //     //double sumMIP= cl->SumMIPLow();
  //     //double sumdE = cl->SumdELow();
  //     //double r     = cl->MeanPositionR();
  //     //double phi   = cl->MeanPositionPhi();
  //     int Mseg  = cl->MaxSeg();
  //     int MADCHi  = cl->MaxAdcHi();
  //     int MADCLow = cl->MaxAdcLow();
  //     //std::cout<<"cluster="<<i<<" , Maxseg="<<Mseg<<" , MaxADC"<<MADCHi<<std::endl;
  //     hptr_array[aft_cl_hgadc_id + p] ->Fill(MADCHi);
  //     hptr_array[aft_cl_lgadc_id + p] ->Fill(MADCLow);
  //     hptr_array[aft_cl_hgadc2d_id + p] ->Fill(Mseg, MADCHi);
  //     hptr_array[aft_cl_lgadc2d_id + p] ->Fill(Mseg, MADCLow);
  //     int fmulti =hodoAna->GetNHitsAFT(p);
  //     for(int j=0; j<fmulti; ++j){
  // 	FiberHit *fhit = hodoAna->GetHitAFT(p,j);
  // 	int seg_temp = fhit->PairId();
  // 	if(seg_temp == Mseg){
  // 	  int Mhit =fhit ->GetNumOfHit();
  // 	  for(int k=0; k<Mhit; ++k){
  // 	    double Mtime =fhit->GetLeading(k);
  // 	    hptr_array[aft_cl_tdc_id + p] ->Fill(Mtime);
  // 	    hptr_array[aft_cl_tdc2d_id + p] ->Fill(Mseg, Mtime);
  // 	  }
  // 	}
  //     }
  //   }
  // }

  // delete rawData;
  // delete hodoAna;

#if 0
    // Debug, dump data relating this detector
    gUnpacker.dump_data_device(k_device);
#endif
  }//AFT

#if DEBUG
  std::cout << __FILE__ << " " << __LINE__ << std::endl;
#endif


//   //------------------------------------------------------------------
//   // CFT
//   //------------------------------------------------------------------
//   {
//     // data type
//     static const int k_device   = gUnpacker.get_device_id("CFT");
//     static const int k_leading  = gUnpacker.get_data_id("CFT" , "leading");
//     static const int k_trailing = gUnpacker.get_data_id("CFT", "trailing");
//     static const int k_highgain = gUnpacker.get_data_id("CFT" , "highgain");
//     static const int k_lowgain  = gUnpacker.get_data_id("CFT", "lowgain");
//     // TDC gate range
//     static const int tdc_min = gUser.GetParameter("CFT_TDC", 0);
//     static const int tdc_max = gUser.GetParameter("CFT_TDC", 1);

//     // SequentialID
//     int cft_t_id    = gHist.getSequentialID(kCFT, 0, kTDC,     1);
//     int cft_t_2d_id = gHist.getSequentialID(kCFT, 0, kTDC2D,   1);

//     int cft_tot_id     = gHist.getSequentialID(kCFT, 0, kADC,    1);
//     int cft_tot_2d_id  = gHist.getSequentialID(kCFT, 0, kADC2D,  1);
//     int cft_ctot_id    = gHist.getSequentialID(kCFT, 0, kADC,   11);
//     int cft_ctot_2d_id = gHist.getSequentialID(kCFT, 0, kADC2D, 11);

//     int cft_hg_id = gHist.getSequentialID(kCFT, 0, kHighGain, 1);
//     int cft_pe_id = gHist.getSequentialID(kCFT, 0, kPede    , 1);
//     int cft_lg_id = gHist.getSequentialID(kCFT, 0, kLowGain,  1);

//     int cft_hg_2d_id = gHist.getSequentialID(kCFT, 0, kHighGain, 11);
//     int cft_pe_2d_id = gHist.getSequentialID(kCFT, 0, kPede    , 11);
//     int cft_lg_2d_id = gHist.getSequentialID(kCFT, 0, kLowGain,  11);

//     int cft_chg_id = gHist.getSequentialID(kCFT, 0, kHighGain, 21);
//     int cft_clg_id = gHist.getSequentialID(kCFT, 0, kLowGain,  21);

//     int cft_chg_2d_id = gHist.getSequentialID(kCFT, 0, kHighGain, 31);
//     int cft_clg_2d_id = gHist.getSequentialID(kCFT, 0, kLowGain,  31);

//     int cft_hg_tot_id = gHist.getSequentialID(kCFT, 0, kHighGain, 41);
//     int cft_lg_tot_id = gHist.getSequentialID(kCFT, 0, kLowGain,  41);

//     int cft_hit_id     = gHist.getSequentialID(kCFT, 0, kHitPat,  1);
//     int cft_chit_id    = gHist.getSequentialID(kCFT, 0, kHitPat, 11);
//     int cft_chitbgo_id = gHist.getSequentialID(kCFT, 0, kMulti,  21);
//     int cft_mul_id     = gHist.getSequentialID(kCFT, 0, kMulti,   1);
//     int cft_cmul_id    = gHist.getSequentialID(kCFT, 0, kMulti,  11);
//     int cft_cmulbgo_id = gHist.getSequentialID(kCFT, 0, kMulti,  21);

//     int cft_cl_hgadc_id =gHist.getSequentialID(kCFT, kCluster, kHighGain, 1);
//     int cft_cl_lgadc_id =gHist.getSequentialID(kCFT, kCluster, kLowGain,  1);
//     int cft_cl_tdc_id   =gHist.getSequentialID(kCFT, kCluster, kTDC,      1);

//     int cft_cl_hgadc2d_id =gHist.getSequentialID(kCFT, kCluster, kHighGain, 11);
//     int cft_cl_lgadc2d_id =gHist.getSequentialID(kCFT, kCluster, kLowGain, 11);
//     int cft_cl_tdc2d_id =gHist.getSequentialID(kCFT,kCluster, kTDC2D, 1);

//     int multiplicity[NumOfLayersCFT] ;
//     int cmultiplicity[NumOfLayersCFT];
//     int cbgomultiplicity[NumOfLayersCFT];

//     for(int l=0; l<NumOfLayersCFT; ++l){
//       multiplicity[l]     = 0;
//       cmultiplicity[l]    = 0;
//       cbgomultiplicity[l] = 0;
//       for(int i = 0; i<NumOfSegCFT[l]; ++i){
//         int nhit_l = gUnpacker.get_entries(k_device, l, i, 0, k_leading );
//         int nhit_t = gUnpacker.get_entries(k_device, l, i, 0, k_trailing );
//         int hit_l_max = 0;
//         int hit_t_max = 0;
//         if(nhit_l==0)
// 	  continue;
// 	for(int m = 0; m<nhit_l; ++m){
// 	  int tdc = gUnpacker.get(k_device, l, i, 0, k_leading, m);
// 	  hptr_array[cft_t_id+l]->Fill(tdc);
// 	  hptr_array[cft_t_2d_id+l]->Fill(i, tdc);
// 	  hptr_array[cft_hit_id+l]->Fill(i);
// 	  ++multiplicity[l];
// 	  if(tdc_min < tdc && tdc < tdc_max){
// 	    ++cmultiplicity[l];
// 	    if( bgo_is_hit )
// 	      ++cbgomultiplicity[l];
// 	    hptr_array[cft_chit_id+l]->Fill(i);
// 	    if( bgo_is_hit )
// 	      hptr_array[cft_chitbgo_id+l]->Fill(i);
// 	  }
// 	}

// 	if(nhit_l != 0){
// 	  hit_l_max = gUnpacker.get(k_device, l, i, 0, k_leading,  nhit_l - 1);
// 	}
// 	if(nhit_t != 0){
// 	  hit_t_max = gUnpacker.get(k_device, l, i, 0, k_trailing, nhit_t - 1);
// 	}
// 	if(nhit_l == nhit_t && hit_l_max > hit_t_max){
// 	  for(int m = 0; m<nhit_l; ++m){
// 	    int tdc = gUnpacker.get(k_device, l, i, 0, k_leading, m);
// 	    int tdc_t = gUnpacker.get(k_device, l, i, 0, k_trailing, m);
// 	    int tot = tdc - tdc_t;
// 	    hptr_array[cft_tot_id+l]->Fill(tot);
// 	    hptr_array[cft_tot_2d_id+l]->Fill(i, tot);

// 	    if(tdc_min < tdc && tdc < tdc_max){
// 	      hptr_array[cft_ctot_id+l]->Fill(tot);
// 	      hptr_array[cft_ctot_2d_id+l]->Fill(i, tot);

// 	      if(m!= nhit_l-1 )continue;
// 	      for(int j=0; j<2; j++){
// 		if(j==0){
// 		  int nhit_hg = gUnpacker.get_entries(k_device, l, i, 0, k_highgain);
// 		  if(nhit_hg==0)continue;
// 		  int adc_hg = gUnpacker.get(k_device, l, i, 0, k_highgain, 0);
// 		  hptr_array[cft_chg_id+l]->Fill(adc_hg);
// 		  hptr_array[cft_chg_2d_id+l]->Fill(i, adc_hg);
// 		  hptr_array[cft_hg_tot_id+l]->Fill(adc_hg, tot);
// 		}
// 		if(j==1){
// 		  int nhit_lg = gUnpacker.get_entries(k_device, l, i, 0, k_lowgain);
// 		  if(nhit_lg==0)continue;
// 		  int adc_lg = gUnpacker.get(k_device, l, i, 0, k_lowgain, 0);
// 		  hptr_array[cft_clg_id+l]->Fill(adc_lg);
// 		  hptr_array[cft_clg_2d_id+l]->Fill(i, adc_lg);
// 		  hptr_array[cft_lg_tot_id+l]->Fill(adc_lg, tot);
// 		}
// 	      }
// 	    }
// 	  }
// 	}
//       }

//       hptr_array[cft_mul_id+l]->Fill(multiplicity[l]);
//       hptr_array[cft_cmul_id+l]->Fill(cmultiplicity[l]);
//       hptr_array[cft_cmulbgo_id+l]->Fill(cbgomultiplicity[l]);

//       for(int i = 0; i<NumOfSegCFT[l]; ++i){
//         int nhit_hg = gUnpacker.get_entries(k_device, l, i, 0, k_highgain);
//         if(nhit_hg==0)continue;
// 	for(int m = 0; m<nhit_hg; ++m){
// 	  int adc_hg = gUnpacker.get(k_device, l, i, 0, k_highgain, m);
// 	  hptr_array[cft_hg_id+l]->Fill(adc_hg);
// 	  hptr_array[cft_hg_2d_id+l]->Fill(i, adc_hg);
// 	  double pedeCFT = 0.;
// 	  double gainCFT = 100.;
// 	  //                pedeCFT = hman->GetPedestal(DetIdCFT,layer,i,0);
// 	  //                gainCFT = hman->GetGain(DetIdCFT,layer,i,0);
// 	  double adc_pe = ((double)adc_hg - pedeCFT)/gainCFT;
// 	  hptr_array[cft_pe_id+l]->Fill(adc_pe);
// 	  hptr_array[cft_pe_2d_id+l]->Fill(i, adc_pe);
// 	}
//       }
//       for(int i = 0; i<NumOfSegCFT[l]; ++i){
//         int nhit_lg = gUnpacker.get_entries(k_device, l, i, 0, k_lowgain );
//         if(nhit_lg==0)continue;
// 	for(int m = 0; m<nhit_lg; ++m){
// 	  int adc_lg = gUnpacker.get(k_device, l, i, 0, k_lowgain, m);
// 	  hptr_array[cft_lg_id+l]->Fill(adc_lg);
// 	  hptr_array[cft_lg_2d_id+l]->Fill(i, adc_lg);
// 	}
//       }
//     }
//     hptr_array[cft_mul_id+NumOfLayersCFT]->Fill(multiplicity[0] + multiplicity[2]
// 						+ multiplicity[4] + multiplicity[6]);
//     hptr_array[cft_mul_id+NumOfLayersCFT + 1]->Fill(multiplicity[1] + multiplicity[3]
// 						    + multiplicity[5] + multiplicity[7]);
//     hptr_array[cft_cmul_id+NumOfLayersCFT]->Fill(cmultiplicity[0] + cmultiplicity[2]
// 						 + cmultiplicity[4] + cmultiplicity[6]);
//     hptr_array[cft_cmul_id+NumOfLayersCFT + 1]->Fill(cmultiplicity[1] + cmultiplicity[3]
// 						     + cmultiplicity[5] + cmultiplicity[7]);


  //   //clustering analysis
  // hodoAna->DecodeCFTHits(rawData);
  // // Fiber Cluster
  // for(int p = 0; p<NumOfPlaneCFT; ++p){
  //   //int nhits = hodoAna->GetNHitsCFT(p);
  //   //std::cout<<"plane= "<<p<<",NHits="<<nhits<<std::endl;

  //   //hodoAna->TimeCutCFT(p, -30, 30); // CATCH@J-PARC
  //   hodoAna->AdcCutCFT(p, 0, 4000); // CATCH@J-PARC
  //   //hodoAna->AdcCutCFT(p, 50, 4000); // CATCH@J-PARC  for proton
  //   //hodoAna->WidthCutCFT(p, 60, 300); // pp scattering
  //   //hodoAna->WidthCutCFT(p, 30, 300); // cosmic ray

  //   int ncl = hodoAna->GetNClustersCFT(p);
  //   //std::cout<<"plane= "<<p<<",NClusters="<<ncl<<std::endl;
  //   for(int i=0; i<ncl; ++i){
  //     FiberCluster *cl = hodoAna->GetClusterCFT(p,i);
  //     if(!cl) continue;
  //     //      double size  = cl->ClusterSize();
  //     //double seg = cl->MeanSeg();
  //     //double ctime = cl->CMeanTime();
  //     //double width = -cl->minWidth();
  //     //double width = cl->Width();
  //     //double sumADC= cl->SumAdcLow();
  //     //double sumMIP= cl->SumMIPLow();
  //     //double sumdE = cl->SumdELow();
  //     //double r     = cl->MeanPositionR();
  //     //double phi   = cl->MeanPositionPhi();
  //     int Mseg  = cl->MaxSeg();
  //     int MADCHi  = cl->MaxAdcHi();
  //     int MADCLow = cl->MaxAdcLow();
  //     //std::cout<<"cluster="<<i<<" , Maxseg="<<Mseg<<" , MaxADC"<<MADCHi<<std::endl;
  //     hptr_array[cft_cl_hgadc_id + p] ->Fill(MADCHi);
  //     hptr_array[cft_cl_lgadc_id + p] ->Fill(MADCLow);
  //     hptr_array[cft_cl_hgadc2d_id + p] ->Fill(Mseg, MADCHi);
  //     hptr_array[cft_cl_lgadc2d_id + p] ->Fill(Mseg, MADCLow);
  //     int fmulti =hodoAna->GetNHitsCFT(p);
  //     for(int j=0; j<fmulti; ++j){
  // 	FiberHit *fhit = hodoAna->GetHitCFT(p,j);
  // 	int seg_temp = fhit->PairId();
  // 	if(seg_temp == Mseg){
  // 	  int Mhit =fhit ->GetNumOfHit();
  // 	  for(int k=0; k<Mhit; ++k){
  // 	    double Mtime =fhit->GetLeading(k);
  // 	    hptr_array[cft_cl_tdc_id + p] ->Fill(Mtime);
  // 	    hptr_array[cft_cl_tdc2d_id + p] ->Fill(Mseg, Mtime);
  // 	  }
  // 	}
  //     }
  //   }
  // }

  // delete rawData;
  // delete hodoAna;

// #if 0
//     // Debug, dump data relating this detector
//     gUnpacker.dump_data_device(k_device);
// #endif
//   }//CFT

// #if DEBUG
//   std::cout << __FILE__ << " " << __LINE__ << std::endl;
// #endif

  return 0;
} //process_event()


} //analyzer
