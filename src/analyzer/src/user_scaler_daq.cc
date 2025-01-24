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
  ConfMan::GetInstance().Initialize(argv);

  // flags
  gScaler.SetFlag( ScalerAnalyzer::kSeparateComma );
  gScaler.SetFlag( ScalerAnalyzer::kSemiOnline );
  gScaler.SetFlag( ScalerAnalyzer::kScalerDaq );
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
    Int_t c = 0;
    Int_t r = 0;
    gScaler.Set( c, r++, ScalerInfo( "CLK 1MHz",            2,  0 ) );
    gScaler.Set( c, r++, ScalerInfo( "Real-Time",           2,  0 ) );
    gScaler.Set( c, r++, ScalerInfo( "Live-Time",           2,  14 ) );
    gScaler.Set( c, r++, ScalerInfo( "L1-Req",              2,  1 ) );
    gScaler.Set( c, r++, ScalerInfo( "L1-Acc",              2,  2 ) );
    gScaler.Set( c, r++, ScalerInfo( "L2-Acc",              2,  3 ) );
    //gScaler.Set( c, r++, ScalerInfo( "T0xTOFx/E-Veto",  2,  8 ) );
    gScaler.Set( c, r++, ScalerInfo( "RF",                       1,  81 ) );
    gScaler.Set( c, r++, ScalerInfo( "Tagger-COIN-All",              1,  82 ) );
    gScaler.Set( c, r++, ScalerInfo( "T0",                  2,  4 ) );
    gScaler.Set( c, r++, ScalerInfo( "UpVeto",  2,  8 ) );
    gScaler.Set( c, r++, ScalerInfo( "SAC-Sum",             2,  6 ) );
    gScaler.Set( c, r++, ScalerInfo( "E-Veto",              2,  7 ) );
    gScaler.Set( c, r++, ScalerInfo( "TOFOR",              2,  5 ) );

    gScaler.Set( c, r++, ScalerInfo( "Tagger-COIN1",                 1,  94 ) );
    gScaler.Set( c, r++, ScalerInfo( "Tagger-COIN2",                 1,  95 ) );
    gScaler.Set( c, r++, ScalerInfo( "T0-MT",                  1,  90 ) );
    gScaler.Set( c, r++, ScalerInfo( "T0(L)",                    1,  83 ) );
    gScaler.Set( c, r++, ScalerInfo( "T0(R)",                    1,  84 ) );
    gScaler.Set( c, r++, ScalerInfo( "SAC-Sum(in-hatch)",                  1,  85 ) );
    gScaler.Set( c, r++, ScalerInfo( "E-Veto-MT",              1,  91 ) );
    gScaler.Set( c, r++, ScalerInfo( "E-Veto(L)",                1,  86 ) );
    gScaler.Set( c, r++, ScalerInfo( "E-Veto(R)",                1,  87 ) );
    gScaler.Set( c, r++, ScalerInfo( "TOF-MT(UOR-1,DOR-1)",  1,  88 ) );
    gScaler.Set( c, r++, ScalerInfo( "TOF-MT(UOR-2,DOR-2)",  1,  89 ) );
    gScaler.Set( c, r++, ScalerInfo( "CFT Phi1 OR",         2,  10 ) );
    gScaler.Set( c, r++, ScalerInfo( "CFT Phi2 OR",         2,  11 ) );
    gScaler.Set( c, r++, ScalerInfo( "CFT Phi3 OR",         2,  12 ) );
    gScaler.Set( c, r++, ScalerInfo( "CFT Phi4 OR",         2,  13 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO",                 2,  9 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-OR1",                 1,  92 ) );
    gScaler.Set( c, r++, ScalerInfo( "BGO-OR2",                 1,  93 ) );
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
    if(gScaler.Get("CLK 1MHz") >5900000){
      gScaler.Print();
    }
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
