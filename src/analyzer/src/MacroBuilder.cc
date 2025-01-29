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
#include <TStyle.h>
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

[[maybe_unused]] void
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
  int TOF_multi_id = HistMaker::getUniqueID(kTOF, 0, kMulti, 0);


  c1->cd(1);
  TH1 *h2 = (TH1*)GHist::get(TAG_SFF_id);
  h2->Draw();
  c1->cd(2);
  TH1 *h3 = (TH1*)GHist::get(TAG_SFB_id);
  h3->Draw();
  c1->cd(3);
  TH1 *h4 = (TH1*)GHist::get(TAG_PL_id);
  h4->Draw();

  c1->cd(4);
  TH1 *h1 = (TH1*)GHist::get(RF_id);
  h1->Draw();
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

  c1->cd(10);
  TH1 *h9 = (TH1*)GHist::get(T0L_id);
  h9->Draw();
  c1->cd(11);
  TH1 *h10 = (TH1*)GHist::get(T0R_id);
  h10->Draw();
  c1->cd(12);
  TH1 *h11 = (TH1*)GHist::get(TOF_id);
  h11->Draw();

  c1->cd(9);
  TH1 *h12 = (TH1*)GHist::get(TOF_multi_id);
  h12->Draw();

  c1->Update();

  return c1;
}

//_____________________________________________________________________________
TCanvas*
TAG_SFF_TDC1(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int TAG_SF_base_id = HistMaker::getUniqueID(kTAG_SF, 0, kTDC, 0);

  for( int i=0; i<11; ++i ){
    c->cd(i+1);
    TH1 *h = GHist::get(TAG_SF_base_id + i);
    h->Draw();
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TAG_SFF_TDC2(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int TAG_SF_base_id = HistMaker::getUniqueID(kTAG_SF, 0, kTDC, 11);

  for( int i=0; i<11; ++i ){
    c->cd(i+1);
    TH1 *h = GHist::get(TAG_SF_base_id + i);
    h->Draw();
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TAG_SFF_TDC3(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int TAG_SF_base_id = HistMaker::getUniqueID(kTAG_SF, 0, kTDC, 22);

  for( int i=0; i<11; ++i ){
    c->cd(i+1);
    TH1 *h = GHist::get(TAG_SF_base_id + i);
    h->Draw();
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TAG_SFF_TDC4(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int TAG_SF_base_id = HistMaker::getUniqueID(kTAG_SF, 0, kTDC, 33);

  for( int i=0; i<11; ++i ){
    c->cd(i+1);
    TH1 *h = GHist::get(TAG_SF_base_id + i);
    h->Draw();
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TAG_SFF_TDC5(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int TAG_SF_base_id = HistMaker::getUniqueID(kTAG_SF, 0, kTDC, 44);

  for( int i=0; i<11; ++i ){
    c->cd(i+1);
    TH1 *h = GHist::get(TAG_SF_base_id + i);
    h->Draw();
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TAG_SFB_TDC1(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int TAG_SF_base_id = HistMaker::getUniqueID(kTAG_SF, 0, kTDC, NumOfSegTAG_SF);

  for( int i=0; i<11; ++i ){
    c->cd(i+1);
    TH1 *h = GHist::get(TAG_SF_base_id + i);
    h->Draw();
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TAG_SFB_TDC2(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int TAG_SF_base_id = HistMaker::getUniqueID(kTAG_SF, 0, kTDC, NumOfSegTAG_SF + 11);

  for( int i=0; i<11; ++i ){
    c->cd(i+1);
    TH1 *h = GHist::get(TAG_SF_base_id + i);
    h->Draw();
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TAG_SFB_TDC3(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int TAG_SF_base_id = HistMaker::getUniqueID(kTAG_SF, 0, kTDC, NumOfSegTAG_SF + 22);

  for( int i=0; i<11; ++i ){
    c->cd(i+1);
    TH1 *h = GHist::get(TAG_SF_base_id + i);
    h->Draw();
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TAG_SFB_TDC4(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int TAG_SF_base_id = HistMaker::getUniqueID(kTAG_SF, 0, kTDC, NumOfSegTAG_SF + 33);

  for( int i=0; i<11; ++i ){
    c->cd(i+1);
    TH1 *h = GHist::get(TAG_SF_base_id + i);
    h->Draw();
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TAG_SFB_TDC5(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int TAG_SF_base_id = HistMaker::getUniqueID(kTAG_SF, 0, kTDC, NumOfSegTAG_SF + 44);

  for( int i=0; i<11; ++i ){
    c->cd(i+1);
    TH1 *h = GHist::get(TAG_SF_base_id + i);
    h->Draw();
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TAG_PL_TDC(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int TAG_PL_base_id = HistMaker::getUniqueID(kTAG_PL, 0, kTDC, 0);

  for( int i=0; i<NumOfSegTAG_PL; ++i ){
    c->cd(i+1);
    TH1 *h = GHist::get(TAG_PL_base_id + i);
    h->Draw();
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TAG_Multi(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(2, 2);

  int TAG_SF_base_id = HistMaker::getUniqueID(kTAG_SF, 0, kMulti, 0);
  int TAG_PL_base_id = HistMaker::getUniqueID(kTAG_PL, 0, kMulti, 0);

  //draw TAG_SF_Multi
  for( int i=0; i<NumOfLayersTAG_SF; ++i ){
    c->cd(i+1);
    TH1 *h = GHist::get(TAG_SF_base_id + i);
    h->Draw();
  }
  c->cd(3);
  TH1 *h = GHist::get(TAG_PL_base_id);
  h->Draw();
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
U_Veto(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(2, 2);

  int base_id_a   = HistMaker::getUniqueID(kU_Veto, 0, kADC, 0);
  int base_id_awt = HistMaker::getUniqueID(kU_Veto, 0, kADCwTDC, 0);
  int base_id_t   = HistMaker::getUniqueID(kU_Veto, 0, kTDC, 0);
  int base_id_h   = HistMaker::getUniqueID(kU_Veto, 0, kHitPat, 0);
  int base_id_m   = HistMaker::getUniqueID(kU_Veto, 0, kMulti, 0);

  c->cd(1);
  TH1 *h1 = GHist::get(base_id_a);
  h1->Draw();
  TH1 *hh1 = GHist::get(base_id_awt);
  hh1->SetLineColor(kRed);
  hh1->Draw("same");

  c->cd(2);
  TH1 *h2 = GHist::get(base_id_t);
  h2->Draw();

  c->cd(3);
  TH1 *h3 = GHist::get(base_id_h);
  h3->Draw();

  c->cd(4);
  TH1 *h4 = GHist::get(base_id_m);
  h4->Draw();

  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
T0(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(2, 3);

  int base_id_a   = HistMaker::getUniqueID(kT0, 0, kADC, 0);
  int base_id_awt = HistMaker::getUniqueID(kT0, 0, kADCwTDC, 0);
  int base_id_t   = HistMaker::getUniqueID(kT0, 0, kTDC, 0);
  int base_id_h   = HistMaker::getUniqueID(kT0, 0, kHitPat, 0);
  int base_id_m   = HistMaker::getUniqueID(kT0, 0, kMulti, 0);

  for (int i=0; i<2; i++){
    c->cd(i+1);
    TH1 *h1 = GHist::get(base_id_a + i);
    h1->Draw();
    TH1 *hh1 = GHist::get(base_id_awt + i);
    hh1->SetLineColor(kRed);
    hh1->Draw("same");
  }
  for (int i=0; i<2; i++){
    c->cd(i+3);
    TH1 *h2 = GHist::get(base_id_t + i);
    h2->Draw();
  }
  c->cd(5);
  TH1 *h3 = GHist::get(base_id_h);
  h3->Draw();
  c->cd(6);
  TH1 *h4 = GHist::get(base_id_m);
  h4->Draw();

  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
SAC(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(4, 4);

  int base_id_a   = HistMaker::getUniqueID(kSAC, 0, kADC, 0);
  int base_id_awt = HistMaker::getUniqueID(kSAC, 0, kADCwTDC, 0);
  int base_id_t = HistMaker::getUniqueID(kSAC, 0, kTDC, 0);
  int base_id_h   = HistMaker::getUniqueID(kSAC, 0, kHitPat, 0);
  int base_id_m = HistMaker::getUniqueID(kSAC, 0, kMulti, 0);

  c->cd(1);
  TH1 *h1 = GHist::get(base_id_a);
  h1->Draw();
  TH1 *hh1 = GHist::get(base_id_awt);
  hh1->SetLineColor(kRed);
  hh1->Draw("same");

  for (int i=1; i<NumOfSegSAC; i++){
    c->cd(i+4);
    TH1 *h2 = GHist::get(base_id_a+i);
    h2->Draw();
    TH1 *hh2 = GHist::get(base_id_awt+i);
    hh2->SetLineColor(kRed);
    hh2->Draw("same");
  }

  c->cd(9);
  TH1 *h3 = GHist::get(base_id_t);
  h3->Draw();

  c->cd(10);
  TH1 *h4 = GHist::get(base_id_h);
  h4->Draw();

  c->cd(11);
  TH1 *h5 = GHist::get(base_id_m);
  h5->Draw();

  for (int i=1; i<NumOfSegSAC; i++){
    c->cd(i+12);
    TH1 *h6 = GHist::get(base_id_m+i);
    h6->Draw();
  }

  c->Update();
  return c;
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
    h->Draw("colz");
  }
  for( int i=0; i<NumOfLayersSDC1; ++i ){
    c->cd(i+7);
    TH2 *h1 = (TH2*)GHist::get(SDC1_base_id + i);
    if( !h1 ) continue;
    h1->SetStats(0);
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
    h->Draw("colz");
  }
  for( int i=0; i<NumOfLayersSDC1; ++i ){
    c->cd(i+7);
    TH2 *h1 = (TH2*)GHist::get(SDC1_base_id_ctot + i);
    if( !h1 ) continue;
    h1->SetStats(0);
    h1->Draw("colz");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
SDCIn_TOTTDC2D(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int SDC0_base_id = HistMaker::getUniqueID(kSDC0, 0, kTOTTDC2D,  kTOTcutOffset);
  int SDC1_base_id = HistMaker::getUniqueID(kSDC1, 0, kTOTTDC2D,  kTOTcutOffset);

  for( int i=0; i<NumOfLayersSDC0; ++i ){
    if(i<2) c->cd(i+1);
    if(i>=2) c->cd(i+2);
    TH1 *h = GHist::get(SDC0_base_id + i);
    h->Draw("colz");
  }
  for( int i=0; i<NumOfLayersSDC1; ++i ){
    c->cd(i+7);
    TH1 *h1 = GHist::get(SDC1_base_id + i);
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
  c->Clear();
  c->Divide(3, 4);

  //draw TDC2D
  int SDC2_base_id = HistMaker::getUniqueID(kSDC2, 0, kTDC2D, 20+kTOTcutOffset);
  int SDC3_base_id = HistMaker::getUniqueID(kSDC3, 0, kTDC2D, 20+kTOTcutOffset);

  for( int i=0; i<NumOfLayersSDC2; ++i ){
    c->cd(i+1);
    TH2 *h = (TH2*)GHist::get(SDC2_base_id + i);
    if( !h ) continue;
    h->SetStats(0);
    h->Draw("colz");
  }
  for( int i=0; i<NumOfLayersSDC3; ++i ){
    c->cd(i+7);
    TH2 *h1 = (TH2*)GHist::get(SDC3_base_id + i);
    if( !h1 ) continue;
    h1->SetStats(0);
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
    h->Draw("colz");
  }
  for( int i=0; i<NumOfLayersSDC3; ++i ){
    c->cd(i+7);
    TH2 *h1 = (TH2*)GHist::get(SDC3_base_id_ctot + i);
    if( !h1 ) continue;
    h1->SetStats(0);
    h1->Draw("colz");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
SDCOut_TOTTDC2D(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(3, 4);

  //draw TDC
  int SDC2_base_id = HistMaker::getUniqueID(kSDC2, 0, kTOTTDC2D,  kTOTcutOffset);
  int SDC3_base_id = HistMaker::getUniqueID(kSDC3, 0, kTOTTDC2D,  kTOTcutOffset);

  for( int i=0; i<NumOfLayersSDC2; ++i ){
    c->cd(i+1);
    TH1 *h = GHist::get(SDC2_base_id + i);
    h->Draw("colz");
  }
  for( int i=0; i<NumOfLayersSDC3; ++i ){
    c->cd(i+7);
    TH1 *h1 = GHist::get(SDC3_base_id + i);
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
    gStyle->SetStatY(0.4);
  }
  for( int i=0; i<2; ++i ){
    c->cd(i+4)->SetLogz();
    TH1 *h1 = (TH1*)GHist::get(SDC1_base_id + i);
    h1->Draw();
    gStyle->SetStatY(0.4);
  }
  for( int i=0; i<2; ++i ){
    c->cd(i+7)->SetLogz();
    TH1 *h2 = (TH1*)GHist::get(SDC2_base_id + i);
    h2->Draw();
    gStyle->SetStatY(0.4);
  }
  for( int i=0; i<2; ++i ){
    c->cd(i+10)->SetLogz();
    TH1 *h3 = (TH1*)GHist::get(SDC3_base_id + i);
    h3->Draw();
    gStyle->SetStatY(0.4);
  }
  c->Update();
  gStyle->SetStatY(1.0);
  return c;
}

//_____________________________________________________________________________
TCanvas*
E_Veto(){
  auto c = new TCanvas(__func__, __func__);
  c->Divide(2, 3);

  int base_id_a   = HistMaker::getUniqueID(kE_Veto, 0, kADC, 0);
  int base_id_awt = HistMaker::getUniqueID(kE_Veto, 0, kADCwTDC, 0);
  int base_id_t   = HistMaker::getUniqueID(kE_Veto, 0, kTDC, 0);
  int base_id_h   = HistMaker::getUniqueID(kE_Veto, 0, kHitPat, 0);
  int base_id_m   = HistMaker::getUniqueID(kE_Veto, 0, kMulti, 0);

  for (int i=0; i<2; i++){
    c->cd(i+1);
    TH1 *h1 = GHist::get(base_id_a + i);
    h1->Draw();
    TH1 *hh1 = GHist::get(base_id_awt + i);
    hh1->SetLineColor(kRed);
    hh1->Draw("same");
  }
  for (int i=0; i<2; i++){
    c->cd(i+3);
    TH1 *h2 = GHist::get(base_id_t + i);
    h2->Draw();
  }
  c->cd(5);
  TH1 *h3 = GHist::get(base_id_h);
  h3->Draw();
  c->cd(6);
  TH1 *h4 = GHist::get(base_id_m);
  h4->Draw();

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

  // draw ADC U0-15
  int TOF_base_id = HistMaker::getUniqueID( kTOF, 0, kADC, 0);
  int TOF_base_id_wtdc = HistMaker::getUniqueID( kTOF, 0, kADCwTDC, 0);
  for( int i=0; i<seg_per_canvas; ++i ){
    c->cd(i+1)->SetLogy();
    TH1 *h = (TH1*)GHist::get( TOF_base_id + i );
    if( !h ) continue;
    h->Draw();
    TH1 *hh = (TH1*)GHist::get( TOF_base_id_wtdc + i );
    if( !hh ) continue;
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

  // draw ADC U16-31
  int TOF_base_id = HistMaker::getUniqueID( kTOF, 0, kADC, 16);
  int TOF_base_id_wtdc = HistMaker::getUniqueID( kTOF, 0, kADCwTDC, 16);
  for( int i=0; i<seg_per_canvas; ++i ){
    c->cd(i+1)->SetLogy();
    TH1 *h = (TH1*)GHist::get( TOF_base_id + i );
    if( !h ) continue;
    h->Draw();
    TH1 *hh = (TH1*)GHist::get( TOF_base_id_wtdc + i );
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TOF_ADCU3()
{
  auto c = new TCanvas(__func__, __func__);
  c->Divide(4, 4);

  static const Int_t seg_per_canvas = 16;

  // draw ADC U32-48
  int TOF_base_id = HistMaker::getUniqueID( kTOF, 0, kADC, 32);
  int TOF_base_id_wtdc = HistMaker::getUniqueID( kTOF, 0, kADCwTDC, 32);
  for( int i=0; i<seg_per_canvas; ++i ){
    c->cd(i+1)->SetLogy();
    TH1 *h = (TH1*)GHist::get( TOF_base_id + i );
    if( !h ) continue;
    h->Draw();
    TH1 *hh = (TH1*)GHist::get( TOF_base_id_wtdc + i );
    if( !hh ) continue;
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

  // draw ADC U0-15
  int TOF_base_id = HistMaker::getUniqueID( kTOF, 0, kADC,NumOfSegTOF +  0);
  int TOF_base_id_wtdc = HistMaker::getUniqueID( kTOF, 0, kADCwTDC, NumOfSegTOF + 0);
  for( int i=0; i<seg_per_canvas; ++i ){
    c->cd(i+1)->SetLogy();
    TH1 *h = (TH1*)GHist::get( TOF_base_id + i );
    if( !h ) continue;
    h->Draw();
    TH1 *hh = (TH1*)GHist::get( TOF_base_id_wtdc + i );
    if( !hh ) continue;
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

  // draw ADC U16-31
  int TOF_base_id = HistMaker::getUniqueID( kTOF, 0, kADC, NumOfSegTOF + 16);
  int TOF_base_id_wtdc = HistMaker::getUniqueID( kTOF, 0, kADCwTDC, NumOfSegTOF + 16);
  for( int i=0; i<seg_per_canvas; ++i ){
    c->cd(i+1)->SetLogy();
    TH1 *h = (TH1*)GHist::get( TOF_base_id + i );
    if( !h ) continue;
    h->Draw();
    TH1 *hh = (TH1*)GHist::get( TOF_base_id_wtdc + i );
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TOF_ADCD3()
{
  auto c = new TCanvas(__func__, __func__);
  c->Divide(4, 4);

  static const Int_t seg_per_canvas = 16;

  // draw ADC U32-48
  int TOF_base_id = HistMaker::getUniqueID( kTOF, 0, kADC, NumOfSegTOF + 32);
  int TOF_base_id_wtdc = HistMaker::getUniqueID( kTOF, 0, kADCwTDC, NumOfSegTOF + 32);
  for( int i=0; i<seg_per_canvas; ++i ){
    c->cd(i+1)->SetLogy();
    TH1 *h = (TH1*)GHist::get( TOF_base_id + i );
    if( !h ) continue;
    h->Draw();
    TH1 *hh = (TH1*)GHist::get( TOF_base_id_wtdc + i );
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________

//tdc sita
//_____________________________________________________________________________

TCanvas*
TOF_TDCU1()
{
  auto c = new TCanvas(__func__, __func__);
  c->Divide(4, 4);

  static const Int_t seg_per_canvas = 16;

  // draw TDC U0-15
  int TOF_base_id = HistMaker::getUniqueID( kTOF, 0, kTDC, 0);

  for( int i=0; i<seg_per_canvas; ++i ){
    c->cd(i+1)->SetLogy();
    TH1 *h = (TH1*)GHist::get( TOF_base_id + i );
    if( !h ) continue;
    h->Draw();
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TOF_TDCU2()
{
  auto c = new TCanvas(__func__, __func__);
  c->Divide(4, 4);

  static const Int_t seg_per_canvas = 16;

  // draw TDC U16-31
  int TOF_base_id = HistMaker::getUniqueID( kTOF, 0, kTDC, 16);

  for( int i=0; i<seg_per_canvas; ++i ){
    c->cd(i+1)->SetLogy();
    TH1 *h = (TH1*)GHist::get( TOF_base_id + i );
    if( !h ) continue;
    h->Draw();
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TOF_TDCU3()
{
  auto c = new TCanvas(__func__, __func__);
  c->Divide(4, 4);

  static const Int_t seg_per_canvas = 16;

  // draw TDC U32-48
  int TOF_base_id = HistMaker::getUniqueID( kTOF, 0, kTDC, 32);

  for( int i=0; i<seg_per_canvas; ++i ){
    c->cd(i+1)->SetLogy();
    TH1 *h = (TH1*)GHist::get( TOF_base_id + i );
    if( !h ) continue;
    h->Draw();
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________

TCanvas*
TOF_TDCD1()
{
  auto c = new TCanvas(__func__, __func__);
  c->Divide(4, 4);

  static const Int_t seg_per_canvas = 16;

  // draw TDC U0-15
  int TOF_base_id = HistMaker::getUniqueID( kTOF, 0, kTDC,NumOfSegTOF +  0);

  for( int i=0; i<seg_per_canvas; ++i ){
    c->cd(i+1)->SetLogy();
    TH1 *h = (TH1*)GHist::get( TOF_base_id + i );
    if( !h ) continue;
    h->Draw();
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TOF_TDCD2()
{
  auto c = new TCanvas(__func__, __func__);
  c->Divide(4, 4);

  static const Int_t seg_per_canvas = 16;

  // draw TDC U16-31
  int TOF_base_id = HistMaker::getUniqueID( kTOF, 0, kTDC, NumOfSegTOF + 16);

  for( int i=0; i<seg_per_canvas; ++i ){
    c->cd(i+1)->SetLogy();
    TH1 *h = (TH1*)GHist::get( TOF_base_id + i );
    if( !h ) continue;
    h->Draw();
  }
  c->Update();
  return c;
}

//_____________________________________________________________________________
TCanvas*
TOF_TDCD3()
{
  auto c = new TCanvas(__func__, __func__);
  c->Divide(4, 4);

  static const Int_t seg_per_canvas = 16;

  // draw TDC U32-48
  int TOF_base_id = HistMaker::getUniqueID( kTOF, 0, kTDC, NumOfSegTOF + 32);

  for( int i=0; i<seg_per_canvas; ++i ){
    c->cd(i+1)->SetLogy();
    TH1 *h = (TH1*)GHist::get( TOF_base_id + i );
    if( !h ) continue;
    h->Draw();
  }
  c->Update();
  return c;
}







//tdc
//_____________________________________________________________________________
TCanvas*
DAQ()
{
  std::vector<Int_t> hist_id = {
    HistMaker::getUniqueID(kDAQ, kEB, kHitPat),
    HistMaker::getUniqueID(kDAQ, kVME, kHitPat2D),
    // HistMaker::getUniqueID(kDAQ, kEASIROC, kHitPat2D),
    HistMaker::getUniqueID(kDAQ, kHUL, kHitPat2D),
    HistMaker::getUniqueID(kDAQ, kVEASIROC, kHitPat2D),
    // HistMaker::getUniqueID(kDAQ,  kCLite,   kHitPat2D),
    // HistMaker::getUniqueID(kDAQ,  kOpt,     kHitPat2D),
  };
  auto c1 = new TCanvas(__func__, __func__);
  c1->Divide(1, 2);
  c1->cd(1)->Divide(3, 1);
  for(Int_t i=0, n=hist_id.size(); i<n; ++i){
    if(i != 3){
      c1->cd(1)->cd(i + 1); //->SetGrid();
    }else{
      c1->cd(2); //->SetGrid();
    }
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

  int base_id = HistMaker::getUniqueID(kCFT, 0, kTDC, 1);

  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(base_id+l);
    if(!h) continue;
    h->Draw();
  }
  return c1;
}


//_____________________________________________________________________________
TCanvas*
CFTTDC2D()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);

  int base_id = HistMaker::getUniqueID(kCFT, 0, kTDC2D, 1);

  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(base_id+l);
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

  int base_id = HistMaker::getUniqueID(kCFT, 0, kADC, 1);
  int base_id_ct = HistMaker::getUniqueID(kCFT, 0, kADC, 11);

  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(base_id+l);
    if(!h) continue;
    h->Draw();
    TH1 *hh = GHist::get(base_id_ct+l);
    if(!hh) continue;
    hh->SetLineColor(kRed);
    hh->Draw("same");
  }
  return c1;
}



//_____________________________________________________________________________
TCanvas*
CFTTOT2D()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);

  int base_id = HistMaker::getUniqueID(kCFT, 0, kADC2D, 1);

  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(base_id+l);
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTCTOT2D()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);

  int base_id = HistMaker::getUniqueID(kCFT, 0, kADC2D, 11);

  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(base_id+l);
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

  int base_id = HistMaker::getUniqueID(kCFT, 0, kHighGain, 1);
  int base_id_ch = HistMaker::getUniqueID(kCFT, 0, kHighGain, 21);

  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(base_id+l);
    if(!h) continue;
    h->Draw();
    TH1 *hh = GHist::get(base_id_ch+l);
    if(!hh) continue;
    hh->SetLineColor(kRed);
    hh->Draw("same");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTLowGain()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);

  int base_id = HistMaker::getUniqueID(kCFT, 0, kLowGain, 1);
  int base_id_cl = HistMaker::getUniqueID(kCFT, 0, kLowGain, 21);

  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(base_id+l);
    if(!h) continue;
    h->Draw();
    TH1 *hh = GHist::get(base_id_cl+l);
    if(!hh) continue;
    hh->SetLineColor(kRed);
    hh->Draw("same");
  }

  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTPedestal()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);

  int base_id = HistMaker::getUniqueID(kCFT, 0, kPede, 1);

  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(base_id+l);
    if(!h) continue;
    h->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTHighGain2D()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);

  int base_id = HistMaker::getUniqueID(kCFT, 0, kHighGain, 11);

  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(base_id+l);
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

  int base_id = HistMaker::getUniqueID(kCFT, 0, kLowGain, 11);

  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(base_id+l);
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

  int base_id = HistMaker::getUniqueID(kCFT, 0, kPede, 11);

  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(base_id);
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTCHighGain2D()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);

  int base_id = HistMaker::getUniqueID(kCFT, 0, kHighGain, 31);

  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(base_id+l);
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTCLowGain2D()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);

  int base_id = HistMaker::getUniqueID(kCFT, 0, kLowGain, 31);

  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(base_id+l);
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTHighGainxTOT()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);

  int base_id = HistMaker::getUniqueID(kCFT, 0, kHighGain, 41);

  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(base_id+l);
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTLowGainxTOT()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);

  int base_id = HistMaker::getUniqueID(kCFT, 0, kLowGain, 41);

  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h = GHist::get(base_id+l);
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

  int base_id = HistMaker::getUniqueID(kCFT, 0, kHitPat, 1);
  int base_id_ct = HistMaker::getUniqueID(kCFT, 0, kHitPat, 11);
  int base_id_ctwb = HistMaker::getUniqueID(kCFT, 0, kHitPat, 21);

  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h1 = GHist::get(base_id+l);
    if(!h1) continue;
    h1->Draw();
    TH1 *h2 = GHist::get(base_id_ct+l);
    if(!h2) continue;
    h2->SetLineColor(kRed);
    h2->Draw("same");
    TH1 *h3 = GHist::get(base_id_ctwb+l);
    if(!h3) continue;
    h3->SetLineColor(kGreen);
    h3->Draw("same");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTMulti()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);

  int base_id = HistMaker::getUniqueID(kCFT, 0, kMulti, 1);
  int base_id_ct = HistMaker::getUniqueID(kCFT, 0, kMulti, 11);

  for(Int_t l=0; l<NumOfLayersCFT; ++l){
    c1->cd(l+1);
    TH1 *h1 = GHist::get(base_id+l);
    if(!h1) continue;
    h1->Draw();
    TH1 *h2 = GHist::get(base_id_ct+l);
    if(!h2) continue;
    h2->SetLineColor(kRed);
    h2->Draw("same");
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

  int base_id = HistMaker::getUniqueID(kBGO, 0, kFADC, 1);

  for(Int_t i=0; i<NumOfSegBGO; ++i){
    c1->cd(i+1);
    TH1 *h = GHist::get(base_id+i);
    if(!h) continue;
    h->GetXaxis()->SetRangeUser(80, 200);
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BGOFADCwTDC()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(6, 4);

  int base_id = HistMaker::getUniqueID(kBGO, 0, kFADCwTDC, 1);

  for(Int_t i=0; i<NumOfSegBGO; ++i){
    c1->cd(i+1);
    TH1 *h = GHist::get(base_id+i);
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

  int base_id = HistMaker::getUniqueID(kBGO, 0, kADC, 1);
  int base_id_wt = HistMaker::getUniqueID(kBGO, 0, kADCwTDC, 1);

  for(Int_t i=0; i<NumOfSegBGO; ++i){
    c1->cd(i+1);
    TH1 *h = GHist::get(base_id+i);
    if(!h) continue;
    h->Draw();
    TH1 *hh = GHist::get(base_id_wt+i);
    if(!hh) continue;
    hh->SetLineColor(kRed+1);
    hh->Draw("same");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BGOTDC()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(6, 4);
  TH1 *h;
  for(Int_t i=0; i<NumOfSegBGO; ++i){
    c1->cd(i+1);
    h = GHist::get(HistMaker::getUniqueID(kBGO, 0, kTDC, i+1));
    if(!h) continue;
    h->Draw();
  }
  return c1;
}

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

  Int_t base_id = HistMaker::getUniqueID(kBGO, 0, kTDC, 25);
  Int_t base_id_h = HistMaker::getUniqueID(kBGO, 0, kHitPat, 1);
  Int_t base_id_m = HistMaker::getUniqueID(kBGO, 0, kMulti, 1);

  for(Int_t i=0; i<NumOfSegBGO_T; ++i){
    c1->cd(i+1);
    TH1 *h1 = (TH1*)GHist::get(base_id + i);
    if(!h1) continue;
    h1->Draw();
  }

  c1->cd(5);
  TH1 *h2 = (TH1*)GHist::get(base_id_h);
  h2->Draw();
  TH1 *hh2 = (TH1*)GHist::get(base_id_h+1);
  hh2->SetLineColor(kRed);
  hh2->Draw("same");
  c1->cd(7);
  TH1 *h3 = (TH1*)GHist::get(base_id_m);
  h3->Draw();
  TH1 *hh3 = (TH1*)GHist::get(base_id_m+1);
  hh3->SetLineColor(kRed);
  hh3->Draw("same");

  return c1;
}


//_____________________________________________________________________________
TCanvas*
PiIDHighGain()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(8, 4);

  int base_id = HistMaker::getUniqueID(kPiID, 0, kHighGain, 1);
  int base_id_wt = HistMaker::getUniqueID(kPiID, 0, kADCwTDC, 1);

  TH1 *h;
  for(Int_t i=0; i<NumOfSegPiID; ++i){
    c1->cd(i+1);
    h = GHist::get(base_id+i);
    if(!h) continue;
    h->Draw();
    h = GHist::get(base_id_wt);
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

  int base_id = HistMaker::getUniqueID(kPiID, 0, kLowGain, 1);
  int base_id_wt = HistMaker::getUniqueID(kPiID, 0, kADCwTDC, NumOfSegPiID+1);

  TH1 *h;
  for(Int_t i=0; i<NumOfSegPiID; ++i){
    c1->cd(i+1);
    h = GHist::get(base_id+i);
    if(!h) continue;
    h->Draw();
    h = GHist::get(base_id_wt+i);
    if(!h) continue;
    h->SetLineColor(kRed+1);
    h->Draw("same");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
PiIDTDC()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(8, 4);

  int base_id = HistMaker::getUniqueID(kPiID, 0, kTDC, 1);

  TH1 *h;
  for(Int_t i=0; i<NumOfSegPiID; ++i){
    c1->cd(i+1);
    h = GHist::get(base_id+i);
    if(!h) continue;
    h->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
PiIDADC2DHitMulti()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(2, 2);

  int base_id = HistMaker::getUniqueID(kPiID, 0, kADC2D, 1);
  int base_id_h = HistMaker::getUniqueID(kPiID, 0, kHitPat, 1);
  int base_id_m = HistMaker::getUniqueID(kPiID, 0, kMulti,  1);

  TH1 *h;
  for(Int_t i=0; i<2; ++i){
    c1->cd(i+1);
    h = GHist::get(base_id+i*10);
    if(!h) continue;
    h->Draw();
  }

  c1->cd(3);
  for(Int_t i=0; i<2; ++i){
    h = GHist::get(base_id_h+i);
    if(!h) continue;
    if(i==0) h->Draw();
    if(i==1){
      h->SetLineColor(kRed);
      h->Draw("same");
    }
  }
  c1->cd(4);
  for(Int_t i=0; i<2; ++i){
    h = GHist::get(base_id_m+i);
    if(!h) continue;
    if(i==0) h->Draw();
    if(i==1){
      h->SetLineColor(kRed);
      h->Draw("same");
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
Correlation_CATCH()
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(2, 2);

  int base_id = HistMaker::getUniqueID(kCorrelation_catch, 0, 0, 1);

  TH1 *h;
  for(Int_t i=0; i<4; ++i){
    c1->cd(i+1);
    h = GHist::get(base_id+i);
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
