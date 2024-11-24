// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispSAC()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // TDC gate range
  const UserParamMan& gUser = UserParamMan::GetInstance();
  static const unsigned int tdc_min = gUser.GetParameter("SAC_TDC", 0);
  static const unsigned int tdc_max = gUser.GetParameter("SAC_TDC", 1);

{
  const int n_seg_ac = 4;

  // draw SAC ADC
  TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  c->Clear();
  c->Divide(2,2);
  int idac[n_seg_ac] = {
    HistMaker::getUniqueID(kSAC,  0, kADC, 1),
    HistMaker::getUniqueID(kSAC,  0, kADC, 2),
    HistMaker::getUniqueID(kSAC,  0, kADC, 3),
    HistMaker::getUniqueID(kSAC,  0, kADC, 4),
  };
  int idac_wtdc[n_seg_ac] = {
    HistMaker::getUniqueID(kSAC,  0, kADCwTDC, 1),
    HistMaker::getUniqueID(kSAC,  0, kADCwTDC, 2),
    HistMaker::getUniqueID(kSAC,  0, kADCwTDC, 3),
    HistMaker::getUniqueID(kSAC,  0, kADCwTDC, 4),
  };

  for(int i = 0; i<n_seg_ac; ++i){
    c->cd(i+1);
    gPad->SetLogy();
    TH1 *h = (TH1*)GHist::get(idac[i]);
    h->GetXaxis()->SetRangeUser(0,4000);
    h->Draw();
    TH1 *hh = (TH1*)GHist::get(idac_wtdc[i]);
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }

  c->Update();

  c->cd(0);
}

{
  const int n_seg_ac = 4;

  // draw SAC TDC for Rooms
  TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
  c->Clear();
  c->Divide(2,2);
  int idac[n_seg_ac] = {
    HistMaker::getUniqueID(kSAC,  0, kTDC, 1),
    HistMaker::getUniqueID(kSAC,  0, kTDC, 2),
    HistMaker::getUniqueID(kSAC,  0, kTDC, 3),
    HistMaker::getUniqueID(kSAC,  0, kTDC, 4),
  };

  for(int i = 0; i<n_seg_ac; ++i){
    c->cd(i+1);
    //    gPad->SetLogy();
    TH1 *h = (TH1*)GHist::get(idac[i]);
    h->GetXaxis()->SetRangeUser(tdc_min-100, tdc_max+100);
    h->Draw();
  }

  c->Update();

  c->cd(0);
}

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
