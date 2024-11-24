#include "HistMaker.hh"

// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

//_____________________________________________________________________
void
dispCFTHitMulti( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  std::vector<Int_t> id = { kHitPat, kMulti };
  for( Int_t i=0, n=id.size(); i<n; ++i ){
    for( Int_t j=0; j<3; ++j ){
      TCanvas *c = (TCanvas*)gROOT->FindObject( Form("c%d", i*3+j+1) );
      c->Clear();
      c->Divide( 4, 2 );
      for( Int_t l=0; l<NumOfLayersCFT; ++l ){
	c->cd( l + 1 );
	TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, id[i], l+10*j+1));
	if( !h ) continue;
	h->Draw();
      }
      c->Update();
    }
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
