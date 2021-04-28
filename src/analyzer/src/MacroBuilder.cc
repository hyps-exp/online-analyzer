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
SetText( TLatex* text, Int_t align, Double_t size, Int_t ndc=1 )
{
  text->SetNDC( ndc );
  text->SetTextAlign( align );
  text->SetTextSize( size );
}
}

namespace analyzer
{

namespace macro
{

//_____________________________________________________________________________
TObject*
Get( TString name )
{
  std::string process = Main::getInstance().getArgv().at(0);
  Int_t n = process.find("bin");
  TString path = process.substr(0, n)+"src/analyzer/macro/";
  if( name.Contains(".C") )
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
Correlation( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 3, 2 );
  for( Int_t i=0; i<6; ++i ){
    c1->cd( i+1 );
    auto h = GHist::get( HistMaker::getUniqueID( kCorrelation, 0, 0, i+1 ) );
    if( !h ) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CorrelationFHT( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(2, 2);
  for( Int_t i=0; i<4; ++i ){
    c1->cd(i+1);
    TH1 *h = GHist::get( HistMaker::getUniqueID(kCorrelation, 0, 0, i+7) );
    if( !h ) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
HitPattern( void )
{
  return HitPatternE03();
  // return HitPatternE40();
}

//_____________________________________________________________________________
TCanvas*
BH1ADC( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 6, 4 );
  for( Int_t i=0; i<NumOfSegBH1; ++i ){
    for( Int_t j=0; j<kUorD; ++j ){
      c1->cd( i+j*(NumOfSegBH1+1)+1 )->SetLogy();
      auto h1id = HistMaker::getUniqueID( kBH1, 0, kADC, i+j*NumOfSegBH1+1 );
      auto h1 = GHist::get( h1id );
      if( !h1 ) continue;
      h1->GetXaxis()->SetRangeUser( 0, 2000 );
      h1->Draw();
      auto h2id = HistMaker::getUniqueID( kBH1, 0, kADCwTDC, i+j*NumOfSegBH1+1 );
      auto h2 = GHist::get( h2id );
      if( !h2 ) continue;
      h2->GetXaxis()->SetRangeUser( 0, 2000 );
      h2->SetLineColor( kRed+1 );
      h2->Draw( "same" );
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BH1TDC( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 6, 4 );
  for( Int_t i=0; i<NumOfSegBH1; ++i ){
    for( Int_t j=0; j<kUorD; ++j ){
      c1->cd( i+j*(NumOfSegBH1+1)+1 ); //->SetLogy();
      auto h1id = HistMaker::getUniqueID( kBH1, 0, kTDC, i+j*NumOfSegBH1+1 );
      auto h1 = GHist::get( h1id );
      if( !h1 ) continue;
      auto xmin = gUser.GetParameter( "BH1_TDC", 0 );
      auto xmax = gUser.GetParameter( "BH1_TDC", 1 );
      h1->GetXaxis()->SetRangeUser( xmin, xmax );
      h1->Draw();
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BFT( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 1, 2 );
  {
    c1->cd( 1 )->Divide( 4, 2 );
    std::vector<Int_t> id = {
      HistMaker::getUniqueID( kBFT, 0, kTDC, 11 ),
      HistMaker::getUniqueID( kBFT, 0, kADC, 11 ),
      HistMaker::getUniqueID( kBFT, 0, kHitPat, 11 ),
      HistMaker::getUniqueID( kBFT, 0, kMulti, 11 )
    };
    for( Int_t i=0, n=id.size(); i<n; ++i ){
      for( Int_t j=0; j<kUorD; ++j ){
	c1->cd( 1 )->cd( i+j*n+1 );
	if( i == n-1 && j == kD ) continue;
	auto h1 = GHist::get( id[i]+j );
	if( !h1 ) continue;
	h1->Draw();
      }
    }
  }
  {
    c1->cd( 2 )->Divide( 2, 2 );
    std::vector<Int_t> id = {
      HistMaker::getUniqueID( kBFT, 0, kTDC2D ),
      HistMaker::getUniqueID( kBFT, 0, kADC2D )
    };
    for( Int_t i=0, n=id.size(); i<n; ++i ){
      for( Int_t j=0; j<kUorD; ++j ){
	c1->cd( 2 )->cd( i+j*n+1 );
	auto h1 = GHist::get( id[i]+j );
	if( !h1 ) continue;
	h1->Draw( "colz" );
      }
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BAC( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 2, 1 );

  std::vector<Int_t> adc_id = {
    HistMaker::getUniqueID(kBAC,  0, kADC, 1)
  };

  std::vector<Int_t> awt_id = {
    HistMaker::getUniqueID(kBAC,  0, kADCwTDC, 1)
  };

  std::vector<Int_t> tdc_id = {
    HistMaker::getUniqueID(kBAC,  0, kTDC, 1)
  };

  for( Int_t i=0, n=adc_id.size(); i<n; ++i ){
    c1->cd(i+1)->SetLogy();
    TH1 *h = GHist::get( adc_id[i] );
    if( !h ) continue;
    h->Draw();
    TH1 *hh = GHist::get( awt_id[i] );
    if( !hh ) continue;
    hh->SetLineColor(kRed+1);
    hh->Draw("same");
    c1->cd(i+1+n)->SetLogy();
    TH1 *hhh = GHist::get( tdc_id[i] );
    if( !hhh ) continue;
    hhh->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BH2ADC( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 4, 4 );
  for( Int_t i=0; i<NumOfSegBH2; ++i ){
    for( Int_t j=0; j<kUorD; ++j ){
      c1->cd(i+j*NumOfSegBH2+1)->SetLogy();
      auto h1id = HistMaker::getUniqueID(kBH2, 0, kADC, i+j*NumOfSegBH2+1);
      auto h1 = GHist::get(h1id);
      if( !h1 ) continue;
      h1->GetXaxis()->SetRangeUser(0, 2000);
      h1->Draw();
      auto h2id = HistMaker::getUniqueID(kBH2, 0, kADCwTDC, i+j*NumOfSegBH2+1);
      auto h2 = GHist::get(h2id);
      if(!h2) continue;
      h2->GetXaxis()->SetRangeUser(0, 2000);
      h2->SetLineColor(kRed+1);
      h2->Draw("same");
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BH2TDC( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 4, 4 );
  for( Int_t i=0; i<NumOfSegBH2; ++i ){
    for( Int_t j=0; j<kUorD; ++j ){
      c1->cd( i+j*NumOfSegBH2+1 ); //->SetLogy();
      auto h1id = HistMaker::getUniqueID( kBH2, 0, kTDC, i+j*NumOfSegBH2+1 );
      auto h1 = GHist::get( h1id );
      if( !h1 ) continue;
      auto xmin = gUser.GetParameter( "BH2_TDC", 0 );
      auto xmax = gUser.GetParameter( "BH2_TDC", 1 );
      h1->GetXaxis()->SetRangeUser( xmin, xmax );
      h1->Draw();
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
T0( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide(4, 4);
  for( Int_t i=0; i<NumOfSegBH2; ++i ){
    for( Int_t j=0; j<kUorD; ++j ){
      c1->cd( i+j*NumOfSegBH2+1 ); //->SetLogy();
      auto h1id = HistMaker::getUniqueID(kBH2, 0, kTDC, i+j*NumOfSegBH2+1);
      auto h1 = GHist::get(h1id);
      if(!h1) continue;
      auto xmin = gUser.GetParameter("BH2_TDC", 0);
      auto xmax = gUser.GetParameter("BH2_TDC", 1);
      h1->GetXaxis()->SetRangeUser(xmin, xmax);
      h1->Draw();
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
ACs( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 4, 2 );

  std::vector<Int_t> adc_id = {
    HistMaker::getUniqueID(kBAC,  0, kADC, 1),
    HistMaker::getUniqueID(kBAC,  0, kADC, 2),
    HistMaker::getUniqueID(kPVAC, 0, kADC, 1),
    HistMaker::getUniqueID(kFAC,  0, kADC, 1)
  };

  std::vector<Int_t> awt_id = {
    HistMaker::getUniqueID(kBAC,  0, kADCwTDC, 1),
    HistMaker::getUniqueID(kBAC,  0, kADCwTDC, 2),
    HistMaker::getUniqueID(kPVAC, 0, kADCwTDC, 1),
    HistMaker::getUniqueID(kFAC,  0, kADCwTDC, 1)
  };

  std::vector<Int_t> tdc_id = {
    HistMaker::getUniqueID(kBAC,  0, kTDC, 1),
    HistMaker::getUniqueID(kBAC,  0, kTDC, 2),
    HistMaker::getUniqueID(kPVAC, 0, kTDC, 1),
    HistMaker::getUniqueID(kFAC,  0, kTDC, 1)
  };

  for( Int_t i=0, n=adc_id.size(); i<n; ++i ){
    c1->cd(i+1)->SetLogy();
    TH1 *h = GHist::get( adc_id[i] );
    if( !h ) continue;
    h->Draw();
    TH1 *hh = GHist::get( awt_id[i] );
    if( !hh ) continue;
    hh->SetLineColor(kRed+1);
    hh->Draw("same");
    c1->cd(i+1+n)->SetLogy();
    TH1 *hhh = GHist::get( tdc_id[i] );
    if( !hhh ) continue;
    hhh->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
HTOFADCU(void)
{
  auto c1 = new TCanvas(__func__, __func__);
  c1->Divide(6, 6);
  for(Int_t i=0; i<NumOfSegHTOF; ++i){
    c1->cd(i+1)->SetLogy();
    auto h1id = HistMaker::getUniqueID(kHTOF, 0, kADC, i+1);
    auto h1 = GHist::get(h1id);
    if(!h1) continue;
    // h1->GetXaxis()->SetRangeUser(0, 2000);
    h1->Draw();
    auto h2id = HistMaker::getUniqueID(kHTOF, 0, kADCwTDC, i+1);
    auto h2 = GHist::get(h2id);
    if(!h2) continue;
    h2->SetLineColor(kRed+1);
    h2->Draw("same");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
HTOFADCD(void)
{
  auto c1 = new TCanvas(__func__, __func__);
  c1->Divide(6, 6);
  for(Int_t i=0; i<NumOfSegHTOF; ++i){
    c1->cd(i+1)->SetLogy();
    auto h1id = HistMaker::getUniqueID(kHTOF, 0, kADC, i+NumOfSegHTOF+1);
    auto h1 = GHist::get(h1id);
    if(!h1) continue;
    // h1->GetXaxis()->SetRangeUser(0, 2000);
    h1->Draw();
    auto h2id = HistMaker::getUniqueID(kHTOF, 0, kADCwTDC, i+NumOfSegHTOF+1);
    auto h2 = GHist::get(h2id);
    if(!h2) continue;
    h2->SetLineColor(kRed+1);
    h2->Draw("same");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
HTOFTDCU( void )
{
  auto c1 = new TCanvas(__func__, __func__);
  c1->Divide(6, 6);
  for(Int_t i=0; i<NumOfSegHTOF; ++i){
    c1->cd(i+1);
    auto h1id = HistMaker::getUniqueID(kHTOF, 0, kTDC, i+1);
    auto h1 = GHist::get(h1id);
    if(!h1) continue;
    auto xmin = gUser.GetParameter("HTOF_TDC", 0);
    auto xmax = gUser.GetParameter("HTOF_TDC", 1);
    h1->GetXaxis()->SetRangeUser(xmin, xmax);
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
HTOFTDCD( void )
{
  auto c1 = new TCanvas(__func__, __func__);
  c1->Divide(6, 6);
  for(Int_t i=0; i<NumOfSegHTOF; ++i){
    c1->cd(i+1);
    auto h1id = HistMaker::getUniqueID(kHTOF, 0, kTDC, i+NumOfSegHTOF+1);
    auto h1 = GHist::get(h1id);
    if(!h1) continue;
    auto xmin = gUser.GetParameter("HTOF_TDC", 0);
    auto xmax = gUser.GetParameter("HTOF_TDC", 1);
    h1->GetXaxis()->SetRangeUser(xmin, xmax);
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BeamHitMulti(void)
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  std::vector<Int_t> id = {
    HistMaker::getUniqueID(kBH1, 0, kHitPat),
    HistMaker::getUniqueID(kBAC, 0, kHitPat),
    HistMaker::getUniqueID(kBH2, 0, kHitPat),
    HistMaker::getUniqueID(kHTOF, 0, kHitPat),
    HistMaker::getUniqueID(kBH1, 0, kMulti),
    HistMaker::getUniqueID(kBAC, 0, kMulti),
    HistMaker::getUniqueID(kBH2, 0, kMulti),
    HistMaker::getUniqueID(kHTOF, 0, kMulti),
  };
  c1->Divide(4, 2);
  for(Int_t i=0, n=id.size(); i<n; ++i){
    c1->cd(i+1);
    auto h = GHist::get(id.at(i));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
FBHTDC( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(8,4);
  for( Int_t i=0; i<NumOfSegFBH*2; ++i ){
    c1->cd(i+1);
    TH1 *h = GHist::get( HistMaker::getUniqueID(kFBH, 0, kTDC, i+1));
    if( !h ) return 0;
    h->Draw();
  }

  return c1;
}

//_____________________________________________________________________________
TCanvas*
FBHTOT( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(8,4);
  for( Int_t i=0; i<NumOfSegFBH*2; ++i ){
    c1->cd(i+1);
    TH1 *h = GHist::get( HistMaker::getUniqueID(kFBH, 0, kADC, i+1));
    if( !h ) return 0;
    h->Draw();
  }

  return c1;
}

//_____________________________________________________________________________
TCanvas*
FBHHitMulti( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);

  std::vector<Int_t> id = {
    HistMaker::getUniqueID(kFBH, 0, kTDC, NumOfSegFBH*2+1),
    HistMaker::getUniqueID(kFBH, 0, kADC, NumOfSegFBH*2+1),
    HistMaker::getUniqueID(kFBH, 0, kHitPat, 1),
    HistMaker::getUniqueID(kFBH, 0, kTDC2D, 1),
    HistMaker::getUniqueID(kFBH, 0, kADC2D, 1),
    HistMaker::getUniqueID(kFBH, 0, kMulti, 1)
  };

  c1->Divide(3,2);
  for( Int_t i=0; i<6; ++i ){
    c1->cd(i+1);
    TH1 *h = GHist::get( id[i] );
    if( !h ) return 0;
    h->Draw("colz");
  }

  return c1;
}

//_____________________________________________________________________________
TCanvas*
SFTTDCTOT( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(1,2);

  {
    c1->cd(1)->Divide(4,2);
    std::vector<Int_t> id = {
      HistMaker::getUniqueID(kSFT, 0, kTDC, 1),
      HistMaker::getUniqueID(kSFT, 0, kTDC, 2),
      HistMaker::getUniqueID(kSFT, 0, kTDC, 3),
      HistMaker::getUniqueID(kSFT, 0, kTDC, 4),
      HistMaker::getUniqueID(kSFT, 0, kTDC, 11),
      HistMaker::getUniqueID(kSFT, 0, kTDC, 12),
      HistMaker::getUniqueID(kSFT, 0, kTDC, 13),
      HistMaker::getUniqueID(kSFT, 0, kTDC, 14)
    };
    for( Int_t i=0, n=id.size(); i<n; ++i ){
      c1->cd(1)->cd(i+1);
      TH1 *h = GHist::get(id[i]);
      if( !h ) continue;
      h->Draw();
    }
  }

  {
    c1->cd(2)->Divide(4,2);
    std::vector<Int_t> id = {
      HistMaker::getUniqueID(kSFT, 0, kADC, 1),
      HistMaker::getUniqueID(kSFT, 0, kADC, 2),
      HistMaker::getUniqueID(kSFT, 0, kADC, 3),
      HistMaker::getUniqueID(kSFT, 0, kADC, 4),
      HistMaker::getUniqueID(kSFT, 0, kADC, 11),
      HistMaker::getUniqueID(kSFT, 0, kADC, 12),
      HistMaker::getUniqueID(kSFT, 0, kADC, 13),
      HistMaker::getUniqueID(kSFT, 0, kADC, 14)
    };

    for( Int_t i=0, n=id.size(); i<n; ++i ){
      c1->cd(2)->cd(i+1);
      TH1 *h = GHist::get(id[i]);
      if( !h ) continue;
      h->Draw();
    }
  }

  return c1;
}

//_____________________________________________________________________________
TCanvas*
SFTHitMulti( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(1,2);

  {
    c1->cd(1)->Divide(4,2);
    std::vector<Int_t> id = {
      HistMaker::getUniqueID(kSFT, 0, kHitPat, 1),
      HistMaker::getUniqueID(kSFT, 0, kHitPat, 2),
      HistMaker::getUniqueID(kSFT, 0, kHitPat, 3),
      HistMaker::getUniqueID(kSFT, 0, kHitPat, 4),
      HistMaker::getUniqueID(kSFT, 0, kHitPat, 11),
      HistMaker::getUniqueID(kSFT, 0, kHitPat, 12),
      HistMaker::getUniqueID(kSFT, 0, kHitPat, 13),
      HistMaker::getUniqueID(kSFT, 0, kHitPat, 14)
    };
    for( Int_t i=0, n=id.size(); i<n; ++i ){
      c1->cd(1)->cd(i+1);
      TH1 *h = GHist::get(id[i]);
      if( !h ) continue;
      h->Draw();
    }
  }

  {
    c1->cd(2)->Divide(3,2);
    std::vector<Int_t> id = {
      HistMaker::getUniqueID(kSFT, 0, kMulti, 1),
      HistMaker::getUniqueID(kSFT, 0, kMulti, 2),
      HistMaker::getUniqueID(kSFT, 0, kMulti, 3),
      HistMaker::getUniqueID(kSFT, 0, kMulti, 11),
      HistMaker::getUniqueID(kSFT, 0, kMulti, 12),
      HistMaker::getUniqueID(kSFT, 0, kMulti, 13)
    };

    for( Int_t i=0, n=id.size(); i<n; ++i ){
      c1->cd(2)->cd(i+1);
      TH1 *h = GHist::get(id[i]);
      if( !h ) continue;
      h->Draw();
    }
  }

  return c1;
}

//_____________________________________________________________________________
TCanvas*
SFT2D( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(1,1);

  {
    c1->cd(1)->Divide(4,2);
    std::vector<Int_t> id = {
      HistMaker::getUniqueID(kSFT, 0, kTDC2D, 1),
      HistMaker::getUniqueID(kSFT, 0, kTDC2D, 2),
      HistMaker::getUniqueID(kSFT, 0, kTDC2D, 3),
      HistMaker::getUniqueID(kSFT, 0, kTDC2D, 4),
      HistMaker::getUniqueID(kSFT, 0, kADC2D, 1),
      HistMaker::getUniqueID(kSFT, 0, kADC2D, 2),
      HistMaker::getUniqueID(kSFT, 0, kADC2D, 3),
      HistMaker::getUniqueID(kSFT, 0, kADC2D, 4)
    };
    for( Int_t i=0, n=id.size(); i<n; ++i ){
      c1->cd(1)->cd(i+1);
      TH1 *h = GHist::get(id[i]);
      if( !h ) continue;
      h->Draw("colz");
    }
  }

  return c1;
}

//_____________________________________________________________________________
TCanvas*
SCHTDCU( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 8, 4 );
  for( Int_t i=0; i<NumOfSegSCH/2; ++i ){
    c1->cd( i+1 );
    auto h = GHist::get( HistMaker::getUniqueID( kSCH, 0, kTDC, i+1 ) );
    if( !h ) continue;
    h->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SCHTDCD( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 8, 4 );
  for( Int_t i=0; i<NumOfSegSCH/2; ++i ){
    c1->cd( i+1 );
    auto h = GHist::get( HistMaker::getUniqueID( kSCH, 0, kTDC, i+1+NumOfSegSCH/2 ) );
    if( !h ) continue;
    h->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SCHTOTU( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 8, 4 );
  for( Int_t i=0; i<NumOfSegSCH/2; ++i ){
    c1->cd( i+1 );
    auto h = GHist::get( HistMaker::getUniqueID( kSCH, 0, kADC, i+1 ) );
    if( !h ) continue;
    h->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SCHTOTD( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 8, 4 );
  for( Int_t i=0; i<NumOfSegSCH/2; ++i ){
    c1->cd( i+1 );
    auto h = GHist::get( HistMaker::getUniqueID( kSCH, 0, kADC, i+1+NumOfSegSCH/2 ) );
    if( !h ) continue;
    h->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SCHHitMulti( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  std::vector<Int_t> id = {
    HistMaker::getUniqueID( kSCH, 0, kTDC, NumOfSegSCH+1 ),
    HistMaker::getUniqueID( kSCH, 0, kADC, NumOfSegSCH+1 ),
    HistMaker::getUniqueID( kSCH, 0, kHitPat, 1 ),
    HistMaker::getUniqueID( kSCH, 0, kTDC2D, 1 ),
    HistMaker::getUniqueID( kSCH, 0, kADC2D, 1 ),
    HistMaker::getUniqueID( kSCH, 0, kMulti, 1 )
  };
  c1->Divide( 3, 2 );
  for( Int_t i=0, n=id.size(); i<n; ++i ){
    c1->cd( i+1 );
    auto h = GHist::get( id[i] );
    if( !h ) continue;
    h->Draw( "colz" );
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
TOFADCU( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 6, 4 );
  for( Int_t i=0; i<NumOfSegTOF; ++i ){
    c1->cd( i+1 )->SetLogy();
    auto h1 = GHist::get( HistMaker::getUniqueID( kTOF, 0, kADC, i+1 ) );
    if( !h1 ) continue;
    h1->GetXaxis()->SetRangeUser( 0, 4000 );
    h1->Draw();
    auto h2 = GHist::get( HistMaker::getUniqueID( kTOF, 0, kADCwTDC, i+1 ) );
    if( !h2 ) continue;
    h2->GetXaxis()->SetRangeUser( 0, 4000 );
    h2->SetLineColor( kRed+1 );
    h2->Draw( "same" );
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
TOFADCD( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 6, 4 );
  for( Int_t i=0; i<NumOfSegTOF; ++i ){
    c1->cd( i+1 )->SetLogy();
    auto h1 = GHist::get( HistMaker::getUniqueID( kTOF, 0, kADC, i+NumOfSegTOF+1 ) );
    if( !h1 ) continue;
    h1->GetXaxis()->SetRangeUser( 0, 4000 );
    h1->Draw();
    auto h2 = GHist::get( HistMaker::getUniqueID( kTOF, 0, kADCwTDC, i+NumOfSegTOF+1 ) );
    if( !h2 ) continue;
    h2->GetXaxis()->SetRangeUser( 0, 4000 );
    h2->SetLineColor( kRed+1 );
    h2->Draw( "same" );
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
TOFTDCU( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 6, 4 );
  for( Int_t i=0; i<NumOfSegTOF; ++i ){
    c1->cd( i+1 ); //->SetLogy();
    auto h1 = GHist::get( HistMaker::getUniqueID( kTOF, 0, kTDC, i+1 ) );
    if( !h1 ) continue;
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
TOFTDCD( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 6, 4 );
  for( Int_t i=0; i<NumOfSegTOF; ++i ){
    c1->cd( i+1 ); //->SetLogy();
    auto h1 = GHist::get( HistMaker::getUniqueID( kTOF, 0, kTDC, i+NumOfSegTOF+1 ) );
    if( !h1 ) continue;
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
TOF_HT( void )
{
  Int_t id = HistMaker::getUniqueID(kTOF_HT, 0, kTDC, 1);

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(5,3);
  for( Int_t i=0; i<NumOfSegHtTOF; ++i ){
    c1->cd(i+1);
    TH1 *h = GHist::get(id+i);
    if( !h ) continue;
    h->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BVHTDC( void )
{
  auto c1 = new TCanvas(__func__, __func__);
  auto base_id = HistMaker::getUniqueID(kBVH, 0, kTDC);
  c1->Divide(2, 2);
  for(Int_t i=0; i<NumOfSegBVH; ++i){
    c1->cd(i+1);
    auto h1 = GHist::get(base_id+i);
    if(!h1) continue;
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
LACTDC( void )
{
  auto base_id = HistMaker::getUniqueID( kLAC, 0, kTDC );
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 6, 5 );
  for( Int_t i=0; i<NumOfSegLAC; ++i ){
    c1->cd( i+1 );
    auto h1 = GHist::get( base_id+i );
    if( !h1 ) continue;
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
WCADCU( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 5, 4 );
  for( Int_t i=0; i<NumOfSegWC; ++i ){
    c1->cd( i+1 )->SetLogy();
    auto h1 = GHist::get( HistMaker::getUniqueID( kWC, 0, kADC, i+1 ) );
    if( !h1 ) continue;
    h1->GetXaxis()->SetRangeUser( 0, 4000 );
    h1->Draw();
    auto h2 = GHist::get( HistMaker::getUniqueID( kWC, 0, kADCwTDC, i+1 ) );
    if( !h2 ) continue;
    h2->GetXaxis()->SetRangeUser( 0, 4000 );
    h2->SetLineColor( kRed+1 );
    h2->Draw( "same" );
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
WCADCD( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 5, 4 );
  for( Int_t i=0; i<NumOfSegWC; ++i ){
    c1->cd( i+1 )->SetLogy();
    auto h1 = GHist::get( HistMaker::getUniqueID( kWC, 0, kADC, i+1+NumOfSegWC ) );
    if( !h1 ) continue;
    h1->GetXaxis()->SetRangeUser( 0, 4000 );
    h1->Draw();
    auto h2 = GHist::get( HistMaker::getUniqueID( kWC, 0, kADCwTDC, i+1+NumOfSegWC ) );
    if( !h2 ) continue;
    h2->GetXaxis()->SetRangeUser( 0, 4000 );
    h2->SetLineColor( kRed+1 );
    h2->Draw( "same" );
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
WCADCSUM( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 5, 4 );
  for( Int_t i=0; i<NumOfSegWC; ++i ){
    c1->cd( i+1 )->SetLogy();
    auto h1 = GHist::get( HistMaker::getUniqueID( kWC, 0, kADC, i+1+NumOfSegWC*2 ) );
    if( !h1 ) continue;
    h1->GetXaxis()->SetRangeUser( 0, 4000 );
    h1->Draw();
    auto h2 = GHist::get( HistMaker::getUniqueID( kWC, 0, kADCwTDC, i+1+NumOfSegWC*2 ) );
    if( !h2 ) continue;
    h2->GetXaxis()->SetRangeUser( 0, 4000 );
    h2->SetLineColor( kRed+1 );
    h2->Draw( "same" );
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
WCTDCU( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 5, 4 );
  for( Int_t i=0; i<NumOfSegWC; ++i ){
    c1->cd( i+1 ); //->SetLogy();
    auto h1 = GHist::get( HistMaker::getUniqueID( kWC, 0, kTDC, i+1 ) );
    if( !h1 ) continue;
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
WCTDCD( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 5, 4 );
  for( Int_t i=0; i<NumOfSegWC; ++i ){
    c1->cd( i+1 ); //->SetLogy();
    auto h1 = GHist::get( HistMaker::getUniqueID( kWC, 0, kTDC, i+1+NumOfSegWC ) );
    if( !h1 ) continue;
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
WCTDCSUM( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 5, 4 );
  for( Int_t i=0; i<NumOfSegWC; ++i ){
    c1->cd( i+1 ); //->SetLogy();
    auto h1 = GHist::get( HistMaker::getUniqueID( kWC, 0, kTDC, i+1+NumOfSegWC*2 ) );
    if( !h1 ) continue;
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
LC( void )
{
  Int_t id = HistMaker::getUniqueID(kLC, 0, kTDC, 1);

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(7,4);
  for( Int_t i=0; i<NumOfSegLC; ++i ){
    c1->cd(i+1);
    TH1 *h = GHist::get(id+i);
    if( !h ) continue;
    h->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
ScatHitMulti(void)
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  std::vector<Int_t> id = {
    HistMaker::getUniqueID(kTOF, 0, kHitPat),
    HistMaker::getUniqueID(kBVH, 0, kHitPat),
    HistMaker::getUniqueID(kLAC, 0, kHitPat),
    HistMaker::getUniqueID(kWC, 0, kHitPat),
    HistMaker::getUniqueID(kTOF, 0, kMulti),
    HistMaker::getUniqueID(kBVH, 0, kMulti),
    HistMaker::getUniqueID(kLAC, 0, kMulti),
    HistMaker::getUniqueID(kWC, 0, kMulti),
  };
  c1->Divide(4, 2);
  for(Int_t i=0, n=id.size(); i<n; ++i){
    c1->cd(i+1);
    auto h = GHist::get(id.at(i));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
TPC( void )
{
  std::vector<Int_t> id = {
    HistMaker::getUniqueID( kTPC, 0, kADC ),
    HistMaker::getUniqueID( kTPC, 0, kTDC ),
    HistMaker::getUniqueID( kTPC, 0, kPede ),
    HistMaker::getUniqueID( kTPC, 0, kMulti )
  };

  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 2, 2 );
  for( Int_t i=0, n=id.size(); i<n; ++i ){
    c1->cd( i+1 );
    auto h = GHist::get( id[i] );
    if( h ) h->Draw( "colz" );
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
TPCADCPAD( void )
{
  auto id = HistMaker::getUniqueID( kTPC, 0, kADC2D );
  auto c1 = new TCanvas( __func__, __func__ );
  c1->cd()->SetLogz();
  auto h = GHist::get( id );
  if( h ) h->Draw( "colz" );

  Double_t l = (500./2.)/(1+sqrt(2.));
  Double_t px[9]={-l*(1+sqrt(2.)),-l,l,l*(1+sqrt(2.)),
    l*(1+sqrt(2.)),l,-l,-l*(1+sqrt(2.)),
    -l*(1+sqrt(2.))};
  Double_t py[9]={l,l*(1+sqrt(2.)),l*(1+sqrt(2.)),l,
    -l,-l*(1+sqrt(2.)),-l*(1+sqrt(2.)),-l,
    l};
  TPolyLine* pLine = new TPolyLine( 9, px, py );
  pLine->SetLineColor(1);
  pLine->SetFillColorAlpha(kWhite, 0);
  pLine->Draw();

  return c1;
}

//_____________________________________________________________________________
TCanvas*
TPCTDCPAD( void )
{
  auto id = HistMaker::getUniqueID( kTPC, 0, kADC2D, 3 );
  auto c1 = new TCanvas( __func__, __func__ );
  c1->cd();
  auto h = GHist::get( id );
  if( h ) h->Draw( "colz" );

  Double_t l = (500./2.)/(1+sqrt(2.));
  Double_t px[9]={-l*(1+sqrt(2.)),-l,l,l*(1+sqrt(2.)),
    l*(1+sqrt(2.)),l,-l,-l*(1+sqrt(2.)),
    -l*(1+sqrt(2.))};
  Double_t py[9]={l,l*(1+sqrt(2.)),l*(1+sqrt(2.)),l,
    -l,-l*(1+sqrt(2.)),-l*(1+sqrt(2.)),-l,
    l};
  TPolyLine* pLine = new TPolyLine( 9, px, py );
  pLine->SetLineColor(1);
  pLine->SetFillColorAlpha(kWhite, 0);
  pLine->Draw();

  return c1;
}

//_____________________________________________________________________________
TCanvas*
TPCFADC( void )
{
  auto id = HistMaker::getUniqueID( kTPC, 0, kFADC );
  auto c1 = new TCanvas( __func__, __func__ );
  c1->cd()->SetLogz();
  auto h = GHist::get( id );
  if( h ) h->Draw( "colz" );
  return c1;
}

//_____________________________________________________________________________
TCanvas*
TriggerFlagU( void )
{
  auto c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 4);
  auto base_id = HistMaker::getUniqueID(kTriggerFlag, 0, kTDC);
  for(Int_t i=0; i<16; ++i){
    c1->cd(i+1)->SetLogy();
    auto h1 = GHist::get(base_id+i);
    if(!h1) continue;
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
TriggerFlagD( void )
{
  auto c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 4);
  auto base_id = HistMaker::getUniqueID(kTriggerFlag, 0, kTDC, 17);
  for(Int_t i=0; i<16; ++i){
    c1->cd(i+1)->SetLogy();
    auto h1 = GHist::get(base_id+i);
    if(!h1) continue;
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
TriggerFlagHitPat( void )
{
  auto c1 = new TCanvas(__func__, __func__);
  c1->cd(1);//->SetLogy();
  auto h1 = GHist::get(HistMaker::getUniqueID(kTriggerFlag, 0, kHitPat));
  if(h1) h1->Draw();
  return c1;
}

//_____________________________________________________________________________
TCanvas*
MsTTDC( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(6, 4);
  for( Int_t i=0; i<NumOfSegTOF; ++i ){
    c1->cd(i+1);//->SetLogy();
    TH1 *h = GHist::get( HistMaker::getUniqueID(kMsT, 0, kTDC, i+1) );
    if( !h ) continue;
    h->GetXaxis()->SetRangeUser(180000, 210000);
    h->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
MsTHitPat( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(2, 2);
  std::vector<Int_t> id = {
    HistMaker::getUniqueID(kTOF, 0, kHitPat, 1),
    HistMaker::getUniqueID(kSCH, 0, kHitPat, 1),
    HistMaker::getUniqueID(kMsT, 0, kHitPat, 0),
    HistMaker::getUniqueID(kMsT, 0, kHitPat, 1)
  };
  for( Int_t i=0, n=id.size(); i<n; ++i ){
    c1->cd(i+1);//->SetLogy();
    TH1 *h = GHist::get( id.at(i) );
    if( !h ) continue;
    // h->GetXaxis()->SetRangeUser(0,2000);
    h->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BC3TDCTOT( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4,3);
  for( Int_t i=0; i<NumOfLayersBC3; ++i ){
    Int_t pad = i + 1;
    c1->cd(pad);
    { // TDC
      TH1 *h = GHist::get( HistMaker::getUniqueID(kBC3, 0, kTDC, i+1) );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser(100, 700);
      h->Draw();
      TH1 *hh = GHist::get( HistMaker::getUniqueID(kBC3, 0, kTDC, i+1+kTOTcutOffset) );
      if( !hh ) continue;
      hh->SetLineColor(kRed+1);
      hh->Draw("same");
    }
    c1->cd(pad+NumOfLayersBC3);
    { // TOT
      TH1 *h = GHist::get( HistMaker::getUniqueID(kBC3, 0, kADC, i+1) );
      if( !h ) continue;
      h->Draw();
      TH1 *hh = GHist::get( HistMaker::getUniqueID(kBC3, 0, kADC, i+1+kTOTcutOffset) );
      if( !hh ) continue;
      hh->SetLineColor(kRed+1);
      hh->Draw("same");
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BC3HitMulti( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(3,4);
  for( Int_t i=0; i<NumOfLayersBC3; ++i ){
    c1->cd(i+1);
    TH1 *h = GHist::get( HistMaker::getUniqueID(kBC3, 0, kHitPat, i+1) );
    TH1 *h_wtot = GHist::get( HistMaker::getUniqueID(kBC3, 0, kHitPat, i+1+kTOTcutOffset) );
    if( !h ) continue;
    h->Draw();
    h_wtot->SetLineColor(kRed+1);
    h_wtot->Draw("same");
    c1->cd(i+1+NumOfLayersBC3);
    TH1 *hh = GHist::get( HistMaker::getUniqueID(kBC3, 0, kMulti, i+1) );
    TH1 *hh_wt = GHist::get( HistMaker::getUniqueID(kBC3, 0, kMulti, i+1
						    + NumOfLayersBC3 + kTOTcutOffset) );
    if( !hh ) continue;
    if( !hh_wt ) continue;
    hh_wt->GetXaxis()->SetRangeUser(0, 20);
    hh_wt->SetLineColor(kRed+1);
    hh_wt->Draw();
    hh->Draw("same");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BC4TDCTOT( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4,3);
  for( Int_t i=0; i<NumOfLayersBC4; ++i ){
    Int_t pad = i + 1;
    c1->cd(pad);
    { // TDC
      TH1 *h = GHist::get( HistMaker::getUniqueID(kBC4, 0, kTDC, i+1) );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser(100, 700);
      h->Draw();
      TH1 *hh = GHist::get( HistMaker::getUniqueID(kBC4, 0, kTDC, i+1+kTOTcutOffset) );
      if( !hh ) continue;
      hh->SetLineColor(kRed+1);
      hh->Draw("same");
    }
    c1->cd(pad+NumOfLayersBC4);
    { // TOT
      TH1 *h = GHist::get( HistMaker::getUniqueID(kBC4, 0, kADC, i+1) );
      if( !h ) continue;
      h->Draw();
      TH1 *hh = GHist::get( HistMaker::getUniqueID(kBC4, 0, kADC, i+1+kTOTcutOffset) );
      if( !hh ) continue;
      hh->SetLineColor(kRed+1);
      hh->Draw("same");
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BC4HitMulti( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(3,4);
  for( Int_t i=0; i<NumOfLayersBC4; ++i ){
    c1->cd(i+1);
    TH1 *h = GHist::get( HistMaker::getUniqueID(kBC4, 0, kHitPat, i+1) );
    TH1 *h_wtot = GHist::get( HistMaker::getUniqueID(kBC4, 0, kHitPat, i+1+kTOTcutOffset) );
    if( !h ) continue;
    h->Draw();
    h_wtot->SetLineColor(kRed+1);
    h_wtot->Draw("same");
    c1->cd(i+1+NumOfLayersBC4);
    TH1 *hh = GHist::get( HistMaker::getUniqueID(kBC4, 0, kMulti, i+1) );
    TH1 *hh_wt = GHist::get( HistMaker::getUniqueID(kBC4, 0, kMulti, i+1
						    + NumOfLayersBC4 + kTOTcutOffset) );
    if( !hh ) continue;
    if( !hh_wt ) continue;
    hh->GetXaxis()->SetRangeUser(0, 20);
    hh_wt->SetLineColor(kRed+1);
    hh_wt->Draw();
    hh->Draw("same");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SDC1TDCTOT( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 4, 3 );
  for( Int_t i=0; i<NumOfLayersSDC1; ++i ){
    c1->cd( i+1 );
    {
      auto h1 = GHist::get( HistMaker::getUniqueID( kSDC1, 0, kTDC, i+1 ) );
      if( !h1 ) continue;
      h1->GetXaxis()->SetRangeUser( 100, 700 );
      h1->Draw();
      auto h2 = GHist::get( HistMaker::getUniqueID( kSDC1, 0, kTDC, i+1+kTOTcutOffset ) );
      if( !h2 ) continue;
      h2->SetLineColor( kRed+1 );
      h2->Draw( "same" );
    }
    c1->cd( i+1+NumOfLayersSDC1 );
    {
      auto h1 = GHist::get( HistMaker::getUniqueID( kSDC1, 0, kADC, i+1 ) );
      if( !h1 ) continue;
      h1->Draw();
      auto h2 = GHist::get( HistMaker::getUniqueID(kSDC1, 0, kADC, i+1+kTOTcutOffset) );
      if( !h2 ) continue;
      h2->SetLineColor( kRed+1 );
      h2->Draw( "same" );
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SDC1HitMulti( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 3, 4 );
  for( Int_t i=0; i<NumOfLayersSDC1; ++i ){
    c1->cd( i+1 );
    auto h1 = GHist::get( HistMaker::getUniqueID( kSDC1, 0, kHitPat, i+1 ) );
    auto h1_wtot = GHist::get( HistMaker::getUniqueID( kSDC1, 0, kHitPat, i+1+kTOTcutOffset ) );
    if( !h1 || !h1_wtot ) continue;
    h1->Draw();
    h1_wtot->SetLineColor( kRed+1 );
    h1_wtot->Draw( "same" );
    c1->cd( i+1+NumOfLayersSDC1 );
    auto h2 = GHist::get( HistMaker::getUniqueID( kSDC1, 0, kMulti, i+1 ) );
    auto h2_wt = GHist::get( HistMaker::getUniqueID( kSDC1, 0, kMulti, i+1
                                                     + NumOfLayersSDC1 + kTOTcutOffset ) );
    if( !h2 ) continue;
    if( !h2_wt ) continue;
    h2_wt->GetXaxis()->SetRangeUser( 0, 20 );
    h2_wt->SetLineColor( kRed+1 );
    h2_wt->Draw();
    h2->Draw( "same" );
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SDC2TDCTOT( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 4, 2 );
  for( Int_t i=0; i<NumOfLayersSDC2; ++i ){
    c1->cd( i+1 );
    {
      auto h1 = GHist::get( HistMaker::getUniqueID( kSDC2, 0, kTDC, i+1 ) );
      if( !h1 ) continue;
      h1->GetXaxis()->SetRangeUser( 100, 700 );
      h1->Draw();
      auto h2 = GHist::get( HistMaker::getUniqueID( kSDC2, 0, kTDC, i+1+kTOTcutOffset ) );
      if( !h2 ) continue;
      h2->SetLineColor( kRed+1 );
      h2->Draw( "same" );
    }
    c1->cd( i+1+NumOfLayersSDC2 );
    {
      auto h1 = GHist::get( HistMaker::getUniqueID( kSDC2, 0, kADC, i+1 ) );
      if( !h1 ) continue;
      h1->Draw();
      auto h2 = GHist::get( HistMaker::getUniqueID( kSDC2, 0, kADC, i+1+kTOTcutOffset ) );
      if( !h2 ) continue;
      h2->SetLineColor( kRed+1 );
      h2->Draw( "same" );
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SDC2HitMulti( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 4, 2 );
  for( Int_t i=0; i<NumOfLayersSDC2; ++i ){
    c1->cd( i+1 );
    auto h1 = GHist::get( HistMaker::getUniqueID( kSDC2, 0, kHitPat, i+1 ) );
    auto h1_wtot = GHist::get( HistMaker::getUniqueID( kSDC2, 0, kHitPat, i+1+kTOTcutOffset ) );
    if( !h1 || !h1_wtot ) continue;
    h1->Draw();
    h1_wtot->SetLineColor( kRed+1 );
    h1_wtot->Draw( "same" );
    c1->cd( i+1+NumOfLayersSDC2 );
    auto h2 = GHist::get( HistMaker::getUniqueID( kSDC2, 0, kMulti, i+1 ) );
    auto h2_wt = GHist::get( HistMaker::getUniqueID( kSDC2, 0, kMulti, i+1
                                                     + NumOfLayersSDC2 + kTOTcutOffset ) );
    if( !h2 ) continue;
    if( !h2_wt ) continue;
    h2_wt->GetXaxis()->SetRangeUser( 0, 20 );
    h2_wt->SetLineColor( kRed+1 );
    h2_wt->Draw();
    h2->Draw( "same" );
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SAC( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(1,2);

  {
    c1->cd(1)->Divide(2,2);
    std::vector<Int_t> id = {
      HistMaker::getUniqueID(kSAC, 0, kADC, 1),
      HistMaker::getUniqueID(kSAC, 0, kADC, 2),
      HistMaker::getUniqueID(kSAC, 0, kADC, 3),
      HistMaker::getUniqueID(kSAC, 0, kADC, 4)
    };
    std::vector<Int_t> id_wt = {
      HistMaker::getUniqueID(kSAC, 0, kADCwTDC, 1),
      HistMaker::getUniqueID(kSAC, 0, kADCwTDC, 2),
      HistMaker::getUniqueID(kSAC, 0, kADCwTDC, 3),
      HistMaker::getUniqueID(kSAC, 0, kADCwTDC, 4)
    };
    for( Int_t i=0, n=id.size(); i<n; ++i ){
      c1->cd(1)->cd(i+1)->SetLogy();
      TH1 *h = GHist::get(id[i]);
      TH1 *h_wt = GHist::get(id_wt[i]);
      if( !h ) continue;
      if( !h_wt ) continue;
      h->Draw();
      h_wt->SetLineColor(kRed+1);
      h_wt->Draw("same");
    }
  }

  {
    c1->cd(2)->Divide(2,2);
    std::vector<Int_t> id = {
      HistMaker::getUniqueID(kSAC, 0, kTDC, 1),
      HistMaker::getUniqueID(kSAC, 0, kTDC, 2),
      HistMaker::getUniqueID(kSAC, 0, kTDC, 3),
      HistMaker::getUniqueID(kSAC, 0, kTDC, 4)
    };

    for( Int_t i=0, n=id.size(); i<n; ++i ){
      c1->cd(2)->cd(i+1);//->SetLogy();
      TH1 *h = GHist::get(id[i]);
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser(0,1500);
      h->Draw();
    }
  }

  return c1;
}

//_____________________________________________________________________________
TCanvas*
SDC3TDCTOT( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 1, 3 );
  {
    c1->cd( 1 )->Divide(4,1);
    for( Int_t i=0; i<NumOfLayersSDC3; ++i ){
      c1->cd( 1 )->cd( i+1 );
      auto h1 = GHist::get( HistMaker::getUniqueID( kSDC3, 0, kTDC, i+1 ) );
      if( !h1 ) continue;
      h1->Draw();
      auto h2 = GHist::get( HistMaker::getUniqueID( kSDC3, 0, kTDC, i+11 ) );
      if( !h2 ) continue;
      h2->SetLineColor( kRed+1 );
      h2->Draw( "same" );
    }
  }
  {
    c1->cd( 2 )->Divide( 4, 1 );
    for( Int_t i=0; i<NumOfLayersSDC3; ++i ){
      c1->cd( 2 )->cd( i+1 );
      auto h1 = GHist::get( HistMaker::getUniqueID( kSDC3, 0, kTDC2D, i+1 ) );
      if( !h1 ) continue;
      h1->Draw();
      auto h2 = GHist::get( HistMaker::getUniqueID( kSDC3, 0, kTDC2D, i + 11 ) );
      if( !h2 ) continue;
      h2->SetLineColor( kRed+1 );
      h2->Draw( "same" );
    }
  }
  {
    c1->cd(3)->Divide( 4, 1 );
    for( Int_t i=0; i<NumOfLayersSDC3; ++i ){
      c1->cd( 3 )->cd( i+1 );
      auto h1 = GHist::get( HistMaker::getUniqueID( kSDC3, 0, kADC, i+1 ) );
      if( !h1 ) continue;
      h1->Draw();
      auto h2 = GHist::get( HistMaker::getUniqueID( kSDC3, 0, kADC, i + 11 ) );
      if( !h2 ) continue;
      h2->SetLineColor( kRed+1 );
      h2->Draw( "same" );
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SDC3HitMulti( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 1, 2 );
  {
    c1->cd( 1 )->Divide( 4, 1 );
    for( Int_t i=0; i<NumOfLayersSDC3; ++i ){
      c1->cd( 1 )->cd( i+1 );
      auto h1 = GHist::get( HistMaker::getUniqueID( kSDC3, 0, kHitPat, i+1 ) );
      if( !h1 ) continue;
      h1->Draw();
      auto h2 = GHist::get( HistMaker::getUniqueID( kSDC3, 0, kHitPat, i + 11 ) );
      if( !h2 ) continue;
      h2->SetLineColor( kRed+1 );
      h2->Draw( "same" );
    }
  }
  {
    c1->cd( 2 )->Divide( 4, 1 );
    for( Int_t i=0; i<NumOfLayersSDC3; ++i ){
      c1->cd( 2 )->cd( i+1 );
      auto h1 = GHist::get( HistMaker::getUniqueID( kSDC3, 0, kMulti, i+1 ) );
      if( !h1 ) continue;
      h1->GetXaxis()->SetRangeUser( 0, 20 );
      auto h2 = GHist::get( HistMaker::getUniqueID( kSDC3, 0, kMulti, i+1 + NumOfLayersSDC3 + 10 ) );
      if( !h2 ) continue;
      h2->SetLineColor( kRed+1 );
      h2->Draw();
      h1->Draw( "same" );
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SDC4TDCTOT( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(1,3);

  {
    c1->cd(1)->Divide(4,1);
    for( Int_t i=0; i<NumOfLayersSDC4; ++i ){
      c1->cd(1)->cd(i+1);
      TH1 *h = GHist::get( HistMaker::getUniqueID(kSDC4, 0, kTDC, i+1) );
      if( !h ) continue;
      h->Draw();
      TH1 *hh = GHist::get( HistMaker::getUniqueID(kSDC4, 0, kTDC, i+11) );
      if( !hh ) continue;
      hh->SetLineColor(kRed+1);
      hh->Draw("same");
    }
  }

  {
    c1->cd(2)->Divide(4,1);
    for( Int_t i=0; i<NumOfLayersSDC4; ++i ){
      c1->cd(2)->cd(i+1);
      TH1 *h = GHist::get( HistMaker::getUniqueID(kSDC4, 0, kTDC2D, i+1) );
      if( !h ) continue;
      h->Draw();
      TH1 *hh = GHist::get( HistMaker::getUniqueID(kSDC4, 0, kTDC2D, i + 11) );
      if( !hh ) continue;
      hh->SetLineColor(kRed+1);
      hh->Draw("same");
    }
  }

  {
    c1->cd(3)->Divide(4,1);
    for( Int_t i=0; i<NumOfLayersSDC4; ++i ){
      c1->cd(3)->cd(i+1);
      TH1 *h = GHist::get( HistMaker::getUniqueID(kSDC4, 0, kADC, i+1) );
      if( !h ) continue;
      h->Draw();
      TH1 *hh = GHist::get( HistMaker::getUniqueID(kSDC4, 0, kADC, i + 11) );
      if( !hh ) continue;
      hh->SetLineColor(kRed+1);
      hh->Draw("same");
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SDC4HitMulti( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(1,2);

  {
    c1->cd(1)->Divide(4,1);
    for( Int_t i=0; i<NumOfLayersSDC4; ++i ){
      c1->cd(1)->cd(i+1);
      TH1 *h = GHist::get( HistMaker::getUniqueID(kSDC4, 0, kHitPat, i+1) );
      if( !h ) continue;
      TH1 *hh = GHist::get( HistMaker::getUniqueID(kSDC4, 0, kHitPat, i + 11) );
      if( !hh ) continue;
      hh->SetLineColor(kRed+1);
      h->Draw();
      hh->Draw("same");
    }
  }

  {
    c1->cd(2)->Divide(4,1);
    for( Int_t i=0; i<NumOfLayersSDC4; ++i ){
      c1->cd(2)->cd(i+1);
      TH1 *h = GHist::get( HistMaker::getUniqueID(kSDC4, 0, kMulti, i+1) );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser(0, 20);
      TH1 *hh = GHist::get( HistMaker::getUniqueID(kSDC4, 0, kMulti, i+1 + NumOfLayersSDC4 + 10) );
      if( !hh ) continue;
      hh->SetLineColor(kRed+1);
      hh->Draw();
      h->Draw("same");
    }
  }

  return c1;
}

//_____________________________________________________________________________
TCanvas*
HitPatternBeam( void )
{
  std::vector<Int_t> hist_id = {
    HistMaker::getUniqueID(kBH1,  0, kHitPat),
    HistMaker::getUniqueID(kBFT,  0, kHitPat, 1),
    HistMaker::getUniqueID(kBFT,  0, kHitPat, 2),
    HistMaker::getUniqueID(kBC3,  0, kHitPat),
    HistMaker::getUniqueID(kBC4,  0, kHitPat, 5),
    HistMaker::getUniqueID(kBH2,  0, kHitPat)
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(3, 2);
  for (Int_t i=0, n=hist_id.size(); i<n; ++i) {
    c1->cd(i + 1);
    auto h1 = GHist::get(hist_id.at(i));
    if (!h1) continue;
    h1->SetMinimum(0);
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
HitPatternScat( void )
{
  std::vector<Int_t> hist_id = {
    HistMaker::getUniqueID(kHTOF, 0, kHitPat),
    HistMaker::getUniqueID(kSDC1, 0, kHitPat, 3),
    HistMaker::getUniqueID(kSDC2, 0, kHitPat),
    HistMaker::getUniqueID(kSCH, 0, kHitPat),
    HistMaker::getUniqueID(kSDC3, 0, kHitPat),
    HistMaker::getUniqueID(kSDC4, 0, kHitPat, 3),
    HistMaker::getUniqueID(kTOF, 0, kHitPat),
    HistMaker::getUniqueID(kBVH, 0, kHitPat),
    HistMaker::getUniqueID(kLAC, 0, kHitPat),
    HistMaker::getUniqueID(kWC, 0, kHitPat)
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 3);
  for (Int_t i=0, n=hist_id.size(); i<n; ++i) {
    c1->cd(i + 1);
    auto h1 = GHist::get(hist_id.at(i));
    if (!h1) continue;
    h1->SetMinimum(0);
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
HitPatternE03( void )
{
  std::vector<Int_t> hist_id_1 = {
    HistMaker::getUniqueID( kBH1,  0, kHitPat ),
    HistMaker::getUniqueID( kBFT,  0, kHitPat, 1 ),
    HistMaker::getUniqueID( kBFT,  0, kHitPat, 2 ),
    HistMaker::getUniqueID( kBC3,  0, kHitPat ),
    HistMaker::getUniqueID( kBC4,  0, kHitPat, 5 ),
    HistMaker::getUniqueID( kBH2,  0, kHitPat )
  };
  std::vector<Int_t> hist_id_2 = {
    HistMaker::getUniqueID( kSDC1, 0, kHitPat, 3 ),
    HistMaker::getUniqueID( kSDC2, 0, kHitPat ),
    HistMaker::getUniqueID( kSCH,  0, kHitPat ),
    HistMaker::getUniqueID( kSDC3, 0, kHitPat ),
    HistMaker::getUniqueID( kSDC4, 0, kHitPat, 3 ),
    HistMaker::getUniqueID( kTOF,  0, kHitPat ),
    HistMaker::getUniqueID( kLAC,  0, kHitPat ),
    HistMaker::getUniqueID( kWC,   0, kHitPat )
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide( 1, 2 );
  {
    c1->cd( 1 )->Divide(3, 2 );
    for( Int_t i=0, n=hist_id_1.size(); i<n; ++i ){
      c1->cd( 1 )->cd( i+1 );
      auto h1 = GHist::get( hist_id_1.at( i ) );
      if( !h1 ) continue;
      h1->SetMinimum( 0 );
      h1->Draw();
    }
  }
  {
    c1->cd( 2 )->Divide( 4, 2 );
    for( Int_t i=0, n=hist_id_2.size(); i<n; ++i ){
      c1->cd( 2 )->cd( i+1 );
      auto h1 = GHist::get( hist_id_2.at( i ) );
      if( !h1 ) continue;
      h1->SetMinimum( 0 );
      h1->Draw();
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
HitPatternE40( void )
{
  std::vector<Int_t> id = {
    HistMaker::getUniqueID(kBH1,  0, kHitPat),
    HistMaker::getUniqueID(kBFT,  0, kHitPat),
    HistMaker::getUniqueID(kBFT,  0, kHitPat, 2),

    HistMaker::getUniqueID(kBC3,  0, kHitPat, 1),
    HistMaker::getUniqueID(kBC4,  0, kHitPat, 5),
    HistMaker::getUniqueID(kBH2,  0, kHitPat, 1)
  };
  std::vector<Int_t> id2 = {
    HistMaker::getUniqueID(kSDC1, 0, kHitPat, 1),
    HistMaker::getUniqueID(kSDC2, 0, kHitPat, 1),
    HistMaker::getUniqueID(kSFT,  0, kHitPat, 1),
    HistMaker::getUniqueID(kSCH,  0, kHitPat),
    HistMaker::getUniqueID(kFHT1, 0, kHitPat, 1),

    HistMaker::getUniqueID(kSDC3, 0, kHitPat, 1),
    HistMaker::getUniqueID(kSDC4, 0, kHitPat, 3),
    HistMaker::getUniqueID(kTOF,  0, kHitPat),
    HistMaker::getUniqueID(kLC,  0, kHitPat)
  };
  // HistMaker::getUniqueID(kFHT1, 0, kHitPat),
  // HistMaker::getUniqueID(kFHT1, 0, kHitPat,FHTOffset),
  // HistMaker::getUniqueID(kFHT1, 1, kHitPat),
  // HistMaker::getUniqueID(kFHT1, 1, kHitPat,FHTOffset),
  // HistMaker::getUniqueID(kFHT2, 0, kHitPat),
  // HistMaker::getUniqueID(kFHT2, 0, kHitPat,FHTOffset),
  // HistMaker::getUniqueID(kFHT2, 1, kHitPat),
  // HistMaker::getUniqueID(kFHT2, 1, kHitPat,FHTOffset),

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(1,2);
  {
    c1->cd(1)->Divide(3,2);
    for( Int_t i=0, n=id.size(); i<n; ++i ){
      c1->cd(1)->cd(i+1);
      TH1* h = GHist::get( id.at(i) );
      if( !h ) continue;
      h->SetMinimum(0);
      h->Draw();
    }
  }
  {
    c1->cd(2)->Divide(4,2);
    for( Int_t i=0, n=id2.size(); i<n; ++i ){
      c1->cd(2)->cd(i+1);
      TH1* h = GHist::get( id2.at(i) );
      if( !h ) continue;
      h->SetMinimum(0);
      h->Draw();
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
DAQ( void )
{
  std::vector<Int_t> hist_id = {
    HistMaker::getUniqueID(kDAQ, kEB, kHitPat),
    HistMaker::getUniqueID(kDAQ, kVME, kHitPat2D),
    HistMaker::getUniqueID(kDAQ, kEASIROC, kHitPat2D),
    HistMaker::getUniqueID(kDAQ, kHUL, kHitPat2D),
    HistMaker::getUniqueID(kDAQ, kCoBo, kHitPat2D),
    // HistMaker::getUniqueID(kDAQ,  kCLite,   kHitPat2D),
    // HistMaker::getUniqueID(kDAQ,  kOpt,     kHitPat2D),
  };
  auto c1 = new TCanvas(__func__, __func__);
  c1->Divide(3, 2);
  for(Int_t i=0, n=hist_id.size(); i<n; ++i){
    c1->cd(i + 1); //->SetGrid();
    auto h1 = GHist::get(hist_id.at(i));
    if(!h1) continue;
    h1->Draw( "colz" );
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
GeADC( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 4, 5 );
  const auto base_id = HistMaker::getUniqueID( kGe, 0, kADC );
  for( Int_t i=0; i<NumOfSegGe; ++i ){
    c1->cd( i+1 );
    auto h1 = GHist::get( base_id + i );
    if( !h1 ) continue;
    h1->Draw();
  }
  std::vector<Int_t> hist_id = {
    HistMaker::getUniqueID( kGe, 0, kADC, NumOfSegGe+1 ),
    HistMaker::getUniqueID( kGe, 0, kADC, NumOfSegGe+2 ),
    HistMaker::getUniqueID( kGe, 0, kADC2D ),
    HistMaker::getUniqueID( kGe, 0, kHitPat ),
  };
  for( Int_t i=0, n=hist_id.size(); i<n; ++i ){
    c1->cd( i+1+NumOfSegGe );
    auto h1 = GHist::get( hist_id[i] );
    if( !h1 ) continue;
    h1->Draw( "colz" );
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
GeCRM( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 4, 4 );
  const auto base_id = HistMaker::getUniqueID( kGe, 0, kCRM );
  for( Int_t i=0; i<NumOfSegGe; ++i ){
    c1->cd( i+1 );
    auto h1 = GHist::get( base_id + i );
    if( !h1 ) continue;
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
GeTFA( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 4, 4 );
  const auto base_id = HistMaker::getUniqueID( kGe, 0, kTFA );
  for( Int_t i=0; i<NumOfSegGe; ++i ){
    c1->cd( i+1 );
    auto h1 = GHist::get( base_id + i );
    if( !h1 ) continue;
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
GeRST( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 4, 4 );
  const auto base_id = HistMaker::getUniqueID( kGe, 0, kRST );
  for( Int_t i=0; i<NumOfSegGe; ++i ){
    c1->cd( i+1 );
    auto h1 = GHist::get( base_id + i );
    if( !h1 ) continue;
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
GeMultiCRM( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 4, 4 );
  const auto base_id = HistMaker::getUniqueID( kGe, 0, kMulti );
  for( Int_t i=0; i<NumOfSegGe; ++i ){
    c1->cd( i+1 );
    auto h1 = GHist::get( base_id + i );
    if( !h1 ) continue;
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
GeMultiTFA( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 4, 4 );
  const auto base_id = HistMaker::getUniqueID( kGe, 0, kMulti, NumOfSegGe+1 );
  for( Int_t i=0; i<NumOfSegGe; ++i ){
    c1->cd( i+1 );
    auto h1 = GHist::get( base_id + i );
    if( !h1 ) continue;
    h1->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
GeTFACRM( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 4, 4 );
  const auto base_id = HistMaker::getUniqueID( kGe, 0, kTFA_CRM );
  for( Int_t i=0; i<NumOfSegGe; ++i ){
    c1->cd( i+1 );
    auto h1 = GHist::get( base_id + i );
    if( !h1 ) continue;
    h1->Draw( "colz" );
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
GeTFAADC( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 4, 4 );
  const auto base_id = HistMaker::getUniqueID( kGe, 0, kTFA_ADC );
  for( Int_t i=0; i<NumOfSegGe; ++i ){
    c1->cd( i+1 );
    auto h1 = GHist::get( base_id + i );
    if( !h1 ) continue;
    h1->Draw( "colz" );
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BGOTDC( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 8, 6 );
  const auto base_id = HistMaker::getUniqueID( kBGO, 0, kTDC );
  for( Int_t i=0; i<NumOfSegBGO; ++i ){
    c1->cd( i+1 );
    auto h1 = GHist::get( base_id + i );
    if( !h1 ) continue;
    h1->Draw( "colz" );
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas* GeRSTADC( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 4, 4 );
  const auto base_id = HistMaker::getUniqueID( kGe, 0, kRST_ADC );
  for( Int_t i=0; i<NumOfSegGe; ++i ){
    c1->cd( i+1 );
    auto h1 = GHist::get( base_id + i );
    if( !h1 ) continue;
    h1->Draw( "colz" );
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
FHT1TDC( void )
{
  const int FHTOffset = 200;

  std::vector<Int_t> hid = {
    HistMaker::getUniqueID(kFHT1, 0, kTDC,    NumOfSegFHT1+1),
    HistMaker::getUniqueID(kFHT1, 0, kTDC,    FHTOffset+NumOfSegFHT1+1),
    HistMaker::getUniqueID(kFHT1, 1, kTDC,    NumOfSegFHT1+1),
    HistMaker::getUniqueID(kFHT1, 1, kTDC,    FHTOffset+NumOfSegFHT1+1),
    HistMaker::getUniqueID(kFHT1, 0, kTDC2D,  1),
    HistMaker::getUniqueID(kFHT1, 0, kTDC2D,  FHTOffset+1),
    HistMaker::getUniqueID(kFHT1, 1, kTDC2D,  1),
    HistMaker::getUniqueID(kFHT1, 1, kTDC2D,  FHTOffset+1)
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t i=0, n=hid.size(); i<n; ++i ){
    c1->cd(i+1);
    TH1* h = GHist::get( hid.at(i) );
    if( !h ) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
FHT1TOT( void )
{
  const int FHTOffset = 200;

  std::vector<Int_t> hid = {
    HistMaker::getUniqueID(kFHT1, 0, kADC,    NumOfSegFHT1+1),
    HistMaker::getUniqueID(kFHT1, 0, kADC,    FHTOffset+NumOfSegFHT1+1),
    HistMaker::getUniqueID(kFHT1, 1, kADC,    NumOfSegFHT1+1),
    HistMaker::getUniqueID(kFHT1, 1, kADC,    FHTOffset+NumOfSegFHT1+1),
    HistMaker::getUniqueID(kFHT1, 0, kADC2D,  1),
    HistMaker::getUniqueID(kFHT1, 0, kADC2D,  FHTOffset+1),
    HistMaker::getUniqueID(kFHT1, 1, kADC2D,  1),
    HistMaker::getUniqueID(kFHT1, 1, kADC2D,  FHTOffset+1)
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t i=0, n=hid.size(); i<n; ++i ){
    c1->cd(i+1);
    TH1* h = GHist::get( hid.at(i) );
    if( !h ) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
FHT1HitMulti( void )
{
  const int FHTOffset = 200;

  std::vector<Int_t> hid = {
    HistMaker::getUniqueID(kFHT1, 0, kHitPat, 1),
    HistMaker::getUniqueID(kFHT1, 0, kHitPat, FHTOffset+1),
    HistMaker::getUniqueID(kFHT1, 1, kHitPat, 1),
    HistMaker::getUniqueID(kFHT1, 1, kHitPat, FHTOffset+1),
    HistMaker::getUniqueID(kFHT1, 0, kMulti,  1),
    HistMaker::getUniqueID(kFHT1, 0, kMulti,  FHTOffset+1),
    HistMaker::getUniqueID(kFHT1, 1, kMulti,  1),
    HistMaker::getUniqueID(kFHT1, 1, kMulti,  FHTOffset+1),
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t i=0, n=hid.size(); i<n; ++i ){
    c1->cd(i+1);
    TH1* h = GHist::get( hid.at(i) );
    if( !h ) continue;
    if( i > 3 ) h->GetXaxis()->SetRangeUser(0, 20);
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
FHT2TDC( void )
{
  const int FHTOffset = 200;

  std::vector<Int_t> hid = {
    HistMaker::getUniqueID(kFHT2, 0, kTDC,    NumOfSegFHT2+1),
    HistMaker::getUniqueID(kFHT2, 0, kTDC,    FHTOffset+NumOfSegFHT2+1),
    HistMaker::getUniqueID(kFHT2, 1, kTDC,    NumOfSegFHT2+1),
    HistMaker::getUniqueID(kFHT2, 1, kTDC,    FHTOffset+NumOfSegFHT2+1),
    HistMaker::getUniqueID(kFHT2, 0, kTDC2D,  1),
    HistMaker::getUniqueID(kFHT2, 0, kTDC2D,  FHTOffset+1),
    HistMaker::getUniqueID(kFHT2, 1, kTDC2D,  1),
    HistMaker::getUniqueID(kFHT2, 1, kTDC2D,  FHTOffset+1)
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t i=0, n=hid.size(); i<n; ++i ){
    c1->cd(i+1);
    TH1* h = GHist::get( hid.at(i) );
    if( !h ) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
FHT2TOT( void )
{
  const int FHTOffset = 200;

  std::vector<Int_t> hid = {
    HistMaker::getUniqueID(kFHT2, 0, kADC,    NumOfSegFHT2+1),
    HistMaker::getUniqueID(kFHT2, 0, kADC,    FHTOffset+NumOfSegFHT2+1),
    HistMaker::getUniqueID(kFHT2, 1, kADC,    NumOfSegFHT2+1),
    HistMaker::getUniqueID(kFHT2, 1, kADC,    FHTOffset+NumOfSegFHT2+1),
    HistMaker::getUniqueID(kFHT2, 0, kADC2D,  1),
    HistMaker::getUniqueID(kFHT2, 0, kADC2D,  FHTOffset+1),
    HistMaker::getUniqueID(kFHT2, 1, kADC2D,  1),
    HistMaker::getUniqueID(kFHT2, 1, kADC2D,  FHTOffset+1)
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t i=0, n=hid.size(); i<n; ++i ){
    c1->cd(i+1);
    TH1* h = GHist::get( hid.at(i) );
    if( !h ) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
FHT2HitMulti( void )
{
  const int FHTOffset = 200;

  std::vector<Int_t> hid = {
    HistMaker::getUniqueID(kFHT2, 0, kHitPat, 1),
    HistMaker::getUniqueID(kFHT2, 0, kHitPat, FHTOffset+1),
    HistMaker::getUniqueID(kFHT2, 1, kHitPat, 1),
    HistMaker::getUniqueID(kFHT2, 1, kHitPat, FHTOffset+1),
    HistMaker::getUniqueID(kFHT2, 0, kMulti,  1),
    HistMaker::getUniqueID(kFHT2, 0, kMulti,  FHTOffset+1),
    HistMaker::getUniqueID(kFHT2, 1, kMulti,  1),
    HistMaker::getUniqueID(kFHT2, 1, kMulti,  FHTOffset+1),
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t i=0, n=hid.size(); i<n; ++i ){
    c1->cd(i+1);
    TH1* h = GHist::get( hid.at(i) );
    if( !h ) continue;
    if( i > 3 ) h->GetXaxis()->SetRangeUser(0, 20);
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTTDC( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kTDC, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTTDC2D( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kTDC2D, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTTOT( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kADC, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTTOT2D( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kADC2D, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTHighGain( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kHighGain, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTHighGain2D( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kHighGain, l+11));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTLowGain( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kLowGain, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTLowGain2D( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kLowGain, l+11));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTPedestal( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kPede, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTPedestal2D( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, 0, kPede, l+11));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTHitPat( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
    c1->cd(l+1);
    TH1 *h1 = GHist::get(HistMaker::getUniqueID(kCFT, 0, kHitPat, l+1));
    if( !h1 ) continue;
    h1->Draw();
    TH1 *h2 = GHist::get(HistMaker::getUniqueID(kCFT, 0, kHitPat, l+11));
    if( !h2 ) continue;
    h2->SetLineColor(kRed+1);
    h2->Draw("same");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTMulti( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
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
CFTClusterHighGain( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, kCluster, kHighGain, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTClusterHighGain2D( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, kCluster, kHighGain, l+11));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTClusterLowGain( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, kCluster, kLowGain, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTClusterLowGain2D( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, kCluster, kLowGain, l+11));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTClusterTDC( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, kCluster, kTDC, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTClusterTDC2D( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  for( Int_t l=0; l<NumOfLayersCFT; ++l ){
    c1->cd(l+1);
    TH1 *h = GHist::get(HistMaker::getUniqueID(kCFT, kCluster, kTDC2D, l+1));
    if(!h) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BGOFADC( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(6, 4);
  TH1 *h;
  for( Int_t i=0; i<NumOfSegBGO; ++i ){
    c1->cd(i+1);
    h = GHist::get( HistMaker::getUniqueID(kBGO, 0, kFADC, i+1) );
    if(!h) continue;
    h->GetXaxis()->SetRangeUser( 80, 200 );
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BGOADC( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(6, 4);
  TH1 *h;
  for( Int_t i=0; i<NumOfSegBGO; ++i ){
    c1->cd(i+1);
    h = GHist::get( HistMaker::getUniqueID(kBGO, 0, kADC, i+1) );
    if(!h) continue;
    h->Draw();
    h = GHist::get( HistMaker::getUniqueID(kBGO, 0, kADCwTDC, i+1) );
    if(!h) continue;
    h->SetLineColor(kRed+1);
    h->Draw("same");
  }
  return c1;
}

//_____________________________________________________________________________
// TCanvas*
// BGOTDC( void )
// {
//   TCanvas *c1 = new TCanvas(__func__, __func__);
//   c1->Divide(6, 4);
//   TH1 *h;
//   for( Int_t i=0; i<NumOfSegBGO; ++i ){
//     c1->cd(i+1);
//     h = GHist::get( HistMaker::getUniqueID(kBGO, 0, kTDC, i+1) );
//     if(!h) continue;
//     h->Draw();
//   }
//   return c1;
// }

//_____________________________________________________________________________
TCanvas*
BGOADCTDC2D( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(2, 2);
  TH1 *h;
  for( Int_t i=0; i<3; ++i ){
    c1->cd(i+1);
    h = GHist::get( HistMaker::getUniqueID(kBGO, 0, kADC2D, i+1) );
    if(!h) continue;
    h->Draw("colz");
  }
  h = GHist::get( HistMaker::getUniqueID(kBGO, 0, kTDC2D) );
  c1->cd(4);
  if(h)
    h->Draw("colz");
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BGOHitMulti( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4, 2);
  TH1 *h;
  Int_t id = HistMaker::getUniqueID(kBGO, 0, kTDC, 25);
  for( Int_t i=0; i<NumOfSegBGO_T; ++i ){
    c1->cd(i+1);
    h = (TH1*)GHist::get( id + i );
    if( !h ) continue;
    h->Draw();
  }
  id = HistMaker::getUniqueID(kBGO, 0, kHitPat, 1);
  h = (TH1*)GHist::get( id );
  c1->cd(5);
  h->Draw();
  id = HistMaker::getUniqueID(kBGO, 0, kHitPat, 2);
  h = (TH1*)GHist::get( id );
  c1->cd(6);
  h->Draw();
  id = HistMaker::getUniqueID(kBGO, 0, kMulti, 1);
  h = (TH1*)GHist::get( id );
  c1->cd(7);
  h->Draw();
  id = HistMaker::getUniqueID(kBGO, 0, kMulti, 2);
  h = (TH1*)GHist::get( id );
  c1->cd(8);
  h->Draw();
  return c1;
}

//_____________________________________________________________________________
TCanvas*
PiIDTDC( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(8, 4);
  TH1 *h;
  for( Int_t i=0; i<NumOfSegPiID; ++i ){
    c1->cd(i+1);
    h = GHist::get(HistMaker::getUniqueID(kPiID, 0, kTDC, i+1));
    if(!h) continue;
    h->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
PiIDHighGain( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(8, 4);
  TH1 *h;
  for( Int_t i=0; i<NumOfSegPiID; ++i ){
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
PiIDLowGain( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(8, 4);
  TH1 *h;
  for( Int_t i=0; i<NumOfSegPiID; ++i ){
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
PiIDHitMulti( void )
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
  for( Int_t i=0, n=id.size(); i<n; ++i ){
    c1->cd(i+1);
    h = GHist::get( id.at(i) );
    if(!h) continue;
    h->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
HitPatternE07( void )
{
  std::vector<Int_t> id = {
    HistMaker::getUniqueID(kBH1,  0, kHitPat),
    HistMaker::getUniqueID(kBFT,  0, kHitPat),
    HistMaker::getUniqueID(kBC4,  0, kHitPat, 6),
    HistMaker::getUniqueID(kFBH,  0, kHitPat),
    HistMaker::getUniqueID(kSDC1, 0, kHitPat, 1),
    HistMaker::getUniqueID(kSCH,  0, kHitPat),
    HistMaker::getUniqueID(kSDC2, 0, kHitPat, 1),
    HistMaker::getUniqueID(kTOF,  0, kHitPat)
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4,4);
  for( Int_t i=0, n=id.size(); i<n; ++i ){
    c1->cd(i+1);
    TH1* h = GHist::get( id[i] );
    h->SetMinimum(0);
    h->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
EMC( void )
{
  Int_t id = HistMaker::getUniqueID(kEMC, 0, kXYpos);
  TH1 *h = GHist::get(id);
  if( !h ) return 0;
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->cd();
  h->SetStats(kFALSE);
  h->Draw("colz");
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SSD1ADCTDC( void )
{
  Int_t id[kAorT] = {
    HistMaker::getUniqueID(kSSD1, 0, kADC2D),
    HistMaker::getUniqueID(kSSD1, 0, kTDC2D)
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4,2);
  for( Int_t i=0; i<NumOfLayersSSD1; ++i ){
    for( Int_t j=0; j<kAorT; ++j ){
      c1->cd(i+1+j*NumOfLayersSSD1);
      TH1 *h = GHist::get(id[j]+i);
      if( !h ) continue;
      h->Draw("colz");
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SSD2ADCTDC( void )
{
  Int_t id[kAorT] = {
    HistMaker::getUniqueID(kSSD2, 0, kADC2D),
    HistMaker::getUniqueID(kSSD2, 0, kTDC2D)
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4,2);
  for( Int_t i=0; i<NumOfLayersSSD2; ++i ){
    for( Int_t j=0; j<kAorT; ++j ){
      c1->cd(i+1+j*NumOfLayersSSD2);
      TH1 *h = GHist::get(id[j]+i);
      if( !h ) continue;
      h->Draw("colz");
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SSD1HitMulti( void )
{
  Int_t id[kAorT] = {
    HistMaker::getUniqueID(kSSD1, 0, kHitPat),
    HistMaker::getUniqueID(kSSD1, 0, kMulti)
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4,2);
  for( Int_t i=0; i<NumOfLayersSSD1; ++i ){
    for( Int_t j=0; j<kAorT; ++j ){
      c1->cd(i+1+j*NumOfLayersSSD1);
      TH1 *h = GHist::get(id[j]+2*i);
      if( !h ) continue;
      h->Draw("colz");
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SSD2HitMulti( void )
{
  Int_t id[kAorT] = {
    HistMaker::getUniqueID(kSSD2, 0, kHitPat),
    HistMaker::getUniqueID(kSSD2, 0, kMulti)
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4,2);
  for( Int_t i=0; i<NumOfLayersSSD2; ++i ){
    for( Int_t j=0; j<kAorT; ++j ){
      c1->cd(i+1+j*NumOfLayersSSD2);
      TH1 *h = GHist::get(id[j]+2*i);
      if( !h ) continue;
      h->Draw("colz");
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SSD1CHitMulti( void )
{
  Int_t id[kAorT] = {
    HistMaker::getUniqueID(kSSD1, 0, kHitPat),
    HistMaker::getUniqueID(kSSD1, 0, kMulti)
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4,2);
  for( Int_t i=0; i<NumOfLayersSSD1; ++i ){
    for( Int_t j=0; j<kAorT; ++j ){
      c1->cd(i+1+j*NumOfLayersSSD1);
      Int_t uid = j==kA ? id[j]+2*i+1 : id[j]+2*i;
      TH1 *h = GHist::get(uid);
      if( !h ) continue;
      if(j==kT) h->GetXaxis()->SetRangeUser(0,40);
      h->Draw("colz");
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SSD2CHitMulti( void )
{
  Int_t id[kAorT] = {
    HistMaker::getUniqueID(kSSD2, 0, kHitPat),
    HistMaker::getUniqueID(kSSD2, 0, kMulti)
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4,2);
  for( Int_t i=0; i<NumOfLayersSSD2; ++i ){
    for( Int_t j=0; j<kAorT; ++j ){
      c1->cd(i+1+j*NumOfLayersSSD2);
      Int_t uid = j==kA ? id[j]+2*i+1 : id[j]+2*i;
      TH1 *h = GHist::get(uid);
      if( !h ) continue;
      if(j==kT) h->GetXaxis()->SetRangeUser(0,40);
      h->Draw("colz");
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SSD1dETime( void )
{
  std::vector<Int_t> id = {
    HistMaker::getUniqueID(kSSD1, 0, kDeltaE),
    HistMaker::getUniqueID(kSSD1, 0, kDeltaE2D),
    HistMaker::getUniqueID(kSSD1, 0, kCTime),
    HistMaker::getUniqueID(kSSD1, 0, kCTime2D)
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4,4);
  for( Int_t i=0; i<NumOfLayersSSD1; ++i ){
    for( Int_t j=0, n=id.size(); j<n; ++j ){
      c1->cd(i+1+j*NumOfLayersSSD1);
      TH1 *h = GHist::get(id[j]+i);
      if( !h ) continue;
      h->Draw("colz");
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SSD2dETime( void )
{
  std::vector<Int_t> id = {
    HistMaker::getUniqueID(kSSD2, 0, kDeltaE),
    HistMaker::getUniqueID(kSSD2, 0, kDeltaE2D),
    HistMaker::getUniqueID(kSSD2, 0, kCTime),
    HistMaker::getUniqueID(kSSD2, 0, kCTime2D)
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4,4);
  for( Int_t i=0; i<NumOfLayersSSD2; ++i ){
    for( Int_t j=0, n=id.size(); j<n; ++j ){
      c1->cd(i+1+j*NumOfLayersSSD2);
      TH1 *h = GHist::get(id[j]+i);
      if( !h ) continue;
      h->Draw("colz");
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SSD12Chisqr( void )
{
  std::vector<Int_t> id = {
    HistMaker::getUniqueID(kSSD1, 0, kChisqr),
    HistMaker::getUniqueID(kSSD2, 0, kChisqr)
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4,2);
  for( Int_t i=0; i<NumOfLayersSSD1; ++i ){
    for( Int_t j=0, n=id.size(); j<n; ++j ){
      c1->cd(i+1+j*NumOfLayersSSD1);
      TH1 *h = GHist::get(id[j]+i);
      if( !h ) continue;
      h->Draw("colz");
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BcOutEfficiency( void )
{
  std::vector<Int_t> hist_id = {
    HistMaker::getUniqueID( kBC3, 0, kMulti, NumOfLayersBC3 ),
    HistMaker::getUniqueID( kBC4, 0, kMulti, NumOfLayersBC4 )
  };
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 3, 4 );
  for( Int_t i=0; i<NumOfLayersBC3; ++i ){
    for( Int_t j=0, n=hist_id.size(); j<n; ++j ){
      c1->cd( i+1+j*NumOfLayersBC3 );
      auto h1 = GHist::get( hist_id[j]+i+1 );
      auto h2 = GHist::get( hist_id[j]+i+1+kTOTcutOffset );
      if( !h1 || !h2 ) continue;
      h2->GetXaxis()->SetRangeUser( 0, 20 );
      h2->Draw();
      h1->Draw( "same" );
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SdcInOutEfficiency( void )
{
  std::vector<Int_t> hist_id = {
    HistMaker::getUniqueID( kSDC1, 0, kMulti, NumOfLayersSDC1 ),
    HistMaker::getUniqueID( kSDC2, 0, kMulti, NumOfLayersSDC2 ),
    HistMaker::getUniqueID( kSDC3, 0, kMulti, NumOfLayersSDC3 ),
    HistMaker::getUniqueID( kSDC4, 0, kMulti, NumOfLayersSDC4 )
  };
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 1, hist_id.size() );
  c1->cd( 1 )->Divide( NumOfLayersSDC1, 1 );
  for( Int_t i=0; i<NumOfLayersSDC1; ++i ){
    c1->cd( 1 )->cd( i+1 );
    auto h1 = GHist::get( hist_id[0]+i+1 );
    auto h2 = GHist::get( hist_id[0]+i+1+kTOTcutOffset );
    if( !h1 || !h2 ) continue;
    h2->GetXaxis()->SetRangeUser( 0, 20 );
    h2->Draw();
    h1->Draw( "same" );
  }
  c1->cd( 2 )->Divide( NumOfLayersSDC2, 1 );
  for( Int_t i=0; i<NumOfLayersSDC2; ++i ){
    c1->cd( 2 )->cd( i+1 );
    auto h1 = GHist::get( hist_id[1]+i+1 );
    auto h2 = GHist::get( hist_id[1]+i+1+kTOTcutOffset );
    if( !h1 || !h2 ) continue;
    h2->GetXaxis()->SetRangeUser( 0, 20 );
    h2->Draw();
    h1->Draw( "same" );
  }
  c1->cd( 3 )->Divide( NumOfLayersSDC3, 1 );
  for( Int_t i=0; i<NumOfLayersSDC3; ++i ){
    c1->cd( 3 )->cd( i+1 );
    auto h1 = GHist::get( hist_id[2]+i+1 );
    auto h2 = GHist::get( hist_id[2]+i+11 );
    if( !h1 || !h2 ) continue;
    h2->GetXaxis()->SetRangeUser( 0, 20 );
    h2->Draw();
    h1->Draw( "same" );
  }
  c1->cd( 4 )->Divide( NumOfLayersSDC4, 1 );
  for( Int_t i=0; i<NumOfLayersSDC4; ++i ){
    c1->cd( 4 )->cd( i+1 );
    auto h1 = GHist::get( hist_id[3]+i+1 );
    auto h2 = GHist::get( hist_id[3]+i+11 );
    if( !h1 || !h2 ) continue;
    h2->GetXaxis()->SetRangeUser( 0, 20 );
    h2->Draw();
    h1->Draw( "same" );
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
CFTEfficiency( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4,4);
  for( Int_t i=0; i<NumOfLayersCFT; ++i ){
    c1->cd(i+1);
    TH1 *h1 = GHist::get( HistMaker::getUniqueID(kCFT, 0, kMulti, i+11) );
    if( !h1 ) continue;
    h1->Draw();
    c1->cd(i+1+NumOfLayersCFT);
    TH1 *h2 = GHist::get( HistMaker::getUniqueID(kCFT, 0, kMulti, i+21) );
    if( !h2 ) continue;
    h2->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
SSDEfficiency( void )
{
  std::vector<Int_t> id = {
    HistMaker::getUniqueID(kSSD1, 0, kMulti),
    HistMaker::getUniqueID(kSSD2, 0, kMulti)
  };

  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4,2);
  for( Int_t i=0; i<NumOfLayersSSD1; ++i ){
    for( Int_t j=0, n=id.size(); j<n; ++j ){
      c1->cd(i+1+j*NumOfLayersSSD1);
      TH1 *h = GHist::get(id[j]+2*i+1);
      if( !h ) continue;
      h->Draw("colz");
    }
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BFTSCHTOT( void )
{
  auto c1 = new TCanvas( __func__, __func__ );
  c1->Divide( 3, 1 );
  std::vector<Int_t> hist_id = {
    HistMaker::getUniqueID( kBFT, 0, kADC, 11 ),
    HistMaker::getUniqueID( kBFT, 0, kADC, 12 ),
    HistMaker::getUniqueID( kSCH, 0, kADC, NumOfSegSCH+1 ),
  };
  for( Int_t i=0, n=hist_id.size(); i<n; ++i ){
    c1->cd( i+1 );
    auto h = GHist::get( hist_id[i] );
    if( !h ) continue;
    h->GetXaxis()->SetRangeUser( -20, 100 );
    h->Draw();
  }
  return c1;
}


//_____________________________________________________________________________
// TCanvas*
// BFTSFTSCHTOT( void )
// {
//   TCanvas *c1 = new TCanvas(__func__, __func__);
//   c1->Divide(4,2);
//   std::vector<Int_t> id = {
//     HistMaker::getUniqueID(kBFT, 0, kADC, 11),
//     HistMaker::getUniqueID(kBFT, 0, kADC, 12),
//     HistMaker::getUniqueID(kSFT, 0, kADC, 11),
//     HistMaker::getUniqueID(kSFT, 0, kADC, 12),
//     HistMaker::getUniqueID(kSFT, 0, kADC, 13),
//     HistMaker::getUniqueID(kSFT, 0, kADC, 14),
//     HistMaker::getUniqueID(kSCH, 0, kADC, kSCH_1to16_Offset),
//     HistMaker::getUniqueID(kSCH, 0, kADC, kSCH_17to64_Offset)
//   };
//   for( Int_t i=0, n=id.size(); i<n; ++i ){
//     c1->cd(i+1);
//     TH1 *h = GHist::get(id[i]);
//     if( !h ) continue;
//     h->GetXaxis()->SetRangeUser(-20, 100);
//     h->Draw();
//   }
//   return c1;
// }

//_____________________________________________________________________________
TCanvas*
FHTTOT( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide(4,2);
  std::vector<Int_t> id = {
    HistMaker::getUniqueID(kFHT1, 0, kADC, NumOfSegFHT1+1),
    HistMaker::getUniqueID(kFHT1, 0, kADC, FHTOffset+NumOfSegFHT1+1),
    HistMaker::getUniqueID(kFHT1, 1, kADC, NumOfSegFHT1+1),
    HistMaker::getUniqueID(kFHT1, 1, kADC, FHTOffset+NumOfSegFHT1+1),
    HistMaker::getUniqueID(kFHT2, 0, kADC, NumOfSegFHT2+1),
    HistMaker::getUniqueID(kFHT2, 0, kADC, FHTOffset+NumOfSegFHT2+1),
    HistMaker::getUniqueID(kFHT2, 1, kADC, NumOfSegFHT2+1),
    HistMaker::getUniqueID(kFHT2, 1, kADC, FHTOffset+NumOfSegFHT2+1)
  };
  for( Int_t i=0, n=id.size(); i<n; ++i ){
    c1->cd(i+1);
    TH1 *h = GHist::get(id[i]);
    if( !h ) continue;
    h->GetXaxis()->SetRangeUser(-20, 100);
    h->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BeamProfileX( ParticleType p )
{
  TCanvas *c1 = new TCanvas(__func__+ParticleName[p],
			    __func__+ParticleName[p]);
  c1->Divide( 5, 2 );
  Int_t id = HistMaker::getUniqueID( kMisc, p, kHitPat);
  for( Int_t i=0; i<NProfile*2; ++i ){
    c1->cd(i+1);
    TH1 *h = GHist::get( id + i );
    if( !h ) continue;
    h->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BeamProfileY( ParticleType p )
{
  TCanvas *c1 = new TCanvas(__func__+ParticleName[p],
			    __func__+ParticleName[p]);
  c1->Divide( 5, 2 );
  Int_t id = HistMaker::getUniqueID( kMisc, p, kHitPat );
  for( Int_t i=0; i<NProfile*2; ++i ){
    c1->cd( i + 1 );
    TH1 *h = GHist::get( id + i + NProfile*2 );
    if( !h ) continue;
    h->Draw();
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BeamProfileXY( ParticleType p )
{
  TCanvas *c1 = new TCanvas(__func__+ParticleName[p],
			    __func__+ParticleName[p]);
  c1->Divide( 3, 2 );
  Int_t id = HistMaker::getUniqueID( kMisc, p, kHitPat );
  for( Int_t i=0; i<NProfile; ++i ){
    c1->cd( i + 1 );
    TH1 *h = GHist::get( id + i + NProfile*4 );
    if( !h ) continue;
    h->Draw("colz");
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BeamProfileFF( void )
{
  TCanvas *c1 = new TCanvas(__func__, __func__);
  c1->Divide( 2, 2 );
  for( ParticleType ip : { kAll, kKaon, kPion } ){
    c1->cd( 1 );
    Int_t id = HistMaker::getUniqueID( kMisc, ip, kHitPat );
    TH1 *h = GHist::get( id + FF0 );
    if( !h ) continue;
    h->Draw( ip == kAll ? "" : "same" );
    c1->cd( 2 );
    h = GHist::get( id + FF0 + NProfile*2 );
    if( !h ) continue;
    h->Draw( ip == kAll ? "" : "same" );
    c1->cd( 3 );
    h = GHist::get( id + FF0 + NProfile*4 );
    if( !h ) continue;
    h->Draw( ip == kAll ? "box" : "box same" );
    c1->cd( 4 );
    h = GHist::get( id + NProfile*5 );
    if( !h ) continue;
    h->Draw( ip == kAll ? "box" : "box same" );
  }
  return c1;
}

//_____________________________________________________________________________
TCanvas*
BeamEnvelope( ParticleType p )
{
  TCanvas* c1 = new TCanvas(__func__, __func__);
  c1->Divide( 2, 2 );
  return c1;
}

//_____________________________________________________________________________
void
UpdateBcOutEfficiency( void )
{
  static auto c1 = dynamic_cast<TCanvas*>( gROOT->FindObject("BcOutEfficiency") );
  if( !c1 )
    return;
  static const std::vector<TString> name = {
    "BC3_Multi_x0", "BC3_Multi_x1", "BC3_Multi_v0", "BC3_Multi_v1",
    "BC3_Multi_u0", "BC3_Multi_u1", "BC4_Multi_u0", "BC4_Multi_u1",
    "BC4_Multi_v0", "BC4_Multi_v1", "BC4_Multi_x0", "BC4_Multi_x1"
  };
  static std::vector<TLatex*> tex( name.size()*2 );
  for( Int_t i=0, n=name.size(); i<n; ++i ){
    c1->cd( i+1 );
    auto h1 = dynamic_cast<TH1*>( gPad->FindObject( name[i]+"_wTDC" ) );
    TString n2 = name[i]+"_wTDC";
    n2.ReplaceAll( "_Multi", "_CMulti" );
    auto h2 = dynamic_cast<TH1*>( gPad->FindObject( n2 ) );
    if( !h1 || !h2 )
      continue;
    Double_t zero1 = h1->GetBinContent( 1 );
    Double_t all1  = h1->GetEntries();
    Double_t eff1  = 1. - zero1/all1;
    Double_t zero2 = h2->GetBinContent( 1 );
    Double_t all2  = h2->GetEntries();
    Double_t eff2  = 1. - zero2/all2;
    if( tex[2*i] ) delete tex[2*i];
    if( tex[2*i+1] ) delete tex[2*i+1];
    tex[2*i] = new TLatex;
    tex[2*i]->SetNDC();
    tex[2*i]->SetTextAlign( 32 );
    tex[2*i]->SetTextSize( 0.150 );
    tex[2*i+1] = dynamic_cast<TLatex*>( tex[2*i]->Clone() );
    tex[2*i]->SetText( 0.770, 0.600, Form( "eff. %.3f", eff1 ) );
    tex[2*i+1]->SetText( 0.770, 0.500, Form( "#color[%d]{%.3f}", kRed+1, eff2 ) );
    tex[2*i]->Draw();
    tex[2*i+1]->Draw();
  }
}

//_____________________________________________________________________________
void
UpdateSdcInOutEfficiency( void )
{
  static auto c1 = dynamic_cast<TCanvas*>( gROOT->FindObject("SdcInOutEfficiency") );
  if( !c1 )
    return;
  static std::vector<TString> name = {
    "SDC1_Multi_v0", "SDC1_Multi_v1", "SDC1_Multi_x0", "SDC1_Multi_x1",
    "SDC1_Multi_u0", "SDC1_Multi_u1",
    "SDC2_Multi_x0", "SDC2_Multi_x1", "SDC2_Multi_y0", "SDC2_Multi_y1",
    "SDC3_Multi_x0", "SDC3_Multi_x1", "SDC3_Multi_y0", "SDC3_Multi_y1",
    "SDC4_Multi_y0", "SDC4_Multi_y1", "SDC4_Multi_x0", "SDC4_Multi_x1"
  };
  static std::vector<TLatex*> tex( name.size()*2 );
  for( Int_t i=0, n=name.size(); i<n; ++i ){
    if( i< NumOfLayersSDC1 )
      c1->cd( 1 )->cd( i+1 );
    else if( i< NumOfLayersSDC1+NumOfLayersSDC2 )
      c1->cd( 2 )->cd( i+1-NumOfLayersSDC1 );
    else if( i< NumOfLayersSDC1+NumOfLayersSDC2+NumOfLayersSDC3 )
      c1->cd( 3 )->cd( i+1-NumOfLayersSDC1-NumOfLayersSDC2 );
    else if( i< NumOfLayersSDC1+NumOfLayersSDC2+NumOfLayersSDC3+NumOfLayersSDC4 )
      c1->cd( 4 )->cd( i+1-NumOfLayersSDC1-NumOfLayersSDC2-NumOfLayersSDC3 );
    auto h1 = dynamic_cast<TH1*>( gPad->FindObject( name[i]+"_wTDC" ) );
    TString n2 = name[i]+"_wTDC";
    n2.ReplaceAll( "_Multi", "_CMulti" );
    auto h2 = dynamic_cast<TH1*>( gPad->FindObject( n2 ) );
    if( !h1 || !h2 )
      continue;
    Double_t zero1 = h1->GetBinContent( 1 );
    Double_t all1  = h1->GetEntries();
    Double_t eff1  = 1. - zero1/all1;
    Double_t zero2 = h2->GetBinContent( 1 );
    Double_t all2  = h2->GetEntries();
    Double_t eff2  = 1. - zero2/all2;
    if( tex[2*i] ) delete tex[2*i];
    if( tex[2*i+1] ) delete tex[2*i+1];
    tex[2*i] = new TLatex;
    tex[2*i]->SetNDC();
    tex[2*i]->SetTextAlign( 32 );
    tex[2*i]->SetTextSize( 0.150 );
    tex[2*i+1] = dynamic_cast<TLatex*>( tex[2*i]->Clone() );
    tex[2*i]->SetText( 0.770, 0.600, Form( "eff. %.3f", eff1 ) );
    tex[2*i+1]->SetText( 0.770, 0.500, Form("#color[%d]{%.3f}", kRed+1, eff2 ) );
    tex[2*i]->Draw();
    tex[2*i+1]->Draw();
  }
}

//_____________________________________________________________________________
void
UpdateCFTEfficiency( void )
{
  static TCanvas *c1 = (TCanvas*)gROOT->FindObject("CFTEfficiency");
  if( !c1 )
    return;
  static std::vector<TString> name = {
    "CFT_CMulti_UV1", "CFT_CMulti_PHI1", "CFT_CMulti_UV2", "CFT_CMulti_PHI2",
    "CFT_CMulti_UV3", "CFT_CMulti_PHI3", "CFT_CMulti_UV4", "CFT_CMulti_PHI4",
    "CFT_CMulti_UV1_wBGO", "CFT_CMulti_PHI1_wBGO",
    "CFT_CMulti_UV2_wBGO", "CFT_CMulti_PHI2_wBGO",
    "CFT_CMulti_UV3_wBGO", "CFT_CMulti_PHI3_wBGO",
    "CFT_CMulti_UV4_wBGO", "CFT_CMulti_PHI4_wBGO"
  };
  static std::vector<TText*> tex(name.size());

  for( Int_t i=0, n=name.size(); i<n; ++i ){
    c1->cd(i+1);
    TH1 *h = (TH1*)gPad->FindObject(name[i]);
    if( !h ){ std::cout << "!?!?!?" << std::endl; continue; }
    Double_t zero = h->GetBinContent(1);
    Double_t all  = h->GetEntries();
    Double_t eff  = 1. - zero/all;
    if( tex[i] ) delete tex[i];
    tex[i] = new TText;
    tex[i]->SetNDC();
    tex[i]->SetTextSize(0.130);
    tex[i]->SetText(0.300,0.600,Form("eff. %.3f",
				     eff));
    tex[i]->Draw();
  }
}

//_____________________________________________________________________________
void
UpdateSSDEfficiency( void )
{
  static TCanvas *c1 = (TCanvas*)gROOT->FindObject("SSDEfficiency");
  if( !c1 )
    return;
  static std::vector<TString> name = {
    "SSD1_CMulti_y0", "SSD1_CMulti_x0", "SSD1_CMulti_y1", "SSD1_CMulti_x1",
    "SSD2_CMulti_x0", "SSD2_CMulti_y0", "SSD2_CMulti_x1", "SSD2_CMulti_y1"
  };
  static std::vector<TText*> tex(name.size());
  for( Int_t i=0, n=name.size(); i<n; ++i ){
    c1->cd(i+1);
    TH1 *h = (TH1*)gPad->FindObject(name[i]);
    if( !h ) continue;
    Double_t zero = h->GetBinContent(1);
    Double_t all  = h->GetEntries();
    Double_t eff  = 1. - zero/all;
    if( tex[i] ) delete tex[i];
    tex[i] = new TText;
    tex[i]->SetNDC();
    tex[i]->SetTextSize(0.100);
    tex[i]->SetText(0.500,0.600,Form("eff. %.3f",
				     eff));
    tex[i]->Draw();
  }
}

//_____________________________________________________________________________
void
UpdateT0PeakFitting( void )
{
  {
    static auto c1 = dynamic_cast<TCanvas*>( gROOT->FindObject( "T0" ) );
    if( !c1 )
      return;
    static std::vector<TText*> tex( NumOfSegBH2*2 );
    static const auto xmin = gUser.GetParameter( "BH2_TDC", 0 );
    static const auto xmax = gUser.GetParameter( "BH2_TDC", 1 );
    for( Int_t i=0; i<NumOfSegBH2; ++i ){
      c1->cd(i+1);
      auto h = dynamic_cast<TH1*>( gPad->FindObject( Form( "BH2_TDC_%dU", i+1 ) ) );
      if( !h || h->GetEntries() == 0 ) continue;
      TF1 f( "f", "gaus", xmin, xmax );
      Double_t p = h->GetBinCenter( h->GetMaximumBin() );
      Double_t w = 500.;
      for( Int_t ifit=0; ifit<3; ++ifit ){
	h->Fit( "f", "Q", "", p-2*w, p+2*w );
	p = f.GetParameter( 1 );
	w = f.GetParameter( 2 );
      }
      if( tex[2*i] ) delete tex[2*i];
      if( tex[2*i+1] ) delete tex[2*i+1];
      tex[2*i] = new TLatex;
      tex[2*i]->SetNDC();
      tex[2*i]->SetTextSize( 0.100 );
      tex[2*i+1] = dynamic_cast<TLatex*>( tex[2*i]->Clone() );
      tex[2*i]->SetText( 0.200, 0.750, Form( "%.0f", p ) );
      tex[2*i+1]->SetText( 0.200, 0.650, Form( "%.0f", w ) );
      tex[2*i]->Draw();
      tex[2*i+1]->Draw();
    }
  }
}

//_____________________________________________________________________________
void
UpdateTOTPeakFitting( void )
{
  UpdateTOTPeakFittingE03();
  // UpdateTOTPeakFittingE40();
}

//_____________________________________________________________________________
void
UpdateTOTPeakFittingE03( void )
{
  static auto c1 = dynamic_cast<TCanvas*>( gROOT->FindObject("BFTSCHTOT") );
  if( !c1 )
    return;
  static std::vector<TString> name = {
    "BFT_CTOT_U", "BFT_CTOT_D", "SCH_TOT_ALL",
  };
  static std::vector<Double_t> optval = {
    gUser.GetParameter( "BFT_TOTREF" ),
    gUser.GetParameter( "BFT_TOTREF" ),
    gUser.GetParameter( "SCH_TOTREF" ),
  };
  static std::vector<TLine*> line( name.size() );
  static std::vector<TText*> tex( name.size() );
  for( Int_t i=0, n=name.size(); i<n; ++i ){
    c1->cd( i+1 );
    auto h = dynamic_cast<TH1*>( gPad->FindObject( name[i] ) );
    if( !h ) continue;
    TF1 f( "f", "gaus", 0., 100. );
    Double_t p = h->GetBinCenter( h->GetMaximumBin() );
    if( p < 30. ) p = 60.;
    Double_t w = 10.;
    for( Int_t ifit=0; ifit<3; ++ifit ){
      Double_t fmin = p - w;
      Double_t fmax = p + w;
      h->Fit( "f", "Q", "", fmin, fmax );
      p = f.GetParameter( 1 );
      w = f.GetParameter( 2 ) * 1.;
    }
    if( tex[i] ) delete tex[i];
    tex[i] = new TText;
    tex[i]->SetNDC();
    tex[i]->SetTextSize( 0.100 );
    tex[i]->SetText( 0.200, 0.750, Form( "%.2f", p ) );
    tex[i]->Draw();
    if( line[i] ) delete line[i];
    line[i] = new TLine( optval[i], 0., optval[i], h->GetMaximum() );
    line[i]->Draw();
  }
}

//_____________________________________________________________________________
void
UpdateTOTPeakFittingE40( void )
{
  {
    static TCanvas *c1 = (TCanvas*)gROOT->FindObject("BFTSFTSCHTOT");
    if( !c1 )
      return;
    static std::vector<TString> name = {
      "BFT_CTOT_U", "BFT_CTOT_D", "SFT_CTOT_U", "SFT_CTOT_V",
      "SFT_CTOT_X", "SFT_CTOT_XP", "SCH_TOT (1-16ch)", "SCH_TOT (17-64ch)"
    };
    static std::vector<Double_t> optval = {
      41.0, 41.0, 65.0, 65.0,
      67.5, 67.5, 42.0, 54.0
    };
    static std::vector<TLine*> line(name.size());
    static std::vector<TText*> tex(name.size());
    for( Int_t i=0, n=name.size(); i<n; ++i ){
      c1->cd(i+1);
      TH1 *h = (TH1*)gPad->FindObject(name[i]);
      if( !h ) continue;
      TF1 f("f", "gaus", 0., 100.);
      Double_t p = h->GetBinCenter(h->GetMaximumBin());
      if( p < 30. ) p = 60.;
      Double_t w = 10.;
      for( Int_t ifit=0; ifit<3; ++ifit ){
	Double_t fmin = p - w;
	Double_t fmax = p + w;
	if( name[i].Contains("SFT_CTOT") && ifit>0 )
	  fmin = p - w*0.75;
	h->Fit("f", "Q", "", fmin, fmax);
	p = f.GetParameter(1);
	w = f.GetParameter(2) * 1.;
      }
      if( tex[i] ) delete tex[i];
      tex[i] = new TText;
      tex[i]->SetNDC();
      tex[i]->SetTextSize(0.100);
      tex[i]->SetText( 0.200, 0.750, Form("%.2f", p));
      tex[i]->Draw();
      if( line[i] ) delete line[i];
      line[i] = new TLine(optval[i], 0., optval[i], h->GetMaximum());
      line[i]->Draw();
    }
  }
  {
    static TCanvas *c1 = (TCanvas*)gROOT->FindObject("FHTTOT");
    static std::vector<TString> name = {
      "FHT1_UX1_TOT", "FHT1_DX1_TOT", "FHT1_UX2_TOT", "FHT1_DX2_TOT",
      "FHT2_UX1_TOT", "FHT2_DX1_TOT", "FHT2_UX2_TOT", "FHT2_DX2_TOT"
    };
    static std::vector<Double_t> optval = {
      58.0, 60.0, 58.0, 60.0,
      60.0, 60.0, 60.0, 60.0
    };
    static std::vector<TLine*> line(name.size());
    static std::vector<TText*> tex(name.size());
    for( Int_t i=0, n=name.size(); i<n; ++i ){
      c1->cd(i+1);
      TH1 *h = (TH1*)gPad->FindObject(name[i]);
      if( !h ) continue;
      TF1 f("f", "gaus", 0., 100.);
      Double_t p = h->GetBinCenter(h->GetMaximumBin());
      if( p < 40. ) p = 60.;
      Double_t w = 10.;
      for( Int_t ifit=0; ifit<3; ++ifit ){
	h->Fit("f", "Q", "", p-w, p+w );
	p = f.GetParameter(1);
	w = f.GetParameter(2) * 1.;
      }
      if( tex[i] ) delete tex[i];
      tex[i] = new TText;
      tex[i]->SetNDC();
      tex[i]->SetTextSize(0.100);
      tex[i]->SetText( 0.200, 0.750, Form("%.2f", p));
      tex[i]->Draw();
      if( line[i] ) delete line[i];
      line[i] = new TLine(optval[i], 0., optval[i], h->GetMaximum());
      line[i]->Draw();
    }
  }
}

//_____________________________________________________________________________
void
UpdateBeamProfile( ParticleType p )
{
  static std::vector<TLatex*> title1_raw( NParticleType*200 + NProfile*2 );
  static std::vector<TLatex*> title2_raw( NParticleType*200 + NProfile*2 );
  static std::vector<TLatex*> title1_fit( NParticleType*200 + NProfile*2 );
  static std::vector<TLatex*> title2_fit( NParticleType*200 + NProfile*2 );
  static std::vector<TLatex*> tex1_raw( NParticleType*200 + NProfile*2 );
  static std::vector<TLatex*> tex2_raw( NParticleType*200 + NProfile*2 );
  static std::vector<TLatex*> tex1_fit( NParticleType*200 + NProfile*2 );
  static std::vector<TLatex*> tex2_fit( NParticleType*200 + NProfile*2 );
  {
    TCanvas *c1 = (TCanvas*)gROOT->FindObject("BeamProfileX" +
					      ParticleName[p]);
    for( Int_t i=0; i<NProfile; ++i ){
      // raw
      c1->cd( i + 1 );
      TString n1 = Form("BeamProfile FF %d X (%s)",
			(Int_t)Profiles[i], ParticleName[p].Data());
      TH1 *h1 = (TH1*)gPad->FindObject( n1 );
      Int_t it = p*200 + i;
      if( !h1 ) continue;
      if( !title1_raw[it] ){
	title1_raw[it] = new TLatex(0.180, 0.830, "Mean");
	SetText( title1_raw[it], 12, 0.060 );
	title1_raw[it]->Draw();
      }
      if( !title2_raw[it] ){
	title2_raw[it] = new TLatex(0.180, 0.700, "StdDev");
	SetText( title2_raw[it], 12, 0.060 );
	title2_raw[it]->Draw();
      }
      Double_t p1 = h1->GetMean();
      Double_t p2  = h1->GetStdDev();
      if (p == kAll) {
        envelope_x_mean[i] = p1;
        envelope_x_rms[i] = p2;
      }
      if( tex1_raw[it] ) delete tex1_raw[it];
      tex1_raw[it] = new TLatex(0.400, 0.770, Form("%.1f", p1));
      SetText( tex1_raw[it], 32, 0.080 );
      tex1_raw[it]->Draw();
      if( tex2_raw[it] ) delete tex2_raw[it];
      tex2_raw[it] = new TLatex(0.400, 0.640, Form("%.1f", p2));
      SetText( tex2_raw[it], 32, 0.080 );
      tex2_raw[it]->Draw();
      // fit
      c1->cd( i + 1 + NProfile );
      TString n2 = Form("BeamProfile FF %d X Fit (%s)",
			(Int_t)Profiles[i], ParticleName[p].Data());
      TH1 *h2 = (TH1*)gPad->FindObject( n2 );
      if( !h2 ) continue;
      if( !title1_fit[it] ){
	title1_fit[it] = new TLatex(0.180, 0.830, "Mean");
	SetText( title1_fit[it], 12, 0.060 );
	title1_fit[it]->Draw();
      }
      if( !title2_fit[it] ){
	title2_fit[it] = new TLatex(0.180, 0.700, "Sigma");
	SetText( title2_fit[it], 12, 0.060 );
	title2_fit[it]->Draw();
      }
      if( h2->GetEntries() == 0 )
	continue;
      TF1 f("f", "gaus");
      p1 = h2->GetBinCenter(h2->GetMaximumBin());
      p2 = h2->GetStdDev();
      for( Int_t ifit=0; ifit<3; ++ifit ){
	h2->Fit("f", "Q", "", p1-2*p2, p1+2*p2 );
	p1 = f.GetParameter(1);
	p2 = f.GetParameter(2);
      }
      if (p == kAll) {
        envelope_xfit_mean[i] = p1;
        envelope_xfit_sigma[i] = p2;
      }
      if( tex1_fit[it] ) delete tex1_fit[it];
      tex1_fit[it] = new TLatex(0.400, 0.770, Form("%.1f", p1));
      SetText( tex1_fit[it], 32, 0.080 );
      tex1_fit[it]->Draw();
      if( tex2_fit[it] ) delete tex2_fit[it];
      tex2_fit[it] = new TLatex(0.400, 0.640, Form("%.1f", p2));
      SetText( tex2_fit[it], 32, 0.080 );
      tex2_fit[it]->Draw();
    }
  }
  {
    TCanvas *c1 = (TCanvas*)gROOT->FindObject("BeamProfileY" +
					      ParticleName[p]);
    for( Int_t i=0; i<NProfile; ++i ){
      // raw
      c1->cd( i + 1 );
      TString n1 = Form("BeamProfile FF %d Y (%s)",
			(Int_t)Profiles[i], ParticleName[p].Data());
      TH1 *h1 = (TH1*)gPad->FindObject( n1 );
      Int_t it = p*200 + 100 + i;
      if( !h1 ) continue;
      if( !title1_raw[it] ){
	title1_raw[it] = new TLatex(0.180, 0.830, "Mean");
	SetText( title1_raw[it], 12, 0.060 );
	title1_raw[it]->Draw();
      }
      if( !title2_raw[it] ){
	title2_raw[it] = new TLatex(0.180, 0.700, "StdDev");
	SetText( title2_raw[it], 12, 0.060 );
	title2_raw[it]->Draw();
      }
      Double_t p1 = h1->GetMean();
      Double_t p2  = h1->GetStdDev();
      if (p == kAll) {
        envelope_y_mean[i] = p1;
        envelope_y_rms[i] = p2;
      }
      if( tex1_raw[it] ) delete tex1_raw[it];
      tex1_raw[it] = new TLatex(0.440, 0.770, Form("%.1f", p1));
      SetText( tex1_raw[it], 32, 0.080 );
      tex1_raw[it]->Draw();
      if( tex2_raw[it] ) delete tex2_raw[it];
      tex2_raw[it] = new TLatex(0.440, 0.640, Form("%.1f", p2));
      SetText( tex2_raw[it], 32, 0.080 );
      tex2_raw[it]->Draw();
      // fit
      c1->cd( i + 1 + NProfile );
      TString n2 = Form("BeamProfile FF %d Y Fit (%s)",
			(Int_t)Profiles[i], ParticleName[p].Data());
      TH1 *h2 = (TH1*)gPad->FindObject( n2 );
      if( !h2 ) continue;
      if( !title1_fit[it] ){
	title1_fit[it] = new TLatex(0.180, 0.830, "Mean");
	SetText( title1_fit[it], 12, 0.060 );
	title1_fit[it]->Draw();
      }
      if( !title2_fit[it] ){
	title2_fit[it] = new TLatex(0.180, 0.700, "Sigma");
	SetText( title2_fit[it], 12, 0.060 );
	title2_fit[it]->Draw();
      }
      if( h2->GetEntries() == 0 )
	continue;
      TF1 f("f", "gaus");
      p1 = h2->GetBinCenter(h2->GetMaximumBin());
      p2 = h2->GetStdDev();
      for( Int_t ifit=0; ifit<3; ++ifit ){
	h2->Fit("f", "Q", "", p1-2*p2, p1+2*p2 );
	p1 = f.GetParameter(1);
	p2 = f.GetParameter(2);
      }
      if (p == kAll) {
        envelope_yfit_mean[i] = p1;
        envelope_yfit_sigma[i] = p2;
      }
      if( tex1_fit[it] ) delete tex1_fit[it];
      tex1_fit[it] = new TLatex(0.440, 0.770, Form("%.1f", p1));
      SetText( tex1_fit[it], 32, 0.080 );
      tex1_fit[it]->Draw();
      if( tex2_fit[it] ) delete tex2_fit[it];
      tex2_fit[it] = new TLatex(0.440, 0.640, Form("%.1f", p2));
      SetText( tex2_fit[it], 32, 0.080 );
      tex2_fit[it]->Draw();
    }
  }
}

} // namespace http

} // namespace analyzer
