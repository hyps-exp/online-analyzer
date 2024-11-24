// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;
#include<algorithm>

#include "DetectorID.hh"

void dispBH2Filter( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  {
    TCanvas *c = dynamic_cast<TCanvas*>(gROOT->FindObject("c1"));
    c->Clear();
    c->Divide(3,2);
    const Int_t raw_id = HistMaker::getUniqueID(kMisc, 0, kHitPat2D);
    const Int_t cut_id = HistMaker::getUniqueID(kMisc, 1, kHitPat2D);
    for( Int_t i=0; i<NumOfLayersBcOut/2; ++i ){
      c->cd(i+1);
      TH1 *h;
      h = dynamic_cast<TH1*>(GHist::get(raw_id+i));
      if( !h ) continue;
      h->SetFillColor(kBlack);
      h->SetLineColor(kBlack);
      h->SetMarkerColor(kBlack);
      h->Draw("box");
      h = dynamic_cast<TH1*>(GHist::get(cut_id+i));
      if( !h ) continue;
      h->SetFillColor(kRed);
      h->SetLineColor(kRed);
      h->SetMarkerColor(kRed);
      h->Draw("box same");
    }// for(i)
    c->Update();
  }

  {
    TCanvas *c = dynamic_cast<TCanvas*>(gROOT->FindObject("c2"));
    c->Clear();
    c->Divide(3,2);
    const Int_t raw_id = HistMaker::getUniqueID(kMisc, 0, kHitPat2D);
    const Int_t cut_id = HistMaker::getUniqueID(kMisc, 1, kHitPat2D);
    for( Int_t i=0; i<NumOfLayersBcOut/2; ++i ){
      c->cd(i+1);
      TH1 *h;
      h = dynamic_cast<TH1*>(GHist::get(raw_id+i+NumOfLayersBcOut/2));
      if( !h ) continue;
      h->SetFillColor(kBlack);
      h->SetLineColor(kBlack);
      h->SetMarkerColor(kBlack);
      h->Draw("box");
      h = dynamic_cast<TH1*>(GHist::get(cut_id+i+NumOfLayersBcOut/2));
      if( !h ) continue;
      h->SetFillColor(kRed);
      h->SetLineColor(kRed);
      h->SetMarkerColor(kRed);
      h->Draw("box same");
    }// for(i)
    c->Update();
  }
  {
    TCanvas *c = dynamic_cast<TCanvas*>(gROOT->FindObject("c3"));
    c->Clear();
    c->Divide(2,2);
    TH1 *h;
    h = dynamic_cast<TH1*>( GHist::get( HistMaker::getUniqueID(kMisc, 0, kMulti, 1) ) );
    Bool_t bh2filter = ( h == 0 );
    if( bh2filter ){
      c->cd(1);
      h = dynamic_cast<TH1*>( GHist::get( HistMaker::getUniqueID(kMisc, 0, kMulti, 2) ) );
      if( h ) h->Draw();
      double Nof0     = h->GetBinContent(1);
      double NofTotal = h->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;
      static TLatex *text = new TLatex;
      text->SetNDC();
      text->SetTextSize(0.08);
      text->DrawLatex(0.42, 0.35, Form("plane eff. %.3f", eff));
      c->cd(2);
      h = dynamic_cast<TH1*>( GHist::get( HistMaker::getUniqueID(kMisc, 2, kHitPat2D, 2) ) );
      if( h ) h->Draw("colz");
      c->cd(3);
      h = dynamic_cast<TH1*>( GHist::get( HistMaker::getUniqueID(kMisc, 0, kChisqr, 2) ) );
      if( h ) h->Draw("colz");
    } else {
      c->cd(1);
      h = dynamic_cast<TH1*>( GHist::get( HistMaker::getUniqueID(kMisc, 0, kMulti, 1) ) );
      if( h ) h->Draw();
      double Nof0     = h->GetBinContent(1);
      double NofTotal = h->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;
      static TLatex *text = new TLatex;
      text->SetNDC();
      text->SetTextSize(0.08);
      text->DrawLatex(0.42, 0.35, Form("plane eff. %.2f", eff));
      c->cd(2);
      h = dynamic_cast<TH1*>( GHist::get( HistMaker::getUniqueID(kMisc, 2, kHitPat2D, 1) ) );
      if( h ) h->Draw("colz");
      c->cd(3);
      h = dynamic_cast<TH1*>( GHist::get( HistMaker::getUniqueID(kMisc, 0, kMulti, 2) ) );
      if( h ) h->Draw();
      c->cd(4);
      h = dynamic_cast<TH1*>( GHist::get( HistMaker::getUniqueID(kMisc, 2, kHitPat2D, 2) ) );
      if( h ) h->Draw("colz");
    }
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
