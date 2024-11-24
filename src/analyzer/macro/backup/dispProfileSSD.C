// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispProfileSSD()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const int n_hist = 4;
  
  const int ssd_id[n_hist] = {
    HistMaker::getUniqueID(kSSD1, 0, kHitPat2D, 1),
    HistMaker::getUniqueID(kSSD1, 0, kHitPat2D, 2),
    HistMaker::getUniqueID(kSSD2, 0, kHitPat2D, 1),
    HistMaker::getUniqueID(kSSD2, 0, kHitPat2D, 2)
  }
  // draw SSD
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(3,2);
    for(int h=0; h<n_hist; ++h){
      if(h==1) continue;
      c->cd(h +1)->SetGrid();
      TH1 *h = (TH1*)GHist::get(ssd_id[h]);
      h->Draw("colz");
    }
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
