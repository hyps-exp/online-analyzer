// -*- C++ -*-

#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cstdlib>

#include "MatrixParamMan.hh"
#include "ConfMan.hh"
#include "DetectorID.hh"
#include "Exception.hh"
#include "FuncName.hh"

ClassImp(MatrixParamMan);

//_____________________________________________________________________
MatrixParamMan::MatrixParamMan( void )
  : TObject()
{
}

//_____________________________________________________________________
MatrixParamMan::~MatrixParamMan( void )
{
}

//_____________________________________________________________________
Bool_t
MatrixParamMan::Initialize( const TString& filename_2d,
			    const TString& filename_3d )
{
  {
    std::ifstream ifs2d( filename_2d );
    if( !ifs2d.is_open() ){
      std::cerr << "#E " << FUNC_NAME << " "
      		<< "No such parameter file : " << filename_2d << std::endl;
      std::exit( EXIT_FAILURE );
    }

    m_enable_2d.resize( NumOfSegTOF );
    for( Int_t i=0; i<NumOfSegTOF; ++i ){
      m_enable_2d.at(i).resize( NumOfSegSCH );
    }

    std::string line;
    Int_t tofseg = 0;
    while( !ifs2d.eof() && std::getline( ifs2d, line ) ){
      if( line.empty() ) continue;

      std::string param[2];
      std::istringstream iss( line );
      iss >> param[0] >> param[1];

      if( param[0].at(0) == '#' ) continue;

      if( param[0].substr(2).at(0)=='0' )
	param[0] = param[0].substr(3);
      else
	param[0] = param[0].substr(2);

      Int_t channel = std::strtol( param[0].c_str(), NULL, 0 );
      Int_t enable  = std::strtol( param[1].substr(0,1).c_str(), NULL, 0 );

      m_enable_2d[tofseg][channel] = enable;
      if( channel==NumOfSegSCH-1 ) ++tofseg;
    }

#if 1
    // std::cout << "#D " << FUNC_NAME << " "
    //           << "2D Parameter" << std::endl;
    for( Int_t i=NumOfSegTOF-1; i>=0; --i ){
      std::cout << std::setw(2) << i << " :";
      for( Int_t j=0; j<NumOfSegSCH; ++j ){
	std::cout << m_enable_2d[i][j];
      }
      std::cout << std::endl;
    }
#endif
  }

  {
    std::ifstream ifs3d( filename_3d );
    if( !ifs3d.is_open() ){
      std::cerr << "#E " << FUNC_NAME << " "
      		<< "No such parameter file : " << filename_3d << std::endl;
      std::exit( EXIT_FAILURE );
    }

    m_enable_3d.resize( NumOfSegTOF );
    for( Int_t i=0; i<NumOfSegTOF; ++i ){
      m_enable_3d[i].resize( NumOfSegSCH );
      for( Int_t j=0; j<NumOfSegSCH; ++j ){
	m_enable_3d[i][j].resize( NumOfSegClusteredFBH );
      }
    }

    std::string line;
    Int_t tofseg = 0;
    while( !ifs3d.eof() && std::getline( ifs3d, line ) ){
      if( line.empty() ) continue;

      std::string param[2];
      std::istringstream iss( line );
      iss >> param[0] >> param[1];

      if( param[0].at(0) == '#' ) continue;

      if( param[0].substr(2).at(0)=='0' )
	param[0] = param[0].substr(3);
      else
	param[0] = param[0].substr(2);

      Int_t channel = std::strtol( param[0].c_str(), NULL, 0 );
      Int_t enable[NumOfSegClusteredFBH] = {};
      for( Int_t i=0; i<NumOfSegClusteredFBH; ++i ){
	enable[i] = std::strtol( param[1].substr(i,1).c_str(), NULL, 0 );
	m_enable_3d[tofseg][channel][i] = enable[i];
      }

      if( channel==NumOfSegSCH-1 ) ++tofseg;
    }

#if 0
    // std::cout << "#D " << FUNC_NAME << " "
    //           << "3D Parameter" << std::endl;
    for( Int_t k=0; k<NumOfSegClusteredFBH; ++k ){
      std::cout << "Clustered FBH : " << k << std::endl;
      for( Int_t i=NumOfSegTOF-1; i>=0; --i ){
	std::cout << std::setw(2) << i << " :";
	for( Int_t j=0; j<NumOfSegSCH; ++j ){
	  std::cout << m_enable_3d[i][j][k];
	}
	std::cout << std::endl;
      }
    }
#endif
  }

  return true;
}

//_____________________________________________________________________
Bool_t
MatrixParamMan::IsAccept( Int_t detA, Int_t detB )
{
  try {
    Int_t enable = m_enable_2d.at( detA ).at( detB );
    return ( enable == 1 );
  } catch( const std::out_of_range& e ) {
    throw Exception( FUNC_NAME+" "+e.what() );
  }
}

//_____________________________________________________________________
Bool_t
MatrixParamMan::IsAccept( Int_t detA, Int_t detB, Int_t detC )
{
  try {
    Int_t enable = m_enable_3d.at( detA ).at( detB ).at( detC );
    return ( enable == 1 );
  } catch( const std::out_of_range& e ) {
    throw Exception( FUNC_NAME+" "+e.what() );
  }
}
