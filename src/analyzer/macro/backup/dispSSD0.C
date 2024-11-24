// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispSSD0()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const int n_layer = 2*2;
  const int n_hist  = 4;
  
  int ssd0_id[n_hist][n_layer];
  for(int l=0; l<n_layer; ++l){
    ssd0_id[0][l] = HistMaker::getUniqueID(kSSD0, 0, kADC2D,  l+1);
    ssd0_id[1][l] = HistMaker::getUniqueID(kSSD0, 0, kTDC2D,  l+1);
    ssd0_id[2][l] = HistMaker::getUniqueID(kSSD0, 0, kHitPat, l+1);
    ssd0_id[3][l] = HistMaker::getUniqueID(kSSD0, 0, kMulti,  l+1);
  }
  // draw SSD
  for(int i=0; i<n_hist; ++i){
    TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", i+1));
    if(!c) continue;
    c->Clear();
    c->Divide(2,2);
    for(int l=0; l<n_layer; ++l){
      c->cd(l+1)->SetGrid();
      TH1 *h = (TH1*)GHist::get(ssd0_id[i][l]);
      if(!h) continue;
      h->Draw("colz");
    }
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
