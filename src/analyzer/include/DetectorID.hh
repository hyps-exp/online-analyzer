// -*- C++ -*-

#ifndef DETECTOR_ID_HH
#define DETECTOR_ID_HH

#include <iostream>
#include <TString.h>
#include <unordered_set>

const Int_t NumOfSegCaenV792  =  32;
const Int_t NumOfSegTOF_HRTDC =  64;
const Int_t NumOfSegCaenV1725 =  16;
const Int_t DetIdTAG_SF       = 777; // This value is temporarily
const Int_t NumOfLayersTAG_SF =   2;
const Int_t NumOfSegTAG_SF    =  55;
const Int_t DetIdTAG_PL       = 778; // This value is temporarily
const Int_t NumOfSegTAG_PL    =  10;
const Int_t DetIdE_Veto       = 779; // This value is temporarily
const Int_t NumOfSegE_Veto    =   1;
const Int_t DetIdU_Veto       = 782; // This value is temporarily
const Int_t NumOfSegU_Veto    =   1;
const Int_t DetIdT0           = 780; // This value is temporarily
const Int_t NumOfSegT0        =   1;
const Int_t DetIdRF           = 781; // This value is temporarily
const Int_t NumOfSegRF        =   1;

inline const std::unordered_set<Int_t> TOF_Absent =
  {0, 1, 2, 3, 4};

// Counters ___________________________________________________________
const Int_t DetIdBH1      =  1;
const Int_t DetIdBH2      =  2;
const Int_t DetIdBAC      =  3;
const Int_t DetIdPVAC     =  4;
const Int_t DetIdFAC      =  5;
const Int_t DetIdSCH      =  6;
const Int_t DetIdTOF      =  7;
const Int_t DetIdHTOF     =  8;
const Int_t DetIdBVH      =  99;
const Int_t DetIdLAC      = 12;
const Int_t DetIdWC       = 13; // E42
const Int_t DetIdAC1       = 14;
const Int_t DetIdSAC3       = 15;
const Int_t DetIdSFV       = 16;
const Int_t NumOfSegBH1     = 11;
const Int_t NumOfSegBH2     =  8;
const Int_t NumOfSegBAC     =  2;
const Int_t NumOfSegPVAC    =  1;
const Int_t NumOfSegFAC     =  1;
const Int_t NumOfSegSCH     = 64;
// const Int_t NumOfSegTOF     = 24; // HYPS
const Int_t NumOfSegTOF     = 48; // HYPS
const Int_t NumOfSegHTOF    = 34;
const Int_t NumOfSegBVH     =  4;
const Int_t NumOfSegLAC     = 30;
const Int_t NumOfSegAC1     =  22;
const Int_t NumOfSegWC      =  12;
const Int_t NumOfSegSAC3    =  2;
const Int_t NumOfSegSFV     =  7;

// Misc _______________________________________________________________
const Int_t DetIdTrig       = 21;
const Int_t DetIdScaler     = 22;
const Int_t DetIdMsT        = 25;
const Int_t DetIdMtx        = 26;
const Int_t DetIdFpgaBH2Mt  = 29;
const Int_t DetIdVmeRm      = 81;
const Int_t DetIdMsTRM      = 82;
const Int_t DetIdHulRM      = 83;
const Int_t NumOfSegTrig    = 32;
const Int_t NumOfSegScaler  = 128;
const Int_t NumOfPlaneVmeRm = 10;
const Int_t NumOfVmeRm      = 10;

const Int_t LSOGeFlag       = 0; // HbxTrig
const Int_t GeCoinFlag      = 1; // HbxTrig
const Int_t SpillOnFlag     = 2; // HbxTrig
const Int_t SpillOffFlag    = 3; // HbxTrig

//const Int_t NumOfSegTFlag  = ; // defined in the following

const Int_t DetIdVmeCalib      = 999;
const Int_t NumOfPlaneVmeCalib =   5;
const Int_t NumOfSegVmeCalib   =  32;

// Trackers ___________________________________________________________
const Int_t DetIdBC3    = 103;
const Int_t DetIdBC4    = 114;
const Int_t DetIdSDC0   = 104;
const Int_t DetIdSDC1   = 105;
const Int_t DetIdSDC2   = 106;
const Int_t DetIdSDC3   = 107;
// const Int_t DetIdDC1    = 901; // HYPS
// const Int_t DetIdDC2    = 902; // HYPS
// const Int_t DetIdDC3    = 903; // HYPS
const Int_t DetIdSDC4   = 108;
const Int_t DetIdSDC5   = 109;
const Int_t DetIdBFT    = 110;
const Int_t DetIdBcOut  = 20;
const Int_t DetIdSdcIn  = 30;
const Int_t DetIdSdcOut = 40;

const Int_t PlMinBcIn        =   1;
const Int_t PlMaxBcIn        =  12;
const Int_t PlMinBcOut       =  13;
const Int_t PlMaxBcOut       =  24;
const Int_t PlMinSdcIn       =   1;
const Int_t PlMaxSdcIn       =  10;
const Int_t PlMinSdcOut      =  31;
const Int_t PlMaxSdcOut      =  42;
const Int_t PlOffsBc         = 100;
const Int_t PlOffsSdcIn      =   0;
const Int_t PlOffsSdcOut     =  30;

const Int_t NumOfLayersBc     =   6;
// const Int_t NumOfLayersDC1    =   6; // HYPS
// const Int_t NumOfDimDC1       =   4; // HYPS
// const Int_t NumOfLayersDC2    =   5; // HYPS
// const Int_t NumOfDimDC2       =   3; // HYPS
// const Int_t NumOfLayersDC3    =   5; // HYPS
// const Int_t NumOfDimDC3       =   3; // HYPS
const Int_t NumOfLayersBC3    =   6;
const Int_t NumOfDimBC3       =   3;
const Int_t NumOfLayersBC4    =   6;
const Int_t NumOfDimBC4       =   3;
const Int_t NumOfLayersSDC0   =   4; // HYPS
const Int_t NumOfDimSDC0      =   2; // HYPS
const Int_t NumOfLayersSDC1   =   6; // HYPS
const Int_t NumOfDimSDC1      =   4; // HYPS
const Int_t NumOfLayersSDC2   =   5; // HYPS
const Int_t NumOfDimSDC2      =   3; // HYPS
const Int_t NumOfLayersSDC3   =   5; // HYPS
const Int_t NumOfDimSDC3      =   3; // HYPS
const Int_t NumOfLayersSDC4   =   4;
const Int_t NumOfDimSDC4      =   2;
const Int_t NumOfLayersSDC5   =   4;
const Int_t NumOfDimSDC5      =   2;
const Int_t NumOfWireSDC0     = 112; // HYPS
const Int_t NumOfWireSDC1     = 48;  // HYPS
const Int_t NumOfWireSDC2     = 104;  // HYPS
const Int_t NumOfWireSDC3     = 104;  // HYPS
const Int_t NumOfWireSDC0_HYPS[NumOfLayersSDC0] = {112, 112, 112, 112};  // HYPS
const Int_t NumOfWireSDC1_HYPS[NumOfLayersSDC1] = {48, 48, 48, 48, 48, 48};  // HYPS
const Int_t NumOfWireSDC2_HYPS[NumOfLayersSDC2] = {79, 78, 78, 104, 104};  // HYPS
const Int_t NumOfWireSDC3_HYPS[NumOfLayersSDC3] = {79, 78, 78, 104, 104};  // HYPS
const Int_t NumOfWireBC3      =  64;
const Int_t NumOfWireBC4      =  64;
// const Int_t NumOfWireSDC1     =  64;
// const Int_t NumOfWireSDC2X    =  70; //not use for e70
// const Int_t NumOfWireSDC2Y    =  40; //not use for e70
// const Int_t NumOfWireSDC2     =  44;
// const Int_t NumOfWireSDC3     = 128;
const Int_t NumOfWireSDC4X    =  96; //not use for e70
const Int_t NumOfWireSDC4Y    =  64; //not use for e70
const Int_t NumOfWireSDC4     = 128;
const Int_t NumOfWireSDC5X    = 128;
const Int_t NumOfWireSDC5Y    =  96;
const Double_t MaxDriftLengthBC3  =  1.5;
const Double_t MaxDriftLengthBC4  =  1.5;
const Double_t MaxDriftLengthSDC1 =  3.0;
const Double_t MaxDriftLengthSDC2 =  1.0;
const Double_t MaxDriftLengthSDC3 =  4.5;
const Double_t MaxDriftLengthSDC4 = 10.0;
const Double_t MaxDriftTimeBC3    =  80.0;
const Double_t MaxDriftTimeBC4    =  80.0;
const Double_t MaxDriftTimeSDC1   = 120.0;
const Double_t MaxDriftTimeSDC2   = 120.0;
const Double_t MaxDriftTimeSDC3   = 260.0;
const Double_t MaxDriftTimeSDC4   = 260.0;
const Double_t MaxDriftTimeSDC5   = 260.0;
const Int_t NumOfLayersBcIn   = PlMaxBcIn   - PlMinBcIn   + 1;
const Int_t NumOfLayersBcOut  = PlMaxBcOut  - PlMinBcOut  + 1;
const Int_t NumOfLayersSdcIn  = PlMaxSdcIn  - PlMinSdcIn  + 1;
const Int_t NumOfLayersSdcOut = PlMaxSdcOut - PlMinSdcOut + 1;

const Int_t MaxWireBC3      =  64;
const Int_t MaxWireBC4      =  64;

const Int_t MaxWireSDC1     =  64;
// const Int_t MaxWireSDC2     = 220;
const Int_t MaxWireSDC2     =  44;
const Int_t MaxWireSDC3     = 128;
const Int_t MaxWireSDC4     = 128;
const Int_t MaxWireSDC4X    = 128;
const Int_t MaxWireSDC4Y    = 128;
const Int_t MaxWireSDC5     = 128;

const Int_t NumOfPlaneBFT   =   2;
const Int_t NumOfSegBFT     = 160;

// HulRm -----------------------------------------------
const Int_t NumOfHulRm   = 4;

// Matrix ----------------------------------------------
// const Int_t NumOfSegSFT_Mtx = 48;
const Int_t NumOfSegTOF_Mtx = 28;

// MsT -------------------------------------------------
enum TypesMst{typeHrTdc, typeLrTdc, typeFlag, NumOfTypesMst};
const Int_t NumOfMstHrTdc = 32;
const Int_t NumOfMstLrTdc = 64;
const Int_t NumOfMstFlag  = 7;
enum dTypesMst
  {
   mstClear,
   mstAccept,
   finalClear,
   cosolationAccept,
   fastClear,
   level2,
   noDecision,
   size_dTypsMsT
  };

// Scaler ----------------------------------------------
const Int_t NumOfScaler  = 2;

// E72  _______________________________________________________________
const Int_t DetIdE72BAC     = 501;
const Int_t DetIdE90SAC     = 502;
const Int_t DetIdE72KVC     = 503;
const Int_t DetIdE42BH2     = 504;
const Int_t DetIdT1         = 505;
const Int_t DetIdT2         = 506;
const Int_t NumOfSegE72BAC  = 1;
const Int_t NumOfSegE90SAC  = 2;
const Int_t NumOfSegE72KVC  = 4;
const Int_t NumOfSegE42BH2  = 8;
const Int_t NumOfSegT1      = 1;
const Int_t NumOfSegT2      = 1;
const Int_t NumOfSegE72Parasite  = 8;

namespace e72parasite
{
  enum EE72Parasite
  {
    kT1,
    kE42BH2,
    kE72BAC,
    kE90SAC1,
    kE90SAC2,
    kE72KVC,
    kE72KVCSUM,
    kT2
  };

  const std::vector<TString> SE72Parasite =
  {
    "T1",
    "BH2",
    "BAC",
    "SAC-6",
    "SAC-8",
    "KVC",
    "KVCSUM",
    "T2",
  };
}


// TF  _______________________________________________________________
const Int_t DetIdTF_TF      = 507;
const Int_t DetIdTF_GN1     = 508;
const Int_t DetIdTF_GN2     = 509;
const Int_t NumOfSegTF_TF   = 1;
const Int_t NumOfSegTF_GN1  = 1;
const Int_t NumOfSegTF_GN2  = 1;

// E42
const Int_t DetIdTPC       = 70;
const Int_t NumOfLayersTPC = 32;
const Int_t NumOfTimeBucket = 170;

// AFT for E70
const Int_t DetIdAFT          = 112;
const Int_t NumOfPlaneAFT     = 36;
const std::vector<TString> NameOfPlaneAFT = {"X0", "X1", "Y0", "Y1"};
const Int_t NumOfSegAFTX      = 32;
const Int_t NumOfSegAFTY      = 16;
const Int_t NumOfSegAFT[4]    = {NumOfSegAFTX, NumOfSegAFTX, NumOfSegAFTY, NumOfSegAFTY};

// VMEEASIROC Board
const Int_t DetIdVEASIROC         = 116;
const std::vector<Int_t> PlaneIdOfVMEEASIROC =
  {16, 17, 31, 32, 33, 34, 49, 50, 51, 52, 53, 69, 70, 71, 72, 73, 74, 95,
   19, 20, 21, 22, 35, 36, 37, 38, 54, 55, 56, 57, 58, 75, 76, 77, 78, 79, 80,
   23, 24, 25, 26, 39, 40, 41, 42, 43, 59, 60, 61, 62, 63, 81, 82, 83, 84, 85,
   27, 28, 29, 30, 44, 45, 46, 47, 48, 64, 65, 66, 67, 68, 86,  87, 88, 89,
   90, 91, 92, 94, 93};
//const Int_t NumOfPlaneVMEEASIROC = PlaneIdOfVMEEASIROC.size();
const Int_t NumOfPlaneVMEEASIROC = 79;
const Int_t NumOfSegVMEEASIROC    = 64;

// Trigger Flag
namespace trigger
{
  enum ETriggerFlag
  {
    kTrigAPS,
    kTrigBPS,
    kTrigCPS,
    kTrigDPS,
    kTrigEPS,
    kTrigFPS,
    kTringPSORA,
    kTringPSORB,
    kClockPS,
    kReserve2PS,
    kLevel1OR,
    kclock10MHz,
    kclock1MHz,
    kclock100kHz,
    kclock10kHz,
    kclock1kHz,
    kL1SpillOn,
    kL1SpillOff,
    kSpillOnEnd,
    kSpillOffEnd,
    NTriggerFlag
  };

  const std::vector<TString> STriggerFlag =
    {
     "TrigA-PS",
     "TrigB-PS",
     "TrigC-PS",
     "TrigD-PS",
     "TrigE-PS",
     "TrigF-PS",
     "Trig-PSORA",
     "Trig-PSORB",
     "Clock-PS",
     "Reserve2-PS",
     "Level1OR",
     "clock10MHz",
     "clock1MHz",
     "clock100kHz",
     "clock10kHz",
     "clock1kHz",
     "L1SpillOn",
     "L1SpillOff",
     "SpillOnEnd",
     "SpillOffEnd",
    };

  // enum ETriggerFlag
  // {
  //   kL1SpillOn,
  //   kL1SpillOff,
  //   kSpillOnEnd,
  //   kSpillOffEnd,
  //   kTofTiming,
  //   kMatrix2D1,
  //   kMatrix2D2,
  //   kMatrix3D,
  //   kBeamA,
  //   kBeamB,
  //   kBeamC,
  //   kBeamD,
  //   kBeamE,
  //   kBeamF,
  //   kTrigA,
  //   kTrigB,
  //   kTrigC,
  //   kTrigD,
  //   kTrigE,
  //   kTrigF,
  //   kTrigAPS,
  //   kTrigBPS,
  //   kTrigCPS,
  //   kTrigDPS,
  //   kTrigEPS,
  //   kTrigFPS,
  //   kLevel1A,
  //   kLevel1B,
  //   kClockPS,
  //   kReserve2PS,
  //   kLevel1OR,
  //   kEssDischarge,
  //   NTriggerFlag
  // };

  // const std::vector<TString> STriggerFlag =
  //   {
  //    "L1SpillOn",
  //    "L1SpillOff",
  //    "SpillEnd",
  //    "SpillOnEnd",
  //    "TofTiming",
  //    "Matrix2D1",
  //    "Matrix2D2",
  //    "Matrix3D",
  //    "BeamA",
  //    "BeamB",
  //    "BeamC",
  //    "BeamD",
  //    "BeamE",
  //    "BeamF",
  //    "TrigA",
  //    "TrigB",
  //    "TrigC",
  //    "TrigD",
  //    "TrigE",
  //    "TrigF",
  //    "TrigA-PS",
  //    "TrigB-PS",
  //    "TrigC-PS",
  //    "TrigD-PS",
  //    "TrigE-PS",
  //    "TrigF-PS",
  //    "Level1A",
  //    "Level1B",
  //    "Clock-PS",
  //    "Reserve2-PS",
  //    "Level1OR",
  //    "EssDischarge",
  //   };
}
const Int_t NumOfSegTFlag = trigger::NTriggerFlag;

namespace DetHtTOF
{
  enum EHtTOF
    {
     TOF1_20,
     TOF5_22,
     TOF18,
     TOF4_10,
     TOF2_11,
     TOF8_24,
     TOF6_9,
     TOF7_23,
     TOF12,
     TOF19,
     TOF13,
     TOF14,
     TOF15,
     TOF16,
     TOF3_21,
     TOF17,
     NTOF_HT
    };

  const std::vector<TString> SHtTOF     =
    {
     "TOF(1,20)",
     "TOF(5,22)",
     "TOF18",
     "TOF(4,10)",
     "TOF(2,11)",
     "TOF(8,24)",
     "TOF(6,9)",
     "TOF(7,23)",
     "TOF12",
     "TOF19",
     "TOF13",
     "TOF14",
     "TOF15",
     "TOF16",
     "TOF(3,21)",
     "TOF17",
    };
};

// Old Detectors
//E42 test in E40 beamtime
//const Int_t DetIdT1       = 14; // E42
//const Int_t DetIdT2       = 15; // E42
//const Int_t NumOfSegT1      =  1; // E42
//const Int_t NumOfSegT2      =  1; // E42

//E40 Detectors
const Int_t DetIdSAC      =  9;
const Int_t DetIdLC       = 10;
const Int_t DetIdHtTOF    = 11; // high threshold TOF
const Int_t NumOfSegHtTOF   = 16;
const Int_t NumOfSegClusteredFBH = 31;
const Int_t NumOfRoomsSAC   =  4;
const Int_t NumOfSegLC      = 28;
const Int_t NumOfSegSAC     =  5; // HYPS
const Int_t NumOfSegFBH     = 16;
const Int_t DetIdSFT  = 111;
const Int_t DetIdCFT  = 113;
const Int_t DetIdBGO  = 114;
const Int_t DetIdPiID = 115;
const Int_t DetIdFHT1 = 131;
const Int_t DetIdFHT2 = 132;
const Int_t PlMinFHT1        =  80;
const Int_t PlMaxFHT1        =  83;
const Int_t PlMinFHT2        =  84;
const Int_t PlMaxFHT2        =  87;
const Int_t PlOffsVP         =  20;
const Int_t PlOffsSft        =   6;
const Int_t PlOffsFbt        =  80;
const Int_t NumOfLayersSFT    = 3;
const Int_t NumOfLayersCFT    =  8;
const Int_t NumOfLayersFHT1   = 2;
const Int_t NumOfLayersFHT2   = 2;
//const Int_t NumOfLayersFHT    = PlMaxFHT2   - PlMinFHT1   + 1;
const Int_t NumOfLayersFHT    = 2;
//const Int_t NumOfLayersSdcOut = PlMaxSdcOut - PlMinSdcOut + 1 + (PlMaxFHT2 - PlMinFHT1 + 1); // including FHT
const Int_t NumOfLayersVP     = 5;
const Int_t MaxSegFHT1      =  48;
const Int_t MaxSegFHT2      =  64;
const Int_t NumOfSegFHT1          = 48;
const Int_t NumOfSegFHT2          = 64;
// SFT X layer has U D plane.
// SFT UV layers have only U plnane.
// enum SFT_PLANE{ SFT_X1, SFT_X2, SFT_V, SFT_U };
//SFT
enum SFT_PLANE{ SFT_U, SFT_V, SFT_X1, SFT_X2 };
const Int_t NumOfPlaneSFT   =   4;
const Int_t NumOfSegSFT_X   = 256;
const Int_t NumOfSegSFT_UV  = 320;
const Int_t NumOfSegSFT[NumOfPlaneSFT] = { NumOfSegSFT_UV,
					 NumOfSegSFT_UV,
					 NumOfSegSFT_X,
					 NumOfSegSFT_X };
const Int_t NumOfSegCSFT    = 48;
// CFT
const Int_t NumOfPlaneCFT   =   8;
enum CFT_PLANE{CFT_U1, CFT_PHI1, CFT_V2, CFT_PHI2, CFT_U3, CFT_PHI3, CFT_V4, CFT_PHI4};
enum CFT_PLANE_{CFT_UV1, CFT_PHI1_, CFT_UV2, CFT_PHI2_, CFT_UV3, CFT_PHI3_, CFT_UV4, CFT_PHI4_};
const Int_t NumOfSegCFT_UV1   = 426;
const Int_t NumOfSegCFT_PHI1  = 584;
const Int_t NumOfSegCFT_UV2   = 472;
const Int_t NumOfSegCFT_PHI2  = 692;
const Int_t NumOfSegCFT_UV3   = 510;
const Int_t NumOfSegCFT_PHI3  = 800;
const Int_t NumOfSegCFT_UV4   = 538;
const Int_t NumOfSegCFT_PHI4  = 910;
const Int_t NumOfSegCFT[NumOfPlaneCFT]  = {426,584,472,692,510,800,538,910};
// BGO
const Double_t BGO_X = 30.;
const Double_t BGO_Y = 25.;
const Double_t BGO_Z = 400.;
const Int_t    NumOfBGOUnit = 8;
const Int_t    NumOfBGOInOneUnit = 2;//pair unit
const Double_t RadiusOfBGOSurface = 100.;
const Int_t    NumOfBGOInOneUnit2 = 1;//single unit
const Double_t RadiusOfBGOSurface2 = 120.;
const Int_t NumOfSegBGO = NumOfBGOUnit*(NumOfBGOInOneUnit+NumOfBGOInOneUnit2);//24
const Int_t NumOfSegBGO_T=   4;

// PiID counter
const Int_t NumOfSegPiID =  32;
const Int_t NumOfPiIDUnit = 8;
const Int_t NumOfPiIDInOneUnit = 3;
const Double_t PiID_X = 30.;
const Double_t PiID_Y = 10.;
const Double_t PiID_Z = 400.;
const Double_t RadiusOfPiIDSurface = 164.;
const Int_t    NumOfPiIDInOneUnit2 = 1;//single unit
const Double_t PiID2_X = 40.;
const Double_t PiID2_Y = 10.;
const Double_t PiID2_Z = 400.;
const Double_t RadiusOfPiID2Surface = 180.;

// Tracker const
const Int_t FHTOffset             = 200;
const Int_t NumOfUDStructureFHT   = 2;

// E07 Detectors
const Int_t DetIdFBH  =  4;
const Int_t DetIdEMC      = 23;
const Int_t DetIdSSDT     = 24;
const Int_t NumOfSegEMC      =  1;
const Int_t NumOfSegSSDT     = 16;
const Int_t DetIdSSD1 = 121;
const Int_t DetIdSSD2 = 122;
const Int_t DetIdSsd    = 50;
const Int_t NumOfLayersSsd  = 8;
const Int_t NumOfLayersSSD1 = 4;
const Int_t NumOfLayersSSD2 = 4;
const Int_t NumOfSamplesSSD = 8;
const Int_t NumOfSegSSD1 = 1536;
const Int_t NumOfSegSSD2 = 1536;
const Double_t SamplingIntervalSSD = 25.; //[ns]
const Int_t DetIdGC   =  8;
const Int_t DetIdYOON =  98;
const Int_t DetIdRC   = 21;
// const Int_t DetIdAC1  = 22;
// const Int_t DetIdBVH  = 23;
const Int_t DetIdNRC  = 24;
const Int_t DetIdBH2_E07 =  33;
const Int_t DetIdBAC_E07 =  34;
const Int_t DetIdSAC1    =  37;
const Int_t DetIdKFAC    =  44;
const Int_t NumOfSegBH2_E07 =  2;
const Int_t NumOfSegBAC_E07 =  4 + 2;
const Int_t NumOfSegSAC1    =  1;
const Int_t NumOfSegKFAC    =  2;
const Int_t NumOfLayersSSD0 =  2;
// E05 Detectors
const Int_t DetIdKIC     = 41;
const Int_t NumOfSegMsT  = 24;
const Int_t NumOfSegKIC  =  4;
// E13 Detectors
const Int_t DetIdBMW     = 10;
const Int_t DetIdMatrix  = 12;
//const Int_t DetIdSFV     = 24;
const Int_t DetIdTOFMT   = 25;
const Int_t DetIdSP0     = 26;
//const Int_t DetIdGe      = 27;
const Int_t DetIdPWO     = 28;
const Int_t DetIdPWOADC  = 29;
const Int_t NumOfSegBAC_SAC = 6;
//const Int_t NumOfSegSFV = 18;
//const Int_t NumOfSegGe   = 32;

const Int_t NumOfSegPWO  = 238;
const Int_t NumOfBoxPWO  = 22;
const Int_t NumOfUnitPWO[NumOfBoxPWO] =
  {
   6, 9, 6, 14, 14, 21, 14, 14, 6, 9, 6,
   6, 9, 6, 14, 14, 21, 14, 14, 6, 9, 6
  };

const Int_t SegIdPWO[NumOfBoxPWO] =
  {
   4, 0, 0, 4, 0, 0, 1, 5, 2, 1, 6,
   1, 2, 5, 6, 2, 1, 3, 7, 7, 3, 3,
  };

const Int_t NumOfLayersSP0 = 8;
const Int_t NumOfSegSP0    = 5;

//E03 HBX detctors
const Int_t DetIdGe         = 27;
// const Int_t DetIdBGO        = 114;
const Int_t DetIdHbxTrig    = 23;
const Int_t NumOfSegGe      = 16;
// const Int_t NumOfSegBGO     = 24;
const Int_t NumOfSegHbxTrig = 4;

const Int_t DetIdBC1   = 101;
const Int_t DetIdBC2   = 102;
//const Int_t DetIdSDC4  = 108;
const Int_t DetIdK6BDC = 109;
const Int_t DetIdHDC   = 113;
const Int_t DetIdSSD0  = 120;

const Int_t DetIdBcIn   = 10;

const Int_t NumOfLayersBC1  =  6;
const Int_t NumOfLayersBC2  =  6;
const Int_t NumOfLayersHDC  =  4;
//const Int_t NumOfLayersSDC4 =  6;

const Int_t NumOfWireBC1    =  256;
const Int_t NumOfWireBC2    =  256;
const Int_t NumOfWireHDC   =  112;
const Int_t NumOfWireSDC3x  =  108;
//const Int_t NumOfWireSDC4x  =  108;
const Int_t NumOfWireSDC3uv =  120;
//const Int_t NumOfWireSDC4uv =  120;
const Int_t NumOfSegSSD0    = 1536;

#endif
