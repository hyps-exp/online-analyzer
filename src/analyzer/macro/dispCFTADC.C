// -*- C++ -*-

#include "DetectorID.hh"

#include "UserParamMan.hh"

using namespace hddaq::gui; // for Updater

//_____________________________________________________________________________
void
dispCFTADC( void )
{

  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  {
    std::vector<Int_t> id = { kHighGain, kLowGain, kPede };
    for( Int_t i=0, n=id.size(); i<n; ++i ){
      TCanvas *c = (TCanvas*)gROOT->FindObject( Form("c%d", i+1) );
      c->Clear();
      c->Divide( 4, 2 );
      for( Int_t l=0; l<NumOfLayersCFT; ++l ){
	c->cd(l+1);
	TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, id[i]==kPede? 0:kCluster, id[i], l+1));
	if(!h) continue;

	if(id[i] == kLowGain) h->GetXaxis()->SetRangeUser(0, 1000);
	h->Draw("colz");
      }
      c->Update();
    }
  }

  { // 2D
    std::vector<Int_t> id = { kHighGain, kLowGain, kPede };
    for( Int_t i=0, n=id.size(); i<n; ++i ){
      TCanvas *c = (TCanvas*)gROOT->FindObject( Form("c%d", i+4) );
      c->Clear();
      c->Divide( 4, 2 );
      for( Int_t l=0; l<NumOfLayersCFT; ++l ){
	c->cd(l+1);
	TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, id[i], l+11));
	if(!h) continue;
	h->Draw("colz");
      }
      c->Update();
    }
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------

}
