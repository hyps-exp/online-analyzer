// -*- C++ -*-

#ifndef HISTMAKER_HH
#define HISTMAKER_HH

#include <vector>
#include <map>
#include <bitset>
#include <string>

#include <TObject.h>
#include <TROOT.h>
#include <TString.h>

enum eUorD { kU, kD, kUorD };
enum eAorT { kA, kT, kAorT };

enum DetectorType {
  kDetectorZero,
  // Detector unique ID in the beam line
  kBH1, kBFT, kBC3, kBC4, kBH2, kBAC, kBH2MT,
  kMsT, kMsT_T0, kMtx3D,
  // Detector unique ID in the S-2S system
  kAC1, kSDC1, kSDC2, kSDC3, kSDC4, kSDC5, kTOF, kSAC3, kSFV,
  // HYPS
  kDC1,
  // VMEEASIROC unique ID
  kVMEEASIROC,
  //E70
  kAFT,
  // E72E90
  kE72BAC, kE90SAC, kE72KVC, kE42BH2, kE72Parasite,
  // TF
  kTF_TF, kTF_GN1, kTF_GN2,
  // E42
  kBH2_E42, kBH2_E42MT, kWC, kWCMT, kT1, kT1MT, kT2, kT2MT,
  kHTOF, kTPC, kBVH,
  // Old detectors in the KURAMA system
  kPVAC, kFAC, kSCH, kLAC,
  // Old detectors E07
  kFBH, kSSDT, kSSD1, kSSD2, kEMC,
  // Old detectors E40
  kCFT,kBGO,kPiID,kSFT,kFHT1,kFHT2,kLC,kSAC,kTOF_HT,
  // Old detectors
  kBMW, kBAC_SAC, kSFV_SAC3, kGe, kPWO, kSP0,
  kBH2_E07, kBAC_E07, kSSD0, kSAC1, kKFAC,
  kKIC, kHDC, kTOFMT,
  // Others
  kTriggerFlag, kDAQ, kCorrelation,
  kCorrelation_catch, kMisc,
  kTimeStamp, kDCEff,
  sizeDetectorType,
  factorDetectorType = 10000000
};

enum SubDetectorType {
  kSubDetectorZero,
  // FHT Layers
  kFHT_L1, kFHT_L2,
  //CFT, AFT cluster
  kCluster,
  // Detector unique sub ID in Counters
  kSP0_L1, kSP0_L2, kSP0_L3, kSP0_L4,
  kSP0_L5, kSP0_L6, kSP0_L7, kSP0_L8,
  // Sub Detector ID in HUL
  kHulTOF,     kHulFBH,     kHulSCH,
  kHulTOFxFBH, kHulTOFxSCH, kHulFBHxSCH,
  kHul2DHitPat, kHul3DHitPat,
  kHul2D, kHul3D, kSelfCorr,
  // DAQ unique sub ID
  kEB, kTKO, kVME, kCLite, kEASIROC, kCAMAC, kCoBo,
  kMiscNode, kHUL, kOpt, kHULOverflow,
  sizeSubDetectorType,
  factorSubDetectorType = 100000
};

enum DataType {
  kDataTypeZero,
  // Usual data type
  kADC,   kTDC,   kHitPat,   kMulti,
  kADC2D, kTDC2D, kHitPat2D, kMulti2D,
  kADCwTDC, kFADC, kFADCwTDC,
  kDeltaE, kCTime, kDeltaE2D, kCTime2D,
  kChisqr,
  // Extra data type for CFT
  kHighGain, kLowGain, kPede,
  // Extra data type for AFT
  kHighGain2D, kLowGain2D, kPede2D,
  kTOT, kTOT2D, kMultiHitTdc,
  kHighGainXTDC,  kTOTXTDC,
  kHighGainXTOT,kLowGainXTOT, kHighGainvsTOT,
  // Extra data type for EMC
  kSerial, kXpos, kYpos, kXYpos, kTime,
  // Extra data type for Ge detector
  kCRM,   kTFA,   kPUR,   kRST,
  kCRM2D, kTFA2D, kPUR2D, kRST2D,
  kTFA_CRM, kTFA_ADC, kCRM_ADC, kRST_ADC,
  kFlagTDC, kFlagHitPat,
  sizeDataType,
  factorDataType = 1000
};

enum ParticleType {
  kKaon, kPion, kAll, NParticleType
};

static const TString ParticleName[NParticleType] =
  { "Kaon", "Pion", "All" };

const double FFfromVO = 1200.;
enum EProfile { FFm600, FFm300, FF0, FFp300, FFp600, NProfile };
const Double_t Profiles[] = { -600, -300, 0, 300, 600 };

const int kTOTcutOffset      = 20;
// const int kSCH_1to16_Offset  = 64+1;
// const int kSCH_17to64_Offset = 64+2;

TString getStr_FromEnum(const char* c);

class TH1;
class TH2;
class TList;

class HistMaker : public TObject
{
  // Declaration of the private parameters ---------------------------------
  // histogram unique and sequential ID
  Int_t current_hist_id_;
  std::map<Int_t, Int_t>   idmap_seq_from_unique_;
  std::map<Int_t, Int_t>   idmap_unique_from_seq_;
  std::map<TString, Int_t> idmap_seq_from_name_;

  // data set of string which means the created detector name
  std::vector<TString>     name_created_detectors_;
  std::vector<TString>     name_ps_files_;

  // return value of insert
  typedef std::pair<std::map<Int_t, Int_t>::iterator, Bool_t> TypeRetInsert;

public:
  // Public functions ------------------------------------------------------
  ~HistMaker( void );
  static HistMaker& getInstance( void );
  static void       getListOfDetectors( std::vector<TString>& vec );
  static void       getListOfPsFiles( std::vector<TString>& vec );
  static Int_t      getNofHist( void );
  static Int_t      getUniqueID( Int_t detector_type, Int_t subdetector_type,
				 Int_t data_type, Int_t channel=1 );
  static Int_t      getUniqueID( Int_t sequential_id );
  static Int_t      getSequentialID( Int_t detector_type, Int_t subdetector_type,
				     Int_t data_type, Int_t channel=1 );
  static Int_t      getSequentialID( Int_t unique_id );
  static Int_t      getSequentialID( const TString& name );

  Int_t  setHistPtr( std::vector<TH1*>& vec );

  TH1*   createTH1( Int_t unique_id, const TString& title,
		    Int_t nbinx, Int_t xmin, Int_t xmax,
		    const TString& xtitle="", const TString& ytitle="" );
  TH2*   createTH2( Int_t unique_id, const TString& title,
		    Int_t nbinx, Int_t xmin, Int_t xmax,
		    Int_t nbiny, Int_t ymin, Int_t ymax,
		    const TString& xtitle="", const TString& ytitle="" );
  TH2*   createTH2( Int_t unique_id, const TString& title,
		    Int_t nbinx, Double_t xmin, Double_t xmax,
		    Int_t nbiny, Double_t ymin, Double_t ymax,
		    const TString& xtitle="", const TString& ytitle="" );
  TH2*   createTH2Poly( Int_t unique_id, const TString& title,
                        Double_t xmin, Double_t xmax,
                        Double_t ymin, Double_t ymax );

  TString MakeDetectorName( const TString& name );

  TList* createTimeStamp( Bool_t flag_ps=true );
  TList* createDC1( Bool_t flag_ps=true );
  TList* createBH1( Bool_t flag_ps=true );
  TList* createBFT( Bool_t flag_ps=true );
  TList* createBC3( Bool_t flag_ps=true );
  TList* createBC4( Bool_t flag_ps=true );
  TList* createBH2( Bool_t flag_ps=true );
  TList* createAFT( Bool_t flag_ps=true );
  TList* createBAC( Bool_t flag_ps=true );
  TList* createFAC( Bool_t flag_ps=true );
  TList* createPVAC( Bool_t flag_ps=true );
  TList* createVMEEASIROC( Bool_t flag_ps=true );
  TList* createSDC1( Bool_t flag_ps=true );
  TList* createSDC2( Bool_t flag_ps=true );
  TList* createSCH( Bool_t flag_ps=true );
  TList* createSDC3( Bool_t flag_ps=true );
  TList* createSDC4( Bool_t flag_ps=true );
  TList* createSDC5( Bool_t flag_ps=true );
  TList* createTOF( Bool_t flag_ps=true );
  TList* createLAC( Bool_t flag_ps=true );
  TList* createAC1( Bool_t flag_ps=true );
  TList* createSAC3( Bool_t flag_ps=true );
  TList* createSFV( Bool_t flag_ps=true );
  TList* createMsT( Bool_t flag_ps=true );
  TList* createMsT_T0( Bool_t flag_ps=true );
  TList* createMatrix( Bool_t flag_ps=false );
  TList* createHTOF( Bool_t flag_ps=true );
  TList* createTPC( Bool_t flag_ps=true );
  TList* createBVH( Bool_t flag_ps=true );
  TList* createTriggerFlag( Bool_t flag_ps=true );
  TList* createCorrelation( Bool_t flag_ps=true );
  TList* createCorrelation_catch( Bool_t flag_ps=true );
  TList* createDAQ( Bool_t flag_ps=true );
  TList* createDCEff( Bool_t flag_ps=true );
  TList* createBTOF( Bool_t flag_ps=true );
  TList* createE72E90( Bool_t flag_ps=true );
  TList* createTF_TF( Bool_t flag_ps=true );
  TList* createTF_GN1( Bool_t flag_ps=true );
  TList* createTF_GN2( Bool_t flag_ps=true );

  // Beam Profile
  TList* createBeamProfile( Bool_t flag_ps=true );
  TList* createBeamProfileE42( Bool_t flag_ps=true );

  // Old functions E40
  TList* createSFT( Bool_t flag_ps=true );
  TList* createCFT( Bool_t flag_ps=true );
  TList* createBGO( Bool_t flag_ps=true );
  TList* createPiID( Bool_t flag_ps=true );
  TList* createSAC( Bool_t flag_ps=true );
  TList* createFHT1( Bool_t flag_ps=true );
  TList* createFHT2( Bool_t flag_ps=true );
  TList* createTOF_HT( Bool_t flag_ps=true );
  TList* createLC( Bool_t flag_ps=true );

  // E42
  TList* createBH2_E42( Bool_t flag_ps=true );
  TList* createWC( Bool_t flag_ps=true );
  TList* createT1( Bool_t flag_ps=true );
  TList* createT2( Bool_t flag_ps=true );

  // Old functions E07
  TList* createFBH( Bool_t flag_ps=true );
  TList* createSSDT( Bool_t flag_ps=true );
  TList* createSSD1( Bool_t flag_ps=true );
  TList* createSSD2( Bool_t flag_ps=true );
  TList* createEMC( Bool_t flag_ps=true );
  // Old functions
  TList* createBMW( Bool_t flag_ps=true );
  TList* createBAC_SAC( Bool_t flag_ps=true );
  TList* createSFV_SAC3( Bool_t flag_ps=true );
  TList* createGe( Bool_t flag_ps=true );
  TList* createPWO( Bool_t flag_ps=true );
  TList* createBH2_E07( Bool_t flag_ps=true );
  TList* createBAC_E07( Bool_t flag_ps=true );
  TList* createSSD0( Bool_t flag_ps=true );
  TList* createSAC1( Bool_t flag_ps=true );
  TList* createKFAC( Bool_t flag_ps=true );
  TList* createKIC( Bool_t flag_ps=true );
  TList* createHDC( Bool_t flag_ps=true );
  TList* createSP0( Bool_t flag_ps=true );
  //  TList* createSDC4( Bool_t flag_ps=true );
  TList* createTOFMT( Bool_t flag_ps=true );
  TList* createTriggerFlag_E07( Bool_t flag_ps=true );
  TList* createPWO_E05( Bool_t flag_ps=true );

private:
  HistMaker( void );
  HistMaker( const HistMaker& object );
  HistMaker& operator=( const HistMaker& object );

  ClassDef(HistMaker, 0)
};

// getInstance -----------------------------------------------------------
inline HistMaker& HistMaker::getInstance( void )
{
  static HistMaker object;
  return object;
}

// getNofHist ------------------------------------------------------------
inline Int_t HistMaker::getNofHist( void )
{
  HistMaker& g= HistMaker::getInstance();
  return g.idmap_unique_from_seq_.size();
}

// getUniqueID -----------------------------------------------------------
inline Int_t HistMaker::getUniqueID( Int_t detector_type, Int_t subdetector_type,
				     Int_t data_type, Int_t channel )
{
  Int_t unique_id = 0;
  unique_id += detector_type*factorDetectorType;
  unique_id += subdetector_type*factorSubDetectorType;
  unique_id += data_type*factorDataType;
  unique_id += channel;
  return unique_id;
}

// getUniqueID -----------------------------------------------------------
inline Int_t HistMaker::getUniqueID( Int_t sequential_id )
{
  HistMaker& g= HistMaker::getInstance();
  return g.idmap_unique_from_seq_[sequential_id];
}

// getSequentialID -------------------------------------------------------
inline Int_t HistMaker::getSequentialID( Int_t detector_type, Int_t subdetector_type,
					 Int_t data_type, Int_t channel )
{
  HistMaker& g= HistMaker::getInstance();
  Int_t unique_id = g.getUniqueID(detector_type, subdetector_type, data_type, channel);
  return g.idmap_seq_from_unique_[unique_id];
}

// getSequentialID -------------------------------------------------------
inline Int_t HistMaker::getSequentialID( Int_t unique_id )
{
  HistMaker& g= HistMaker::getInstance();
  return g.idmap_seq_from_unique_[unique_id];
}

// getSequentialID -------------------------------------------------------
inline Int_t HistMaker::getSequentialID( const TString& name )
{
  return HistMaker::getInstance().idmap_seq_from_name_[name];
}


#endif
