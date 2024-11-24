// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispBcOutFF( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const int n_hist = 5;
  const Int_t NumHist=(3*n_hist+1);//X, Y, XY, U
  const Int_t trig =1; //0: unbias, 1: K-beam, 2: pi-beam

  // XY position
  for( int i=0; i<n_hist; ++i ){
    TCanvas *c = (TCanvas*)gROOT->FindObject( Form("c%d", i+1 ) );
    c->Clear();
    c->Divide(2,2);

    int hx_id = HistMaker::getUniqueID(kMisc, 0, kHitPat, i+1+(NumHist*trig));
    c->cd(1);
    TH1 *h_x = (TH1*)GHist::get( hx_id );
    if( h_x ){
      // h_x->GetXaxis()->SetRangeUser(-200,200);
      h_x->Draw();
    }

    int hy_id = HistMaker::getUniqueID(kMisc, 0, kHitPat, i+6+(NumHist*trig));
    c->cd(2);
    TH1 *h_y = (TH1*)GHist::get( hy_id );
    if( h_y ){
      // h_y->GetXaxis()->SetRangeUser(-200,200);
      h_y->Draw();
    }

    int hxy_id = HistMaker::getUniqueID(kMisc, 0, kHitPat, i+11+(NumHist*trig));
    c->cd(3);
    TH1 *h_xy = (TH1*)GHist::get( hxy_id );
    if( h_xy ){
      // h_y->GetXaxis()->SetRangeUser(-200,200);
      h_xy->Draw("colz");
    }
    c->Update();
  }
  TCanvas *c = (TCanvas*)gROOT->FindObject("c6");
  c->Clear();
  c->Divide(2,2);
  for( int i=0; i<3; ++i ){
    c->cd(i+1);
    int hxu_id = HistMaker::getUniqueID(kMisc, 0, kHitPat, 16+(NumHist*i));
    TH1 *h_xu = (TH1*)GHist::get( hxu_id );
    if( h_xu ){
      h_xu->GetXaxis()->SetRangeUser(-150,150);
      h_xu->GetYaxis()->SetRangeUser(-0.4,0.4);
      h_xu->GetYaxis()->CenterTitle();
      h_xu->Draw("colz");
    }
  }
  c->Update();


  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
