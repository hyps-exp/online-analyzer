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
  ConfMan::GetInstance().Initialize(argv);

  // flags
  gScaler.SetFlag( ScalerAnalyzer::kSeparateComma );
  gScaler.SetFlag( ScalerAnalyzer::kSemiOnline );
  gScaler.SetFlag( ScalerAnalyzer::kScalerE42 );
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
  // please do not use a white space character.

  {
    Int_t c = ScalerAnalyzer::kLeft;
    Int_t r = 0;
    gScaler.Set( c, r++, ScalerInfo( "10M-Clock",   0, 0 ) );
    gScaler.Set( c, r++, ScalerInfo( "BAC-SUM",     3, 0 ) );
    gScaler.Set( c, r++, ScalerInfo( "SAC-6SUM",    3, 1 ) );
    gScaler.Set( c, r++, ScalerInfo( "SAC-8SUM",    3, 2 ) );
    gScaler.Set( c, r++, ScalerInfo( "KVC-1U",      3, 3 ) );
    gScaler.Set( c, r++, ScalerInfo( "KVC-1D",      3, 4 ) );
    gScaler.Set( c, r++, ScalerInfo( "KVC-1SUM",    3, 5 ) );
    gScaler.Set( c, r++, ScalerInfo( "KVC-2U",      3, 6 ) );
    gScaler.Set( c, r++, ScalerInfo( "KVC-2D",      3, 7 ) );
    gScaler.Set( c, r++, ScalerInfo( "KVC-2SUM",    3, 8 ) );
    gScaler.Set( c, r++, ScalerInfo( "KVC-3U",      3, 9 ) );
    gScaler.Set( c, r++, ScalerInfo( "KVC-3D",      3,10 ) );
    gScaler.Set( c, r++, ScalerInfo( "KVC-3SUM",    3,11 ) );
    gScaler.Set( c, r++, ScalerInfo( "KVC-4U",      3,12 ) );
    gScaler.Set( c, r++, ScalerInfo( "KVC-4D",      3,13 ) );
    gScaler.Set( c, r++, ScalerInfo( "KVC-4SUM",    3,14 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-4U",      3,16 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-4D",      3,17 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-1MT",     3,18 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-2MT",     3,19 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-3MT",     3,20 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-4MT",     3,21 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-5MT",     3,22 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-6MT",     3,23 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-7MT",     3,24 ) );
    gScaler.Set( c, r++, ScalerInfo( "BH2-8MT",     3,25 ) );
    gScaler.Set( c, r++, ScalerInfo( "T1",          3,26 ) );
    gScaler.Set( c, r++, ScalerInfo( "T2",          3,27 ) );
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
