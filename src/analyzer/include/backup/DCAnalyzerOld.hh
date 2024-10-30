// Author : Hitoshi Sugimura

#ifndef DCANALYZER_OLD_H
#define DCANALYZER_OLD_H

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <map>

//______________________________________________________________________________
class DCRHC
{
private:
  std::vector<std::vector<int> >     m_hitwire;
  std::vector<std::vector<double> >  m_hitpos;
  std::vector<std::vector<int> >     m_tdc;
  std::vector<std::vector<double> >  m_dtime;
  std::vector<std::vector<double> >  m_dlength;
  std::vector<double> buf_hitpos;
  std::vector<int>    H;
  std::vector<double> A;
  std::vector<double> z;
  std::vector<double> sinvector;
  std::vector<double> cosvector;
  std::vector<std::vector<double> > inv_matrix;
  std::vector<int> lr;
  double X0;
  double Y0;
  double dXdZ;
  double dYdZ;
  int    detid;
  double chi2;

public:
  DCRHC( int DetectorID );
  ~DCRHC( void );
  bool   TrackSearch( int min_plane );
  bool   PairPlaneSearch( int pairplane );
  bool   make_MinimumPoint( double& dxdZ,double& dydZ,double& x0,double& y0 );
  double GetChisquare( void ){ return chi2; }
  double GetDriftLength( int plane );
  double GetDriftTime( int plane );
  int    Getlr( int plane );
  double GetPosX( int PosZ );
  double GetPosY( int PosZ );
  double GetResidual( int plane );
  double GetResidualSdcOut( int plane );
  double GetU( void ){ return dXdZ; }
  double GetV( void ){ return dYdZ; }
  double GetX0( void ){ return X0; }
  double GetY0( void ){ return Y0; }
  std::vector<int > GetTdc( int plane );
  std::vector<int > GetHitWire( int plane );
  void FullTrackSearch( void );
  void HitPosition( int DetectorID );
  void hoge( void );
  void init_BcOut( void );
  void init_SdcIn( void );
  void init_SdcOut( void );
  void init_Ssd( void );
  void pushback( int DetectorID );
  void pushback_BcOut( void );
  void pushback_SdcIn( void );
  void pushback_SdcOut( void );
  void pushback_Ssd( void );
  void ReHitPosition( int DetectorID );
  void SetGeomBcOut( std::vector<double> & wo,
		     std::vector<double> & pit,
		     std::vector<double> & wc );
  void SetGeomSdcIn( std::vector<double> & wo,
		     std::vector<double> & pit,
		     std::vector<double> & wc );
  void SetGeomSdcOut( std::vector<double> & wo,
		      std::vector<double> & pit,
		      std::vector<double> & wc );
  void makeAvector( void );
  void makeChisquare( int DetectorID );
  void makeChisquareSdcOut();
  void makeDriftLength( int DetectorID );
  void makeDriftTime( int DetectorID );
  void makeHvector( int DetectorID );
  void makeMatrix( void );
  int  num_Hitplane( void );
  std::vector<double> resolveLR( double left1,
				 double right1,
				 double left2,
				 double right2 );
  double TimeToLength( std::vector<std::vector<double> > dc_geom,
		       std::map<double,double> t0list );
private:
  DCRHC( const DCRHC& );
  DCRHC& operator = ( const DCRHC );
};

// Global functions
std::vector<std::vector<double> >
InvMatrix( std::vector<std::vector<double> > Matrix );

std::vector<double>
angle( int degree );

#endif
