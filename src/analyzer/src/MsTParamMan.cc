/*
 */

#include <sstream>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <algorithm>

#include "MsTParamMan.hh"
#include "DetectorID.hh"

static const std::string class_name("MsTParamMan");

//_____________________________________________________________________
MsTParamMan::MsTParamMan( void )
{
}

//_____________________________________________________________________
MsTParamMan::~MsTParamMan( void )
{
}

//_____________________________________________________________________
bool
MsTParamMan::Initialize( const std::string& filename )
{
  static const std::string func_name("["+class_name+"::"+__func__+"()]");

  {
    std::ifstream ifs( filename.c_str() );
    if( !ifs.is_open() ){
      std::cerr << "#E " << func_name << " "
		<< "No such parameter file : " << filename << std::endl;
      std::exit( EXIT_FAILURE );
    }

    const int NumOfSegDetA = NumOfSegTOF+2;
    const int NumOfSegDetB = NumOfSegSCH-5;

    m_low_threshold.resize( NumOfSegDetA );
    m_high_threshold.resize( NumOfSegDetA );
    for( int i=0; i<NumOfSegDetA; ++i ){
      m_low_threshold[i].resize( NumOfSegDetB );
      m_high_threshold[i].resize( NumOfSegDetB );
    }

    std::string line;
    int tofseg = 0;
    int LorH   = 0;
    while( !ifs.eof() && std::getline( ifs, line ) ){
      if( line.empty() ) continue;

      std::string param[NumOfSegDetB];
      std::istringstream iss( line );

      for( int i=0; i<NumOfSegDetB; ++i ){
	iss >> param[i];
      }
      if( param[0].at(0) == '#' ) continue;
      if( param[1] == "Mem_Thr_Low" ){
	tofseg =  0;
	LorH   = -1;
	continue;
      }
      if( param[1] == "Mem_Thr_Hi" ){
	tofseg =  0;
	LorH   =  1;
	continue;
      }

      if( param[NumOfSegDetB-1].empty() ) continue;
      for( int i=0; i<NumOfSegDetB; ++i ){
	std::replace( param[i].begin(), param[i].end(), '(', ' ');
	std::replace( param[i].begin(), param[i].end(), ')', ' ');
	std::replace( param[i].begin(), param[i].end(), ',', ' ');
	if( LorH == -1 )
	  m_low_threshold[tofseg][i] = std::strtol( param[i].c_str(), NULL, 0 );
	if( LorH ==  1 )
	  m_high_threshold[tofseg][i] = std::strtol( param[i].c_str(), NULL, 0 );
      }
      ++tofseg;
    }

#if 0
    // Low
    std::cout << "Low Threshold" << std::endl;
    for( int i=0; i<NumOfSegDetA; ++i ){
      std::cout << i << "\t:";
      for( int j=0; j<NumOfSegDetB; ++j ){
	std::cout << " " << m_low_threshold[i][j];
      }
      std::cout << std::endl;
    }
    // High
    std::cout << "High Threshold" << std::endl;
    for( int i=0; i<NumOfSegDetA; ++i ){
      std::cout << i << "\t:";
      for( int j=0; j<NumOfSegDetB; ++j ){
	std::cout << " " << m_high_threshold[i][j];
      }
      std::cout << std::endl;
    }
#endif
  }

  std::cout << "#D " << func_name << " "
	    << "Initialized"
	    << std::endl;

  return true;
}

//_____________________________________________________________________
bool
MsTParamMan::IsAccept( int detA, int detB, int tdc )
{
  int low  = m_low_threshold[detA][detB];
  int high = m_high_threshold[detA][detB];
  return ( low < tdc && tdc < high );
}
