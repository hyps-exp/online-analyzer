// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispKIC()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const int n_seg = 4;

  int kic_id[n_seg*2] = {
    HistMaker::getUniqueID(kKIC, 0, kADC, 1),
    HistMaker::getUniqueID(kKIC, 0, kADC, 2),
    HistMaker::getUniqueID(kKIC, 0, kADC, 3),
    HistMaker::getUniqueID(kKIC, 0, kADC, 4),
    HistMaker::getUniqueID(kKIC, 0, kTDC, 1),
    HistMaker::getUniqueID(kKIC, 0, kTDC, 2),
    HistMaker::getUniqueID(kKIC, 0, kTDC, 3),
    HistMaker::getUniqueID(kKIC, 0, kTDC, 4)
  };

  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(4,2);
    for(int i = 0; i<n_seg*2; ++i){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get(kic_id[i]);
      h->GetXaxis()->SetRangeUser(0,2000);
      h->Draw();
    }
  }
  c->Update();

  c->cd(0);

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
