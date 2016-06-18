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

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  static const std::string space_name("analyzer");

  static const int NumOfAPVDAQ = 3;
  static const int MaxEventBuffer = 100;
  int  event_buffer = 0;
  int  emc_state = -1;
  int  ssd_state = -1;
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

  static UnpackerManager& g_unpacker = GUnpacker::get_instance();
  
  const int run_number   = g_unpacker.get_root()->get_run_number();
  const int event_number = g_unpacker.get_event_number();
 
  event_buffer = (++event_buffer)%MaxEventBuffer;

  for( int apv=0; apv<NumOfAPVDAQ; ++apv ){
    for( int l=0; l<NumOfLayersSSD1; ++l )
      ssd1_flag[l][apv][event_buffer] = false;
    for( int l=0; l<NumOfLayersSSD2; ++l )
      ssd2_flag[l][apv][event_buffer] = false;
  }

  // EMC -----------------------------------------------------------
  {
    static const int k_device = g_unpacker.get_device_id("EMC");
    static const int k_state   = g_unpacker.get_data_id("EMC", "state");
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
	if( nhit_flag==0 ) continue;
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
	if( nhit_flag==0 ) continue;
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

  ssd_state = 0;
  for( int i=0; i<MaxEventBuffer; ++i ){
    for( int apv=0; apv<NumOfAPVDAQ; ++apv ){
      for( int l=0; l<NumOfLayersSSD1; ++l ){
	if( ssd1_flag[l][apv][i] )
	  ssd_state |= 0x1<<(apv+l*NumOfAPVDAQ);
      }
      for( int l=0; l<NumOfLayersSSD2; ++l ){
	if( ssd2_flag[l][apv][i] ){
	  ssd_state |= 0x1<<(apv+l*NumOfAPVDAQ+NumOfAPVDAQ*NumOfLayersSSD1);
	}
      }
    }
  }

  std::cout << "#D Event# " << std::setw(6) << event_number << std::endl;

  if( emc_state==0 )
    std::cout << "   EMC state : " << "\e[32;1m" << emc_state
	      << "\e[m" << std::endl;
  else if( emc_state==1 )
    std::cout << "   EMC state : " << "\e[35;1m" << emc_state
	      << "\e[m" << std::endl;
  else
    std::cout << "   EMC state : " << "\e[31;1m" << emc_state
	      << "\e[m" << std::endl;

  if( ssd_state==0xffffff )
    std::cout << "   SSD state : " << "\e[32;1m" << static_cast< std::bitset<24> >(ssd_state) << "\e[m" << std::endl;
  else if( ( ssd_state&0xfff )==0xfff )
    std::cout << "   SSD state : " << "\e[33;1m" << static_cast< std::bitset<24> >(ssd_state) << "\e[m" << std::endl;
  else
    std::cout << "   SSD state : " << "\e[31;1m" << static_cast< std::bitset<24> >(ssd_state) << "\e[m" << std::endl;

  return 0;
}

}