#include<iostream>
#include<cstdlib>
#include<string>
#include<algorithm>

#include"DetectorID.hh"
#include"HistHelper.hh"
#include"HistMaker.hh"

#include<TH1.h>
#include<TH2.h>
#include<TList.h>
#include<TDirectory.h>
#include<TString.h>

ClassImp(HistMaker)

// getStr_FromEnum ----------------------------------------------------------
// The method to get std::string from enum value 
#define CONV_STRING(x) getStr_FromEnum(#x)
std::string getStr_FromEnum(const char* c){
  std::string str = c;
  return str.substr(1);
}

static const std::string MyName = "HistMaker::";

// Constructor -------------------------------------------------------------
HistMaker::HistMaker():
  current_hist_id_(0)
{

}

// Destructor --------------------------------------------------------------
HistMaker::~HistMaker()
{

}

// -------------------------------------------------------------------------
// getListOfDetectors
// -------------------------------------------------------------------------
void HistMaker::getListOfDetectors(std::vector<std::string>& vec)
{
  HistMaker& g = HistMaker::getInstance();
  std::copy(g.name_created_detectors_.begin(),
	    g.name_created_detectors_.end(),
	    back_inserter(vec)
	    );
  
}

// -------------------------------------------------------------------------
// getListOfPsFiles
// -------------------------------------------------------------------------
void HistMaker::getListOfPsFiles(std::vector<std::string>& vec)
{
  HistMaker& g = HistMaker::getInstance();
  std::copy(g.name_ps_files_.begin(),
	    g.name_ps_files_.end(),
	    back_inserter(vec)
	    );
  
}

// -------------------------------------------------------------------------
// setHistPtr
// -------------------------------------------------------------------------
int HistMaker::setHistPtr(std::vector<TH1*>& vec)
{
  static const std::string MyFunc = "setHistPtr ";
  
  vec.resize(current_hist_id_);
  for(int i = 0; i<current_hist_id_; ++i){
    int unique_id = getUniqueID(i);
    vec[i] = GHist::get(unique_id);
    if(vec[i] == NULL){
      std::cerr << "#E: " << MyName << MyFunc
		<< "Pointer is NULL\n"
		<< " Unique ID    : " << unique_id << "\n"
		<< " Sequential ID: " << i << std::endl;
      gDirectory->ls();
      return -1;
    }
  }

  return 0;
}

// -------------------------------------------------------------------------
// CreateTH1 
// -------------------------------------------------------------------------
TH1* HistMaker::createTH1(int unique_id, const char* title,
			  int nbinx, int xmin, int xmax,
			  const char* xtitle, const char* ytitle
			  )
{
  static const std::string MyFunc = "createTH1 ";

  // Add information to dictionaries which will be used to find the histogram
  int sequential_id = current_hist_id_++;
  idmap_seq_from_unique_[unique_id]     = sequential_id;
  idmap_seq_from_name_[title]           = sequential_id;
  idmap_unique_from_seq_[sequential_id] = unique_id;

  // create histogram using the static method of HistHelper class
  TH1 *h = GHist::I1(unique_id, title, nbinx, xmin, xmax);
  if(!h){
    std::cerr << "#E: " << MyName << MyFunc
	      << "Fail to create TH1"
	      << std::endl;
    std::cerr << " " << unique_id << " " << title << std::endl;
    std::exit(-1);
    //    return h;
  }
  
  h->GetXaxis()->SetTitle(xtitle);
  h->GetYaxis()->SetTitle(ytitle);
  return h;
}

// -------------------------------------------------------------------------
// CreateTH2 
// -------------------------------------------------------------------------
TH2* HistMaker::createTH2(int unique_id, const char* title,
			  int nbinx, int xmin, int xmax,
			  int nbiny, int ymin, int ymax,
			  const char* xtitle, const char* ytitle
			  )
{
  static const std::string MyFunc = "createTH2 ";

  // Add information to dictionaries which will be used to find the histogram
  int sequential_id = current_hist_id_++;
  idmap_seq_from_unique_[unique_id]     = sequential_id;
  idmap_seq_from_name_[title]           = sequential_id;
  idmap_unique_from_seq_[sequential_id] = unique_id;

  // create histogram using the static method of HistHelper class
  TH2 *h = GHist::I2(unique_id, title,
		  nbinx, xmin, xmax,
		  nbiny, ymin, ymax);
  if(!h){
    std::cerr << "#E: " << MyName << MyFunc
	      << "Fail to create TH2"
	      << std::endl;
    std::cerr << " " << unique_id << " " << title << std::endl;
    std::exit(-1);
    //    return h;
  }
  
  h->GetXaxis()->SetTitle(xtitle);
  h->GetYaxis()->SetTitle(ytitle);
  return h;
}

// -------------------------------------------------------------------------
// createBH1 
// -------------------------------------------------------------------------
TList* HistMaker::createBH1(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kBH1);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // Make unique ID
    int target_id = getUniqueID(kBH1, 0, kADC, 0);
    for(int i = 0; i<NumOfSegBH1*2; ++i){
      const char* title = NULL;
      if(i < NumOfSegBH1){
	int seg = i+1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	int seg = i+1-NumOfSegBH1; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }

      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kBH1, 0, kTDC, 0);
    for(int i = 0; i<NumOfSegBH1*2; ++i){
      const char* title = NULL;
      if(i < NumOfSegBH1){
	int seg = i+1; // 1 origin 
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	int seg = i+1-NumOfSegBH1; // 1 origin 
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }

      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    const char* title = "BH1_hit_pattern";
    int target_id = getUniqueID(kBH1, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegBH1, 0, NumOfSegBH1,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "BH1_multiplicity";
    int target_id = getUniqueID(kBH1, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegBH1, 0, NumOfSegBH1,
			   "Multiplicity", ""));
  }
  
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createBFT
// -------------------------------------------------------------------------
TList* HistMaker::createBFT(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kBFT);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // TDC---------------------------------------------------------
  {
    int target_id = getUniqueID(kBFT, 0, kTDC, 0);
    // Add to the top directory
    top_dir->Add(createTH1(++target_id, "BFT_TDC_U", // 1 origin
			   1024, 0, 1024,
			   "TDC [ch]", ""));

    top_dir->Add(createTH1(++target_id, "BFT_TDC_D", // 1 origin
			   1024, 0, 1024,
			   "TDC [ch]", ""));
  }

  // Hit parttern -----------------------------------------------
  {
    int target_id = getUniqueID(kBFT, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(++target_id, "BFT_HitPat_U", // 1 origin
			   NumOfSegBFT, 0, NumOfSegBFT,
			   "Segment", ""));

    top_dir->Add(createTH1(++target_id, "BFT_HitPat_D", // 1 origin
			   NumOfSegBFT, 0, NumOfSegBFT,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "BFT_multiplicity";
    int target_id = getUniqueID(kBFT, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(++target_id, title, // 1 origin
			   30, 0, 30,
			   "Multiplicity", ""));
  }
  
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createBC3
// -------------------------------------------------------------------------
TList* HistMaker::createBC3(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kBC3);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  const char* name_layer[] = {"x0", "x1", "v0", "v1", "u0", "u1"};

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kBC3, 0, kTDC, 0);
    for(int i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1024, 0, 1024,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kBC3, 0, kHitPat, 0);
    for(int i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     NumOfWireBC3, 0, NumOfWireBC3,
			     "wire", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    int target_id = getUniqueID(kBC3, 0, kMulti, 0);
    for(int i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     10, 0, 10,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kBC3, 0, kMulti, NumOfLayersBC3);
    for(int i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     10, 0, 10,
			     "Multiplicity", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }
  
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createBC4
// -------------------------------------------------------------------------
TList* HistMaker::createBC4(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kBC4);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  const char* name_layer[] = {"u0", "u1", "v0", "v1", "x0", "x1"};

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kBC4, 0, kTDC, 0);
    for(int i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1024, 0, 1024,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kBC4, 0, kHitPat, 0);
    for(int i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     NumOfWireBC4, 0, NumOfWireBC4,
			     "wire", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    int target_id = getUniqueID(kBC4, 0, kMulti, 0);
    for(int i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     10, 0, 10,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kBC4, 0, kMulti, NumOfLayersBC4);
    for(int i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     10, 0, 10,
			     "Multiplicity", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }
  
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createBMW
// -------------------------------------------------------------------------
TList* HistMaker::createBMW(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kBMW);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // ADC---------------------------------------------------------
  {
    int target_id = getUniqueID(kBMW, 0, kADC, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, "BMW_ADC", // 1 origin
			   0x1000, 0, 0x1000,
			   "ADC [ch]", ""));
  }

  // TDC---------------------------------------------------------
  {
    int target_id = getUniqueID(kBMW, 0, kTDC, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, "BMW_TDC", // 1 origin
			   0x1000, 0, 0x1000,
			   "TDC [ch]", ""));
  }
  
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createBH2 
// -------------------------------------------------------------------------
TList* HistMaker::createBH2(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kBH2);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kBH2, 0, kADC, 0);
    for(int i = 0; i<NumOfSegBH2; ++i){
      const char* title = NULL;
      int seg = i+1; // 1 origin
      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kBH2, 0, kTDC, 0);
    for(int i = 0; i<NumOfSegBH2; ++i){
      const char* title = NULL;
      int seg = i+1; // 1 origin
      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    const char* title = "BH2_hit_pattern";
    int target_id = getUniqueID(kBH2, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegBH2, 0, NumOfSegBH2,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "BH2_multiplicity";
    int target_id = getUniqueID(kBH2, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegBH2, 0, NumOfSegBH2,
			   "Multiplicity", ""));
  }
  
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createBAC_SAC
// -------------------------------------------------------------------------
TList* HistMaker::createBAC_SAC(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kBAC_SAC);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // ACs configuration
  const char* name_acs[] = {"BAC0a", "BAC0b", "BAC1", "BAC2", "SAC1", "SAC2"};

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kBAC_SAC, 0, kADC, 0);
    for(int i = 0; i<6; ++i){
      const char* title = NULL;
      title = Form("%s_%s", name_acs[i], nameSubDir);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kBAC_SAC, 0, kTDC, 0);
    for(int i = 0; i<6; ++i){
      const char* title = NULL;
      title = Form("%s_%s", name_acs[i], nameSubDir);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }
  
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createSDC2
// -------------------------------------------------------------------------
TList* HistMaker::createSDC2(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kSDC2);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  const char* name_layer[] = {"x0", "x1", "v0", "v1", "u0", "u1"};

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kSDC2, 0, kTDC, 0);
    for(int i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1024, 0, 1024,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kSDC2, 0, kHitPat, 0);
    for(int i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     NumOfWireSDC2, 0, NumOfWireSDC2,
			     "wire", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    int target_id = getUniqueID(kSDC2, 0, kMulti, 0);
    for(int i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     10, 0, 10,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC2, 0, kMulti, NumOfLayersSDC2);
    for(int i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     10, 0, 10,
			     "Multiplicity", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }
  
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createHDC
// -------------------------------------------------------------------------
TList* HistMaker::createHDC(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kHDC);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  const char* name_layer[] = {"u0", "u1", "x0", "x1"};

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kHDC, 0, kTDC, 0);
    for(int i = 0; i<NumOfLayersHDC; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1024, 0, 1024,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kHDC, 0, kHitPat, 0);
    for(int i = 0; i<NumOfLayersHDC; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     NumOfWireHDC, 0, NumOfWireHDC,
			     "wire", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    int target_id = getUniqueID(kHDC, 0, kMulti, 0);
    for(int i = 0; i<NumOfLayersHDC; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     10, 0, 10,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kHDC, 0, kMulti, NumOfLayersHDC);
    for(int i = 0; i<NumOfLayersHDC; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     10, 0, 10,
			     "Multiplicity", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }
  
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createSP0
// -------------------------------------------------------------------------
TList* HistMaker::createSP0(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kSP0);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // Declaration of sub-detector directory
  for(int sd = 0; sd<NumOfLayersSP0; ++sd){
    const char* nameSubDetector = Form("SP0_L%d", sd+1);
    TList *subdet_dir = new TList;
    subdet_dir->SetName(nameSubDetector);

    // ADC---------------------------------------------------------
    {
      // Declaration of the sub-directory
      std::string strSubDir  = CONV_STRING(kADC);
      const char* nameSubDir = strSubDir.c_str();
      TList *sub_dir = new TList;
      sub_dir->SetName(nameSubDir);

      // Make histogram and add it
      int target_id = getUniqueID(kSP0, kSP0_L1+sd, kADC, 0);
      for(int i = 0; i<NumOfSegSP0*2; ++i){
	const char* title = NULL;
	if(i < NumOfSegSP0){
	  int seg = i+1; // 1 origin
	  title = Form("%s_%s_%dU", nameSubDetector, nameSubDir, seg);
	}else{
	  int seg = i+1-NumOfSegSP0; // 1 origin
	  title = Form("%s_%s_%dD", nameSubDetector, nameSubDir, seg);
	}

	sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			       0x1000, 0, 0x1000,
			       "ADC [ch]", ""));
      }

      // insert sub directory
      subdet_dir->Add(sub_dir);
    }

    // TDC---------------------------------------------------------
    {
      // Declaration of the sub-directory
      std::string strSubDir  = CONV_STRING(kTDC);
      const char* nameSubDir = strSubDir.c_str();
      TList *sub_dir = new TList;
      sub_dir->SetName(nameSubDir);

      // Make histogram and add it
      int target_id = getUniqueID(kSP0, kSP0_L1+sd, kTDC, 0);
      for(int i = 0; i<NumOfSegSP0*2; ++i){
	const char* title = NULL;
	if(i < NumOfSegSP0){
	  int seg = i+1; // 1 origin 
	  title = Form("%s_%s_%dU", nameSubDetector, nameSubDir, seg);
	}else{
	  int seg = i+1-NumOfSegSP0; // 1 origin 
	  title = Form("%s_%s_%dD", nameSubDetector, nameSubDir, seg);
	}

	sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			       0x1000, 0, 0x1000,
			       "TDC [ch]", ""));
      }

      // insert sub directory
      subdet_dir->Add(sub_dir);
    }

    // HitPat-------------------------------------------------------
    {
      // Make histogram and add it
      int target_id = getUniqueID(kSP0, kSP0_L1+sd, kHitPat, 0);
      for(int i = 0; i<2; ++i){
	const char* title = NULL;
	if(i == 0){
	  title = Form("%s_HitPat_%dU", nameSubDetector, sd);
	}else{
	  title = Form("%s_HitPat_%dD", nameSubDetector, sd);
	}

	subdet_dir->Add(createTH1(target_id + i+1, title, // 1 origin
				  NumOfSegSP0, 0, NumOfSegSP0,
				  "Segment", ""));
      }
    }

    top_dir->Add(subdet_dir);
  }
  
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createSDC3
// -------------------------------------------------------------------------
TList* HistMaker::createSDC3(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kSDC3);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  const char* name_layer[] = {"v0", "x0", "u0", "v1", "x1", "u1"};

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kSDC3, 0, kTDC, 0);
    for(int i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1600, 0, 1600,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kSDC3, 0, kHitPat, 0);
    for(int i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     NumOfWireSDC3uv, 0, NumOfWireSDC3uv,
			     "wire", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    int target_id = getUniqueID(kSDC3, 0, kMulti, 0);
    for(int i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     10, 0, 10,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC3, 0, kMulti, NumOfLayersSDC3);
    for(int i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     10, 0, 10,
			     "Multiplicity", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }
  
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createSDC4
// -------------------------------------------------------------------------
TList* HistMaker::createSDC4(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kSDC4);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  const char* name_layer[] = {"v0", "x0", "u0", "v1", "x1", "u1"};

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kSDC4, 0, kTDC, 0);
    for(int i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1600, 0, 1600,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kSDC4, 0, kHitPat, 0);
    for(int i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     NumOfWireSDC4uv, 0, NumOfWireSDC4uv,
			     "wire", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    int target_id = getUniqueID(kSDC4, 0, kMulti, 0);
    for(int i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     10, 0, 10,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC4, 0, kMulti, NumOfLayersSDC4);
    for(int i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     10, 0, 10,
			     "Multiplicity", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }
  
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createTOF
// -------------------------------------------------------------------------
TList* HistMaker::createTOF(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kTOF);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kTOF, 0, kADC, 0);
    for(int i = 0; i<NumOfSegTOF*2; ++i){
      const char* title = NULL;
      if(i < NumOfSegTOF){
	int seg = i+1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	int seg = i+1-NumOfSegTOF; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }

      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kTOF, 0, kTDC, 0);
    for(int i = 0; i<NumOfSegTOF*2; ++i){
      const char* title = NULL;
      if(i < NumOfSegTOF){
	int seg = i+1; // 1 origin 
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	int seg = i+1-NumOfSegTOF; // 1 origin 
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }

      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    const char* title = "TOF_hit_pattern";
    int target_id = getUniqueID(kTOF, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegTOF, 0, NumOfSegTOF,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "TOF_multiplicity";
    int target_id = getUniqueID(kTOF, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegTOF, 0, NumOfSegTOF,
			   "Multiplicity", ""));
  }
  
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createTOFMT
// -------------------------------------------------------------------------
TList* HistMaker::createTOFMT(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kTOFMT);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kTOFMT, 0, kTDC, 0);
    for(int i = 0; i<NumOfSegTOF; ++i){
      const char* title = NULL;
      int seg = i+1; // 1 origin 
      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    const char* title = "TOFMT_hit_pattern";
    int target_id = getUniqueID(kTOFMT, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegTOF, 0, NumOfSegTOF,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "TOFMT_multiplicity";
    int target_id = getUniqueID(kTOFMT, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegTOF, 0, NumOfSegTOF,
			   "Multiplicity", ""));
  }
  
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createSFV_SAC3
// -------------------------------------------------------------------------
TList* HistMaker::createSFV_SAC3(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kSFV_SAC3);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  const int NofLoop = 7;
  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kSFV_SAC3, 0, kADC, 0);
    for(int i = 0; i<NofLoop; ++i){
      const char* title = NULL;
      if(i<NofLoop-1){
	int seg = i+1; // 1 origin
	title = Form("%s_%s_%d", "SFV", nameSubDir, seg);
      }else{
	title = Form("%s_%s", "SAC3", nameSubDir);
      }
      
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kSFV_SAC3, 0, kTDC, 0);
    for(int i = 0; i<NofLoop; ++i){
      const char* title = NULL;
      if(i<NofLoop-1){
	int seg = i+1; // 1 origin
	title = Form("%s_%s_%d", "SFV", nameSubDir, seg);
      }else{
	title = Form("%s_%s", "SAC3", nameSubDir);
      }
      
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    const char* title = "SFV_SAC3_hit_pattern";
    int target_id = getUniqueID(kSFV_SAC3, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NofLoop, 0, NofLoop,
			   "Segment", ""));
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createLC
// -------------------------------------------------------------------------
TList* HistMaker::createLC(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kLC);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kLC, 0, kADC, 0);
    for(int i = 0; i<NumOfSegLC*2; ++i){
      const char* title = NULL;
      if(i < NumOfSegLC){
	int seg = i+1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	int seg = i+1-NumOfSegLC; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }

      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kLC, 0, kTDC, 0);
    for(int i = 0; i<NumOfSegLC*2; ++i){
      const char* title = NULL;
      if(i < NumOfSegLC){
	int seg = i+1; // 1 origin 
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	int seg = i+1-NumOfSegLC; // 1 origin 
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }

      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    const char* title = "LC_hit_pattern";
    int target_id = getUniqueID(kLC, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegLC, 0, NumOfSegLC,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "LC_multiplicity";
    int target_id = getUniqueID(kLC, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegLC, 0, NumOfSegLC,
			   "Multiplicity", ""));
  }
  
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createGe
// -------------------------------------------------------------------------
TList* HistMaker::createGe(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kGe);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kGe, 0, kADC, 0);
    for(int i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      int seg = i+1; // 1 origin
      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x2000, 0, 0x2000,
			     "ADC [ch]", ""));
    }

    // Sum histogram
    sub_dir->Add(createTH1(++target_id + NumOfSegGe, "Ge_ADC_Sum", // 1 origin
			   0x2000, 0, 0x2000,
			   "ADC [ch]", ""));

    // 2D histogram
    target_id = getUniqueID(kGe, 0, kADC2D, 0);
    sub_dir->Add(createTH2(++target_id, "Ge_ADC_2D", // 1 origin
			   NumOfSegGe, 0, NumOfSegGe,
			   0x2000, 0, 0x2000,
			   "Ge segment", "ADC [ch]"));

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // CRM---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kCRM);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kGe, 0, kCRM, 0);
    for(int i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      int seg = i+1; // 1 origin 
      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     2000, 0, 10000,
			     "CRM [ch]", ""));
    }

    // 2D histogram
    target_id = getUniqueID(kGe, 0, kCRM2D, 0);
    sub_dir->Add(createTH2(++target_id, "Ge_CRM_2D", // 1 origin
			   NumOfSegGe, 0, NumOfSegGe,
			   2000, 0, 10000,
			   "Ge segment", "CRM [ch]"));

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TFA---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kTFA);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kGe, 0, kTFA, 0);
    for(int i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      int seg = i+1; // 1 origin 
      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     2000, 0, 10000,
			     "TFA [ch]", ""));
    }

    // 2D histogram
    target_id = getUniqueID(kGe, 0, kTFA2D, 0);
    sub_dir->Add(createTH2(++target_id, "Ge_TFA_2D", // 1 origin
			   NumOfSegGe, 0, NumOfSegGe,
			   2000, 0, 10000,
			   "Ge segment", "TFA [ch]"));

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // PUR---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kPUR);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kGe, 0, kPUR, 0);
    for(int i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      int seg = i+1; // 1 origin 
      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     2000, 0, 10000,
			     "PUR [ch]", ""));
    }

    // 2D histogram
    target_id = getUniqueID(kGe, 0, kPUR2D, 0);
    sub_dir->Add(createTH2(++target_id, "Ge_PUR_2D", // 1 origin
			   NumOfSegGe, 0, NumOfSegGe,
			   2000, 0, 10000,
			   "Ge segment", "PUR [ch]"));

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // RST---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kRST);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    int target_id = getUniqueID(kGe, 0, kRST, 0);
    for(int i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      int seg = i+1; // 1 origin 
      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     100, 0, 010000,
			     "RST [ch]", ""));
    }

    // 2D histogram
    target_id = getUniqueID(kGe, 0, kRST2D, 0);
    sub_dir->Add(createTH2(++target_id, "Ge_RST_2D", // 1 origin
			   NumOfSegGe, 0, NumOfSegGe,
			   100, 0, 10000,
			   "Ge segment", "RST [ch]"));

    // insert sub directory
    top_dir->Add(sub_dir);
  }
  
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createPWO
// -------------------------------------------------------------------------
TList* HistMaker::createPWO(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kPWO);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    int target_id = getUniqueID(kPWO, 0, kADC2D, 0);
    for(int i = 0; i<NumOfBoxPWO; ++i){
      const char* title = NULL;
      int box = i+1; // 1 origin 
      title = Form("%s_%s2d_Box%d", nameDetector, nameSubDir, box);
      sub_dir->Add(createTH2(target_id + i+1, title, // 1 origin
			     NumOfUnitPWO[i], 0, NumOfUnitPWO[i],
			     0x1000, 0, 0x1000,
			     "PWO segment", "ADC [ch]"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    
    // sum hist
    int target_id = getUniqueID(kPWO, 0, kTDC, 0);
    for(int i = 0; i<NumOfBoxPWO; ++i){
      const char* title = NULL;
      int box = i+1; // 1 origin 
      title = Form("%s_%ssum_Box%d", nameDetector, nameSubDir, box);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
  			     0x1000, 0, 0x1000,
  			     "TDC [ch]", ""));
    }

    target_id = getUniqueID(kPWO, 0, kTDC2D, 0);
    for(int i = 0; i<NumOfBoxPWO; ++i){
      const char* title = NULL;
      int box = i+1; // 1 origin 
      title = Form("%s_%s2d_Box%d", nameDetector, nameSubDir, box);
      sub_dir->Add(createTH2(target_id + i+1, title, // 1 origin
  			     NumOfUnitPWO[i], 0, NumOfUnitPWO[i],
  			     0x1000, 0, 0x1000,
  			     "PWO segment", "TDC [ch]"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat---------------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    
    int target_id = getUniqueID(kPWO, 0, kHitPat, 0);
    for(int i = 0; i<NumOfBoxPWO; ++i){
      const char* title = NULL;
      int box = i+1; // 1 origin 
      title = Form("%s_%s_Box%d", nameDetector, nameSubDir, box);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
  			     NumOfUnitPWO[i], 0, NumOfUnitPWO[i],
  			     "PWO segment", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity---------------------------------------------------
  {
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    
    int target_id = getUniqueID(kPWO, 0, kMulti, 0);
    for(int i = 0; i<NumOfBoxPWO; ++i){
      const char* title = NULL;
      int box = i+1; // 1 origin 
      title = Form("%s_%s_Box%d", nameDetector, nameSubDir, box);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
  			     NumOfUnitPWO[i], 0, NumOfUnitPWO[i],
  			     "Multiplicity", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }
  
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createTriggerFlag
// -------------------------------------------------------------------------
TList* HistMaker::createTriggerFlag(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kTriggerFlag);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }
  
  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // TDC---------------------------------------------------------
  {
    // Make histogram and add it
    int target_id = getUniqueID(kTriggerFlag, 0, kTDC, 0);
    for(int i = 0; i<NumOfSegMisc; ++i){
      const char* title = NULL;
      title = Form("%s_%d", nameDetector, i+1);
      top_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     400, 0, 4000,
			     "TDC [ch]", ""));
    }
  }

  // Hit parttern -----------------------------------------------
  {
    const char* title = "Trigger_Entry";
    int target_id = getUniqueID(kTriggerFlag, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   20, 0, 20,
			   "Trigger flag", ""));
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createCorrelation
// -------------------------------------------------------------------------
TList* HistMaker::createCorrelation(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kCorrelation);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }

  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str(); 
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);  

  {
    int target_id = getUniqueID(kCorrelation, 0, 0, 0);
    // BH2 vs BH1 -----------------------------------------------
    top_dir->Add(createTH2(++target_id, "BH2_BH1", // 1 origin
			   NumOfSegBH1, 0, NumOfSegBH1,
			   NumOfSegBH2, 0, NumOfSegBH2,
			   "BH1 seg", "BH2 seg"));

    // LC vs TOF -----------------------------------------------
    top_dir->Add(createTH2(++target_id, "LC_TOF", // 1 origin
			   NumOfSegTOF, 0, NumOfSegTOF,
			   NumOfSegLC,  0, NumOfSegLC,
			   "TOF seg", "LC seg"));

    // BC4 vs BC3 ----------------------------------------------
    top_dir->Add(createTH2(++target_id, "BC4x1_BC3x0", // 1 origin
			   NumOfWireBC3, 0, NumOfWireBC3,
			   NumOfWireBC4, 0, NumOfWireBC4,
			   "BC3 wire", "BC4 wire"));

    // SDC2 vs SDC1 --------------------------------------------
    top_dir->Add(createTH2(++target_id, "HDCx1_SDC2x0", // 1 origin
			   NumOfWireSDC2, 0, NumOfWireSDC2,
			   NumOfWireHDC,  0, NumOfWireHDC,
			   "SDC2 wire", "HDC wire"));
  }

  return top_dir;
}

// -------------------------------------------------------------------------
// createDAQ
// -------------------------------------------------------------------------
TList* HistMaker::createDAQ(bool flag_ps)
{
  // Determine the detector name
  std::string strDet = CONV_STRING(kDAQ);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet); 
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet); 
  }

  // Declaration of the directory
  // Just type conversion from std::string to char*
  const char* nameDetector = strDet.c_str(); 
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);    


  // DAQ infomation --------------------------------------------------
  {
    // Event builder infomation
    int target_id = getUniqueID(kDAQ, kEB, kHitPat, 0);
    top_dir->Add(createTH1(target_id + 1, "Data size EB", // 1 origin
			   5000, 0, 5000,
			   "Data size [words]", ""));

    // Node information
    target_id = getUniqueID(kDAQ, kVME, kHitPat2D, 0);
    top_dir->Add(createTH2(target_id + 1, "Data size VME nodes", // 1 origin
			   7, 0, 7,
			   500, 0, 1000,
			   "VME node ID", "Data size [words]"));

    target_id = getUniqueID(kDAQ, kCopper, kHitPat2D, 0);
    top_dir->Add(createTH2(target_id + 1, "Data size Copper nodes", // 1 origin
			   15, 0, 15,
			   100, 0, 200,
			   "Copper node ID", "Data size [words]"));

    target_id = getUniqueID(kDAQ, kEASIROC, kHitPat2D, 0);
    top_dir->Add(createTH2(target_id + 1, "Data size EASIROC nodes", // 1 origin
			   11, 0, 11,
			   50, 0, 100,
			   "EASIROC node ID", "Data size [words]"));

    target_id = getUniqueID(kDAQ, kCAMAC, kHitPat2D, 0);
    top_dir->Add(createTH2(target_id + 1, "Data size CAMAC nodes", // 1 origin
			   3, 0, 3,
			   50, 0, 100,
			   "CAMAC node ID", "Data size [words]"));
  }
  
  {
    // TKO box information
    // Declaration of the sub-directory
    std::string strSubDir  = CONV_STRING(kTKO);
    const char* nameSubDir = strSubDir.c_str();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    int target_id = getUniqueID(kDAQ, kTKO, kHitPat2D, 0);
    for(int box = 0; box<6; ++box){
      const char* title = NULL;
      title = Form("TKO box%d", box);
      sub_dir->Add(createTH2(target_id + box+1, title, // 1 origin
			     24, 0, 24, 
			     40, 0, 40,
			     "TKO MA", "N of decoded hits"));
      
      top_dir->Add(sub_dir);
    }
  }

  return top_dir;  
}
