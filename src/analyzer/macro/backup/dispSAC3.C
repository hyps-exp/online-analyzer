// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

#include "UserParamMan.hh"

void dispSAC3()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  int n_seg = 2;



  // draw ADC
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(1,1);
    int base_id = HistMaker::getUniqueID(kSAC3, 0, kADC);
    for(int i = 0; i<1; ++i){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get(base_id + i);
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( 0 , 4096 );
      h->Draw();

      }
    c->Update();
  }

  // draw ADCwTDC
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(2,1);
    int base_id = HistMaker::getUniqueID(kSAC3, 0, kADC);
    int adcwtdc_id = HistMaker::getUniqueID(kSAC3, 0 , kADCwTDC);

      c->cd(0+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get(base_id + 0);
      h->GetXaxis()->SetRangeUser( 0 , 4096 );
      h->Draw();
      TH1 *hh = (TH1*)GHist::get(adcwtdc_id + 0);
      hh->GetXaxis()->SetRangeUser( 0 , 4096 );
      hh->SetLineColor( kRed );
      //hh->SetTitle("SAC3_ADCwTDC_1");
      hh->Draw("same");
      //efficiency
      Double_t NofTotal = h->GetEntries();
      Double_t NofYes   = hh->GetEntries();
      Double_t eff      = NofYes/NofTotal;
      Double_t xpos     = h->GetXaxis()->GetBinCenter( h->GetNbinsX() )*0.4;
      Double_t ypos     = h->GetMaximum()*0.7;
      auto text = new TLatex( xpos, ypos, Form( "eff. %.3f", eff ) );
      text->SetTextSize( 0.09 );
      text->Draw();
      //efficiency end


      c->cd(1+1);
      gPad->SetLogy();
      h = GHist::get(base_id + 0);
      h->GetXaxis()->SetRangeUser( 0 , 4096 );
      h->Draw();
      hh = GHist::get(adcwtdc_id + 1);
      hh->GetXaxis()->SetRangeUser( 0 , 4096 );
      hh->SetLineColor( kRed );
      //hh->SetTitle("SAC3_ADCwTDC_2");
      hh->Draw("same");
      //efficiency
      NofTotal = h->GetEntries();
      NofYes   = hh->GetEntries();
      eff      = NofYes/NofTotal;
      xpos     = h->GetXaxis()->GetBinCenter( h->GetNbinsX() )*0.4;
      ypos     = h->GetMaximum()*0.7;
      text = new TLatex( xpos, ypos, Form( "eff. %.3f", eff ) );
      text->SetTextSize( 0.09 );
      text->Draw();
      //efficiency end

    c->Update();
  }

  // draw TDC
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(2,1);
    int base_id = HistMaker::getUniqueID(kSAC3, 0, kTDC);
    for(int i = 0; i<n_seg; ++i){
      c->cd(i+1);
      GHist::get(base_id + i)->Draw();
    }

    c->Update();
  }
  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
