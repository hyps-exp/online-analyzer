#include "DetectorID.hh"

// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

#include "UserParamMan.hh"

void dispTOF()
{
  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // draw ADC U
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(4,4);
    int adc_id     = HistMaker::getUniqueID( kTOF, 0, kADC );
    int adcwtdc_id = HistMaker::getUniqueID( kTOF, 0, kADCwTDC );
    for( int i=0; i<NumOfSegTOF; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( adc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( 0, 4096 );
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
    c->Divide(4,4);
    int adc_id     = HistMaker::getUniqueID(kTOF, 0, kADC, 1+NumOfSegTOF);
    int adcwtdc_id = HistMaker::getUniqueID(kTOF, 0, kADCwTDC, 1+NumOfSegTOF);
    for( int i=0; i<NumOfSegTOF; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( adc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( 0, 4096 );
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
  static const unsigned int tdc_min = gUser.GetParameter("TdcTOF", 0);
  static const unsigned int tdc_max = gUser.GetParameter("TdcTOF", 1);

  // draw TDC U
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(4,4);
    int tdc_id = HistMaker::getUniqueID( kTOF, 0, kTDC );
    for( int i=0; i<NumOfSegTOF; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( tdc_id + i );
      h->GetXaxis()->SetRangeUser( tdc_min, tdc_max );
      if( h ) h->Draw();
    }
    c->Update();
  }

  // draw TDC D
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
    c->Clear();
    c->Divide(4,4);
    int tdc_id = HistMaker::getUniqueID( kTOF, 0, kTDC, 1+NumOfSegTOF );
    for( int i=0; i<NumOfSegTOF; ++i ){
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
}
