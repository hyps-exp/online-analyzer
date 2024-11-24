// -*- C++ -*-

using hddaq::gui::Updater;

//_____________________________________________________________________________
void
effAC1WC( void )
{
  if( Updater::isUpdating() )
    return;
  Updater::setUpdating( true );

  auto c1 = dynamic_cast<TCanvas*>( gROOT->FindObject("c1") );
  if( !c1 ) return;
  c1->Clear();
  c1->Divide( 2, 2 );
  { //AC1 HitPat
    c1->cd( 1 );
    auto h1_id = HistMaker::getUniqueID( kAC1, 0, kHitPat );
    auto h1 = GHist::get( h1_id );
    if( h1 ) h1->Draw();
  }
  { //AC1 Multi
    c1->cd( 2 );
    auto h1_id = HistMaker::getUniqueID( kAC1, 0, kMulti );
    auto h1 = GHist::get( h1_id );
    if( h1 ) h1->Draw();
    auto h2_id = HistMaker::getUniqueID( kAC1, 0, kADCwTDC );
    auto h2 = GHist::get( h2_id + 3);
    Double_t Nof0     = h1->GetBinContent(1);
    Double_t Nofyes   = h2->GetEntries();
    Double_t NofTotal = h1->GetEntries();
    Double_t eff      = 1. - Nof0/NofTotal;
    Double_t effveto  = Nofyes/NofTotal;
    Double_t xpos     = h1->GetXaxis()->GetBinCenter( h1->GetNbinsX() )*0.4;
    Double_t ypos     = h1->GetMaximum()*0.7;
    Double_t ypos2    = h1->GetMaximum()*0.4;
    auto text = new TLatex( xpos, ypos, Form( "eff. %.3f", eff ) );
    text->SetTextSize( 0.09 );
    text->Draw();
    auto text2 = new TLatex( xpos, ypos2, Form( "veto. %.3f", effveto ) );
    text2->SetTextSize( 0.09 );
    text2->Draw();
  }
  { //WC HitPat
    c1->cd( 3 );
    auto h1_id = HistMaker::getUniqueID( kWC, 0, kHitPat );
    auto h1 = GHist::get( h1_id );
    if( h1 ) h1->Draw();
  }
  { //WC Multi
    c1->cd( 4 );
    auto h1_id = HistMaker::getUniqueID( kWC, 0, kMulti );
    auto h1 = GHist::get( h1_id );
    if( h1 ) h1->Draw();
    Double_t Nof0     = h1->GetBinContent(1);
    Double_t NofTotal = h1->GetEntries();
    Double_t eff      = 1. - Nof0/NofTotal;
    Double_t xpos     = h1->GetXaxis()->GetBinCenter( h1->GetNbinsX() )*0.4;
    Double_t ypos     = h1->GetMaximum()*0.7;
    auto text = new TLatex( xpos, ypos, Form( "eff. %.3f", eff ) );
    text->SetTextSize( 0.09 );
    text->Draw();
  }
  c1->Modified();
  c1->Update();
  Updater::setUpdating( false );
}
