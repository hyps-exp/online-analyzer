// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispSSD1Profile( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const Int_t n_hist = 4;
    
  {
    TCanvas *c = dynamic_cast<TCanvas*>(gROOT->FindObject("c1"));
    c->Clear(); c->Divide(2,2);
    for( Int_t i=0; i<n_hist; ++i ){
      c->cd(i+1);
      Int_t hid = HistMaker::getUniqueID(kMisc, 0, kHitPat2D, i+1);
      TH1 *h = GHist::get( hid );
      if( !h ) continue;
      h->Draw();
      c->Modified();
      c->Update();
    }
  }

  TLatex tex;
  tex.SetNDC();
  tex.SetTextSize(0.14);
  const Double_t xpos = 0.15;
  const Double_t ypos = 0.75;

  {
    TCanvas *c = dynamic_cast<TCanvas*>(gROOT->FindObject("c2"));
    c->Clear(); c->Divide(2,2);
    for( Int_t i=0; i<n_hist; ++i ){
      c->cd(i+1);
      Int_t hid = HistMaker::getUniqueID(kMisc, 0, kHitPat2D, i+1);
      if( !GHist::get( hid ) ) continue;
      TH1 *h = dynamic_cast<TH1*>(GHist::get( hid )->Clone());
      TF1 f("f","gaus");
      Double_t mean  =  0.;
      Double_t sigma = 50.;
      for( Int_t j=0; j<5; ++j ){
	h->Fit("f", "Q", "", mean-1.5*sigma, mean+1.5*sigma );
	mean  = f.GetParameter("Mean");
	sigma = f.GetParameter("Sigma");
      }
      tex.DrawLatex(xpos, ypos, Form("%.2lf", sigma));
      c->Modified();
      c->Update();
    }
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
