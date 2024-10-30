// -*- C++ -*-

#ifndef TPC_PAD_HELPER_HH
#define TPC_PAD_HELPER_HH

#include <map>
#include <TString.h>
#include <TVector3.h>

#include "DetectorID.hh"

//_____________________________________________________________________________
class TpcPadParam
{
public:
  TpcPadParam( Int_t asad, Int_t aget, Int_t channel,
               Int_t pad_id, Int_t layer_id, Int_t row_id );
  ~TpcPadParam( void );

public:
  static const Int_t AsAdMask    = 0xff;
  static const Int_t AGetMask    = 0xff;
  static const Int_t ChannelMask = 0xff;
  static const Int_t AsAdShift    =  0;
  static const Int_t AGetShift    =  8;
  static const Int_t ChannelShift = 16;
  static Int_t Key( Int_t asad, Int_t aget, Int_t channel )
    {
      return ( ( ( asad    & AsAdMask )    << AsAdShift    ) |
               ( ( aget    & AGetMask )    << AGetShift    ) |
               ( ( channel & ChannelMask ) << ChannelShift ) );
    }

private:
  Int_t m_asad;
  Int_t m_aget;
  Int_t m_channel;
  Int_t m_pad_id;
  Int_t m_layer_id;
  Int_t m_row_id;
  Int_t m_key;

public:
  Int_t AsAdId( void ) const { return m_asad; }
  Int_t AGetId( void ) const { return m_aget; }
  Int_t Channel( void ) const { return m_channel; }
  Int_t PadId( void ) const { return m_pad_id; }
  void  Print( void ) const;
  Int_t LayerId( void ) const { return m_layer_id; }
  Int_t RowId( void ) const { return m_row_id; }
};

//_____________________________________________________________________________
class TpcPadHelper
{
public:
  static std::string&  ClassName( void );
  static TpcPadHelper& GetInstance( void );
  ~TpcPadHelper( void );

private:
  TpcPadHelper( void );
  TpcPadHelper( const TpcPadHelper& );
  TpcPadHelper& operator=( const TpcPadHelper );

public:
  enum EPadParameter {
    kLayerId,
    kNumOfPad,
    kCenterRadius,
    kDivisionId,
    kRadiusId,
    kPadLength,
    NPadParameter
  };
  static const Double_t PadParameter[NumOfLayersTPC][NPadParameter];

private:
  typedef std::map<Int_t,TpcPadParam*> TpcGeomMap;
  TpcGeomMap m_map;

public:
  TpcPadParam* GetParam( Int_t asad, Int_t aget, Int_t ch ) const;
  TpcPadParam* GetParam( Int_t layer, Int_t row ) const;
  TpcPadParam* GetParam( Int_t pad ) const;
  TVector3     GetPoint( Int_t pad ) const;
  Int_t        GetPadId( Int_t asad, Int_t aget, Int_t ch ) const;
  Int_t        GetPadId( Int_t layer, Int_t row ) const;
  Int_t        GetLayerId( Int_t asad, Int_t aget, Int_t ch ) const;
  Int_t        GetRowId( Int_t asad, Int_t aget, Int_t ch ) const;
  Bool_t       Initialize( const TString& file_name );

};

//_____________________________________________________________________________
inline std::string&
TpcPadHelper::ClassName( void )
{
  static std::string s_name( "TpcPadHelper" );
  return s_name;
}

//_____________________________________________________________________________
inline TpcPadHelper&
TpcPadHelper::GetInstance( void )
{
  static TpcPadHelper s_instance;
  return s_instance;
}

#endif
