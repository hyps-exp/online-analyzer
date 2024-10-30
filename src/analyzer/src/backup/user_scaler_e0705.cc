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
  bool flag_semi_online    = false;

  static const int NofCh = 33;

  enum dispColumn{
    left, center, right,
    size_dispColumn
  };

  enum id_vme03{
    id_vme03_0, id_vme03_1, id_vme03_2,
    size_id_vme03
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

  static long long unsigned int prev[size_dispColumn][NofCh] = {{0}, {0}, {0}};
  static long long unsigned int curr[size_dispColumn][NofCh] = {{0}, {0}, {0}};
  static long long unsigned int val[size_dispColumn][NofCh]  = {{0}, {0}, {0}};

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
  if( argv.size() == 4 ){
    flag_spill_by_spill = true;
  }

  // semi online flag
  if( argv.size() == 3 ){
    if( argv[2].find(":",0) == std::string::npos ){
      flag_semi_online = true;
    }
  }

  for(int i = 0; i<33; ++i){
    cont_info[left].push_back(   scaler_info("n/a", id_vme03_1, i, false) );
    cont_info[center].push_back( scaler_info("n/a", id_vme03_2, i, false) );
    cont_info[right].push_back(  scaler_info("n/a", id_vme03_0, i, false) );
  }

  // left column (counter info)
  int index = 0;
  {scaler_info tmp("K beam ",   id_vme03_1, 28, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("pi beam",   id_vme03_1, 29, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("/p beam",   id_vme03_1, 30, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-OR",    id_vme03_1,  0, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-1",     id_vme03_1,  1, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-2",     id_vme03_1,  2, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-3",     id_vme03_1,  3, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-4",     id_vme03_1,  4, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-5",     id_vme03_1,  5, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-6",     id_vme03_1,  6, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-7",     id_vme03_1,  7, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-8",     id_vme03_1,  8, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-9",     id_vme03_1,  9, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-10",    id_vme03_1, 10, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH1-11",    id_vme03_1, 11, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH2-OR",    id_vme03_1, 13, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH2-1",     id_vme03_2,  2, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH2-2",     id_vme03_2,  3, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH2-3",     id_vme03_1, 16, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH2-4",     id_vme03_1, 17, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BH2-5",     id_vme03_1, 18, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BAC1",      id_vme03_1, 21, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("BAC2",      id_vme03_1, 22, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("TOF",       id_vme03_1, 24, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("LAC-OR",    id_vme03_1, 25, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("LAC(a)",    id_vme03_0,  6, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("LAC(b)",    id_vme03_0,  7, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("LAC(c)",    id_vme03_0,  8, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("LC",        id_vme03_1, 27, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("KIC-U",     id_vme03_1, 23, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("KIC-D",     id_vme03_1, 26, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("KIC-L",     id_vme03_0, 22, true); cont_info[left][index++] = tmp;}
  {scaler_info tmp("KIC-R",     id_vme03_0, 25, true); cont_info[left][index++] = tmp;}

  // center column (E07 info)
  index = 0;
  {scaler_info tmp("K beam",  id_vme03_2, 20, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("pi beam", id_vme03_2, 21, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("/p beam", id_vme03_2, 22, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("BH2-1",   id_vme03_2,  8, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("BH2-2",   id_vme03_2,  9, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("BMW",     id_vme03_2, 15, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("BAC1",    id_vme03_2, 10, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("BAC2",    id_vme03_2, 11, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("PVAC",    id_vme03_2, 12, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("FAC",     id_vme03_2, 13, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("SAC1",    id_vme03_2, 14, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("BH2-1U",  id_vme03_2, 16, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("BH2-1D",  id_vme03_2, 17, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("BH2-2U",  id_vme03_2, 18, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("BH2-2D",  id_vme03_2, 19, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("BH1xBH2", id_vme03_2, 23, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("BH2-1xBH2-2", id_vme03_2, 24, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("PVACx/FAC",   id_vme03_2, 25, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("(K,K)",       id_vme03_2, 26, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("(ub)",        id_vme03_2, 27, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("(ub)PS",      id_vme03_2, 28, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("SCH-L",       id_vme03_2, 29, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("SCH-R",       id_vme03_2, 30, true); cont_info[center][index++] = tmp;}
  {scaler_info tmp("FBH",         id_vme03_2, 31, true); cont_info[center][index++] = tmp;}

  // right column (DAQ info)
  index = 0;
  {scaler_info tmp("spill",          id_vme03_0, 0,  true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("10M Clock",      id_vme03_1, 31, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("IM",             id_vme03_0, 1,  true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("TM",             id_vme03_0, 28, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("Real time",      id_vme03_0,  9, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("Live time",      id_vme03_0, 11, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("L1 req",         id_vme03_0, 3,  true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("L1 acc",         id_vme03_1, 19, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("MsT Go",         id_vme03_1, 14, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("MsT Clear",      id_vme03_1, 15, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("MstClr To PS Go",id_vme03_0, 26, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("L2 clear",       id_vme03_0, 27, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("L2 req",         id_vme03_2,  7, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("L2 acc",         id_vme03_2,  1, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("(K,K)",          id_vme03_0, 13, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("(ub,ub)",        id_vme03_0, 14, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("(K,pi)",         id_vme03_0, 15, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("(K,K)PS",        id_vme03_0, 16, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("(ub,ub)PS",      id_vme03_0, 17, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("(ub)PS",         id_vme03_0, 18, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("(K, pi)PS",      id_vme03_0, 19, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("K in",           id_vme03_0, 20, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("pi in",          id_vme03_0, 21, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("BH1xBH2",        id_vme03_2, 15, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("BH2xTOF",        id_vme03_0, 31, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("TOFxLC",         id_vme03_0, 29, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("TOFxLCx/LAC",    id_vme03_0, 23, true); cont_info[right][index++] = tmp;}
  {scaler_info tmp("LACx[TOF/LC]",   id_vme03_0, 24, true); cont_info[right][index++] = tmp;}

  return 0;
}

//____________________________________________________________________________
int
process_end()
{
  std::cout << "\n#D : End of scaler, summarize this run" << std::endl;
  printf("%-10s %10s : %-15s %10s : %-15s %15s\n",
	 "- E05 -", "Integral",
	 "- E07 -", "Integral",
	 "", "Integral");

  for(int i = 0; i<NofCh; ++i){
    scaler_info info[size_dispColumn];

    // Counter & DAQ info
    info[left]   = cont_info[left][i];
    info[center] = cont_info[center][i];
    info[right]  = cont_info[right][i];

    // display
    printf("%-10s %10llu : %-15s %10llu : %-15s %15llu \n",
	   info[left].name.c_str(),   val[left][i],
	   info[center].name.c_str(), val[center][i],
	   info[right].name.c_str(),  val[right][i]
	   );
  }

  std::cout << "\n#D : For the scaler check sheet" << std::endl;
  int    spill      = val[right][0];
  double kbeam      = (double)val[left][0];
  double pibeam     = (double)val[left][1];
  double kk         = (double)val[right][14];
  double kpi        = (double)val[right][16];
  double real_time  = (double)val[right][4];
  double live_time  = (double)val[right][5];
  double l1_req     = (double)val[right][6];
  double l1_acc     = (double)val[right][7];
  int    clear      = val[right][11];
  int    mstgo      = val[right][8];
  int    mstclear   = val[right][9];
  double l2_acc     = (double)val[right][12];
  int    bh2        = val[left][15];
  double bh1xbh2    = (double)val[right][23];
  double TM         = (double)val[right][3];

  printf("\n");
  printf("                                   \t");
  printf("%-20s %15s\n",   "BH2",            separate_comma(bh2).c_str());
  printf("%-20s %15s\t",   "Spill",          separate_comma(spill).c_str());
  printf("%-20s %15s\n",   "MsT Go",         separate_comma(mstgo).c_str());
  printf("%-20s %15s\t",   "K beam",         separate_comma((int)kbeam).c_str());
  printf("%-20s %15s\n",   "MsT Clear",      separate_comma(mstclear).c_str());
  printf("%-20s %15s\t",   "pi beam",        separate_comma((int)pibeam).c_str());
  printf("%-20s %15.4f\n", "K beam/TM",      kbeam/TM);
  printf("%-20s %15s\t",   "(K,K)",          separate_comma((int)kk).c_str());
  printf("%-20s %15.4f\n", "K beam/BH1xBH2", kbeam/bh1xbh2);
  printf("%-20s %15s\t",   "(K,pi)",         separate_comma((int)kpi).c_str());
  printf("%-20s %15.4f\n", "(K,K)/K-beam",   kk/kbeam);
  printf("%-20s %15s\t",   "L1 req",         separate_comma((int)l1_req).c_str());
  printf("%-20s %15.4f\n", "(K,pi)/K-beam",  kpi/kbeam);
  printf("%-20s %15s\t",   "L1 acc",     separate_comma((int)l1_acc).c_str());
  printf("%-20s %15.4f\n", "Live time/Real time", live_time/real_time);
  printf("%-20s %15s\t",   "Total Clear",    separate_comma(clear).c_str());
  printf("%-20s %15.4f\n", "L1 acc/L1 req",  l1_acc/l1_req);
  printf("%-20s %15s\t",   "L2 acc",     separate_comma((int)l2_acc).c_str());
  printf("%-20s %15.4f\n", "L2 acc/L1 acc",  l2_acc/l1_acc);
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
  if(flag_semi_online){
    if(event_count%300 == 0) en_disp = true;
  }else{
    if(event_count%20 == 0)  en_disp = true;
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
	  inclement_spill = true;
	}

	if(i!=0){
	  if(flag_spill_by_spill){
	    val[right][i]  = curr[right][i];
	  }else{
	    val[right][i] += curr[right][i] - prev[right][i];
	  }
	}
      }

    }

    // inclement spill
    if(inclement_spill)	++val[right][0];

    if(en_disp){
      int event_number = g_unpacker.get_event_number();
      std::cout << std::setw(10) << std::left  << "RUN"
		<< std::setw(11) << std::right << run_number << " : "
		<< std::setw(15) << std::left  << "Event Number"
		<< std::setw(11) << std::right << event_number << " : "
		<< std::endl     << std::endl;

      double real_time = (double)val[right][4];
      double live_time = (double)val[right][5];
      double l1_rec    = (double)val[right][6];
      double l1_acc    = (double)val[right][7];
      double l2_acc    = (double)val[right][12];
      double real_live = live_time/real_time;
      double daq_eff   = l1_acc/l1_rec;
      double l2_eff    = l2_acc/l1_acc;

      printf("%-10s %10s : %-15s %10s : %-15s %15s \n",
	     "- E05 -", "Integral",
	     "- E07 -", "Integral",
	     "", "Integral");

      for(int i = 0; i<NofCh; ++i){
	scaler_info info[size_dispColumn];

	// Counter & DAQ info
	info[left]   = cont_info[left][i];
	info[center] = cont_info[center][i];
	info[right]  = cont_info[right][i];

	// display
	printf("%-10s %10llu : %-15s %10llu : %-15s %15llu \n",
	       info[left].name.c_str(),   val[left][i],
	       info[center].name.c_str(), val[center][i],
	       info[right].name.c_str(),  val[right][i]
	       );
      }

      // additional DAQ information
      printf("\n%-10s %9.4f%% : %-15s %9.4f%% : %-15s %14.4f%%\n",
	     "Live/Real",      real_live*100,
	     "DAQ Eff.",       daq_eff*100,
	     "L2 req ratio",   l2_eff*100);

      double duty_factor = 0;
      if(1-daq_eff == 0){
	duty_factor = 100;
      }else{
	duty_factor = daq_eff/(1-daq_eff)*(1/real_live - 1);
      }
      printf("%-10s %8.4f%%\n",
	     "Duty factor",    duty_factor*100);
    }
  }

  ++event_count;
  en_disp = false;

  return 0;
}

}
