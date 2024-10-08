/**
 *  file: ConfMan.hh
 *  date: 2017.04.10
 *
 */

#ifndef CONF_MAN_HH
#define CONF_MAN_HH

#include <iomanip>
#include <map>
#include <bitset>
#include <string>
#include <vector>

#include <TString.h>
//#include <TObject.h>

#include <std_ostream.hh>

class VEvent;

//______________________________________________________________________________
class ConfMan// : public TObject
{
public:
  static TString ClassName( void );
  static ConfMan& GetInstance( void );
  ~ConfMan( void );

private:
  ConfMan( void );
  ConfMan( const ConfMan& );
  ConfMan& operator=(const ConfMan&);
  enum ArgumentList {
    kProcess, kConfPath, kStreamPath,
    sizeArgumentList
  };
  TString path_file_[sizeArgumentList];

  // File path list in configuration file
  std::map<TString, TString> m_key_map;
  std::map<TString, Int_t>   m_int_map;
  std::map<TString, Double_t>   m_double_map;

  // Flags
  enum Flag {
    kIsGood,
    sizeFlag
  };
  std::bitset<sizeFlag> flag_;

private:
  typedef std::map<std::string, std::string> StrList;
  typedef std::map<std::string, double>      DoubleList;
  typedef std::map<std::string, int>         IntList;
  typedef std::map<std::string, bool>        BoolList;
  typedef StrList::const_iterator    StrIterator;
  typedef DoubleList::const_iterator DoubleIterator;
  typedef IntList::const_iterator    IntIterator;
  typedef BoolList::const_iterator   BoolIterator;
  bool        m_is_ready;
  StrList     m_file;
  StrList     m_string;
  DoubleList  m_double;
  IntList     m_int;
  BoolList    m_bool;

public:
  //VEvent* EventAllocator( void );
  Bool_t  Contains( const TString& key ) const;
  template <typename T>
  static const T& Get( const std::string& key ) { return T(); }
  bool    Initialize( void );
  bool    Initialize( const std::string& file_name );
  bool    Initialize(const std::vector<std::string>& argv);
  //bool    InitializeHistograms( void );
  //bool    InitializeParameterFiles( void );
  bool    IsGood( void) const {return flag_[kIsGood];}
  bool    InitializeUnpacker( void );
  bool    IsReady( void ) const { return m_is_ready; }
  bool    Finalize( void );
  //bool    FinalizeProcess( void );
  // Initialize Parameter
  template <typename T>
  bool    InitializeParameter( void );
  template <typename T>
  bool    InitializeParameter( const std::string& key );
  template <typename T>
  bool    InitializeParameter( const std::string& key1,
			       const std::string& key2 );
  template <typename T>
  bool    InitializeParameter( const std::string& key1,
			       const std::string& key2,
                               const std::string& key3 );

private:
  std::string FilePath( const std::string& src ) const;
  bool        ShowResult( bool s, const std::string& name ) const;

  //  ClassDef(ConfMan,0);
};

//______________________________________________________________________________

inline TString
ConfMan::ClassName( void )
{
  static TString g_name("ConfMan");
  return g_name;
}

//______________________________________________________________________________
inline ConfMan&
ConfMan::GetInstance( void )
{
  static ConfMan g_instance;
  return g_instance;
}

//______________________________________________________________________________
template <>
inline const std::string&
ConfMan::Get<std::string>( const std::string& key )
{
  return GetInstance().m_string[key];
}

//______________________________________________________________________________
template <>
inline const double&
ConfMan::Get<double>( const std::string& key )
{
  return GetInstance().m_double[key];
}

//______________________________________________________________________________
template <>
inline const int&
ConfMan::Get<int>( const std::string& key )
{
  return GetInstance().m_int[key];
}

//______________________________________________________________________________
template <>
inline const bool&
ConfMan::Get<bool>( const std::string& key )
{
  return GetInstance().m_bool[key];
}

//______________________________________________________________________________
inline bool
ConfMan::ShowResult( bool s, const std::string& name ) const
{
  if( s )
    hddaq::cout << std::setw(20) << std::left
		<< " ["+name+"]"
		<< "-> Initialized" << std::endl;
  else
    hddaq::cout << std::setw(20) << std::left
		<< " ["+name+"]"
		<< "-> Failed" << std::endl;
  return s;
}

//______________________________________________________________________________
template <typename T>
inline bool
ConfMan::InitializeParameter( void )
{
  return
    ShowResult( T::GetInstance().Initialize(),
		T::ClassName() );
}

 //______________________________________________________________________________
/*
template <typename T>
inline bool
ConfMan::InitializeParameter( const std::string& key )
{
  return
    ShowResult( T::GetInstance().Initialize(m_key_map[key]),
		T::ClassName() );
}

//______________________________________________________________________________
template <typename T>
inline bool
ConfMan::InitializeParameter( const std::string& key1,
			      const std::string& key2 )
{
  return
    ShowResult( T::GetInstance().Initialize(m_key_map[key1],
					    m_key_map[key2]),
		T::ClassName() );
}

 //______________________________________________________________________________
*/
template <typename T>
inline bool
ConfMan::InitializeParameter( const std::string& key )
{
  return
    ShowResult( T::GetInstance().Initialize(m_file[key]),
		T::ClassName() );
}

//______________________________________________________________________________
template <typename T>
inline bool
ConfMan::InitializeParameter( const std::string& key1,
			      const std::string& key2 )
{
  return
    ShowResult( T::GetInstance().Initialize(m_file[key1],
					    m_file[key2]),
		std::string( T::ClassName() ) );
}

//______________________________________________________________________________
template <typename T>
inline bool
ConfMan::InitializeParameter( const std::string& key1,
			      const std::string& key2,
                              const std::string& key3 )
{
  return
    ShowResult( T::GetInstance().Initialize( m_file[key1],
                                             m_file[key2],
                                             m_file[key3] ),
		std::string( T::ClassName() ) );
}

#endif
