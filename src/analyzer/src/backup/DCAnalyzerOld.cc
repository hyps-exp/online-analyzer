// -*- C++ -*-

// Author : Hitoshi Sugimura

#include "DCAnalyzerOld.hh"

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <cmath>

#include "UnpackerManager.hh"
#include "UserParamMan.hh"
#include "DetectorID.hh"
#include "DCTdcCalibMan.hh"
#include "DCGeomMan.hh"
#include "DCDriftParamMan.hh"
#include "FuncName.hh"

#define DEBUG  0
#define TDC1st 1

namespace
{
  using namespace hddaq::unpacker;
  using namespace hddaq;

  UnpackerManager& g_unpacker = GUnpacker::get_instance();

  DCTdcCalibMan&   t0man    = DCTdcCalibMan::GetInstance();
  DCDriftParamMan& driftman = DCDriftParamMan::GetInstance();
  DCGeomMan&       geomman  = DCGeomMan::GetInstance();
  const UserParamMan &gUser = UserParamMan::GetInstance();

  // tdc cut
  const int tdc_min = 550;
  const int tdc_max = 700;
  const int sdc2_tdc_min = 550;
  const int sdc2_tdc_max = 700;
}

//______________________________________________________________________________
DCRHC::DCRHC( int DetectorID )
{
  if(DetectorID==DetIdBcOut)  init_BcOut();
  if(DetectorID==DetIdSdcIn)  init_SdcIn();
  if(DetectorID==DetIdSdcOut) init_SdcOut();
  if(DetectorID==DetIdSsd)    init_Ssd();
  detid = DetectorID;
  chi2 = -1;
}
//______________________________________________________________________________
void DCRHC::init_BcOut( void )
{
  m_hitwire.resize(NumOfLayersBcOut);
  m_hitpos.resize(NumOfLayersBcOut);
  m_tdc.resize(NumOfLayersBcOut);

  for(int id=113;id<125;++id)
    {
      std::vector<double> tilt = angle(geomman.GetTiltAngle(id));
      sinvector.push_back(tilt[0]);
      cosvector.push_back(tilt[1]);
      z.push_back(geomman.GetLocalZ(id));
    }
}
//______________________________________________________________________________
void DCRHC::init_SdcIn( void )
{
  m_hitwire.resize(NumOfLayersSdcIn);
  m_hitpos.resize(NumOfLayersSdcIn);
  m_tdc.resize(NumOfLayersSdcIn);

  for(int id=1;id<11;++id)
    {
      std::vector<double> tilt = angle(geomman.GetTiltAngle(id));
      sinvector.push_back(tilt[0]);
      cosvector.push_back(tilt[1]);
      z.push_back(geomman.GetLocalZ(id));
    }
}
//______________________________________________________________________________
void DCRHC::init_SdcOut( void )
{
  m_hitwire.resize(NumOfLayersSdcOut);
  m_hitpos.resize(NumOfLayersSdcOut);
  m_tdc.resize(NumOfLayersSdcOut);

  for(int id=31;id<43;++id)
    {
      std::vector<double> tilt = angle(geomman.GetTiltAngle(id));
      sinvector.push_back(tilt[0]);
      cosvector.push_back(tilt[1]);
      z.push_back(geomman.GetLocalZ(id));
    }
}
//______________________________________________________________________________
void DCRHC::init_Ssd( void )
{
  m_hitwire.resize(NumOfLayersSsd);
  m_hitpos.resize(NumOfLayersSsd);
  m_tdc.resize(NumOfLayersSsd);

  for(int id=141; id<=150; ++id){
    std::vector<double> tilt = angle(geomman.GetTiltAngle(id));
    sinvector.push_back(tilt[0]);
    cosvector.push_back(tilt[1]);
    z.push_back(geomman.GetLocalZ(id));
  }
}
//______________________________________________________________________________
DCRHC::~DCRHC( void )
{
  m_hitwire.clear();
  m_hitpos.clear();
  m_tdc.clear();
  m_dtime.clear();
  m_dlength.clear();
  lr.clear();
}
//______________________________________________________________________________
void DCRHC::hoge( void )
{
  std::cout<<"////////// vector ////////////"<<std::endl;
  std::cout<<"[ ";
  for(int i=0;i<12;++i)
    {
      std::cout<<sinvector[i]<<" ";
    }
  std::cout<<"]"<<std::endl;
  std::cout<<"////////// Hit Wire ////////////"<<std::endl;
  for(unsigned int i=0;i<m_hitwire.size();++i)
    {
      std::cout<<"[";
      for(unsigned int j=0;j<m_hitwire[i].size();++j)
	{
	  std::cout <<" "<< m_hitwire[i][j];
	}
      std::cout<<" ]"<<std::endl;
    }
  std::cout<<"////////// tdc ////////////"<<std::endl;
  for(unsigned int i=0;i<m_tdc.size();++i)
    {
      std::cout<<"[";
      for(unsigned int j=0;j<m_tdc[i].size();++j)
	{
	  std::cout <<" "<< m_tdc[i][j];
	}
      std::cout<<" ]"<<std::endl;
    }
  std::cout<<"///// Drift Time /////"<<std::endl;
  for(unsigned int i=0;i<m_dtime.size();++i)
    {
      std::cout<<"[ ";
      for(unsigned int j=0;j<m_dtime[i].size();++j)
	{
	  std::cout<<m_dtime[i][j]<<" ";
	}
      std::cout<<"]"<<std::endl;
    }
  std::cout<<"///// Drift Length /////"<<std::endl;
  for(unsigned int i=0;i<m_dlength.size();++i)
    {
      std::cout<<"[ ";
      for(unsigned int j=0;j<m_dlength[i].size();++j)
	{
	  std::cout<<m_dlength[i][j]<<" ";
	}
      std::cout<<"]"<<std::endl;
    }

}
//______________________________________________________________________________
void DCRHC::pushback( int DetectorID )
{
  if(DetectorID==DetIdBcOut)  pushback_BcOut();
  if(DetectorID==DetIdSdcIn)  pushback_SdcIn();
  if(DetectorID==DetIdSdcOut) pushback_SdcOut();
  if(DetectorID==DetIdSsd)    pushback_Ssd();
  return ;
}
//______________________________________________________________________________
void DCRHC::pushback_BcOut( void )
{
  static const double MinTdcBC3 = gUser.GetParameter("TdcBC3", 0);
  static const double MaxTdcBC3 = gUser.GetParameter("TdcBC3", 1);
  static const double MinTdcBC4 = gUser.GetParameter("TdcBC4", 0);
  static const double MaxTdcBC4 = gUser.GetParameter("TdcBC4", 1);

  for( int layer=0; layer<NumOfLayersBcOut; ++layer ){
      m_hitwire[layer].clear();
      m_tdc[layer].clear();
  }
  for( int layer=0; layer<NumOfLayersBC3; ++layer ){
    for( int wire=0; wire<NumOfWireBC3; ++wire ){
      int nhits = g_unpacker.get_entries( DetIdBC3, layer, 0, wire, 0 );
      if( nhits==0 ) continue;
#if TDC1st
      int tdc1st = 0;
#endif
      for( int i=0; i<nhits; ++i ){
	int tdc = g_unpacker.get(DetIdBC3,layer,0,wire,0,i);
	if( tdc<MinTdcBC3 || MaxTdcBC3<tdc ) continue;
#if TDC1st
	if( tdc1st < tdc ) tdc1st = tdc;
#else
	m_hitwire[layer].push_back(wire);
	m_tdc[layer].push_back(tdc);
#endif
      }
#if TDC1st
      if( tdc1st>0 ){
	m_hitwire[layer].push_back(wire);
	m_tdc[layer].push_back(tdc1st);
      }
#endif
    }
  }

  for( int layer=0; layer<NumOfLayersBC4; ++layer ){
    for( int wire=0; wire<NumOfWireBC4; ++wire ){
      int nhits = g_unpacker.get_entries( DetIdBC4, layer, 0, wire, 0 );
      if( nhits==0 ) continue;
#if TDC1st
      int tdc1st = 0;
#endif
      for( int i=0; i<nhits; ++i ){
	int tdc = g_unpacker.get( DetIdBC4, layer, 0, wire, 0, i );
	if( tdc<MinTdcBC4 && MaxTdcBC4<tdc ) continue;
#if TDC1st
	if( tdc1st < tdc ) tdc1st = tdc;
#else
	m_hitwire[layer + NumOfLayersBC3].push_back(wire);
	m_tdc[layer + NumOfLayersBC3].push_back(tdc);
#endif
      }
#if TDC1st
      if( tdc1st>0 ){
	m_hitwire[layer + NumOfLayersBC3].push_back(wire);
	m_tdc[layer + NumOfLayersBC3].push_back(tdc1st);
      }
#endif
    }
  }

#if DEBUG
  std::cout<<"///////Hit Wire////"<<std::endl;
  for(int i=0;i<12;++i)
    for(int j = 0;j<m_hitwire[i].size();++j)
      {
	std::cout<<"[ "<<m_hitwire[i][j]
		 <<"]"<<std::endl;
      }
#endif
}
//______________________________________________________________________________
void DCRHC::pushback_SdcIn( void )
{
  static const double MinTdcSDC1 = gUser.GetParameter("SDC1_TDC", 0);
  static const double MaxTdcSDC1 = gUser.GetParameter("SDC1_TDC", 1);

  for( int layer=0; layer<NumOfLayersSdcIn; ++layer )
    m_hitwire[layer].clear();
  for( int layer=0; layer<NumOfLayersSDC1; ++layer ){
    for( int wire=0; wire<NumOfWireSDC1; ++wire ){
      int nhits = g_unpacker.get_entries( DetIdSDC1, layer, 0, wire, 0 );
      if(nhits==0) continue;
#if TDC1st
      int tdc1st = 0;
#endif
      for( int i=0; i<nhits; ++i ){
	int tdc = g_unpacker.get( DetIdSDC1, layer, 0, wire, 2, i );
	if( tdc<MinTdcSDC1 && MaxTdcSDC1<tdc ) continue;
#if TDC1st
	if( tdc1st < tdc ) tdc1st = tdc;
#else
	m_hitwire[layer].push_back(wire);
	m_tdc[layer].push_back(tdc);
#endif
      }
#if TDC1st
      if( tdc1st>0 ){
	m_hitwire[layer].push_back(wire);
	m_tdc[layer].push_back(tdc1st);
      }
#endif
    }
  }
}
//______________________________________________________________________________
void DCRHC::pushback_SdcOut( void )
{
  static const double MinTdcSDC3 = gUser.GetParameter("TdcSDC3", 0);
  static const double MaxTdcSDC3 = gUser.GetParameter("TdcSDC3", 1);
  static const double MinTdcSDC4 = gUser.GetParameter("TdcSDC4", 0);
  static const double MaxTdcSDC4 = gUser.GetParameter("TdcSDC4", 1);
  static const double MinTdcSDC5 = gUser.GetParameter("TdcSDC5", 0);
  static const double MaxTdcSDC5 = gUser.GetParameter("TdcSDC5", 1);

  for(int layer=0;layer<NumOfLayersSdcOut;++layer) m_hitwire[layer].clear();

  for( int layer=0; layer<NumOfLayersSDC3; ++layer ){
    for( int wire=0; wire<NumOfWireSDC3; ++wire ){
      int nhits = g_unpacker.get_entries( DetIdSDC3, layer, 0, wire, 0 );
      if( nhits==0 ) continue;
      for( int i=0; i<nhits; ++i ){
	int tdc = g_unpacker.get(DetIdSDC3,layer,0,wire,0,i);
	if( tdc<MinTdcSDC3 && MaxTdcSDC3<tdc ) continue;
	m_hitwire[layer].push_back(wire);
	m_tdc[layer].push_back(tdc);
      }
    }
  }

  for( int layer=0; layer<NumOfLayersSDC4; ++layer ){
    for( int wire=0; wire<NumOfWireSDC4; ++wire ){
      int nhits = g_unpacker.get_entries( DetIdSDC4, layer, 0, wire, 0 );
      if( nhits==0 ) continue;
      for( int i=0; i<nhits; ++i ){
	int tdc = g_unpacker.get(DetIdSDC4,layer,0,wire,0,i);
	if( tdc<MinTdcSDC4 && MaxTdcSDC4<tdc ) continue;
	m_hitwire[layer+NumOfLayersSDC3].push_back(wire);
	m_tdc[layer+NumOfLayersSDC3].push_back(tdc);
      }
    }
  }

  for( int layer=0; layer<NumOfLayersSDC5; ++layer ){
    int nwire = NumOfWireSDC5X;
    if( layer == 0 || layer == 1 ) nwire = NumOfWireSDC5Y;
    for( int wire=0; wire<nwire; ++wire ){
      int nhits = g_unpacker.get_entries( DetIdSDC5, layer, 0, wire, 0 );
      if( nhits==0 ) continue;
      for( int i=0; i<nhits; ++i ){
	int tdc = g_unpacker.get(DetIdSDC5,layer,0,wire,0,i);
	if( tdc<MinTdcSDC5 && MaxTdcSDC5<tdc ) continue;
	m_hitwire[layer+NumOfLayersSDC3+NumOfLayersSDC4].push_back(wire);
	m_tdc[layer+NumOfLayersSDC3+NumOfLayersSDC4].push_back(tdc);
      }
    }
  }

  // for(int layer=0;layer<NumOfLayersSDC4;++layer)
  //   {
  //     ////// x-plane //////
  //     if(layer==1 || layer==4)
  // 	{
  // 	  for(int wire=0;wire<NumOfWireSDC3x;++wire)
  // 	    {
  // 	      int hit = g_unpacker.get_entries(DetIdSDC3,layer,0,wire,0);
  // 	      if(hit==0)continue;
  // 	      int tdc = g_unpacker.get(DetIdSDC3,layer,0,wire,0);
  // 	      m_hitwire[layer].push_back(wire);
  // 	      m_tdc[layer].push_back(tdc);
  // 	    }
  // 	}
  //     ////// u,v-plane //////
  //     else
  // 	for(int wire=0;wire<NumOfWireSDC3uv;++wire)
  // 	  {
  // 	    int hit = g_unpacker.get_entries(DetIdSDC3,layer,0,wire,0);
  // 	    if(hit==0)continue;
  // 	    int tdc = g_unpacker.get(DetIdSDC3,layer,0,wire,0);
  // 	    m_hitwire[layer].push_back(wire);
  // 	    m_tdc[layer].push_back(tdc);
  // 	  }
  //   }
  // for(int layer=0;layer<NumOfLayersSDC3;++layer)
  //   {
  //     ////// x-plane //////
  //     if(layer==1 || layer==4)
  // 	{
  // 	  for(int wire=0;wire<NumOfWireSDC3x;++wire)
  // 	    {
  // 	      int hit = g_unpacker.get_entries(DetIdSDC3,layer,0,wire,0);
  // 	      if(hit==0)continue;
  // 	      int tdc = g_unpacker.get(DetIdSDC3,layer,0,wire,0);
  // 	      m_hitwire[layer + NumOfLayersSDC3].push_back(wire);
  // 	      m_tdc[layer + NumOfLayersSDC3].push_back(tdc);
  // 	    }
  // 	}
  //     ////// u,v-plane //////
  //     else
  // 	for(int wire=0;wire<NumOfWireSDC3uv;++wire)
  // 	  {
  // 	    int hit = g_unpacker.get_entries(DetIdSDC3,layer,0,wire,0);
  // 	    if(hit==0)continue;
  // 	    int tdc = g_unpacker.get(DetIdSDC3,layer,0,wire,0);
  // 	    m_hitwire[layer + NumOfLayersSDC3].push_back(wire);
  // 	    m_tdc[layer + NumOfLayersSDC3].push_back(tdc);
  // 	  }
  //   }
}
//______________________________________________________________________________
void DCRHC::pushback_Ssd( void )
{
  for(int layer=0; layer<NumOfLayersSsd; ++layer){
    m_hitwire[layer].clear();
    m_tdc[layer].clear();
  }
  for(int layer=0; layer<NumOfLayersSSD1; ++layer){
    for(int wire=0; wire<NumOfSegSSD1; ++wire){
      int nhits = g_unpacker.get_entries(DetIdSSD1, layer, wire, 0, 1);
      if(nhits==0)continue;
      m_hitwire[layer + NumOfLayersSSD0].push_back(wire);
      m_tdc[layer + NumOfLayersSSD0].push_back(0);
    }
  }
  for(int layer=0; layer<NumOfLayersSSD2; ++layer){
    for(int wire=0; wire<NumOfSegSSD2; ++wire){
      int nhits = g_unpacker.get_entries(DetIdSSD2, layer, wire, 0, 1);
      if(nhits==0)continue;
      m_hitwire[layer + NumOfLayersSSD0 + NumOfLayersSSD1].push_back(wire);
      m_tdc[layer + NumOfLayersSSD0 + NumOfLayersSSD1].push_back(0);
    }
  }

#if DEBUG
  std::cout<<"///////Hit Wire////"<<std::endl;
  for(int i=0; i<NumOfLayersSsd; ++i){
    for(int j = 0;j<m_hitwire[i].size();++j){
      std::cout<<"[ "<<m_hitwire[i][j]
	       <<"]"<<std::endl;
    }
  }
#endif
}
//______________________________________________________________________________
void DCRHC::HitPosition( int DetectorID )
{
  std::vector<double> wire_offset, pitch, wire_center;

  int plid=0;
  if(DetectorID==DetIdBcOut)  plid = 113;
  if(DetectorID==DetIdSdcIn)  plid =   1;
  if(DetectorID==DetIdSdcOut) plid =  31;
  if(DetectorID==DetIdSsd)    plid = 141;
  for(unsigned int plane=0;plane<m_hitwire.size();++plane, ++plid)
    {
      ////////// require Multiplicity = 1 /////////////
      if(m_hitwire[plane].size()!=1) continue;
      int wireid = m_hitwire[plane][0];
      double hitposition = geomman.CalcWirePosition(plid, wireid);
      m_hitpos[plane].push_back(hitposition);
    }

#if DEBUG
  std::cout<<"///////////Hit Position/////////////"<<std::endl;
  for(unsigned int i=0;i<m_hitpos.size();++i)
    {
      std::cout<<"[ ";
      if(m_hitpos[i].size()==1)
	{
	  std::cout<<m_hitpos[i][0]<<" ";
	}
      std::cout<<"]"<<std::endl;
    }
#endif
  return;
}

//______________________________________________________________________________
void DCRHC::makeHvector( int DetectorID )
{
  if(DetectorID==DetIdSdcOut||DetectorID==DetIdSsd)
    {
      for(unsigned int i=0;i<m_hitpos.size();++i)
	{
	  if(m_hitpos[i].size()==1)
	    H.push_back(1);
	  else
	    H.push_back(0);
	}
    }
  else
    {
      for(unsigned int i=0;i<m_hitpos.size()/2;++i)
	{
	  if(m_hitpos[2*i].size()==1 && m_hitpos[2*i+1].size()==1)
	    {
	      H.push_back(1);
	      H.push_back(1);
	    }
	  else
	    {
	      H.push_back(0);
	      H.push_back(0);
	    }
	}
    }
#if 0
  std::cout<<"///////// H vector /////////"<<std::endl;
  std::cout<<"[ ";
  for(unsigned int i=0;i<m_hitwire.size();++i)
    {
      std::cout<<H[i]<<" ";
    }
  std::cout<<"]"<<std::endl;
#endif

  return;
}
//______________________________________________________________________________
void DCRHC::makeAvector( void )
{
  double a,b,c,d;
  a=b=c=d=0;
  for(unsigned int plane=0;plane<m_hitwire.size();++plane)
    {
      if(H[plane])
	{
	  a += H[plane] * m_hitpos[plane][0] * cosvector[plane];
	  b += H[plane] * m_hitpos[plane][0] * z[plane] * cosvector[plane];
	  c += H[plane] * m_hitpos[plane][0] * sinvector[plane];
	  d += H[plane] * m_hitpos[plane][0] * z[plane] * sinvector[plane];
	}
    }
  A.push_back(a);
  A.push_back(b);
  A.push_back(c);
  A.push_back(d);
#if DEBUG
  std::cout<<"///// A vector /////"<<std::endl;
  for(unsigned int i=0;i<A.size();++i)
    {
      std::cout<<A[i]<<" "<<std::endl;
    }
#endif
  return;
}
//______________________________________________________________________________
void DCRHC::makeMatrix( void )
{
  // 4*4 matrix
  // A B C D
  // B E D G
  // C D I F
  // D G F H
  double paraA,paraB,paraC,paraD,paraE,paraF,paraG,paraH,paraI;
  paraA=paraB=paraC=paraD=paraE=paraF=paraG=paraH=paraI=0;

  for(unsigned int i=0;i<m_hitwire.size();++i)
    {
      paraA += H[i]*cosvector[i]*cosvector[i];
      paraB += H[i]*cosvector[i]*cosvector[i]*z[i];
      paraC += H[i]*sinvector[i]*cosvector[i];
      paraD += H[i]*sinvector[i]*cosvector[i]*z[i];
      paraE += H[i]*cosvector[i]*cosvector[i]*z[i]*z[i];
      paraF += H[i]*sinvector[i]*sinvector[i]*z[i];
      paraG += H[i]*sinvector[i]*cosvector[i]*z[i]*z[i];
      paraH += H[i]*sinvector[i]*sinvector[i]*z[i]*z[i];
      paraI += H[i]*sinvector[i]*sinvector[i];
    }
  double trackMatrix[4][4];
  trackMatrix[0][0] = paraA;
  trackMatrix[0][1] = paraB;
  trackMatrix[0][2] = paraC;
  trackMatrix[0][3] = paraD;
  trackMatrix[1][0] = paraB;
  trackMatrix[1][1] = paraE;
  trackMatrix[1][2] = paraD;
  trackMatrix[1][3] = paraG;
  trackMatrix[2][0] = paraC;
  trackMatrix[2][1] = paraD;
  trackMatrix[2][2] = paraI;
  trackMatrix[2][3] = paraF;
  trackMatrix[3][0] = paraD;
  trackMatrix[3][1] = paraG;
  trackMatrix[3][2] = paraF;
  trackMatrix[3][3] = paraH;
#if DEBUG
  std::cout<<"///// Tracking Matrix /////"<<std::endl;
  for(int i=0;i<4;++i)
    {
      std::cout<<"[ ";
      for(int j=0;j<4;++j)
	{
	  std::cout<<std::setw(15)<<trackMatrix[i][j]<<" ";
	}
      std::cout<<"]"<<std::endl;
    }
#endif
  std::vector<std::vector<double> > matrix(4);
  //  std::vector<std::vector<double> > inv_matrix;
  for(int i=0;i<4;++i)
    for(int j=0;j<4;++j)
      {
	matrix[i].push_back(trackMatrix[i][j]);
      }

  inv_matrix = InvMatrix(matrix);
#if DEBUG
  std::cout<<"///// Inverse Tracking Matrix /////"<<std::endl;
  for(int i=0;i<4;++i)
    {
      std::cout<<"[ ";
      for(int j=0;j<4;++j)
	{
	  std::cout<<std::setw(15)<<inv_matrix[i][j]<<" ";
	}
      std::cout<<"]"<<std::endl;
    }
#endif
  return;
}
//______________________________________________________________________________
bool DCRHC::make_MinimumPoint( double& dxdZ,double& dydZ,double& x0,double& y0 )
{
  double a,b,c,d;
  a=b=c=d=0;

  makeMatrix();
  for(int i=0;i<4;++i)
    {
      a += inv_matrix[0][i] * A[i];
      b += inv_matrix[1][i] * A[i];
      c += inv_matrix[2][i] * A[i];
      d += inv_matrix[3][i] * A[i];
    }

  dXdZ = b;
  dYdZ = d;
  X0 = a;
  Y0 = c;

  dxdZ = b;
  dydZ = d;
  x0 = a;
  y0 = c;
#if DEBUG
  std::cout<<"//// Minimum Parameter ////"<<std::endl;
  std::cout<<"dXdZ = "<<std::setw(14)<<b<<std::endl;
  std::cout<<"dYdZ = "<<std::setw(14)<<d<<std::endl;
  std::cout<<"X0   = "<<std::setw(14)<<a<<std::endl;
  std::cout<<"Y0   = "<<std::setw(14)<<c<<std::endl;
#endif
  return true;
}
//______________________________________________________________________________
bool DCRHC::TrackSearch(int min_plane)
{
  pushback(detid);
  HitPosition(detid);
  makeHvector(detid);
  makeDriftTime(detid);
  makeDriftLength(detid);
  ReHitPosition(detid);
    //  hoge();
  //  getchar();
  int num_plane=0;
  for(unsigned int i=0;i<m_hitwire.size();++i)
    {
      num_plane += H[i];
    }
  if(num_plane>min_plane)
    {
      if(detid==DetIdSdcOut)
	{
	  FullTrackSearch();
	}
      makeAvector();
      double dxdZ,dydZ,x0,y0;
      make_MinimumPoint(dxdZ,dydZ,x0,y0);
      makeChisquare(detid);
      if( chi2 < 10. )
	return true;
    }

  return false;
}
//______________________________________________________________________________
double DCRHC::GetPosX( int PosZ )
{
  return dXdZ * PosZ + X0;
}
//______________________________________________________________________________
double DCRHC::GetPosY( int PosZ )
{
  return dYdZ * PosZ + Y0;
}
//______________________________________________________________________________
int DCRHC::num_Hitplane( void )
{
  int a=0;
  for(unsigned int i=0;i<m_hitwire.size();++i)
    {
      a += H[i];
    }
  return a;
}
//______________________________________________________________________________
double DCRHC::GetResidual( int plane )
{
  double residue=200;
  double axis = cosvector[plane]*(X0 + dXdZ*z[plane])+sinvector[plane]*(Y0+dYdZ*z[plane]);
  if(H[plane]==1)
    residue = m_hitpos[plane][0] - axis;
  return residue;
}
//______________________________________________________________________________
void DCRHC::makeChisquare( int DetectorID )
{
  int plid=0;
  if(DetectorID==DetIdBcOut)  plid = 113;
  if(DetectorID==DetIdSdcIn)  plid =   1;
  if(DetectorID==DetIdSdcOut) plid =  31;
  if(DetectorID==DetIdSsd)    plid = 141;
  chi2 = 0;
  int j = 0;
  for(unsigned int plane=0;plane<m_hitwire.size();++plane ,++plid)
    {
      if(H[plane]==0)continue;
      double resolution = geomman.GetResolution(plid);
      double residual   = GetResidual(plane);
      chi2 += residual*residual/resolution/resolution;
      ++j;
    }
  chi2 = chi2/(j-2);
}
//______________________________________________________________________________
void DCRHC::makeDriftTime( int DetectorID )
{
  int plid=0;
  if(DetectorID==DetIdBcOut)  plid = 113;
  if(DetectorID==DetIdSdcIn)  plid =   1;
  if(DetectorID==DetIdSdcOut) plid =  31;
  m_dtime.clear();
  m_dtime.resize(m_hitwire.size());
  for(unsigned int i=0;i<m_hitwire.size();++i,++plid)
    {
      for(unsigned int j=0;j<m_hitwire[i].size();++j)
	{
	  if(DetectorID==DetIdSsd){
	    m_dtime[i].push_back(0);
	    continue;
	  }
	  int wireid = m_hitwire[i][j];
	  //	  p0 = t0man.GetParam0(plid,wireid);
	  //	  p1 = t0man.GetParam1(plid,wireid);
	  //	  double drift_time = - (m_tdc[i][j] * p1) - p0;
	  double drift_time;
	  t0man.GetTime(plid, wireid, m_tdc[i][j], drift_time);
	  m_dtime[i].push_back(drift_time);
	}
    }
#if DEBUG
  std::cout<<"///// Drift Time /////"<<std::endl;
  for(unsigned int i=0;i<m_dtime.size();++i)
    {
      std::cout<<"[ ";
      for(unsigned int j=0;j<m_dtime[i].size();++j)
	{
	  std::cout<<m_dtime[i][j]<<" ";
	}
      std::cout<<"]"<<std::endl;
    }
#endif
}
//______________________________________________________________________________
double DCRHC::GetDriftTime( int plane )
{
  if(m_dtime[plane].size()>0)
    return m_dtime[plane][0];
  else
    return -10;
}
//______________________________________________________________________________
void DCRHC::makeDriftLength( int DetectorID )
{
  int plid;
  if(DetectorID==DetIdBcOut)  plid = 113;
  if(DetectorID==DetIdSdcIn)  plid =   1;
  if(DetectorID==DetIdSdcOut) plid =  31;
  m_dlength.clear();
  m_dlength.resize(m_hitwire.size());
  double drift_length,t;
  for(unsigned int i=0;i<m_dtime.size();++i,++plid)
    {
      for(unsigned int j=0;j<m_dtime[i].size();++j)
	{
	  if(DetectorID==DetIdSsd){
	    m_dlength[i].push_back(0);
	    continue;
	  }
	  int wireid = m_hitwire[i][j];
	  double dt;
	  t = m_dtime[i][j];
	  driftman.CalcDrift(plid, wireid, t, dt, drift_length);
	  m_dlength[i].push_back(drift_length);
	}
    }
#if DEBUG
  std::cout<<"///// Drift Length /////"<<std::endl;
  for(unsigned int i=0;i<m_dlength.size();++i)
    {
      std::cout<<"[ ";
      for(unsigned int j=0;j<m_dlength[i].size();++j)
	{
	  std::cout<<m_dlength[i][j]<<" ";
	}
      std::cout<<"]"<<std::endl;
    }
#endif
}
//______________________________________________________________________________
double DCRHC::GetDriftLength( int plane )
{
  if(m_dlength[plane].size()>0)
    return m_dlength[plane][0];
  else
    return -20;
}
//______________________________________________________________________________
std::vector<double> DCRHC::resolveLR( double left1,double right1,
				      double left2,double right2 )
{
  std::vector<double> LeftRight(2),RightLeft(2);
  LeftRight[0] = left1;
  LeftRight[1] = right2;
  RightLeft[0] = right1;
  RightLeft[1] = left2;
  double selection = fabs(left1 - right2) - fabs(right1 - left2);
  if(selection<0)
    {
      lr.push_back(-1);
      lr.push_back(1);
      return LeftRight;
    }
  else
    {
      lr.push_back(1);
      lr.push_back(-1);
      return RightLeft;
    }
}
//______________________________________________________________________________
void DCRHC::ReHitPosition( int DetectorID )
{
  if(DetectorID==DetIdSsd) return;
  if(DetectorID==DetIdBcOut || DetectorID==DetIdSdcIn)
    {
      for(unsigned int i=0;i<m_hitpos.size()/2;++i)
	{
	  if(m_hitpos[2*i].size()==0 || m_hitpos[2*i+1].size()==0)
	    {
	      lr.push_back(0);
	      lr.push_back(0);
	      continue;
	    }
	  double left1  = m_hitpos[2*i][0] - m_dlength[2*i][0];
	  double right1 = m_hitpos[2*i][0] + m_dlength[2*i][0];
	  double left2  = m_hitpos[2*i+1][0] - m_dlength[2*i+1][0];
	  double right2 = m_hitpos[2*i+1][0] + m_dlength[2*i+1][0];
	  std::vector<double> selection = resolveLR(left1,right1,left2,right2);
	  m_hitpos[2*i][0] = selection[0];
	  m_hitpos[2*i+1][0] = selection[1];
	}
    }
  else
    {
      for(unsigned int i=0;i<m_hitpos.size();++i)
	{
	  if(m_hitpos[i].size()!=1)continue;
	  double left  = m_hitpos[i][0] - m_dlength[i][0];
	  double right = m_hitpos[i][0] + m_dlength[i][0];
	  m_hitpos[i].clear();
	  m_hitpos[i].push_back(left);
	  m_hitpos[i].push_back(right);
	}
    }
#if DEBUG
  std::cout<<"/////// ReCalc Hit Position /////////"<<std::endl;
  for(unsigned int i=0;i<m_hitpos.size();++i)
    {
      std::cout<<"[ ";
      if(m_hitpos[i].size()==1 || m_hitpos[i].size()==2)
	{
	  std::cout<<m_hitpos[i][0]<<" ";
	}
      std::cout<<"]"<<std::endl;
    }
#endif
}
//______________________________________________________________________________
std::vector<int > DCRHC::GetTdc( int plane )
{
  return m_tdc[plane];
}
//______________________________________________________________________________
std::vector<int > DCRHC::GetHitWire( int plane )
{
  return m_hitwire[plane];
}
//______________________________________________________________________________
void DCRHC::FullTrackSearch( void )
{
  double buf_chi2 = -1;
  buf_chi2=10000;
  std::vector<double> bufbuf_hitpos;
#if 0
  std::cout<<"#D DCRHC::FullTrackSearch()"<<std::endl;
#endif
  for(int p12=0;p12<2;++p12){for(int p11=0;p11<2;++p11){
      for(int p10=0;p10<2;++p10){
	for(int p9=0;p9<2;++p9){for(int p8=0;p8<2;++p8){for(int p7=0;p7<2;++p7){
	      for(int p6=0;p6<2;++p6){for(int p5=0;p5<2;++p5){for(int p4=0;p4<2;++p4){
		    for(int p3=0;p3<2;++p3){for(int p2=0;p2<2;++p2){
			for(int p1=0;p1<2;++p1){
			  if(H[0]==1)
			    buf_hitpos.push_back(m_hitpos[0][p1]);
			  else
			    buf_hitpos.push_back(0);
			  if(H[1]==1)
			    buf_hitpos.push_back(m_hitpos[1][p2]);
			  else
			    buf_hitpos.push_back(0);
			  if(H[2]==1)
			    buf_hitpos.push_back(m_hitpos[2][p3]);
			  else
			    buf_hitpos.push_back(0);
			  if(H[3]==1)
			    buf_hitpos.push_back(m_hitpos[3][p4]);
			  else
			    buf_hitpos.push_back(0);
			  if(H[4]==1)
			    buf_hitpos.push_back(m_hitpos[4][p5]);
			  else
			    buf_hitpos.push_back(0);
			  if(H[5]==1)
			    buf_hitpos.push_back(m_hitpos[5][p6]);
			  else
			    buf_hitpos.push_back(0);
			  if(H[6]==1)
			    buf_hitpos.push_back(m_hitpos[6][p7]);
			  else
			    buf_hitpos.push_back(0);
			  if(H[7]==1)
			    buf_hitpos.push_back(m_hitpos[7][p8]);
			  else
			    buf_hitpos.push_back(0);
			  if(H[8]==1)
			    buf_hitpos.push_back(m_hitpos[8][p9]);
			  else
			    buf_hitpos.push_back(0);
			  if(H[9]==1)
			    buf_hitpos.push_back(m_hitpos[9][p10]);
			  else
			    buf_hitpos.push_back(0);
			  if(H[10]==1)
			    buf_hitpos.push_back(m_hitpos[10][p11]);
			  else
			    buf_hitpos.push_back(0);
			  if(H[11]==1)
			    buf_hitpos.push_back(m_hitpos[11][p12]);
			  else
			    buf_hitpos.push_back(0);
			  double a,b,c,d;
			  a=b=c=d=0;
			  for(unsigned int plane=0;plane<m_hitwire.size();++plane)
			    {
			      a += H[plane] * buf_hitpos[plane] * cosvector[plane];
			      b += H[plane] * buf_hitpos[plane] * z[plane] * cosvector[plane];
			      c += H[plane] * buf_hitpos[plane] * sinvector[plane];
			      d += H[plane] * buf_hitpos[plane] * z[plane] * sinvector[plane];
			    }

			  A.push_back(a);
			  A.push_back(b);
			  A.push_back(c);
			  A.push_back(d);
			  double dxdZ,dydZ,x0,y0;
			  make_MinimumPoint(dxdZ,dydZ,x0,y0);
			  makeChisquareSdcOut();
			  if(buf_chi2>chi2)
			    {
			      lr.clear();
			      buf_chi2 = chi2;
			      bufbuf_hitpos = buf_hitpos;
			      lr.push_back(p1);
			      lr.push_back(p2);
			      lr.push_back(p3);
			      lr.push_back(p4);
			      lr.push_back(p5);
			      lr.push_back(p6);
			      lr.push_back(p7);
			      lr.push_back(p8);
			      lr.push_back(p9);
			      lr.push_back(p10);
			      lr.push_back(p11);
			      lr.push_back(p12);
			    }
			  buf_hitpos.clear();
			  A.clear();
			}}}}}}}}}}}
  }
  // scan end
  for(int i=0;i<12;++i)
    {
      m_hitpos[i].clear();
    }
  if(bufbuf_hitpos.size()>11)
    {
      for(int i=0;i<12;++i)
	{
	  m_hitpos[i].push_back(bufbuf_hitpos[i]);
	}
    }
#if DEBUG
  std::cout<<"///////// Hit Position after search //////////"<<std::endl;

  for(unsigned int i=0;i<m_hitpos.size();++i)
    {
      std::cout<<"[ ";
      for(unsigned int j=0;j<m_hitpos[i].size();++j)
	{
	  std::cout<<m_hitpos[i][j]<<" ";
	}
      std::cout<<"]"<<std::endl;
    }

#endif
}
//______________________________________________________________________________
double DCRHC::GetResidualSdcOut( int plane )
{
  double residue=200;
  double axis = cosvector[plane]*(X0 + dXdZ*z[plane])+sinvector[plane]*(Y0+dYdZ*z[plane]);
  if(H[plane]==1)
    {
      residue = buf_hitpos[plane] - axis;
    }
  return residue;
}
//______________________________________________________________________________
void DCRHC::makeChisquareSdcOut( void )
{
  int plid =  31;
  chi2 = 0;
  int j = 0;
  for(unsigned int plane=0;plane<m_hitwire.size();++plane ,++plid)
    {
      if(H[plane]==0)continue;
      double resolution = geomman.GetResolution(plid);
      double residual   = GetResidualSdcOut(plane);
      chi2 += residual*residual/resolution/resolution;
      ++j;
    }
  chi2 = chi2/(j-2);
}
//______________________________________________________________________________
int DCRHC::Getlr( int plane )
{
  return lr[plane];
}
//______________________________________________________________________________

#include"cmath"

std::vector<std::vector<double> >
InvMatrix( std::vector<std::vector<double> > Matrix )
{
  std::vector<std::vector<double> > inv_matrix(4);
  double buf;
  //make unit matrix
  for(int i=0;i<4;++i)
    {
      for(int j=0;j<4;++j)
	{
	  if(i==j)
	    inv_matrix[i].push_back(1);
	  else
	    inv_matrix[i].push_back(0);
	}
    }

  for(int i=0;i<4;++i)
    {
      buf = 1/Matrix[i][i];
      for(int j=0;j<4;++j)
	{
	  Matrix[i][j] *= buf;
	  inv_matrix[i][j] *= buf;
	}
      for(int j=0;j<4;++j)
	{
	  if(i!=j)
	    {
	      buf=Matrix[j][i];
	      for(int k=0;k<4;++k)
		{
		  Matrix[j][k] -= Matrix[i][k]*buf;
		  inv_matrix[j][k] -= inv_matrix[i][k]*buf;
		}
	    }
	}
    }
  //output
  //   for(int i=0;i<4;++i)
  //     {
  //       std::cout<<"[ ";
  //       for(int j=0;j<4;++j)
  // 	{
  // 	  std::cout<<inv_matrix[i][j]<<" ";
  // 	}
  //       std::cout <<"]"<<std::endl;
  //     }

  return inv_matrix;

}

std::vector<double>
angle( int degree )
{
  double pi,unit,ragian;
  pi = 2.0*asin(1.0);
  unit = pi/180.0;
  //  ragian1 = 75*unit;
  ragian = degree*unit;
  //  ragian_30 = 30*unit;
  std::vector<double> tilt;

  tilt.push_back(sin(ragian));
  tilt.push_back(cos(ragian));
  tilt.push_back(tan(ragian));
  //   tilt.push_back(sin(ragian_30));
  //   tilt.push_back(cos(ragian_30));
  //   tilt.push_back(tan(ragian_30));
  return tilt;
}
