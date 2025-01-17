// -*- C++ -*-

#ifndef MACRO_BUILDER_HH
#define MACRO_BUILDER_HH

#include "HistMaker.hh"

class TCanvas;
class TObject;
class TString;

namespace analyzer
{

//______________________________________________________________________________
namespace macro
{
TObject* Get(TString name);
}

//______________________________________________________________________________
// for HttpServer
namespace http
{
const std::vector<Double_t>& GetEnvelopeXMean();
const std::vector<Double_t>& GetEnvelopeXfitMean();
const std::vector<Double_t>& GetEnvelopeXRMS();
const std::vector<Double_t>& GetEnvelopeXfitSigma();
const std::vector<Double_t>& GetEnvelopeYMean();
const std::vector<Double_t>& GetEnvelopeYfitMean();
const std::vector<Double_t>& GetEnvelopeYRMS();
const std::vector<Double_t>& GetEnvelopeYfitSigma();

TCanvas* Correlation();
TCanvas* CorrelationFHT();
TCanvas* HitPattern();
TCanvas* HitPatternBeam();
TCanvas* HitPatternScat();
TCanvas* TriggerFlagU();
TCanvas* TriggerFlagD();
TCanvas* TriggerFlagHitPat();
TCanvas* BH1ADC();
TCanvas* BH1TDC();
TCanvas* BFT();
TCanvas* BH2ADC();
TCanvas* BH2TDC();
TCanvas* HTOFADCU();
TCanvas* HTOFADCD();
TCanvas* HTOFTDCU();
TCanvas* HTOFTDCD();
TCanvas* BAC();
TCanvas* ACs();
TCanvas* BeamHitMulti(void);
TCanvas* SCHTDCU();
TCanvas* SCHTDCD();
TCanvas* SCHTOTU();
TCanvas* SCHTOTD();
TCanvas* SCHHitMulti();
TCanvas* TOFADCU();
TCanvas* TOFADCD();
TCanvas* TOFTDCU();
TCanvas* TOFTDCD();
TCanvas* BVHTDC();
TCanvas* LACTDC();
TCanvas* AC1TDC();
TCanvas* WCADCU();
TCanvas* WCADCD();
TCanvas* WCADCSUM();
TCanvas* WCTDCU();
TCanvas* WCTDCD();
TCanvas* WCTDCSUM();
TCanvas* ScatHitMulti(void);
TCanvas* MsTTDC();
TCanvas* MsTHitPat();
TCanvas* BC3TDCTOT();
TCanvas* BC3HitMulti();
TCanvas* BC4TDCTOT();
TCanvas* BC4HitMulti();
TCanvas* SDC1TDCTOT();
TCanvas* SDC1HitMulti();
TCanvas* SDC2TDCTOT();
TCanvas* SDC2HitMulti();
TCanvas* SDC3TDCTOT();
TCanvas* SDC3HitMulti();
TCanvas* SDC4TDCTOT();
TCanvas* SDC4HitMulti();
TCanvas* SDC5TDCTOT();
TCanvas* SDC5HitMulti();
TCanvas* T0();
TCanvas* BcOutEfficiency();
TCanvas* SdcInOutEfficiency();
TCanvas* BFTTOT();
TCanvas* BFTAFTTOT();
TCanvas* BFTSCHTOT();
TCanvas* BFTSFTSCHTOT();
TCanvas* DAQ();
TCanvas* BcOutSdcInMultiHit();
void     UpdateTOTPeakFitting();

// BeamProfile
// TCanvas* BeamProfileX(ParticleType p);
// TCanvas* BeamProfileY(ParticleType p);
// TCanvas* BeamProfileXY(ParticleType p);
// TCanvas* BeamProfileFF();
// TCanvas* BeamEnvelope(ParticleType p);

// E42
TCanvas* TPC();
TCanvas* TPC2D();
TCanvas* TPC3D();
TCanvas* TPCADCPAD();
TCanvas* TPCTDCPAD();
TCanvas* TPCHTOFPAD();
TCanvas* TPCFADC();

// E03
TCanvas* GeADC();
TCanvas* GeCRM();
TCanvas* GeTFA();
TCanvas* GeRST();
TCanvas* GeMultiCRM();
TCanvas* GeMultiTFA();
TCanvas* GeTFACRM();
TCanvas* GeTFAADC();
TCanvas* GeRSTADC();
TCanvas* BGOTDC();
TCanvas* HitPatternE03();
void     UpdateTOTPeakFittingE03();

// E07
TCanvas* HitPatternE07();
TCanvas* SSD1ADCTDC();
TCanvas* SSD2ADCTDC();
TCanvas* SSD1HitMulti();
TCanvas* SSD2HitMulti();
TCanvas* SSD1CHitMulti();
TCanvas* SSD2CHitMulti();
TCanvas* SSD1dETime();
TCanvas* SSD2dETime();
TCanvas* SSD12Chisqr();
TCanvas* SSDEfficiency();
TCanvas* EMC();
TCanvas* FBHTDC();
TCanvas* FBHTOT();
TCanvas* FBHHitMulti();

// E40
TCanvas* FHTTOT();
TCanvas* SAC();
TCanvas* LC();
TCanvas* TOF_HT();
TCanvas* FHT1TDC();
TCanvas* FHT1TOT();
TCanvas* FHT1HitMulti();
TCanvas* FHT2TDC();
TCanvas* FHT2TOT();
TCanvas* FHT2HitMulti();
TCanvas* SFTTDCTOT();
TCanvas* SFTHitMulti();
TCanvas* SFT2D();
TCanvas* HitPatternE40();
TCanvas* SFT();
TCanvas* FHT1TDC();
TCanvas* FHT1TOT();
TCanvas* FHT1HitMulti();
TCanvas* FHT2TDC();
TCanvas* FHT2TOT();
TCanvas* FHT2HitMulti();
TCanvas* CFTTDC();
TCanvas* CFTTDC2D();
TCanvas* CFTTOT();
TCanvas* CFTTOT2D();
TCanvas* CFTHighGain();
TCanvas* CFTHighGain2D();
TCanvas* CFTLowGain();
TCanvas* CFTLowGain2D();
TCanvas* CFTPedestal();
TCanvas* CFTPedestal2D();
TCanvas* CFTHitPat();
TCanvas* CFTMulti();
TCanvas* CFTEfficiency();
TCanvas* CFTClusterHighGain();
TCanvas* CFTClusterHighGain2D();
TCanvas* CFTClusterLowGain();
TCanvas* CFTClusterLowGain2D();
TCanvas* CFTClusterTDC();
TCanvas* CFTClusterTDC2D();
// TCanvas* BGOFADC();
// TCanvas* BGOADC();
// TCanvas* BGOTDC();
// TCanvas* BGOADCTDC2D();
// TCanvas* BGOHitMulti();
TCanvas* PiIDTDC();
TCanvas* PiIDHighGain();
TCanvas* PiIDLowGain();
TCanvas* PiIDHitMulti();

// E70
TCanvas* AFTTDCU();
TCanvas* AFTTDCD();
TCanvas* AFTTOTU();
TCanvas* AFTTOTD();
TCanvas* AFTHGU();
TCanvas* AFTHGD();
TCanvas* AFTLGU();
TCanvas* AFTLGD();
TCanvas* AFTTDCU2D();
TCanvas* AFTTDCD2D();
TCanvas* AFTTOTU2D();
TCanvas* AFTTOTD2D();
TCanvas* AFTHGU2D();
TCanvas* AFTHGD2D();
TCanvas* AFTLGU2D();
TCanvas* AFTLGD2D();
TCanvas* AFTHitPatX();
TCanvas* AFTHitPatY();
TCanvas* AFTMulti();
TCanvas* AFTEfficiency();
void     UpdateTOTPeakFittingE70();

void     UpdateBcOutEfficiency();
void     UpdateSdcInOutEfficiency();
void     UpdateCFTEfficiency();
void     UpdateAFTEfficiency();
void     UpdateSSDEfficiency();
void     UpdateT0PeakFitting();
void     UpdateTOTPeakFittingE40();
  //void     UpdateBeamProfile(ParticleType p);
}

};

#endif
