// -*- C++ -*-

using hddaq::gui::Updater;

#include "UserParamMan.hh"

//_____________________________________________________________________________
void
dispHTOF()
{
  if(Updater::isUpdating())
    return;
  Updater::setUpdating(true);

  const auto& gUser = UserParamMan::GetInstance();
  const auto tdc_min = gUser.GetParameter("TdcHTOF", 0);
  const auto tdc_max = gUser.GetParameter("TdcHTOF", 1);

  { ///// ADC U
    auto c1 = dynamic_cast<TCanvas*>(gROOT->FindObject("c1"));
    c1->Clear();
    c1->Divide(6, 6);
    auto adc_hid = HistMaker::getUniqueID(kHTOF, 0, kADC);
    auto awt_hid = HistMaker::getUniqueID(kHTOF, 0, kADCwTDC);
    for(Int_t i=0; i<NumOfSegHTOF; ++i){
      c1->cd(i+1)->SetLogy();
      auto h1 = dynamic_cast<TH1*>(GHist::get(adc_hid+i));
      if(!h1) continue;
      // h1->GetXaxis()->SetRangeUser(0, 2000);
      h1->Draw();
      auto h2 = dynamic_cast<TH1*>(GHist::get(awt_hid+i));
      if(!h2) continue;
      // h2->GetXaxis()->SetRangeUser(0, 2000);
      h2->SetLineColor(kRed+1);
      h2->Draw("same");
    }
    c1->Update();
  }
  { ///// ADC D
    auto c1 = dynamic_cast<TCanvas*>(gROOT->FindObject("c2"));
    c1->Clear();
    c1->Divide(6, 6);
    auto adc_hid = HistMaker::getUniqueID(kHTOF, 0, kADC, NumOfSegHTOF+1);
    auto awt_hid = HistMaker::getUniqueID(kHTOF, 0, kADCwTDC, NumOfSegHTOF+1);
    for(Int_t i=0; i<NumOfSegHTOF; ++i){
      c1->cd(i+1);
      gPad->SetLogy();
      auto h1 = GHist::get(adc_hid+i);
      if(!h1) continue;
      // h1->GetXaxis()->SetRangeUser(0, 2000);
      h1->Draw();
      auto h2 = GHist::get(awt_hid+i);
      if(!h2) continue;
      // h2->GetXaxis()->SetRangeUser(0, 2000);
      h2->SetLineColor(kRed+1);
      h2->Draw("same");
    }
    c1->Update();
  }
  { ///// TDC U
    auto c1 = dynamic_cast<TCanvas*>(gROOT->FindObject("c3"));
    c1->Clear();
    c1->Divide(6, 6);
    auto tdc_hid = HistMaker::getUniqueID(kHTOF, 0, kTDC);
    for(Int_t i=0; i<NumOfSegHTOF; ++i){
      c1->cd(i+1);
      auto h1 = dynamic_cast<TH1*>(GHist::get(tdc_hid+i));
      if(!h1) continue;
      h1->GetXaxis()->SetRangeUser(tdc_min, tdc_max);
      h1->Draw();
    }
    c1->Update();
  }
  { ///// TDC D
    auto c1 = dynamic_cast<TCanvas*>(gROOT->FindObject("c4"));
    c1->Clear();
    c1->Divide(6, 6);
    auto tdc_hid = HistMaker::getUniqueID(kHTOF, 0, kTDC, NumOfSegHTOF+1);
    for(Int_t i=0; i<NumOfSegHTOF; ++i){
      c1->cd(i+1);
      auto h1 = dynamic_cast<TH1*>(GHist::get(tdc_hid+i));
      if(!h1) continue;
      h1->GetXaxis()->SetRangeUser(tdc_min, tdc_max);
      h1->Draw();
    }
    c1->Update();
  }

  Updater::setUpdating(false);
}
