// -*- C++ -*-

#include"DetectorID.hh"

using namespace hddaq::gui;

//_____________________________________________________________________________
void
dispPVAC( void )
{
  if( Updater::isUpdating() )
    return;
  Updater::setUpdating( true );

  auto c1 = dynamic_cast<TCanvas*>( gROOT->FindObject( "c1" ) );
  c1->Clear();
  c1->Divide( 2, 2 );
  Int_t id[NumOfSegPVAC*3] = {
    HistMaker::getUniqueID( kPVAC,  0, kADC,     1 ),
    HistMaker::getUniqueID( kPVAC,  0, kADCwTDC, 1 ),
    HistMaker::getUniqueID( kPVAC,  0, kTDC,     1 ),
  };
  TH1 *h = nullptr;

  // ADC
  c1->cd( 1 )->SetLogy();
  h = dynamic_cast<TH1*>( GHist::get( id[0] ) );
  h->GetXaxis()->SetRangeUser( 0, 0x1000 );
  h->Draw();
  // ADCwTDC
  h = dynamic_cast<TH1*>( GHist::get( id[1] ) );
  h->SetLineColor( kRed );
  h->Draw( "same" );
  // TDC
  c1->cd( 3 )->SetLogy();
  h = dynamic_cast<TH1*>( GHist::get( id[2] ) );
  h->GetXaxis()->SetRangeUser( 0, 2000000 );
  h->Draw();

  c1->Update();
  c1->cd(0);

  Updater::setUpdating( false );
}
