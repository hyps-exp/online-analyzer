// Author: Tomonori Takahashi

#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

#include <TGFileBrowser.h>
#include <TH1.h>
#include <TString.h>
#include <TFile.h>
#include <TCanvas.h>

#include "user_analyzer.hh"
#include "Controller.hh"
#include "ConfMan.hh"
#include "HistMaker.hh"
#include "MacroBuilder.hh"
#include "DetectorID.hh"
#include "RawData.hh"
#include "HodoAnalyzer.hh"
#include "Hodo2Hit.hh"
#include "HodoCluster.hh"
#include "BH2Cluster.hh"
#include "DCDriftParamMan.hh"
#include "DCGeomMan.hh"
#include "DCTdcCalibMan.hh"
#include "HodoParamMan.hh"
#include "HodoPHCMan.hh"
#include "MsTParamMan.hh"
#include "UserParamMan.hh"

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

#if 0
    const int n_ref = 4;
    const int ref_seg[n_ref] = {
      5, 13, 17, 21
    };
#endif

    const int n_ref = 24;
    const int ref_seg[n_ref] = {
      1, 2, 3, 4, 5, 6, 7, 8,
      9, 10, 11, 12, 13, 14, 15, 16,
      17, 18, 19, 20, 21, 22, 23, 24
    };

    TH1* ref_hist[n_ref];
    TH1* ref_hist_t0[n_ref];
    std::string out_pdf = "";

    const double mst_tdc_min = 180*1000;
    const double mst_tdc_max = 220*1000;

    RawData      *rawData = nullptr;
    HodoAnalyzer *hodoAna = nullptr;

    const double to_tdc = 1./(-0.000939002);
   }

 //____________________________________________________________________________
 int
 process_begin(const std::vector<std::string>& argv)
 {
   ConfMan& gConfMan = ConfMan::GetInstance();
   gConfMan.Initialize(argv);
   gConfMan.InitializeParameter<HodoParamMan>("HDPRM");
   gConfMan.InitializeParameter<HodoPHCMan>("HDPHC");
   gConfMan.InitializeParameter<DCGeomMan>("DCGEOM");
   gConfMan.InitializeParameter<DCTdcCalibMan>("TDCCALIB");
   gConfMan.InitializeParameter<DCDriftParamMan>("DRFTPM");
   gConfMan.InitializeParameter<UserParamMan>("USER");
   //  GUnpacker::get_instance().set_decode_mode(false);

   std::string runno      = argv.at(argv.size()-1);
   std::string ref_runno  = argv.at(argv.size()-2);
   std::string result_dir = argv.at(argv.size()-3);

   UserParamMan::GetInstance();

   // Reference
   const std::string ref_file = result_dir + "/rootfile/run0" +ref_runno + ".root";
   new TFile(ref_file.c_str());
   if(!gFile->IsOpen()){
     std::cerr << "#E: " << __func__ << "() "
	       << "Rootfile open error (" << ref_file << ")"
	       << std::endl;
     return -1;
   }
   for(int i = 0; i<n_ref; ++i){
     ref_hist[i]    = dynamic_cast<TH1*>(gFile->Get(Form("MsT_TOF_TDC_%d",    ref_seg[i])));
     ref_hist_t0[i] = dynamic_cast<TH1*>(gFile->Get(Form("MsT_T0_TOF_TDC_%d", ref_seg[i])));
   }

   // Output root file
   const std::string out_file = result_dir + "/rootfile/run" + runno + ".root";
   new TFile(out_file.c_str(),"recreate");

   // pdf
   out_pdf = result_dir + "/pdf/run" + runno + "_ref" + ref_runno + ".pdf";

   // HistMaker
   HistMaker& gHist = HistMaker::getInstance();
   gHist.createMsT();
   gHist.createMsT_T0();
   if(0 != gHist.setHistPtr(hptr_array)){ return -1; }

   // mst-tof ________________________________________
   const int mst_tof_id    = gHist.getSequentialID( kMsT,    0, kTDC);
   const int mst_tof_t0_id = gHist.getSequentialID( kMsT_T0, 0, kTDC);
   for(int i = 0; i<n_ch_tof; ++i){
     hptr_array[mst_tof_id + i]->Rebin(5);
     hptr_array[mst_tof_t0_id + i]->Rebin(5);
   }// for(i)

   return 0;
 }

 //____________________________________________________________________________
 int
 process_end()
 {
   HistMaker& gHist = HistMaker::getInstance();

 #if 0
   const int tgt_seg_id[] =
     {
       gHist.getSequentialID(kMsT, 0, kTDC, ref_seg[0]),
       gHist.getSequentialID(kMsT, 0, kTDC, ref_seg[1]),
       gHist.getSequentialID(kMsT, 0, kTDC, ref_seg[2]),
       gHist.getSequentialID(kMsT, 0, kTDC, ref_seg[3])
     };

   TCanvas *c1 = new TCanvas("c1", "c1", 1200, 2400);
   c1->Divide(2,2);
   for(int i = 0; i<n_ref; ++i){
     c1->cd(i+1);
     double scale = hptr_array[tgt_seg_id[i]]->GetEntries()/ref_hist[i]->GetEntries();
     ref_hist[i]->GetXaxis()->SetRangeUser(180*1000, 220*1000);
     ref_hist[i]->SetLineColor(2);
     ref_hist[i]->Scale(scale);
     ref_hist[i]->Draw("HIST");

     hptr_array[tgt_seg_id[i]]->Draw("same");
   }// for(i)

   c1->Print(out_pdf.c_str());
 #endif

#if 1
   // Raw MsT TDC _______________________________________________________________________________
   {
     int n_canvas = 3;
     int n_hist   = 8;
     int base_id = gHist.getSequentialID(kMsT, 0, kTDC, 1);
     for(int c = 0; c<n_canvas; ++c){
       TCanvas *c1 = new TCanvas(Form("c%d",c+1), Form("c%d", c+1), 1200, 2400);
       c1->Divide(2,4);
       for(int i = 0; i<n_hist; ++i){
	 c1->cd(i+1);
	 //      double scale = hptr_array[base_id + n_hist*c + i]->GetEntries()/ref_hist[n_hist*c + i]->GetEntries();
	 double scale = hptr_array[base_id + n_hist*c + i]->GetMaximum()/ref_hist[n_hist*c + i]->GetMaximum();
	 ref_hist[n_hist*c + i]->GetXaxis()->SetRangeUser(mst_tdc_min, mst_tdc_max);
	 ref_hist[n_hist*c + i]->SetLineColor(2);
	 ref_hist[n_hist*c + i]->Scale(scale);
	 ref_hist[n_hist*c + i]->Draw("HIST");

	 hptr_array[base_id + n_hist*c + i]->Draw("same");
       }

       std::string out_path = out_pdf;
       if(c==0) out_path += "(";
       //       if(c==2) out_path += ")";
       c1->Print(out_path.c_str());
     }
   }

   // Raw MsT-T0 TDC _____________________________________________________________________________
   {
     int n_canvas = 3;
     int n_hist   = 8;
     int base_id = gHist.getSequentialID(kMsT_T0, 0, kTDC, 1);
     for(int c = 0; c<n_canvas; ++c){
       TCanvas *c1 = new TCanvas(Form("c%d",c+1+3), Form("c%d", c+1+3), 1200, 2400);
       c1->Divide(2,4);
       for(int i = 0; i<n_hist; ++i){
	 c1->cd(i+1);
	 //      double scale = hptr_array[base_id + n_hist*c + i]->GetEntries()/ref_hist[n_hist*c + i]->GetEntries();
	 double scale = hptr_array[base_id + n_hist*c + i]->GetMaximum()/ref_hist_t0[n_hist*c + i]->GetMaximum();
	 ref_hist_t0[n_hist*c + i]->GetXaxis()->SetRangeUser(mst_tdc_min, mst_tdc_max);
	 ref_hist_t0[n_hist*c + i]->SetLineColor(2);
	 ref_hist_t0[n_hist*c + i]->Scale(scale);
	 ref_hist_t0[n_hist*c + i]->Draw("HIST");

	 hptr_array[base_id + n_hist*c + i]->Draw("same");
       }

       std::string out_path = out_pdf;
       //       if(c==0) out_path += "(";
       if(c==2) out_path += ")";
       c1->Print(out_path.c_str());
     }
   }

#endif

   if(hodoAna) {delete hodoAna; hodoAna = nullptr;}
   if(rawData) {delete rawData; rawData = nullptr;}

   return 0;
 }

 //____________________________________________________________________________
 int
 process_event()
 {
   static UnpackerManager& gUnpacker = GUnpacker::get_instance();
   HistMaker & gHist = HistMaker::getInstance();

   if(hodoAna) {delete hodoAna; hodoAna = nullptr;}
   if(rawData) {delete rawData; rawData = nullptr;}
   rawData = new RawData;
   hodoAna = new HodoAnalyzer;
   rawData->DecodeHits();
   hodoAna->DecodeBH2Hits(rawData);
   BH2Cluster *bh2cl_time0 = hodoAna->GetTime0BH2Cluster();

   // Raw MsT TDC dist. _______________________________________________________
   // trigger flag
   bool pipi_flag = false;
   const int tflag_tdc_min = 900;
   const int tflag_tdc_max = 1000;
   {
     static const int k_device = gUnpacker.get_device_id("TFlag");
     static const int k_tdc    = gUnpacker.get_data_id("TFlag", "tdc");

     int nhit = gUnpacker.get_entries( k_device, 0, trigger::kBeamPiPS, 0, k_tdc );
     for(int m = 0; m<nhit; ++m){
       int tdc = gUnpacker.get( k_device, 0, trigger::kBeamPiPS, 0, k_tdc, m );
       if(tflag_tdc_min < tdc && tdc < tflag_tdc_max) pipi_flag = true;
     }// for(m)
   }

   if(!pipi_flag) return 0;

   // MsT TOF
   {
     static const int k_device = gUnpacker.get_device_id("MsT");
     static const int k_tof    = gUnpacker.get_plane_id("MsT", "TOF");
     static const int k_tdc    = 0;

     static const int mst_tof_id    = gHist.getSequentialID( kMsT,    0, kTDC);
     static const int mst_tof_t0_id = gHist.getSequentialID( kMsT_T0, 0, kTDC);

     // TOF
     for(int i = 0; i<n_ch_tof; ++i){
       int nhit = gUnpacker.get_entries( k_device, k_tof, i, 0, k_tdc );
       for(int m = 0; m<nhit; ++m){
	 int tdc = gUnpacker.get( k_device, k_tof, i, 0, k_tdc, m );
	 if(mst_tdc_min < tdc && tdc < mst_tdc_max){
	   hptr_array[mst_tof_id + i]->Fill(tdc);

	   if(bh2cl_time0){
	     int t0 = static_cast<int>(bh2cl_time0->Time0()*to_tdc);
	     hptr_array[mst_tof_t0_id + i]->Fill(tdc-t0);
	   }
	 }
       }// for(m)
     }// for(i)
   }

   return 0;
 }

}
