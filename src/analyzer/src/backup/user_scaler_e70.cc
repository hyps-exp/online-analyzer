// -*- C++ -*-

#include <iomanip>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "DAQNode.hh"
#include "UnpackerManager.hh"
#include "Unpacker.hh"

#include "ConfMan.hh"
#include "DetectorID.hh"
#include "PrintHelper.hh"
#include "ScalerAnalyzer.hh"
#include "user_analyzer.hh"

namespace
{
using hddaq::unpacker::GUnpacker;
const auto&    gUnpacker = GUnpacker::get_instance();
ScalerAnalyzer scaler_main;
Scaler         nspill_scaler_sheet  = 1; // Number of spill for Scaler Sheet
}

namespace analyzer
{

//____________________________________________________________________________
Int_t
process_begin(const std::vector<std::string>& argv)
{
  ConfMan::GetInstance().Initialize(argv);

  // flags
  scaler_main.SetFlag(ScalerAnalyzer::kSeparateComma);
  scaler_main.SetFlag(ScalerAnalyzer::kSemiOnline);
  for(Int_t i=0, n=argv.size(); i<n; ++i){
    TString v = argv[i];
    if(v.Contains("--print")){
      scaler_main.SetFlag(ScalerAnalyzer::kScalerSheet);
    }
    if(v.Contains("--spill=")){
      nspill_scaler_sheet = v.ReplaceAll("--spill=","").Atoi();
    }
    if(v.Contains(":")){
      scaler_main.SetFlag(ScalerAnalyzer::kSemiOnline, false);
    }
    if(v.Contains("--spill-by-spill")){
      scaler_main.SetFlag(ScalerAnalyzer::kSpillBySpill);
    }
    if(v.Contains("--spillon")){
      scaler_main.SetFlag(ScalerAnalyzer::kSpillOn);
    }
    if(v.Contains("--spilloff")){
      scaler_main.SetFlag(ScalerAnalyzer::kSpillOff);
    }
    if(v.Contains("--hbx")){
      scaler_main.SetFlag(ScalerAnalyzer::kScalerHBX);
    }
  }
  // for(Int_t i=0; i<ScalerAnalyzer::nFlag; ++i){
  //   scaler_hbx.SetFlag(scaler_main.GetFlag(i));
  // }

  //////////////////// Set Channels
  // ScalerAnalylzer::Set(Int_t column,
  //                       Int_t raw,
  //                       ScalerInfo(name, module, channel));
  // scaler information is defined from here.
  // please do not use a white space character.
  {
    Int_t c = ScalerAnalyzer::kLeft;
    Int_t r = 0;
    scaler_main.Set(c, r++, ScalerInfo("BH1",        0, 16));
    scaler_main.Set(c, r++, ScalerInfo("BH1-SUM",   -1, -1));
    scaler_main.Set(c, r++, ScalerInfo("BH1-01",     1,  0));
    scaler_main.Set(c, r++, ScalerInfo("BH1-02",     1,  1));
    scaler_main.Set(c, r++, ScalerInfo("BH1-03",     1,  2));
    scaler_main.Set(c, r++, ScalerInfo("BH1-04",     1,  3));
    scaler_main.Set(c, r++, ScalerInfo("BH1-05",     1,  4));
    scaler_main.Set(c, r++, ScalerInfo("BH1-06",     1,  5));
    scaler_main.Set(c, r++, ScalerInfo("BH1-07",     1,  6));
    scaler_main.Set(c, r++, ScalerInfo("BH1-08",     1,  7));
    scaler_main.Set(c, r++, ScalerInfo("BH1-09",     1,  8));
    scaler_main.Set(c, r++, ScalerInfo("BH1-10",     1,  9));
    scaler_main.Set(c, r++, ScalerInfo("BH1-11",     1, 10));
    scaler_main.Set(c, r++, ScalerInfo("BH2",        0, 17));
    scaler_main.Set(c, r++, ScalerInfo("BH2-SUM",   -1, -1));
    scaler_main.Set(c, r++, ScalerInfo("BH2-01",     0, 64));
    scaler_main.Set(c, r++, ScalerInfo("BH2-02",     0, 65));
    scaler_main.Set(c, r++, ScalerInfo("BH2-03",     0, 66));
    scaler_main.Set(c, r++, ScalerInfo("BH2-04",     0, 67));
    scaler_main.Set(c, r++, ScalerInfo("BH2-05",     0, 68));
    scaler_main.Set(c, r++, ScalerInfo("BH2-06",     0, 69));
    scaler_main.Set(c, r++, ScalerInfo("BH2-07",     0, 70));
    scaler_main.Set(c, r++, ScalerInfo("BH2-08",     0, 71));
    scaler_main.Set(c, r++, ScalerInfo("V792gate-1",    2, 2));
    scaler_main.Set(c, r++, ScalerInfo("V792gate-2",    2, 3));
    scaler_main.Set(c, r++, ScalerInfo("V792gate-3",    2, 4));
    scaler_main.Set(c, r++, ScalerInfo("AFT-01",     0, 80));
    scaler_main.Set(c, r++, ScalerInfo("AFT-02",     0, 81));
    scaler_main.Set(c, r++, ScalerInfo("AFT-03",     0, 82));
  }

  {
    Int_t c = ScalerAnalyzer::kCenter;
    Int_t r = 0;
    scaler_main.Set(c, r++, ScalerInfo("10M-Clock",    0,  0));
    //    scaler_main.Set(c, r++, ScalerInfo("10M-Clock-scr03",    2, 1));
    scaler_main.Set(c, r++, ScalerInfo("TM",           0,  9));
    scaler_main.Set(c, r++, ScalerInfo("SY",           0, 10));
    scaler_main.Set(c, r++, ScalerInfo("K-Beam",       0, 35));
    scaler_main.Set(c, r++, ScalerInfo("Pi-Beam",      0, 39));
    scaler_main.Set(c, r++, ScalerInfo("Beam",      0, 36));
    scaler_main.Set(c, r++, ScalerInfo("BH1-1/100-PS", 1, 11));
    scaler_main.Set(c, r++, ScalerInfo("BH1-1/1e5-PS", 1, 12));
    //    scaler_main.Set(c, r++, ScalerInfo("TOF-24",       0, 29));
    scaler_main.Set(c, r++, ScalerInfo("Mtx2D-1",      0, 32));
    scaler_main.Set(c, r++, ScalerInfo("Mtx2D-2",      0, 33));
    scaler_main.Set(c, r++, ScalerInfo("Mtx3D",        0, 34));
    // scaler_main.Set(c, r++, ScalerInfo("Other1",       0, 25));
    // scaler_main.Set(c, r++, ScalerInfo("Other2",       0, 26));
    scaler_main.Set(c, r++, ScalerInfo("Other3",       0, 27));
    scaler_main.Set(c, r++, ScalerInfo("Other4",       0, 28));
    scaler_main.Set(c, r++, ScalerInfo("BEAM-A",       0, 35));
    scaler_main.Set(c, r++, ScalerInfo("BEAM-B",       0, 36));
    scaler_main.Set(c, r++, ScalerInfo("BEAM-C",       0, 37));
    scaler_main.Set(c, r++, ScalerInfo("BEAM-D",       0, 38));
    scaler_main.Set(c, r++, ScalerInfo("BEAM-E",       0, 39));
    scaler_main.Set(c, r++, ScalerInfo("BEAM-F",       0, 40));
    scaler_main.Set(c, r++, ScalerInfo("BAC",        0, 18));
    scaler_main.Set(c, r++, ScalerInfo("TOF",        0, 22));
    scaler_main.Set(c, r++, ScalerInfo("AC1",        0, 23));
    scaler_main.Set(c, r++, ScalerInfo("WC",         0, 24));
    scaler_main.Set(c, r++, ScalerInfo("V792gate-4",    2, 5));
    scaler_main.Set(c, r++, ScalerInfo("V792gate-5",    2, 6));
    scaler_main.Set(c, r++, ScalerInfo("V792gate-6",    2, 7));
    scaler_main.Set(c, r++, ScalerInfo("AFT-04",     0, 83));
    scaler_main.Set(c, r++, ScalerInfo("AFT-05",     0, 84));
    scaler_main.Set(c, r++, ScalerInfo("AFT-06",     0, 85));
  }

  {
    Int_t c = ScalerAnalyzer::kRight;
    Int_t r = 0;
    scaler_main.Set(c, r++, ScalerInfo("Spill",        -1, -1));
    scaler_main.Set(c, r++, ScalerInfo("Real-Time",     0,  1));
    scaler_main.Set(c, r++, ScalerInfo("Live-Time",     0,  2));
    scaler_main.Set(c, r++, ScalerInfo("L1-Req",        0,  3));
    scaler_main.Set(c, r++, ScalerInfo("L1-Acc",        0,  4));
    // scaler_main.Set(c, r++, ScalerInfo("MstClr",        0,  5));
    // scaler_main.Set(c, r++, ScalerInfo("Clear",         0,  6));
    scaler_main.Set(c, r++, ScalerInfo("L2-Req",        0,  7));
    scaler_main.Set(c, r++, ScalerInfo("L2-Acc",        0,  8));
    scaler_main.Set(c, r++, ScalerInfo("TRIG-A",        0, 41));
    scaler_main.Set(c, r++, ScalerInfo("TRIG-B",        0, 42));
    scaler_main.Set(c, r++, ScalerInfo("TRIG-C",        0, 43));
    scaler_main.Set(c, r++, ScalerInfo("TRIG-D",        0, 44));
    scaler_main.Set(c, r++, ScalerInfo("TRIG-E",        0, 45));
    scaler_main.Set(c, r++, ScalerInfo("TRIG-F",        0, 46));
    scaler_main.Set(c, r++, ScalerInfo("TRIG-A-PS",     0, 48));
    scaler_main.Set(c, r++, ScalerInfo("TRIG-B-PS",     0, 49));
    scaler_main.Set(c, r++, ScalerInfo("TRIG-C-PS",     0, 50));
    scaler_main.Set(c, r++, ScalerInfo("TRIG-D-PS",     0, 51));
    scaler_main.Set(c, r++, ScalerInfo("TRIG-E-PS",     0, 52));
    scaler_main.Set(c, r++, ScalerInfo("TRIG-F-PS",     0, 53));
    scaler_main.Set(c, r++, ScalerInfo("TRIG-PSOR-A",   0, 54));
    scaler_main.Set(c, r++, ScalerInfo("TRIG-PSOR-B",   0, 55));
    scaler_main.Set(c, r++, ScalerInfo("Clock-PS",      0, 56));
    scaler_main.Set(c, r++, ScalerInfo("Reserve2-PS",   0, 57));
    scaler_main.Set(c, r++, ScalerInfo("Level1-PS",     0, 58));
    scaler_main.Set(c, r++, ScalerInfo("V792gate-7",    2, 8));
    scaler_main.Set(c, r++, ScalerInfo("V792gate-8",    2, 9));
    scaler_main.Set( c, r++, ScalerInfo( "AFT-07",      0, 86));
    scaler_main.Set( c, r++, ScalerInfo( "AFT-08",      0, 87));
    scaler_main.Set( c, r++, ScalerInfo( "AFT-09",      0, 88));
  }

  scaler_main.PrintFlags();

  return 0;
}

//____________________________________________________________________________
Int_t
process_end(void)
{
  if(scaler_main.GetFlag(ScalerAnalyzer::kScalerSheet))
    return 0;

  scaler_main.Print();

  return 0;

}

//____________________________________________________________________________
Int_t
process_event(void)
{
  static Int_t  event_count = 0;
  static Bool_t en_disp     = false;

  if(scaler_main.GetFlag(ScalerAnalyzer::kScalerSheet) && event_count==0)
    hddaq::cout << "waiting spill end " << std::flush;

  ++event_count;

  if(!scaler_main.Decode())
    return 0;

  if(scaler_main.GetFlag(ScalerAnalyzer::kSemiOnline)){
    if(event_count%300 == 0) en_disp = true;
  } else {
    if(event_count%10 == 0) en_disp = true;
  }

  if(scaler_main.IsSpillEnd())
    en_disp = true;

  if(scaler_main.GetFlag(ScalerAnalyzer::kScalerSheet) &&
      !scaler_main.IsSpillEnd()){
    if(event_count%100==0)
      hddaq::cout << "." << std::flush;
    return 0;
  }

  if(en_disp){
    scaler_main.Print();
    en_disp = false;
  }

  if(scaler_main.IsSpillEnd() &&
      scaler_main.GetFlag(ScalerAnalyzer::kScalerSheet)){
    hddaq::cout << "found spill end "
		<< scaler_main.Get("Spill") << "/" << nspill_scaler_sheet
		<< std::endl;

    if(scaler_main.Get("Spill") == nspill_scaler_sheet){
      scaler_main.PrintScalerSheet();
      return -1;
    }

    if(scaler_main.Get("Spill") > nspill_scaler_sheet){
      hddaq::cout << "something is wrong!" << std::endl;
      return -1;
    }

    hddaq::cout << "waiting spill end " << std::flush;
  }

  return 0;
}

}
