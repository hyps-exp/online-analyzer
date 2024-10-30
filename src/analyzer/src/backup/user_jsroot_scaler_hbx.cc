// -*- C++ -*-

// Author: Shuhei Hayakawa

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <TAxis.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TGFileBrowser.h>
#include <TH1.h>
#include <TH2.h>
#include <TGraph.h>
#include <TLegend.h>
#include <TMath.h>
#include <TPaveText.h>
#include <TStyle.h>
#include <TString.h>

#include <THttpServer.h>
#include <TKey.h>
#include <TSystem.h>
#include <TTimeStamp.h>

#include <DAQNode.hh>
#include <filesystem_util.hh>
#include <Unpacker.hh>
#include <UnpackerConfig.hh>
#include <UnpackerManager.hh>
#include <std_ostream.hh>

#include "user_analyzer.hh"

#include "ConfMan.hh"
#include "DetectorID.hh"
#include "HttpServer.hh"
#include "ScalerAnalyzer.hh"

#define CFT 0

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  namespace
  {
    HttpServer&   gHttp = HttpServer::GetInstance();
    ScalerAnalyzer scaler_on;
    ScalerAnalyzer scaler_off;
  }

//____________________________________________________________________________
Int_t
process_begin( const std::vector<std::string>& argv )
{
  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  if( !gConfMan.IsGood() ) return -1;
  hddaq::set_tag_summary( "/tmp/jsroot_scaler_hbx_tag.txt" );

  gHttp.SetPort( 9093 );
  gHttp.Open();
  gHttp.SetItemField("/","_monitoring","100");
  gHttp.SetItemField("/","_layout","vert3");
  gHttp.SetItemField("/","_drawitem","[ScalerOn,ScalerOff,Tag]");
  gHttp.CreateItem("/ScalerOn", "DAQ Scaler On");
  gHttp.SetItemField("/ScalerOn", "_kind", "Text");
  gHttp.CreateItem("/ScalerOff", "DAQ Scaler Off");
  gHttp.SetItemField("/ScalerOff", "_kind", "Text");
  gHttp.CreateItem("/Tag", "Tag Check");
  gHttp.SetItemField("/Tag", "_kind", "Text");
  std::stringstream ss;
  ss << "<div style='color: white; background-color: black;"
     << "width: 100%; height: 100%;'>"
     << "Tag cheker is running" << "</div>";
  gHttp.SetItemField("/Tag", "value", ss.str().c_str());
  gHttp.Hide("/Reset");

  scaler_on.SetFlag( ScalerAnalyzer::kSeparateComma );
  scaler_on.SetFlag( ScalerAnalyzer::kSpillBySpill );
  scaler_on.SetFlag( ScalerAnalyzer::kSpillOn );
  scaler_on.SetFlag( ScalerAnalyzer::kScalerHBX );

  scaler_off.SetFlag( ScalerAnalyzer::kSeparateComma );
  scaler_off.SetFlag( ScalerAnalyzer::kSpillBySpill );
  scaler_off.SetFlag( ScalerAnalyzer::kSpillOff );
  scaler_off.SetFlag( ScalerAnalyzer::kScalerHBX );

  //////////////////// Set Channels
  // ScalerAnalylzer::Set( Int_t column,
  //                       Int_t raw,
  //                       ScalerInfo( name, module, channel ) );
  // scaler information is defined from here.
  // please do not use a white space character.
  {
    Int_t c = ScalerAnalyzer::kLeft;
    Int_t r = 0;
    scaler_on.Set( c, r++, ScalerInfo( "Level1-PS",     0, 58 ) );
    for( Int_t i=0; i<16; ++i ){
      scaler_on.Set( c, r++, ScalerInfo( Form( "TFA-%02d", i+1 ), 2, i ) );
    }
    scaler_on.Set( c, r++, ScalerInfo( "LSO1",         2, 48 ) );
    scaler_on.Set( c, r++, ScalerInfo( "LSO2",         2, 49 ) );
    scaler_on.Set( c, r++, ScalerInfo( "LSO1xGe13",    2, 50 ) );
    scaler_on.Set( c, r++, ScalerInfo( "LSO2xGe24",    2, 51 ) );
  }

  {
    Int_t c = ScalerAnalyzer::kCenter;
    Int_t r = 0;
    scaler_on.Set( c, r++, ScalerInfo( "10M-Clock",    0,  0 ) );
    for( Int_t i=0; i<16; ++i ){
      scaler_on.Set( c, r++, ScalerInfo( Form( "CRM-%02d", i+1 ), 2, i+16 ) );
    }
    scaler_on.Set( c, r++, ScalerInfo( "LSO1High",     2, 54 ) );
    scaler_on.Set( c, r++, ScalerInfo( "LSO2High",     2, 55 ) );
    scaler_on.Set( c, r++, ScalerInfo( "GeHigh1",      2, 56 ) );
    scaler_on.Set( c, r++, ScalerInfo( "GeHigh2",      2, 57 ) );
  }

  {
    Int_t c = ScalerAnalyzer::kRight;
    Int_t r = 0;
    scaler_on.Set( c, r++, ScalerInfo( "Spill",        -1, -1 ) );
    for( Int_t i=0; i<16; ++i ){
      scaler_on.Set( c, r++, ScalerInfo( Form( "Reset-%02d", i+1 ), 2, i+32 ) );
    }
    scaler_on.Set( c, r++, ScalerInfo( "GeCoin1",      2, 52 ) );
    scaler_on.Set( c, r++, ScalerInfo( "GeCoin2",      2, 53 ) );
    scaler_on.Set( c, r++, ScalerInfo( "Other3",       0, 27 ) );
    scaler_on.Set( c, r++, ScalerInfo( "Other4",       0, 28 ) );
  }

  for( Int_t i=0; i<ScalerAnalyzer::MaxColumn; ++i ){
    for( Int_t j=0; j<ScalerAnalyzer::MaxRow; ++j ){
      scaler_off.Set( i, j, scaler_on.GetScalerInfo( i, j ) );
    }
  }

  scaler_on.PrintFlags();
  scaler_off.PrintFlags();

  return 0;
}

//____________________________________________________________________________
Int_t
process_end()
{
  return 0;
}

//____________________________________________________________________________
Int_t
process_event( void )
{
  static auto& gUnpacker = GUnpacker::get_instance();
  static auto* root = gUnpacker.get_root();
  static auto& gConfig = GConfig::get_instance();
  static const TString tout = gConfig.get_control_param( "tout" );

  static Int_t count = 0;
  count++;

  Int_t run_number = root->get_run_number();
  Int_t event_number = gUnpacker.get_event_number();

  std::stringstream ss;

  // Tag
  ss.str("");
  ss << "<div style='color: white; background-color: black;"
     << "width: 100%; height: 100%;'>";
  ss << "RUN " << run_number << "   Event " << event_number
     << "<br>";
  if( !gUnpacker.is_good() ){
    std::ifstream ifs( tout );
    if( ifs.good() ){
      TString buf;
      while( !ifs.eof() ){
	buf.ReadLine(ifs, false);
	if( buf.Contains("!") && !buf.Contains("............!") )
	  buf = "<font color='yellow'>" + buf + "</font>";
	// if( buf.Contains("bUuSELselthdg") )
	//   ss << TString(' ', 24);
	ss << buf << "<br>";
      }
      ifs.close();
      tag_summary.seekp( 0, std::ios_base::beg );
    } else {
      ss << Form( "Failed to read %s", tout.Data() );
    }
    ss << "</div>";
    gHttp.SetItemField("/Tag", "value", ss.str().c_str());
  }

  // Scaler Spill On
  if( scaler_on.Decode() ){
    if( count%50 != 0 && !scaler_on.IsSpillEnd() )
      // if( !scaler_on.IsSpillEnd() )
      return 0;

    ss.str("");
    ss << "<div style='color: white; background-color: black;"
       << "width: 100%; height: 100%;'>";
    ss << "<table border=\"0\" width=\"700\" cellpadding=\"0\">";
    TString end_mark = scaler_on.IsSpillEnd() ? "Spill End" : "";
    ss << "<tr><td width=\"100\">RUN</td><td align=\"right\" width=\"100\">"
       << scaler_on.SeparateComma( run_number ) << "</td><td width=\"100\">"
       << " : Event Number" << "</td><td align=\"right\">"
       << scaler_on.SeparateComma( event_number ) << "</td>"
       << "<td width=\"100\">" << " : " << "</td>"
       << "<td align=\"right\" width=\"100\">" << end_mark << "</td>"
       << "<tr><td></td><td></td><td></td></tr>";
    for(Int_t j=0; j<ScalerAnalyzer::MaxRow; ++j){
      ss << "<tr>";
      for(Int_t i=0; i<ScalerAnalyzer::MaxColumn; ++i){
        TString n = scaler_on.GetScalerName(i, j);
        if( n.Contains("n/a") )
          continue;
	ss << "<td>";
	if(i != 0)
	  ss << " : ";
	ss << n << "</td>"
	   << "<td align=\"right\">"
           << scaler_on.SeparateComma(scaler_on.Get(i, j)) << "</td>";
      }
      ss << "</tr>";
    }
    ss << "</table>";
    ss << "</div>";
    gHttp.SetItemField( "/ScalerOn", "value", ss.str().c_str() );
  }

  // Scaler Spill Off
  if( scaler_off.Decode() ){
    if( count%50 != 0 && !scaler_off.IsSpillEnd() )
      // if( !scaler_off.IsSpillEnd() )
      return 0;

    ss.str("");
    ss << "<div style='color: white; background-color: black;"
       << "width: 100%; height: 100%;'>";
    ss << "<table border=\"0\" width=\"700\" cellpadding=\"0\">";
    TString end_mark = scaler_off.IsSpillEnd() ? "Spill End" : "";
    ss << "<tr><td width=\"100\">RUN</td><td align=\"right\" width=\"100\">"
       << scaler_off.SeparateComma( run_number ) << "</td><td width=\"100\">"
       << " : Event Number" << "</td><td align=\"right\">"
       << scaler_off.SeparateComma( event_number ) << "</td>"
       << "<td width=\"100\">" << " : " << "</td>"
       << "<td align=\"right\" width=\"100\">" << end_mark << "</td>"
       << "<tr><td></td><td></td><td></td></tr>";
    for(Int_t j=0; j<ScalerAnalyzer::MaxRow; ++j){
      ss << "<tr>";
      for(Int_t i=0; i<ScalerAnalyzer::MaxColumn; ++i){
        TString n = scaler_off.GetScalerName(i, j);
        if( n.Contains("n/a") )
          continue;
	ss << "<td>";
	if(i != 0)
	  ss << " : ";
	ss << n << "</td>"
	   << "<td align=\"right\">"
           << scaler_off.SeparateComma(scaler_off.Get(i, j)) << "</td>";
      }
      ss << "</tr>";
    }
    ss << "</table>";
    ss << "</div>";
    gHttp.SetItemField( "/ScalerOff", "value", ss.str().c_str() );
  }

  if ( gUnpacker.is_good() ){
    ss.str("");
    ss << "<div style='color: white; background-color: black;"
       << "width: 100%; height: 100%;'>"
       << "Tag cheker is running" << "</div>";
    gHttp.SetItemField("/Tag", "value", ss.str().c_str());
  }

  if( scaler_on.IsSpillEnd() || scaler_off.IsSpillEnd() ){
    gSystem->Sleep(150);
    gSystem->ProcessEvents();
  }

  // if ( !gUnpacker.is_good() ){
  //   static const TString host( gSystem->Getenv( "HOSTNAME" ) );
  //   static auto prev_spill = scaler_on.Get( "Spill" );
  //   auto        curr_spill = scaler_on.Get( "Spill" );
  //   if( host.Contains( "k18term4" ) &&
  //     	event_number > 1 && prev_spill != curr_spill ){
  //     std::cout << "exec tagslip sound!" << std::endl;
  //     gSystem->Exec( "ssh axis@eb0 \"aplay ~/sound/tagslip.wav\" &" );
  //   }
  //   prev_spill = curr_spill;
  // }
  return 0;
}
}
