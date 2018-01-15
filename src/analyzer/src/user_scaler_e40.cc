// -*- C++ -*-

// Author: Tomonori Takahashi

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

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  namespace
  {
    UnpackerManager& gUnpacker = GUnpacker::get_instance();
    ScalerAnalyzer&  gScaler   = ScalerAnalyzer::GetInstance();
    ///// Number of spill for Scaler Sheet
    Scaler nspill_scaler_sheet  = 1;
  }

//____________________________________________________________________________
Int_t
process_begin( const std::vector<std::string>& argv )
{
  ConfMan::getInstance().initialize(argv);

  // flags
  gScaler.SetFlag( ScalerAnalyzer::kSeparateComma );
  gScaler.SetFlag( ScalerAnalyzer::kSemiOnline );
  for( Int_t i=0, n=argv.size(); i<n; ++i ){
    TString v = argv[i];
    if( v.Contains("--print") ){
      gScaler.SetFlag( ScalerAnalyzer::kScalerSheet );
    }
    if( v.Contains("--spill=") ){
      nspill_scaler_sheet = v.ReplaceAll("--spill=","").Atoi();
    }
    if( v.Contains(":") ){
      gScaler.SetFlag( ScalerAnalyzer::kSemiOnline, false );
    }
    if( v.Contains("--spill-by-spill") ){
      gScaler.SetFlag( ScalerAnalyzer::kSpillBySpill );
    }
  }

  //////////////////// Set Channels
  // ScalerAnalylzer::Set( Int_t column,
  //                       Int_t raw,
  //                       ScalerInfo( name, module, channel ) );
  // scaler information is defined from here.
  // using a space character is not recommended.
  {
    Int_t c = ScalerAnalyzer::kLeft;
    Int_t r = 0;
    gScaler.Set( c, r++, ScalerInfo( "10M-Clock",  0, 50 ) );
    gScaler.Set( c, r++, ScalerInfo( "Beam",       0,  0 ) );
    gScaler.Set( c, r++, ScalerInfo( "pi-Beam",    0,  1 ) );
    gScaler.Set( c, r++, ScalerInfo( "p-Beam",     0,  2 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH1",        0,  3 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH1-SUM",   -1, -1 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH1-01",     0,  4 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH1-02",     0,  5 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH1-03",     0,  6 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH1-04",     0,  7 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH1-05",     0,  8 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH1-06",     0,  9 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH1-07",     0, 10 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH1-08",     0, 11 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH1-09",     0, 12 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH1-10",     0, 13 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH1-11",     0, 14 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2",        0, 15 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-SUM",   -1, -1 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-01",     0, 16 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-02",     0, 17 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-03",     0, 18 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-04",     0, 19 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-05",     0, 20 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-06",     0, 21 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-07",     0, 22 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-08",     0, 23 ) );
    gScaler.Set( c, r++, ScalerInfo( "SAC",        0, 24 ) );
    gScaler.Set( c, r++, ScalerInfo( "SCH",        0, 25 ) );
    gScaler.Set( c, r++, ScalerInfo( "TOF",        0, 26 ) );
    gScaler.Set( c, r++, ScalerInfo( "TOF-HT",     0, 27 ) );
    gScaler.Set( c, r++, ScalerInfo( "TOF-24",     0, 63 ) );
    gScaler.Set( c, r++, ScalerInfo( "LC",         0, 28 ) );
  }

  {
    Int_t c = ScalerAnalyzer::kCenter;
    Int_t r = 0;
    gScaler.Set( c, r++, ScalerInfo( "BGO-03",         1, 32 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-04",         1, 33 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-05",         1, 34 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-06",         1, 35 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-07",         1, 36 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-08",         1, 37 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-09",         1, 38 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-10",         1, 39 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-11",         1, 40 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-12",         1, 41 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-13",         1, 42 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-14",         1, 43 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-15",         1, 44 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-16",         1, 45 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-17",         1, 46 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-18",         1, 47 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-19",         1, 48 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-20",         1, 49 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-21",         1, 50 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-22",         1, 51 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-23",         1, 52 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-24",         1, 53 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-OR",         1, 54 ) );
    gScaler.Set( c, r++, ScalerInfo( "CFT-Phi1",       1, 55 ) );
    gScaler.Set( c, r++, ScalerInfo( "CFT-Phi2",       1, 56 ) );
    gScaler.Set( c, r++, ScalerInfo( "CFT-Phi3",       1, 57 ) );
    gScaler.Set( c, r++, ScalerInfo( "CFT-Phi4",       1, 58 ) );
    gScaler.Set( c, r++, ScalerInfo( "PiID",           1, 59 ) );
    gScaler.Set( c, r++, ScalerInfo( "PiScat",         1, 60 ) );
    gScaler.Set( c, r++, ScalerInfo( "pScat",          1, 61 ) );
  }


  {
    Int_t c = ScalerAnalyzer::kRight;
    Int_t r = 0;
    gScaler.Set( c, r++, ScalerInfo( "Spill",         0, 49 ) );
    gScaler.Set( c, r++, ScalerInfo( "IM",            0, 51 ) );
    gScaler.Set( c, r++, ScalerInfo( "TM",            0, 52 ) );
    gScaler.Set( c, r++, ScalerInfo( "Real-Time",     0, 53 ) );
    gScaler.Set( c, r++, ScalerInfo( "Live-Time",     0, 54 ) );
    gScaler.Set( c, r++, ScalerInfo( "L1-Req",        0, 55 ) );
    gScaler.Set( c, r++, ScalerInfo( "L1-Acc",        0, 56 ) );
    gScaler.Set( c, r++, ScalerInfo( "Matrix",        0, 57 ) );
    gScaler.Set( c, r++, ScalerInfo( "Mst-Acc",       0, 58 ) );
    gScaler.Set( c, r++, ScalerInfo( "Mst-Clr",       0, 59 ) );
    gScaler.Set( c, r++, ScalerInfo( "Clear",         0, 60 ) );
    gScaler.Set( c, r++, ScalerInfo( "L2-Req",        0, 61 ) );
    gScaler.Set( c, r++, ScalerInfo( "L2-Acc",        0, 62 ) );
    gScaler.Set( c, r++, ScalerInfo( "(pi,TOF)",      0, 34 ) );
    gScaler.Set( c, r++, ScalerInfo( "(pi,pi)",       0, 35 ) );
    gScaler.Set( c, r++, ScalerInfo( "(pi,p)",        0, 36 ) );
    gScaler.Set( c, r++, ScalerInfo( "Coin1",         0, 37 ) );
    gScaler.Set( c, r++, ScalerInfo( "Coin2",         0, 38 ) );
    gScaler.Set( c, r++, ScalerInfo( "CoinE03",       0, 39 ) );
    gScaler.Set( c, r++, ScalerInfo( "(BH2,K)",       0, 40 ) );
    gScaler.Set( c, r++, ScalerInfo( "Beam-PS",       0, 41 ) );
    gScaler.Set( c, r++, ScalerInfo( "(pi,TOF)-PS",   0, 42 ) );
    gScaler.Set( c, r++, ScalerInfo( "(pi,pi)-PS",    0, 43 ) );
    gScaler.Set( c, r++, ScalerInfo( "(pi,p)-PS",     0, 44 ) );
    gScaler.Set( c, r++, ScalerInfo( "Coin1-PS",      0, 45 ) );
    gScaler.Set( c, r++, ScalerInfo( "Coin2-PS",      0, 46 ) );
    gScaler.Set( c, r++, ScalerInfo( "CoinE03-PS",    0, 47 ) );
    gScaler.Set( c, r++, ScalerInfo( "Kscat",         0, 48 ) );
    gScaler.Set( c, r++, ScalerInfo( "Other1",        0, 29 ) );
    gScaler.Set( c, r++, ScalerInfo( "Other2",        0, 30 ) );
    gScaler.Set( c, r++, ScalerInfo( "Other3",        0, 31 ) );
    gScaler.Set( c, r++, ScalerInfo( "Other4",        0, 32 ) );
    gScaler.Set( c, r++, ScalerInfo( "Other5",        0, 33 ) );
  }

  gScaler.PrintFlags();

  return 0;
}

//____________________________________________________________________________
Int_t
process_end( void )
{
  if( gScaler.GetFlag( ScalerAnalyzer::kScalerSheet ) )
    return 0;

  gScaler.Print();

  return 0;

}

//____________________________________________________________________________
Int_t
process_event( void )
{
  static Int_t  event_count = 0;
  static Bool_t en_disp     = false;

  if( gScaler.GetFlag( ScalerAnalyzer::kScalerSheet ) && event_count==0 )
    std::cout << "waiting spill end " << std::flush;

  ++event_count;

  gScaler.Decode();

  if( gScaler.GetFlag( ScalerAnalyzer::kSemiOnline ) ){
    if( event_count%300 == 0 ) en_disp = true;
  } else {
    if( event_count%10 == 0 ) en_disp = true;
  }

  if( gScaler.IsSpillEnd() )
    en_disp = true;

  if( gScaler.GetFlag( ScalerAnalyzer::kScalerSheet ) &&
      !gScaler.IsSpillEnd() ){
    if( event_count%100==0 )
      std::cout << "." << std::flush;
    return 0;
  }

  if( en_disp ){
    gScaler.Print();
    en_disp = false;
  }

  if( gScaler.IsSpillEnd() &&
      gScaler.GetFlag( ScalerAnalyzer::kScalerSheet ) ){
    std::cout << "found spill end "
    	      << gScaler.Get("Spill") << "/" << nspill_scaler_sheet
	      << std::endl;

    if( gScaler.Get("Spill") == nspill_scaler_sheet ){
      gScaler.PrintScalerSheet();
      return -1;
    }

    if( gScaler.Get("Spill") > nspill_scaler_sheet ){
      std::cout << "something is wrong!" << std::endl;
      return -1;
    }

    std::cout << "waiting spill end " << std::flush;
  }

  return 0;
}

}
