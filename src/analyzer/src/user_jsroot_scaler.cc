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
#include "RMAnalyzer.hh"
#include "ScalerAnalyzer.hh"

namespace analyzer
{
using namespace hddaq::unpacker;
using namespace hddaq;

namespace
{
HttpServer& gHttp = HttpServer::GetInstance();
auto& gRM = RMAnalyzer::GetInstance();
ScalerAnalyzer gScaler;
const std::chrono::milliseconds flush_interval(100);
const Int_t alert_interval = 10;
Scaler tagger_rate_threshold = 0;
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
  gHttp.SetItemField("/", "_layout", "vert2");
  gHttp.SetItemField("/", "_drawitem", "[ScalerOn,Tag]");
  gHttp.CreateItem("/ScalerOn", "DAQ Scaler On");
  gHttp.SetItemField("/ScalerOn", "_kind", "Text");
  gHttp.CreateItem("/Tag", "Tag Check");
  gHttp.SetItemField("/Tag", "_kind", "Text");
  std::stringstream ss;
  ss << "<div style='color: white; background-color: black;"
     << "width: 100%; height: 100%;'>"
     << "Tag cheker is running" << "</div>";
  gHttp.SetItemField("/Tag", "value", ss.str().c_str());
  gHttp.Hide("/Reset");

  std::ifstream ifs("/misc/subdata/scaler/tagger_rate_threshold.txt");
  TString line;
  if (ifs.is_open()) {
    while (line.ReadLine(ifs)) {
      line = line.Strip(TString::kBoth); // Remove space before and after the strings
      if (line.IsNull() || line.BeginsWith("#")) {
	continue;
      } else {
	tagger_rate_threshold = line.Atof();
	std::cout << "#D Tagger rate threshold is set to " << tagger_rate_threshold << std::endl;
	break;
      }
    }
  }

  gScaler.SetFlag(ScalerAnalyzer::kSeparateComma);
  gScaler.SetFlag(ScalerAnalyzer::kSemiOnline);
  gScaler.SetFlag(ScalerAnalyzer::kSpillBySpill);

  //////////////////// Set Channels
  // ScalerAnalylzer::Set(Int_t column,
  //                       Int_t raw,
  //                       ScalerInfo(name, module, channel));
  // scaler information is defined from here.
  // please do not use a white space character.

  for(Int_t c=ScalerAnalyzer::kLeft; c<ScalerAnalyzer::kRight; ++c)
    {
      // Int_t c = ScalerAnalyzer::kLeft;
      Int_t r = 0;
      gScaler.Set(c, r++, ScalerInfo("CLK-1MHz",      2,    0));
      gScaler.Set(c, r++, ScalerInfo("Real-Time",     2,    0));
      gScaler.Set(c, r++, ScalerInfo("Live-Time",     2,    1));
      gScaler.Set(c, r++, ScalerInfo("L1-Req",        2,    2));
      gScaler.Set(c, r++, ScalerInfo("L1-Acc",        2,    3));
      gScaler.Set(c, r++, ScalerInfo("L2-Acc",        2,    4));
      gScaler.Set(c, r++, ScalerInfo("CLEAR",         2,    5));
      gScaler.Set(c, r++, ScalerInfo("QDC-GATE",      1,   88));
      gScaler.Set(c, r++, ScalerInfo("TAG-ALL",       2,    8));
      gScaler.Set(c, r++, ScalerInfo("Trig-A",        2,   89));
      gScaler.Set(c, r++, ScalerInfo("Trig-B",        2,   90));
      gScaler.Set(c, r++, ScalerInfo("Trig-C",        2,   91));
      gScaler.Set(c, r++, ScalerInfo("Trig-D",        2,   92));
      gScaler.Set(c, r++, ScalerInfo("Trig-E",        2,   93));
      gScaler.Set(c, r++, ScalerInfo("Trig-F",        2,   94));
      gScaler.Set(c, r++, ScalerInfo("TRIG-A-PS",     2,   32));
      gScaler.Set(c, r++, ScalerInfo("TRIG-B-PS",     2,   33));
      gScaler.Set(c, r++, ScalerInfo("TRIG-C-PS",     2,   34));
      gScaler.Set(c, r++, ScalerInfo("TRIG-D-PS",     2,   35));
      gScaler.Set(c, r++, ScalerInfo("TRIG-E-PS",     2,   36));
      gScaler.Set(c, r++, ScalerInfo("TRIG-F-PS",     2,   37));
      gScaler.Set(c, r++, ScalerInfo("TRIG-PSOR-A",   2,   38));
      gScaler.Set(c, r++, ScalerInfo("TRIG-PSOR-B",   2,   39));
      gScaler.Set(c, r++, ScalerInfo("Clock-PS",      2,   40));
      gScaler.Set(c, r++, ScalerInfo("Rsv2-PS",       2,   41));
      gScaler.Set(c, r++, ScalerInfo("Level1-OR",     2,   42));
      gScaler.Set(c, r++, ScalerInfo("RF",            1,   81));
      gScaler.Set(c, r++, ScalerInfo("TAG-SF-OR",     2,    6));
      gScaler.Set(c, r++, ScalerInfo("TAG-PL-OR",     2,    7));
      gScaler.Set(c, r++, ScalerInfo("U-Veto",        2,   66));
      gScaler.Set(c, r++, ScalerInfo("T0",            2,   65));
      gScaler.Set(c, r++, ScalerInfo("SAC",           2,   68));
      gScaler.Set(c, r++, ScalerInfo("E-Veto",        2,   69));
      gScaler.Set(c, r++, ScalerInfo("TOF",           2,    9));
      gScaler.Set(c, r++, ScalerInfo("TOF-HUL",       2,   71));
      // gScaler.Set(c, r++, ScalerInfo("TOFOR-1",       2,    6));
      // gScaler.Set(c, r++, ScalerInfo("TOFOR-2",       2,    7));
      // gScaler.Set(c, r++, ScalerInfo("T0",          1,  90));
      // gScaler.Set(c, r++, ScalerInfo("T0-L",        1,  83));
      // gScaler.Set(c, r++, ScalerInfo("T0-R",        1,  84));
      // gScaler.Set(c, r++, ScalerInfo("AC",          1,  85));
      // gScaler.Set(c, r++, ScalerInfo("E-Veto-MT",   1,  91));
      // gScaler.Set(c, r++, ScalerInfo("E-Veto-L",    1,  86));
      // gScaler.Set(c, r++, ScalerInfo("E-Veto-R",    1,  87));
      // gScaler.Set(c, r++, ScalerInfo("TOF-MT-2",    1,  89));
      gScaler.Set(c, r++, ScalerInfo("CFT-Phi1-OR",   2,  10));
      gScaler.Set(c, r++, ScalerInfo("CFT-Phi2-OR",   2,  11));
      gScaler.Set(c, r++, ScalerInfo("CFT-Phi3-OR",   2,  12));
      gScaler.Set(c, r++, ScalerInfo("CFT-Phi4-OR",   2,  13));
      gScaler.Set(c, r++, ScalerInfo("BGO",           2,  14));
      // gScaler.Set(c, r++, ScalerInfo("BGO-OR1",     1,  92));
      // gScaler.Set(c, r++, ScalerInfo("BGO-OR2",     1,  93));
    }

  // {
  //   Int_t c = ScalerAnalyzer::kRight;
  //   Int_t r = 0;
  //   gScaler.Set(c, r++, ScalerInfo("Up-Veto",  2,  98));
  //   gScaler.Set(c, r++, ScalerInfo("T0",       2,  97));
  //   gScaler.Set(c, r++, ScalerInfo("SAC",      2,  100));
  //   gScaler.Set(c, r++, ScalerInfo("E-Veto",   2,  101));
  //   gScaler.Set(c, r++, ScalerInfo("TOF-OR",   2,  103));
  //   gScaler.Set(c, r++, ScalerInfo("BGO",      2,  107));
  //   gScaler.Set(c, r++, ScalerInfo("CFT",      2,  108));
  //   gScaler.Set(c, r++, ScalerInfo("L1-Acc",   2,  112));
  //   gScaler.Set(c, r++, ScalerInfo("TAG",      2,  113));
  //   gScaler.Set(c, r++, ScalerInfo("Beam-A",   2,  115));
  //   gScaler.Set(c, r++, ScalerInfo("Beam-B",   2,  116));
  //   gScaler.Set(c, r++, ScalerInfo("Beam-C",   2,  117));
  //   gScaler.Set(c, r++, ScalerInfo("Beam-D",   2,  118));
  //   gScaler.Set(c, r++, ScalerInfo("Beam-E",   2,  119));
  //   gScaler.Set(c, r++, ScalerInfo("Beam-F",   2,  120));
  // }

  gScaler.PrintFlags();

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
  static auto prev_run = run_number;
  Int_t event_number = gUnpacker.get_event_number();

  gRM.Decode();
  static auto prev_spill_mtm = gRM.SpillNumber();
  static Int_t overflow = 0;
  if(run_number != prev_run){
    overflow = 0;
  }
  if(run_number == prev_run && gRM.SpillNumber() < prev_spill_mtm){
    ++overflow;
  }
  Int_t spill_number = 256 * overflow + gRM.SpillNumber();
  prev_run = run_number;
  prev_spill_mtm = gRM.SpillNumber();

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

  const Int_t MaxDispRow = 39;

  // Scaler Spill On
  auto now = std::chrono::duration_cast<std::chrono::milliseconds>
    (std::chrono::system_clock::now().time_since_epoch());
  // static auto prev_flush = now;
  // Bool_t flush_flag = (now - prev_flush) > flush_interval;
  // prev_flush = now;

  gScaler.Decode();

  Bool_t flush_flag = (gScaler.Get("CLK-1MHz") > 5.9e6);

  if(flush_flag){
    std::ofstream ofs("/misc/subdata/scaler/spill.txt");
    TTimeStamp ts;
    ts.Add(-TTimeStamp::GetZoneOffset());
    ofs << ts.AsString("s") << std::endl;
    // if(!gScaler.IsSpillEnd())
    ss.str("");
    ss << "<div style='color: white; background-color: black;"
       << "width: 100%; height: 100%;'>";
    ss << "<table border=\"0\" width=\"700\" cellpadding=\"0\">";
    // TString end_mark = gScaler.IsSpillEnd() ? "Spill End" : "";
    auto end_mark = gScaler.SeparateComma(spill_number);
    ss << "<tr><td width=\"100\">RUN</td><td align=\"right\" width=\"100\">"
       << gScaler.SeparateComma(run_number) << "</td><td width=\"100\">"
       << " : Event Number" << "</td><td align=\"right\">"
       << gScaler.SeparateComma(event_number) << "</td>"
       << "<td width=\"100\">" << " : Spill Number" << "</td>"
       << "<td align=\"right\" width=\"100\">" << end_mark << "</td>"
       << "<tr><td></td><td></td><td></td></tr>";
    ofs << Form("%-20s", "Run") << run_number << std::endl
	<< Form("%-20s", "Spill") << spill_number << std::endl
	<< Form("%-20s", "Event") << event_number << std::endl;
    for(Int_t j=0; j<MaxDispRow; ++j){
      ss << "<tr>";
      for(Int_t i=0; i<ScalerAnalyzer::MaxColumn; ++i){
	// for(Int_t i=0; i<ScalerAnalyzer::MaxColumn; ++i){
	TString n = gScaler.GetScalerName(i, j);
	ss << "<td>";
	if(i != 0)
	  ss << " : ";
	auto val = gScaler.Get(i, j);
	if(i == ScalerAnalyzer::kCenter){
	  n += "-Hz";
	  val = 1.e6 * val / gScaler.Get("CLK-1MHz");
	}
	ss << n << "</td>"
	   << "<td align=\"right\">"
	   << gScaler.SeparateComma(val) << "</td>";
	if(n.Contains("n/a"))
	  continue;
	ofs << Form("%-20s", n.Data()) << val << std::endl;
      }
      ss << "</tr>";
    }
    ss << "<tr><td></td><td></td><td></td></tr>"
       << "<tr><td>T0/TAG-ALL</td>"
       << "<td align=\"right\">"
       << Form("%.6f", gScaler.Fraction("T0", "TAG-ALL"))
       << "</td>"
       << "<td> : Live/Real</td>"
       << "<td align=\"right\">"
       << Form("%.6f", gScaler.Fraction("Live-Time", "Real-Time"))
       << "</td>"
       << "<td> : DAQ-Eff</td>"
       << "<td align=\"right\">"
       << Form("%.6f", gScaler.Fraction("L1-Acc", "L1-Req"))
       << "</td>"
       << "</tr></tr>"
       << "<td>L1Req/TAG-ALL</td>"
       << "<td align=\"right\">"
       << Form("%.6f", gScaler.Fraction("L1-Req", "TAG-ALL"))
       << "</td>"
       << "<td> : L2-Eff</td>"
       << "<td align=\"right\">"
       << Form("%.6f", gScaler.Fraction("L2-Acc", "L1-Acc"))
       << "</td>"
       << "<td> : Duty-Factor</td>"
       << "<td align=\"right\">"
       << Form("%.6f", gScaler.Duty())
       << "</td>"
       << "</tr>";
    ss << "</table>";
    ss << "</div>";
    gHttp.SetItemField("/ScalerOn", "value", ss.str().c_str());
    ofs << std::endl
	<< Form("%-20s", "T0/TAG-ALL") << Form("%.6f", gScaler.Fraction("T0", "TAG-ALL")) << std::endl
	<< Form("%-20s", "Live/Real") << Form("%.6f", gScaler.Fraction("Live-Time", "Real-Time")) << std::endl
	<< Form("%-20s", "DAQ-Eff") << Form("%.6f", gScaler.Fraction("L1-Acc", "L1-Req")) << std::endl
	<< Form("%-20s", "L2-Eff") << Form("%.6f", gScaler.Fraction("L2-Acc", "L1-Acc")) << std::endl
	<< Form("%-20s", "Duty") << Form("%.6f", gScaler.Duty()) << std::endl;
    // gSystem->Sleep(2000);
    auto tagger_rate = 1.e6 * gScaler.Get("TAG-ALL") / gScaler.Get("CLK-1MHz");

    if(tagger_rate < tagger_rate_threshold){
      static const TString host(gSystem->Getenv("HOSTNAME"));
      static auto prev_time = std::time(0);
      auto        curr_time = std::time(0);
      if(host.Contains("online") &&
	 event_number > 1 && curr_time - prev_time > alert_interval){
	std::cout << "exec alert sound!" << std::endl;
	gSystem->Exec("ssh db-hyps \"aplay /home/sks/sound/alarm_sound.wav\" &");
	gSystem->Exec("ssh monitor-hyps.rcnp.osaka-u.ac.jp \"aplay /home/sks/sound/alarm_sound.wav\" &");
	prev_time = curr_time;
      }
      std::cout << "[Warning] Tagger rate is decreasing. "
		<< tagger_rate << " " << prev_time << " " << curr_time << std::endl;
    }
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
      std::cout << "[Warning] Tag is not good." << std::endl;
      static const TString host(gSystem->Getenv("HOSTNAME"));
      static auto prev_time = std::time(0);
      auto        curr_time = std::time(0);
      if(host.Contains("online") &&
	 event_number > 1 && curr_time - prev_time > 3){
	std::cout << "exec tagslip sound!" << std::endl;
	gSystem->Exec("ssh db-hyps \"aplay /home/sks/sound/tagslip.wav\" &");
	gSystem->Exec("ssh monitor-hyps.rcnp.osaka-u.ac.jp \"aplay /home/sks/sound/tagslip.wav\" &");
	prev_time = curr_time;
      }
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

  if(gScaler.IsSpillEnd()){
    gSystem->Sleep(150);
    gSystem->ProcessEvents();
  }

  gSystem->ProcessEvents();
  return 0;
}
}
