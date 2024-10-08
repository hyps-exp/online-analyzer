// -*- C++ -*-

#ifndef KURAMA_FIELD_MAP_HH
#define KURAMA_FIELD_MAP_HH

#include <string>
#include <vector>

#include <TObject.h>
#include <TString.h>

//______________________________________________________________________________
class KuramaFieldMap : public TObject
{
public:
  KuramaFieldMap( const TString& file_name );
  ~KuramaFieldMap( void );

private:
  KuramaFieldMap( const KuramaFieldMap& );
  KuramaFieldMap& operator =( const KuramaFieldMap& );

private:
  struct XYZ { Double_t x, y, z; };
  typedef std::vector< std::vector< std::vector<XYZ> > > Field;
  Bool_t      m_is_ready;
  TString     m_file_name;
  Field       B;
  Int_t       Nx, Ny, Nz;
  Double_t    X0, Y0, Z0;
  Double_t    dX, dY, dZ;

public:
  Bool_t Initialize( void );
  Bool_t IsReady( void ) const { return m_is_ready; }
  Bool_t GetFieldValue( const Double_t pointCM[3],
			Double_t *BfieldTesla ) const;

private:
  void ClearField( void );

  ClassDef(KuramaFieldMap,0);
};

#endif
