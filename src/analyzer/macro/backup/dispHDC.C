// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispHDC()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  int n_layer = 4;

  // draw TDC
  TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  c->Clear();
  c->Divide(3,2);
  int base_id = HistMaker::getUniqueID(kHDC, 0, kTDC);
  for(int i = 0; i<n_layer; ++i){
    c->cd(i+1);
    TH1 *h = (TH1*)GHist::get(base_id + i);
    //    h->GetXaxis()->SetRangeUser(256,1000);
    h->Draw();
  }

  c->Update();

  // draw TDC1st
  c = (TCanvas*)gROOT->FindObject("c2");
  c->Clear();
  c->Divide(3,2);
  int base_id = HistMaker::getUniqueID(kHDC, 0, kTDC2D);
  for(int i = 0; i<n_layer; ++i){
    c->cd(i+1);
    TH1 *h = (TH1*)GHist::get(base_id + i);
    //    h->GetXaxis()->SetRangeUser(256,1000);
    h->Draw();
  }

  c->Update();

  // draw HitPat
  c = (TCanvas*)gROOT->FindObject("c3");
  c->Clear();
  c->Divide(3,2);
  int base_id = HistMaker::getUniqueID(kHDC, 0, kHitPat);
  for(int i = 0; i<n_layer; ++i){
    c->cd(i+1);
    GHist::get(base_id + i)->Draw();
  }

  c->Update();

  // draw Multi
  c = (TCanvas*)gROOT->FindObject("c4");
  c->Clear();
  c->Divide(3,2);
  int base_id = HistMaker::getUniqueID(kHDC, 0, kMulti);
  for(int i = 0; i<n_layer; ++i){
    c->cd(i+1);
    TH1 *h_wot = (TH1*)GHist::get(base_id + i)->Clone();
    TH1 *h_wt  = (TH1*)GHist::get(base_id + i + n_layer)->Clone();
    h_wot->SetMaximum(h_wt->GetMaximum()*1.1);
    h_wot->Draw();
    h_wt->SetLineColor(2);
    h_wt->Draw("same");
  }

  c->Update();

  c->cd(0);

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
