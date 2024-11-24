// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispLAC()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  int n_seg = 30;
#if 0
  // draw ADC U
  TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  c->Clear();
  c->Divide(5,6);
  int base_id = HistMaker::getUniqueID(kLAC, 0, kADC, 1);
  for(int i = 0; i<n_seg; ++i){
    c->cd(i+1);
    gPad->SetLogy();
    TH1 *h = (TH1*)GHist::get(base_id + i);
    h->GetXaxis()->SetRangeUser(0,3000);
    h->Draw();
  }

  c->Update();
#endif

  // draw TDC U
  //  c = (TCanvas*)gROOT->FindObject("c2");
  TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  c->Clear();
  c->Divide(5,6);
  int base_id = HistMaker::getUniqueID(kLAC, 0, kTDC, 1);
  for(int i = 0; i<n_seg; ++i){
    c->cd(i+1);
    GHist::get(base_id + i)->Draw();
  }

  c->Update();

  c->cd(0);

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
