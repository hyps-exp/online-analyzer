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
TCanvas* SDCIn_TDC();
TCanvas* SDCIn_TDC1st();
TCanvas* SDCIn_TOT();
TCanvas* SDCIn_HitPat();
TCanvas* SDCIn_Multi();
TCanvas* SDCIn_TDC2D();
TCanvas* SDCIn_TDC2DC();
TCanvas* SDCOut_TDC();
TCanvas* SDCOut_TDC1st();
TCanvas* SDCOut_TOT();
TCanvas* SDCOut_HitPat();
TCanvas* SDCOut_Multi();
TCanvas* SDCOut_TDC2D();
TCanvas* SDCOut_TDC2DC();
TCanvas* SDC_Correlation();
TCanvas* TOF_ADCU1();
TCanvas* TOF_ADCU2();
TCanvas* TOF_ADCD1();
TCanvas* TOF_ADCD2();
TCanvas* DAQ();
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
}

};

#endif
