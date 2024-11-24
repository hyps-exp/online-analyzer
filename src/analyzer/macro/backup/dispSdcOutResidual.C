// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispSdcOutResidual( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  gStyle->SetOptStat(1111110);
  int n_layer = 12;

  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(4,3);
    int base_id = HistMaker::getUniqueID(kMisc, 0, kHitPat, 200);
    for( int i=0; i<n_layer; ++i ){
      c->cd(i+1);
      TH1 *h = GHist::get(base_id + i);
      if( h ) h->Draw();
      // TH1 *hh = (TH1*)GHist::get(base_id_ctot + i);
      // if( !hh ) continue;
      // hh->SetLineColor( kRed );
      // hh->Draw("same");
    }
    c->Update();
  }

  // // draw HitPat_SDC4
  // {
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
  //   c->Clear();
  //   c->Divide(2,2);
  //   int base_id = HistMaker::getUniqueID(kSDC4, 0, kHitPat);
  //   int base_id_ctot = HistMaker::getUniqueID(kSDC4, 0, kHitPat, 11);
  //   for( int i=0; i<n_layer; ++i ){
  //     c->cd(i+1);
  //     TH1 *h = GHist::get(base_id + i);
  //     if( h ) h->Draw();
  //     TH1 *hh = (TH1*)GHist::get(base_id_ctot + i);
  //     if( !hh ) continue;
  //     hh->SetLineColor( kRed );
  //     hh->Draw("same");
  //   }
  //   c->Update();
  // }

  // // draw HitPat_SDC5
  // {
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
  //   c->Clear();
  //   c->Divide(2,2);
  //   int base_id = HistMaker::getUniqueID(kSDC5, 0, kHitPat);
  //   int base_id_ctot = HistMaker::getUniqueID(kSDC5, 0, kHitPat, 11);
  //   for( int i=0; i<n_layer; ++i ){
  //     c->cd(i+1);
  //     TH1 *h = GHist::get(base_id + i);
  //     if( h ) h->Draw();
  //     TH1 *hh = (TH1*)GHist::get(base_id_ctot + i);
  //     if( !hh ) continue;
  //     hh->SetLineColor( kRed );
  //     hh->Draw("same");
  //   }
  //   c->Update();
  // }


  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
