// -*- C++ -*-

// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

//______________________________________________________________________________
void
dispSSD2Time( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const int n_layer = 4;
  const int n_hist  = 2;

  int ssd2_id[n_hist][n_layer];
  for( int l=0; l<n_layer; ++l ){
    ssd2_id[0][l] = HistMaker::getUniqueID(kSSD2, 0, kCTime,  l+1);
    ssd2_id[1][l] = HistMaker::getUniqueID(kSSD2, 0, kCTime2D,  l+1);
  }
  // draw SSD
  for(int i=0; i<n_hist; ++i){
    TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", i+1));
    if(!c) continue;
    c->Clear();
    c->Divide(2,2);
    for(int l=0; l<n_layer; ++l){
      c->cd(l+1)->SetGrid();
      TH1 *h = (TH1*)GHist::get(ssd2_id[i][l]);
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
