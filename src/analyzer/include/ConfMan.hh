#ifndef CONFMAN_H
#define CONFMAN_H 1

#include<string>
#include<vector>
#include<map>
#include<bitset>

class ConfMan{
  // Private parameter declarations ----------------------------------------
  // Arguments of main function
  enum ArgumentList{
    kProcess, kConfPath, kStreamPath,
    sizeArgumentList
  };
  std::string path_file_[sizeArgumentList];

  // File path list in configuration file
  std::map<std::string, std::string> name_file_;

  // Flags
  enum Flag{
    kIsGood,
    sizeFlag
  };
  std::bitset<sizeFlag> flag_;

public:
  // Public functions ------------------------------------------------------
  ~ConfMan();
  void initialize(const std::vector<std::string>& argv);
  static ConfMan&  getInstance();
  bool isGood();

  void initializeHodoParamMan();
  void initializeHodoPHCMan();

  void initializeDCGeomMan();
  void initializeDCTdcCalibMan();
  void initializeDCDriftParamMan();

  void initializeUserParamMan();

  void initializeGeAdcCalibMan();
  
private:
  ConfMan();
  ConfMan(const ConfMan& object);
  ConfMan& operator =(const ConfMan& object);
};

// GetInstance
inline ConfMan& ConfMan::getInstance()
{
  static ConfMan object;
  return object;
}

// isGood
inline bool ConfMan::isGood()
{
  return flag_[kIsGood];
}

#endif
