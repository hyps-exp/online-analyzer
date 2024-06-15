// -*- C++ -*-

// Author: Tomonori Takahashi

#include <ctime>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

#include "defines.hh"
#include "user_analyzer.hh"
#include "UnpackerManager.hh"
#include "Unpacker.hh"
#include "DAQNode.hh"
#include "ConfMan.hh"

namespace analyzer
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  namespace
  {
    ConfMan& gConfMan = ConfMan::GetInstance();
    UnpackerManager& gUnpacker = GUnpacker::get_instance();
    //std::vector<std::string> target = { "hul01", "hul03" };
    //std::vector<std::string> target = { "vme01" };
    std::vector<std::string> target = {
				       // "vme01",
				       // "vme02",
				       "hul03sdc-1",
				       // "hul01scr-1",
				       // "hul01scr-2",
				       // "hul01scr-3",
				       // "hul01hr-4",
				       // "hul01scr-4",
				       // "hul01scr-2",
				       // "hul_hbx_scr",
				       // "hul02lac",
				       // "hul01hr-1",
				       // "hul01hr-2",
				       // "hul01hr-3"
    };
  }

//______________________________________________________________________________
int
process_begin(const std::vector<std::string>& argv)
{
  gConfMan.Initialize(argv);

  for( int i=0, n=target.size(); i<n; ++i ){
    int node_id = gUnpacker.get_fe_id( target.at(i) );
    Unpacker *node = gUnpacker.get_root()->get_child(node_id);
    if( !node ) continue;
    node->set_dump_mode(defines::k_hex);
  }

  // for( auto&& c : gUnpacker.get_root()->get_child_list() ){
  //   c.second->set_dump_mode(defines::k_hex);
  // }

  return 0;
}

//______________________________________________________________________________
int
process_end( void )
{
  return 0;
}

//______________________________________________________________________________
int
process_event( void )
{
#if 0
  Int_t device_id = gUnpacker.get_device_id("E72BAC");
  gUnpacker.dump_data_device(device_id);
#endif

#if 0
  // std::cout << TString('=', 80) << std::endl;
  Int_t eb_id = gUnpacker.get_fe_id("k18eb"); // Event builder
  std::time_t eb_time = gUnpacker.get_node_header(eb_id, DAQNode::k_unix_time);
  Int_t i = 0;
  for( auto&& c : gUnpacker.get_root()->get_child_list() ){
    if( c.second ){
      TString n = c.second->get_name();
      std::time_t t = gUnpacker.get_node_header(c.first, DAQNode::k_unix_time);
      if( t == 0 )
	continue;
      if( TMath::Abs( t - eb_time ) <= 1 )
        continue;
      char date[64];
      std::strftime(date, sizeof(date), "%Y/%m/%d %a %H:%M:%S", std::localtime(&t));
      char eb_date[64];
      std::strftime(eb_date, sizeof(eb_date), "%Y/%m/%d %a %H:%M:%S", std::localtime(&eb_time));
      cout << std::left << std::setw(20) << n
      	   << std::left << std::setw(28) << date << " " << eb_date << std::endl;
    }
    //c.second->get_header(DAQNode::k_unix_time) << std::endl;
  }
  // std::cout << std::endl;
#endif

  // int node_id = gUnpacker.get_fe_id( target.at(0) );
  // int node_id = gUnpacker.get_fe_id( "hul04scr" );
  // gUnpacker.dump_data_fe(node_id);

  /*
    int node_id = gUnpacker.get_fe_id("k18eb"); // Event builder
    std::cout << g_unpacker.get_node_header(node_id, DAQNode::k_magic) << std::dec << std::endl;
    std::cout << g_unpacker.get_node_header(node_id, DAQNode::k_data_size) << std::endl;
    std::cout << g_unpacker.get_node_header(node_id, DAQNode::k_event_number) << std::endl;
    std::cout << g_unpacker.get_node_header(node_id, DAQNode::k_run_number) << std::hex <<std::endl;
    std::cout << g_unpacker.get_node_header(node_id, DAQNode::k_node_id) << std::endl;
    std::cout << g_unpacker.get_node_header(node_id, DAQNode::k_node_type) << std::dec <<std::endl;
    std::cout << g_unpacker.get_node_header(node_id, DAQNode::k_number_of_blocks) << std::dec <<std::endl;
    std::cout << g_unpacker.get_node_header(node_id, DAQNode::k_unix_time) << std::dec <<std::endl;

    std::cout << "EEE" << std::endl;
    std::cout << g_unpacker.get_root()->get_run_number() << std::endl;
  */
  return 0;
}

}
