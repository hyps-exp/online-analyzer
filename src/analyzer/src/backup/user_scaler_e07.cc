// -*- C++ -*-

// Author: Tomonori Takahashi

#include <iomanip>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <TCanvas.h>
#include <TLatex.h>
#include <TLine.h>
#include <TSystem.h>
#include <TTimeStamp.h>

#include "DAQNode.hh"
#include "UnpackerManager.hh"
#include "Unpacker.hh"

#include "ConfMan.hh"
#include "DetectorID.hh"
#include "PrintHelper.hh"
#include "user_analyzer.hh"

#define SeparateComma 1

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  typedef unsigned long long Scaler;

  UnpackerManager& gUnpacker = GUnpacker::get_instance();
  int  g_run_number;
  bool g_spill_end         = false;
  bool flag_spill_by_spill = false;
  bool flag_semi_online    = false;

  static const std::size_t MaxRow = 32;
  enum eDisp { kLeft, kRight, MaxColumn };

  struct ScalerInfo
  {
    std::string name;
    int         module_id;
    int         channel;
    bool        flag_disp;
    Scaler      data; // integral in a spill
    Scaler      curr; // current value
    Scaler      prev; // previous value

    ScalerInfo( void ) {}
    ScalerInfo( std::string n, int id, int Ch, bool flag)
      : name(n), module_id(id), channel(Ch), flag_disp(flag),
	data(0), curr(0), prev(0)
    {}
  };

  std::vector< std::vector<ScalerInfo> > info;

  ///// for Scaler Sheet
  Scaler g_spill = 0;
  bool   flag_scaler_sheet   = false;
  Scaler spill_scaler_sheet  = 1;

  ///// for EMC
  static double emc_pos_x_start = -9999.;
  static double emc_pos_y_start = -9999.;
  static double emc_pos_x_stop  = -9999.;
  static double emc_pos_y_stop  = -9999.;
  static const double emc_x_offset = 500000 - 303300;
  static const double emc_y_offset = 500000 + 164000;

//____________________________________________________________________________
inline std::string
separate_comma( Scaler number )
{
  std::vector<Scaler> sep_num;

  while( number/1000 ){
    sep_num.push_back(number%1000);
    number /= 1000;
  }

  std::stringstream ss; ss<<number;
  std::vector<Scaler>::reverse_iterator itr, end=sep_num.rend();
  for( itr=sep_num.rbegin(); itr!=end; ++itr ){
    ss << "," << std::setfill('0') << std::setw(3) << *itr;
  }
  return ss.str();
}

//____________________________________________________________________________
inline Scaler
Get( const std::string& key )
{
  static const std::string func_name("[::Get()]");

  std::vector<ScalerInfo>::const_iterator itr, itr_end;
  for( std::size_t i=0; i<MaxColumn; ++i ){
    itr_end = info[i].end();
    for( itr=info[i].begin(); itr!=itr_end; ++itr ){
      if( itr->name == key ) return itr->data;
    }
  }

  std::cerr << "#W " << func_name << " "
	    << "no such ScalerInfo : " << key << std::endl;

  return 0;
}

//____________________________________________________________________________
inline void
Print( void )
{
  PrintHelper phelper( 6, std::ios::dec | std::ios::fixed, std::cout );

  std::cout << "\033[2J" << std::endl;

  double kpi_ratio = (double)Get("K_beam")/Get("pi_beam");

  TString end_mark = g_spill_end ? "Spill End" : "";

  int event_number = gUnpacker.get_event_number();
  std::cout << std::left  << std::setw(12) << "RUN"
	    << std::right << std::setw(16) << g_run_number << " : "
	    << std::left  << std::setw(12) << "Event Number"
	    << std::right << std::setw(16) << event_number
	    << std::endl  << std::endl
	    << std::left  << std::setw(12) << "K/pi"
	    << std::right << std::setw(16) << kpi_ratio << "   "
	    << std::left  << std::setw(12) << ""
	    << std::right << std::setw(16) << end_mark
	    << std::endl;

  double real_time = (double)Get("Real_Time");
  double live_time = (double)Get("Live_Time");
  double l1_req    = (double)Get("L1_Req");
  double l1_acc    = (double)Get("L1_Acc");
  double l2_acc    = (double)Get("L2_Acc");
  double real_live = live_time/real_time;
  double daq_eff   = l1_acc/l1_req;
  double l2_eff    = l2_acc/l1_acc;
  double duty_factor = daq_eff/(1.-daq_eff)*(1./real_live - 1.);
  if( duty_factor > 1. ) duty_factor = 1.;

  for( std::size_t i=0; i<MaxRow; ++i ){
    std::cout << std::left  << std::setw(12) << info[kLeft][i].name
	      << std::right << std::setw(16)
#if SeparateComma
	      << separate_comma( info[kLeft][i].data )
#else
	      << info[kLeft][i].data
#endif
	      << " : "
	      << std::left  << std::setw(12) << info[kRight][i].name
	      << std::right << std::setw(16)
#if SeparateComma
	      << separate_comma( info[kRight][i].data )
#else
	      << info[kRight][i].data
#endif
	      << std::endl;
  }

  std::cout << std::endl  << std::setprecision(6) << std::fixed
	    << std::left  << std::setw(12) << "Live/Real"
	    << std::right << std::setw(14) << real_live*100 << " % : "
	    << std::left  << std::setw(12) << "DAQ_Eff"
	    << std::right << std::setw(14) << daq_eff*100 << " %" << std::endl
	    << std::left  << std::setw(12) << "L2_Eff"
	    << std::right << std::setw(14) << l2_eff*100 << " % : "
	    << std::left  << std::setw(12) << "Duty factor"
	    << std::right << std::setw(14) << duty_factor*100 << " %"
	    << std::endl  << std::endl;
}

//____________________________________________________________________________
inline void
DrawOneBox( double x, double y,
	    const std::string& title1, const std::string& val1 )
{
  TLatex tex;
  tex.SetNDC();
  tex.SetTextSize(0.04);
  tex.SetTextAlign(12);
  tex.DrawLatex( x, y, title1.c_str() );
  tex.SetTextAlign(32);
  tex.DrawLatex( x+0.28, y, val1.c_str() );
}

//____________________________________________________________________________
inline void
DrawOneLine( const std::string& title1, const std::string& val1,
	     const std::string& title2, const std::string& val2,
	     const std::string& title3, const std::string& val3 )
{
  static int i = 0;
  const double ystep = 0.05;
  const double y0 = 0.95;
  double y = y0 - (i+1)*ystep;
  double x[] = { 0.05, 0.35, 0.67 };
  DrawOneBox( x[0], y, title1, val1 );
  DrawOneBox( x[1], y, title2, val2 );
  DrawOneBox( x[2], y, title3, val3 );
  TLine line;
  line.SetNDC();
  line.SetLineColor(kGray);
  line.DrawLine( 0.05, y-0.5*ystep, 0.95, y-0.5*ystep );
  line.SetLineColor(kBlack);
  line.DrawLine( 0.34, y-0.5*ystep, 0.34, y+0.5*ystep );
  line.DrawLine( 0.66, y-0.5*ystep, 0.66, y+0.5*ystep );
  if( i==1 || i==5 )
    line.DrawLine( 0.05, y-0.5*ystep, 0.95, y-0.5*ystep );

  ++i;
}

//____________________________________________________________________________
inline void
DrawOneLine( const std::string& title1,
	     const std::string& title2,
	     const std::string& title3 )
{
  DrawOneLine( title1, separate_comma( Get(title1) ),
	       title2, separate_comma( Get(title2) ),
	       title3, separate_comma( Get(title3) ) );
}

//____________________________________________________________________________
inline void
PrintScalerSheet( void )
{
  std::ostringstream oss;
  PrintHelper phelper( 6, std::ios::dec | std::ios::fixed, oss );

  TCanvas c1( "c1", "c1", 1200, 800 );

  TTimeStamp stamp;
  stamp.Add( -stamp.GetZoneOffset() );

  DrawOneLine( stamp.AsString("s"), "",
	       "", "",
	       "Name", "" );
  DrawOneLine( Form("#color[%d]{Run#}", kRed+1), separate_comma( g_run_number ),
	       "Ref Run#", "",
	       "Event#", separate_comma( gUnpacker.get_event_number() ) );
  DrawOneLine( "K-Beam", separate_comma( Get("K_beam") ),
	       "#pi-Beam", separate_comma( Get("pi_beam") ),
	       "D4", "[T]" );
  DrawOneLine( "p-Beam", separate_comma( Get("/p_beam") ),
	       "BH1xBH2", separate_comma( Get("BH1xBH2") ),
	       "KURAMA", "[T]" );
  DrawOneLine( "BH2xTOF", separate_comma( Get("BH2xTOF") ),
	       "Spill", separate_comma( g_spill ),
	       "Delay", "[s]" );
  DrawOneLine( "IM", separate_comma( Get("IM") ),
	       "TM", separate_comma( Get("TM") ),
	       "Width", "[s]" );
  DrawOneLine( "BH1", "K_in", "L1_Req" );
  DrawOneLine( "BH2", "pi_in", "L1_Acc" );
  DrawOneLine( "BAC1", "K_out", "MST_Acc" );
  DrawOneLine( "BAC2", "pi_out", "MST_Clear" );
  DrawOneLine( "PVAC", "(ub)", "MTX_Acc" );
  DrawOneLine( "FAC", "(ub,ub)", "L2_Req" );
  DrawOneLine( "SCH", "(K,pi)", "L2_Acc" );
  DrawOneLine( "TOF", "(K,K)", "FBH" );

  const TString& scaler_sheet_pdf("/tmp/scaler_sheet.pdf");
  c1.Print( scaler_sheet_pdf );

  const TString& print_command("lpr "+scaler_sheet_pdf);
  gSystem->Exec( print_command );
}

//____________________________________________________________________________
int
process_begin(const std::vector<std::string>& argv)
{
  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);

  flag_spill_by_spill = true;

  if( argv.size() > 3 ){
    // scaler sheet flag
    if( argv[3]=="--print" ){
      flag_spill_by_spill = false;
      flag_scaler_sheet = true;
      if( argv.size()==5 && argv[4].substr(0,8)=="--spill=" ){
	std::istringstream iss;
	iss.str( argv[4].substr(8) );
	iss >> spill_scaler_sheet;
      }
    }
  }

  // semi online flag
  if( argv.size() == 3 ){
    if( argv[2].find(":",0) == std::string::npos ){
      flag_semi_online = true;
      flag_spill_by_spill = false;
    }
  }

  info.resize(MaxColumn);

  for( std::size_t i=0; i<MaxRow; ++i ){
    info[kLeft].push_back( ScalerInfo("n/a", kLeft, i, false) );
    info[kRight].push_back( ScalerInfo("n/a", kRight, i, false) );
  }

  { // kLeft column (counter info)
    int index = 0;
    info[kLeft][index++] = ScalerInfo("K_beam",  0, 19, true);
    info[kLeft][index++] = ScalerInfo("pi_beam", 0, 20, true);
    info[kLeft][index++] = ScalerInfo("/p_beam", 0, 21, true);
    info[kLeft][index++] = ScalerInfo("BH1",     0,  0, true);
    info[kLeft][index++] = ScalerInfo("BH1-1",   0,  1, true);
    info[kLeft][index++] = ScalerInfo("BH1-2",   0,  2, true);
    info[kLeft][index++] = ScalerInfo("BH1-3",   0,  3, true);
    info[kLeft][index++] = ScalerInfo("BH1-4",   0,  4, true);
    info[kLeft][index++] = ScalerInfo("BH1-5",   0,  5, true);
    info[kLeft][index++] = ScalerInfo("BH1-6",   0,  6, true);
    info[kLeft][index++] = ScalerInfo("BH1-7",   0,  7, true);
    info[kLeft][index++] = ScalerInfo("BH1-8",   0,  8, true);
    info[kLeft][index++] = ScalerInfo("BH1-9",   0,  9, true);
    info[kLeft][index++] = ScalerInfo("BH1-10",  0, 10, true);
    info[kLeft][index++] = ScalerInfo("BH1-11",  0, 11, true);
    info[kLeft][index++] = ScalerInfo("BH2",     0, 12, true);
    info[kLeft][index++] = ScalerInfo("BAC1",    0, 13, true);
    info[kLeft][index++] = ScalerInfo("BAC2",    0, 14, true);
    info[kLeft][index++] = ScalerInfo("FBH",     0, 42, true);
    info[kLeft][index++] = ScalerInfo("PVAC",    0, 15, true);
    info[kLeft][index++] = ScalerInfo("FAC",     0, 16, true);
    info[kLeft][index++] = ScalerInfo("SCH",     0, 17, true);
    info[kLeft][index++] = ScalerInfo("TOF",     0, 18, true);
  }

  { // kRight column (DAQ info)
    int index = 0;
    info[kRight][index++] = ScalerInfo("Spill",        0, 32, true);
    info[kRight][index++] = ScalerInfo("10M Clock",    0, 33, true);
    info[kRight][index++] = ScalerInfo("IM",           0, 34, true);
    info[kRight][index++] = ScalerInfo("TM",           0, 35, true);
    info[kRight][index++] = ScalerInfo("Real_Time",    0, 36, true);
    info[kRight][index++] = ScalerInfo("Live_Time",    0, 37, true);
    info[kRight][index++] = ScalerInfo("L1_Req",       0, 38, true);
    info[kRight][index++] = ScalerInfo("L1_Acc",       0, 39, true);
    info[kRight][index++] = ScalerInfo("MTX_Acc",      0, 40, true);
    info[kRight][index++] = ScalerInfo("MTX-1",        0, 41, true);
    info[kRight][index++] = ScalerInfo("MST_Acc",      0, 44, true);
    info[kRight][index++] = ScalerInfo("MST_Clear",    0, 45, true);
    info[kRight][index++] = ScalerInfo("MST_Clear_PS", 0, 46, true);
    info[kRight][index++] = ScalerInfo("L2_Clear",     0, 47, true);
    info[kRight][index++] = ScalerInfo("L2_Req",       0, 48, true);
    info[kRight][index++] = ScalerInfo("L2_Acc",       0, 49, true);
    info[kRight][index++] = ScalerInfo("(ub)",         0, 50, true);
    info[kRight][index++] = ScalerInfo("(ub,ub)",      0, 51, true);
    info[kRight][index++] = ScalerInfo("(K,pi)",       0, 52, true);
    info[kRight][index++] = ScalerInfo("(K,K)",        0, 53, true);
    info[kRight][index++] = ScalerInfo("(ub)PS",       0, 54, true);
    info[kRight][index++] = ScalerInfo("(ub,ub)PS",    0, 55, true);
    info[kRight][index++] = ScalerInfo("(K,pi)PS",     0, 56, true);
    info[kRight][index++] = ScalerInfo("(K,K)PS",      0, 57, true);
    info[kRight][index++] = ScalerInfo("K_in",         0, 25, true);
    info[kRight][index++] = ScalerInfo("pi_in",        0, 26, true);
    info[kRight][index++] = ScalerInfo("K_out",        0, 27, true);
    info[kRight][index++] = ScalerInfo("pi_out",       0, 28, true);
    info[kRight][index++] = ScalerInfo("BH1xBH2",      0, 22, true);
    info[kRight][index++] = ScalerInfo("BH2xTOF",      0, 23, true);
    info[kRight][index++] = ScalerInfo("PVACx/FAC",    0, 24, true);
  }

  std::cout << "#D flag" << std::left << std::endl
	    << " key = " << std::setw(20) << "flag_spill_by_spill"
	    << " val = " << flag_spill_by_spill << std::endl
	    << " key = " << std::setw(20) << "flag_semi_online"
	    << " val = " << flag_semi_online    << std::endl
	    << " key = " << std::setw(20) << "flag_scaler_sheet"
	    << " val = " << flag_scaler_sheet   << std::endl;

  return 0;
}

//____________________________________________________________________________
int
process_end()
{
  if( flag_scaler_sheet )
    return 0;

  PrintHelper phelper( 6, std::ios::dec | std::ios::fixed, std::cout );

  std::cout << "\n#D : End of scaler, summarize this run" << std::endl;

  Print();

  Scaler spill      = Get("Spill");
  Scaler kbeam      = Get("K_beam");
  Scaler pibeam     = Get("pi_beam");
  Scaler kk         = Get("(K,K)");
  Scaler bh1xbh2    = Get("BH1xBH2");
  Scaler real_time  = Get("Real_Time");
  Scaler live_time  = Get("Live_Time");
  Scaler l1_req     = Get("L1_Req");
  Scaler l1_acc     = Get("L1_Acc");
  Scaler clear      = Get("L2_Clear");
  Scaler l2_acc     = Get("L2_Acc");
  Scaler bh2        = Get("BH2");
  Scaler fbh        = Get("FBH");
  Scaler sch        = Get("SCH");
  Scaler TM         = Get("TM");
  double daq_eff    = (double)l1_acc/l1_req;
  double real_live  = (double)live_time/real_time;
  double duty_factor = 0;
  if( 1-daq_eff == 0 )
    duty_factor = 1.;
  else
    duty_factor = daq_eff/(1-daq_eff)*(1/real_live - 1);

  // summary
  std::cout << std::endl
	    << "#D : For the check sheet" << std::endl << std::endl
	    << std::left  << std::setw(20) << ""
	    << std::right << std::setw(16) << "" << " : "
	    << std::left  << std::setw(20) << "BH2"
	    << std::right << std::setw(16) << separate_comma(bh2) << std::endl
	    << std::left  << std::setw(20) << "Spill"
	    << std::right << std::setw(16) << separate_comma(spill) << " : "
	    << std::left  << std::setw(20) << "FBH"
	    << std::right << std::setw(16) << separate_comma(fbh) << std::endl
	    << std::left  << std::setw(20) << "K_beam"
	    << std::right << std::setw(16) << separate_comma(kbeam) << " : "
	    << std::left  << std::setw(20) << "K_beam/TM"
	    << std::right << std::setw(16) << (double)kbeam/TM << std::endl
	    << std::left  << std::setw(20) << "pi_beam"
	    << std::right << std::setw(16) << separate_comma(pibeam) << " : "
	    << std::left  << std::setw(20) << "K_beam/BH1xBH2"
	    << std::right << std::setw(16) << (double)kbeam/bh1xbh2 << std::endl
	    << std::left  << std::setw(20) << "BH1xBH2"
	    << std::right << std::setw(16) << separate_comma(bh1xbh2) << " : "
	    << std::left  << std::setw(20) << "K_beam/SCH"
	    << std::right << std::setw(16) << (double)kbeam/sch << std::endl
	    << std::left  << std::setw(20) << "(K,K)"
	    << std::right << std::setw(16) << separate_comma(kk) << " : "
	    << std::left  << std::setw(20) << "(K,K)/K_beam"
	    << std::right << std::setw(16) << (double)kk/kbeam << std::endl
	    << std::left  << std::setw(20) << "L1_Req"
	    << std::right << std::setw(16) << separate_comma(l1_req) << " : "
	    << std::left  << std::setw(20) << "Live/Real"
	    << std::right << std::setw(16) << real_live << std::endl
	    << std::left  << std::setw(20) << "L1_Acc"
	    << std::right << std::setw(16) << separate_comma(l1_acc) << " : "
	    << std::left  << std::setw(20) << "L1_Acc/L1_Req"
	    << std::right << std::setw(16) << daq_eff << std::endl
	    << std::left  << std::setw(20) << "L2_Clear"
	    << std::right << std::setw(16) << separate_comma(clear) << " : "
	    << std::left  << std::setw(20) << "L2_Acc/L1_Acc"
	    << std::right << std::setw(16) << (double)l2_acc/l1_acc << std::endl
	    << std::left  << std::setw(20) << "L2_Acc"
	    << std::right << std::setw(16) << separate_comma(l2_acc) << " : "
	    << std::left  << std::setw(20) << "Duty_Factor"
	    << std::right << std::setw(16) << duty_factor << std::endl
	    << std::endl;

  // EMC
  std::cout << std::setprecision(3)
	    << std::left  << std::setw(20) << "EMC Start Position"
	    << "( " << emc_pos_x_start << ", " << emc_pos_y_start << " )"
	    << std::endl
	    << std::left  << std::setw(20) << "EMC Stop Position"
	    << "( " << emc_pos_x_stop << ", " << emc_pos_y_stop << " )"
	    << std::endl << std::endl;

  return 0;

}

//____________________________________________________________________________
int
process_event()
{
  static int  event_count = 0;
  static bool en_disp     = false;
  static int  scaler_id   = gUnpacker.get_device_id("Scaler");

  if( flag_semi_online ){
    // if( event_count%300 == 0 ) en_disp = true;
  } else {
    if( event_count%10 == 0 ) en_disp = true;
  }

  if( flag_scaler_sheet && event_count==0 )
    std::cout << "#D waiting spill end " << std::flush;

  // TFlag
  g_spill_end = false;
  {
    static const int k_device = gUnpacker.get_device_id("TFlag");
    static const int k_seg    = SegIdScalerTrigger;
    static const int k_tdc    = gUnpacker.get_data_id("TFlag", "tdc");
    if( gUnpacker.get_entries( k_device, 0, k_seg, 0, k_tdc )>0 &&
	gUnpacker.get( k_device, 0, k_seg, 0, k_tdc )>0 )
      {
	g_spill_end = true;
      }
  }
  if( g_spill_end )
    en_disp = true;

  // EMC
  {
    static const int k_device = gUnpacker.get_device_id("EMC");
    static const int k_xpos   = gUnpacker.get_data_id("EMC", "xpos");
    static const int k_ypos   = gUnpacker.get_data_id("EMC", "ypos");
    double xpos = -9999.;
    double ypos = -9999.;
    int xpos_nhit = gUnpacker.get_entries( k_device, 0, 0, 0, k_xpos );
    if( xpos_nhit!=0 ) xpos = gUnpacker.get( k_device, 0, 0, 0, k_xpos );
    int ypos_nhit = gUnpacker.get_entries( k_device, 0, 0, 0, k_ypos );
    if( ypos_nhit!=0 ) ypos = gUnpacker.get( k_device, 0, 0, 0, k_ypos );
    if( xpos_nhit>0 && ypos_nhit>0 ){
      if( event_count==0 ){
	emc_pos_x_start = (xpos-emc_x_offset)/1000.;
	emc_pos_y_start = (ypos-emc_y_offset)/1000.;
      }
      emc_pos_x_stop   = (xpos-emc_x_offset)/1000.;
      emc_pos_y_stop   = (ypos-emc_y_offset)/1000.;
    }
  }

  ++event_count;

  if( flag_scaler_sheet && !g_spill_end ){
    if( event_count%100==0 )
      std::cout << "." << std::flush;
    return 0;
  }

  if( !gUnpacker.get_entries( scaler_id, 0, 0, 0, 0 ) ){
    en_disp = false;
    return 0;
  }

  if(g_run_number != gUnpacker.get_root()->get_run_number()){
    g_run_number = gUnpacker.get_root()->get_run_number();
    g_spill = 0;
    info[kRight][0].data = 0;
    for( std::size_t i=0; i<MaxColumn; ++i ){
      for( std::size_t j=0; j<MaxRow; ++j ){
	if( i==kRight && j==0 ) continue;
	info[i][j].data = 0;
      }
    }
  }

  {
    bool inclement_spill = false;
    // scaler
    static int scaler_id    = gUnpacker.get_device_id("Scaler");

    for( std::size_t i=0; i<MaxColumn; ++i ){
      for( std::size_t j=0; j<MaxRow; ++j ){
	if( !info[i][j].flag_disp ) continue;
	if( i==kRight && j==0 ) continue;
	int module_id = info[i][j].module_id;
	int channel   = info[i][j].channel;
	int nhit = gUnpacker.get_entries( scaler_id, module_id, 0, channel, 0 );
	if( nhit<=0 ) continue;
	Scaler val = gUnpacker.get( scaler_id, module_id, 0, channel, 0 );

	if( info[i][j].prev > val || flag_scaler_sheet ){
	  inclement_spill = true;
	  info[i][j].prev = 0;
	}

	if( flag_spill_by_spill && inclement_spill )
	  info[i][j].data = 0;

	info[i][j].curr  = val;
	info[i][j].data += val - info[i][j].prev;
	info[i][j].prev  = info[i][j].curr;
      }
    }

    // inclement spill
    if( inclement_spill )
      info[kRight][0].data++;

    if( en_disp /* && !flag_scaler_sheet */ ){
      Print();
    }
  }

  en_disp = false;

  if( g_spill_end && flag_scaler_sheet ){
    ++g_spill;
    std::cout << "#D found spill end "
	      << g_spill << "/" << spill_scaler_sheet << std::endl;
    if( g_spill==spill_scaler_sheet ){
      PrintScalerSheet();
      return -1;
    }
    std::cout << "#D waiting spill end " << std::flush;
  }

  return 0;
}

}
