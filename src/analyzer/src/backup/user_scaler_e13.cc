// Author: Tomonori Takahashi

#include <iomanip>
#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <vector>

#include "user_analyzer.hh"
#include "UnpackerManager.hh"
#include "Unpacker.hh"
#include "DAQNode.hh"
#include "ConfMan.hh"

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  bool flag_spill_by_spill = false;

  static const int NofCh = 32;

  enum dispColumn{
    left, center, right,
    Ge_left, Ge_right,
    size_dispColumn
  };

  enum id_vme03{
    id_vme03_0, id_vme03_1, id_vme03_2,
    size_id_vme03
  };

  enum id_vme06{
    id_CRM, id_Reset,
    size_idGeType
  };

  struct scaler_info{
    std::string name;
    int  module_id;
    int  ch;
    bool flag_disp;

    scaler_info(){;}
    scaler_info(const char* Name, int Id, int Ch, bool flag):
      name(Name), module_id(Id), ch(Ch), flag_disp(flag)
    {;}
  };

  std::vector<scaler_info> cont_info[size_dispColumn];

  static unsigned int prev[size_dispColumn][NofCh] = {{0}, {0}, {0}, {0}, {0} };
  static unsigned int curr[size_dispColumn][NofCh] = {{0}, {0}, {0}, {0}, {0} };
  static unsigned int val[size_dispColumn][NofCh]  = {{0}, {0}, {0}, {0}, {0} };

//____________________________________________________________________________
std::string
separate_comma(int number)
{
  std::vector<int> sep_num;
  int abs_number = abs(number);
  int sign       = number >= 0 ? 1 : -1;

  while(abs_number/1000){
    sep_num.push_back(abs_number%1000);
    abs_number /= 1000;
  }

  std::stringstream ss;
  ss<<abs_number*sign;
  for(std::vector<int>::reverse_iterator i=sep_num.rbegin();
      i!=sep_num.rend();i++){
    ss<<","<<std::setfill('0')<<std::setw(3)<<*i;
  }
  return ss.str();
}

//____________________________________________________________________________
int
process_begin(const std::vector<std::string>& argv)
{
  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize(argv);

  // spill by spill flag
  if(4 == argv.size()){
    flag_spill_by_spill = true;
  }

  for(int i = 0; i<32; ++i){
    cont_info[left].push_back(   scaler_info("N/A", id_vme03_1, i, false) );
    cont_info[center].push_back( scaler_info("N/A", id_vme03_2, i, false) );
    cont_info[right].push_back(  scaler_info("N/A", id_vme03_0, i, false) );

    char name_crm[128];
    sprintf(name_crm, "CRM%d", i);
    cont_info[Ge_left].push_back(  scaler_info(name_crm, id_CRM,   i, true) );

    char name_reset[128];
    sprintf(name_reset, "RST%d", i);
    cont_info[Ge_right].push_back( scaler_info(name_reset, id_Reset, i, true) );
  }

  // left column (counter info)
  int index = 0;
  {scaler_info tmp("BH1OR",     id_vme03_1, 0,  true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-1",     id_vme03_1, 1,  true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-2",     id_vme03_1, 2,  true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-3",     id_vme03_1, 3,  true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-4",     id_vme03_1, 4,  true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-5",     id_vme03_1, 5,  true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-6",     id_vme03_1, 6,  true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-7",     id_vme03_1, 7,  true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-8",     id_vme03_1, 8,  true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-9",     id_vme03_1, 9,  true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-10",    id_vme03_1, 10, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-11",    id_vme03_1, 11, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BMW",       id_vme03_1, 12, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH2OR",     id_vme03_1, 13, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH2-1",     id_vme03_2,  2, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH2-2",     id_vme03_2,  3, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH2-3",     id_vme03_1, 16, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH2-4",     id_vme03_1, 17, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH2-5",     id_vme03_1, 18, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BAC1",      id_vme03_1, 21, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BAC2",      id_vme03_1, 22, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("SAC1",      id_vme03_1, 23, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("TOF",       id_vme03_1, 24, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("SAC3",      id_vme03_1, 25, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("SFV",       id_vme03_1, 26, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("SMF",       id_vme03_1, 27, true); cont_info[left][index++] = tmp;}

  // center column (etc. info)
  index = 0;
  {scaler_info tmp("K beam",        id_vme03_1, 28, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("pi beam",       id_vme03_1, 29, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("/p beam",       id_vme03_1, 30, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("Real time",     id_vme03_1, 14, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("Live time",     id_vme03_1, 15, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("BTOF Go",       id_vme03_2,  5, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("BTOF clear",    id_vme03_2,  6, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("CRM-OR",        id_vme03_2,  1, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("/p beam (e07)", id_vme03_2,  7, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("TOFxSMFx/MTX",  id_vme03_2,  8, true); cont_info[center][index++] = tmp;}

  // right column (DAQ info)
  index = 0;
  {scaler_info tmp("spill",          id_vme03_0, 0,  true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("IM",             id_vme03_0, 1,  true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("TM",             id_vme03_0, 28, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("L1 req",         id_vme03_0, 3,  true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("L1 acc",         id_vme03_1, 19, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("Coin Go",        id_vme03_1, 20, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("Coin clear",     id_vme03_0,  6, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("Overflow Go",    id_vme03_0,  7, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("Overflow clear", id_vme03_0,  8, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("CoinxOver",      id_vme03_2,  4, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("Total clear",    id_vme03_0,  9, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("L2 req",         id_vme03_0, 11, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("L2 acc",         id_vme03_0, 12, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("(K, pi)",        id_vme03_0, 13, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("(ub, ub)",       id_vme03_0, 14, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("(pi, pi)",       id_vme03_0, 15, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("(K, pi)PS",      id_vme03_0, 16, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("(ub, ub)PS",     id_vme03_0, 17, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("(ub)PS",         id_vme03_0, 18, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("(pi, pi)PS",     id_vme03_0, 19, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("K in",           id_vme03_0, 20, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("pi in",          id_vme03_0, 21, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("pi Tout",        id_vme03_0, 22, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("K Tout",         id_vme03_0, 23, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("SFVx/SAC3",      id_vme03_0, 24, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("pi Out",         id_vme03_0, 25, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("(K,pi) w/oSMF",  id_vme03_0, 26, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("TOFx/SFV",       id_vme03_0, 27, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("TOFxSMF",        id_vme03_0, 29, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("BH1xBH2",        id_vme03_0, 10, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("BH2xTOF",        id_vme03_0, 31, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("10M Clock",      id_vme03_1, 31, true); cont_info[right][index++] = tmp;}

  return 0;
}

//____________________________________________________________________________
int
process_end()
{
  std::cout << "\n#D : End of scaler, summarize this run" << std::endl;
  printf("%-10s %10s : %-15s %10s : %-15s %10s : %-6s %12s : %-6s %12s\n",
	 "", "Integral",
	 "", "Integral",
	 "", "Integral",
	 "", "kHz",
	 "", "Hz");

  for(int i = 0; i<NofCh; ++i){
    scaler_info info[size_dispColumn];

    // Counter & DAQ info
    info[left]   = cont_info[left][i];
    info[center] = cont_info[center][i];
    info[right]  = cont_info[right][i];

    // Ge info
    info[Ge_left]  = cont_info[Ge_left][i];
    info[Ge_right] = cont_info[Ge_right][i];

    // display
    printf("%-10s %10u : %-15s %10u : %-15s %10u : %-6s %12f : %-6s %12f\n",
	   info[left].name.c_str(),   val[left][i],
	   info[center].name.c_str(), val[center][i],
	   info[right].name.c_str(),  val[right][i],
	   info[Ge_left].name.c_str(),  (double)(val[Ge_left][i]*10000./val[Ge_left][31]),
	   info[Ge_right].name.c_str(), (double)(val[Ge_right][i]*10000000./val[Ge_left][31])
	   );
  }

  std::cout << "\n#D : For the scaler check sheet" << std::endl;
  int    spill      = val[right][0];
  int    ge_or      = val[center][5];
  double kbeam      = (double)val[center][0];
  double kpi        = (double)val[right][13];
  double real_time  = (double)val[center][3];
  double live_time  = (double)val[center][4];
  double l1_req     = (double)val[right][3];
  double l1_acc     = (double)val[right][4];
  int    ge_coin_go    = val[right][5];
  int    ge_coin_clear = val[right][6];
  int    ge_over_go    = val[right][7];
  int    ge_over_clear = val[right][8];
  int    clear      = val[right][10];
  double l2_req     = (double)val[right][11];
  double l2_acc     = (double)val[right][12];
  int    bh2        = val[left][13];
  int    pibeam     = val[center][1];
  double TM         = (double)val[right][2];
  double bh1bh2     = (double)val[right][29];

  printf("%-20s %15s\t",   "",               "");
  printf("%-20s %15s\n",   "BH2 OR",         separate_comma(bh2).c_str());
  printf("%-20s %15s\t",   "Spill",          separate_comma(spill).c_str());
  printf("%-20s %15s\n",   "Ge Coin Go",     separate_comma(ge_coin_go).c_str());
  //printf("%-20s %15s\n",   "Ge OR",          separate_comma(ge_or).c_str());
  printf("%-20s %15s\t",   "K-beam",         separate_comma((int)kbeam).c_str());
  printf("%-20s %15s\n",   "Ge Overflow Go", separate_comma(ge_over_go).c_str());
  printf("%-20s %15s\t",   "Pi-beam",        separate_comma(pibeam).c_str());
  printf("%-20s %15.4f\n", "K-beam/TM",      kbeam/TM);
  printf("%-20s %15s\t",   "(K,pi)",         separate_comma((int)kpi).c_str());
  printf("%-20s %15.4f\n", "K-beam/BH1xBH2", kbeam/bh1bh2);
  printf("%-20s %15s\t",   "Trig1 Req.",     separate_comma((int)l1_req).c_str());
  printf("%-20s %15.4f\n", "(K,pi)/K-beam",  kpi/kbeam);
  printf("%-20s %15s\t",   "Trig1 Acc.",     separate_comma((int)l1_acc).c_str());
  printf("%-20s %15.4f\n", "Live time/Real time", live_time/real_time);
  printf("%-20s %15s\t",   "Total Clear",    separate_comma(clear).c_str());
  printf("%-20s %15.4f\n", "L1 acc/L1 req",  l1_acc/l1_req);
  printf("%-20s %15s\t",   "Trig2 Acc.",     separate_comma((int)l2_acc).c_str());
  printf("%-20s %15.4f\n", "L2 acc/L1 acc", l2_acc/l1_acc);
  printf("\n");

  return 0;
}

//____________________________________________________________________________
int
process_event()
{
  static UnpackerManager& g_unpacker = GUnpacker::get_instance();

  static int run_number = g_unpacker.get_root()->get_run_number();
  static int event_count = 0;
  static bool en_disp = false;
  if(flag_spill_by_spill){
    if(event_count%3 == 0)   en_disp = true;
  }else{
    if(event_count%400 == 0) en_disp = true;
  }

  // clear console
  if(en_disp) printf("\033[2J");

  if(run_number != g_unpacker.get_root()->get_run_number()){
    for (int col = 0; col<size_dispColumn; ++col){
      for(int i = 0; i<NofCh; ++i){
	val[col][i] = 0;
      }
    }

    run_number = g_unpacker.get_root()->get_run_number();
  }

  bool inclement_spill = false;
  {
    // scaler
    static int scaler_id    = g_unpacker.get_device_id("scaler");
    static int ge_scaler_id = g_unpacker.get_device_id("GeScaler");

    for(int i = 0; i<NofCh; ++i){
      scaler_info info[size_dispColumn];

      // Counter & DAQ info
      info[left]   = cont_info[left][i];
      info[center] = cont_info[center][i];
      info[right]  = cont_info[right][i];

      // Left column
      if(info[left].flag_disp){
	prev[left][i] = curr[left][i];
	curr[left][i] = g_unpacker.get(scaler_id, info[left].module_id, 0, info[left].ch, 0);

	if(curr[left][i] < prev[left][i]){
	  prev[left][i] = 0;
	  inclement_spill = true;
	}

	if(flag_spill_by_spill){
	  val[left][i]  = curr[left][i];
	}else{
	  val[left][i] += curr[left][i] - prev[left][i];
	}

      }

      // Center column
      if(info[center].flag_disp){
	prev[center][i] = curr[center][i];
	curr[center][i] = g_unpacker.get(scaler_id, info[center].module_id, 0, info[center].ch, 0);

	if(curr[center][i] < prev[center][i]){
	  prev[center][i] = 0;
	}

	if(flag_spill_by_spill){
	  val[center][i]  = curr[center][i];
	}else{
	  val[center][i] += curr[center][i] - prev[center][i];
	}
      }

      // Right column
      if(info[right].flag_disp){
	prev[right][i] = curr[right][i];
	curr[right][i] = g_unpacker.get(scaler_id, info[right].module_id, 0, info[right].ch, 0);

	if(curr[right][i] < prev[right][i]){
	  prev[right][i] = 0;
	}

	if(i == 0){
	  // spill
	  if(inclement_spill) ++val[right][0];
	}else{
	  if(flag_spill_by_spill){
	    val[right][i]  = curr[right][i];
	  }else{
	    val[right][i] += curr[right][i] - prev[right][i];
	  }
	}
      }

      // Ge info
      info[Ge_left]  = cont_info[Ge_left][i];
      info[Ge_right] = cont_info[Ge_right][i];

      // Ge CRM
      if(info[Ge_left].flag_disp){
	curr[Ge_left][i] = g_unpacker.get(ge_scaler_id, info[Ge_left].module_id, 0, info[Ge_left].ch, 0);
	val[Ge_left][i] = curr[Ge_left][i];
      }

      // Ge Reset
      if(info[Ge_right].flag_disp){
	curr[Ge_right][i] = g_unpacker.get(ge_scaler_id, info[Ge_right].module_id, 0, info[Ge_right].ch, 0);
	val[Ge_right][i] = curr[Ge_right][i];
      }
    }

    if(en_disp){
      printf("%-10s %10s : %-15s %10s : %-15s %10s : %-6s %12s : %-6s %12s\n",
	     "", "Integral",
	     "", "Integral",
	     "", "Integral",
	     "", "kHz",
	     "", "Hz");

      for(int i = 0; i<NofCh; ++i){
	scaler_info info[size_dispColumn];

	// Counter & DAQ info
	info[left]   = cont_info[left][i];
	info[center] = cont_info[center][i];
	info[right]  = cont_info[right][i];

	// Ge info
	info[Ge_left]  = cont_info[Ge_left][i];
	info[Ge_right] = cont_info[Ge_right][i];

	// display
	printf("%-10s %10u : %-15s %10u : %-15s %10u : %-6s %12f : %-6s %12f\n",
	       info[left].name.c_str(),   val[left][i],
	       info[center].name.c_str(), val[center][i],
	       info[right].name.c_str(),  val[right][i],
	       info[Ge_left].name.c_str(),  (double)(val[Ge_left][i]*10000./val[Ge_left][31]),
	       info[Ge_right].name.c_str(), (double)(val[Ge_right][i]*10000000./val[Ge_left][31])
	       );
      }
    }
  }

  ++event_count;
  en_disp = false;

  return 0;
}

}
