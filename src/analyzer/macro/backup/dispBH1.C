// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

#include "UserParamMan.hh"

void dispBH1()
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
    c->Divide(4,3);
    Int_t adc_id     = HistMaker::getUniqueID(kBH1, 0, kADC);
    Int_t adcwtdc_id = HistMaker::getUniqueID(kBH1, 0, kADCwTDC);
    for( Int_t i=0; i<NumOfSegBH1; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( adc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser(0,2000);
      h->Draw();
      TH1 *hh = (TH1*)GHist::get( adcwtdc_id + i );
      if( !hh ) continue;
      hh->GetXaxis()->SetRangeUser(0,2000);
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
    Int_t adc_id     = HistMaker::getUniqueID(kBH1, 0, kADC, 1+NumOfSegBH1);
    Int_t adcwtdc_id = HistMaker::getUniqueID(kBH1, 0, kADCwTDC, 1+NumOfSegBH1);
    for( Int_t i=0; i<NumOfSegBH1; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = GHist::get( adc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser(0,2000);
      h->Draw();
      TH1 *hh = GHist::get( adcwtdc_id + i );
      if( !hh ) continue;
      hh->GetXaxis()->SetRangeUser(0,2000);
      hh->SetLineColor( kRed );
      hh->Draw("same");
    }
    c->Update();
  }

  // TDC gate range
  static const unsigned int tdc_min = gUser.GetParameter("TdcBH1", 0);
  static const unsigned int tdc_max = gUser.GetParameter("TdcBH1", 1);

  // draw TDC U
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(4,3);
    Int_t tdc_id = HistMaker::getUniqueID(kBH1, 0, kTDC);
    for( Int_t i=0; i<NumOfSegBH1; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( tdc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser(tdc_min,tdc_max);
      h->Draw();
    }
    c->Update();
  }

  // draw TDC D
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
    c->Clear();
    c->Divide(4,3);
    Int_t tdc_id = HistMaker::getUniqueID(kBH1, 0, kTDC, 1+NumOfSegBH1);
    for( Int_t i=0; i<NumOfSegBH1; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( tdc_id + i);
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser(tdc_min,tdc_max);
      h->Draw();
    }
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
