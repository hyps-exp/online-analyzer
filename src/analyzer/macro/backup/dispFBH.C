// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispFBH( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const int n_seg     = 16;
    
  int fbh_id_c1[] = {
    HistMaker::getUniqueID(kFBH, 0, kTDC,    1),
    HistMaker::getUniqueID(kFBH, 0, kADC,    1),
    HistMaker::getUniqueID(kFBH, 0, kTDC,    n_seg*2+1),
    HistMaker::getUniqueID(kFBH, 0, kADC,    n_seg*2+1),
    HistMaker::getUniqueID(kFBH, 0, kHitPat, 1),
    HistMaker::getUniqueID(kFBH, 0, kTDC2D,  1),
    HistMaker::getUniqueID(kFBH, 0, kADC2D,  1),
    HistMaker::getUniqueID(kFBH, 0, kMulti,  1)
  };

  // // Draw TDC UP
  // {
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  //   c->Clear();
  //   c->Divide(4,4);
  //   for( int i=0; i<n_seg; ++i ){
  //     c->cd(i+1)->SetGrid();
  //     TH1 *h = (TH1*)GHist::get( fbh_id_c1[0]+i );
  //     h->Draw();
  //   }
  //   c->Update();
  // }

  // // Draw TDC DOWN
  // {
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
  //   c->Clear();
  //   c->Divide(4,4);
  //   for( int i=0; i<n_seg; ++i ){
  //     c->cd(i+1)->SetGrid();
  //     TH1 *h = (TH1*)GHist::get( fbh_id_c1[0]+n_seg+i );
  //     h->Draw();
  //   }
  //   c->Update();
  // }

  // // Draw TOT UP
  // {
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
  //   c->Clear();
  //   c->Divide(4,4);
  //   for( int i=0; i<n_seg; ++i ){
  //     c->cd(i+1)->SetGrid();
  //     TH1 *h = (TH1*)GHist::get( fbh_id_c1[1]+i );
  //     h->Draw();
  //   }
  //   c->Update();
  // }

  // // Draw TOT DOWN
  // {
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
  //   c->Clear();
  //   c->Divide(4,4);
  //   for( int i=0; i<n_seg; ++i ){
  //     c->cd(i+1)->SetGrid();
  //     TH1 *h = (TH1*)GHist::get( fbh_id_c1[1]+n_seg+i );
  //     h->Draw();
  //   }
  //   c->Update();
  // }

  //
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
    c->Clear();
    c->Divide(3,2);
    for( int i=0; i<6; ++i ){
      c->cd(i+1)->SetGrid();
      TH1 *h = (TH1*)GHist::get( fbh_id_c1[2+i] );
      h->Draw("colz");
    }
    c->Update();
  }
    
  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
