// -*- C++ -*-

#ifndef DC_PARAMETERS_HH
#define DC_PARAMETERS_HH

#include <Rtypes.h>
#include <TString.h>

#include <std_ostream.hh>

//______________________________________________________________________________
struct DCPairPlaneInfo
{
  Bool_t pair;
  Bool_t honeycomb;
  Bool_t fiber;
  Int_t  id1, id2;
  Double_t CellSize;

  void Print( const TString& arg="", std::ostream& ost=hddaq::cout ) const
  {
    ost << "[DCPairPlaneInfo::Print()] " << arg << std::endl
	<< " pair      : " << pair      << std::endl
	<< " honeycomb : " << honeycomb << std::endl
	<< " fiber     : " << fiber     << std::endl
	<< " layer1    : " << id1       << std::endl
	<< " layer2    : " << id2       << std::endl
	<< " cell size : " << CellSize  << std::endl;
  }
};

extern const DCPairPlaneInfo PPInfoBcOut[], PPInfoSdcIn[], PPInfoSdcOut[];
extern const Int_t NPPInfoBcOut, NPPInfoSdcIn, NPPInfoSdcOut;

#ifdef DefStatic
const DCPairPlaneInfo PPInfoBcOut[] = {
  // { pair_plane, honeycomb, fiber, id1, id2, CellSize }
  { true, false, false, 1,  2,  3.0 }, { true, false, false,  3,  4,  3.0 },
  { true, false, false, 5,  6,  3.0 }, { true, false, false,  7,  8,  3.0 },
  { true, false, false, 9, 10,  3.0 }, { true, false, false, 11, 12,  3.0 }
};

const DCPairPlaneInfo PPInfoSdcIn[] = {
  // { pair_plane, honeycomb, fiber, id1, id2, CellSize }
  // SDC1
  { true, true, false, 1,  2,  6.0 }, { true, true, false, 3,  4,  6.0 },
  { true, true, false, 5,  6,  6.0 },
  // SFT
  { false, false, true, 7,  7,  3.0 }, { false, false, true, 8,  8,  3.0 },
  { false, false, true, 9,  9,  3.0 },

  // { false, true, false, 1,  1,  6.0 }, { false, true, false, 2,  2,  6.0 },
  // { false, true, false, 3,  3,  6.0 }, { false, true, false, 4,  4,  6.0 },
  // { false, true, false, 5,  5,  6.0 }, { false, true, false, 6,  6,  6.0 }

  // { true, false, false, 1,  2,  6.0 }, { true, false, false, 3,  4,  6.0 },
  // { true, false, false, 5,  6,  6.0 }
};

const DCPairPlaneInfo PPInfoSdcOut[] = {
  // { pair_plane, honeycomb, fiber, id1, id2, CellSize }
  { true, true, false, 1, 2,  9.0 }, { true, true, false, 3, 4,  9.0 },
  { true, true, false, 5, 6, 20.0 }, { true, true, false, 7, 8, 20.0 }

  // { false, true, false, 1, 1,  9.0 }, { false, true, false, 2, 2,  9.0 },
  // { false, true, false, 3, 3,  9.0 }, { false, true, false, 4, 4,  9.0 },
  // { false, true, false, 5, 5, 20.0 }, { false, true, false, 6, 6, 20.0 },
  // { false, true, false, 7, 7, 20.0 }, { false, true, false, 8, 8, 20.0 }

  // { true, false, false, 1, 2,  9.0 }, { true, false, false, 3, 4,  9.0 },
  // { true, false, false, 5, 6, 20.0 }, { true, false, false, 7, 8, 20.0 }
};

const Int_t NPPInfoBcOut  = sizeof(PPInfoBcOut)/sizeof(DCPairPlaneInfo);
const Int_t NPPInfoSdcIn  = sizeof(PPInfoSdcIn)/sizeof(DCPairPlaneInfo);
const Int_t NPPInfoSdcOut = sizeof(PPInfoSdcOut)/sizeof(DCPairPlaneInfo);

#endif

//DL Ranges (BC1&2 for Time range -5 ns <[Time gate]<75 ns)
const Double_t MinDLBc[25] = {
   0.0,
   // BC1
  -5.0, -5.0, -5.0, -5.0, -5.0, -5.0,
   // BC2
  -5.0, -5.0, -5.0, -5.0, -5.0, -5.0,
   // BC3
  -0.5, -0.5, -0.5, -0.5, -0.5, -0.5,
   // BC4
  -0.5, -0.5, -0.5, -0.5, -0.5, -0.5
};

const Double_t MaxDLBc[25] = {
  0.0,
  // BC1
  75.0, 75.0, 75.0, 75.0, 75.0, 75.0,
  // BC2
  75.0, 75.0, 75.0, 75.0, 75.0, 75.0,
  // BC3
  1.8, 1.8, 1.8, 1.8, 1.8, 1.8,
  // BC4
  1.8, 1.8, 1.8, 1.8, 1.8, 1.8
};

const Double_t MinDLSdc[] = {
  0.0,
  // SFT
  //0.0, -5.0, -5.0, -5.0,
  //SDC1
  -0.5, -0.5, -0.5, -0.5, -0.5, -0.5,
  // Dummy Id 07-30
  0.0, 0.0, 0.0, 0.0,
  0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
  0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
  // SDC2
  -0.5, -0.5, -0.5, -0.5,
  // SDC3
  -0.5, -0.5, -0.5, -0.5
};

const Double_t MaxDLSdc[] = {
  0.0,
  // SFT
  //0.0, 75.0, 75.0, 75.0,
  // SDC1
  10.0, 10.0, 10.0, 10.0, 10.0, 10.0,
  // Dummy Id 07-30
  0.0, 0.0, 0.0, 0.0,
  0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
  0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
  // SDC2
  12.0, 12.0, 12.0, 12.0,
  // SDC3
  25.0, 25.0, 25.0, 25.0
};

#endif