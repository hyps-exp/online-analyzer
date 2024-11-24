// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispACs_SFV()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const int n_seg_ac = 8;

  // draw ACs ADC & TDC
  TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  c->Clear();
  c->Divide(4,2);
  int idac[n_seg_ac];
  idac[0] = HistMaker::getUniqueID(kBAC_SAC,  0, kADC, 3); // bac1
  idac[1] = HistMaker::getUniqueID(kBAC_SAC,  0, kADC, 4); // bac2
  idac[2] = HistMaker::getUniqueID(kBAC_SAC,  0, kADC, 5); // sac1
  idac[3] = HistMaker::getUniqueID(kSFV_SAC3, 0, kADC, 7); // sac3

  idac[4] = HistMaker::getUniqueID(kBAC_SAC,  0, kTDC, 3); // bac1
  idac[5] = HistMaker::getUniqueID(kBAC_SAC,  0, kTDC, 4); // bac2
  idac[6] = HistMaker::getUniqueID(kBAC_SAC,  0, kTDC, 5); // sac1
  idac[7] = HistMaker::getUniqueID(kSFV_SAC3, 0, kTDC, 7); // sac3
  for(int i = 0; i<n_seg_ac; ++i){
    c->cd(i+1);
    gPad->SetLogy();
    TH1 *h = (TH1*)GHist::get(idac[i]);
    h->GetXaxis()->SetRangeUser(0,4000);
    h->Draw();
  }

  c->Update();

  // SFV ADC and TDC
  int n_seg_sfv = 6;

  TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
  c->Clear();
  c->Divide(6,2);
  int base_id = HistMaker::getUniqueID(kSFV_SAC3, 0, kADC);
  for(int i = 0; i<n_seg_sfv; ++i){
    c->cd(i+1);
    gPad->SetLogy();
    TH1 *h = (TH1*)GHist::get(base_id + i);
    h->GetXaxis()->SetRangeUser(0,4000);
    h->Draw();
  }

  base_id = HistMaker::getUniqueID(kSFV_SAC3, 0, kTDC);
  for(int i = 0; i<n_seg_sfv; ++i){
    c->cd(i+7);
    gPad->SetLogy();
    TH1 *h = (TH1*)GHist::get(base_id + i);
    h->GetXaxis()->SetRangeUser(0,4000);
    h->Draw();
  }

  c->Update();

  c->cd(0);

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
