/*
  ConfMan.cc
*/

#include"ConfMan.hh"
#include"UnpackerManager.hh"
#include"Unpacker.hh"
#include"filesystem_util.hh"

#include"HodoParamMan.hh"
#include"HodoPHCMan.hh"
#include"DCGeomMan.hh"
#include"DCTdcCalibMan.hh"
#include"DCDriftParamMan.hh"
#include"UserParamMan.hh"
#include"GeAdcCalibMan.hh"

#include<iostream>
#include<fstream>
#include<sstream>
#include<vector>
#include<algorithm>
#include<iterator>

static const std::string MyName = "ConfMan::";

// Constructor -------------------------------------------------------------
ConfMan::ConfMan()
{
  flag_.reset();
}
 
// Destructor --------------------------------------------------------------
ConfMan::~ConfMan()
{

}

// initialize --------------------------------------------------------------
void ConfMan::initialize(const std::vector<std::string>& argv)
{
  using namespace hddaq;
  using namespace hddaq::unpacker;
  static const std::string MyFunc = "initialize ";

  std::cout << "#D " << MyName << MyFunc
	    << "argument list" << std::endl;
  std::copy(argv.begin(), argv.end(),
	    std::ostream_iterator<std::string>(std::cout, "\n"));
  std::cout << std::endl;

  int nArg = argv.size();
  if (sizeArgumentList > nArg)
    {
      std::cerr << "#E" << MyName << MyFunc
		<< "usage:\n"
		<< argv[kProcess]
		<< " [config file] [input stream]"
		<< std::endl;
      std::exit(1);
    }


  const std::string& confFile(argv[kConfPath]);
  const std::string& dataSrc(argv[kStreamPath]);

  std::cout << " config file = " << confFile << std::endl;
  std::string dir = dirname(confFile);
  dir += "/";
  std::cout << " dir = " << dir << std::endl;
  std::ifstream conf(confFile.c_str());
  while (conf.good())
    {
      std::string l;
      std::getline(conf, l);
      std::istringstream iss(l);
      std::istream_iterator<std::string> issBegin(iss);
      std::istream_iterator<std::string> issEnd;
      std::vector<std::string> param(issBegin, issEnd);
      if (param.empty())
	continue;
      if (param[0].empty())
	continue;
      if (param.size()==2)
	{
	  const std::string& name = param[0];
	  std::string value = param[1];
	  std::cout << " key = " << name
		    << " value = " << value << std::endl;
	  if (value.find("/")!=0)
	    value = realpath(dir + value);
	  name_file_[name] = value;
	}
    }

  // initialize unpacker system 
  UnpackerManager& g_unpacker = GUnpacker::get_instance();
  g_unpacker.set_config_file(name_file_["UNPACKER:"],
			     name_file_["DIGIT:"],
			     name_file_["CMAP:"]);
  g_unpacker.set_istream(dataSrc);

  // Initialize of ConfMan and Unpacker were done
  flag_.set(kIsGood);
}

// initialize HodoParamMan --------------------------------------------------
void ConfMan::initializeHodoParamMan()
{
  if(name_file_["HDPRM:"] != ""){
    HodoParamMan& gHodoParam = HodoParamMan::GetInstance();
    gHodoParam.SetFileName(name_file_["HDPRM:"]);
    flag_[kIsGood] = gHodoParam.Initialize();
  }else{
    std::cout << "#E " << MyName
	      << " File path does not exist in " << name_file_["HDPRM:"] 
	      << std::endl;
    flag_.reset(kIsGood);
  }
}

// initialize HodoPHCMan ----------------------------------------------------
void ConfMan::initializeHodoPHCMan()
{
  if(name_file_["HDPHC:"] != ""){
    HodoPHCMan& gHodoPHC = HodoPHCMan::GetInstance();
    gHodoPHC.SetFileName(name_file_["HDPHC:"]);
    flag_[kIsGood] = gHodoPHC.Initialize();
  }else{
    std::cout << "#E " << MyName
	      << " File path does not exist in " << name_file_["HDPHC:"] 
	      << std::endl;
    flag_.reset(kIsGood);
  }
}

// initialize DCGeomMan -----------------------------------------------------
void ConfMan::initializeDCGeomMan()
{
  if(name_file_["DCGEOM:"] != ""){
    DCGeomMan& gDCGeo = DCGeomMan::GetInstance();
    flag_[kIsGood] = gDCGeo.Initialize(name_file_["DCGEOM:"]);
  }else{
    std::cout << "#E " << MyName
	      << " File path does not exist in " << name_file_["DCGEOM:"] 
	      << std::endl;
    flag_.reset(kIsGood);
  }
}

// initialize DCTdcCalibMan
void ConfMan::initializeDCTdcCalibMan()
{
  if(name_file_["TDCCALIB:"] != ""){
    DCTdcCalibMan& gDCTdcCalib = DCTdcCalibMan::GetInstance();
    gDCTdcCalib.SetFileName(name_file_["TDCCALIB:"]);
    flag_[kIsGood] = gDCTdcCalib.Initialize();
  }else{
    std::cout << "#E " << MyName
	      << " File path does not exist in " << name_file_["TDCCALIB:"] 
	      << std::endl;
    flag_.reset(kIsGood);
  }
}

// initialize DCDriftMan
void ConfMan::initializeDCDriftParamMan()
{
  if(name_file_["DRFTPM:"] != ""){
    DCDriftParamMan& gDCDriftParam = DCDriftParamMan::GetInstance();
    gDCDriftParam.SetFileName(name_file_["DRFTPM:"]);
    flag_[kIsGood] = gDCDriftParam.Initialize();
  }else{
    std::cout << "#E " << MyName
	      << " File path does not exist in " << name_file_["DRFTPM:"] 
	      << std::endl;
    flag_.reset(kIsGood);
  }
}

// initialize UserParamMan
void ConfMan::initializeUserParamMan()
{
  if(name_file_["USER:"] != ""){
    UserParamMan& gUserParam = UserParamMan::getInstance();
    flag_[kIsGood] = gUserParam.initialize(name_file_["USER:"]);
  }else{
    std::cout << "#E " << MyName
	      << " File path does not exist in " << name_file_["USER:"] 
	      << std::endl;
    flag_.reset(kIsGood);
  }
}

// initialize GeAdcCalibMan
void ConfMan::initializeGeAdcCalibMan()
{
  if(name_file_["GEADC:"] != ""){
    GeAdcCalibMan& gGeAdcCalib = GeAdcCalibMan::GetInstance();
    flag_[kIsGood] = gGeAdcCalib.Initialize(name_file_["GEADC:"]);
  }else{
    std::cout << "#E " << MyName
	      << " File path does not exist in " << name_file_["GEADC:"] 
	      << std::endl;
    flag_.reset(kIsGood);
  }
}

