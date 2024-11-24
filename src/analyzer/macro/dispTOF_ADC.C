#include "DetectorID.hh"

// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

#include "UserParamMan.hh"

void dispTOF_ADC()
{
  const UserParamMan& gUser = UserParamMan::GetInstance();

  static const Int_t seg_per_canvas = 16;
  static const Int_t adc_min = 0;
  static const Int_t adc_max = 2048;

  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // draw ADC U0-15
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(4,4);
    int adc_id     = HistMaker::getUniqueID( kTOF, 0, kADC,     0);
    // int adcwtdc_id = HistMaker::getUniqueID( kTOF, 0, kADCwTDC, 0);
    for( int i=0; i<seg_per_canvas; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( adc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( adc_min, adc_max);
      h->Draw();

      // TH1 *hh = (TH1*)GHist::get( adcwtdc_id + i );
      // if( !hh ) continue;
      // hh->GetXaxis()->SetRangeUser( adc_min, adc_max);
      // hh->SetLineColor( kRed );
      // hh->Draw("same");
    }
    c->Update();
  }

  // draw ADC U16-31
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(4,4);
    int adc_id     = HistMaker::getUniqueID( kTOF, 0, kADC,     0 + 16);
    // int adcwtdc_id = HistMaker::getUniqueID( kTOF, 0, kADCwTDC, 0 + 16);
    for( int i=0; i<seg_per_canvas; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( adc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( adc_min, adc_max);
      h->Draw();

      // TH1 *hh = (TH1*)GHist::get( adcwtdc_id + i );
      // if( !hh ) continue;
      // hh->GetXaxis()->SetRangeUser( adc_min, adc_max);
      // hh->SetLineColor( kRed );
      // hh->Draw("same");
    }
    c->Update();
  }

  // draw ADC U32-47
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(4,4);
    int adc_id     = HistMaker::getUniqueID( kTOF, 0, kADC,     0 + 32);
    // int adcwtdc_id = HistMaker::getUniqueID( kTOF, 0, kADCwTDC, 0 + 32);
    for( int i=0; i<seg_per_canvas; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( adc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( adc_min, adc_max);
      h->Draw();

      // TH1 *hh = (TH1*)GHist::get( adcwtdc_id + i );
      // if( !hh ) continue;
      // hh->GetXaxis()->SetRangeUser( adc_min, adc_max);
      // hh->SetLineColor( kRed );
      // hh->Draw("same");
    }
    c->Update();
  }

  // draw ADC D0-15
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
    c->Clear();
    c->Divide(4,4);
    int adc_id     = HistMaker::getUniqueID(kTOF, 0, kADC,     NumOfSegTOF);
    // int adcwtdc_id = HistMaker::getUniqueID(kTOF, 0, kADCwTDC, NumOfSegTOF);
    for( int i=0; i<seg_per_canvas; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( adc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( adc_min, adc_max);
      h->Draw();

      // TH1 *hh = (TH1*)GHist::get( adcwtdc_id + i );
      // if( !hh ) continue;
      // hh->GetXaxis()->SetRangeUser( adc_min, adc_max);
      // hh->SetLineColor( kRed );
      // hh->Draw("same");
    }
    c->Update();
  }

  // draw ADC D16-31
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
    c->Clear();
    c->Divide(4,4);
    int adc_id     = HistMaker::getUniqueID(kTOF, 0, kADC,     NumOfSegTOF + 16);
    // int adcwtdc_id = HistMaker::getUniqueID(kTOF, 0, kADCwTDC, NumOfSegTOF + 16);
    for( int i=0; i<seg_per_canvas; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( adc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( adc_min, adc_max);
      h->Draw();

      // TH1 *hh = (TH1*)GHist::get( adcwtdc_id + i );
      // if( !hh ) continue;
      // hh->GetXaxis()->SetRangeUser( adc_min, adc_max);
      // hh->SetLineColor( kRed );
      // hh->Draw("same");
    }
    c->Update();
  }

  // draw ADC D32-47
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c6");
    c->Clear();
    c->Divide(4,4);
    int adc_id     = HistMaker::getUniqueID(kTOF, 0, kADC,     NumOfSegTOF + 32);
    // int adcwtdc_id = HistMaker::getUniqueID(kTOF, 0, kADCwTDC, NumOfSegTOF + 32);
    for( int i=0; i<seg_per_canvas; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( adc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( adc_min, adc_max);
      h->Draw();

      // TH1 *hh = (TH1*)GHist::get( adcwtdc_id + i );
      // if( !hh ) continue;
      // hh->GetXaxis()->SetRangeUser( adc_min, adc_max);
      // hh->SetLineColor( kRed );
      // hh->Draw("same");
    }
    c->Update();
  }

  // //TOF TDC gate range
  // static const unsigned int tdc_min = gUser.GetParameter("TdcTOF", 0);
  // static const unsigned int tdc_max = gUser.GetParameter("TdcTOF", 1);

  // // draw TDC U0-15
  // {
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c7");
  //   c->Clear();
  //   c->Divide(4,4);
  //   int tdc_id = HistMaker::getUniqueID( kTOF, 0, kTDC );
  //   for( int i=0; i<15; ++i ){
  //     c->cd(i+1);
  //     TH1 *h = (TH1*)GHist::get( tdc_id + i );
  //     h->GetXaxis()->SetRangeUser( tdc_min, tdc_max );
  //     if( h ) h->Draw();
  //   }
  //   c->Update();
  // }

  // // draw TDC U16-31
  // {
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c8");
  //   c->Clear();
  //   c->Divide(4,4);
  //   int tdc_id = HistMaker::getUniqueID( kTOF, 0, kTDC );
  //   for( int i=16; i<32; ++i ){
  //     c->cd(i+1);
  //     TH1 *h = (TH1*)GHist::get( tdc_id + i );
  //     h->GetXaxis()->SetRangeUser( tdc_min, tdc_max );
  //     if( h ) h->Draw();
  //   }
  //   c->Update();
  // }

  // // draw TDC U32-47
  // {
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
  //   c->Clear();
  //   c->Divide(4,4);
  //   int tdc_id = HistMaker::getUniqueID( kTOF, 0, kTDC );
  //   for( int i=32; i<48; ++i ){
  //     c->cd(i+1);
  //     TH1 *h = (TH1*)GHist::get( tdc_id + i );
  //     h->GetXaxis()->SetRangeUser( tdc_min, tdc_max );
  //     if( h ) h->Draw();
  //   }
  //   c->Update();
  // }

  // // draw TDC D0-15
  // {
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
  //   c->Clear();
  //   c->Divide(4,4);
  //   int tdc_id = HistMaker::getUniqueID( kTOF, 0, kTDC, 1+NumOfSegTOF );
  //   for( int i=0; i<16; ++i ){
  //     c->cd(i+1);
  //     TH1 *h = (TH1*)GHist::get( tdc_id + i );
  //     h->GetXaxis()->SetRangeUser( tdc_min, tdc_max );
  //     if( h ) h->Draw();
  //   }
  //   c->Update();
  // }

  // // draw TDC D16-31
  // {
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
  //   c->Clear();
  //   c->Divide(4,4);
  //   int tdc_id = HistMaker::getUniqueID( kTOF, 0, kTDC, 1+NumOfSegTOF );
  //   for( int i=16; i<32; ++i ){
  //     c->cd(i+1);
  //     TH1 *h = (TH1*)GHist::get( tdc_id + i );
  //     h->GetXaxis()->SetRangeUser( tdc_min, tdc_max );
  //     if( h ) h->Draw();
  //   }
  //   c->Update();
  // }

  // // draw TDC D32-47
  // {
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
  //   c->Clear();
  //   c->Divide(4,4);
  //   int tdc_id = HistMaker::getUniqueID( kTOF, 0, kTDC, 1+NumOfSegTOF );
  //   for( int i=32; i<48; ++i ){
  //     c->cd(i+1);
  //     TH1 *h = (TH1*)GHist::get( tdc_id + i );
  //     h->GetXaxis()->SetRangeUser( tdc_min, tdc_max );
  //     if( h ) h->Draw();
  //   }
  //   c->Update();
  // }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
