// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispACs_E07()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const int n_hist = 4; // bac1, bac2, pvac, fac
  // const int n_seg_bac  = 4+2;
  // const int n_seg_pvac = 4+1;
  // const int n_seg_fac  = 6+1;
  // const int n_seg_sac1 =   1;
  
  int adc_id[n_hist] = {
    HistMaker::getUniqueID(kBAC_E07,  0, kADC, 1),// BAC1
    HistMaker::getUniqueID(kBAC_E07,  0, kADC, 2),// BAC2
    HistMaker::getUniqueID(kPVAC,     0, kADC, 1),// PVAC
    HistMaker::getUniqueID(kFAC,      0, kADC, 1),// FAC
  };

  int tdc_id[n_hist] = {
    HistMaker::getUniqueID(kBAC_E07,  0, kTDC, 1),// BAC1
    HistMaker::getUniqueID(kBAC_E07,  0, kTDC, 2),// BAC2
    HistMaker::getUniqueID(kPVAC,     0, kTDC, 1),// PVAC
    HistMaker::getUniqueID(kFAC,      0, kTDC, 1),// FAC
  };

  // int bac_id[n_seg_bac*2] = {
  //   HistMaker::getUniqueID(kBAC_E07,  0, kADC, 1),// BAC1-SUM
  //   HistMaker::getUniqueID(kBAC_E07,  0, kADC, 3),// BAC1-1
  //   HistMaker::getUniqueID(kBAC_E07,  0, kADC, 4),// BAC1-2
  //   HistMaker::getUniqueID(kBAC_E07,  0, kADC, 2),// BAC2-SUM
  //   HistMaker::getUniqueID(kBAC_E07,  0, kADC, 5),// BAC2-1
  //   HistMaker::getUniqueID(kBAC_E07,  0, kADC, 6),// BAC2-2
  //   HistMaker::getUniqueID(kBAC_E07,  0, kTDC, 1),// BAC1-SUM
  //   HistMaker::getUniqueID(kBAC_E07,  0, kTDC, 3),// BAC1-1  
  //   HistMaker::getUniqueID(kBAC_E07,  0, kTDC, 4),// BAC1-2  
  //   HistMaker::getUniqueID(kBAC_E07,  0, kTDC, 2),// BAC2-SUM
  //   HistMaker::getUniqueID(kBAC_E07,  0, kTDC, 5),// BAC2-1  
  //   HistMaker::getUniqueID(kBAC_E07,  0, kTDC, 6) // BAC2-2  
  // };

  // int pvac_id[n_seg_pvac*2] = {
  //   HistMaker::getUniqueID(kPVAC,  0, kADC, 1),// PVAC-SUM
  //   HistMaker::getUniqueID(kPVAC,  0, kADC, 2),// PVAC-1  
  //   HistMaker::getUniqueID(kPVAC,  0, kADC, 3),// PVAC-2  
  //   HistMaker::getUniqueID(kPVAC,  0, kADC, 4),// PVAC-3  
  //   HistMaker::getUniqueID(kPVAC,  0, kADC, 5),// PVAC-4  
  //   HistMaker::getUniqueID(kPVAC,  0, kTDC, 1),// PVAC-SUM
  //   HistMaker::getUniqueID(kPVAC,  0, kTDC, 2),// PVAC-1  
  //   HistMaker::getUniqueID(kPVAC,  0, kTDC, 3),// PVAC-2  
  //   HistMaker::getUniqueID(kPVAC,  0, kTDC, 4),// PVAC-3  
  //   HistMaker::getUniqueID(kPVAC,  0, kTDC, 5),// PVAC-4  
  // };

  // int fac_id[n_seg_fac*2] = {
  //   HistMaker::getUniqueID(kFAC,  0, kADC, 1),// FAC-SUM
  //   HistMaker::getUniqueID(kFAC,  0, kADC, 2),// FAC-1	
  //   HistMaker::getUniqueID(kFAC,  0, kADC, 3),// FAC-2	
  //   HistMaker::getUniqueID(kFAC,  0, kADC, 4),// FAC-3	
  //   HistMaker::getUniqueID(kFAC,  0, kADC, 5),// FAC-4	
  //   HistMaker::getUniqueID(kFAC,  0, kADC, 6),// FAC-5	
  //   HistMaker::getUniqueID(kFAC,  0, kADC, 7),// FAC-6	
  //   HistMaker::getUniqueID(kFAC,  0, kTDC, 1),// FAC-SUM
  //   HistMaker::getUniqueID(kFAC,  0, kTDC, 2),// FAC-1	
  //   HistMaker::getUniqueID(kFAC,  0, kTDC, 3),// FAC-2	
  //   HistMaker::getUniqueID(kFAC,  0, kTDC, 4),// FAC-3	
  //   HistMaker::getUniqueID(kFAC,  0, kTDC, 5),// FAC-4	
  //   HistMaker::getUniqueID(kFAC,  0, kTDC, 6),// FAC-5	
  //   HistMaker::getUniqueID(kFAC,  0, kTDC, 7) // FAC-6	
  // };

  // int sac1_id[n_seg_sac1*2] = {
  //   HistMaker::getUniqueID(kSAC1,  0, kADC, 1),// SAC1-SUM
  //   // HistMaker::getUniqueID(kSAC1,  0, kADC, 2),// SAC1-1	
  //   // HistMaker::getUniqueID(kSAC1,  0, kADC, 3),// SAC1-2	
  //   // HistMaker::getUniqueID(kSAC1,  0, kADC, 4),// SAC1-3	
  //   // HistMaker::getUniqueID(kSAC1,  0, kADC, 5),// SAC1-4	
  //   // HistMaker::getUniqueID(kSAC1,  0, kADC, 6),// SAC1-5	
  //   HistMaker::getUniqueID(kSAC1,  0, kTDC, 1),// SAC1-SUM
  //   // HistMaker::getUniqueID(kSAC1,  0, kTDC, 2),// SAC1-1	
  //   // HistMaker::getUniqueID(kSAC1,  0, kTDC, 3),// SAC1-2	
  //   // HistMaker::getUniqueID(kSAC1,  0, kTDC, 4),// SAC1-3	
  //   // HistMaker::getUniqueID(kSAC1,  0, kTDC, 5),// SAC1-4	
  //   // HistMaker::getUniqueID(kSAC1,  0, kTDC, 6),// SAC1-5	
  // };

  { // draw ACs ADC
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(2,2);
    for(int i = 0; i<n_hist; ++i){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get(adc_id[i]);
      h->GetXaxis()->SetRangeUser(0,4000);
      h->Draw();
    }
  }
  c->Update();

  { // draw ACs TDC
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(2,2);
    for(int i = 0; i<n_hist; ++i){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get(tdc_id[i]);
      h->GetXaxis()->SetRangeUser(0,4000);
      h->Draw();
    }
  }
  c->Update();

  // { // draw BAC ADC & TDC
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  //   c->Clear();
  //   c->Divide(3,4);
  //   for(int i = 0; i<n_seg_bac*2; ++i){
  //     c->cd(i+1);
  //     gPad->SetLogy();
  //     TH1 *h = (TH1*)GHist::get(bac_id[i]);
  //     h->GetXaxis()->SetRangeUser(0,4000);
  //     h->Draw();
  //   }
  // }
  // c->Update();

  // { // draw PVAC ADC & TDC
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
  //   c->Clear();
  //   c->Divide(3,4);
  //   for(int i = 0; i<n_seg_pvac*2; ++i){
  //     if(i<n_seg_pvac) c->cd(i+1);
  //     else             c->cd(i+2);
  //     gPad->SetLogy();
  //     TH1 *h = (TH1*)GHist::get(pvac_id[i]);
  //     h->GetXaxis()->SetRangeUser(0,4000);
  //     h->Draw();
  //   }
  // }
  // c->Update();

  // { // draw FAC ADC & TDC
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
  //   c->Clear();
  //   c->Divide(4,4);
  //   for(int i = 0; i<n_seg_fac*2; ++i){
  //     if(i<n_seg_fac) c->cd(i+1);
  //     else            c->cd(i+2);
  //     gPad->SetLogy();
  //     TH1 *h = (TH1*)GHist::get(fac_id[i]);
  //     h->GetXaxis()->SetRangeUser(0,4000);
  //     h->Draw();
  //   }
  // }
  // c->Update();

  // { // draw SAC1 ADC & TDC
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
  //   c->Clear();
  //   c->Divide(2,2);
  //   for(int i = 0; i<n_seg_sac1*2; ++i){
  //     c->cd(i+1);
  //     gPad->SetLogy();
  //     TH1 *h = (TH1*)GHist::get(sac1_id[i]);
  //     h->GetXaxis()->SetRangeUser(0,4000);
  //     h->Draw();
  //   }
  // }
  // c->Update();

  c->cd(0);

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
