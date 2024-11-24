// -*- C++ -*-

#include "DetectorID.hh"
#include "UserParamMan.hh"

using namespace hddaq::gui; // for Updater

//_____________________________________________________________________________
void
dispWC( void )
{
  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const Double_t xmin =    0.;
  const Double_t xmax = 4096.;

  // draw ADC U
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(4,3);
    int adc_id     = HistMaker::getUniqueID( kWC, 0, kADC );
    int adcwtdc_id = HistMaker::getUniqueID( kWC, 0, kADCwTDC );
    for( int i=0; i<NumOfSegWC; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( adc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( xmin, xmax );
      h->Draw();
      TH1 *hh = (TH1*)GHist::get( adcwtdc_id + i );
      if( !hh ) continue;
      hh->GetXaxis()->SetRangeUser( 0, 4096 );
      hh->SetLineColor( kRed );
      hh->Draw("same");
    }
    c->Update();
  }

  // draw ADC D
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(4,3);
    int adc_id     = HistMaker::getUniqueID(kWC, 0, kADC, 1+NumOfSegWC);
    int adcwtdc_id = HistMaker::getUniqueID(kWC, 0, kADCwTDC, 1+NumOfSegWC);
    for( int i=0; i<NumOfSegWC; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( adc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( xmin, xmax );
      h->Draw();
      TH1 *hh = (TH1*)GHist::get( adcwtdc_id + i );
      if( !hh ) continue;
      hh->GetXaxis()->SetRangeUser( 0, 4096 );
      hh->SetLineColor( kRed );
      hh->Draw("same");
    }
    c->Update();
  }

  // draw ADC Sum
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(4,3);
    int adc_id     = HistMaker::getUniqueID(kWC, 0, kADC, 1+NumOfSegWC*2);
    int adcwtdc_id = HistMaker::getUniqueID(kWC, 0, kADCwTDC, 1+NumOfSegWC*2);
    for( int i=0; i<NumOfSegWC; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( adc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( xmin, xmax );
      h->Draw();
      TH1 *hh = (TH1*)GHist::get( adcwtdc_id + i );
      if( !hh ) continue;
      hh->GetXaxis()->SetRangeUser( 0, 4096 );
      hh->SetLineColor( kRed );
      hh->Draw("same");
    }
    c->Update();
  }

  //TOF TDC gate range
  static const unsigned int tdc_min = gUser.GetParameter("TdcWC", 0);
  static const unsigned int tdc_max = gUser.GetParameter("TdcWC", 1);

  // draw TDC U
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
    c->Clear();
    c->Divide(4,3);
    int tdc_id = HistMaker::getUniqueID( kWC, 0, kTDC );
    for( int i=0; i<NumOfSegWC; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( tdc_id + i );
      h->GetXaxis()->SetRangeUser( tdc_min, tdc_max );
      if( h ) h->Draw();
    }
    c->Update();
  }

  // draw TDC D
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
    c->Clear();
    c->Divide(4,3);
    int tdc_id = HistMaker::getUniqueID( kWC, 0, kTDC, 1+NumOfSegWC );
    for( int i=0; i<NumOfSegWC; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( tdc_id + i );
      h->GetXaxis()->SetRangeUser( tdc_min, tdc_max );
      if( h ) h->Draw();
    }
    c->Update();
  }

  // draw TDC Sum
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c6");
    c->Clear();
    c->Divide(4,3);
    int tdc_id = HistMaker::getUniqueID( kWC, 0, kTDC, 1+NumOfSegWC*2 );
    for( int i=0; i<NumOfSegWC; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( tdc_id + i );
      h->GetXaxis()->SetRangeUser( tdc_min, tdc_max );
      if( h ) h->Draw();
    }
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------





  // if( Updater::isUpdating() )
  //   return;
  // Updater::setUpdating( true );

  // const auto& gUser = UserParamMan::GetInstance();
  // const UInt_t tdc_min = gUser.GetParameter( "TdcWC", 0 );
  // const UInt_t tdc_max = gUser.GetParameter( "TdcWC", 1 );

  // auto c1 = dynamic_cast<TCanvas*>( gROOT->FindObject("c1") );
  // c1->Clear();
  // c1->Divide( 4, 5 );
  // Int_t icanvas = 0;
  // Int_t id = 0;
  // TH1* h1 = nullptr;
  // for( Int_t i=0; i<NumOfSegWC; ++i ){
  //   for( Int_t ud=0; ud<2; ++ud ){
  //     if( i == NumOfSegWC-1 && ud == 1 )
  // 	continue;
  //     // ADC
  //     c1->cd( ++icanvas )->SetLogy();
  //     id = HistMaker::getUniqueID( kWC, 0, kADC, i+ud*NumOfSegWC+1 );
  //     h1 = dynamic_cast<TH1*>( GHist::get( id ) );
  //     if( h1 ){
  // 	h1->GetXaxis()->SetRangeUser( 0, 0x1000 );
  // 	h1->Draw();
  //     }
  //     // ADCw/TDC
  //     id = HistMaker::getUniqueID( kWC, 0, kADCwTDC, i+ud*NumOfSegWC+1 );
  //     h1 = dynamic_cast<TH1*>( GHist::get( id ) );
  //     if( h1 ){
  // 	h1->GetXaxis()->SetRangeUser( 0, 0x1000 );
  // 	h1->SetLineColor( kRed );
  // 	h1->Draw( "same" );
  //     }
  //     // TDC
  //     c1->cd( icanvas + NumOfSegWC*2 - 1 )->SetLogy();
  //     id = HistMaker::getUniqueID( kWC, 0, kTDC, i+ud*NumOfSegWC+1 );
  //     h1 = dynamic_cast<TH1*>( GHist::get( id ) );
  //     if( h1 ){
  // 	h1->GetXaxis()->SetRangeUser( tdc_min, tdc_max );
  // 	h1->Draw();
  //     }
  //   }
  // }

  // c1->Update();
  // c1->cd(0);

  // Updater::setUpdating( false );
}
