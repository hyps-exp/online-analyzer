#include "DetectorID.hh"

// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

#include "UserParamMan.hh"

void dispTOF_TDC()
{
  const UserParamMan& gUser = UserParamMan::GetInstance();

  static const Int_t seg_per_canvas = 16;
  static const Int_t tdc_min = 0;
  static const Int_t tdc_max = 2048;

  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // draw TDC U0-15
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(4,4);
    int tdc_id     = HistMaker::getUniqueID( kTOF, 0, kTDC, 0);
    for( int i=0; i<seg_per_canvas; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( tdc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( tdc_min, tdc_max);
      h->Draw();
    }
    c->Update();
  }

  // draw TDC U16-31
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(4,4);
    int tdc_id     = HistMaker::getUniqueID( kTOF, 0, kTDC, 16);
    for( int i=0; i<seg_per_canvas; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( tdc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( tdc_min, tdc_max);
      h->Draw();
    }
    c->Update();
  }

  // draw TDC U32-47
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(4,4);
    int tdc_id     = HistMaker::getUniqueID( kTOF, 0, kTDC, 32);
    for( int i=0; i<seg_per_canvas; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( tdc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( tdc_min, tdc_max);
      h->Draw();
    }
    c->Update();
  }

  // draw TDC D0-15
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
    c->Clear();
    c->Divide(4,4);
    int tdc_id     = HistMaker::getUniqueID(kTOF, 0, kTDC, NumOfSegTOF);
    for( int i=0; i<seg_per_canvas; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( tdc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( tdc_min, tdc_max);
      h->Draw();
    }
    c->Update();
  }

  // draw TDC D16-31
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
    c->Clear();
    c->Divide(4,4);
    int tdc_id     = HistMaker::getUniqueID(kTOF, 0, kTDC, NumOfSegTOF + 16);
    for( int i=0; i<seg_per_canvas; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( tdc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( tdc_min, tdc_max);
      h->Draw();
    }
    c->Update();
  }

  // draw TDC D32-47
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c6");
    c->Clear();
    c->Divide(4,4);
    int tdc_id     = HistMaker::getUniqueID(kTOF, 0, kTDC, NumOfSegTOF + 32);
    for( int i=0; i<seg_per_canvas; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( tdc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( tdc_min, tdc_max);
      h->Draw();
    }
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
