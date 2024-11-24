// -*- C++ -*-

using hddaq::gui::Updater;

//_____________________________________________________________________________
void
dispBFT( void )
{
  if( Updater::isUpdating() )
    return;
  Updater::setUpdating( true );

  const auto& gUser = UserParamMan::GetInstance();
  const Double_t TotRef = gUser.GetParameter( "TotRefBFT" );

  std::vector<Int_t> hid_c1 = {
    HistMaker::getUniqueID( kBFT, 0, kTDC, 1 ),
    HistMaker::getUniqueID( kBFT, 0, kTDC, 2 ),
    HistMaker::getUniqueID( kBFT, 0, kADC, 1 ),
    HistMaker::getUniqueID( kBFT, 0, kADC, 2 ),
    //HistMaker::getUniqueID( kBFT, 0, kTDC, 11 ),
    //HistMaker::getUniqueID( kBFT, 0, kTDC, 12 ),
    //HistMaker::getUniqueID( kBFT, 0, kADC, 11 ),
    //HistMaker::getUniqueID( kBFT, 0, kADC, 12 ),
  };
  std::vector<Int_t> hid_c2 = {
    HistMaker::getUniqueID( kBFT, 0, kHitPat, 1 ),
    HistMaker::getUniqueID( kBFT, 0, kHitPat, 2 ),
    HistMaker::getUniqueID( kBFT, 0, kMulti, 1 ),
    //HistMaker::getUniqueID( kBFT, 0, kHitPat, 11 ),
    //HistMaker::getUniqueID( kBFT, 0, kHitPat, 12 ),
    //HistMaker::getUniqueID( kBFT, 0, kMulti, 11 ),
  };

  std::vector<Int_t> hid_c3 = {
    HistMaker::getUniqueID( kBFT, 0, kTDC2D, 1 ),
    HistMaker::getUniqueID( kBFT, 0, kTDC2D, 2 ),
    HistMaker::getUniqueID( kBFT, 0, kADC2D, 1 ),
    HistMaker::getUniqueID( kBFT, 0, kADC2D, 2 ),
  };

  // draw TDC/TOT
  {
    auto c = dynamic_cast<TCanvas*>( gROOT->FindObject("c1") );
    c->Clear();
    c->Divide( 2, 2 );
    for( Int_t i=0, n=hid_c1.size(); i<n; ++i ){
      c->cd( i+1 ); //->SetGrid(2,2);
      auto h = GHist::get( hid_c1[i] );
      if( TString( h->GetTitle() ).Contains("TOT") ) {
	h->GetXaxis()->SetRangeUser( 0., 100. );
      }
      h->Draw();
      if( TString( h->GetTitle() ).Contains("TOT") ) {
	Double_t peak = h->GetMaximum();
	auto l = new TLine( TotRef, 0, TotRef, peak );
	l->SetLineColor( kRed );
	l->Draw( "same" );
      }
    }
    c->Update();
  }

  // draw HitPat/Multi
  {
    auto c = dynamic_cast<TCanvas*>( gROOT->FindObject("c2") );
    c->Clear();
    c->Divide( 2, 2 );
    for( Int_t i=0, n=hid_c2.size(); i<n; ++i ){
      c->cd( i+1 ); //->SetGrid();
      auto h = GHist::get( hid_c2[i] );
      h->Draw();
    }
    c->Update();
  }

  // draw TDC/TOT 2D
  {
    auto c = dynamic_cast<TCanvas*>( gROOT->FindObject("c3") );
    c->Clear();
    c->Divide( 2, 2 );
    for( Int_t i=0, n=hid_c3.size(); i<n; ++i ){
      c->cd( i+1 ); //->SetGrid();
      auto h = GHist::get( hid_c3[i] );
      h->Draw("colz");
    }
    c->Update();
  }

  Updater::setUpdating( false );
}
