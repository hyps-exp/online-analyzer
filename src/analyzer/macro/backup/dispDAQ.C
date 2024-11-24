// -*- C++ -*-
#include "DetectorID.hh"

using hddaq::gui::Updater;

//_____________________________________________________________________________
void
dispDAQ( void )
{
  if( Updater::isUpdating() )
    return;
  Updater::setUpdating( true );

  //gStyle->SetOptStat(1111110);

  std::vector<Int_t> hist_id = {
    HistMaker::getUniqueID( kDAQ, kEB, kHitPat ),
    HistMaker::getUniqueID( kDAQ, kVME, kHitPat2D ),
    HistMaker::getUniqueID( kDAQ, kEASIROC, kHitPat2D ),
    HistMaker::getUniqueID( kDAQ, kHUL, kHitPat2D ),
  };

  {
    auto c = dynamic_cast<TCanvas*>( gROOT->FindObject("c1") );
    c->Clear();
    c->Divide( 1, 2 );
    c->cd(1)->Divide( 3, 1 );
    for( Int_t i=0, n=hist_id.size(); i<n; ++i ){
      if( i < n -1 )
        c->cd( 1 )->cd( i+1 );
      else
        c->cd( 2 );
      gPad->SetGrid();
      TH1 *h = GHist::get( hist_id[i] );
      if( h ) h->Draw("colz");
    }
    c->Update();
  }

  {
    auto c = dynamic_cast<TCanvas*>( gROOT->FindObject("c2") );
    c->Clear();
    c->Divide( 6, 4 );
    Int_t target_id = HistMaker::getUniqueID(kDAQ, 0, kMultiHitTdc);
    Int_t n = NumOfLayersBcOut+NumOfLayersSdcIn+1;
    for( Int_t i=0; i<n; i++ ){
      c->cd(i+1);
      gPad->SetGrid();
      c->GetPad(i+1)->SetLogz();
      TH1 *h = GHist::get( target_id++ );
      h->SetStats(0);
      if( h ) h->Draw("colz");
      TF1 *f = new TF1("f", "16", 0., 100.);
      f->SetLineColor(kBlack);
      f->Draw("same");
    }
    c->Update();
  }

  {
    auto c = dynamic_cast<TCanvas*>( gROOT->FindObject("c3") );
    c->Clear();
    c->Divide( 4, 2 );
    Int_t target_id = HistMaker::getUniqueID(kBH2, 0, kTDC, 20);
    for( Int_t i=0; i<NumOfSegBH2; i++ ){
      c->cd(i+1);
      c->GetPad(i+1)->SetLogy();
      TH1 *h = GHist::get( target_id+i );
      if( h ) h->Draw();
    }
    c->Update();
  }

  Updater::setUpdating( false );
}
