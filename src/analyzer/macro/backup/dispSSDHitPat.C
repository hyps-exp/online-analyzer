// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispSSDHitPat()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const int n_hist_1 = 8;
  const int n_hist_2 = 4;

  int hist_id_1[n_hist_1] = {
    HistMaker::getUniqueID(kSSD1, 0, kHitPat, 2),
    HistMaker::getUniqueID(kSSD1, 0, kHitPat, 4),
    HistMaker::getUniqueID(kSSD1, 0, kHitPat, 6),
    HistMaker::getUniqueID(kSSD1, 0, kHitPat, 8),
    HistMaker::getUniqueID(kSSD2, 0, kHitPat, 2),
    HistMaker::getUniqueID(kSSD2, 0, kHitPat, 4),
    HistMaker::getUniqueID(kSSD2, 0, kHitPat, 6),
    HistMaker::getUniqueID(kSSD2, 0, kHitPat, 8)
  };

  int hist_id_2[n_hist_2] = {
    HistMaker::getUniqueID(kSSD1, 0, kHitPat2D, 1),
    HistMaker::getUniqueID(kSSD1, 0, kHitPat2D, 2),
    HistMaker::getUniqueID(kSSD2, 0, kHitPat2D, 1),
    HistMaker::getUniqueID(kSSD2, 0, kHitPat2D, 2)
  };
  
  // draw SSD
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(4,2);
    for(int i=0; i<n_hist_1; ++i){
      c->cd(i+1)->SetGrid();
      TH1 *h = (TH1*)GHist::get( hist_id_1[i] );
      h->Draw("colz");
      c->Update();
    }
  }
  
  // draw SSD
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(2,2);
    for(int i=0; i<n_hist_2; ++i){
      c->cd(i+1)->SetGrid();
      TH1 *h = (TH1*)GHist::get( hist_id_2[i] );
      h->Draw("colz");
      c->Update();
    }
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
