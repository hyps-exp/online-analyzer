// -*- C++ -*-

#include "MacroBuilder.hh"

#include <iostream>
#include <string>

#include <TCanvas.h>
#include <TF1.h>
#include <TGraph.h>
#include <TH1.h>
#include <TH2.h>
#include <TLatex.h>
#include <TLine.h>
#include <TMacro.h>
#include <TPolyLine.h>
#include <TString.h>
#include <TText.h>

#include "DetectorID.hh"
#include "Main.hh"
#include "HistHelper.hh"
#include "HistMaker.hh"
#include "UserParamMan.hh"

namespace
{
const auto& gUser = UserParamMan::GetInstance();
std::vector<Double_t> envelope_x_mean(NProfile);
std::vector<Double_t> envelope_xfit_mean(NProfile);
std::vector<Double_t> envelope_x_rms(NProfile);
std::vector<Double_t> envelope_xfit_sigma(NProfile);
std::vector<Double_t> envelope_y_mean(NProfile);
std::vector<Double_t> envelope_yfit_mean(NProfile);
std::vector<Double_t> envelope_y_rms(NProfile);
std::vector<Double_t> envelope_yfit_sigma(NProfile);

void
SetText(TLatex* text, Int_t align, Double_t size, Int_t ndc=1)
{
  text->SetNDC(ndc);
  text->SetTextAlign(align);
  text->SetTextSize(size);
}
}

namespace analyzer
{

namespace macro
{

//_____________________________________________________________________________
TObject*
Get(TString name)
{
  std::string process = Main::getInstance().getArgv().at(0);
  Int_t n = process.find("bin");
  TString path = process.substr(0, n)+"src/analyzer/macro/";
  if(name.Contains(".C"))
    name.ReplaceAll(".C","");
  return new TMacro(path+name+".C");
}

} // namespace macro

//_____________________________________________________________________________
// For HttpServer
namespace http
{

const std::vector<Double_t>&
GetEnvelopeXMean() { return envelope_x_mean; }
const std::vector<Double_t>&
GetEnvelopeXfitMean() { return envelope_xfit_mean; }
const std::vector<Double_t>&
GetEnvelopeXRMS() { return envelope_x_rms; }
const std::vector<Double_t>&
GetEnvelopeXfitSigma() { return envelope_xfit_sigma; }
const std::vector<Double_t>&
GetEnvelopeYMean() { return envelope_y_mean; }
const std::vector<Double_t>&
GetEnvelopeYfitMean() { return envelope_yfit_mean; }
const std::vector<Double_t>&
GetEnvelopeYRMS() { return envelope_y_rms; }
const std::vector<Double_t>&
GetEnvelopeYfitSigma() { return envelope_yfit_sigma; }

//_____________________________________________________________________________



//_____________________________________________________________________________
TCanvas*
Counter_TDC()
{
  auto c1 = new TCanvas(__func__, __func__);
  c1->Divide(3, 4);

  int RF_id = HistMaker::getUniqueID(kRF, 0, kTDC, 0);
  int TAG_SFF_id = HistMaker::getUniqueID(kTAG_SF, 0, kHitPat, 0);
  int TAG_SFB_id = HistMaker::getUniqueID(kTAG_SF, 0, kHitPat, 1);
  int TAG_PL_id = HistMaker::getUniqueID(kTAG_PL, 0, kHitPat, 0);
  int UVeto_id = HistMaker::getUniqueID(kU_Veto, 0, kTDC, 0);
  int SAC_id = HistMaker::getUniqueID(kSAC, 0, kTDC, 0);
  int EVetoL_id = HistMaker::getUniqueID(kE_Veto, 0, kTDC, 0);
  int EVetoR_id = HistMaker::getUniqueID(kE_Veto, 0, kTDC, 1);
  int T0L_id = HistMaker::getUniqueID(kT0, 0, kTDC, 0);
  int T0R_id = HistMaker::getUniqueID(kT0, 0, kTDC, 0) + 1;
  int TOF_id = HistMaker::getUniqueID(kTOF, 0, kHitPat, 0);

  c1->cd(1);
  TH1 *h1 = (TH1*)GHist::get(RF_id);
  h1->Draw();
  c1->cd(2);
  TH1 *h2 = (TH1*)GHist::get(TAG_SFF_id);
  h2->Draw();
  c1->cd(3);
  TH1 *h3 = (TH1*)GHist::get(TAG_SFB_id);
  h3->Draw();
  c1->cd(4);
  TH1 *h4 = (TH1*)GHist::get(TAG_PL_id);
  h4->Draw();
  c1->cd(5);
  TH1 *h5 = (TH1*)GHist::get(UVeto_id);
  h5->Draw();
  c1->cd(6);
  TH1 *h6 = (TH1*)GHist::get(SAC_id);
  h6->Draw();
  c1->cd(7);
  TH1 *h7 = (TH1*)GHist::get(EVetoL_id);
  h7->Draw();
  c1->cd(8);
  TH1 *h8 = (TH1*)GHist::get(EVetoR_id);
  h8->Draw();
  c1->cd(9);
  TH1 *h9 = (TH1*)GHist::get(T0L_id);
  h9->Draw();
  c1->cd(10);
  TH1 *h10 = (TH1*)GHist::get(T0R_id);
  h10->Draw();
  c1->cd(11);
  TH1 *h11 = (TH1*)GHist::get(TOF_id);
  h11->Draw();
  c1->cd(4);
  c1->Update();

  return c1;
}

//_____________________________________________________________________________
TCanvas*
SDCIn_TDC(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int SDC0_base_id = HistMaker::getUniqueID(kSDC0, 0, kTDC, 0);
  int SDC0_base_id_ctot = HistMaker::getUniqueID(kSDC0, 0, kTDC, kTOTcutOffset);
  int SDC1_base_id = HistMaker::getUniqueID(kSDC1, 0, kTDC, 0);
  int SDC1_base_id_ctot = HistMaker::getUniqueID(kSDC1, 0, kTDC, kTOTcutOffset);

  for( int i=0; i<NumOfLayersSDC0; ++i ){
    if(i<2) c->cd(i+1);
    if(i>=2) c->cd(i+2);
    TH1 *h = GHist::get(SDC0_base_id + i);
    h->Draw();
    TH1 *hh =  GHist::get(SDC0_base_id_ctot + i);
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }
  for( int i=0; i<NumOfLayersSDC1; ++i ){
    c->cd(i+7);
    TH1 *h1 = GHist::get(SDC1_base_id + i);
    h1->Draw();
    TH1 *hh1 =  GHist::get(SDC1_base_id_ctot + i);
    if( !hh1 ) continue;
    hh1->SetLineColor( kRed );
    hh1->Draw("same");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
SDCIn_TDC1st(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int SDC0_base_id = HistMaker::getUniqueID(kSDC0, 0, kTDC2D, 0);
  int SDC0_base_id_ctot = HistMaker::getUniqueID(kSDC0, 0, kTDC2D, kTOTcutOffset);
  int SDC1_base_id = HistMaker::getUniqueID(kSDC1, 0, kTDC2D, 0);
  int SDC1_base_id_ctot = HistMaker::getUniqueID(kSDC1, 0, kTDC2D, kTOTcutOffset);

  for( int i=0; i<NumOfLayersSDC0; ++i ){
    if(i<2) c->cd(i+1);
    if(i>=2) c->cd(i+2);
    TH1 *h = (TH1*)GHist::get(SDC0_base_id + i);
    h->Draw();
    TH1 *hh = (TH1*)GHist::get( SDC0_base_id_ctot + i );
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }
  for( int i=0; i<NumOfLayersSDC1; ++i ){
    c->cd(i+7);
    TH1 *h1 = (TH1*)GHist::get(SDC1_base_id + i);
    h1->Draw();
    TH1 *hh1 = (TH1*)GHist::get( SDC1_base_id_ctot + i );
    if( !hh1 ) continue;
    hh1->SetLineColor( kRed );
    hh1->Draw("same");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
SDCIn_TOT(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int SDC0_base_id = HistMaker::getUniqueID(kSDC0, 0, kADC, 0);
  int SDC0_base_id_ctot = HistMaker::getUniqueID(kSDC0, 0, kADC, kTOTcutOffset);
  int SDC0_base_id_tot1st = HistMaker::getUniqueID(kSDC0, 0, kTDC2D,  10+kTOTcutOffset);
  int SDC1_base_id = HistMaker::getUniqueID(kSDC1, 0, kADC, 0);
  int SDC1_base_id_ctot = HistMaker::getUniqueID(kSDC1, 0, kADC, kTOTcutOffset);
  int SDC1_base_id_tot1st = HistMaker::getUniqueID(kSDC1, 0, kTDC2D,  10+kTOTcutOffset);

  for( int i=0; i<NumOfLayersSDC0; ++i ){
    if(i<2) c->cd(i+1);
    if(i>=2) c->cd(i+2);
    TH1 *h = (TH1*)GHist::get(SDC0_base_id + i);
    h->Draw();
    TH1 *h1st = (TH1*)GHist::get(SDC0_base_id_tot1st);
    if( !h1st ) continue;
    h1st->SetLineColor( kGreen );
    h1st->Draw("same");
    TH1 *hh = (TH1*)GHist::get( SDC0_base_id_ctot + i );
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
    TF1 f("f", "gaus", 0., 100.);
    f.SetLineColor(kBlue);
    Double_t p = hh->GetBinCenter(hh->GetMaximumBin());
    // if(p < 30.) p = 70.;
    Double_t w = 10.;
    for(Int_t ifit=0; ifit<3; ++ifit){
      Double_t fmin = p - w;
      Double_t fmax = p + w;
      h->Fit("f", "Q", "", fmin, fmax);
      p = f.GetParameter(1);
      w = f.GetParameter(2) * 1.;
    }
    f.Draw("same");
    TLatex *text = new TLatex();
    text->SetNDC();
    text->SetTextSize(0.07);
    text->DrawLatex(0.400, 0.500, Form("%.1f", p));
  }
  for( int i=0; i<NumOfLayersSDC1; ++i ){
    c->cd(i+7);
    TH1 *h1 = (TH1*)GHist::get(SDC1_base_id + i);
    h1->Draw();
    TH1 *h1st1 = (TH1*)GHist::get(SDC1_base_id_tot1st);
    if( !h1st1 ) continue;
    h1st1->SetLineColor( kGreen );
    h1st1->Draw("same");
    TH1 *hh1 = (TH1*)GHist::get( SDC1_base_id_ctot + i );
    if( !hh1 ) continue;
    hh1->SetLineColor( kRed );
    hh1->Draw("same");
    TF1 f1("f1", "gaus", 0., 100.);
    f1.SetLineColor(kBlue);
    Double_t p = hh1->GetBinCenter(hh1->GetMaximumBin());
    // if(p < 30.) p = 70.;
    Double_t w = 10.;
    for(Int_t ifit=0; ifit<3; ++ifit){
      Double_t fmin = p - w;
      Double_t fmax = p + w;
      h1->Fit("f1", "Q", "", fmin, fmax);
      p = f1.GetParameter(1);
      w = f1.GetParameter(2) * 1.;
    }
    f1.Draw("same");
    TLatex *text1 = new TLatex();
    text1->SetNDC();
    text1->SetTextSize(0.07);
    text1->DrawLatex(0.400, 0.500, Form("%.1f", p));
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
SDCIn_HitPat(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int SDC0_base_id = HistMaker::getUniqueID(kSDC0, 0, kHitPat, 0);
  int SDC0_base_id_ctot = HistMaker::getUniqueID(kSDC0, 0, kHitPat, kTOTcutOffset);
  int SDC1_base_id = HistMaker::getUniqueID(kSDC1, 0, kHitPat, 0);
  int SDC1_base_id_ctot = HistMaker::getUniqueID(kSDC1, 0, kHitPat, kTOTcutOffset);

  for( int i=0; i<NumOfLayersSDC0; ++i ){
    if(i<2) c->cd(i+1);
    if(i>=2) c->cd(i+2);
    TH1 *h = (TH1*)GHist::get(SDC0_base_id + i);
    h->Draw();
    TH1 *hh = (TH1*)GHist::get( SDC0_base_id_ctot + i );
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }
  for( int i=0; i<NumOfLayersSDC1; ++i ){
    c->cd(i+7);
    TH1 *h1 = (TH1*)GHist::get(SDC1_base_id + i);
    h1->Draw();
    TH1 *hh1 = (TH1*)GHist::get( SDC1_base_id_ctot + i );
    if( !hh1 ) continue;
    hh1->SetLineColor( kRed );
    hh1->Draw("same");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
SDCIn_Multi(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int SDC0_base_id = HistMaker::getUniqueID(kSDC0, 0, kMulti, 0);
  int SDC0_base_id_ctot = HistMaker::getUniqueID(kSDC0, 0, kMulti, NumOfLayersSDC0);
  int SDC1_base_id = HistMaker::getUniqueID(kSDC1, 0, kMulti, 0);
  int SDC1_base_id_ctot = HistMaker::getUniqueID(kSDC1, 0, kMulti, NumOfLayersSDC1);

  for( int i=0; i<NumOfLayersSDC0; ++i ){
    if(i<2) c->cd(i+1);
    if(i>=2) c->cd(i+2);
    TH1 *h = (TH1*)GHist::get(SDC0_base_id + i);
    TH1 *hh = (TH1*)GHist::get( SDC0_base_id_ctot + i );
    if( !h || !hh ) continue;
    h->Draw();
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }
  for( int i=0; i<NumOfLayersSDC1; ++i ){
    c->cd(i+7);
    TH1 *h1 = (TH1*)GHist::get(SDC1_base_id + i);
    TH1 *hh1 = (TH1*)GHist::get( SDC1_base_id_ctot + i );
    if( !h1 || !hh1 ) continue;
    h1->Draw();
    hh1->SetLineColor( kRed );
    hh1->Draw("same");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
SDCIn_TDC2D(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC2D
  int SDC0_base_id = HistMaker::getUniqueID(kSDC0, 0, kTDC2D, 20+kTOTcutOffset);
  int SDC1_base_id = HistMaker::getUniqueID(kSDC1, 0, kTDC2D, 20+kTOTcutOffset);

  for( int i=0; i<NumOfLayersSDC0; ++i ){
    if(i<2) c->cd(i+1);
    if(i>=2) c->cd(i+2);
    TH2 *h = (TH2*)GHist::get(SDC0_base_id + i);
    if( !h ) continue;
    h->SetStats(0);
    h->GetYaxis()->SetRangeUser(0,1500);
    h->Draw("colz");
  }
  for( int i=0; i<NumOfLayersSDC1; ++i ){
    c->cd(i+7);
    TH2 *h1 = (TH2*)GHist::get(SDC1_base_id + i);
    if( !h1 ) continue;
    h1->SetStats(0);
    h1->GetYaxis()->SetRangeUser(0,1500);
    h1->Draw("colz");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
SDCIn_TDC2DC(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC2D
  int SDC0_base_id_ctot = HistMaker::getUniqueID(kSDC0, 0, kTDC2D, 30+kTOTcutOffset);
  int SDC1_base_id_ctot = HistMaker::getUniqueID(kSDC1, 0, kTDC2D, 30+kTOTcutOffset);

  for( int i=0; i<NumOfLayersSDC0; ++i ){
    if(i<2) c->cd(i+1);
    if(i>=2) c->cd(i+2);
    TH2 *h = (TH2*)GHist::get(SDC0_base_id_ctot + i);
    if( !h ) continue;
    h->SetStats(0);
    h->GetYaxis()->SetRangeUser(0,1500);
    h->Draw("colz");
  }
  for( int i=0; i<NumOfLayersSDC1; ++i ){
    c->cd(i+7);
    TH2 *h1 = (TH2*)GHist::get(SDC1_base_id_ctot + i);
    if( !h1 ) continue;
    h1->SetStats(0);
    h1->GetYaxis()->SetRangeUser(0,1500);
    h1->Draw("colz");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
SDCOut_TDC(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int SDC2_base_id = HistMaker::getUniqueID(kSDC2, 0, kTDC, 0);
  int SDC2_base_id_ctot = HistMaker::getUniqueID(kSDC2, 0, kTDC, kTOTcutOffset);
  int SDC3_base_id = HistMaker::getUniqueID(kSDC3, 0, kTDC, 0);
  int SDC3_base_id_ctot = HistMaker::getUniqueID(kSDC3, 0, kTDC, kTOTcutOffset);

  for( int i=0; i<NumOfLayersSDC2; ++i ){
    c->cd(i+1);
    TH1 *h = GHist::get(SDC2_base_id + i);
    h->Draw();
    TH1 *hh =  GHist::get(SDC2_base_id_ctot + i);
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }
  for( int i=0; i<NumOfLayersSDC3; ++i ){
    c->cd(i+7);
    TH1 *h1 = GHist::get(SDC3_base_id + i);
    h1->Draw();
    TH1 *hh1 =  GHist::get(SDC3_base_id_ctot + i);
    if( !hh1 ) continue;
    hh1->SetLineColor( kRed );
    hh1->Draw("same");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
SDCOut_TDC1st(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC1st
  int SDC2_base_id = HistMaker::getUniqueID(kSDC2, 0, kTDC2D, 0);
  int SDC2_base_id_ctot = HistMaker::getUniqueID(kSDC2, 0, kTDC2D, kTOTcutOffset);
  int SDC3_base_id = HistMaker::getUniqueID(kSDC3, 0, kTDC2D, 0);
  int SDC3_base_id_ctot = HistMaker::getUniqueID(kSDC3, 0, kTDC2D, kTOTcutOffset);

  for( int i=0; i<NumOfLayersSDC2; ++i ){
    c->cd(i+1);
    TH1 *h = (TH1*)GHist::get(SDC2_base_id + i);
    h->Draw();
    TH1 *hh = (TH1*)GHist::get( SDC2_base_id_ctot + i );
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }
  for( int i=0; i<NumOfLayersSDC3; ++i ){
    c->cd(i+7);
    TH1 *h1 = (TH1*)GHist::get(SDC3_base_id + i);
    h1->Draw();
    TH1 *hh1 = (TH1*)GHist::get( SDC3_base_id_ctot + i );
    if( !hh1 ) continue;
    hh1->SetLineColor( kRed );
    hh1->Draw("same");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
SDCOut_TOT(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TOT
  int SDC2_base_id = HistMaker::getUniqueID(kSDC2, 0, kADC, 0);
  int SDC2_base_id_ctot = HistMaker::getUniqueID(kSDC2, 0, kADC, kTOTcutOffset);
  int SDC2_base_id_tot1st = HistMaker::getUniqueID(kSDC2, 0, kTDC2D,  10+kTOTcutOffset);
  int SDC3_base_id = HistMaker::getUniqueID(kSDC3, 0, kADC, 0);
  int SDC3_base_id_ctot = HistMaker::getUniqueID(kSDC3, 0, kADC, kTOTcutOffset);
  int SDC3_base_id_tot1st = HistMaker::getUniqueID(kSDC3, 0, kTDC2D,  10+kTOTcutOffset);

  for( int i=0; i<NumOfLayersSDC2; ++i ){
    c->cd(i+1);
    TH1 *h = (TH1*)GHist::get(SDC2_base_id + i);
    h->Draw();
    TH1 *h1st = (TH1*)GHist::get(SDC2_base_id_tot1st);
    if( !h1st ) continue;
    h1st->SetLineColor( kGreen );
    h1st->Draw("same");
    TH1 *hh = (TH1*)GHist::get( SDC2_base_id_ctot + i );
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
    TF1 f("f", "gaus", 0., 100.);
    f.SetLineColor(kBlue);
    Double_t p = hh->GetBinCenter(hh->GetMaximumBin());
    // if(p < 30.) p = 70.;
    Double_t w = 10.;
    for(Int_t ifit=0; ifit<3; ++ifit){
      Double_t fmin = p - w;
      Double_t fmax = p + w;
      h->Fit("f", "Q", "", fmin, fmax);
      p = f.GetParameter(1);
      w = f.GetParameter(2) * 1.;
    }
    f.Draw("same");
    TLatex *text = new TLatex();
    text->SetNDC();
    text->SetTextSize(0.07);
    text->DrawLatex(0.400, 0.500, Form("%.1f", p));
  }
  for( int i=0; i<NumOfLayersSDC3; ++i ){
    c->cd(i+7);
    TH1 *h1 = (TH1*)GHist::get(SDC3_base_id + i);
    h1->Draw();
    TH1 *h1st1 = (TH1*)GHist::get(SDC3_base_id_tot1st);
    if( !h1st1 ) continue;
    h1st1->SetLineColor( kGreen );
    h1st1->Draw("same");
    TH1 *hh1 = (TH1*)GHist::get( SDC3_base_id_ctot + i );
    if( !hh1 ) continue;
    hh1->SetLineColor( kRed );
    hh1->Draw("same");
    TF1 f1("f1", "gaus", 0., 100.);
    f1.SetLineColor(kBlue);
    Double_t p = hh1->GetBinCenter(hh1->GetMaximumBin());
    // if(p < 30.) p = 70.;
    Double_t w = 10.;
    for(Int_t ifit=0; ifit<3; ++ifit){
      Double_t fmin = p - w;
      Double_t fmax = p + w;
      h1->Fit("f1", "Q", "", fmin, fmax);
      p = f1.GetParameter(1);
      w = f1.GetParameter(2) * 1.;
    }
    f1.Draw("same");
    TLatex *text1 = new TLatex();
    text1->SetNDC();
    text1->SetTextSize(0.07);
    text1->DrawLatex(0.400, 0.500, Form("%.1f", p));
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
SDCOut_HitPat(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw HitPat
  int SDC2_base_id = HistMaker::getUniqueID(kSDC2, 0, kHitPat, 0);
  int SDC2_base_id_ctot = HistMaker::getUniqueID(kSDC2, 0, kHitPat, kTOTcutOffset);
  int SDC3_base_id = HistMaker::getUniqueID(kSDC3, 0, kHitPat, 0);
  int SDC3_base_id_ctot = HistMaker::getUniqueID(kSDC3, 0, kHitPat, kTOTcutOffset);

  for( int i=0; i<NumOfLayersSDC2; ++i ){
    c->cd(i+1);
    TH1 *h = (TH1*)GHist::get(SDC2_base_id + i);
    h->Draw();
    TH1 *hh = (TH1*)GHist::get( SDC2_base_id_ctot + i );
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }
  for( int i=0; i<NumOfLayersSDC3; ++i ){
    c->cd(i+7);
    TH1 *h1 = (TH1*)GHist::get(SDC3_base_id + i);
    h1->Draw();
    TH1 *hh1 = (TH1*)GHist::get( SDC3_base_id_ctot + i );
    if( !hh1 ) continue;
    hh1->SetLineColor( kRed );
    hh1->Draw("same");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
SDCOut_Multi(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw Multi
  int SDC2_base_id = HistMaker::getUniqueID(kSDC2, 0, kMulti, 0);
  int SDC2_base_id_ctot = HistMaker::getUniqueID(kSDC2, 0, kMulti, NumOfLayersSDC2);
  int SDC3_base_id = HistMaker::getUniqueID(kSDC3, 0, kMulti, 0);
  int SDC3_base_id_ctot = HistMaker::getUniqueID(kSDC3, 0, kMulti, NumOfLayersSDC3);

  for( int i=0; i<NumOfLayersSDC2; ++i ){
    c->cd(i+1);
    TH1 *h = (TH1*)GHist::get(SDC2_base_id + i);
    TH1 *hh = (TH1*)GHist::get( SDC2_base_id_ctot + i );
    if( !h || !hh ) continue;
    h->Draw();
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }
  for( int i=0; i<NumOfLayersSDC3; ++i ){
    c->cd(i+7);
    TH1 *h1 = (TH1*)GHist::get(SDC3_base_id + i);
    TH1 *hh1 = (TH1*)GHist::get( SDC3_base_id_ctot + i );
    if( !h1 || !hh1 ) continue;
    h1->Draw();
    hh1->SetLineColor( kRed );
    hh1->Draw("same");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
SDCOut_TDC2D(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC2D
  int SDC2_base_id = HistMaker::getUniqueID(kSDC2, 0, kTDC2D, 20+kTOTcutOffset);
  int SDC3_base_id = HistMaker::getUniqueID(kSDC3, 0, kTDC2D, 20+kTOTcutOffset);

  for( int i=0; i<NumOfLayersSDC2; ++i ){
    c->cd(i+1);
    TH2 *h = (TH2*)GHist::get(SDC2_base_id + i);
    if( !h ) continue;
    h->SetStats(0);
    h->GetYaxis()->SetRangeUser(0,1500);
    h->Draw("colz");
  }
  for( int i=0; i<NumOfLayersSDC3; ++i ){
    c->cd(i+7);
    TH2 *h1 = (TH2*)GHist::get(SDC3_base_id + i);
    if( !h1 ) continue;
    h1->SetStats(0);
    h1->GetYaxis()->SetRangeUser(0,1500);
    h1->Draw("colz");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
SDCOut_TDC2DC(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC2DC
  int SDC2_base_id_ctot = HistMaker::getUniqueID(kSDC2, 0, kTDC2D, 30+kTOTcutOffset);
  int SDC3_base_id_ctot = HistMaker::getUniqueID(kSDC3, 0, kTDC2D, 30+kTOTcutOffset);

  for( int i=0; i<NumOfLayersSDC2; ++i ){
    c->cd(i+1);
    TH2 *h = (TH2*)GHist::get(SDC2_base_id_ctot + i);
    if( !h ) continue;
    h->SetStats(0);
    h->GetYaxis()->SetRangeUser(0,1500);
    h->Draw("colz");
  }
  for( int i=0; i<NumOfLayersSDC3; ++i ){
    c->cd(i+7);
    TH2 *h1 = (TH2*)GHist::get(SDC3_base_id_ctot + i);
    if( !h1 ) continue;
    h1->SetStats(0);
    h1->GetYaxis()->SetRangeUser(0,1500);
    h1->Draw("colz");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
SDC_Correlation(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw SDC_Correlation
  int SDC0_base_id = HistMaker::getUniqueID(kSDC0, 0, kCorr, 20);
  int SDC1_base_id = HistMaker::getUniqueID(kSDC1, 0, kCorr, 20);
  int SDC2_base_id = HistMaker::getUniqueID(kSDC2, 0, kCorr, 20);
  int SDC3_base_id = HistMaker::getUniqueID(kSDC3, 0, kCorr, 20);

  for( int i=0; i<2; ++i ){
    c->cd(i+1)->SetLogz();
    TH1 *h = (TH1*)GHist::get(SDC0_base_id + i);
    h->Draw();
  }
  for( int i=0; i<2; ++i ){
    c->cd(i+4)->SetLogz();
    TH1 *h1 = (TH1*)GHist::get(SDC1_base_id + i);
    h1->Draw();
  }
  for( int i=0; i<2; ++i ){
    c->cd(i+7)->SetLogz();
    TH1 *h2 = (TH1*)GHist::get(SDC2_base_id + i);
    h2->Draw();
  }
  for( int i=0; i<2; ++i ){
    c->cd(i+10)->SetLogz();
    TH1 *h3 = (TH1*)GHist::get(SDC3_base_id + i);
    h3->Draw();
  }
  c->Update();
  return c;
}



//_____________________________________________________________________________
TCanvas*
TOF_ADCU1()
{
  auto c = new TCanvas(__func__, __func__);
  c->Divide(4, 4);

  static const Int_t seg_per_canvas = 16;
  static const Int_t adc_min = 0;
  static const Int_t adc_max = 2048;

  // draw ADC U12-27
  int TOF_base_id = HistMaker::getUniqueID( kTOF, 0, kADC, 12);
  int TOF_base_id_wtdc = HistMaker::getUniqueID( kTOF, 0, kADCwTDC, 12);
  for( int i=0; i<seg_per_canvas; ++i ){
    c->cd(i+1)->SetLogy();
    TH1 *h = (TH1*)GHist::get( TOF_base_id + i );
    if( !h ) continue;
    h->GetXaxis()->SetRangeUser( adc_min, adc_max);
    h->Draw();
    TH1 *hh = (TH1*)GHist::get( TOF_base_id_wtdc + i );
    if( !hh ) continue;
    hh->GetXaxis()->SetRangeUser( adc_min, adc_max);
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TOF_ADCU2()
{
  auto c = new TCanvas(__func__, __func__);
  c->Divide(4, 4);

  static const Int_t seg_per_canvas = 16;
  static const Int_t adc_min = 0;
  static const Int_t adc_max = 2048;

  // draw ADC U28-43
  int TOF_base_id = HistMaker::getUniqueID( kTOF, 0, kADC, 28);
  int TOF_base_id_wtdc = HistMaker::getUniqueID( kTOF, 0, kADCwTDC, 28);
  for( int i=0; i<seg_per_canvas; ++i ){
    c->cd(i+1)->SetLogy();
    TH1 *h = (TH1*)GHist::get( TOF_base_id + i );
    if( !h ) continue;
    h->GetXaxis()->SetRangeUser( adc_min, adc_max);
    h->Draw();
    TH1 *hh = (TH1*)GHist::get( TOF_base_id_wtdc + i );
    if( !hh ) continue;
    hh->GetXaxis()->SetRangeUser( adc_min, adc_max);
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TOF_ADCD1()
{
  auto c = new TCanvas(__func__, __func__);
  c->Divide(4, 4);

  static const Int_t seg_per_canvas = 16;
  static const Int_t adc_min = 0;
  static const Int_t adc_max = 2048;

  // draw ADC D12-27
  int TOF_base_id = HistMaker::getUniqueID( kTOF, 0, kADC, NumOfSegTOF + 12);
  int TOF_base_id_wtdc = HistMaker::getUniqueID( kTOF, 0, kADCwTDC,NumOfSegTOF +  12);
  for( int i=0; i<seg_per_canvas; ++i ){
    c->cd(i+1)->SetLogy();
    TH1 *h = (TH1*)GHist::get( TOF_base_id + i );
    if( !h ) continue;
    h->GetXaxis()->SetRangeUser( adc_min, adc_max);
    h->Draw();
    TH1 *hh = (TH1*)GHist::get( TOF_base_id_wtdc + i );
    if( !hh ) continue;
    hh->GetXaxis()->SetRangeUser( adc_min, adc_max);
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TOF_ADCD2()
{
  auto c = new TCanvas(__func__, __func__);
  c->Divide(4, 4);

  static const Int_t seg_per_canvas = 16;
  static const Int_t adc_min = 0;
  static const Int_t adc_max = 2048;

  // draw ADC D28-43
  int TOF_base_id = HistMaker::getUniqueID( kTOF, 0, kADC, NumOfSegTOF + 28);
  int TOF_base_id_wtdc = HistMaker::getUniqueID( kTOF, 0, kADCwTDC, NumOfSegTOF + 28);
  for( int i=0; i<seg_per_canvas; ++i ){
    c->cd(i+1)->SetLogy();
    TH1 *h = (TH1*)GHist::get( TOF_base_id + i );
    if( !h ) continue;
    h->GetXaxis()->SetRangeUser( adc_min, adc_max);
    h->Draw();
    TH1 *hh = (TH1*)GHist::get( TOF_base_id_wtdc + i );
    if( !hh ) continue;
    hh->GetXaxis()->SetRangeUser( adc_min, adc_max);
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }
  c->Update();
  return c;
}


//_____________________________________________________________________________
TCanvas*
DAQ()
{
  std::vector<Int_t> hist_id = {
    HistMaker::getUniqueID(kDAQ, kEB, kHitPat),
    HistMaker::getUniqueID(kDAQ, kVME, kHitPat2D),
    HistMaker::getUniqueID(kDAQ, kEASIROC, kHitPat2D),
    HistMaker::getUniqueID(kDAQ, kHUL, kHitPat2D),
    HistMaker::getUniqueID(kDAQ, kVMEEASIROC, kHitPat2D),
    // HistMaker::getUniqueID(kDAQ,  kCLite,   kHitPat2D),
    // HistMaker::getUniqueID(kDAQ,  kOpt,     kHitPat2D),
  };
  auto c1 = new TCanvas(__func__, __func__);
  c1->Divide(3, 2);
  for(Int_t i=0, n=hist_id.size(); i<n; ++i){
    c1->cd(i + 1); //->SetGrid();
    auto h1 = GHist::get(hist_id.at(i));
    if(!h1) continue;
    h1->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTTDC()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kTDC, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTTDC2D()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kTDC2D, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTTOT()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kADC, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTTOT2D()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kADC2D, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTHighGain()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kHighGain, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTHighGain2D()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kHighGain, l+11));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTLowGain()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kLowGain, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTLowGain2D()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kLowGain, l+11));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTPedestal()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kPede, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTPedestal2D()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kPede, l+11));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTHitPat()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h1 = GHist::get(HistMaker::getUniqueID(kCFT, 0, kHitPat, l+1));
    if(!h1) continue;
    h1->Draw();
    TH1 *h2 = GHist::get(HistMaker::getUniqueID(kCFT, 0, kHitPat, l+11));
    if(!h2) continue;
    h2->SetLineColor(kRed+1);
    h2->Draw("same");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTMulti()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h1 = GHist::get(HistMaker::getUniqueID(kCFT, 0, kMulti, l+1));
    if(!h1) continue;
    TH1 *h2 = GHist::get(HistMaker::getUniqueID(kCFT, 0, kMulti, l+11));
    if(!h2) continue;
    h2->SetLineColor(kRed+1);
    h2->Draw();
    h1->Draw("same");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTClusterHighGain()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, kCluster, kHighGain, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTClusterHighGain2D()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, kCluster, kHighGain, l+11));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTClusterLowGain()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, kCluster, kLowGain, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTClusterLowGain2D()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, kCluster, kLowGain, l+11));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTClusterTDC()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, kCluster, kTDC, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTClusterTDC2D()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, kCluster, kTDC2D, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BGOFADC()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(6, 4);
  TH1 *h;
  for(Int_t i=0; i<NumOfSegBGO; ++i){
    c1->cd(i+1);
    h = GHist::get(HistMaker::getUniqueID(kBGO, 0, kFADC, i+1));
    if(!h) continue;
    h->GetXaxis()->SetRangeUser(80, 200);
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BGOADC()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(6, 4);
  TH1 *h;
  for(Int_t i=0; i<NumOfSegBGO; ++i){
    c1->cd(i+1);
    h = GHist::get(HistMaker::getUniqueID(kBGO, 0, kADC, i+1));
    if(!h) continue;
    h->Draw();
    h = GHist::get(HistMaker::getUniqueID(kBGO, 0, kADCwTDC, i+1));
    if(!h) continue;
    h->SetLineColor(kRed+1);
    h->Draw("same");
  }
  return c1;
}

//_____________________________________________________________________________
// TCanvas*
// BGOTDC()
// {
//   TCanvas *c1 = new TCanvas(__func__, __func__);
//   c1->Divide(6, 4);
//   TH1 *h;
//   for(Int_t i=0; i<NumOfSegBGO; ++i){
//     c1->cd(i+1);
//     h = GHist::get(HistMaker::getUniqueID(kBGO, 0, kTDC, i+1));
//     if(!h) continue;
//     h->Draw();
//   }
//   return c1;
// }

//_____________________________________________________________________________
TCanvas*
BGOADCTDC2D()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(2, 2);
  TH1 *h;
  for(Int_t i=0; i<3; ++i){
    c1->cd(i+1);
    h = GHist::get(HistMaker::getUniqueID(kBGO, 0, kADC2D, i+1));
    if(!h) continue;
    h->Draw("colz");
  }
  h = GHist::get(HistMaker::getUniqueID(kBGO, 0, kTDC2D));
  c1->cd(4);
  if(h)
    h->Draw("colz");
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BGOHitMulti()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  TH1 *h;
  Int_t id = HistMaker::getUniqueID(kBGO, 0, kTDC, 25);
  for(Int_t i=0; i<NumOfSegBGO_T; ++i){
    c1->cd(i+1);
    h = (TH1*)GHist::get(id + i);
    if(!h) continue;
    h->Draw();
  }
  id = HistMaker::getUniqueID(kBGO, 0, kHitPat, 1);
  h = (TH1*)GHist::get(id);
  c1->cd(5);
  h->Draw();
  id = HistMaker::getUniqueID(kBGO, 0, kHitPat, 2);
  h = (TH1*)GHist::get(id);
  c1->cd(6);
  h->Draw();
  id = HistMaker::getUniqueID(kBGO, 0, kMulti, 1);
  h = (TH1*)GHist::get(id);
  c1->cd(7);
  h->Draw();
  id = HistMaker::getUniqueID(kBGO, 0, kMulti, 2);
  h = (TH1*)GHist::get(id);
  c1->cd(8);
  h->Draw();
  return c1;
}

//_____________________________________________________________________________
TCanvas*
PiIDTDC()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(8, 4);
  TH1 *h;
  for(Int_t i=0; i<NumOfSegPiID; ++i){
    c1->cd(i+1);
    h = GHist::get(HistMaker::getUniqueID(kPiID, 0, kTDC, i+1));
    if(!h) continue;
    h->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
PiIDHighGain()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(8, 4);
  TH1 *h;
  for(Int_t i=0; i<NumOfSegPiID; ++i){
    c1->cd(i+1);
    h = GHist::get(HistMaker::getUniqueID(kPiID, 0, kHighGain, i+1));
    if(!h) continue;
    h->Draw();
    h = GHist::get(HistMaker::getUniqueID(kPiID, 0, kADCwTDC, i+1));
    if(!h) continue;
    h->SetLineColor(kRed+1);
    h->Draw("same");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
PiIDLowGain()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(8, 4);
  TH1 *h;
  for(Int_t i=0; i<NumOfSegPiID; ++i){
    c1->cd(i+1);
    h = GHist::get(HistMaker::getUniqueID(kPiID, 0, kLowGain, i+1));
    if(!h) continue;
    h->Draw();
    h = GHist::get(HistMaker::getUniqueID(kPiID, 0, kADCwTDC, i+1+NumOfSegPiID));
    if(!h) continue;
    h->SetLineColor(kRed+1);
    h->Draw("same");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
PiIDHitMulti()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(2, 2);
  std::vector<Int_t> id = {
    HistMaker::getUniqueID(kPiID, 0, kHitPat, 1),
    HistMaker::getUniqueID(kPiID, 0, kHitPat, 2),
    HistMaker::getUniqueID(kPiID, 0, kMulti,  1),
    HistMaker::getUniqueID(kPiID, 0, kMulti,  2)
  };
  TH1 *h;
  for(Int_t i=0, n=id.size(); i<n; ++i){
    c1->cd(i+1);
    h = GHist::get(id.at(i));
    if(!h) continue;
    h->Draw();
  }
  return c1;
}
//_____________________________________________________________________________
TCanvas*
CFTEfficiency()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4,4);
  for(Int_t i=0; i<NumOfLayersCFT; ++i){
    c1->cd(i+1);
    TH1 *h1 = GHist::get(HistMaker::getUniqueID(kCFT, 0, kMulti, i+11));
    if(!h1) continue;
    h1->Draw();
    c1->cd(i+1+NumOfLayersCFT);
    TH1 *h2 = GHist::get(HistMaker::getUniqueID(kCFT, 0, kMulti, i+21));
    if(!h2) continue;
    h2->Draw();
  }
  return c1;
}

} // namespace http

} // namespace analyzer
