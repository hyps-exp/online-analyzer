// -*- C++ -*-

using hddaq::gui::Updater;

//_____________________________________________________________________________
void
dispCorrelation( void )
{
  if( Updater::isUpdating() )
    return;
  Updater::setUpdating( true );

  {
    auto c = dynamic_cast<TCanvas*>( gROOT->FindObject("c1") );
    if( !c )
      return;
    c->Clear();
    c->Divide( 4, 2 );
    const auto base_id = HistMaker::getUniqueID( kCorrelation, 0, 0 );
    for( Int_t i=0; i<8; ++i ){
      c->cd( i+1 ); //->SetGrid();
      auto h = GHist::get( base_id+i );
      if( !h ) continue;
      h->Draw("colz");
    }
    c->Update();
  }

  // {
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
  //   c->Clear();
  //   c->Divide(2,2);
  //   for( Int_t i=0; i<4; ++i ){
  //     c->cd(i+1);//->SetGrid();
  //     TH1 *h = GHist::get( HistMaker::getUniqueID(kCorrelation, 0, 0, i+7) );
  //     if( !h ) continue;
  //     h->Draw("colz");
  //   }
  //   c->Update();
  // }

  Updater::setUpdating( false );
}
