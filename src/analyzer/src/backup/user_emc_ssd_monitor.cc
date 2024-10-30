// Author: Tomonori Takahashi

#include <iomanip>
#include <iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <vector>

#include <TTimeStamp.h>

#include "DetectorID.hh"
#include "user_analyzer.hh"
#include "UnpackerManager.hh"
#include "Unpacker.hh"
#include "DAQNode.hh"
#include "ConfMan.hh"
#include "EMCParamMan.hh"

namespace analyzer
{

  namespace
  {
    using namespace hddaq::unpacker;
    using namespace hddaq;

    const std::string space_name("analyzer");

    const std::string& hostname = std::getenv("HOSTNAME");
    const std::string& exechost = "sksterm4";
    bool emc_alart[3] = { false, false, false };
    const Double_t spill_prescale = 1.;
    const Double_t spill_cycle    = 5.52; // [s]
    const Double_t emc_x_offset = 500000 - 303300;
    const Double_t emc_y_offset = 500000 + 164000;
    const Int_t NumOfAPVDAQ = 3;
    const Int_t MaxEventBuffer = 100;
    Int_t  event_buffer = 0;
    Int_t  emc_state = -1;
    std::bitset<NumOfAPVDAQ*(NumOfLayersSSD1+NumOfLayersSSD1)> ssd_bit;
    bool ssdt_flag = false;
    bool ssd1_flag[NumOfLayersSSD1][NumOfAPVDAQ][MaxEventBuffer];
    bool ssd2_flag[NumOfLayersSSD2][NumOfAPVDAQ][MaxEventBuffer];
  }

//____________________________________________________________________________
Int_t
process_begin( const std::vector<std::string>& argv )
{
  static const std::string func_name("["+space_name+"::"+__func__+"()]");

  ConfMan& gConfMan = ConfMan::GetInstance();
  gConfMan.Initialize( argv );
  gConfMan.InitializeEMCParamMan();
  if( !gConfMan.IsGood() ) return -1;

  for( Int_t i=0; i<MaxEventBuffer; ++i ){
    for( Int_t apv=0; apv<NumOfAPVDAQ; ++apv ){
      for( Int_t l=0; l<NumOfLayersSSD1; ++l )
	ssd1_flag[l][apv][i] = true;
      for( Int_t l=0; l<NumOfLayersSSD2; ++l )
	ssd2_flag[l][apv][i] = true;
    }
  }

  return 0;
}

//____________________________________________________________________________
Int_t
process_end( void )
{
  return 0;
}

//____________________________________________________________________________
Int_t
process_event( void )
{
  static const std::string func_name("["+space_name+"::"+__func__+"()]");

  static UnpackerManager& g_unpacker  = GUnpacker::get_instance();
  static EMCParamMan&     emc_manager = EMCParamMan::GetInstance();

  const Int_t run_number   = g_unpacker.get_root()->get_run_number();
  const Int_t event_number = g_unpacker.get_event_number();

  event_buffer++;
  event_buffer = event_buffer%MaxEventBuffer;

  for( Int_t apv=0; apv<NumOfAPVDAQ; ++apv ){
    for( Int_t l=0; l<NumOfLayersSSD1; ++l )
      ssd1_flag[l][apv][event_buffer] = false;
    for( Int_t l=0; l<NumOfLayersSSD2; ++l )
      ssd2_flag[l][apv][event_buffer] = false;
  }

  // EMC -----------------------------------------------------------
  static const Int_t nspill = emc_manager.NSpill()*spill_prescale;
  static Int_t spill  = 0;
  static Int_t rspill = 0;
  {
    static const Int_t k_device = g_unpacker.get_device_id("EMC");
    static const Int_t k_xpos   = g_unpacker.get_data_id("EMC", "xpos");
    static const Int_t k_ypos   = g_unpacker.get_data_id("EMC", "ypos");
    static const Int_t k_state  = g_unpacker.get_data_id("EMC", "state");
    Double_t xpos = -9999.;
    Double_t ypos = -9999.;
    Int_t xpos_nhit = g_unpacker.get_entries( k_device, 0, 0, 0, k_xpos );
    if( xpos_nhit!=0 ) xpos = g_unpacker.get( k_device, 0, 0, 0, k_xpos );
    Int_t ypos_nhit = g_unpacker.get_entries( k_device, 0, 0, 0, k_ypos );
    if( ypos_nhit!=0 ) ypos = g_unpacker.get( k_device, 0, 0, 0, k_ypos );
    xpos -= emc_x_offset;
    ypos -= emc_y_offset;
    Double_t pos2spill = emc_manager.Pos2Spill( xpos, ypos )*spill_prescale;
    if( spill > pos2spill ){
      emc_alart[0] = false; emc_alart[1] = false; emc_alart[2] = false;
    }
    spill = pos2spill;
    rspill = nspill - spill;

    Int_t nhit = g_unpacker.get_entries( k_device, 0, 0, 0, k_state );
    if( nhit>0 )
      emc_state = g_unpacker.get( k_device, 0, 0, 0, k_state );
    else
      emc_state = -1;
  }

  // SSDT ---------------------------------------------------------
  {
    static const Int_t k_device = g_unpacker.get_device_id("SSDT");
    static const Int_t k_tdc    = g_unpacker.get_data_id("SSDT","tdc");
    Int_t nhit = g_unpacker.get_entries( k_device, 0, 0, 0, k_tdc );
    if( nhit!=0 ){
      Int_t tdc  = g_unpacker.get( k_device, 0, 0, 0, k_tdc );
      if( tdc>0 ) ssdt_flag = true;
    }
  }

  // SSD1 ---------------------------------------------------------
  {
    static const Int_t k_device = g_unpacker.get_device_id("SSD1");
    static const Int_t k_adc    = g_unpacker.get_data_id("SSD1","adc");
    static const Int_t k_flag   = g_unpacker.get_data_id("SSD1","flag");
    for( Int_t l=0; l<NumOfLayersSSD1; ++l ){
      for( Int_t seg=0; seg<NumOfSegSSD1; ++seg ){
	Int_t  nhit_adc  = g_unpacker.get_entries( k_device, l, seg, 0, k_adc  );
	Int_t  nhit_flag = g_unpacker.get_entries( k_device, l, seg, 0, k_flag );
	if( nhit_flag==0 || nhit_adc==0 ) continue;
	Int_t flag = g_unpacker.get( k_device, l, seg, 0, k_flag );
	if( flag==0 ) continue;
	Int_t adc = g_unpacker.get( k_device, l, seg, 0, k_adc );
	if( adc>0 ){
	  Int_t apv = seg/(NumOfSegSSD1/NumOfAPVDAQ);
	  ssd1_flag[l][apv][event_buffer] = true;
	}
      }//for(seg)
    }//for(l)
  }

  // SSD2 ---------------------------------------------------------
  {
    static const Int_t k_device = g_unpacker.get_device_id("SSD2");
    static const Int_t k_adc    = g_unpacker.get_data_id("SSD2","adc");
    static const Int_t k_flag   = g_unpacker.get_data_id("SSD2","flag");
    for( Int_t l=0; l<NumOfLayersSSD2; ++l ){
      for( Int_t seg=0; seg<NumOfSegSSD2; ++seg ){
	Int_t  nhit_adc  = g_unpacker.get_entries( k_device, l, seg, 0, k_adc  );
	Int_t  nhit_flag = g_unpacker.get_entries( k_device, l, seg, 0, k_flag );
	if( nhit_flag==0 || nhit_adc==0 ) continue;
	Int_t flag = g_unpacker.get( k_device, l, seg, 0, k_flag );
	if( flag==0 ) continue;
	Int_t adc = g_unpacker.get( k_device, l, seg, 0, k_adc );
	if( adc>0 ){
	  Int_t apv = seg/(NumOfSegSSD2/NumOfAPVDAQ);
	  ssd2_flag[l][apv][event_buffer] = true;
	}
      }//for(seg)
    }//for(l)
  }

  ssd_bit.reset();
  for( Int_t i=0; i<MaxEventBuffer; ++i ){
    for( Int_t apv=0; apv<NumOfAPVDAQ; ++apv ){
      for( Int_t l=0; l<NumOfLayersSSD1; ++l ){
	if( ssd1_flag[l][apv][i] ){
	  ssd_bit.set(apv+l*NumOfAPVDAQ);
	}
      }
      for( Int_t l=0; l<NumOfLayersSSD2; ++l ){
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
    Int_t rsec  = Int_t(rspill*spill_cycle);
    Int_t rhour = rsec/3600;
    Int_t rmin  = rsec/60 - rhour*60;
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
    TTimeStamp stamp;
    stamp.Add( -TTimeStamp::GetZoneOffset() );
    stamp.Add( Int_t(rspill*spill_cycle) );
    std::cout << std::setw(4) << rspill << " : "
	      << rhour << "h " << rmin << "m " << rsec << "s\e[m" << std::endl
	      << "   End Time  : " << stamp.AsString("s") << std::endl;
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
