// -*- C++ -*-

// Author: Shuhei Hayakawa

#include <chrono>
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
HttpServer& gHttp = HttpServer::GetInstance();
ScalerAnalyzer scaler_on;
ScalerAnalyzer scaler_off;
const std::chrono::milliseconds flush_interval(100);
}

//____________________________________________________________________________
Int_t
process_begin(const std::vector<std::string>& argv)
{
  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);
  if(!gConfMan.IsGood()) return -1;

  gHttp.SetPort(9091);
  gHttp.Open();
  gHttp.SetItemField("/", "_monitoring", "100");
  gHttp.SetItemField("/", "_layout", "vert3");
  gHttp.SetItemField("/", "_drawitem", "[ScalerOn,ScalerOff,Tag]");
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

  scaler_on.SetFlag(ScalerAnalyzer::kSeparateComma);
  scaler_on.SetFlag(ScalerAnalyzer::kSpillBySpill);
  scaler_on.SetFlag(ScalerAnalyzer::kSpillOn);

  scaler_off.SetFlag(ScalerAnalyzer::kSeparateComma);
  scaler_off.SetFlag(ScalerAnalyzer::kSpillBySpill);
  scaler_off.SetFlag(ScalerAnalyzer::kSpillOff);

  //////////////////// Set Channels
  // ScalerAnalylzer::Set(Int_t column,
  //                       Int_t raw,
  //                       ScalerInfo(name, module, channel));
  // scaler information is defined from here.
  // please do not use a white space character.
  {
    Int_t c = ScalerAnalyzer::kLeft;
    Int_t r = 0;
    scaler_on.Set(c, r++, ScalerInfo("CLK 1MHz",            2,  0));
    scaler_on.Set(c, r++, ScalerInfo("Real-Time",           2,  0));
    scaler_on.Set(c, r++, ScalerInfo("Live-Time",           2,  14));
    scaler_on.Set(c, r++, ScalerInfo("L1-Req",              2,  1));
    scaler_on.Set(c, r++, ScalerInfo("L1-Acc",              2,  2));
    scaler_on.Set(c, r++, ScalerInfo("L2-Acc",              2,  3));
    //scaler_on.Set(c, r++, ScalerInfo("T0xTOFx/E-Veto",  2,  8));
    scaler_on.Set(c, r++, ScalerInfo("RF",                       1,  81));
    scaler_on.Set(c, r++, ScalerInfo("Tagger-COIN-All",              1,  82));
    scaler_on.Set(c, r++, ScalerInfo("T0",                  2,  4));
    scaler_on.Set(c, r++, ScalerInfo("UpVeto",  2,  8));
    scaler_on.Set(c, r++, ScalerInfo("SAC-Sum",             2,  6));
    scaler_on.Set(c, r++, ScalerInfo("E-Veto",              2,  7));
    scaler_on.Set(c, r++, ScalerInfo("TOFOR",              2,  5));

    scaler_on.Set(c, r++, ScalerInfo("Tagger-COIN1",                 1,  94));
    scaler_on.Set(c, r++, ScalerInfo("Tagger-COIN2",                 1,  95));
    scaler_on.Set(c, r++, ScalerInfo("T0-MT",                  1,  90));
    scaler_on.Set(c, r++, ScalerInfo("T0(L)",                    1,  83));
    scaler_on.Set(c, r++, ScalerInfo("T0(R)",                    1,  84));
    scaler_on.Set(c, r++, ScalerInfo("SAC-Sum(in-hatch)",                  1,  85));
    scaler_on.Set(c, r++, ScalerInfo("E-Veto-MT",              1,  91));
    scaler_on.Set(c, r++, ScalerInfo("E-Veto(L)",                1,  86));
    scaler_on.Set(c, r++, ScalerInfo("E-Veto(R)",                1,  87));
    scaler_on.Set(c, r++, ScalerInfo("TOF-MT(UOR-1,DOR-1)",  1,  88));
    scaler_on.Set(c, r++, ScalerInfo("TOF-MT(UOR-2,DOR-2)",  1,  89));
    scaler_on.Set(c, r++, ScalerInfo("CFT Phi1 OR",         2,  10));
    scaler_on.Set(c, r++, ScalerInfo("CFT Phi2 OR",         2,  11));
    scaler_on.Set(c, r++, ScalerInfo("CFT Phi3 OR",         2,  12));
    scaler_on.Set(c, r++, ScalerInfo("CFT Phi4 OR",         2,  13));
    scaler_on.Set(c, r++, ScalerInfo("BGO",                 2,  9));
    scaler_on.Set(c, r++, ScalerInfo("BGO-OR1",                 1,  92));
    scaler_on.Set(c, r++, ScalerInfo("BGO-OR2",                 1,  93));
  }

  for(Int_t i=0; i<ScalerAnalyzer::MaxColumn; ++i){
    for(Int_t j=0; j<ScalerAnalyzer::MaxRow; ++j){
      scaler_off.Set(i, j, scaler_on.GetScalerInfo(i, j));
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
process_event()
{
  static auto& gUnpacker = GUnpacker::get_instance();
  static auto* root = gUnpacker.get_root();
  static auto& gConfig = GConfig::get_instance();
  static const TString tout = gConfig.get_control_param("tout");

  Int_t run_number = root->get_run_number();
  Int_t event_number = gUnpacker.get_event_number();

  std::stringstream ss;

  // Tag
  ss.str("");
  ss << "<div style='color: white; background-color: black;"
     << "width: 100%; height: 100%;'>";
  ss << "RUN " << run_number << "   Event " << event_number
     << "<br>";
  if(!gUnpacker.is_good()){
    std::ifstream ifs(tout);
    if(ifs.good()){
      TString buf;
      while(!ifs.eof()){
	buf.ReadLine(ifs, false);
	if(buf.Contains("!") && !buf.Contains("............!"))
	  buf = "<font color='yellow'>" + buf + "</font>";
	// if(buf.Contains("bUuSELselthdg"))
	//   ss << TString(' ', 24);
	ss << buf << "<br>";
      }
      ifs.close();
      tag_summary.seekp(0, std::ios_base::beg);
    } else {
      ss << Form("Failed to read %s", tout.Data());
    }
    ss << "</div>";
    gHttp.SetItemField("/Tag", "value", ss.str().c_str());
  }

  const Int_t MaxDispRow = 29;

  // Scaler Spill On
  auto now = std::chrono::duration_cast<std::chrono::milliseconds>
    (std::chrono::system_clock::now().time_since_epoch());
  static auto prev_flush = now;
  Bool_t flush_flag = true;//(now - prev_flush) > flush_interval;
  prev_flush = now;

  if(scaler_on.Decode()){
    if(flush_flag && !scaler_on.IsSpillEnd())
      // if(!scaler_on.IsSpillEnd())
      return 0;

    ss.str("");
    ss << "<div style='color: white; background-color: black;"
       << "width: 100%; height: 100%;'>";
    ss << "<table border=\"0\" width=\"700\" cellpadding=\"0\">";
    TString end_mark = scaler_on.IsSpillEnd() ? "Spill End" : "";
    ss << "<tr><td width=\"100\">RUN</td><td align=\"right\" width=\"100\">"
       << scaler_on.SeparateComma(run_number) << "</td><td width=\"100\">"
       << " : Event Number" << "</td><td align=\"right\">"
       << scaler_on.SeparateComma(event_number) << "</td>"
       << "<td width=\"100\">" << " : " << "</td>"
       << "<td align=\"right\" width=\"100\">" << end_mark << "</td>"
       << "<tr><td></td><td></td><td></td></tr>";
    for(Int_t j=0; j<MaxDispRow; ++j){
      ss << "<tr>";
      for(Int_t i=0; i<ScalerAnalyzer::MaxColumn; ++i){
	// for(Int_t i=0; i<ScalerAnalyzer::MaxColumn; ++i){
        TString n = scaler_on.GetScalerName(i, j);
        // if(n.Contains("n/a"))
        //   continue;
	ss << "<td>";
	if(i != 0)
	  ss << " : ";
	ss << n << "</td>"
	   << "<td align=\"right\">"
           << scaler_on.SeparateComma(scaler_on.Get(i, j)) << "</td>";
      }
      ss << "</tr>";
    }
    ss << "<tr><td></td><td></td><td></td></tr>"
       << "<tr><td>K-Beam/TM</td>"
       << "<td align=\"right\">"
       << Form("%.6f", scaler_on.Fraction("K-Beam", "TM"))
       << "</td>"
       << "<td> : Live/Real</td>"
       << "<td align=\"right\">"
       << Form("%.6f", scaler_on.Fraction("Live-Time", "Real-Time"))
       << "</td>"
       << "<td> : DAQ-Eff</td>"
       << "<td align=\"right\">"
       << Form("%.6f", scaler_on.Fraction("L1-Acc", "L1-Req"))
       << "</td>"
       << "</tr></tr>"
       << "<td>L1Req/K-Beam</td>"
       << "<td align=\"right\">"
       << Form("%.6f", scaler_on.Fraction("L1-Req", "K-Beam"))
       << "</td>"
       << "<td> : L2-Eff</td>"
       << "<td align=\"right\">"
       << Form("%.6f", scaler_on.Fraction("L2-Acc", "L1-Acc"))
       << "</td>"
       << "<td> : Duty-Factor</td>"
       << "<td align=\"right\">"
       << Form("%.6f", scaler_on.Duty())
       << "</td>"
       << "</tr>";
    ss << "</table>";
    ss << "</div>";
    gHttp.SetItemField("/ScalerOn", "value", ss.str().c_str());
  }

  // Scaler Spill Off
  if(scaler_off.Decode()){
    if(flush_flag && !scaler_off.IsSpillEnd())
      // if(!scaler_off.IsSpillEnd())
      return 0;

    ss.str("");
    ss << "<div style='color: white; background-color: black;"
       << "width: 100%; height: 100%;'>";
    ss << "<table border=\"0\" width=\"700\" cellpadding=\"0\">";
    TString end_mark = scaler_off.IsSpillEnd() ? "Spill End" : "";
    ss << "<tr><td width=\"100\">RUN</td><td align=\"right\" width=\"100\">"
       << scaler_off.SeparateComma(run_number) << "</td><td width=\"100\">"
       << " : Event Number" << "</td><td align=\"right\">"
       << scaler_off.SeparateComma(event_number) << "</td>"
       << "<td width=\"100\">" << " : " << "</td>"
       << "<td align=\"right\" width=\"100\">" << end_mark << "</td>"
       << "<tr><td></td><td></td><td></td></tr>";
    for(Int_t j=0; j<MaxDispRow; ++j){
      // for(Int_t j=0; j<ScalerAnalyzer::MaxRow; ++j){
      ss << "<tr>";
      for(Int_t i=0; i<ScalerAnalyzer::MaxColumn; ++i){
        TString n = scaler_off.GetScalerName(i, j);
        // if(n.Contains("n/a"))
        //   continue;
	ss << "<td>";
	if(i != 0)
	  ss << " : ";
	ss << n << "</td>"
	   << "<td align=\"right\">"
           << scaler_off.SeparateComma(scaler_off.Get(i, j)) << "</td>";
      }
      ss << "</tr>";
    }
    ss << "<tr><td></td><td></td><td></td></tr>"
       << "<tr><td>K-Beam/TM</td>"
       << "<td align=\"right\">"
       << Form("%.6f", scaler_off.Fraction("K-Beam", "TM"))
       << "</td>"
       << "<td> : Live/Real</td>"
       << "<td align=\"right\">"
       << Form("%.6f", scaler_off.Fraction("Live-Time", "Real-Time"))
       << "</td>"
       << "<td> : DAQ-Eff</td>"
       << "<td align=\"right\">"
       << Form("%.6f", scaler_off.Fraction("L1-Acc", "L1-Req"))
       << "</td>"
       << "</tr></tr>"
       << "<td>L1Req/K-Beam</td>"
       << "<td align=\"right\">"
       << Form("%.6f", scaler_off.Fraction("L1-Req", "K-Beam"))
       << "</td>"
       << "<td> : L2-Eff</td>"
       << "<td align=\"right\">"
       << Form("%.6f", scaler_off.Fraction("L2-Acc", "L1-Acc"))
       << "</td>"
       << "<td> : Duty-Factor</td>"
       << "<td align=\"right\">"
       << Form("%.6f", scaler_off.Duty())
       << "</td>"
       << "</tr>";
    ss << "</table>";
    ss << "</div>";
    gHttp.SetItemField("/ScalerOff", "value", ss.str().c_str());
  }

  { ///// Tag Checker
    static const auto& gConfig = GConfig::get_instance();
    static const TString tout(gConfig.get_control_param("tout"));
    std::stringstream ss;
    ss << "<div style='color: white; background-color: black;"
       << "width: 100%; height: 100%;'>";
    ss << "RUN " << run_number << "   Event " << event_number
       << "<br>";
    if(!gUnpacker.is_good()){
      std::ifstream ifs(tout);
      if(ifs.good()){
	TString buf;
	while(!ifs.eof()){
	  buf.ReadLine(ifs, false);
	  if(buf.Contains("!") && !buf.Contains("............!"))
	    buf = "<font color='yellow'>" + buf + "</font>";
	  // if(buf.Contains("bUuSELselthdg")){
	  //   ss << TString(' ', 24);
	  ss << buf << "<br>";
	}
	ifs.close();
	hddaq::tag_summary.seekp(0, std::ios_base::beg);
      }else{
	ss << Form("Failed to read %s", tout.Data());
      }
      ss << "</div>";
      gHttp.SetItemField("/Tag", "value", ss.str().c_str());
    }
  }

  if(scaler_on.IsSpillEnd() || scaler_off.IsSpillEnd()){
    gSystem->Sleep(150);
    gSystem->ProcessEvents();
  }

  gSystem->ProcessEvents();
  return 0;
}
}
