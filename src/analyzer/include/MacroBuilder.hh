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


TCanvas* Counter_TDC();
TCanvas* TAG_SFF_TDC1();
TCanvas* TAG_SFF_TDC2();
TCanvas* TAG_SFF_TDC3();
TCanvas* TAG_SFF_TDC4();
TCanvas* TAG_SFF_TDC5();
TCanvas* TAG_SFB_TDC1();
TCanvas* TAG_SFB_TDC2();
TCanvas* TAG_SFB_TDC3();
TCanvas* TAG_SFB_TDC4();
TCanvas* TAG_SFB_TDC5();
TCanvas* TAG_PL_TDC();
TCanvas* TAG_PL_ADC();
TCanvas* TAG_Multi();
TCanvas* U_Veto();
TCanvas* T0();
TCanvas* SAC();
TCanvas* SDCIn_TDC();
TCanvas* SDCIn_TDC1st();
TCanvas* SDCIn_TOT();
TCanvas* SDCIn_HitPat();
TCanvas* SDCIn_Multi();
TCanvas* SDCIn_TDC2D();
TCanvas* SDCIn_TDC2DC();
TCanvas* SDCIn_TOTTDC2D();
TCanvas* SDCOut_TDC();
TCanvas* SDCOut_TDC1st();
TCanvas* SDCOut_TOT();
TCanvas* SDCOut_HitPat();
TCanvas* SDCOut_Multi();
TCanvas* SDCOut_TDC2D();
TCanvas* SDCOut_TDC2DC();
TCanvas* SDCOut_TOTTDC2D();
TCanvas* SDC_Correlation();
TCanvas* E_Veto();
TCanvas* TOF_ADCU1();
TCanvas* TOF_ADCU2();
TCanvas* TOF_ADCU3();
TCanvas* TOF_ADCD1();
TCanvas* TOF_ADCD2();
TCanvas* TOF_ADCD3();
TCanvas* TOF_TDCU1();
TCanvas* TOF_TDCU2();
TCanvas* TOF_TDCU3();
TCanvas* TOF_TDCD1();
TCanvas* TOF_TDCD2();
TCanvas* TOF_TDCD3();
TCanvas* DAQ();
TCanvas* CFTTDC();
TCanvas* CFTTDC2D();
TCanvas* CFTTOT();
TCanvas* CFTTOT2D();
TCanvas* CFTCTOT2D();
TCanvas* CFTHighGain();
TCanvas* CFTLowGain();
TCanvas* CFTPedestal();
TCanvas* CFTHighGain2D();
TCanvas* CFTLowGain2D();
TCanvas* CFTPedestal2D();
TCanvas* CFTCHighGain2D();
TCanvas* CFTCLowGain2D();
TCanvas* CFTHighGainxTOT();
TCanvas* CFTLowGainxTOT();
TCanvas* CFTHitPat();
TCanvas* CFTMulti();
TCanvas* CFTClusterHighGain();
TCanvas* CFTClusterLowGain();
TCanvas* CFTClusterHighGain2D();
TCanvas* CFTClusterLowGain2D();
TCanvas* CFTClusterTDC();
TCanvas* CFTClusterTDC2D();
// TCanvas* CFTEfficiency();
TCanvas* BGOFADC();
TCanvas* BGOFADCwTDC();
TCanvas* BGOADC();
TCanvas* BGOTDC();
TCanvas* BGOADCTDC2D();
TCanvas* BGOHitMulti();
TCanvas* PiIDTDC();
TCanvas* PiIDHighGain();
TCanvas* PiIDLowGain();
TCanvas* PiIDADC2DHitMulti();
TCanvas* Correlation_CATCH();
}

};

#endif
