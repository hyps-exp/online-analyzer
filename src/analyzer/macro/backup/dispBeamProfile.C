// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispBeamProfile()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  int n_hist = 8;

  // X position
  TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  c->Clear();
  c->Divide(4,2);
  int base_id = HistMaker::getUniqueID(kMisc, 0, kHitPat);
  for(int i = 0; i<n_hist; ++i){
    c->cd(i+1);
    TH1 *h = (TH1*)GHist::get(base_id + i);
    h->GetXaxis()->SetRangeUser(-200,200);
    h->Draw();
  }

  c->Update();

  // Y position
  TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
  c->Clear();
  c->Divide(4,2);
  int base_id = HistMaker::getUniqueID(kMisc, 0, kHitPat, n_hist+1);
  for(int i = 0; i<n_hist; ++i){
    c->cd(i+1);
    TH1 *h = (TH1*)GHist::get(base_id + i);
    h->GetXaxis()->SetRangeUser(-100,100);
    h->Draw();
  }

  c->Update();

  c->cd(0);

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
