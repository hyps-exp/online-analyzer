// Author: Tomonori Takahashi

#include <iomanip>
#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <vector>

#include "DetectorID.hh"
#include "user_analyzer.hh"
#include "UnpackerManager.hh"
#include "Unpacker.hh"
#include "DAQNode.hh"
#include "ConfMan.hh"
#include "EMCParamMan.hh"

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  static const std::string space_name("analyzer");

  static const std::string& hostname = std::getenv("HOSTNAME");
  static const std::string& exechost = "sksterm4";
  static bool emc_alart[3] = { false, false, false };
  static const double spill_prescale = 1.;
  static const double emc_x_offset = 500000 - 303300;
  static const double emc_y_offset = 500000 + 164000;
  static const int NumOfAPVDAQ = 3;
  static const int MaxEventBuffer = 100;
  int  event_buffer = 0;
  int  emc_state = -1;
  std::bitset<NumOfAPVDAQ*(NumOfLayersSSD1+NumOfLayersSSD1)> ssd_bit;
  bool ssdt_flag = false;
  bool ssd1_flag[NumOfLayersSSD1][NumOfAPVDAQ][MaxEventBuffer];
  bool ssd2_flag[NumOfLayersSSD2][NumOfAPVDAQ][MaxEventBuffer];

//____________________________________________________________________________
int
process_begin( const std::vector<std::string>& argv )
{
  static const std::string func_name("["+space_name+"::"+__func__+"()]");

  ConfMan& gConfMan = ConfMan::getInstance();
  gConfMan.initialize( argv );
  gConfMan.initializeEMCParamMan();
  if( !gConfMan.isGood() ) return -1;
  
  for( int i=0; i<MaxEventBuffer; ++i ){
    for( int apv=0; apv<NumOfAPVDAQ; ++apv ){
      for( int l=0; l<NumOfLayersSSD1; ++l )
	ssd1_flag[l][apv][i] = true;
      for( int l=0; l<NumOfLayersSSD2; ++l )
	ssd2_flag[l][apv][i] = true;
    }
  }

  return 0;
}

//____________________________________________________________________________
int
process_end( void )
{
  return 0;
}

//____________________________________________________________________________
int
process_event( void )
{
  static const std::string func_name("["+space_name+"::"+__func__+"()]");

  static UnpackerManager& g_unpacker  = GUnpacker::get_instance();
  static EMCParamMan&     emc_manager = EMCParamMan::GetInstance();
  
  const int run_number   = g_unpacker.get_root()->get_run_number();
  const int event_number = g_unpacker.get_event_number();
 
  event_buffer++;
  event_buffer = event_buffer%MaxEventBuffer;

  for( int apv=0; apv<NumOfAPVDAQ; ++apv ){
    for( int l=0; l<NumOfLayersSSD1; ++l )
      ssd1_flag[l][apv][event_buffer] = false;
    for( int l=0; l<NumOfLayersSSD2; ++l )
      ssd2_flag[l][apv][event_buffer] = false;
  }

  // EMC -----------------------------------------------------------
  static const int nspill = emc_manager.NSpill()*spill_prescale;
  static int spill  = 0;
  static int rspill = 0;
  {
    static const int k_device = g_unpacker.get_device_id("EMC");
    static const int k_xpos   = g_unpacker.get_data_id("EMC", "xpos");
    static const int k_ypos   = g_unpacker.get_data_id("EMC", "ypos");
    static const int k_state  = g_unpacker.get_data_id("EMC", "state");
    double xpos = -9999.;
    double ypos = -9999.;
    int xpos_nhit = g_unpacker.get_entries( k_device, 0, 0, 0, k_xpos );
    if( xpos_nhit!=0 ) xpos = g_unpacker.get( k_device, 0, 0, 0, k_xpos );
    int ypos_nhit = g_unpacker.get_entries( k_device, 0, 0, 0, k_ypos );
    if( ypos_nhit!=0 ) ypos = g_unpacker.get( k_device, 0, 0, 0, k_ypos );
    xpos -= emc_x_offset;
    ypos -= emc_y_offset;
    if( spill > emc_manager.Pos2Spill( xpos, ypos )*spill_prescale ){
      emc_alart[0] = false; emc_alart[1] = false; emc_alart[2] = false;
    }
    spill = emc_manager.Pos2Spill( xpos, ypos )*spill_prescale;
    rspill = nspill - spill;

    int nhit = g_unpacker.get_entries( k_device, 0, 0, 0, k_state );
    if( nhit>0 )
      emc_state = g_unpacker.get( k_device, 0, 0, 0, k_state );
    else
      emc_state = -1;
  }

  // SSDT ---------------------------------------------------------
  {
    static const int k_device = g_unpacker.get_device_id("SSDT");
    static const int k_tdc    = g_unpacker.get_data_id("SSDT","tdc");
    int nhit = g_unpacker.get_entries( k_device, 0, 0, 0, k_tdc );
    if( nhit!=0 ){
      int tdc  = g_unpacker.get( k_device, 0, 0, 0, k_tdc );
      if( tdc>0 ) ssdt_flag = true;
    }
  }

  // SSD1 ---------------------------------------------------------
  {
    static const int k_device = g_unpacker.get_device_id("SSD1");
    static const int k_adc    = g_unpacker.get_data_id("SSD1","adc");
    static const int k_flag   = g_unpacker.get_data_id("SSD1","flag");
    for( int l=0; l<NumOfLayersSSD1; ++l ){
      for( int seg=0; seg<NumOfSegSSD1; ++seg ){
	int  nhit_adc  = g_unpacker.get_entries( k_device, l, seg, 0, k_adc  );
	int  nhit_flag = g_unpacker.get_entries( k_device, l, seg, 0, k_flag );
	if( nhit_flag==0 || nhit_adc==0 ) continue;
	int flag = g_unpacker.get( k_device, l, seg, 0, k_flag );
	if( flag==0 ) continue;
	int adc = g_unpacker.get( k_device, l, seg, 0, k_adc );
	if( adc>0 ){
	  int apv = seg/(NumOfSegSSD1/NumOfAPVDAQ);
	  ssd1_flag[l][apv][event_buffer] = true;
	}
      }//for(seg)
    }//for(l)
  }

  // SSD2 ---------------------------------------------------------
  {
    static const int k_device = g_unpacker.get_device_id("SSD2");
    static const int k_adc    = g_unpacker.get_data_id("SSD2","adc");
    static const int k_flag   = g_unpacker.get_data_id("SSD2","flag");
    for( int l=0; l<NumOfLayersSSD2; ++l ){
      for( int seg=0; seg<NumOfSegSSD2; ++seg ){
	int  nhit_adc  = g_unpacker.get_entries( k_device, l, seg, 0, k_adc  );
	int  nhit_flag = g_unpacker.get_entries( k_device, l, seg, 0, k_flag );
	if( nhit_flag==0 || nhit_adc==0 ) continue;
	int flag = g_unpacker.get( k_device, l, seg, 0, k_flag );
	if( flag==0 ) continue;
	int adc = g_unpacker.get( k_device, l, seg, 0, k_adc );
	if( adc>0 ){
	  int apv = seg/(NumOfSegSSD2/NumOfAPVDAQ);
	  ssd2_flag[l][apv][event_buffer] = true;
	}
      }//for(seg)
    }//for(l)
  }

  ssd_bit.reset();
  for( int i=0; i<MaxEventBuffer; ++i ){
    for( int apv=0; apv<NumOfAPVDAQ; ++apv ){
      for( int l=0; l<NumOfLayersSSD1; ++l ){
	if( ssd1_flag[l][apv][i] ){
	  ssd_bit.set(apv+l*NumOfAPVDAQ);
	}
      }
      for( int l=0; l<NumOfLayersSSD2; ++l ){
	if( ssd2_flag[l][apv][i] ){
	  ssd_bit.set(apv+l*NumOfAPVDAQ+NumOfAPVDAQ*NumOfLayersSSD1);
	}
      }
    }
  }

  std::cout << "#D Run# " << std::setw(5) << run_number << " : "
	    << "Event# " << std::setw(9) << event_number << std::endl;

  if( spill>=0 ){
    std::cout << "   Spill# " << std::setw(4) << spill << "/" << nspill;
    int rsec  = int(rspill*5.52);
    int rhour = rsec/3600;
    int rmin  = rsec/60 - rhour*60;
    rsec = rsec%60;
    std::cout << " -> ";
    if( rhour<1 ){
      std::cout << "\e[33;1m";
      if( !emc_alart[0] && hostname==exechost ){
	emc_alart[0] = true;
	system("sh /home/sks/bin/emc_spill_alart.sh");
      }
    }
    if( rhour<1 && rmin<10 ){
      std::cout << "\e[35;1m";
      if( !emc_alart[1] && hostname==exechost ){
	emc_alart[1] = true;
	system("sh /home/sks/bin/emc_spill_alart.sh");
      }
    }
    if( rspill==0 ){
      std::cout << "\e[31;1m";
      if( !emc_alart[2] && hostname==exechost ){
	emc_alart[2] = true;
	system("sh /home/sks/bin/emc_spill_alart.sh");
      }
    }
    std::cout << std::setw(4) << rspill << " : "
	      << rhour << "h " << rmin << "m " << rsec << "s\e[m" << std::endl;
  }
    
  if( emc_state==0 ){
    std::string s("PowerOff");
    std::cout << "   EMC state : " << "\e[32;1m" << s
	      << "\e[m" << std::endl;
  }else if( emc_state==1 ){
    std::string s("PowerOn");
    std::cout << "   EMC state : " << "\e[35;1m" << s
	      << "\e[m" << std::endl;
    // system("sh /home/sks/bin/emc_state_alart.sh");
  }else{
    std::string s("Moving");
    std::cout << "   EMC state : " << "\e[31;1m" << s
	      << "\e[m" << std::endl;
    // system("sh /home/sks/bin/emc_state_alart.sh");
  }

  {
    TString s = ssd_bit.to_string();
    s.ReplaceAll("1",".");
    s.ReplaceAll("0","!");
    if( ssd_bit.count()==ssd_bit.size() )
      std::cout << "   SSD state : " << "\e[32;1m" << s << "\e[m" << std::endl;
    else if( ( ssd_bit.to_ulong()&0xfff )==0xfff )
      std::cout << "   SSD state : " << "\e[33;1m" << s << "\e[m" << std::endl;
    else
      std::cout << "   SSD state : " << "\e[31;1m" << s << "\e[m" << std::endl;
  }

  return 0;
}

}
