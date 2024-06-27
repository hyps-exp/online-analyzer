// -*- C++ -*-

#include <iostream>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <utility>

#include "DetectorID.hh"
#include "FuncName.hh"
#include "HistHelper.hh"
#include "HistMaker.hh"

#include <TH1.h>
#include <TH2.h>
#include <TH2Poly.h>
#include <TList.h>
#include <TMath.h>
#include <TDirectory.h>
#include <TString.h>
#include <TTimeStamp.h>

#include <Unpacker.hh>
#include <UnpackerManager.hh>

#include "TpcPadHelper.hh"
#include "AftHelper.hh"

ClassImp(HistMaker)

// getStr_FromEnum ----------------------------------------------------------
// The method to get TString from enum value
#define CONV_STRING(x) getStr_FromEnum(#x)
TString getStr_FromEnum(const char* c){
  TString str = c;
  return str.Remove(0,1);
}

// #define DETECTOR_NAME(x) TString(x).ReplaceAll("create", "");

namespace
{
using hddaq::unpacker::GUnpacker;
const auto& gUnpacker  = GUnpacker::get_instance();
      auto& gAftHelper = AftHelper::GetInstance();
}

// Constructor -------------------------------------------------------------
HistMaker::HistMaker( void )
  : TObject(),
    current_hist_id_(0)
{
}

// Destructor -------------------------------------------------------------
HistMaker::~HistMaker( void )
{
}

// -------------------------------------------------------------------------
// getListOfDetectors
// -------------------------------------------------------------------------
void HistMaker::getListOfDetectors( std::vector<TString>& vec )
{
  HistMaker& g = HistMaker::getInstance();
  std::copy( g.name_created_detectors_.begin(),
	     g.name_created_detectors_.end(),
	     back_inserter(vec) );
}

// -------------------------------------------------------------------------
// getListOfPsFiles
// -------------------------------------------------------------------------
void HistMaker::getListOfPsFiles( std::vector<TString>& vec )
{
  HistMaker& g = HistMaker::getInstance();
  std::copy( g.name_ps_files_.begin(),
	     g.name_ps_files_.end(),
	     back_inserter(vec) );
}

// -------------------------------------------------------------------------
// setHistPtr
// -------------------------------------------------------------------------
Int_t HistMaker::setHistPtr( std::vector<TH1*>& vec )
{
  vec.resize(current_hist_id_);
  for( Int_t i = 0; i<current_hist_id_; ++i ){
    Int_t unique_id = getUniqueID(i);
    vec[i] = GHist::get(unique_id);
    if( !vec[i] ){
      std::cerr << "#E " << FUNC_NAME
		<< " Pointer is NULL" << std::endl
		<< " Unique ID    : " << unique_id << std::endl
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
TH1* HistMaker::createTH1( Int_t unique_id, const TString& title,
			   Int_t nbinx, Int_t xmin, Int_t xmax,
			   const TString& xtitle, const TString& ytitle )
{
  // Add information to dictionaries which will be used to find the histogram
  Int_t sequential_id = current_hist_id_++;
  TypeRetInsert ret   = idmap_seq_from_unique_.insert( std::make_pair( unique_id, sequential_id ) );
  idmap_seq_from_name_.insert( std::make_pair( title, sequential_id ) );
  idmap_unique_from_seq_.insert( std::make_pair( sequential_id, unique_id ) );
  if(!ret.second){
    std::cerr << "#E " << FUNC_NAME
	      << " The unique id overlaps with other id"
	      << std::endl;
    std::cerr << " " << unique_id << " " << title << std::endl;
    std::exit(-1);
  }

  // create histogram using the static method of HistHelper class
  TH1 *h = GHist::I1(unique_id, title, nbinx, xmin, xmax);
  if(!h){
    std::cerr << "#E " << FUNC_NAME
	      << " Fail to create TH1"
	      << std::endl;
    std::cerr << " " << unique_id << " " << title << std::endl;
    std::exit(-1);
    //    return h;
  }

  h->GetXaxis()->SetTitle(xtitle);
  h->GetYaxis()->SetTitle(ytitle);
  return h;
}

//_____________________________________________________________________________
TH2* HistMaker::createTH2( Int_t unique_id, const TString& title,
			   Int_t nbinx, Int_t xmin, Int_t xmax,
			   Int_t nbiny, Int_t ymin, Int_t ymax,
			   const TString& xtitle, const TString& ytitle )
{
  // Add information to dictionaries which will be used to find the histogram
  Int_t sequential_id = current_hist_id_++;
  TypeRetInsert ret   = idmap_seq_from_unique_.insert( std::make_pair(unique_id, sequential_id ) );
  idmap_seq_from_name_.insert( std::make_pair(title, sequential_id ) );
  idmap_unique_from_seq_.insert( std::make_pair(sequential_id, unique_id ) );

  if( !ret.second ){
    std::cerr << "#E " << FUNC_NAME
	      << " The unique id overlaps with other id"
	      << std::endl;
    std::cerr << " " << unique_id << " " << title << std::endl;
    std::exit(-1);
  }

  // create histogram using the static method of HistHelper class
  TH2 *h = GHist::I2( unique_id, title,
		      nbinx, xmin, xmax,
		      nbiny, ymin, ymax );
  if(!h){
    std::cerr << "#E " << FUNC_NAME
	      << " Fail to create TH2"
	      << std::endl;
    std::cerr << " " << unique_id << " " << title << std::endl;
    std::exit(-1);
    //    return h;
  }

  h->GetXaxis()->SetTitle(xtitle);
  h->GetYaxis()->SetTitle(ytitle);
  return h;
}

//_____________________________________________________________________________
TH2* HistMaker::createTH2( Int_t unique_id, const TString& title,
			   Int_t nbinx, Double_t xmin, Double_t xmax,
			   Int_t nbiny, Double_t ymin, Double_t ymax,
			   const TString& xtitle, const TString& ytitle )
{
  Int_t sequential_id = current_hist_id_++;
  TypeRetInsert ret =
    idmap_seq_from_unique_.insert( std::make_pair(unique_id, sequential_id ) );
  idmap_seq_from_name_.insert( std::make_pair(title, sequential_id ) );
  idmap_unique_from_seq_.insert( std::make_pair(sequential_id, unique_id ) );
  if( !ret.second ){
    std::cerr << "#E " << FUNC_NAME
	      << " The unique id overlaps with other id"
	      << std::endl;
    std::cerr << " " << unique_id << " " << title << std::endl;
    std::exit(-1);
  }

  TH2 *h = GHist::D2( unique_id, title,
		      nbinx, xmin, xmax, nbiny, ymin, ymax );
  if(!h){
    std::cerr << "#E " << FUNC_NAME << " Fail to create TH2" << std::endl
	      << " " << unique_id << " " << title << std::endl;
    std::exit(-1);
    //    return h;
  }
  h->GetXaxis()->SetTitle(xtitle);
  h->GetYaxis()->SetTitle(ytitle);
  return h;
}

//_____________________________________________________________________________
TH2* HistMaker::createTH2Poly( Int_t unique_id, const TString& title,
                               Double_t xmin, Double_t xmax,
                               Double_t ymin, Double_t ymax )
{
  Int_t sequential_id = current_hist_id_++;
  TypeRetInsert ret =
    idmap_seq_from_unique_.insert( std::make_pair( unique_id, sequential_id ) );
  idmap_seq_from_name_.insert( std::make_pair( title, sequential_id ) );
  idmap_unique_from_seq_.insert( std::make_pair( sequential_id, unique_id ) );
  if( !ret.second ){
    std::cerr << "#E " << FUNC_NAME
	      << " The unique id overlaps with other id"
	      << std::endl;
    std::cerr << " " << unique_id << " " << title << std::endl;
    std::exit(-1);
  }

  TH2 *h = GHist::P2( unique_id, title, xmin, xmax, ymin, ymax );
  if( !h ){
    std::cerr << "#E " << FUNC_NAME << " Fail to create TH2" << std::endl
	      << " " << unique_id << " " << title << std::endl;
    std::exit(-1);
    //    return h;
  } else {
    gDirectory->GetList()->Add( h );
  }
  return h;
}

//______________________________________________________________________________
TString
HistMaker::MakeDetectorName( const TString& name )
{
  TString det = name;
  return det.ReplaceAll("create", "");
}

//_____________________________________________________________________
TList*
HistMaker::createTimeStamp( Bool_t flag_ps )
{
  TString det_name = MakeDetectorName(__func__);
  name_created_detectors_.push_back( det_name );

  if( flag_ps )
    name_ps_files_.push_back( det_name );

  auto top_dir = new TList;
  top_dir->SetName( det_name );

  auto target_id = getUniqueID( kTimeStamp, 0, kTDC );
  Int_t i = 0;
  TTimeStamp tstart( 2020, 12, 1, 0, 0, 0 );
  TTimeStamp tend( 2021, 3, 1, 0, 0, 0 );
  for( auto&& c : gUnpacker.get_root()->get_child_list() ){
    if( !c.second )
      continue;
    top_dir->Add( createTH1( target_id + i,
			     Form( "%s_%s_#%02d", det_name.Data(),
                                   c.second->get_name().c_str(), i ),
			     (tend - tstart)/60, tstart.GetSec(), tend.GetSec(),
			     "TimeStamp", ""));
    ++i;
  }

  return top_dir;
}


// -------------------------------------------------------------------------
// createCaenV1725
// -------------------------------------------------------------------------
TList*
HistMaker::createCaenV1725(Bool_t flag_ps)
{
  TString strDet = CONV_STRING(kCaenV1725);
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    name_ps_files_.push_back(strDet);
  }
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  { ///// FADC
    TString strSubDir  = CONV_STRING(kFADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kCaenV1725, 0, kFADC, 0);
    for(Int_t i = 0; i<NumOfSegCaenV1725; ++i){
      const Char_t* title = Form("%s_%s_%d", nameDetector, nameSubDir, i);
      sub_dir->Add(createTH2(target_id + i, title,
                             1024, 0, 1024, 0x200, 0, 0x3fff,
                             "Sample#", "ADC [ch]"));
    }
    top_dir->Add(sub_dir);
  }

  return top_dir;
}


// -------------------------------------------------------------------------
// createHDC
// -------------------------------------------------------------------------
TList* HistMaker::createHDC( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kHDC);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  const char* name_layer[] = {"u0", "u1", "x0", "x1"};

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kHDC, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfLayersHDC; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1024, 0, 1024,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kHDC, 0, kTDC2D, 0);
    for(Int_t i = 0; i<NumOfLayersHDC; ++i){
      const char* title = NULL;
      title = Form("%s_%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
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
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kHDC, 0, kHitPat, 0);
    for(Int_t i = 0; i<NumOfLayersHDC; ++i){
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
    TString strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    Int_t target_id = getUniqueID(kHDC, 0, kMulti, 0);
    for(Int_t i = 0; i<NumOfLayersHDC; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     10, 0, 10,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kHDC, 0, kMulti, NumOfLayersHDC);
    for(Int_t i = 0; i<NumOfLayersHDC; ++i){
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
// createSDC1
// -------------------------------------------------------------------------
TList* HistMaker::createSDC1( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kSDC1);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  const char* name_layer[NumOfLayersSDC1] = {"XPP", "V", "UP", "U", "XP", "X"};
  const char* name_Selflayer[NumOfDimSDC1] = {"XPP", "V", "UP_U", "XP_X"};
  //  const char* name_layer[NumOfLayersSDC3] = { "y0", "y1", "x0", "x1" };

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC1, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC1, 0, kTDC2D, 0);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    //TDCwide
    target_id = getUniqueID(kSDC1, 0, kTDC, 10);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_%swide_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     5000, 0, 5000,
			     "TDC [ch]", ""));
    }

    // TDCwTOTCUT -------------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC1, 0, kTDC, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC1, 0, kTDC2D, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_C%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    // TDC 2D ---------------------------------------------------
    target_id = getUniqueID(kSDC1, 0, kTDC2D, kTOTcutOffset+20);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i, title,
			     NumOfWireSDC1_HYPS[i], 0, NumOfWireSDC1_HYPS[i],
			     5000, 0, 5000,
			     "Wire Number","TDC [ch]"));
    }

    // TDC 2D wTOTCUT ---------------------------------------------------
    target_id = getUniqueID(kSDC1, 0, kTDC2D, kTOTcutOffset+30);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_C%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i, title,
			     NumOfWireSDC1_HYPS[i], 0, NumOfWireSDC1_HYPS[i],
			     5000, 0, 5000,
			     "Wire Number","TDC [ch]"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSDC1, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     500, 0, 500,
			     "TOT [ch]", ""));
    }

    // CTOT---------------------------------------------------------
    target_id = getUniqueID(kSDC1, 0, kADC, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     500, 0, 500,
			     "TOT [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC1, 0, kHitPat, 0);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     NumOfWireSDC1_HYPS[i], 0, NumOfWireSDC1_HYPS[i],
			     "wire", ""));
    }

    // HitPatwTOTCUT -----------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC1, 0, kHitPat, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     NumOfWireSDC1_HYPS[i], 0, NumOfWireSDC1_HYPS[i],
			     "wire", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // // Self Correlation ----------------------------------------------
  // {
  //   // Declaration of the sub-directory
  //   TString strSubDir  = CONV_STRING(kSelfCorr);
  //   const char* nameSubDir = strSubDir.Data();
  //   TList *sub_dir = new TList;
  //   sub_dir->SetName(nameSubDir);

  //   // Make histogram and add it
  //   for(int i=0; i<NumOfDimSDC1; i++){
  //     Int_t target_id = getUniqueID(kSDC1, kSelfCorr, 0, i);
  //     const char* title = NULL;
  //     title = Form("%s_%s_%s", nameDetector, nameSubDir, name_Selflayer[i]);
  //     sub_dir->Add(createTH2(target_id, title,
  //         		     NumOfWireSDC1_HYPS[i], 0, NumOfWireSDC1_HYPS[i],
  //         		     NumOfWireSDC1_HYPS[i], 0, NumOfWireSDC1_HYPS[i],
  //         		     Form("%s",name_layer[2*i]),Form("%s",name_layer[2*i+1])));
  //   }


  //   // insert sub directory
  //   top_dir->Add(sub_dir);
  // }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    Int_t target_id = getUniqueID(kSDC1, 0, kMulti, 0);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC1_HYPS[i], 0, NumOfWireSDC1_HYPS[i],
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC1, 0, kMulti, NumOfLayersSDC1);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC1_HYPS[i], 0, NumOfWireSDC1_HYPS[i],
			     "Multiplicity", ""));
    }

    // MultiplicitywTOTCUT ----------------------------------------
    // Make histogram and add it
    // without TDC gate
    target_id = getUniqueID(kSDC1, 0, kMulti, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC1_HYPS[i], 0, NumOfWireSDC1_HYPS[i],
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC1, 0, kMulti, NumOfLayersSDC1+kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC1_HYPS[i], 0, NumOfWireSDC1_HYPS[i],
			     "Multiplicity", ""));
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
TList* HistMaker::createSDC2( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kSDC2);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  const char* name_layer[NumOfLayersSDC2] = {"V", "UP", "U", "XP", "X"};
  const char* name_Selflayer[NumOfDimSDC2] = {"V", "UP_U", "XP_X"};

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC2, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC2, 0, kTDC2D, 0);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    //TDCwide
    target_id = getUniqueID(kSDC2, 0, kTDC, 10);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%swide_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     5000, 0, 5000,
			     "TDC [ch]", ""));
    }

    // TDCwTOTCUT -------------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC2, 0, kTDC, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC2, 0, kTDC2D, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_C%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    // TDC 2D ---------------------------------------------------
    target_id = getUniqueID(kSDC2, 0, kTDC2D, kTOTcutOffset+20);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i, title,
			     NumOfWireSDC2_HYPS[i], 0, NumOfWireSDC2_HYPS[i],
			     5000, 0, 5000,
			     "Wire Number","TDC [ch]"));
    }

    // TDC 2D wTOTCUT ---------------------------------------------------
    target_id = getUniqueID(kSDC2, 0, kTDC2D, kTOTcutOffset+30);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_C%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i, title,
			     NumOfWireSDC2_HYPS[i], 0, NumOfWireSDC2_HYPS[i],
			     5000, 0, 5000,
			     "Wire Number","TDC [ch]"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSDC2, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     500, 0, 500,
			     "TOT [ch]", ""));
    }

    // CTOT---------------------------------------------------------
    target_id = getUniqueID(kSDC2, 0, kADC, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     500, 0, 500,
			     "TOT [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC2, 0, kHitPat, 0);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     NumOfWireSDC2_HYPS[i], 0, NumOfWireSDC2_HYPS[i],
			     "wire", ""));
    }

    // HitPatwTOTCUT -----------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC2, 0, kHitPat, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     NumOfWireSDC2_HYPS[i], 0, NumOfWireSDC2_HYPS[i],
			     "wire", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // // Self Correlation ----------------------------------------------
  // {
  //   // Declaration of the sub-directory
  //   TString strSubDir  = CONV_STRING(kSelfCorr);
  //   const char* nameSubDir = strSubDir.Data();
  //   TList *sub_dir = new TList;
  //   sub_dir->SetName(nameSubDir);

  //   // Make histogram and add it
  //   for(int i=0; i<NumOfDimSDC2; i++){
  //     Int_t target_id = getUniqueID(kSDC2, kSelfCorr, 0, i);
  //     const char* title = NULL;
  //     title = Form("%s_%s_%s", nameDetector, nameSubDir, name_Selflayer[i]);
  //     sub_dir->Add(createTH2(target_id, title,
  //         		     NumOfWireSDC2_HYPS[i], 0, NumOfWireSDC2_HYPS[i],
  //         		     NumOfWireSDC2_HYPS[i], 0, NumOfWireSDC2_HYPS[i],
  //         		     Form("%s",name_layer[2*i]),Form("%s",name_layer[2*i+1])));
  //   }


  //   // insert sub directory
  //   top_dir->Add(sub_dir);
  // }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    Int_t target_id = getUniqueID(kSDC2, 0, kMulti, 0);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC2_HYPS[i], 0, NumOfWireSDC2_HYPS[i],
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC2, 0, kMulti, NumOfLayersSDC2);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC2_HYPS[i], 0, NumOfWireSDC2_HYPS[i],
			     "Multiplicity", ""));
    }

    // MultiplicitywTOTCUT ----------------------------------------
    // Make histogram and add it
    // without TDC gate
    target_id = getUniqueID(kSDC2, 0, kMulti, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC2_HYPS[i], 0, NumOfWireSDC2_HYPS[i],
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC2, 0, kMulti, NumOfLayersSDC2+kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC2_HYPS[i], 0, NumOfWireSDC2_HYPS[i],
			     "Multiplicity", ""));
    }


    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}


// -------------------------------------------------------------------------
// createSDC3
// -------------------------------------------------------------------------
TList* HistMaker::createSDC3( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kSDC3);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  const char* name_layer[NumOfLayersSDC3] = {"V", "UP", "U", "XP", "X"};
  const char* name_Selflayer[NumOfDimSDC3] = {"V", "UP_U", "XP_X"};

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC3, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC3, 0, kTDC2D, 0);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    //TDCwide
    target_id = getUniqueID(kSDC3, 0, kTDC, 10);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%swide_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     5000, 0, 5000,
			     "TDC [ch]", ""));
    }

    // TDCwTOTCUT -------------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC3, 0, kTDC, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC3, 0, kTDC2D, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    // TDC 2D ---------------------------------------------------
    target_id = getUniqueID(kSDC3, 0, kTDC2D, kTOTcutOffset+20);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i, title,
			     NumOfWireSDC3_HYPS[i], 0, NumOfWireSDC3_HYPS[i],
			     5000, 0, 5000,
			     "Wire Number","TDC [ch]"));
    }

    // TDC 2D wTOTCUT ---------------------------------------------------
    target_id = getUniqueID(kSDC3, 0, kTDC2D, kTOTcutOffset+30);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i, title,
			     NumOfWireSDC3_HYPS[i], 0, NumOfWireSDC3_HYPS[i],
			     5000, 0, 5000,
			     "Wire Number","TDC [ch]"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSDC3, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     500, 0, 500,
			     "TOT [ch]", ""));
    }

    // CTOT---------------------------------------------------------
    target_id = getUniqueID(kSDC3, 0, kADC, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     500, 0, 500,
			     "TOT [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC3, 0, kHitPat, 0);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     NumOfWireSDC3_HYPS[i], 0, NumOfWireSDC3_HYPS[i],
			     "wire", ""));
    }

    // HitPatwTOTCUT -----------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC3, 0, kHitPat, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     NumOfWireSDC3_HYPS[i], 0, NumOfWireSDC3_HYPS[i],
			     "wire", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // // Self Correlation ----------------------------------------------
  // {
  //   // Declaration of the sub-directory
  //   TString strSubDir  = CONV_STRING(kSelfCorr);
  //   const char* nameSubDir = strSubDir.Data();
  //   TList *sub_dir = new TList;
  //   sub_dir->SetName(nameSubDir);

  //   // Make histogram and add it
  //   for(int i=0; i<NumOfDimSDC3; i++){
  //     Int_t target_id = getUniqueID(kSDC3, kSelfCorr, 0, i);
  //     const char* title = NULL;
  //     title = Form("%s_%s_%s", nameDetector, nameSubDir, name_Selflayer[i]);
  //     sub_dir->Add(createTH2(target_id, title,
  //         		     NumOfWireSDC3_HYPS[i], 0, NumOfWireSDC3_HYPS[i],
  //         		     NumOfWireSDC3_HYPS[i], 0, NumOfWireSDC3_HYPS[i],
  //         		     Form("%s",name_layer[2*i]),Form("%s",name_layer[2*i+1])));
  //   }


  //   // insert sub directory
  //   top_dir->Add(sub_dir);
  // }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    Int_t target_id = getUniqueID(kSDC3, 0, kMulti, 0);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC3_HYPS[i], 0, NumOfWireSDC3_HYPS[i],
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC3, 0, kMulti, NumOfLayersSDC3);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC3_HYPS[i], 0, NumOfWireSDC3_HYPS[i],
			     "Multiplicity", ""));
    }

    // MultiplicitywTOTCUT ----------------------------------------
    // Make histogram and add it
    // without TDC gate
    target_id = getUniqueID(kSDC3, 0, kMulti, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC3_HYPS[i], 0, NumOfWireSDC3_HYPS[i],
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC3, 0, kMulti, NumOfLayersSDC3+kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i, title,
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC3_HYPS[i], 0, NumOfWireSDC3_HYPS[i],
			     "Multiplicity", ""));
    }


    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}


// -------------------------------------------------------------------------
// createBAC
// -------------------------------------------------------------------------
TList* HistMaker::createBAC( Bool_t flag_ps )
{
  TString strDet = CONV_STRING(kBAC);       // Determine the detector name
  name_created_detectors_.push_back(strDet);    // name list of crearted detector
  if(flag_ps) name_ps_files_.push_back(strDet); // name list which are displayed in Ps tab

  const char* nameDetector = strDet.Data(); // Declaration of the directory
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  const char* name_acs[] = { "BAC1", "BAC2" };

  // ADC ---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kADC); // Declaration of the sub-directory
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName( nameSubDir );

    Int_t target_id = getUniqueID(kBAC, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfSegBAC; ++i){
      const char* title = NULL;
      title = Form("%s_%s", name_acs[i], nameSubDir);
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  // ADC w/TDC ---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kADCwTDC); // Declaration of the sub-directory
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName( nameSubDir );

    Int_t target_id = getUniqueID(kBAC, 0, kADCwTDC, 0);
    for( Int_t i=0; i<NumOfSegBAC; ++i ){
      const char* title = NULL;
      title = Form("%s_%s", name_acs[i], nameSubDir);
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  // TDC---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTDC); // Declaration of the sub-directory
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kBAC, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegBAC; ++i){
      const char* title = NULL;
      title = Form("%s_%s", name_acs[i], nameSubDir);
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     //			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  { // Hit parttern -----------------------------------------------
    Int_t target_id = getUniqueID(kBAC, 0, kHitPat, 0);
    top_dir->Add(createTH1(++target_id, "BAC_hit_pattern",
			   NumOfSegBAC, 0, NumOfSegBAC,
			   "Segment", ""));
  }

  { // Multiplicity -----------------------------------------------
    Int_t target_id = getUniqueID(kBAC, 0, kMulti, 0);
    for(Int_t i = 0; i<NumOfSegBAC; ++i){
    const char* title = NULL;
    title = Form("%s_%s", name_acs[i], "multiplicity");
    top_dir->Add(createTH1(++target_id, title,
			   NumOfSegBAC+1, 0, NumOfSegBAC+1,
			   "Multiplicity", ""));
    }
  }

  return top_dir;
}


// -------------------------------------------------------------------------
// createTOF
// -------------------------------------------------------------------------
TList* HistMaker::createTOF( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kTOF);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kTOF, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfSegTOF*2; ++i){
      const char* title = NULL;
      if(i < NumOfSegTOF){
	Int_t seg = i;
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i-NumOfSegTOF;
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }

      sub_dir->Add(createTH1(target_id + i, title,
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADCwTDC ---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADCwTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kTOF, 0, kADCwTDC, 0);
    for( Int_t i=0; i<NumOfSegTOF*2; ++i ){
      const char* title = NULL;
      if( i<NumOfSegTOF ){
	Int_t seg = i;
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i-NumOfSegTOF;
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }

      sub_dir->Add(createTH1(target_id + i, title,
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TDC ---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kTOF, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegTOF*2; ++i){
      const char* title = NULL;
      if(i < NumOfSegTOF){
	Int_t seg = i;
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i-NumOfSegTOF;
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }

      sub_dir->Add(createTH1(target_id + i, title,
			     50000, 0, 2000000,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    const char* title = "TOF_hit_pattern";
    Int_t target_id = getUniqueID(kTOF, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id, title,
			   NumOfSegTOF, 0, NumOfSegTOF,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "TOF_multiplicity";
    Int_t target_id = getUniqueID(kTOF, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id, title,
			   NumOfSegTOF, 0, NumOfSegTOF,
			   "Multiplicity", ""));
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}


// -------------------------------------------------------------------------
// createCFT
// -------------------------------------------------------------------------
TList* HistMaker::createCFT( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kCFT);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);
  const char* name_Layer[] = { "UV1" ,"PHI1", "UV2", "PHI2",
                               "UV3" ,"PHI3", "UV4", "PHI4",
                               "UV_ALL" ,"PHI_ALL"};

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kCFT, 0, kTDC, 0);
    const char* sub_name = "TDC";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1( ++target_id, title , // 1 origin
			      1024, 0, 1024,
			      "TDC [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TDC-2D -------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kCFT, 0, kTDC2D  , 0);
    const char* sub_name = "TDC";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_2D", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegCFT[i], 0, NumOfSegCFT[i],
			     1024, 0, 1024,
			     "Fiber", "TDC [ch]"));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kCFT, 0, kADC, 0);
    const char* sub_name = "TOT";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1( ++target_id, title , // 1 origin
			      200, -50, 150,
			      "TOT [ch]", ""));
    }

    // CTOT---------------------------------------------------------
    target_id = getUniqueID(kCFT, 0, kADC, 10);
    sub_name = "CTOT";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1( ++target_id, title , // 1 origin
			      200, -50, 150,
			      "TOT [ch]", ""));

    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT-2D --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTOT2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kCFT, 0, kADC2D, 0);
    const char* sub_name = "TOT";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_2D_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegCFT[i], 0, NumOfSegCFT[i],
			     200, -50, 150,
			     "Fiber", "TOT [ch]"));
    }

    // CTOT-2D (after cut) --------------------------------------------
    target_id = getUniqueID(kCFT, 0, kADC2D, 10);
    sub_name = "CTOT";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_2D_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegCFT[i], 0, NumOfSegCFT[i],
			     200, -50, 150,
			     "Fiber", "TOT [ch]"));

    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC HighGain------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kHighGain);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kCFT, 0, kHighGain, 0);
    const char* sub_name = "HighGain";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1( ++target_id, title , // 1 origin
			      4096, 0, 4096,
			      "ADC [ch]", ""));
    }

    // ADC HighGain Cut -------------------------------------------
    target_id = getUniqueID(kCFT, 0, kHighGain, 20);
    sub_name = "CHighGain";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1( ++target_id, title , // 1 origin
			      4096, 0, 4096,
			      "ADC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC Pedestal -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kPedestal);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kCFT, 0, kPede, 0);
    const char* sub_name = "Pedestal";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1( ++target_id, title , // 1 origin
			      1024, -200, 824,
			      "ADC [ch]", ""));

    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC LowGain ------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kLowGain);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kCFT, 0, kLowGain , 0);
    const char* sub_name = "LowGain";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1( ++target_id, title , // 1 origin
			      4096, 0, 4096,
			      "ADC [ch]", ""));
    }

    // ADC LowGain Cut --------------------------------------------
    target_id = getUniqueID(kCFT, 0, kLowGain , 20);
    sub_name = "CLowGain";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1( ++target_id, title , // 1 origin
			      4096, 0, 4096,
			      "ADC [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC-2D HighGain ----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kHighGain_2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kCFT, 0, kHighGain, 10);
    const char* sub_name = "HighGain";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_2D_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegCFT[i], 0, NumOfSegCFT[i],
			     4096/8, 0, 4096,
			     "Fiber", "ADC [ch]"));
    }
    // ADC-2D HighGain Cut ------------------------------------------
    target_id = getUniqueID(kCFT, 0, kHighGain, 30);
    sub_name = "CHighGain";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_2D_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegCFT[i], 0, NumOfSegCFT[i],
			     4096/8, 0, 4096,
			     "Fiber", "ADC [ch]"));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC-2D Pedestal ----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kPedestal_2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kCFT, 0, kPede    , 10);
    const char* sub_name = "Pedestal";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_2D_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegCFT[i], 0, NumOfSegCFT[i],
			     4096/8,-200, 824,
			     "Fiber", "ADC [ch]"));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC-2D LowGain -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kLowGain_2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kCFT, 0, kLowGain, 10);
    const char* sub_name = "LowGain";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_2D_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegCFT[i], 0, NumOfSegCFT[i],
			     4096/8, 0, 4096,
			     "Fiber", "ADC [ch]"));
    }
    // ADC-2D LowGain Cut -------------------------------------------
    target_id = getUniqueID(kCFT, 0, kLowGain, 30);
    sub_name = "CLowGain";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_2D_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegCFT[i], 0, NumOfSegCFT[i],
			     4096/8, 0, 4096,
			     "Fiber", "ADC [ch]"));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC HighGain * TOT -----------------------------------------
  {
    TString strSubDir  = CONV_STRING(kHighGainXTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kCFT, 0, kHighGain, 40);
    const char* sub_name = "HighGainXTOT";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH2( ++target_id, title , // 1 origin
			      4096/8, 0, 4096,
			      200, -50, 150,
			      "HighGain [ch]", "TOT [ch]"));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC LowGain * TOT -----------------------------------------
  {
    TString strSubDir  = CONV_STRING(kLowGainXTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kCFT, 0, kLowGain, 40);
    const char* sub_name = "LowGainXTOT";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH2( ++target_id, title , // 1 origin
			      4096/8, 0, 4096,
			      200, -50, 150,
			      "LowGain [ch]", "TOT [ch]"));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }


  // Hit parttern -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kHitPattern);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kCFT, 0, kHitPat, 0);
    const char* sub_name = "HitPat";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     NumOfSegCFT[i], 0, NumOfSegCFT[i],
			     "Fiber", ""));

    }
    // Hit parttern (after cut) -----------------------------------
    target_id = getUniqueID(kCFT, 0, kHitPat, 10);
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      TString title = Form("%s_CHitPat_%s", nameDetector, name_Layer[i] );
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     NumOfSegCFT[i], 0, NumOfSegCFT[i],
			     "Fiber", ""));
    }
    // Hit parttern (w/BGO) -----------------------------------
    target_id = getUniqueID(kCFT, 0, kHitPat, 20);
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      TString title = Form("%s_CHitPat_%s_wBGO", nameDetector, name_Layer[i] );
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     NumOfSegCFT[i], 0, NumOfSegCFT[i],
			     "Fiber", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kMultiplicity);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    //    const char* title = "CFT_multiplicity";
    Int_t target_id = getUniqueID(kCFT, 0, kMulti, 0);
    for(Int_t i=0; i<NumOfLayersCFT + 2; ++i){
      TString title = Form("%s_Multi_%s", nameDetector, name_Layer[i] );
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     30, 0, 30,
			     "Multiplicity", ""));
    }

    // Multiplicity (after cut)------------------------------------
    target_id = getUniqueID(kCFT, 0, kMulti, 10);
    for(Int_t i=0; i<NumOfLayersCFT + 2; ++i){
      TString title = Form("%s_CMulti_%s", nameDetector, name_Layer[i] );
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     30, 0, 30,
			     "Multiplicity", ""));
    }

    // Multiplicity (w/BGO)------------------------------------
    target_id = getUniqueID(kCFT, 0, kMulti, 20);
    for(Int_t i=0; i<NumOfLayersCFT + 2; ++i){
      TString title = Form("%s_CMulti_%s_wBGO", nameDetector, name_Layer[i] );
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     30, 0, 30,
			     "Multiplicity", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  //cluster ADC HighGain -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kCluster_hgadc);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // ADC Cluster HighGain -------------------------------------------
    Int_t target_id = getUniqueID(kCFT, kCluster, kHighGain, 0);
    const char* sub_name = "ClusterHighGain";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1( ++target_id, title , // 1 origin
			      4096, 0, 4096,
			      "ADC [ch]", ""));
    }

    target_id = getUniqueID(kCFT, kCluster, kHighGain, 10);
    sub_name = "seg-MaxHG";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_2D_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegCFT[i], 0, NumOfSegCFT[i],
			     4096/8, 0, 4096,
			     "Fiber", "ADC [ch]"));
    }
    top_dir->Add(sub_dir);
  }

  //cluster ADC LowGain -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kCluster_lgadc);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // ADC Cluster LowGain -------------------------------------------
    Int_t target_id = getUniqueID(kCFT, kCluster, kLowGain, 0);
    const char* sub_name = "ClusterLowGain";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1( ++target_id, title , // 1 origin
			      4096, 0, 4096,
			      "ADC [ch]", ""));
    }

    target_id = getUniqueID(kCFT, kCluster, kLowGain, 10);
    sub_name = "seg-MaxLG";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_2D_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegCFT[i], 0, NumOfSegCFT[i],
			     4096/8, 0, 4096,
			     "Fiber", "ADC [ch]"));
    }
    top_dir->Add(sub_dir);
  }

  //cluster TDC -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kCluster_tdc);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kCFT, kCluster, kTDC, 0);
    const char* sub_name = "ClusterTDC";
    // Add to the top directory
    for(Int_t i=0; i<NumOfLayersCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1( ++target_id, title , // 1 origin
			      1024, 0, 1024,
			      "TDC [ch]", ""));
    }

    target_id = getUniqueID(kCFT, kCluster, kTDC2D, 0);
    sub_name = "seg-TDC maxseg";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneCFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_2D_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegCFT[i], 0, NumOfSegCFT[i],
			     1024, 0, 1024,
			     "Fiber", "TDC [ch]"));
    }
    top_dir->Add(sub_dir);
  }


  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}


// -------------------------------------------------------------------------
// createBGO
// -------------------------------------------------------------------------
TList* HistMaker::createBGO( Bool_t flag_ps )
{
  TString strDet = "BGO";
  name_created_detectors_.push_back(strDet);
  if(flag_ps)
    name_ps_files_.push_back(strDet);

  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    // sum hist
    Int_t target_id = getUniqueID(kBGO, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegBGO; ++i){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
      title = Form("%s_%s%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
  			     1000, 0, 1000,
  			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kBGO, 0, kTDC2D, 0);
    sub_dir->Add(createTH2(++target_id, "BGO_TDC_2D", // 1 origin
			   NumOfSegBGO, 0, NumOfSegBGO,
			   100, 0, 1000,
			   "BGO segment", "TDC [ch]"));
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kBGO, 0, kHitPat, 0);
    sub_dir->Add(createTH1(++target_id, "BGO_HitPat", // 1 origin
			   NumOfSegBGO, 0, NumOfSegBGO,
			   "BGO segment", ""));
    // insert sub directory
    top_dir->Add(sub_dir);
  }
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}


// -------------------------------------------------------------------------
// createPiID
// -------------------------------------------------------------------------
TList* HistMaker::createPiID( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kPiID);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // Make unique ID
    Int_t target_id = getUniqueID(kPiID, 0, kHighGain, 0);
    for(Int_t i = 0; i<NumOfSegPiID; ++i){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
      title = Form("%s_HighGain_%d", nameDetector, seg);

      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     0x1000, 0, 0x1000,
			     1000, 0, 4000,
			     "ADC [ch]", ""));
    }

    target_id = getUniqueID(kPiID, 0, kLowGain, 0);
    for(Int_t i = 0; i<NumOfSegPiID; ++i){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
      title = Form("%s_LowGain_%d", nameDetector, seg);

      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     0x1000, 0, 0x1000,
			     1000, 0, 4000,
			     "ADC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC w/TDC ---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADCwTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // Make unique ID
    Int_t target_id = getUniqueID(kPiID, 0, kADCwTDC, 0);
    for( Int_t i=0; i<NumOfSegPiID; ++i ){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
      title = Form("%s_HighGainwTDC_%d", nameDetector, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     0x1000, 0, 0x1000,
			     1000, 0, 4000,
			     "ADC [ch]", ""));
    }

    target_id = getUniqueID(kPiID, 0, kADCwTDC, NumOfSegPiID);
    for( Int_t i=0; i<NumOfSegPiID; ++i ){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
      title = Form("%s_LowGainwTDC_%d", nameDetector, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     0x1000, 0, 0x1000,
			     1000, 0, 4000,
			     "ADC [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }
  // ADC-2D HighGain ----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kADC_2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kPiID, 0, kADC2D, 0);
    const char* sub_name = "HighGain";
    // Add to the top directory
    const char* title = NULL;
    title = Form("%s_%s", nameDetector, sub_name );
    sub_dir->Add(createTH2(++target_id, title, // 1 origin
			   NumOfSegPiID, 0, NumOfSegPiID,
			   4096/8, 0, 4096,
			   "Segment", "ADC [ch]"));

    // ADC-2D HighGain Cut ------------------------------------------
    target_id = getUniqueID(kPiID, 0, kADC2D, 10);
    sub_name = "LowGain";
    // Add to the top directory
    title = Form("%s_%s", nameDetector, sub_name );
    sub_dir->Add(createTH2(++target_id, title, // 1 origin
			   NumOfSegPiID, 0, NumOfSegPiID,
			   4096/8, 0, 4096,
			   "Segment", "ADC [ch]"));

    // insert sub directory
    top_dir->Add(sub_dir);
  }


  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kPiID, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegPiID; ++i){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     0x1000, 0, 0x1000,
			     1000, 0, 1000,
			     "TDC [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    Int_t target_id = getUniqueID(kPiID, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(++target_id, "PiID_hit_pattern", // 1 origin
			   NumOfSegPiID, 0, NumOfSegPiID,
			   "Segment", ""));

    top_dir->Add(createTH1(++target_id, "PiID_chit_pattern", // 1 origin
			   NumOfSegPiID, 0, NumOfSegPiID,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    Int_t target_id = getUniqueID(kPiID, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(++target_id, "PiID_multiplicity", // 1 origin
			   NumOfSegPiID, 0, NumOfSegPiID,
			   "Multiplicity", ""));

    top_dir->Add(createTH1(++target_id, "PiID_cmultiplicity", // 1 origin
			   NumOfSegPiID, 0, NumOfSegPiID,
			   "Multiplicity", ""));
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}


// -------------------------------------------------------------------------
// createTriggerFlag
// -------------------------------------------------------------------------
TList* HistMaker::createTriggerFlag( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kTriggerFlag);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // TDC---------------------------------------------------------
  {
    // Make histogram and add it
    Int_t target_id = getUniqueID(kTriggerFlag, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegTFlag; ++i){
      // title = Form("%s_%d", nameDetector, i+1);
      TString title = Form( "%s #%d", trigger::STriggerFlag[i].Data(), i );
      top_dir->Add(createTH1(++target_id, title, // 1 origin
			     400, 0, 4000,
			     "TDC [ch]", ""));
    }
  }

  // Hit parttern -----------------------------------------------
  {
    const char* title = "TriggerFlag_HitPat";
    Int_t target_id = getUniqueID(kTriggerFlag, 0, kHitPat, 0);
    // Add to the top directory
    auto h = createTH1(++target_id, title, // 1 origin
		       NumOfSegTFlag, 0., NumOfSegTFlag,
		       "", "");
    for( Int_t i=0, n=trigger::STriggerFlag.size(); i<n; ++i ){
      h->GetXaxis()->SetBinLabel( i+1, trigger::STriggerFlag.at(i) );
    }
    h->SetStats(0);
    top_dir->Add(h);
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}


// -------------------------------------------------------------------------
// createDAQ
// -------------------------------------------------------------------------
TList* HistMaker::createDAQ( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kDAQ);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  std::vector<Int_t> vme_fe_id;
  std::vector<Int_t> hul_fe_id;
  std::vector<Int_t> ea0c_fe_id;
  std::vector<Int_t> cobo_fe_id;
  std::vector<Int_t> vea0c_fe_id;
  for( auto&& c : gUnpacker.get_root()->get_child_list() ){
    if( !c.second )
      continue;
    TString n = c.second->get_name();
    auto id = c.second->get_id();
    if(n.Contains("vme"))
      vme_fe_id.push_back(id);
    if(n.Contains("hul"))
      hul_fe_id.push_back(id);
    if(n.Contains("easiroc"))
      ea0c_fe_id.push_back(id);
    if(n.Contains("cobo"))
      cobo_fe_id.push_back(id);
    if(n.Contains("aft"))
      vea0c_fe_id.push_back(id);
  }

  { //___ EB
    top_dir->Add(createTH1(getUniqueID(kDAQ, kEB, kHitPat),
			   "Data size EB",
			   2000, 0, 200000,
			   "Data size [words]", ""));
  }
  { //___ VME
    auto h = createTH2(getUniqueID(kDAQ, kVME, kHitPat2D),
		       "Data size VME nodes",
		       vme_fe_id.size(), 0, vme_fe_id.size(),
		       100, 0, 1200,
		       "VME node ID", "Data size [words]");
    for(Int_t i=0, n=vme_fe_id.size(); i<n; ++i){
      h->GetXaxis()->SetBinLabel( i+1, "0x"+TString::Itoa(vme_fe_id[i], 16));
    }
    top_dir->Add(h);
  }
  // {
  //   Int_t target_id = getUniqueID(kDAQ, kCLite, kHitPat2D, 0);
  //   auto h = createTH2(target_id + 1, "Data size CLite nodes", // 1 origin
  //                      15, 0, 15,
  //                      200, 0, 400,
  //                      "CLite node ID", "Data size [words]");
  //   top_dir->Add( h );
  // }
  {
    auto h = createTH2(getUniqueID(kDAQ, kEASIROC, kHitPat2D),
		       "Data size EASIROC nodes",
		       ea0c_fe_id.size(), 0, ea0c_fe_id.size(),
		       100, 0, 300,
		       "EASIROC node ID", "Data size [words]");
    for(Int_t i=0, n=ea0c_fe_id.size(); i<n; ++i){
      h->GetXaxis()->SetBinLabel(i+1, "0x"+TString::Itoa(ea0c_fe_id[i], 16));
    }
    top_dir->Add(h);
  }
  {
    auto h = createTH2(getUniqueID(kDAQ, kHUL, kHitPat2D),
		       "Data size HUL nodes", // 1 origin
		       hul_fe_id.size(), 0, hul_fe_id.size(),
		       200, 0, 400,
		       "HUL node ID", "Data size [words]");
    for(Int_t i=0, n=hul_fe_id.size(); i<n; ++i){
      h->GetXaxis()->SetBinLabel(i+1, "0x"+TString::Itoa(hul_fe_id[i], 16));
    }
    top_dir->Add(h);
  }
  // // overflow
  // {
  //   auto h = createTH2(getUniqueID(kDAQ, kHULOverflow, kHitPat2D),
  // 		       "Overflow HUL nodes", // 1 origin
  // 		       hul_fe_id.size(), 0, hul_fe_id.size(),
  // 		       10, 0, 10,
  // 		       "HUL node ID", "Overflow");
  //   for(Int_t i=0, n=hul_fe_id.size(); i<n; ++i){
  //     h->GetXaxis()->SetBinLabel(i+1, "0x"+TString::Itoa(hul_fe_id[i], 16));
  //   }
  //   top_dir->Add(h);
  // }

  // {
  //   Int_t target_id = getUniqueID(kDAQ, kCAMAC, kHitPat2D, 0);
  //   top_dir->Add(createTH2(target_id + 1, "Data size CAMAC nodes", // 1 origin
  //       		   3, 0, 3,
  //       		   100, 0, 200,
  //       		   "CAMAC node ID", "Data size [words]"));
  // }
  // {
  //   Int_t target_id = getUniqueID(kDAQ, kOpt, kHitPat2D, 0);
  //   top_dir->Add(createTH2(target_id + 1, "Data size OptLink nodes", // 1 origin
  //       		   2, 0, 2,
  //       		   500, 0, 1000,
  //       		   "Opt node ID", "Data size [words]"));

  //   target_id = getUniqueID(kDAQ, kMiscNode, kHitPat2D, 0);
  //   top_dir->Add(createTH2(target_id + 1, "Data size Misc nodes", // 1 origin
  //       		   5, 0, 5,
  //       		   100, 0, 200,
  //       		   "Misc node ID", "Data size [words]"));
  // }

  // {
  //   // TKO box information
  //   // Declaration of the sub-directory
  //   TString strSubDir  = CONV_STRING(kTKO);
  //   const char* nameSubDir = strSubDir.Data();
  //   TList *sub_dir = new TList;
  //   sub_dir->SetName(nameSubDir);

  //   Int_t target_id = getUniqueID(kDAQ, kTKO, kHitPat2D, 0);
  //   for(Int_t box = 0; box<6; ++box){
  //     const char* title = NULL;
  //     title = Form("TKO box%d", box);
  //     sub_dir->Add(createTH2(target_id + box+1, title, // 1 origin
  // 			     24, 0, 24,
  // 			     40, 0, 40,
  // 			     "TKO MA", "N of decoded hits"));

  //     top_dir->Add(sub_dir);
  //   }
  // }

  { //___ VEASIROC
    auto h = createTH2(getUniqueID(kDAQ, kVMEEASIROC, kHitPat2D),
		       "Data size VME EASIROC nodes",
		       vea0c_fe_id.size(), 0, vea0c_fe_id.size(),
		       100, 0, 1000,
		       "VME EASIROC node ID", "Data size [words]");
    for(Int_t i=0, n=vea0c_fe_id.size(); i<n; ++i){
      h->GetXaxis()->SetBinLabel(i+1, "0x"+TString::Itoa(vea0c_fe_id[i], 16));
    }
    top_dir->Add(h);
  }

  { //___ MultiHitTdc
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kMultiHitTdc);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kDAQ, 0, kMultiHitTdc);
    const char* sub_name = "MultiHitTdc";
    { // BC3
      const char* name_layer[NumOfLayersBC3] = {"x0", "x1", "v0", "v1", "u0", "u1"};
      TString strDet = CONV_STRING(kBC3);
      const char* nameDetector = strDet.Data();
      // Add to the top directory
      for(Int_t i=0; i<NumOfLayersBC3; ++i){
	const char* title = NULL;
	title = Form("%s_%s_%s", nameDetector, sub_name, name_layer[i]);
	sub_dir->Add(createTH2(target_id++, title, // 1 origin
			       NumOfWireBC3, 0, NumOfWireBC3,
			       20, 0, 20,
			       "ch", "MultiHitTdc"));
      }
    }
    { // BC4
      const char* name_layer[NumOfLayersBC4] = {"u0", "u1", "v0", "v1", "x0", "x1"};
      TString strDet = CONV_STRING(kBC4);
      const char* nameDetector = strDet.Data();
      // Add to the top directory
      for(Int_t i=0; i<NumOfLayersBC4; ++i){
	const char* title = NULL;
	title = Form("%s_%s_%s", nameDetector, sub_name, name_layer[i]);
	sub_dir->Add(createTH2(target_id++, title, // 1 origin
			       NumOfWireBC4, 0, NumOfWireBC4,
			       20, 0, 20,
			       "ch", "MultiHitTdc"));
      }
    }
    { // SDC1
      const char* name_layer[NumOfLayersSDC1] = {"u0", "u1", "x0", "x1", "v0", "v1"};
      TString strDet = CONV_STRING(kSDC1);
      const char* nameDetector = strDet.Data();
      // Add to the top directory
      for(Int_t i=0; i<NumOfLayersSDC1; ++i){
	const char* title = NULL;
	title = Form("%s_%s_%s", nameDetector, sub_name, name_layer[i]);
	sub_dir->Add(createTH2(target_id++, title, // 1 origin
			       NumOfWireSDC1, 0, NumOfWireSDC1,
			       20, 0, 20,
			       "ch", "MultiHitTdc"));
      }
    }
    { // SDC2
      const char* name_layer[NumOfLayersSDC2] = { "v0", "v1", "u0", "u1" };
      TString strDet = CONV_STRING(kSDC2);
      const char* nameDetector = strDet.Data();
      // Add to the top directory
      for(Int_t i=0; i<NumOfLayersSDC2; ++i){
	const char* title = NULL;
	title = Form("%s_%s_%s", nameDetector, sub_name, name_layer[i]);
	sub_dir->Add(createTH2(target_id++, title, // 1 origin
			       NumOfWireSDC2, 0, NumOfWireSDC2,
			       20, 0, 20,
			       "ch", "MultiHitTdc"));
      }
    }
    { // BH2MTLR
      TString strDet = CONV_STRING(kBH2);
      const char* nameDetector = strDet.Data();
      // Add to the top directory
      const char* title = Form("%s_MT_%s", nameDetector, nameSubDir);
      sub_dir->Add(createTH2(target_id++, title, // 1 origin
			     NumOfSegBH2, 0, NumOfSegBH2,
			     20, 0, 20,
			     "ch", "MultiHitTdc"));
    }

    top_dir->Add(sub_dir);
  }

  // { //___ CoBo
  //   auto h = createTH2(getUniqueID( kDAQ, kCoBo, kHitPat2D),
  // 		       "Data size CoBo nodes",
  // 		       cobo_fe_id.size(), 0, cobo_fe_id.size(),
  // 		       100, 0, 200000,
  // 		       "CoBo node ID", "Data size [words]");
  //   for(Int_t i=0, n=cobo_fe_id.size(); i<n; ++i){
  //     h->GetXaxis()->SetBinLabel(i+1, "0x"+TString::Itoa(cobo_fe_id[i], 16));
  //   }
  //   top_dir->Add(h);
  // }

  return top_dir;
}


// -------------------------------------------------------------------------
// createCorrelation
// -------------------------------------------------------------------------
TList* HistMaker::createCorrelation( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kCorrelation);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  {
    Int_t target_id = getUniqueID( kCorrelation, 0, 0, 0 );
    // BH1 vs BFT -----------------------------------------------
    top_dir->Add( createTH2( ++target_id, "BH1_BFT", // 1 origin
                             NumOfSegBH1, 0, NumOfSegBH1,
                             NumOfSegBFT, 0, NumOfSegBFT,
                             "BH1 seg", "BFT seg" ) );

    // BH2 vs BH1 -----------------------------------------------
    top_dir->Add( createTH2( ++target_id, "BH2_BH1", // 1 origin
                             NumOfSegBH1, 0, NumOfSegBH1,
                             NumOfSegBH2, 0, NumOfSegBH2,
                             "BH1 seg", "BH2 seg" ) );

    // // SCH vs TOF -----------------------------------------------
    // top_dir->Add( createTH2( ++target_id, "SCH_TOF", // 1 origin
    //                          NumOfSegSCH, 0, NumOfSegSCH,
    //                          NumOfSegTOF+NumOfSegBVH, 0, NumOfSegTOF+NumOfSegBVH,
    //                          "SCH seg", "TOF seg" ) );
    // BC4 vs BC3 ----------------------------------------------
    top_dir->Add( createTH2( ++target_id, "BC4x1_BC3x0", // 1 origin
                             NumOfWireBC3, 0, NumOfWireBC3,
                             NumOfWireBC4, 0, NumOfWireBC4,
                             "BC3 wire", "BC4 wire" ) );

    // SDC3 vs SDC1 --------------------------------------------
    top_dir->Add( createTH2( ++target_id, "SDC3x1_SDC1x0", // 1 origin
                             NumOfWireSDC1, 0, NumOfWireSDC1,
                             NumOfWireSDC3, 0, NumOfWireSDC3,
                             "SDC1 wire", "SDC3 wire" ) );

    // SDC3 vs SDC4 --------------------------------------------
    top_dir->Add( createTH2( ++target_id, "SDC3x1_SDC4x0", // 1 origin
                             NumOfWireSDC4, 0, NumOfWireSDC4,
                             NumOfWireSDC3, 0, NumOfWireSDC3,
                             "SDC4 wire", "SDC3 wire" ) );

    // TOF vs SDC4 --------------------------------------------
    top_dir->Add( createTH2( ++target_id, "TOF_SDC4x0", // 1 origin
                             NumOfWireSDC4, 0, NumOfWireSDC4,
                             NumOfSegTOF, 0, NumOfSegTOF,
                             "SDC4 wire", "TOF seg" ) );

    // AC1 vs TOF -----------------------------------------------
    top_dir->Add( createTH2( ++target_id, "AC1_TOF", // 1 origin
                             NumOfSegAC1-4, 0, NumOfSegAC1-4,
                             NumOfSegTOF, 0, NumOfSegTOF,
                             "AC1 PMT No", "TOF seg" ) );

    // WC vs TOF -----------------------------------------------
    top_dir->Add( createTH2( ++target_id, "WC_TOF", // 1 origin
                             NumOfSegWC, 0, NumOfSegWC,
                             NumOfSegTOF, 0, NumOfSegTOF,
                             "WC seg", "TOF seg" ) );


    // // FHT1 UX1 vs FHT1 UX2 --------------------------------------------
    // top_dir->Add(createTH2(++target_id, "FHT1_UX1_FHT1_UX2", // 1 origin
    //     		   NumOfSegFHT1, 0, NumOfSegFHT1,
    //     		   NumOfSegFHT1, 0, NumOfSegFHT1,
    //     		   "FHT1 UX1", "FHT1 UX2"));

    // // FHT1 DX1 vs FHT1 DX2 --------------------------------------------
    // top_dir->Add(createTH2(++target_id, "FHT1_DX1_FHT1_DX2", // 1 origin
    //     		   NumOfSegFHT1, 0, NumOfSegFHT1,
    //     		   NumOfSegFHT1, 0, NumOfSegFHT1,
    //     		   "FHT1 DX1", "FHT1 DX2"));

    // // FHT2 UX1 vs FHT2 UX2 --------------------------------------------
    // top_dir->Add(createTH2(++target_id, "FHT2_UX1_FHT2_UX2", // 1 origin
    //     		   NumOfSegFHT2, 0, NumOfSegFHT2,
    //     		   NumOfSegFHT2, 0, NumOfSegFHT2,
    //     		   "FHT2 UX1", "FHT2 UX2"));

    // // FHT2 DX1 vs FHT2 DX2 --------------------------------------------
    // top_dir->Add(createTH2(++target_id, "FHT2_DX1_FHT2_DX2", // 1 origin
    //     		   NumOfSegFHT2, 0, NumOfSegFHT2,
    //     		   NumOfSegFHT2, 0, NumOfSegFHT2,
    //     		   "FHT2 DX1", "FHT2 DX2"));
  }

  { // SCH x TOF for Mtx2D
    Int_t target_id = getUniqueID( kCorrelation, 1, 0, 0 );
    for( Int_t i=0; i<2; ++i ){
      top_dir->Add(createTH2( ++target_id, Form( "SCH_TOF_Mtx2D%d", i+1 ),
                              NumOfSegSCH, 0, NumOfSegSCH,
                              NumOfSegTOF, 0, NumOfSegTOF,
                              "SCH seg", "TOF seg" ) );
    }
  }

  { // SCH x TOF for Mtx3D
    Int_t target_id = getUniqueID( kCorrelation, 2, 0, 0 );
    for( Int_t i=0; i<NumOfSegBH2; ++i ){
      top_dir->Add(createTH2( ++target_id, Form( "SCH_TOF_Mtx3D_BH2-%d", i+1 ),
                              NumOfSegSCH, 0, NumOfSegSCH,
                              NumOfSegTOF+NumOfSegBVH, 0, NumOfSegTOF+NumOfSegBVH,
                              "SCH seg", "TOF seg" ) );
    }
  }

  return top_dir;
}

/*
// -------------------------------------------------------------------------
// createCorrelatio_CATCH
// -------------------------------------------------------------------------
TList* HistMaker::createCorrelation_catch( Bool_t flag_ps )
{
// Determine the detector name
TString strDet = CONV_STRING(kCorrelation_catch);
// name list of crearted detector
name_created_detectors_.push_back(strDet);
if(flag_ps){
// name list which are displayed in Ps tab
name_ps_files_.push_back(strDet);
}

// BGO vs CFT -----------------------------------------------
// Declaration of the directory
// Just type conversion from TString to char*
const char* nameDetector = strDet.Data();
TList *top_dir = new TList;
top_dir->SetName(nameDetector);

const char* name_Layer[] = { "UV1" ,"PHI1", "UV2", "PHI2",
"UV3" ,"PHI3", "UV4", "PHI4",
"UV_ALL" ,"PHI_ALL"};

{
Int_t target_id = getUniqueID(kCorrelation_catch, 0, 0, 0);
for(Int_t l=0; l<NumOfLayersCFT - 4; ++l){
const char* title = NULL;
title = Form("BGO_CFT_%s", name_Layer[l*2+1] );
top_dir->Add(createTH2(++target_id, title, // 1 origin
NumOfSegCFT[l*2+1], 0, NumOfSegCFT[l*2+1],
NumOfSegBGO, 0, NumOfSegBGO,
"CFT seg", "BGO seg"));
}
}

{
Int_t target_id = getUniqueID(kCorrelation_catch, 1, 0, 0);
const char* title = "BGO_PiID";
top_dir->Add(createTH2(++target_id, title, // 1 origin
NumOfSegBGO, 0, NumOfSegBGO,
NumOfSegPiID, 0, NumOfSegPiID,
"BGO seg", "PiID seg"));
}

return top_dir;
}
*/


// -------------------------------------------------------------------------
// createVMEEASIROC
// -------------------------------------------------------------------------
TList* HistMaker::createVMEEASIROC( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kVMEEASIROC);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // TDC-2D -------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kVMEEASIROC, 0, kTDC2D  , 0);
    const char* sub_name = "TDC";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneVMEEASIROC; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%d", nameDetector, sub_name, PlaneIdOfVMEEASIROC[i]);
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegVMEEASIROC, 0, NumOfSegVMEEASIROC,
			     4096, 0, 4096,
			     "ch", "TDC [ch]"));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT-2D -------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTOT2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kVMEEASIROC, 0, kTOT2D  , 0);
    const char* sub_name = "TOT";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneVMEEASIROC; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%d", nameDetector, sub_name, PlaneIdOfVMEEASIROC[i]);
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegVMEEASIROC, 0, NumOfSegVMEEASIROC,
			     150, 0, 150,
			     "ch", "TOT [ch]"));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC-2D HighGain ----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kHighGain_2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kVMEEASIROC, 0, kHighGain, 10);
    const char* sub_name = "HG";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneVMEEASIROC; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%d", nameDetector, sub_name, PlaneIdOfVMEEASIROC[i]);
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegVMEEASIROC, 0, NumOfSegVMEEASIROC,
			     4096, 0, 4096,
			     "ch", "ADC [ch]"));
    }

    // ADC-2D HighGain Cut ------------------------------------------
    target_id = getUniqueID(kVMEEASIROC, 0, kHighGain, 100);
    sub_name = "CHG";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneVMEEASIROC; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%d", nameDetector, sub_name, PlaneIdOfVMEEASIROC[i]);
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegVMEEASIROC, 0, NumOfSegVMEEASIROC,
			     4096, 0, 4096,
			     "ch", "ADC [ch]"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC-2D LowGain -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kLowGain_2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kVMEEASIROC, 0, kLowGain, 10);
    const char* sub_name = "LG";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneVMEEASIROC; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%d", nameDetector, sub_name, PlaneIdOfVMEEASIROC[i]);
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegVMEEASIROC, 0, NumOfSegVMEEASIROC,
			     4096, 0, 4096,
			     "ch", "ADC [ch]"));
    }

    // ADC-2D HighGain Cut ------------------------------------------
    target_id = getUniqueID(kVMEEASIROC, 0, kLowGain, 100);
    sub_name = "CLG";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneVMEEASIROC; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%d", nameDetector, sub_name, PlaneIdOfVMEEASIROC[i]);
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegVMEEASIROC, 0, NumOfSegVMEEASIROC,
			     4096, 0, 4096,
			     "ch", "ADC [ch]"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // MultiHitTdc-2D -------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kMultiHitTdc);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kVMEEASIROC, 0, kMultiHitTdc, 20);
    const char* sub_name = "MultiHitTdc";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneVMEEASIROC; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%d", nameDetector, sub_name, PlaneIdOfVMEEASIROC[i]);
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegVMEEASIROC, 0, NumOfSegVMEEASIROC,
			     20, 0, 20,
			     "ch", "MultiHitTdc"));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC HighGain vs TOT ----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kHighGainvsTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kVMEEASIROC, 0, kHighGainvsTOT, 30);
    const char* sub_name = "HGvsTOT";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneVMEEASIROC; ++i){
      TString strSubSubDir  = Form("%s_%s_%d", nameDetector, sub_name, PlaneIdOfVMEEASIROC[i]);
      const char* nameSubSubDir = strSubSubDir.Data();
      TList *sub_sub_dir = new TList;
      sub_sub_dir->SetName(nameSubSubDir);
      for(Int_t j=0; j<NumOfSegVMEEASIROC; ++j){
	const char* title = NULL;
	title = Form("%s_%s_%d_%d", nameDetector, sub_name, PlaneIdOfVMEEASIROC[i], j);
	sub_sub_dir->Add(createTH2(++target_id, title, // 1 origin
				   4096, 0, 4096,
				   150, 0, 150,
				   "ADC [ch]", "TOT [ch]"));
      }
      sub_dir->Add(sub_sub_dir);
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

    // Hit parttern -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kHitPattern);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kVMEEASIROC, 0, kHitPat, 0);
    const char* sub_name = "HitPat";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneVMEEASIROC; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%d", nameDetector, sub_name, PlaneIdOfVMEEASIROC[i]);
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     NumOfSegVMEEASIROC, 0, NumOfSegVMEEASIROC,
			     "ch", ""));
    }
    top_dir->Add(sub_dir);
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}




//###########################################################
//                                                          #
//            Unused or Old Detectors from here             #
//                                                          #
//###########################################################

#if 0

//_____________________________________________________________________________
TList*
HistMaker::createBH1(Bool_t flag_ps)
{
  TString strDet = CONV_STRING(kBH1);
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    name_ps_files_.push_back(strDet);
  }
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  { ///// ADC
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kBH1, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfSegBH1*2; ++i){
      const char* title = NULL;
      if(i < NumOfSegBH1){
	Int_t seg = i+1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i+1-NumOfSegBH1; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }
  { ///// ADC w/TDC
    TString strSubDir  = CONV_STRING(kADCwTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kBH1, 0, kADCwTDC, 0);
    for( Int_t i=0; i<NumOfSegBH1*2; ++i ){
      const char* title = NULL;
      if( i<NumOfSegBH1 ){
	Int_t seg = i+1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i+1-NumOfSegBH1; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }
  { ///// TDC
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kBH1, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegBH1*2; ++i){
      const char* title = NULL;
      if(i < NumOfSegBH1){
	Int_t seg = i+1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i+1-NumOfSegBH1; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     10000, 0, 400000,
     			     50000, 0, 2000000,
			     "TDC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }
  { ///// Hit parttern
    Int_t target_id = getUniqueID(kBH1, 0, kHitPat, 0);
    top_dir->Add(createTH1(++target_id, "BH1_hit_pattern", // 1 origin
			   NumOfSegBH1, 0, NumOfSegBH1,
			   "Segment", ""));
  }
  { ///// Multiplicity
    Int_t target_id = getUniqueID(kBH1, 0, kMulti, 0);
    top_dir->Add(createTH1(++target_id, "BH1_multiplicity", // 1 origin
			   NumOfSegBH1, 0, NumOfSegBH1,
			   "Multiplicity", ""));
  }
  return top_dir;
}

//_____________________________________________________________________________
TList*
HistMaker::createBFT(Bool_t flag_ps)
{
  TString strDet = CONV_STRING(kBFT);
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    name_ps_files_.push_back(strDet);
  }
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);
  { ///// TDC
    Int_t target_id = getUniqueID(kBFT, 0, kTDC, 0);
    top_dir->Add(createTH1(++target_id, "BFT_TDC_U",
			   1024, 0, 1024,
			   "TDC [ch]", ""));
    top_dir->Add(createTH1(++target_id, "BFT_TDC_D",
			   1024, 0, 1024,
			   "TDC [ch]", ""));
  }
  { ///// TOT
    Int_t target_id = getUniqueID(kBFT, 0, kADC, 0);
    top_dir->Add(createTH1(++target_id, "BFT_TOT_U",
			   200, -50, 150,
			   "TOT [ch]", ""));

    top_dir->Add(createTH1(++target_id, "BFT_TOT_D",
			   200, -50, 150,
			   "TOT [ch]", ""));
  }
  { ///// Hit parttern
    Int_t target_id = getUniqueID(kBFT, 0, kHitPat, 0);
    top_dir->Add(createTH1(++target_id, "BFT_HitPat_U",
			   NumOfSegBFT, 0, NumOfSegBFT,
			   "Segment", ""));
    top_dir->Add(createTH1(++target_id, "BFT_HitPat_D",
			   NumOfSegBFT, 0, NumOfSegBFT,
			   "Segment", ""));
  }
  { ///// Multiplicity
    const char* title = "BFT_multiplicity";
    Int_t target_id = getUniqueID(kBFT, 0, kMulti, 0);
    top_dir->Add(createTH1(++target_id, title,
			   30, 0, 30,
			   "Multiplicity", ""));
  }

  // TDC-2D (after cut) --------------------------------------------
  {
    Int_t target_id = getUniqueID(kBFT, 0, kTDC2D, 0);
    // Add to the top directory
    top_dir->Add(createTH2(++target_id, "BFT_CTDC_U_2D", // 1 origin
			   NumOfSegBFT, 0, NumOfSegBFT,
			   1024, 0, 1024,
			   "Segment", "TDC [ch]"));

    top_dir->Add(createTH2(++target_id, "BFT_CTDC_D_2D", // 1 origin
			   NumOfSegBFT, 0, NumOfSegBFT,
			   1024, 0, 1024,
			   "Segment", "TDC [ch]"));
  }

  // TOT-2D (after cut) --------------------------------------------
  {
    Int_t target_id = getUniqueID(kBFT, 0, kADC2D, 0);
    // Add to the top directory
    top_dir->Add(createTH2(++target_id, "BFT_CTOT_U_2D", // 1 origin
			   NumOfSegBFT, 0, NumOfSegBFT,
			   200, -50, 150,
			   "Segment", "TOT [ch]"));

    top_dir->Add(createTH2(++target_id, "BFT_CTOT_D_2D", // 1 origin
			   NumOfSegBFT, 0, NumOfSegBFT,
			   200, -50, 150,
			   "Segment", "TOT [ch]"));
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createBC3
// -------------------------------------------------------------------------
TList* HistMaker::createBC3( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kBC3);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  const char* name_layer[NumOfLayersBC3] = {"x0", "x1", "v0", "v1", "u0", "u1"};
  const char* name_Selflayer[NumOfDimBC3] = { "x0_x1", "v0_v1","u0_u1" };
  //  const char* name_layer[NumOfLayersSDC3] = { "y0", "y1", "x0", "x1" };

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kBC3, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kBC3, 0, kTDC2D, 0);
    for(Int_t i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    //TDCwide
    target_id = getUniqueID(kBC3, 0, kTDC, 10);
    for(Int_t i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_%swide_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     5000, 0, 5000,
			     "TDC [ch]", ""));
    }

    // TDCwTOTCUT -------------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kBC3, 0, kTDC, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kBC3, 0, kTDC2D, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    // TDC 2D ---------------------------------------------------
    target_id = getUniqueID(kBC3, 0, kTDC2D, kTOTcutOffset+20);
    for(Int_t i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i+1, title,
			     NumOfWireBC3, 0, NumOfWireBC3,
			     5000, 0, 5000,
			     "Wire Number","TDC [ch]"));
    }

    // TDC 2D wTOTCUT ---------------------------------------------------
    target_id = getUniqueID(kBC3, 0, kTDC2D, kTOTcutOffset+30);
    for(Int_t i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i+1, title,
			     NumOfWireBC3, 0, NumOfWireBC3,
			     5000, 0, 5000,
			     "Wire Number","TDC [ch]"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kBC3, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     500, 0, 500,
			     "TOT [ch]", ""));
    }

    // CTOT---------------------------------------------------------
    target_id = getUniqueID(kBC3, 0, kADC, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     500, 0, 500,
			     "TOT [ch]", ""));
    }

     // TOT 2D ---------------------------------------------------------
     target_id = getUniqueID(kBC3, 0, kADC2D, 0);
     for(Int_t i = 0; i<NumOfLayersBC3; ++i){
       const char* title = NULL;
       title = Form("%s_%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
       sub_dir->Add(createTH2(target_id + i+1, title, // 1 origin
     			     NumOfWireBC3, 0, NumOfWireBC3,
     			     500, 0, 500,
     			     "Wire Number","TOT [ch]"));
     }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kBC3, 0, kHitPat, 0);
    for(Int_t i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     NumOfWireBC3, 0, NumOfWireBC3,
			     "wire", ""));
    }

    // HitPatwTOTCUT -----------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kBC3, 0, kHitPat, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     NumOfWireBC3, 0, NumOfWireBC3,
			     "wire", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Self Correlation ----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kSelfCorr);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    for(int i=0; i<NumOfDimBC3; i++){
      Int_t target_id = getUniqueID(kBC3, kSelfCorr, 0, i);
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_Selflayer[i]);
      sub_dir->Add(createTH2(target_id + 1, title, // 1 origin
          		     NumOfWireBC3, 0, NumOfWireBC3,
          		     NumOfWireBC3, 0, NumOfWireBC3,
          		     Form("%s",name_layer[2*i]),Form("%s",name_layer[2*i+1])));
    }


    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    Int_t target_id = getUniqueID(kBC3, 0, kMulti, 0);
    for(Int_t i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireBC3, 0, NumOfWireBC3,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kBC3, 0, kMulti, NumOfLayersBC3);
    for(Int_t i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireBC3, 0, NumOfWireBC3,
			     "Multiplicity", ""));
    }

    // MultiplicitywTOTCUT ----------------------------------------
    // Make histogram and add it
    // without TDC gate
    target_id = getUniqueID(kBC3, 0, kMulti, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireBC3, 0, NumOfWireBC3,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kBC3, 0, kMulti, NumOfLayersBC3+kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersBC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireBC3, 0, NumOfWireBC3,
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
TList* HistMaker::createBC4( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kBC4);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  const char* name_layer[NumOfLayersBC4] = {"u0", "u1", "v0", "v1", "x0", "x1"};
  const char* name_Selflayer[NumOfDimBC4] = { "vu0_u1", "v0_v1","x0_x1" };
  //  const char* name_layer[NumOfLayersSDC3] = { "y0", "y1", "x0", "x1" };

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kBC4, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kBC4, 0, kTDC2D, 0);
    for(Int_t i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    //TDCwide
    target_id = getUniqueID(kBC4, 0, kTDC, 10);
    for(Int_t i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_%swide_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     5000, 0, 5000,
			     "TDC [ch]", ""));
    }

    // TDCwTOTCUT -------------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kBC4, 0, kTDC, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kBC4, 0, kTDC2D, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
		      	     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    // TDC 2D ---------------------------------------------------
    target_id = getUniqueID(kBC4, 0, kTDC2D, kTOTcutOffset+20);
    for(Int_t i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i+1, title,
			     NumOfWireBC4, 0, NumOfWireBC4,
			     5000, 0, 5000,
			     "Wire Number","TDC [ch]"));
    }

    // TDC 2D wTOTCUT ---------------------------------------------------
    target_id = getUniqueID(kBC4, 0, kTDC2D, kTOTcutOffset+30);
    for(Int_t i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i+1, title,
			     NumOfWireBC4, 0, NumOfWireBC4,
			     5000, 0, 5000,
			     "Wire Number","TDC [ch]"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kBC4, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     500, 0, 500,
			     "TOT [ch]", ""));
    }

    // CTOT---------------------------------------------------------
    target_id = getUniqueID(kBC4, 0, kADC, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     500, 0, 500,
			     "TOT [ch]", ""));
    }

     // TOT 2D ---------------------------------------------------------
     target_id = getUniqueID(kBC4, 0, kADC2D, 0);
     for(Int_t i = 0; i<NumOfLayersBC4; ++i){
       const char* title = NULL;
       title = Form("%s_%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
       sub_dir->Add(createTH2(target_id + i+1, title, // 1 origin
     			     NumOfWireBC4, 0, NumOfWireBC4,
     			     500, 0, 500,
     			     "Wire Number","TOT [ch]"));
     }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kBC4, 0, kHitPat, 0);
    for(Int_t i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     NumOfWireBC4, 0, NumOfWireBC4,
			     "wire", ""));
    }

    // HitPatwTOTCUT -----------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kBC4, 0, kHitPat, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     NumOfWireBC4, 0, NumOfWireBC4,
			     "wire", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Self Correlation ----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kSelfCorr);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    for(int i=0; i<NumOfDimBC4; i++){
      Int_t target_id = getUniqueID(kBC4, kSelfCorr, 0, i);
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_Selflayer[i]);
      sub_dir->Add(createTH2(target_id + 1, title, // 1 origin
          		     NumOfWireBC4, 0, NumOfWireBC4,
          		     NumOfWireBC4, 0, NumOfWireBC4,
          		     Form("%s",name_layer[2*i]),Form("%s",name_layer[2*i+1])));
    }


    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    Int_t target_id = getUniqueID(kBC4, 0, kMulti, 0);
    for(Int_t i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireBC4, 0, NumOfWireBC4,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kBC4, 0, kMulti, NumOfLayersBC4);
    for(Int_t i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireBC4, 0, NumOfWireBC4,
			     "Multiplicity", ""));
    }

    // MultiplicitywTOTCUT ----------------------------------------
    // Make histogram and add it
    // without TDC gate
    target_id = getUniqueID(kBC4, 0, kMulti, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireBC4, 0, NumOfWireBC4,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kBC4, 0, kMulti, NumOfLayersBC4+kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersBC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireBC4, 0, NumOfWireBC4,
			     "Multiplicity", ""));
    }


    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createBH2
// -------------------------------------------------------------------------
TList* HistMaker::createBH2( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kBH2);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kBH2, 0, kADC, 0);
    for( Int_t i=0; i<NumOfSegBH2*2; ++i ){
      const char* title = NULL;
      if( i<NumOfSegBH2 ){
	Int_t seg = i +1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i +1 -NumOfSegBH2; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }

      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC w/TDC ---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADCwTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kBH2, 0, kADCwTDC, 0);
    for( Int_t i=0; i<NumOfSegBH2*2; ++i ){
      const char* title = NULL;
      if( i<NumOfSegBH2 ){
	Int_t seg = i +1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i +1 -NumOfSegBH2; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }

      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TDC ----------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kBH2, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegBH2*2; ++i){
      const char* title = NULL;
      if(i < NumOfSegBH2){
	Int_t seg = i+1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i+1-NumOfSegBH2; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }

      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     10000, 0, 400000,
			     50000, 0, 2000000,
			     "TDC [ch]", ""));
    }

    // BH2MTLR(BH2 mean timer low reso.) ----------------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kBH2, 0, kTDC, 20);
    for(Int_t i = 0; i<NumOfSegBH2; ++i){
      Int_t seg = i+1; // 1 origin
      const char* title = Form("%s_MT_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(target_id++, title, // 1 origin
			     5000, 0, 5000,
			     "TDC [ch]", ""));

    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // MeanTimer (FPGA)----------------------------------------------------
  // {
  //   // Declaration of the sub-directory
  //   TString strSubDir  = "FPGA_TDC_MeanTimer";
  //   const char* nameSubDir = strSubDir.Data();
  //   TList *sub_dir = new TList;
  //   sub_dir->SetName(nameSubDir);

  //   // Make histogram and add it
  //   Int_t target_id = getUniqueID(kBH2, 0, kBH2MT, 0);
  //   for(Int_t i = 0; i<NumOfSegBH2; ++i){
  //     const char* title = NULL;
  //     Int_t seg = i+1; // 1 origin
  //     title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);

  //     sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
  // 			     //			     10000, 0, 400000,
  // 			     50000, 0, 2000000,
  // 			     "TDC [ch]", ""));
  //   }

  //   // insert sub directory
  //   top_dir->Add(sub_dir);
  // }

  // Hit parttern -----------------------------------------------
  {
    const char* title = "BH2_hit_pattern";
    Int_t target_id = getUniqueID(kBH2, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegBH2, 0, NumOfSegBH2,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "BH2_multiplicity";
    Int_t target_id = getUniqueID(kBH2, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegBH2+1, 0, NumOfSegBH2+1,
			   "Multiplicity", ""));
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createHTOF
// -------------------------------------------------------------------------
TList* HistMaker::createHTOF( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kHTOF);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kHTOF, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfSegHTOF*2; ++i){
      const char* title = NULL;
      if(i < NumOfSegHTOF){
	Int_t seg = i+1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i+1-NumOfSegHTOF; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }

      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADCwTDC ---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADCwTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kHTOF, 0, kADCwTDC, 0);
    for( Int_t i=0; i<NumOfSegHTOF*2; ++i ){
      const char* title = NULL;
      if( i<NumOfSegHTOF ){
	Int_t seg = i+1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i+1-NumOfSegHTOF; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }

      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TDC ---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kHTOF, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegHTOF*2; ++i){
      const char* title = NULL;
      if(i < NumOfSegHTOF){
	Int_t seg = i+1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i+1-NumOfSegHTOF; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }

      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     50000, 0, 2000000,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    const char* title = "HTOF_hit_pattern";
    Int_t target_id = getUniqueID(kHTOF, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(++target_id, title, // 1 origin
			   NumOfSegHTOF, 0, NumOfSegHTOF,
			   "Segment", ""));
    title = Form( "%s_HitPatternPoly", nameDetector );
    auto h_hit_poly = dynamic_cast<TH2Poly*>
      ( createTH2Poly( ++target_id, title, -400, 400, -400, 400 ) );
    const Double_t L = 337;
    const Double_t t = 10;
    const Double_t w = 68;
    Double_t theta[8];
    Double_t X[5];
    Double_t Y[5];
    Double_t seg_X[5];
    Double_t seg_Y[5];
    for( Int_t i=0; i<8; i++ ){
      theta[i] = (-180+45*i)*acos(-1)/180.;
      for( Int_t j=0; j<4; j++ ){
	seg_X[1] = L-t/2.;
	seg_X[2] = L+t/2.;
	seg_X[3] = L+t/2.;
	seg_X[4] = L-t/2.;
	seg_X[0] = seg_X[4];
	seg_Y[1] = w*j-2*w;
	seg_Y[2] = w*j-2*w;
	seg_Y[3] = w*j-1*w;
	seg_Y[4] = w*j-1*w;
	seg_Y[0] = seg_Y[4];
	for( Int_t k=0; k<5; k++ ){
	  X[k] = cos(theta[i])*seg_X[k]-sin(theta[i])*seg_Y[k];
	  Y[k] = sin(theta[i])*seg_X[k]+cos(theta[i])*seg_Y[k];
	}
	h_hit_poly->AddBin( 5, X, Y );
      }
    }
    h_hit_poly->SetStats( 0 );
    h_hit_poly->SetMinimum( 0. );
    h_hit_poly->SetMaximum( 200. );
    top_dir->Add( h_hit_poly );
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "HTOF_multiplicity";
    Int_t target_id = getUniqueID(kHTOF, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegHTOF, 0, NumOfSegHTOF,
			   "Multiplicity", ""));
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createBH2_E42
// -------------------------------------------------------------------------
// TList* HistMaker::createBH2_E42( Bool_t flag_ps )
// {
//   TString strDet = CONV_STRING(kBH2_E42);   // Determine the detector name
//   name_created_detectors_.push_back(strDet);    // name list of crearted detector
//   if(flag_ps) name_ps_files_.push_back(strDet); // name list which are displayed in Ps tab

//   const char* nameDetector = strDet.Data();
//   TList *top_dir = new TList;
//   top_dir->SetName(nameDetector);

//   { // ADC---------------------------------------------------------
//     TString strSubDir  = CONV_STRING(kADC); // Declaration of the sub-directory
//     const char* nameSubDir = strSubDir.Data();
//     TList *sub_dir = new TList;
//     sub_dir->SetName(nameSubDir);

//     Int_t target_id = getUniqueID(kBH2_E42, 0, kADC, 0);
//     for(Int_t i = 0; i<NumOfSegBH2_E42*2; ++i){
//       const char* title = NULL;
//       if(i < NumOfSegBH2_E42){
// 	Int_t seg = i+1; // 1 origin
// 	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
//       }else{
// 	Int_t seg = i+1-NumOfSegBH2_E42; // 1 origin
// 	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
//       }
//       sub_dir->Add(createTH1(++target_id, title, // 1 origin
// 			     0x1000, 0, 0x1000,
// 			     "ADC [ch]", ""));
//     }
//     top_dir->Add(sub_dir);
//   }

//   // ADC w/TDC ---------------------------------------------------------
//   {
//     // Declaration of the sub-directory
//     TString strSubDir  = CONV_STRING(kADCwTDC);
//     const char* nameSubDir = strSubDir.Data();
//     TList *sub_dir = new TList;
//     sub_dir->SetName(nameSubDir);

//     // Make histogram and add it
//     Int_t target_id = getUniqueID(kBH2_E42, 0, kADCwTDC, 0);
//     for( Int_t i=0; i<NumOfSegBH2_E42*2; ++i ){
//       const char* title = NULL;
//       if( i<NumOfSegBH2_E42 ){
// 	Int_t seg = i +1; // 1 origin
// 	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
//       }else{
// 	Int_t seg = i +1 -NumOfSegBH2_E42; // 1 origin
// 	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
//       }

//       sub_dir->Add(createTH1(++target_id, title, // 1 origin
// 			     0x1000, 0, 0x1000,
// 			     "ADC [ch]", ""));
//     }

//     // insert sub directory
//     top_dir->Add(sub_dir);
//   }

//   // TDC ----------------------------------------------------
//   {
//     // Declaration of the sub-directory
//     TString strSubDir  = CONV_STRING(kTDC);
//     const char* nameSubDir = strSubDir.Data();
//     TList *sub_dir = new TList;
//     sub_dir->SetName(nameSubDir);

//     // Make histogram and add it
//     Int_t target_id = getUniqueID(kBH2_E42, 0, kTDC, 0);
//     for(Int_t i = 0; i<NumOfSegBH2_E42*2; ++i){
//       const char* title = NULL;
//       if(i < NumOfSegBH2_E42){
// 	Int_t seg = i+1; // 1 origin
// 	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
//       }else{
// 	Int_t seg = i+1-NumOfSegBH2_E42; // 1 origin
// 	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
//       }

//       sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
// 			     //			     10000, 0, 400000,
// 			     50000, 0, 2000000,
// 			     "TDC [ch]", ""));
//     }

//     // insert sub directory
//     top_dir->Add(sub_dir);
//   }

//   // MeanTimer (FPGA)----------------------------------------------------
//   {
//     // Declaration of the sub-directory
//     TString strSubDir  = "FPGA_TDC_MeanTimer";
//     const char* nameSubDir = strSubDir.Data();
//     TList *sub_dir = new TList;
//     sub_dir->SetName(nameSubDir);

//     // Make histogram and add it
//     Int_t target_id = getUniqueID(kBH2_E42, 0, kBH2_E42MT, 0);
//     for(Int_t i = 0; i<NumOfSegBH2_E42; ++i){
//       const char* title = NULL;
//       Int_t seg = i+1; // 1 origin
//       title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);

//       sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
// 			     //			     10000, 0, 400000,
// 			     50000, 0, 2000000,
// 			     "TDC [ch]", ""));
//     }

//     // insert sub directory
//     top_dir->Add(sub_dir);
//   }

//   // Hit parttern -----------------------------------------------
//   {

//     TString strSubDir  = CONV_STRING(kHitPat);
//     const char* nameSubDir = strSubDir.Data();
//     TList *sub_dir = new TList;
//     sub_dir->SetName(nameSubDir);

//     Int_t target_id = getUniqueID(kBH2_E42, 0, kHitPat, 0);
//     TString Trig_flag[3] = {"No flag", "K-Beam", "pi-Beam"};
//     TString Hitpat_flag[3] = {"Unbias", "Corrected", "MaxADC"};
//     const char* Tf_name = NULL;
//     const char* Hp_name = NULL;

//     for(Int_t i = 0; i<3; ++i){//Trig flag
//       for(Int_t j = 0; j<3; ++j){//hitpat
// 	const char* title = NULL;
// 	Int_t ID = i*3+j+1; // 1 origin
// 	Tf_name = Trig_flag[i].Data();
// 	Hp_name = Hitpat_flag[j].Data();

// 	title = Form("%s_%s_%s)", nameSubDir, Tf_name, Hp_name);


// 	sub_dir->Add(createTH1(target_id + ID, title, // 1 origin
// 			       NumOfSegBH2_E42, 0, NumOfSegBH2_E42,
// 			       "Segment", ""));
//       }
//     }
//     // insert sub directory
//     top_dir->Add(sub_dir);
//   }

//   // Multiplicity -----------------------------------------------
//   {
//     TString strSubDir  = CONV_STRING(kMulti);
//     const char* nameSubDir = strSubDir.Data();
//     TList *sub_dir = new TList;
//     sub_dir->SetName(nameSubDir);

//     Int_t target_id = getUniqueID(kBH2_E42, 0, kMulti, 0);
//     TString Trig_flag[3] = {"No flag", "K-Beam", "pi-Beam"};
//     TString Hitpat_flag[2] = {"Unbias", "Corrected"};
//     const char* Tf_name = NULL;
//     const char* Hp_name = NULL;

//     for(Int_t i = 0; i<3; ++i){//Trig flag
//       for(Int_t j = 0; j<2; ++j){//Multi
// 	const char* title = NULL;
// 	Int_t ID = i*2+j+1; // 1 origin
// 	Tf_name = Trig_flag[i].Data();
// 	Hp_name = Hitpat_flag[j].Data();

// 	title = Form("%s_%s_%s)", nameSubDir, Tf_name, Hp_name);


// 	sub_dir->Add(createTH1(target_id + ID, title, // 1 origin
// 			       NumOfSegBH2_E42, 0, NumOfSegBH2_E42,
// 			       "Multiplicity", ""));
//       }
//     }
//     // insert sub directory
//     top_dir->Add(sub_dir);
//   }

//   return top_dir;
// }


// -------------------------------------------------------------------------
// createBH2_E07
// -------------------------------------------------------------------------
TList* HistMaker::createBH2_E07( Bool_t flag_ps )
{
  TString strDet = CONV_STRING(kBH2_E07);   // Determine the detector name
  name_created_detectors_.push_back(strDet);    // name list of crearted detector
  if(flag_ps) name_ps_files_.push_back(strDet); // name list which are displayed in Ps tab

  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  { // ADC---------------------------------------------------------
    TString strSubDir  = CONV_STRING(kADC); // Declaration of the sub-directory
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kBH2_E07, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfSegBH2_E07*2; ++i){
      const char* title = NULL;
      if(i < NumOfSegBH2_E07){
	Int_t seg = i+1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i+1-NumOfSegBH2_E07; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  { // TDC---------------------------------------------------------
    TString strSubDir  = CONV_STRING(kTDC); // Declaration of the sub-directory
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kBH2_E07, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegBH2_E07*2; ++i){
      const char* title = NULL;
      if(i < NumOfSegBH2_E07){
	Int_t seg = i+1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i+1-NumOfSegBH2_E07; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  { // Hit parttern -----------------------------------------------
    const char* title = "BH2_E07_hit_pattern";
    Int_t target_id = getUniqueID(kBH2_E07, 0, kHitPat, 0);
    top_dir->Add(createTH1(++target_id, title, // 1 origin
			   NumOfSegBH2_E07, 0, NumOfSegBH2_E07,
			   "Segment", ""));
  }

  { // Multiplicity -----------------------------------------------
    const char* title = "BH2_E07_multiplicity";
    Int_t target_id = getUniqueID(kBH2_E07, 0, kMulti, 0);
    top_dir->Add(createTH1(++target_id, title, // 1 origin
			   NumOfSegBH2_E07+1, 0, NumOfSegBH2_E07+1,
			   "Multiplicity", ""));
  }

  return top_dir;
}

// -------------------------------------------------------------------------
// createAFT
// -------------------------------------------------------------------------
TList* HistMaker::createAFT( Bool_t flag_ps )
{

  // Determine the detector name
  TString strDet = CONV_STRING(kAFT);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kAFT, 0, kTDC, 0);
    const char* sub_name = "TDC";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
	if( ud == 0 ) title = Form("%s_%s_%dU_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s", nameDetector, sub_name, i/4+1, layer_name.Data());

    	sub_dir->Add(createTH1( ++target_id, title , // 1 origin
    				4096, 0, 4096,
    				"TDC [ch]", ""));
      }
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kAFT, 0, kTOT, 0);
    const char* sub_name = "TOT";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
    	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s", nameDetector, sub_name, i/4+1, layer_name.Data());

    	sub_dir->Add(createTH1( ++target_id, title , // 1 origin
    				150, 0, 150,
    				"TOT [ch]", ""));
      }
    }

    // draw TOT for each MPPC vias
    std::vector<TString> name = {
      "AFT_TOT_X-U", "AFT_TOT_X-D",
      "AFT_TOT_Y-U", "AFT_TOT_Y-D"
    };
    for(Int_t i=0, n=name.size(); i<n; ++i){
      sub_dir->Add(createTH1(++target_id, name[i], 150, 0, 150));
    }

  // CTOT---------------------------------------------------------
    target_id = getUniqueID(kAFT, 0, kTOT, 100);
    sub_name = "CTOT";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
    	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s", nameDetector, sub_name, i/4+1, layer_name.Data());

    	sub_dir->Add(createTH1( ++target_id, title , // 1 origin
    				150, 0, 150,
    				"TOT [ch]", ""));
      }
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC HighGain------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kHighGain);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kAFT, 0, kHighGain, 0);
    const char* sub_name = "HG";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
    	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s", nameDetector, sub_name, i/4+1, layer_name.Data());

    	sub_dir->Add(createTH1( ++target_id, title , // 1 origin
  				4096, 0, 4096,
  				"ADC [ch]", ""));
      }
    }

    // ADC HighGain Cut -------------------------------------------
    target_id = getUniqueID(kAFT, 0, kHighGain, 100);
    sub_name = "CHG";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
    	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s", nameDetector, sub_name, i/4+1, layer_name.Data());

    	sub_dir->Add(createTH1( ++target_id, title , // 1 origin
  				4096, 0, 4096,
  				"ADC [ch]", ""));
      }
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC LowGain ------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kLowGain);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kAFT, 0, kLowGain , 0);
    const char* sub_name = "LG";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
    	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s", nameDetector, sub_name, i/4+1, layer_name.Data());

  	sub_dir->Add(createTH1( ++target_id, title , // 1 origin
  				4096, 0, 4096,
  				"ADC [ch]", ""));
      }
    }

  // ADC LowGain Cut --------------------------------------------
    target_id = getUniqueID(kAFT, 0, kLowGain , 100);
    sub_name = "CLG";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
    	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s", nameDetector, sub_name, i/4+1, layer_name.Data());

  	sub_dir->Add(createTH1( ++target_id, title , // 1 origin
  				4096, 0, 4096,
  				"ADC [ch]", ""));
      }
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // // ADC Pedestal -----------------------------------------------
  // {
  //   TString strSubDir  = CONV_STRING(kPedestal);
  //   const char* nameSubDir = strSubDir.Data();
  //   TList *sub_dir = new TList;
  //   sub_dir->SetName(nameSubDir);

  //   Int_t target_id = getUniqueID(kAFT, 0, kPede, 0);
  //   const char* sub_name = "Pedestal";
  //   // Add to the top directory
  //   for(Int_t ud=0; ud<2; ud++){
  //     for(Int_t i=0; i<NumOfPlaneAFT; ++i){
  //   	const char* title = NULL;
  // 	const TString layer_name = NameOfPlaneAFT[i%4];
  //   	if( ud == 0 ) title = Form("%s_%s_%dU_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
  //   	if( ud == 1 ) title = Form("%s_%s_%dD_%s", nameDetector, sub_name, i/4+1, layer_name.Data());

  // 	sub_dir->Add(createTH1( ++target_id, title , // 1 origin
  // 				1024, -200, 824,
  // 				"ADC [ch]", ""));
  //     }
  //   }
  //   // insert sub directory
  //   top_dir->Add(sub_dir);
  // }

  // TDC-2D -------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kAFT, 0, kTDC2D  , 0);
    const char* sub_name = "TDC";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
  	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
  	if( ud == 0 ) title = Form("%s_%s_%dU_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());
  	if( ud == 1 ) title = Form("%s_%s_%dD_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());

  	Int_t aft_nseg = NumOfSegAFT[i%4];
  	sub_dir->Add(createTH2(++target_id, title, // 1 origin
  			       aft_nseg, 0, aft_nseg,
  			       4096, 0, 4096,
  			       "Fiber", "TDC [ch]"));
      }
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT-2D --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTOT2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kAFT, 0, kTOT2D, 0);
    const char* sub_name = "TOT";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
  	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());

  	Int_t aft_nseg = NumOfSegAFT[i%4];
  	sub_dir->Add(createTH2(++target_id, title, // 1 origin
  			       aft_nseg, 0, aft_nseg,
  			       150, 0, 150,
  			       "Fiber", "TOT [ch]"));
      }
    }

    // CTOT-2D (after cut) --------------------------------------------
    target_id = getUniqueID(kAFT, 0, kTOT2D, 100);
    sub_name = "CTOT";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
  	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());

  	Int_t aft_nseg = NumOfSegAFT[i%4];
  	sub_dir->Add(createTH2(++target_id, title, // 1 origin
  			       aft_nseg, 0, aft_nseg,
  			       150, 0, 150,
  			       "Fiber", "TOT [ch]"));

      }
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC-2D HighGain ----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kHighGain_2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kAFT, 0, kHighGain2D, 0);
    const char* sub_name = "HG";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
  	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());

  	Int_t aft_nseg = NumOfSegAFT[i%4];
  	sub_dir->Add(createTH2(++target_id, title, // 1 origin
  			       aft_nseg, 0, aft_nseg,
  			       // 4096/8, 0, 4096,
  			       4096, 0, 4096,
  			       "Fiber", "ADC [ch]"));
      }
    }

    // ADC-2D HighGain Cut ------------------------------------------
    target_id = getUniqueID(kAFT, 0, kHighGain2D, 100);
    sub_name = "CHG";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
  	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());

  	Int_t aft_nseg = NumOfSegAFT[i%4];
  	sub_dir->Add(createTH2(++target_id, title, // 1 origin
  			       aft_nseg, 0, aft_nseg,
  			       // 4096/8, 0, 4096,
  			       4096, 0, 4096,
  			       "Fiber", "ADC [ch]"));
      }
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC-2D LowGain -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kLowGain_2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kAFT, 0, kLowGain2D, 0);
    const char* sub_name = "LG";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
  	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());

  	Int_t aft_nseg = NumOfSegAFT[i%4];
  	sub_dir->Add(createTH2(++target_id, title, // 1 origin
  			       aft_nseg, 0, aft_nseg,
  			       4096/8, 0, 4096,
  			       "Fiber", "ADC [ch]"));
      }
    }

    // ADC-2D LowGain Cut -------------------------------------------
    target_id = getUniqueID(kAFT, 0, kLowGain2D, 100);
    sub_name = "CLG";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
  	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());

  	Int_t aft_nseg = NumOfSegAFT[i%4];
  	sub_dir->Add(createTH2(++target_id, title, // 1 origin
  			       aft_nseg, 0, aft_nseg,
  			       4096/8, 0, 4096,
  			       "Fiber", "ADC [ch]"));
      }
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // // ADC-2D Pedestal ----------------------------------------------
  // {
  //   TString strSubDir  = CONV_STRING(kPedestal_2D);
  //   const char* nameSubDir = strSubDir.Data();
  //   TList *sub_dir = new TList;
  //   sub_dir->SetName(nameSubDir);

  //   Int_t target_id = getUniqueID(kAFT, 0, kPede2D, 0);
  //   const char* sub_name = "Pedestal";
  //   // Add to the top directory
  //   for(Int_t ud=0; ud<2; ud++){
  //     for(Int_t i=0; i<NumOfPlaneAFT; ++i){
  // 	const char* title = NULL;
  // 	const TString layer_name = NameOfPlaneAFT[i%4];
  //   	if( ud == 0 ) title = Form("%s_%s_%dU_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());
  //   	if( ud == 1 ) title = Form("%s_%s_%dD_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());

  // 	Int_t aft_nseg = NumOfSegAFT[i%4];
  // 	sub_dir->Add(createTH2(++target_id, title, // 1 origin
  // 			       aft_nseg, 0, aft_nseg,
  // 			       4096/8,-200, 824,
  // 			       "Fiber", "ADC [ch]"));
  //     }
  //   }
  //   // insert sub directory
  //   top_dir->Add(sub_dir);
  // }

  // TOT * TDC -----------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTOTXTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kAFT, 0, kTOTXTDC, 0);
    const char* sub_name = "TOTXTDC";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
  	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());

  	sub_dir->Add(createTH2( ++target_id, title , // 1 origin
				150, 0, 150,
				4096, 0, 4096,
				"TOT [ch]", "TDC [ch]"));
      }
    }

    { //TOT * TDC segment by segment for AFT correction
      const char* title = NULL;
      std::vector<Int_t> planeArray{0};
      for( Int_t i = 0, n = planeArray.size(); i < n; i++ ){
	const TString layer_name_x = NameOfPlaneAFT[0];
	const TString layer_name_y = NameOfPlaneAFT[2];
	title = Form("%s_%s_%dU_%s_seg%d_2D", nameDetector, sub_name, 1, layer_name_x.Data(), 0);
	sub_dir->Add(createTH2( ++target_id, title ,
				150, 0, 150,
				4096, 0, 4096,
				"TOT [ch]", "TDC [ch]"));
	title = Form("%s_%s_%dU_%s_seg%d_2D", nameDetector, sub_name, 1, layer_name_x.Data(), 31);
	sub_dir->Add(createTH2( ++target_id, title ,
				150, 0, 150,
				4096, 0, 4096,
				"TOT [ch]", "TDC [ch]"));
	title = Form("%s_%s_%dU_%s_seg%d_2D", nameDetector, sub_name, 1, layer_name_y.Data(), 0);
	sub_dir->Add(createTH2( ++target_id, title ,
				150, 0, 150,
				4096, 0, 4096,
				"TOT [ch]", "TDC [ch]"));
	title = Form("%s_%s_%dU_%s_seg%d_2D", nameDetector, sub_name, 1, layer_name_y.Data(), 8);
	sub_dir->Add(createTH2( ++target_id, title ,
				150, 0, 150,
				4096, 0, 4096,
				"TOT [ch]", "TDC [ch]"));
      }
    }

    { //ADC * TDC segment by segment for AFT correction
      const char* title = NULL;
      std::vector<Int_t> planeArray{0, 4, 8};
      for( Int_t i = 0, n = planeArray.size(); i < n; i++ ){
	const TString layer_name_x = NameOfPlaneAFT[0];
	const TString layer_name_y = NameOfPlaneAFT[2];
	title = Form("%s_%s_%dU_%s_seg%d_2D", nameDetector, "ADCXTDC", planeArray[i]+1, layer_name_x.Data(), 0);
	sub_dir->Add(createTH2( ++target_id, title ,
				64, 0, 4096,
				4096, 0, 4096,
				"TDC [ch]", "ADC [ch]"));
	title = Form("%s_%s_%dU_%s_seg%d_2D", nameDetector, "ADCXTDC", planeArray[i]+1, layer_name_x.Data(), 15);
	sub_dir->Add(createTH2( ++target_id, title ,
				64, 0, 4096,
				4096, 0, 4096,
				"TDC [ch]", "ADC [ch]"));
	title = Form("%s_%s_%dU_%s_seg%d_2D", nameDetector, "ADCXTDC", planeArray[i]+1, layer_name_x.Data(), 31);
	sub_dir->Add(createTH2( ++target_id, title ,
				64, 0, 4096,
				4096, 0, 4096,
				"TDC [ch]", "ADC [ch]"));
	title = Form("%s_%s_%dU_%s_seg%d_2D", nameDetector, "ADCXTDC", planeArray[i]+1, layer_name_y.Data(), 0);
	sub_dir->Add(createTH2( ++target_id, title ,
				64, 0, 4096,
				4096, 0, 4096,
				"TDC [ch]", "ADC [ch]"));
	title = Form("%s_%s_%dU_%s_seg%d_2D", nameDetector, "ADCXTDC", planeArray[i]+1, layer_name_y.Data(), 8);
	sub_dir->Add(createTH2( ++target_id, title ,
				64, 0, 4096,
				4096, 0, 4096,
				"TDC [ch]", "ADC [ch]"));
	title = Form("%s_%s_%dU_%s_seg%d_2D", nameDetector, "ADCXTDC", planeArray[i]+1, layer_name_y.Data(), 15);
	sub_dir->Add(createTH2( ++target_id, title ,
				64, 0, 4096,
				4096, 0, 4096,
				"TDC [ch]", "ADC [ch]"));
      }
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC HighGain * TDC -----------------------------------------
  {
    TString strSubDir  = CONV_STRING(kHighGainXTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kAFT, 0, kHighGainXTDC, 0);
    const char* sub_name = "HGXTDC";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
  	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());

  	sub_dir->Add(createTH2( ++target_id, title , // 1 origin
  			      4096/8, 0, 4096,
  			      // 1024, 0, 1024,
  			      4096, 0, 4096,
  			      "TDC [ch]", "HighGain [ch]"));
      }
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC HighGain * TOT -----------------------------------------
  {
    TString strSubDir  = CONV_STRING(kHighGainXTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kAFT, 0, kHighGainXTOT, 0);
    const char* sub_name = "HGXTOT";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
  	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());

  	sub_dir->Add(createTH2( ++target_id, title , // 1 origin
  			      4096/8, 0, 4096,
  			      150, 0, 150,
  			      "HighGain [ch]", "TOT [ch]"));
      }
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADC LowGain * TOT -----------------------------------------
  {
    TString strSubDir  = CONV_STRING(kLowGainXTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kAFT, 0, kLowGainXTOT, 0);
    const char* sub_name = "LGXTOT";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
  	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());

  	sub_dir->Add(createTH2( ++target_id, title , // 1 origin
  			      4096/8, 0, 4096,
  			      150, 0, 150,
  			      "LowGain [ch]", "TOT [ch]"));
      }
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kHitPattern);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kAFT, 0, kHitPat, 0);
    const char* sub_name = "HitPat";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
    	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s", nameDetector, sub_name, i/4+1, layer_name.Data());

  	Int_t aft_nseg = NumOfSegAFT[i%4];
  	sub_dir->Add(createTH1( ++target_id, title, // 1 origin
  				aft_nseg, 0, aft_nseg,
  				"Fiber", ""));
      }
    }

    // Hit parttern (after cut) -----------------------------------
    target_id = getUniqueID(kAFT, 0, kHitPat, 100);
    sub_name = "CHitPat";
    // Add to the top directory
    for(Int_t ud=0; ud<3; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
    	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
  	else if( ud == 2 ) title = Form("%s_%s_%d_%s",  nameDetector, sub_name, i/4+1, layer_name.Data()); // for hitpattern defined by hits on both edges

  	Int_t aft_nseg = NumOfSegAFT[i%4];
  	sub_dir->Add(createTH1( ++target_id, title, // 1 origin
  				aft_nseg, 0, aft_nseg,
  				"Fiber", ""));
      }
    }

    // 2D Hit parttern (after cut) -----------------------------------
    const char* title_x = Form("%s_%sPoly_X_2D", nameDetector, sub_name);
    const char* title_y = Form("%s_%sPoly_Y_2D", nameDetector, sub_name);
    auto h_hit_poly_x = dynamic_cast<TH2Poly*>( createTH2Poly( ++target_id, title_x, -15, 125, -70, 70 ) );
    auto h_hit_poly_y = dynamic_cast<TH2Poly*>( createTH2Poly( ++target_id, title_y, -15, 125, -35, 35 ) );
    const double phi   = gAftHelper.GetPhi();
    const int    npoly = gAftHelper.GetNPoly();
    double X[npoly], Z[npoly];
    for( int iPlane = 0; iPlane < NumOfPlaneAFT; iPlane++ ){
      for( int iSeg = 0; iSeg < NumOfSegAFT[iPlane%4]; iSeg++ ){
	double posx = gAftHelper.GetX( iPlane, iSeg );
	double posz = gAftHelper.GetZ( iPlane, iSeg );
	for( int ipoly = 0; ipoly < npoly; ipoly++ ){
	  X[ipoly] = posx + phi/2.*TMath::Cos(ipoly*2*TMath::Pi()/npoly);
	  Z[ipoly] = posz + phi/2.*TMath::Sin(ipoly*2*TMath::Pi()/npoly);
	}
	if( iPlane%4 == 0 || iPlane%4 == 1 ) h_hit_poly_x->AddBin(npoly, Z, X);
	if( iPlane%4 == 2 || iPlane%4 == 3 ) h_hit_poly_y->AddBin(npoly, Z, X);
      }
    }

    h_hit_poly_x->SetStats( 0 );
    h_hit_poly_y->SetStats( 0 );
    h_hit_poly_x->SetMinimum( 0. );
    h_hit_poly_y->SetMinimum( 0. );
    sub_dir->Add( h_hit_poly_x );
    sub_dir->Add( h_hit_poly_y );

    //with adc
    sub_name = "CHitPat_wa";
    // Add to the top directory
    for(Int_t ud=0; ud<3; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
    	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
  	else if( ud == 2 ) title = Form("%s_%s_%d_%s",  nameDetector, sub_name, i/4+1, layer_name.Data()); // for hitpattern defined by hits on both edges

  	Int_t aft_nseg = NumOfSegAFT[i%4];
  	sub_dir->Add(createTH1( ++target_id, title, // 1 origin
  				aft_nseg, 0, aft_nseg,
  				"Fiber", ""));
      }
    }
    //2D Hit pattern

    //correlation between bh2 and AFT X0 seg (to check event slip)
    sub_name = "Correlation_bh2_AFT_X0";
    // Add to the top directory
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
	if(i!=0){continue;}
	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
	// if( ud == 0 ) title = Form("%s_%s_%dU_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());
	// if( ud == 1 ) title = Form("%s_%s_%dD_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());
	title = Form("BH2_AFT%s_%s_%d_%s_cor", nameDetector, sub_name, i/4+1, layer_name.Data());
	Int_t aft_nseg = NumOfSegAFT[i%4];
	if(i==0){
	  sub_dir->Add(createTH2(++target_id, title, // 1 origin
				 aft_nseg, 0, aft_nseg,
				 // 4096/8, 0, 4096,
				 NumOfSegBH2, 0, NumOfSegBH2,
				 "Fiber", "BH2 [seg]"));
	}
     }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kMultiplicity);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kAFT, 0, kMulti, 0);
    const char* sub_name = "Multi";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneAFT; ++i){
      const TString layer_name = NameOfPlaneAFT[i%4];
      const char* title = Form("%s_%s_%d_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
      Int_t aft_nseg = NumOfSegAFT[i%4];
      sub_dir->Add(createTH1( ++target_id, title, // 1 origin
			      aft_nseg+1, 0, aft_nseg+1,
			      "Fiber", ""));
    }
    for(Int_t i=0; i<NumOfPlaneAFT/2; ++i){
      const TString layer_name = (i%2 == 0) ? "X" : "Y";
      const char* title = Form("%s_%s_%d_%s", nameDetector, sub_name, i/2+1, layer_name.Data());
      Int_t aft_nseg = 2*NumOfSegAFT[(2*i)%4];
      sub_dir->Add(createTH1( ++target_id, title, // 1 origin
			      aft_nseg+1, 0, aft_nseg+1,
			      "Fiber", ""));
    }
    sub_name = "AllMulti";
    for(Int_t i=0; i<NumOfPlaneAFT/2; ++i){
      const TString layer_name = (i%2 == 0) ? "X" : "Y";
      const char* title = Form("%s_%s_%d_%s", nameDetector, sub_name, i/2+1, layer_name.Data());
      Int_t aft_nseg = 2*NumOfSegAFT[(2*i)%4];
      sub_dir->Add(createTH1( ++target_id, title, // 1 origin
			      aft_nseg+1, 0, aft_nseg+1,
			      "Fiber", ""));
    }
    // Multi with HG
    sub_name = "CMulti";
    for(Int_t i=0; i<NumOfPlaneAFT; ++i){
      const TString layer_name = NameOfPlaneAFT[i%4];
      const char* title = Form("%s_%s_%d_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
      Int_t aft_nseg = NumOfSegAFT[i%4];
      sub_dir->Add(createTH1( ++target_id, title, // 1 origin
			      aft_nseg+1, 0, aft_nseg+1,
			      "Fiber", ""));
    }
    for(Int_t i=0; i<NumOfPlaneAFT/2; ++i){
      const TString layer_name = (i%2 == 0) ? "X" : "Y";
      const char* title = Form("%s_%s_%d_%s", nameDetector, sub_name, i/2+1, layer_name.Data());
      Int_t aft_nseg = 2*NumOfSegAFT[(2*i)%4];
      sub_dir->Add(createTH1( ++target_id, title, // 1 origin
			      aft_nseg+1, 0, aft_nseg+1,
			      "Fiber", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  //cluster TDC -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kCluster_tdc);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kAFT, kCluster, kTDC, 0);
    const char* sub_name = "ClusterTDC";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
    	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s", nameDetector, sub_name, i/4+1, layer_name.Data());

  	sub_dir->Add(createTH1( ++target_id, title , // 1 origin
  				4096, 0, 4096,
  				"TDC [ch]", ""));
      }
    }

    target_id = getUniqueID(kAFT, kCluster, kTDC2D, 0);
    sub_name = "seg-TDC maxseg";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
    	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());

  	Int_t aft_nseg = NumOfSegAFT[i%4];
  	sub_dir->Add(createTH2(++target_id, title, // 1 origin
  			       aft_nseg, 0, aft_nseg,
  			       4096, 0, 4096,
  			       "Fiber", "TDC [ch]"));
      }
    }
    top_dir->Add(sub_dir);
  }

  //cluster ADC HighGain -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kCluster_hgadc);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // ADC Cluster HighGain -------------------------------------------
    Int_t target_id = getUniqueID(kAFT, kCluster, kHighGain, 0);
    const char* sub_name = "ClusterHG";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
    	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s", nameDetector, sub_name, i/4+1, layer_name.Data());

  	sub_dir->Add(createTH1( ++target_id, title , // 1 origin
  			      4096, 0, 4096,
  			      "ADC [ch]", ""));
      }
    }

    target_id = getUniqueID(kAFT, kCluster, kHighGain2D, 100);
    sub_name = "seg-MaxHG";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
    	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());

  	Int_t aft_nseg = NumOfSegAFT[i%4];
  	sub_dir->Add(createTH2( ++target_id, title , // 1 origin
  				aft_nseg, 0, aft_nseg,
  				4096/8, 0, 4096,
  				"Fiber", "ADC [ch]"));
      }
    }
    top_dir->Add(sub_dir);
  }

  //cluster ADC LowGain -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kCluster_lgadc);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // ADC Cluster LowGain -------------------------------------------
    Int_t target_id = getUniqueID(kAFT, kCluster, kLowGain, 0);
    const char* sub_name = "ClusterLG";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s", nameDetector, sub_name, i/4+1, layer_name.Data());

  	sub_dir->Add(createTH1( ++target_id, title , // 1 origin
  			      4096, 0, 4096,
  			      "ADC [ch]", ""));
      }
    }

    target_id = getUniqueID(kAFT, kCluster, kLowGain, 100);
    sub_name = "seg-MaxLG";
    // Add to the top directory
    for(Int_t ud=0; ud<2; ud++){
      for(Int_t i=0; i<NumOfPlaneAFT; ++i){
    	const char* title = NULL;
	const TString layer_name = NameOfPlaneAFT[i%4];
    	if( ud == 0 ) title = Form("%s_%s_%dU_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());
    	if( ud == 1 ) title = Form("%s_%s_%dD_%s_2D", nameDetector, sub_name, i/4+1, layer_name.Data());

  	Int_t aft_nseg = NumOfSegAFT[i%4];
  	sub_dir->Add(createTH2( ++target_id, title , // 1 origin
  				aft_nseg, 0, aft_nseg,
  				4096/8, 0, 4096,
  				"Fiber", "ADC [ch]"));
      }
    }
    top_dir->Add(sub_dir);
  }


  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createBAC_E07
// -------------------------------------------------------------------------
TList* HistMaker::createBAC_E07( Bool_t flag_ps )
{
  TString strDet = CONV_STRING(kBAC_E07);   // Determine the detector name
  name_created_detectors_.push_back(strDet);    // name list of crearted detector
  if(flag_ps) name_ps_files_.push_back(strDet); // name list which are displayed in Ps tab

  const char* nameDetector = strDet.Data(); // Declaration of the directory
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  const char* name_acs[NumOfSegBAC_E07] = {
					   "BAC1-SUM", "BAC2-SUM", "BAC1-1", "BAC1-2", "BAC2-1", "BAC2-2"
  };

  { // ADC---------------------------------------------------------
    TString strSubDir  = CONV_STRING(kADC); // Declaration of the sub-directory
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kBAC_E07, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfSegBAC_E07; ++i){
      const char* title = NULL;
      title = Form("%s_%s", name_acs[i], nameSubDir);
      sub_dir->Add(createTH1( ++target_id, title, // 1 origin
			      0x1000, 0, 0x1000,
			      "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  { // TDC---------------------------------------------------------
    TString strSubDir  = CONV_STRING(kTDC); // Declaration of the sub-directory
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kBAC_E07, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegBAC_E07; ++i){
      const char* title = NULL;
      title = Form("%s_%s", name_acs[i], nameSubDir);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  { // Hit parttern -----------------------------------------------
    Int_t target_id = getUniqueID(kBAC_E07, 0, kHitPat, 0);
    top_dir->Add(createTH1(++target_id, "BAC_E07_hit_pattern",
			   NumOfSegBAC_E07, 0, NumOfSegBAC_E07,
			   "Segment", ""));
  }

  { // Multiplicity -----------------------------------------------
    Int_t target_id = getUniqueID(kBAC_E07, 0, kMulti, 0);
    top_dir->Add(createTH1(++target_id, "BAC_E07_multiplicity",
			   NumOfSegBAC_E07+1, 0, NumOfSegBAC_E07+1,
			   "Multiplicity", ""));
  }

  return top_dir;
}

// -------------------------------------------------------------------------
// createFBH
// -------------------------------------------------------------------------
TList* HistMaker::createFBH( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kFBH);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }
  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // TDC---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kFBH, 0, kTDC, 0);
    for(Int_t i=0; i<NumOfSegFBH*2; ++i){
      if( i<NumOfSegFBH ){
	sub_dir->Add(createTH1(++target_id, Form("%s_%s_%dU", // 1 origin
						 nameDetector, nameSubDir, i+1),
			       1024, 0, 1024,
			       "TDC [ch]", ""));
      } else {
	sub_dir->Add(createTH1(++target_id, Form("%s_%s_%dD", // 1 origin
						 nameDetector, nameSubDir, i+1-NumOfSegFBH),
			       1024, 0, 1024,
			       "TDC [ch]", ""));
      }
    }
    top_dir->Add(sub_dir);
  }
  // TOT---------------------------------------------------------
  {
    const char* nameSubDir = "TOT";
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kFBH, 0, kADC, 0);
    for(Int_t i=0; i<NumOfSegFBH*2; ++i){
      if(i<NumOfSegFBH){
	sub_dir->Add(createTH1(++target_id, Form("%s_%s_%dU", // 1 origin
						 nameDetector, nameSubDir, i+1),
			       200, -50, 150,
			       "TOT [ch]", ""));
      }else{
	sub_dir->Add(createTH1(++target_id, Form("%s_%s_%dD", // 1 origin
						 nameDetector, nameSubDir, i+1-NumOfSegFBH),
			       200, -50, 150,
			       "TOT [ch]", ""));
      }
    }
    top_dir->Add(sub_dir);
  }
  // TDC/TOT SUM -----------------------------------------------------
  {
    // TDC
    top_dir->Add(createTH1(getUniqueID(kFBH, 0, kTDC, NumOfSegFBH*2+1),
			   Form("%s_TDC", nameDetector),
			   1024, 0, 1024,
			   "TDC [ch]", ""));
    // TOT
    top_dir->Add(createTH1(getUniqueID(kFBH, 0, kADC, NumOfSegFBH*2+1),
			   Form("%s_TOT", nameDetector),
			   200, -50, 150,
			   "TOT [ch]", ""));
  }
  // TDC-2D --------------------------------------------
  {
    Int_t target_id = getUniqueID(kFBH, 0, kTDC2D, 0);
    top_dir->Add(createTH2(++target_id, "FBH_TDC2D", // 1 origin
			   NumOfSegFBH*2, 0, NumOfSegFBH*2,
			   1024, 0, 1024,
			   "Segment", "TDC [ch]"));
  }
  // TOT-2D --------------------------------------------
  {
    Int_t target_id = getUniqueID(kFBH, 0, kADC2D, 0);
    top_dir->Add(createTH2(++target_id, "FBH_TOT2D", // 1 origin
			   NumOfSegFBH*2, 0, NumOfSegFBH*2,
			   200, -50, 150,
			   "Segment", "TOT [ch]"));
  }
  // Hit parttern --------------------------------------------
  {
    Int_t target_id = getUniqueID(kFBH, 0, kHitPat, 0);
    top_dir->Add(createTH1(++target_id, "FBH_HitPat", // 1 origin
			   NumOfSegFBH, 0, NumOfSegFBH,
			   "Segment", ""));
  }
  // Multiplicity -----------------------------------------------
  {
    const char* title = "FBH_multiplicity";
    Int_t target_id = getUniqueID(kFBH, 0, kMulti, 0);
    top_dir->Add(createTH1(++target_id, title, // 1 origin
			   NumOfSegFBH, 0, NumOfSegFBH,
			   "Multiplicity", ""));
  }
  return top_dir;
}

// -------------------------------------------------------------------------
// createSSDT
// -------------------------------------------------------------------------
TList* HistMaker::createSSDT( Bool_t flag_ps )
{
  TString strDet = CONV_STRING(kSSDT);
  name_created_detectors_.push_back(strDet);
  if(flag_ps) name_ps_files_.push_back(strDet);

  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  { // TDC --------------------------------------------------------
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSDT, 0, kTDC, 0);
    for(Int_t seg=0; seg<NumOfSegSSDT; ++seg){
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_%s_%d", nameDetector, nameSubDir, seg+1),
			     0x1000, 0, 0x1000,
			     "Segment", ""));
    }
    top_dir->Add(sub_dir);
  }

  return top_dir;
}

// -------------------------------------------------------------------------
// createSSD0
// -------------------------------------------------------------------------
TList* HistMaker::createSSD0( Bool_t flag_ps )
{
  TString strDet = CONV_STRING(kSSD0);
  name_created_detectors_.push_back(strDet);
  if(flag_ps) name_ps_files_.push_back(strDet);

  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  const char* nameLayer[NumOfLayersSSD0] = { "x0", "y0" };

  // ADC --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD0, 0, kADC2D, 0);
    for(Int_t l=0; l<NumOfLayersSSD0; ++l){
      sub_dir->Add(createTH2(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD0, 0, NumOfSegSSD0, 0x3ff, 0, 0x3ff,
			     "Segment", "Peak Height  "));
    }
    top_dir->Add(sub_dir);
  }

  // TDC --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD0, 0, kTDC2D, 0);
    for(Int_t l=0; l<NumOfLayersSSD0; ++l){
      sub_dir->Add(createTH2(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD0, 0, NumOfSegSSD0, NumOfSamplesSSD+1, 0, NumOfSamplesSSD+1,
			     "Segment", "Peak Position  "));
    }
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD0, 0, kHitPat, 0);
    for(Int_t l=0; l<NumOfLayersSSD0; ++l){
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD0, 0, NumOfSegSSD0,
			     "Segment", ""));
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_C%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD0, 0, NumOfSegSSD0,
			     "Segment", ""));
    }
    target_id = getUniqueID(kSSD0, 0, kHitPat2D, 0);
    sub_dir->Add(createTH2(++target_id, Form("%s_%s_%s%%%s", nameDetector, nameSubDir,
					     nameLayer[1], nameLayer[0]),
			   NumOfSegSSD0/6, 0, NumOfSegSSD0, NumOfSegSSD0/6, 0, NumOfSegSSD0,
			   nameLayer[0], nameLayer[1]));
    // sub_dir->Add(createTH2(++target_id, Form("%s_C%s_%s%%%s", nameDetector, nameSubDir,
    // 					     nameLayer[1], nameLayer[0]),
    // 			   NumOfSegSSD0/6, 0, NumOfSegSSD0, NumOfSegSSD0/6, 0, NumOfSegSSD0,
    // 			   nameLayer[0], nameLayer[1]));
    top_dir->Add(sub_dir);
  }

  { // Multiplicity -----------------------------------------------
    TString strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD0, 0, kMulti, 0);
    for(Int_t l=0; l<NumOfLayersSSD0; ++l){
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD0+1, 0, NumOfSegSSD0+1,
			     "Multiplicity", ""));
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_C%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD0+1, 0, NumOfSegSSD0+1,
			     "CMultiplicity", ""));
    }
    top_dir->Add(sub_dir);
  }

  return top_dir;
}

// -------------------------------------------------------------------------
// createSSD1
// -------------------------------------------------------------------------
TList* HistMaker::createSSD1( Bool_t flag_ps )
{
  TString strDet = CONV_STRING(kSSD1);
  name_created_detectors_.push_back(strDet);
  if(flag_ps) name_ps_files_.push_back(strDet);

  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  const char* nameLayer[NumOfLayersSSD1] = { "y0", "x0", "y1", "x1" };

  // ADC --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD1, 0, kADC2D, 0);
    for(Int_t l=0; l<NumOfLayersSSD1; ++l){
      sub_dir->Add(createTH2(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD1, 0, NumOfSegSSD1, 0x3ff, 0, 0x3ff,
			     "Segment", "Peak Height  "));
    }
    top_dir->Add(sub_dir);
  }

  // TDC --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD1, 0, kTDC2D, 0);
    for(Int_t l=0; l<NumOfLayersSSD1; ++l){
      sub_dir->Add(createTH2(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD1, 0, NumOfSegSSD1, NumOfSamplesSSD+1, 0, NumOfSamplesSSD+1,
			     "Segment", "Peak Position  "));
    }
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD1, 0, kHitPat, 0);
    for(Int_t l=0; l<NumOfLayersSSD1; ++l){
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD1, 0, NumOfSegSSD1,
			     "Segment", ""));
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_C%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD1, 0, NumOfSegSSD1,
			     "Segment", ""));
    }
    target_id = getUniqueID(kSSD1, 0, kHitPat2D, 0);
    sub_dir->Add(createTH2(++target_id, Form("%s_%s_%s%%%s", nameDetector, nameSubDir,
					     nameLayer[0], nameLayer[1]),
			   NumOfSegSSD1/12, 0, NumOfSegSSD1, NumOfSegSSD1/12, 0, NumOfSegSSD1,
			   nameLayer[1], nameLayer[0]));
    sub_dir->Add(createTH2(++target_id, Form("%s_%s_%s%%%s", nameDetector, nameSubDir,
					     nameLayer[2], nameLayer[3]),
			   NumOfSegSSD1/12, 0, NumOfSegSSD1, NumOfSegSSD1/12, 0, NumOfSegSSD1,
			   nameLayer[3], nameLayer[2]));
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD1, 0, kMulti, 0);
    for(Int_t l=0; l<NumOfLayersSSD1; ++l){
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     100, 0., 100.,
			     "Multiplicity", ""));
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_C%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     100, 0., 100.,
			     "CMultiplicity", ""));
    }
    top_dir->Add(sub_dir);
  }

  // DeltaE --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kDeltaE);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD1, 0, kDeltaE, 0);
    for(Int_t l=0; l<NumOfLayersSSD1; ++l){
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     200, 0., 40000., "DeltaE  "));
    }
    top_dir->Add(sub_dir);
  }

  // DeltaE2D --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kDeltaE2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD1, 0, kDeltaE2D, 0);
    for(Int_t l=0; l<NumOfLayersSSD1; ++l){
      sub_dir->Add(createTH2(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD1/4, 0, NumOfSegSSD1, 200, 0, 40000,
			     "Segment", "DeltaE  "));
    }
    top_dir->Add(sub_dir);
  }

  // Time --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kCTime);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD1, 0, kCTime, 0);
    for(Int_t l=0; l<NumOfLayersSSD1; ++l){
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     200., 0, 200., "Peak Time [ns] "));
    }
    top_dir->Add(sub_dir);
  }

  // Time2D --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kCTime2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD1, 0, kCTime2D, 0);
    for(Int_t l=0; l<NumOfLayersSSD1; ++l){
      sub_dir->Add(createTH2(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD1/4, 0, NumOfSegSSD1, 200, 0, 200,
			     "Segment", "Peak Time [ns] "));
    }
    top_dir->Add(sub_dir);
  }

  // Chisqr --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kChisqr);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD1, 0, kChisqr, 0);
    for(Int_t l=0; l<NumOfLayersSSD1; ++l){
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     500., 0, 500., "Chisqr  "));
    }
    top_dir->Add(sub_dir);
  }

  return top_dir;
}

// -------------------------------------------------------------------------
// createSSD2
// -------------------------------------------------------------------------
TList* HistMaker::createSSD2( Bool_t flag_ps )
{
  TString strDet = CONV_STRING(kSSD2);
  name_created_detectors_.push_back(strDet);
  if(flag_ps) name_ps_files_.push_back(strDet);

  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  const char* nameLayer[NumOfLayersSSD2] = { "x0", "y0", "x1", "y1" };

  // ADC --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD2, 0, kADC2D, 0);
    for(Int_t l=0; l<NumOfLayersSSD2; ++l){
      sub_dir->Add(createTH2(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD2, 0, NumOfSegSSD2, 0x3ff, 0, 0x3ff,
			     "Segment", "Peak Height  "));
    }
    top_dir->Add(sub_dir);
  }

  // TDC --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD2, 0, kTDC2D, 0);
    for(Int_t l=0; l<NumOfLayersSSD2; ++l){
      sub_dir->Add(createTH2(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD2, 0, NumOfSegSSD2, NumOfSamplesSSD+1, 0, NumOfSamplesSSD+1,
			     "Segment", "Peak Position  "));
    }
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD2, 0, kHitPat, 0);
    for(Int_t l=0; l<NumOfLayersSSD2; ++l){
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD2, 0, NumOfSegSSD2,
			     "Segment", ""));
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_C%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD2, 0, NumOfSegSSD2,
			     "Segment", ""));
    }
    target_id = getUniqueID(kSSD2, 0, kHitPat2D, 0);
    sub_dir->Add(createTH2(++target_id, Form("%s_%s_%s%%%s", nameDetector, nameSubDir,
					     nameLayer[1], nameLayer[0]),
			   NumOfSegSSD2/12, 0, NumOfSegSSD2, NumOfSegSSD2/12, 0, NumOfSegSSD2,
			   nameLayer[0], nameLayer[1]));
    sub_dir->Add(createTH2(++target_id, Form("%s_%s_%s%%%s", nameDetector, nameSubDir,
					     nameLayer[3], nameLayer[2]),
			   NumOfSegSSD2/12, 0, NumOfSegSSD2, NumOfSegSSD2/12, 0, NumOfSegSSD2,
			   nameLayer[2], nameLayer[3]));
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD2, 0, kMulti, 0);
    for(Int_t l=0; l<NumOfLayersSSD2; ++l){
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     100, 0., 100.,
			     "Multiplicity", ""));
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_C%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     100, 0., 100.,
			     "CMultiplicity", ""));
    }
    top_dir->Add(sub_dir);
  }

  // DeltaE --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kDeltaE);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD2, 0, kDeltaE, 0);
    for(Int_t l=0; l<NumOfLayersSSD2; ++l){
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     200, 0., 40000., "DeltaE  "));
    }
    top_dir->Add(sub_dir);
  }

  // DeltaE2D --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kDeltaE2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD2, 0, kDeltaE2D, 0);
    for(Int_t l=0; l<NumOfLayersSSD2; ++l){
      sub_dir->Add(createTH2(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD2/4, 0, NumOfSegSSD2, 200, 0, 40000,
			     "Segment", "DeltaE  "));
    }
    top_dir->Add(sub_dir);
  }

  // Time --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kCTime);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD2, 0, kCTime, 0);
    for(Int_t l=0; l<NumOfLayersSSD2; ++l){
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     200., 0, 200., "Peak Time [ns] "));
    }
    top_dir->Add(sub_dir);
  }

  // Time2D --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kCTime2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD2, 0, kCTime2D, 0);
    for(Int_t l=0; l<NumOfLayersSSD2; ++l){
      sub_dir->Add(createTH2(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     NumOfSegSSD2/4, 0, NumOfSegSSD2, 200, 0, 200,
			     "Segment", "Peak Time [ns] "));
    }
    top_dir->Add(sub_dir);
  }

  // Chisqr --------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kChisqr);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSSD2, 0, kChisqr, 0);
    for(Int_t l=0; l<NumOfLayersSSD2; ++l){
      sub_dir->Add(createTH1(++target_id,
			     Form("%s_%s_%s", nameDetector, nameSubDir, nameLayer[l]),
			     500., 0, 500., "Chisqr  "));
    }
    top_dir->Add(sub_dir);
  }

  return top_dir;
}

// -------------------------------------------------------------------------
// createPVAC
// -------------------------------------------------------------------------
TList* HistMaker::createPVAC( Bool_t flag_ps )
{
  TString strDet = CONV_STRING(kPVAC);
  name_created_detectors_.push_back(strDet);
  if(flag_ps) name_ps_files_.push_back(strDet);

  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  const char* name_acs[NumOfSegPVAC] = { "PVAC" };

  // ADC---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kPVAC, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfSegPVAC; ++i){
      sub_dir->Add(createTH1( ++target_id,
			      Form("%s_%s", name_acs[i], nameSubDir),
			      0x1000, 0, 0x1000,
			      "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  // ADC---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kADCwTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kPVAC, 0, kADCwTDC, 0);
    for(Int_t i = 0; i<NumOfSegPVAC; ++i){
      sub_dir->Add(createTH1( ++target_id,
			      Form("%s_%s", name_acs[i], nameSubDir),
			      0x1000, 0, 0x1000,
			      "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  // TDC---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kPVAC, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegPVAC; ++i){
      const char* title = NULL;
      title = Form("%s_%s", name_acs[i], nameSubDir);
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    Int_t target_id = getUniqueID(kPVAC, 0, kHitPat, 0);
    top_dir->Add(createTH1(++target_id, "PVAC_hit_pattern",
			   NumOfSegPVAC, 0, NumOfSegPVAC,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    Int_t target_id = getUniqueID(kPVAC, 0, kMulti, 0);
    top_dir->Add(createTH1(++target_id, "PVAC_multiplicity",
			   NumOfSegPVAC+1, 0, NumOfSegPVAC+1,
			   "Multiplicity", ""));
  }

  return top_dir;
}

// -------------------------------------------------------------------------
// createFAC
// -------------------------------------------------------------------------
TList* HistMaker::createFAC( Bool_t flag_ps )
{
  TString strDet = CONV_STRING(kFAC);
  name_created_detectors_.push_back(strDet);
  if(flag_ps) name_ps_files_.push_back(strDet);

  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  const char* name_acs[NumOfSegFAC] = { "FAC" };

  // ADC---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kFAC, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfSegFAC; ++i){
      const char* title = NULL;
      title = Form("%s_%s", name_acs[i], nameSubDir);
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  // ADC ---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kADCwTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kFAC, 0, kADCwTDC, 0);
    for( Int_t i=0; i<NumOfSegFAC; ++i ){
      const char* title = NULL;
      title = Form("%s_%s", name_acs[i], nameSubDir);
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  // TDC---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kFAC, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegFAC; ++i){
      const char* title = NULL;
      title = Form("%s_%s", name_acs[i], nameSubDir);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    Int_t target_id = getUniqueID(kFAC, 0, kHitPat, 0);
    top_dir->Add(createTH1(++target_id, "FAC_hit_pattern",
			   NumOfSegFAC, 0, NumOfSegFAC,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    Int_t target_id = getUniqueID(kFAC, 0, kMulti, 0);
    top_dir->Add(createTH1(++target_id, "FAC_multiplicity",
			   NumOfSegFAC+1, 0, NumOfSegFAC+1,
			   "Multiplicity", ""));
  }

  return top_dir;
}

// -------------------------------------------------------------------------
// createSAC1
// -------------------------------------------------------------------------
TList* HistMaker::createSAC1( Bool_t flag_ps )
{
  TString strDet = CONV_STRING(kSAC1);
  name_created_detectors_.push_back(strDet);
  if(flag_ps) name_ps_files_.push_back(strDet);

  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  const char* name_acs[NumOfSegSAC1] = { "SAC1" };

  // ADC---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSAC1, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfSegSAC1; ++i){
      const char* title = NULL;
      title = Form("%s_%s", name_acs[i], nameSubDir);
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  // ADC w/TDC ---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kADCwTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSAC1, 0, kADCwTDC, 0);
    for( Int_t i=0; i<NumOfSegSAC1; ++i ){
      const char* title = NULL;
      title = Form("%s_%s", name_acs[i], nameSubDir);
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  // TDC---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSAC1, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegSAC1; ++i){
      const char* title = NULL;
      title = Form("%s_%s", name_acs[i], nameSubDir);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    Int_t target_id = getUniqueID(kSAC1, 0, kHitPat, 0);
    top_dir->Add(createTH1(++target_id, "SAC1_hit_pattern",
			   NumOfSegSAC1, 0, NumOfSegSAC1,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    Int_t target_id = getUniqueID(kSAC1, 0, kMulti, 0);
    top_dir->Add(createTH1(++target_id, "SAC1_multiplicity",
			   NumOfSegSAC1+1-4, 0, NumOfSegSAC1+1-4,
			   "Multiplicity", ""));
  }

  return top_dir;
}

// -------------------------------------------------------------------------
// createSCH
// -------------------------------------------------------------------------
TList* HistMaker::createSCH( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kSCH);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }
  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // TDC---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSCH, 0, kTDC, 0);
    for(Int_t i=0; i<NumOfSegSCH; ++i){
      sub_dir->Add(createTH1(++target_id, Form("%s_%s_%d", // 1 origin
					       nameDetector, nameSubDir, i+1),
			     1024, 0, 1024,
			     "TDC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }
  // TOT---------------------------------------------------------
  {
    const char* nameSubDir = "TOT";
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kSCH, 0, kADC, 0);
    for(Int_t i=0; i<NumOfSegSCH; ++i){
      sub_dir->Add(createTH1(++target_id, Form("%s_%s_%d", // 1 origin
					       nameDetector, nameSubDir, i+1),
			     200, -50, 150,
			     "TOT [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }
  // TDC/TOT SUM -----------------------------------------------------
  {
    // TDC
    top_dir->Add(createTH1(getUniqueID(kSCH, 0, kTDC, NumOfSegSCH+1),
			   Form("%s_TDC_ALL", nameDetector),
			   1024, 0, 1024, "TDC [ch]", ""));

    // TOT
    top_dir->Add(createTH1(getUniqueID(kSCH, 0, kADC, NumOfSegSCH+1),
			   Form("%s_TOT_ALL", nameDetector),
			   200, -50, 150, "TOT [ch]", ""));
  }
  // TDC-2D --------------------------------------------
  {
    Int_t target_id = getUniqueID(kSCH, 0, kTDC2D, 0);
    top_dir->Add(createTH2(++target_id, "SCH_TDC_2D", // 1 origin
			   NumOfSegSCH, 0, NumOfSegSCH,
			   1024, 0, 1024,
			   "Segment", "TDC [ch]"));
  }
  // TOT-2D --------------------------------------------
  {
    Int_t target_id = getUniqueID(kSCH, 0, kADC2D, 0);
    top_dir->Add(createTH2(++target_id, "SCH_TOT_2D", // 1 origin
			   NumOfSegSCH, 0, NumOfSegSCH,
			   200, -50, 150,
			   "Segment", "TOT [ch]"));
  }
  // Hit parttern -----------------------------------------------
  {
    Int_t target_id = getUniqueID(kSCH, 0, kHitPat, 0);
    top_dir->Add(createTH1(++target_id, "SCH_HitPat", // 1 origin
			   NumOfSegSCH, 0, NumOfSegSCH,
			   "Segment", ""));
  }
  // Multiplicity -----------------------------------------------
  {
    const char* title = "SCH_multiplicity";
    Int_t target_id = getUniqueID(kSCH, 0, kMulti, 0);
    top_dir->Add(createTH1(++target_id, title, // 1 origin
			   20, 0, 20,
			   "Multiplicity", ""));
  }
  return top_dir;
}

// -------------------------------------------------------------------------
// createKFAC
// -------------------------------------------------------------------------
TList* HistMaker::createKFAC( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kKFAC);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kKFAC, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfSegKFAC; ++i){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    // ADC2D
    target_id = getUniqueID(kKFAC, 0, kADC2D, 0);
    sub_dir->Add(createTH2(++target_id, "KFAC_ADC_2D",
			   0x1000, 0, 0x1000, 0x1000, 0, 0x1000,
			   "ADC [ch]", "ADC [ch]"));

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createEMC
// -------------------------------------------------------------------------
TList* HistMaker::createEMC( Bool_t flag_ps )
{
  TString strDet = CONV_STRING(kEMC);
  name_created_detectors_.push_back(strDet);
  if(flag_ps) name_ps_files_.push_back(strDet);

  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  { // XY ---------------------------------------------------------
    //Int_t serial_id = getUniqueID(kEMC, 0, kSerial, 0);
    Int_t xpos_id   = getUniqueID(kEMC, 0, kXpos,   0);
    Int_t ypos_id   = getUniqueID(kEMC, 0, kYpos,   0);
    Int_t xypos_id  = getUniqueID(kEMC, 0, kXYpos,  0);
    //Int_t time_id   = getUniqueID(kEMC, 0, kTime,   0);

    for(Int_t i = 0; i<NumOfSegEMC; ++i){
      top_dir->Add(createTH1( ++xpos_id,
			      Form("%s_Xpos", strDet.Data()),
			      0x20, -0x100, 0x100,
			      "Xpos [mm]", ""));
      top_dir->Add(createTH1( ++ypos_id,
			      Form("%s_Ypos", strDet.Data()),
			      0x20, -0x100, 0x100,
			      "Ypos [mm]", ""));
      top_dir->Add(createTH2( ++xypos_id,
			      Form("%s_XYpos", strDet.Data()),
			      0x80, -0x100, 0x100, 0x80, -0x100, 0x100,
			      "Xpos [mm]", "Ypos [mm]"));
    }
  }

  return top_dir;
}

// -------------------------------------------------------------------------
// createKIC
// -------------------------------------------------------------------------
TList* HistMaker::createKIC( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kKIC);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  const char* name_kics[] = { "U", "D", "L", "R" };

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kKIC, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfSegKIC; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_kics[i]);
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kKIC, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegKIC; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_kics[i]);
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    const char* title = "KIC_hit_pattern";
    Int_t target_id = getUniqueID(kKIC, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegKIC, 0, NumOfSegKIC,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "KIC_multiplicity";
    Int_t target_id = getUniqueID(kKIC, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegKIC+1, 0, NumOfSegKIC+1,
			   "Multiplicity", ""));
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createSDC1
// -------------------------------------------------------------------------
TList* HistMaker::createSDC1( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kSDC1);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  const char* name_layer[NumOfLayersSDC1] = {"u0", "u1", "x0", "x1", "v0", "v1"};
  const char* name_Selflayer[NumOfDimSDC1] = { "u0_u1", "x0_x1","v0_v1" };
  //  const char* name_layer[NumOfLayersSDC3] = { "y0", "y1", "x0", "x1" };

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC1, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC1, 0, kTDC2D, 0);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    //TDCwide
    target_id = getUniqueID(kSDC1, 0, kTDC, 10);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_%swide_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     5000, 0, 5000,
			     "TDC [ch]", ""));
    }

    // TDCwTOTCUT -------------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC1, 0, kTDC, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC1, 0, kTDC2D, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_C%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    // TDC 2D ---------------------------------------------------
    target_id = getUniqueID(kSDC1, 0, kTDC2D, kTOTcutOffset+20);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i+1, title,
			     NumOfWireSDC1, 0, NumOfWireSDC1,
			     5000, 0, 5000,
			     "Wire Number","TDC [ch]"));
    }

    // TDC 2D wTOTCUT ---------------------------------------------------
    target_id = getUniqueID(kSDC1, 0, kTDC2D, kTOTcutOffset+30);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_C%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i+1, title,
			     NumOfWireSDC1, 0, NumOfWireSDC1,
			     5000, 0, 5000,
			     "Wire Number","TDC [ch]"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSDC1, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     500, 0, 500,
			     "TOT [ch]", ""));
    }

    // CTOT---------------------------------------------------------
    target_id = getUniqueID(kSDC1, 0, kADC, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     500, 0, 500,
			     "TOT [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC1, 0, kHitPat, 0);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     NumOfWireSDC1, 0, NumOfWireSDC1,
			     "wire", ""));
    }

    // HitPatwTOTCUT -----------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC1, 0, kHitPat, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     NumOfWireSDC1, 0, NumOfWireSDC1,
			     "wire", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Self Correlation ----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kSelfCorr);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    for(int i=0; i<NumOfDimSDC1; i++){
      Int_t target_id = getUniqueID(kSDC1, kSelfCorr, 0, i);
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_Selflayer[i]);
      sub_dir->Add(createTH2(target_id + 1, title, // 1 origin
          		     NumOfWireSDC1, 0, NumOfWireSDC1,
          		     NumOfWireSDC1, 0, NumOfWireSDC1,
          		     Form("%s",name_layer[2*i]),Form("%s",name_layer[2*i+1])));
    }


    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    Int_t target_id = getUniqueID(kSDC1, 0, kMulti, 0);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC1, 0, NumOfWireSDC1,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC1, 0, kMulti, NumOfLayersSDC1);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC1, 0, NumOfWireSDC1,
			     "Multiplicity", ""));
    }

    // MultiplicitywTOTCUT ----------------------------------------
    // Make histogram and add it
    // without TDC gate
    target_id = getUniqueID(kSDC1, 0, kMulti, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC1, 0, NumOfWireSDC1,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC1, 0, kMulti, NumOfLayersSDC1+kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC1; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC1, 0, NumOfWireSDC1,
			     "Multiplicity", ""));
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
TList* HistMaker::createSDC2( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kSDC2);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  // const char* name_layer[NumOfLayersSDC2] = { "x0", "x1", "y0", "y1" };
  const char* name_layer[NumOfLayersSDC2] = { "v0", "v1", "u0", "u1" };
  // const char* name_Selflayer[NumOfDimSDC2] = { "x0_x1", "y0_y1" };
  const char* name_Selflayer[NumOfDimSDC2] = { "v0_v1", "u0_u1" };
  //  const char* name_layer[NumOfLayersSDC2] = { "y0", "y1", "x0", "x1" };

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC2, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC2, 0, kTDC2D, 0);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    //TDCwide
    target_id = getUniqueID(kSDC2, 0, kTDC, 10);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%swide_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     5000, 0, 5000,
			     "TDC [ch]", ""));
    }

    // TDCwTOTCUT -------------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC2, 0, kTDC, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC2, 0, kTDC2D, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_C%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    // TDC 2D ---------------------------------------------------
    target_id = getUniqueID(kSDC2, 0, kTDC2D, kTOTcutOffset+20);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i+1, title,
			     NumOfWireSDC2, 0, NumOfWireSDC2,
			     5000, 0, 5000,
			     "Wire Number","TDC [ch]"));
    }

    // TDC 2D wTOTCUT ---------------------------------------------------
    target_id = getUniqueID(kSDC2, 0, kTDC2D, kTOTcutOffset+30);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_C%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i+1, title,
			     NumOfWireSDC2, 0, NumOfWireSDC2,
			     5000, 0, 5000,
			     "Wire Number","TDC [ch]"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSDC2, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     500, 0, 500,
			     "TOT [ch]", ""));
    }

    // CTOT---------------------------------------------------------
    target_id = getUniqueID(kSDC2, 0, kADC, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     500, 0, 500,
			     "TOT [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    // Int_t nwire;

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC2, 0, kHitPat, 0);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      //   if( i==0 || i==1 ) nwire = NumOfWireSDC2X;
      //   if( i==2 || i==3 ) nwire = NumOfWireSDC2Y;
      const char* title = NULL;
      // title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      // sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
      // 			     nwire, 0, nwire,
      // 			     "wire", ""));
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
       			     NumOfWireSDC2, 0, NumOfWireSDC2,
       			     "wire", ""));
    }

    // HitPatwTOTCUT -----------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC2, 0, kHitPat, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      // if( i==0 || i==1 ) nwire = NumOfWireSDC2X;
      // if( i==2 || i==3 ) nwire = NumOfWireSDC2Y;
      const char* title = NULL;
      // title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      // sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
      // 			     nwire, 0, nwire,
      // 			     "wire", ""));
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     NumOfWireSDC2, 0, NumOfWireSDC2,
			     "wire", ""));

    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Self Correlation ----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kSelfCorr);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    for(int i=0; i<NumOfDimSDC2; i++){
      //	int NumOfWireSDC2;
      // if( i==0 )
      //   NumOfWireSDC2 = NumOfWireSDC2X;
      // if( i==1 )
      //   NumOfWireSDC2 = NumOfWireSDC2Y;
      Int_t target_id = getUniqueID(kSDC2, kSelfCorr, 0, i);
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_Selflayer[i]);
      sub_dir->Add(createTH2(target_id + 1, title, // 1 origin
          		     NumOfWireSDC2, 0, NumOfWireSDC2,
          		     NumOfWireSDC2, 0, NumOfWireSDC2,
          		     Form("%s",name_layer[2*i]),Form("%s",name_layer[2*i+1])));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    Int_t target_id = getUniqueID(kSDC2, 0, kMulti, 0);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC2, 0, NumOfWireSDC2,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC2, 0, kMulti, NumOfLayersSDC2);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
    			     NumOfWireSDC2, 0, NumOfWireSDC2,
			     "Multiplicity", ""));
    }

    // MultiplicitywTOTCUT ----------------------------------------
    // Make histogram and add it
    // without TDC gate
    target_id = getUniqueID(kSDC2, 0, kMulti, kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC2, 0, NumOfWireSDC2,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC2, 0, kMulti, NumOfLayersSDC2+kTOTcutOffset);
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC2, 0, NumOfWireSDC2,
			     "Multiplicity", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  //  Multiplicity2D -----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kMulti2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    // Make histogram and add it
    //   without TDC gate
    Int_t target_id = getUniqueID(kSDC2, 0, kMulti2D, 0);
    int cor_id = 0;
    for(Int_t i = 0; i<NumOfLayersSDC2; ++i){
      for(Int_t j = 0; j<NumOfLayersSDC2; ++j){
	if( i >= j ) continue;
	const char* title = NULL;
	title = Form("%s_%s_%s_%s", nameDetector, nameSubDir, name_layer[i], name_layer[j]);
	sub_dir->Add(createTH2(target_id + cor_id + 1, title,
			       NumOfWireSDC2, 0, NumOfWireSDC2,
			       NumOfWireSDC2, 0, NumOfWireSDC2,
			       Form("%s",name_layer[i]),Form("%s",name_layer[j])));
	cor_id++;
      }
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createSDC3
// -------------------------------------------------------------------------
TList* HistMaker::createSDC3( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kSDC3);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  const char* name_layer[NumOfLayersSDC3] = { "x0", "x1", "y0", "y1" };
  const char* name_Selflayer[NumOfDimSDC3] = { "x0_x1", "y0_y1" };
  //  const char* name_layer[NumOfLayersSDC3] = { "y0", "y1", "x0", "x1" };

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC3, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     2000, 0, 2000,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC3, 0, kTDC2D, 0);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    // TDCwTOTCUT -------------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC3, 0, kTDC, 10);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC3, 0, kTDC2D, 10);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    // TDC 2D ---------------------------------------------------
    target_id = getUniqueID(kSDC3, 0, kTDC2D, 20);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i+1, title,
			     NumOfWireSDC3, 0, NumOfWireSDC3,
			     1500, 0, 1500,
			     "Wire Number","TDC [ch]"));
    }

    // TDC 2D wTOTCUT ---------------------------------------------------
    target_id = getUniqueID(kSDC3, 0, kTDC2D, 30);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i+1, title,
			     NumOfWireSDC3, 0, NumOfWireSDC3,
			     1500, 0, 1500,
			     "Wire Number","TDC [ch]"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSDC3, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     500, 0, 500,
			     "TOT [ch]", ""));
    }

    // CTOT---------------------------------------------------------
    target_id = getUniqueID(kSDC3, 0, kADC, 10);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     500, 0, 500,
			     "TOT [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC3, 0, kHitPat, 0);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     NumOfWireSDC3, 0, NumOfWireSDC3,
			     "wire", ""));
    }

    // HitPatwTOTCUT -----------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC3, 0, kHitPat, 10);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     NumOfWireSDC3, 0, NumOfWireSDC3,
			     "wire", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Self Correlation ----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kSelfCorr);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    for(int i=0; i<NumOfDimSDC3; i++){
      Int_t target_id = getUniqueID(kSDC3, kSelfCorr, 0, i);
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_Selflayer[i]);
      sub_dir->Add(createTH2(target_id + 1, title, // 1 origin
          		     NumOfWireSDC3, 0, NumOfWireSDC3,
          		     NumOfWireSDC3, 0, NumOfWireSDC3,
          		     Form("%s",name_layer[2*i]),Form("%s",name_layer[2*i+1])));
    }


    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    Int_t target_id = getUniqueID(kSDC3, 0, kMulti, 0);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC3, 0, NumOfWireSDC3,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC3, 0, kMulti, NumOfLayersSDC3);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC3, 0, NumOfWireSDC3,
			     "Multiplicity", ""));
    }

    // MultiplicitywTOTCUT ----------------------------------------
    // Make histogram and add it
    // without TDC gate
    target_id = getUniqueID(kSDC3, 0, kMulti, 10);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC3, 0, NumOfWireSDC3,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC3, 0, kMulti, NumOfLayersSDC3+10);
    for(Int_t i = 0; i<NumOfLayersSDC3; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC3, 0, NumOfWireSDC3,
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
TList* HistMaker::createSDC4( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kSDC4);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  const char* name_layer[NumOfLayersSDC4] = { "y0", "y1", "x0", "x1" };
  const char* name_Selflayer[NumOfDimSDC4] = { "y0_y1", "x0_x1" };
  //  const char* name_layer[NumOfLayersSDC4] = { "y0", "y1", "x0", "x1" };

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC4, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC4, 0, kTDC2D, 0);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    // TDCwTOTCUT -------------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC4, 0, kTDC, 10);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC4, 0, kTDC2D, 10);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    // TDC 2D ---------------------------------------------------
    target_id = getUniqueID(kSDC4, 0, kTDC2D, 20);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i+1, title,
			     NumOfWireSDC4, 0, NumOfWireSDC4,
			     1500, 0, 1500,
			     "Wire Number","TDC [ch]"));
    }

    // TDC 2D wTOTCUT ---------------------------------------------------
    target_id = getUniqueID(kSDC4, 0, kTDC2D, 30);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH2(target_id + i+1, title,
			     NumOfWireSDC4, 0, NumOfWireSDC4,
			     1500, 0, 1500,
			     "Wire Number","TDC [ch]"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSDC4, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){

      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     500, 0, 500,
			     "TOT [ch]", ""));
    }

    // CTOT---------------------------------------------------------
    target_id = getUniqueID(kSDC4, 0, kADC, 10);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     500, 0, 500,
			     "TOT [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC4, 0, kHitPat, 0);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     NumOfWireSDC4, 0, NumOfWireSDC4,
			     "wire", ""));
    }

    // HitPatwTOTCUT ----  -------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC4, 0, kHitPat, 10);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     NumOfWireSDC4, 0, NumOfWireSDC4,
			     "wire", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Self Correlation ----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kSelfCorr);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    for(int i=0; i<NumOfDimSDC4; i++){
      Int_t target_id = getUniqueID(kSDC4, kSelfCorr, 0, i);
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_Selflayer[i]);
      sub_dir->Add(createTH2(target_id + 1, title, // 1 origin
          		     NumOfWireSDC4, 0, NumOfWireSDC4,
          		     NumOfWireSDC4, 0, NumOfWireSDC4,
          		     Form("%s",name_layer[2*i]),Form("%s",name_layer[2*i+1])));
    }


    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    Int_t target_id = getUniqueID(kSDC4, 0, kMulti, 0);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC4, 0, NumOfWireSDC4,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC4, 0, kMulti, NumOfLayersSDC4);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC4, 0, NumOfWireSDC4,
			     "Multiplicity", ""));
    }

    // MultiplicitywTOTCUT ----------------------------------------
    // Make histogram and add it
    // without TDC gate
    target_id = getUniqueID(kSDC4, 0, kMulti, 10);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC4, 0, NumOfWireSDC4,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC4, 0, kMulti, NumOfLayersSDC4+10);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     // 128, 0, 128,
			     NumOfWireSDC4, 0, NumOfWireSDC4,
			     "Multiplicity", ""));
    }


    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createSDC5
// -------------------------------------------------------------------------
TList* HistMaker::createSDC5( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kSDC5);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  const char* name_layer[NumOfLayersSDC5] = { "y0", "y1", "x0", "x1" };
  const char* name_Selflayer[NumOfDimSDC5] = { "y0_y1", "x0_x1" };
  //  const char* name_layer[NumOfLayersSDC5] = { "y0", "y1", "x0", "x1" };

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC5, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC5; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC5, 0, kTDC2D, 0);
    for(Int_t i = 0; i<NumOfLayersSDC5; ++i){
      const char* title = NULL;
      title = Form("%s_%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    // TDCwTOTCUT -------------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC5, 0, kTDC, 10);
    for(Int_t i = 0; i<NumOfLayersSDC5; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC5, 0, kTDC2D, 10);
    for(Int_t i = 0; i<NumOfLayersSDC5; ++i){
      const char* title = NULL;
      title = Form("%s_C%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    // TDC 2D ---------------------------------------------------
    target_id = getUniqueID(kSDC5, 0, kTDC2D, 20);
    for(Int_t i = 0; i<NumOfLayersSDC5; ++i){
      const char* title = NULL;
      title = Form("%s_%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      Int_t NumOfWireSDC5 = 0;
      if(name_layer[i]=="y0" || name_layer[i]=="y1") NumOfWireSDC5 = NumOfWireSDC5Y;
      if(name_layer[i]=="x0" || name_layer[i]=="x1") NumOfWireSDC5 = NumOfWireSDC5X;
      sub_dir->Add(createTH2(target_id + i+1, title,
			     NumOfWireSDC5, 0, NumOfWireSDC5,
			     1500, 0, 1500,
			     "Wire Number","TDC [ch]"));
    }

    // TDC 2D wTOTCUT ---------------------------------------------------
    target_id = getUniqueID(kSDC5, 0, kTDC2D, 30);
    for(Int_t i = 0; i<NumOfLayersSDC5; ++i){
      const char* title = NULL;
      title = Form("%s_C%s2D_%s", nameDetector, nameSubDir, name_layer[i]);
      Int_t NumOfWireSDC5 = 0;
      if(name_layer[i]=="y0" || name_layer[i]=="y1") NumOfWireSDC5 = NumOfWireSDC5Y;
      if(name_layer[i]=="x0" || name_layer[i]=="x1") NumOfWireSDC5 = NumOfWireSDC5X;
      sub_dir->Add(createTH2(target_id + i+1, title,
			     NumOfWireSDC5, 0, NumOfWireSDC5,
			     1500, 0, 1500,
			     "Wire Number","TDC [ch]"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSDC5, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC5; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     500, 0, 500,
			     "TOT [ch]", ""));
    }

    // CTOT---------------------------------------------------------
    target_id = getUniqueID(kSDC5, 0, kADC, 10);
    for(Int_t i = 0; i<NumOfLayersSDC5; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     500, 0, 500,
			     "TOT [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t nwire;

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC5, 0, kHitPat, 0);
    for(Int_t i = 0; i<NumOfLayersSDC5; ++i){
      if( i==0 || i==1 ) nwire = NumOfWireSDC5Y;
      if( i==2 || i==3 ) nwire = NumOfWireSDC5X;
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     nwire, 0, nwire,
			     "wire", ""));
    }

    // HitPatwTOTCUT ----  -------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC5, 0, kHitPat, 10);
    for(Int_t i = 0; i<NumOfLayersSDC5; ++i){
      if( i==0 || i==1 ) nwire = NumOfWireSDC5Y;
      if( i==2 || i==3 ) nwire = NumOfWireSDC5X;
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     nwire, 0, nwire,
			     "wire", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Self Correlation ----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kSelfCorr);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    for(int i=0; i<NumOfDimSDC5; i++){
      int NumOfWireSDC5;
      if( i==0 )
	NumOfWireSDC5 = NumOfWireSDC5Y;
      if( i==1 )
	NumOfWireSDC5 = NumOfWireSDC5X;
      Int_t target_id = getUniqueID(kSDC5, kSelfCorr, 0, i);
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_Selflayer[i]);
      sub_dir->Add(createTH2(target_id + 1, title, // 1 origin
          		     NumOfWireSDC5, 0, NumOfWireSDC5,
          		     NumOfWireSDC5, 0, NumOfWireSDC5,
          		     Form("%s",name_layer[2*i]),Form("%s",name_layer[2*i+1])));
    }


    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    Int_t target_id = getUniqueID(kSDC5, 0, kMulti, 0);
    for(Int_t i = 0; i<NumOfLayersSDC5; ++i){

      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     128, 0, 128,
			     // NumOfWireSDC5, 0, NumOfWireSDC5,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC5, 0, kMulti, NumOfLayersSDC5);
    for(Int_t i = 0; i<NumOfLayersSDC5; ++i){

      const char* title = NULL;
      title = Form("%s_%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     128, 0, 128,
			     // NumOfWireSDC5, 0, NumOfWireSDC5,
			     "Multiplicity", ""));
    }

    // MultiplicitywTOTCUT ----------------------------------------
    // Make histogram and add it
    // without TDC gate
    target_id = getUniqueID(kSDC5, 0, kMulti, 10);
    for(Int_t i = 0; i<NumOfLayersSDC5; ++i){

      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     128, 0, 128,
			     // NumOfWireSDC5, 0, NumOfWireSDC5,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC5, 0, kMulti, NumOfLayersSDC5+10);
    for(Int_t i = 0; i<NumOfLayersSDC5; ++i){

      const char* title = NULL;
      title = Form("%s_C%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     128, 0, 128,
			     // NumOfWireSDC5, 0, NumOfWireSDC5,
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
TList* HistMaker::createSP0( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kSP0);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // Declaration of sub-detector directory
  for(Int_t sd = 0; sd<NumOfLayersSP0; ++sd){
    const char* nameSubDetector = Form("SP0_L%d", sd+1);
    TList *subdet_dir = new TList;
    subdet_dir->SetName(nameSubDetector);

    // ADC---------------------------------------------------------
    {
      // Declaration of the sub-directory
      TString strSubDir  = CONV_STRING(kADC);
      const char* nameSubDir = strSubDir.Data();
      TList *sub_dir = new TList;
      sub_dir->SetName(nameSubDir);

      // Make histogram and add it
      Int_t target_id = getUniqueID(kSP0, kSP0_L1+sd, kADC, 0);
      for(Int_t i = 0; i<NumOfSegSP0*2; ++i){
	const char* title = NULL;
	if(i < NumOfSegSP0){
	  Int_t seg = i+1; // 1 origin
	  title = Form("%s_%s_%dU", nameSubDetector, nameSubDir, seg);
	}else{
	  Int_t seg = i+1-NumOfSegSP0; // 1 origin
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
      TString strSubDir  = CONV_STRING(kTDC);
      const char* nameSubDir = strSubDir.Data();
      TList *sub_dir = new TList;
      sub_dir->SetName(nameSubDir);

      // Make histogram and add it
      Int_t target_id = getUniqueID(kSP0, kSP0_L1+sd, kTDC, 0);
      for(Int_t i = 0; i<NumOfSegSP0*2; ++i){
	const char* title = NULL;
	if(i < NumOfSegSP0){
	  Int_t seg = i+1; // 1 origin
	  title = Form("%s_%s_%dU", nameSubDetector, nameSubDir, seg);
	}else{
	  Int_t seg = i+1-NumOfSegSP0; // 1 origin
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
      Int_t target_id = getUniqueID(kSP0, kSP0_L1+sd, kHitPat, 0);
      for(Int_t i = 0; i<2; ++i){
	const char* title = NULL;
	if(i == 0){
	  title = Form("%s_HitPat_U", nameSubDetector);
	}else{
	  title = Form("%s_HitPat_D", nameSubDetector);
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
// createSDC4
// -------------------------------------------------------------------------
TList* HistMaker::createSDC4( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kSDC4);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // layer configuration
  const char* name_layer[NumOfLayersSDC4] = { "y0", "y1", "x0", "x1" };

  // Dimennsion configuration
  const char* name_Selflayer[NumOfDimSDC4] = { "y0_y1", "x0_x1" };

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC4, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC4, 0, kTDC2D, 0);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    // TDCwTOTCUT -------------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC4, 0, kTDC, 10);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }
    target_id = getUniqueID(kSDC4, 0, kTDC2D, 10);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s1st_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSDC4, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     800, 0, 800,
			     "TOT [ch]", ""));
    }
    // CTOT---------------------------------------------------------
    target_id = getUniqueID(kSDC4, 0, kADC, 10);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     800, 0, 800,
			     "TOT [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }


  // HitPat------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t nwire;

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSDC4, 0, kHitPat, 0);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      if( i==0 || i==1 ) nwire = NumOfWireSDC4Y;
      if( i==2 || i==3 ) nwire = NumOfWireSDC4X;
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     nwire, 0, nwire,
			     "wire", ""));
    }

    // HitPatwTOTCUT -----------------------------------------------
    // Make histogram and add it
    target_id = getUniqueID(kSDC4, 0, kHitPat, 10);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      if( i==0 || i==1 ) nwire = NumOfWireSDC4Y;
      if( i==2 || i==3 ) nwire = NumOfWireSDC4X;
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     nwire, 0, nwire,
			     "wire", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Self Correlation ----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kSelfCorr);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    for(int i=0; i<NumOfDimSDC4; i++){
      int NumOfWireSDC4;
      if( i==0 )
	NumOfWireSDC4 = NumOfWireSDC4Y;
      if( i==1 )
	NumOfWireSDC4 = NumOfWireSDC4X;
      Int_t target_id = getUniqueID(kSDC4, kSelfCorr, 0, i);
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_Selflayer[i]);
      sub_dir->Add(createTH2(target_id + 1, title, // 1 origin
          		     NumOfWireSDC4, 0, NumOfWireSDC4,
          		     NumOfWireSDC4, 0, NumOfWireSDC4,
          		     Form("%s",name_layer[2*i]),Form("%s",name_layer[2*i+1])));
    }


    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // without TDC gate
    Int_t target_id = getUniqueID(kSDC4, 0, kMulti, 0);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     128, 0, 128,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC4, 0, kMulti, NumOfLayersSDC4);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     128, 0, 128,
			     "Multiplicity", ""));
    }

    // MultiplicitywTOTCUT ----------------------------------------
    // Make histogram and add it
    // without TDC gate
    target_id = getUniqueID(kSDC4, 0, kMulti, 10);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     128, 0, 128,
			     "Multiplicity", ""));
    }

    // with TDC gate
    target_id = getUniqueID(kSDC4, 0, kMulti, NumOfLayersSDC4+10);
    for(Int_t i = 0; i<NumOfLayersSDC4; ++i){
      const char* title = NULL;
      title = Form("%s_C%s_%s_wTDC", nameDetector, nameSubDir, name_layer[i]);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     20, 0, 20,
			     128, 0, 128,
			     "Multiplicity", ""));
    }


    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}


// -------------------------------------------------------------------------
// createTOF_HT
// -------------------------------------------------------------------------
TList* HistMaker::createTOF_HT( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kTOF_HT);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*

  const char* nameDetector = strDet.Data();

  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    // Make histogram and add it
    Int_t target_id = getUniqueID(kTOF_HT, 0, kTDC, 0);
    //    for(Int_t i = 0; i<NumOfSegTOF_HT*2; ++i){
    for(Int_t i = 0; i<NumOfSegHtTOF; ++i){
      const char* title = NULL;
      if(i < NumOfSegHtTOF){
	//	Int_t seg = i+1; // 1 origin
	//	title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
	//	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg;
	title = DetHtTOF::SHtTOF[i];
	//      }else{
	//	Int_t seg = i+1-NumOfSegTOF_HT; // 1 origin
	//	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
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
    const char* title = "TOF_HT_hit_pattern";
    Int_t target_id = getUniqueID(kTOF_HT, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegHtTOF, 0, NumOfSegHtTOF,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "TOF_HT_multiplicity";
    Int_t target_id = getUniqueID(kTOF_HT, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegHtTOF, 0, NumOfSegHtTOF,
			   "Multiplicity", ""));
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createTOFMT
// -------------------------------------------------------------------------
TList* HistMaker::createTOFMT( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kTOFMT);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kTOFMT, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegTOF; ++i){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
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
    Int_t target_id = getUniqueID(kTOFMT, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegTOF, 0, NumOfSegTOF,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "TOFMT_multiplicity";
    Int_t target_id = getUniqueID(kTOFMT, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegTOF, 0, NumOfSegTOF,
			   "Multiplicity", ""));
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// // -------------------------------------------------------------------------
// // createLAC
// // -------------------------------------------------------------------------
// TList* HistMaker::createLAC( Bool_t flag_ps )
// {
//   // Determine the detector name
//   TString strDet = CONV_STRING(kLAC);
//   // name list of crearted detector
//   name_created_detectors_.push_back(strDet);
//   if(flag_ps){
//     // name list which are displayed in Ps tab
//     name_ps_files_.push_back(strDet);
//   }

//   // Declaration of the directory
//   // Just type conversion from TString to char*
//   const char* nameDetector = strDet.Data();
//   TList *top_dir = new TList;
//   top_dir->SetName(nameDetector);

// #if 0
//   // ADC---------------------------------------------------------
//   {
//     // Declaration of the sub-directory
//     TString strSubDir  = CONV_STRING(kADC);
//     const char* nameSubDir = strSubDir.Data();
//     TList *sub_dir = new TList;
//     sub_dir->SetName(nameSubDir);

//     // Make histogram and add it
//     Int_t target_id = getUniqueID(kLAC, 0, kADC, 0);
//     for(Int_t i = 0; i<NumOfSegLAC; ++i){
//       const char* title = NULL;
//       Int_t seg = i+1; // 1 origin
//       title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
//       sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
// 			     0x1000, 0, 0x1000,
// 			     "ADC [ch]", ""));
//     }
//     // insert sub directory
//     top_dir->Add(sub_dir);
//   }
// #endif

//   // TDC---------------------------------------------------------
//   {
//     // Declaration of the sub-directory
//     TString strSubDir  = CONV_STRING(kTDC);
//     const char* nameSubDir = strSubDir.Data();
//     TList *sub_dir = new TList;
//     sub_dir->SetName(nameSubDir);

//     // Make histogram and add it
//     Int_t target_id = getUniqueID(kLAC, 0, kTDC, 0);
//     for(Int_t i = 0; i<NumOfSegLAC; ++i){
//       const char* title = NULL;
//       if(i < NumOfSegLAC){
// 	Int_t seg = i+1; // 1 origin
// 	title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
//       }

//       sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
// 			     0x1000, 0, 0x1000,
// 			     "TDC [ch]", ""));
//     }

//     // insert sub directory
//     top_dir->Add(sub_dir);
//   }

//   // Hit parttern -----------------------------------------------
//   {
//     const char* title = "LAC_hit_pattern";
//     Int_t target_id = getUniqueID(kLAC, 0, kHitPat, 0);
//     // Add to the top directory
//     top_dir->Add(createTH1(target_id + 1, title, // 1 origin
// 			   NumOfSegLAC, 0, NumOfSegLAC,
// 			   "Segment", ""));
//   }

//   // Multiplicity -----------------------------------------------
//   {
//     const char* title = "LAC_multiplicity";
//     Int_t target_id = getUniqueID(kLAC, 0, kMulti, 0);
//     // Add to the top directory
//     top_dir->Add(createTH1(target_id + 1, title, // 1 origin
// 			   NumOfSegLAC, 0, NumOfSegLAC,
// 			   "Multiplicity", ""));
//   }

//   // Return the TList pointer which is added into TGFileBrowser
//   return top_dir;
// }

// -------------------------------------------------------------------------
// createAC1
// -------------------------------------------------------------------------
TList* HistMaker::createAC1( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kAC1);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // SUM ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kAC1, 0, kADC, 0);
    for(Int_t i = 0; i<4; ++i){
      Int_t seg = i+1;
      const char* title = NULL;
      if(i<2){
        title = Form("%s_%s_SUM%d", nameDetector, nameSubDir, seg);
      }
      else{
	if(i<3){
          title = Form("%s_%s_ALLSUM", nameDetector, nameSubDir);
        }
        else{
          title = Form("%s_%s_ALLSUM_trig", nameDetector, nameSubDir);
        }
      }
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
	  		     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }
  { // SUM ADCwTDC---------------------------------------------------------
    TString strSubDir  = CONV_STRING(kADCwTDC); // Declaration of the sub-directory
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kAC1, 0, kADCwTDC, 0);
    for(Int_t i = 0; i<4; ++i){
      Int_t seg = i+1;
      const char* title = NULL;
      if(i<2){
        title = Form("%s_%s_SUM%d", nameDetector, nameSubDir, seg);
      }
      else{
        if(i<3){
          title = Form("%s_%s_ALLSUM", nameDetector, nameSubDir);
        }
        else{
          title = Form("%s_%s_ALLSUM_trig", nameDetector, nameSubDir);
        }
      }
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }


  // TDC SUM and INDIVISUAL---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kAC1, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegAC1; ++i){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
      if(i<18){
        title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      }
      else{
	if(i<20){
          title = Form("%s_%s_SUM%d", nameDetector, nameSubDir, seg-18);
        }
	else{
	  if(i==20){
            title = Form("%s_%s_ALLSUM", nameDetector, nameSubDir);
	  }
	  else{
            title = Form("%s_%s_ALLSUM_trig", nameDetector, nameSubDir);
	  }
	}
      }
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  { ///// Hit parttern
    Int_t target_id = getUniqueID(kAC1, 0, kHitPat, 0);
    top_dir->Add(createTH1(++target_id, "AC1_hit_pattern", // 1 origin
			   NumOfSegAC1-4, 0, NumOfSegAC1-4,
			   "PMT No", ""));
  }
  { ///// Multiplicity
    Int_t target_id = getUniqueID(kAC1, 0, kMulti, 0);
    top_dir->Add(createTH1(++target_id, "AC1_multiplicity", // 1 origin
			   NumOfSegAC1-4, 0, NumOfSegAC1-4,
			   "Multiplicity", ""));
  }
  return top_dir;

  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}


// -------------------------------------------------------------------------
// createLC
// -------------------------------------------------------------------------
TList* HistMaker::createLC( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kLC);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  //  // ADC---------------------------------------------------------
  //  {
  //    // Declaration of the sub-directory
  //    TString strSubDir  = CONV_STRING(kADC);
  //    const char* nameSubDir = strSubDir.Data();
  //    TList *sub_dir = new TList;
  //    sub_dir->SetName(nameSubDir);
  //
  //    // Make histogram and add it
  //    Int_t target_id = getUniqueID(kLC, 0, kADC, 0);
  //    for(Int_t i = 0; i<NumOfSegLC*2; ++i){
  //      const char* title = NULL;
  //      if(i < NumOfSegLC){
  //	Int_t seg = i+1; // 1 origin
  //	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
  //      }else{
  //	Int_t seg = i+1-NumOfSegLC; // 1 origin
  //	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
  //      }
  //
  //      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
  //			     0x1000, 0, 0x1000,
  //			     "ADC [ch]", ""));
  //    }
  //
  //    // insert sub directory
  //    top_dir->Add(sub_dir);
  //  }
  //
  //  // ADCwTDC ---------------------------------------------------------
  //  {
  //    // Declaration of the sub-directory
  //    TString strSubDir  = CONV_STRING(kADCwTDC);
  //    const char* nameSubDir = strSubDir.Data();
  //    TList *sub_dir = new TList;
  //    sub_dir->SetName(nameSubDir);
  //
  //    // Make histogram and add it
  //    Int_t target_id = getUniqueID(kLC, 0, kADCwTDC, 0);
  //    for( Int_t i=0; i<NumOfSegLC*2; ++i ){
  //      const char* title = NULL;
  //      if( i<NumOfSegLC ){
  //	Int_t seg = i+1; // 1 origin
  //	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
  //      }else{
  //	Int_t seg = i+1-NumOfSegLC; // 1 origin
  //	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
  //      }
  //
  //      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
  //			     0x1000, 0, 0x1000,
  //			     "ADC [ch]", ""));
  //    }
  //
  //    // insert sub directory
  //    top_dir->Add(sub_dir);
  //  }

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kLC, 0, kTDC, 0);
    //    for(Int_t i = 0; i<NumOfSegLC*2; ++i){
    for(Int_t i = 0; i<NumOfSegLC; ++i){
      const char* title = NULL;
      if(i < NumOfSegLC){
	Int_t seg = i+1; // 1 origin
	title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
	//      }else{
	//	Int_t seg = i+1-NumOfSegLC; // 1 origin
	//	title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
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
    Int_t target_id = getUniqueID(kLC, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegLC, 0, NumOfSegLC,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "LC_multiplicity";
    Int_t target_id = getUniqueID(kLC, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegLC, 0, NumOfSegLC,
			   "Multiplicity", ""));
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createWC
// -------------------------------------------------------------------------
TList* HistMaker::createWC( Bool_t flag_ps )
{
  TString strDet = CONV_STRING(kWC);   // Determine the detector name
  name_created_detectors_.push_back(strDet);    // name list of crearted detector
  if(flag_ps) name_ps_files_.push_back(strDet); // name list which are displayed in Ps tab

  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  { // ADC---------------------------------------------------------
    TString strSubDir  = CONV_STRING(kADC); // Declaration of the sub-directory
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kWC, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfSegWC*3; ++i){
      const char* title = NULL;
      if(i < NumOfSegWC){
	Int_t seg = i+1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else if(i < NumOfSegWC*2){
	Int_t seg = i+1-NumOfSegWC; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i+1-NumOfSegWC*2; // 1 origin
	title = Form("%s_%s_%dSUM", nameDetector, nameSubDir, seg);
      }
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  { // ADCwTDC---------------------------------------------------------
    TString strSubDir  = CONV_STRING(kADCwTDC); // Declaration of the sub-directory
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kWC, 0, kADCwTDC, 0);
    for(Int_t i = 0; i<NumOfSegWC*3; ++i){
      const char* title = NULL;
      if(i < NumOfSegWC){
	Int_t seg = i+1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else if(i < NumOfSegWC*2){
	Int_t seg = i+1-NumOfSegWC; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i+1-NumOfSegWC*2; // 1 origin
	title = Form("%s_%s_%dSUM", nameDetector, nameSubDir, seg);
      }
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADCwTDC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    // Make histogram and add it
    Int_t target_id = getUniqueID(kWC, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegWC*3; ++i){
      const char* title = NULL;
      if(i < NumOfSegWC){
	Int_t seg = i+1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else if(i < NumOfSegWC*2){
	Int_t seg = i+1-NumOfSegWC; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i+1-NumOfSegWC*2; // 1 origin
	title = Form("%s_%s_%dSUM", nameDetector, nameSubDir, seg);
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
    const char* title = "WC_hit_pattern";
    Int_t target_id = getUniqueID(kWC, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegWC, 0, NumOfSegWC,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "WC_multiplicity";
    Int_t target_id = getUniqueID(kWC, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegWC, 0, NumOfSegWC,
			   "Multiplicity", ""));
  }

  return top_dir;
}

// -------------------------------------------------------------------------
// createSAC3
// -------------------------------------------------------------------------
TList* HistMaker::createSAC3( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kSAC3);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // SUM ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSAC3, 0, kADC, 0);
    //for(Int_t i = 0; i<NumOfSegSAC3; ++i){
    for(Int_t i = 0; i<1; ++i){
      //Int_t seg = i+1;
      const char* title = NULL;
      title = Form("%s_%s", nameDetector, nameSubDir);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }
  { // SUM ADCwTDC---------------------------------------------------------
    TString strSubDir  = CONV_STRING(kADCwTDC); // Declaration of the sub-directory
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSAC3, 0, kADCwTDC, 0);
    //for(Int_t i = 0; i<2; ++i){
    for(Int_t i = 0; i<NumOfSegSAC3; ++i){
      Int_t seg = i+1;
      const char* title = NULL;
      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }


  // TDC SUM and INDIVISUAL---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSAC3, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegSAC3; ++i){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
        title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  return top_dir;

  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}


// -------------------------------------------------------------------------
// createSFV
// -------------------------------------------------------------------------
TList* HistMaker::createSFV( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kSFV);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // SUM ADC---------------------------------------------------------
//  {
//    // Declaration of the sub-directory
//    TString strSubDir  = CONV_STRING(kADC);
//    const char* nameSubDir = strSubDir.Data();
//    TList *sub_dir = new TList;
//    sub_dir->SetName(nameSubDir);
//
//    // Make histogram and add it
//    Int_t target_id = getUniqueID(kSFV, 0, kADC, 0);
//    for(Int_t i = 0; i<NumOfSegSAC3; ++i){
//      Int_t seg = i+1;
//      const char* title = NULL;
//      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
//      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
//			     0x1000, 0, 0x1000,
//			     "ADC [ch]", ""));
//    }
//    // insert sub directory
//    top_dir->Add(sub_dir);
//  }
//  { // SUM ADCwTDC---------------------------------------------------------
//    TString strSubDir  = CONV_STRING(kADCwTDC); // Declaration of the sub-directory
//    const char* nameSubDir = strSubDir.Data();
//    TList *sub_dir = new TList;
//    sub_dir->SetName(nameSubDir);
//
//    // Make histogram and add it
//    Int_t target_id = getUniqueID(kSFV, 0, kADCwTDC, 0);
//    for(Int_t i = 0; i<2; ++i){
//      Int_t seg = i+1;
//      const char* title = NULL;
//      title = Form("%s_%s_SUM%d", nameDetector, nameSubDir, seg);
//      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
//			     0x1000, 0, 0x1000,
//			     "ADC [ch]", ""));
//    }
//    top_dir->Add(sub_dir);
//  }


  // TDC SUM and INDIVISUAL---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kSFV, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegSFV; ++i){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
      if(i<6){
        title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      }
      else{
        title = Form("%s_%s_SUM", nameDetector, nameSubDir);
      }
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  { ///// Hit parttern
    Int_t target_id = getUniqueID(kSFV, 0, kHitPat, 0);
    top_dir->Add(createTH1(++target_id, "SFV_hit_pattern", // 1 origin
			   NumOfSegSFV-1, 0, NumOfSegSFV-1,
			   "Segment", ""));
  }
  { ///// Multiplicity
    Int_t target_id = getUniqueID(kSFV, 0, kMulti, 0);
    top_dir->Add(createTH1(++target_id, "SFV_multiplicity", // 1 origin
			   NumOfSegSFV-1, 0, NumOfSegSFV-1,
			   "Multiplicity", ""));
  }
  return top_dir;

  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}


// -------------------------------------------------------------------------
// createT1
// -------------------------------------------------------------------------
TList* HistMaker::createT1( Bool_t flag_ps )
{
  TString strDet = CONV_STRING(kT1);   // Determine the detector name
  name_created_detectors_.push_back(strDet);    // name list of crearted detector
  if(flag_ps) name_ps_files_.push_back(strDet); // name list which are displayed in Ps tab

  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  { // ADC---------------------------------------------------------
    TString strSubDir  = CONV_STRING(kADC); // Declaration of the sub-directory
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kT1, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfSegT1*2; ++i){
      const char* title = NULL;
      if(i < NumOfSegT1){
	Int_t seg = i+1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i+1-NumOfSegT1; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kT1, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegT1; ++i){
      const char* title = NULL;
      if(i < NumOfSegT1){
	Int_t seg = i+1; // 1 origin
	title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
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
    const char* title = "T1_hit_pattern";
    Int_t target_id = getUniqueID(kT1, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegT1, 0, NumOfSegT1,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "T1_multiplicity";
    Int_t target_id = getUniqueID(kT1, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegT1, 0, NumOfSegT1,
			   "Multiplicity", ""));
  }

  return top_dir;
}
// -------------------------------------------------------------------------
// createT2
// -------------------------------------------------------------------------
TList* HistMaker::createT2( Bool_t flag_ps )
{
  TString strDet = CONV_STRING(kT2);   // Determine the detector name
  name_created_detectors_.push_back(strDet);    // name list of crearted detector
  if(flag_ps) name_ps_files_.push_back(strDet); // name list which are displayed in Ps tab

  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  { // ADC---------------------------------------------------------
    TString strSubDir  = CONV_STRING(kADC); // Declaration of the sub-directory
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kT2, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfSegT2*2; ++i){
      const char* title = NULL;
      if(i < NumOfSegT2){
	Int_t seg = i+1; // 1 origin
	title = Form("%s_%s_%dU", nameDetector, nameSubDir, seg);
      }else{
	Int_t seg = i+1-NumOfSegT2; // 1 origin
	title = Form("%s_%s_%dD", nameDetector, nameSubDir, seg);
      }
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kT2, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegT2; ++i){
      const char* title = NULL;
      if(i < NumOfSegT2){
	Int_t seg = i+1; // 1 origin
	title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
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
    const char* title = "T2_hit_pattern";
    Int_t target_id = getUniqueID(kT2, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegT2, 0, NumOfSegT2,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "T2_multiplicity";
    Int_t target_id = getUniqueID(kT2, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegT2, 0, NumOfSegT2,
			   "Multiplicity", ""));
  }

  return top_dir;
}


// -------------------------------------------------------------------------
// createMsT
// -------------------------------------------------------------------------
TList* HistMaker::createMsT( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kMsT);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // HR-TDC ---------------------------------------------------------
  {
    TList *sub_dir = new TList;
    sub_dir->SetName("MsT_TOF_TDC");
    Int_t target_id = getUniqueID(kMsT, 0, kTDC, 0);
    for(Int_t seg=0; seg<NumOfSegTOF; ++seg){
      sub_dir->Add(createTH1(++target_id, Form("%s_TOF_TDC_%d", nameDetector, seg+1),
			     10000, 0, 400000,
			     "TDC", ""));
    }
    top_dir->Add(sub_dir);
  }

#if 0
  // HR-TDC (accept) ---------------------------------------------------------
  {
    TList *sub_dir = new TList;
    sub_dir->SetName("MsT_TOF_TDC_Acc");
    Int_t target_id = getUniqueID(kMsT, 0, kTDC, NumOfSegTOF);
    for(Int_t seg=0; seg<NumOfSegTOF; ++seg){
      sub_dir->Add(createTH1(++target_id, Form("%s_TOF_TDC_%d_Acc", nameDetector, seg+1),
			     10000, 0, 400000,
			     "TDC", ""));
    }
    top_dir->Add(sub_dir);
  }
#endif

  // LR-TDC ---------------------------------------------------------
  {
    TList *sub_dir = new TList;
    sub_dir->SetName("MsT_SCH_TDC");
    Int_t target_id = getUniqueID(kMsT, 0, kTDC, NumOfSegTOF*2);
    for(Int_t seg=0; seg<NumOfSegSCH; ++seg){
      sub_dir->Add(createTH1(++target_id, Form("%s_SCH_TDC_%d", nameDetector, seg+1),
			     50, 0, 50,
			     "TDC", ""));
    }
    top_dir->Add(sub_dir);
  }

  // Timing Counter TDC 2D -------------------------------------------------------
  {
    Int_t target_id = getUniqueID(kMsT, 0, kTDC2D, 0);
    top_dir->Add(createTH2(++target_id, Form("%s_TOF_TDC_2D", nameDetector),
			   NumOfSegTOF, 0, NumOfSegTOF, 10000, 0, 400000,
			   "Segment", ""));
  }

  // HitPattern  ----------------------------------------------------
  {
    Int_t target_id = getUniqueID(kMsT, 0, kHitPat, 0);
    top_dir->Add(createTH1(target_id, Form("%s_TOF_HitPat", nameDetector),
			   NumOfSegTOF, 0, NumOfSegTOF,
			   "Segment", ""));
    top_dir->Add(createTH1(target_id + 1, Form("%s_SCH_HitPat", nameDetector),
			   NumOfSegSCH, 0, NumOfSegSCH,
			   "Segment", ""));
    top_dir->Add(createTH1(target_id + 2, "MsT_Flag",
			   10, 0., 10., ""));
  }

  // Hit parttern 2D Flag --------------------------------------------
  {
    Int_t target_id = getUniqueID(kMsT, 0, kHitPat2D, 0);
    top_dir->Add(createTH2(target_id, "MsT_Flag_2D", // 1 origin
			   4, 0., 4., 4, 0., 4.,
			   "MsT Flag", "Soft Flag  "));
    GHist::get(target_id)->SetMarkerSize(6);
  }

  return top_dir;
}

// -------------------------------------------------------------------------
// createMsT_T0
// -------------------------------------------------------------------------
TList* HistMaker::createMsT_T0( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kMsT_T0);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // HR-TDC ---------------------------------------------------------
  {
    TList *sub_dir = new TList;
    sub_dir->SetName("MsT_T0_TOF_TDC");
    Int_t target_id = getUniqueID(kMsT_T0, 0, kTDC, 0);
    for(Int_t seg=0; seg<NumOfSegTOF; ++seg){
      sub_dir->Add(createTH1(++target_id, Form("%s_TOF_TDC_%d", nameDetector, seg+1),
			     10000, 0, 400000,
			     "TDC", ""));
    }
    top_dir->Add(sub_dir);
  }

  return top_dir;
}

//_____________________________________________________________________________
TList*
HistMaker::createMatrix(Bool_t flag_ps)
{
  TString strDet = CONV_STRING(kMtx3D);
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    name_ps_files_.push_back(strDet);
  }

  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  ///// Mtx2D
  Int_t mtx2d_id = getUniqueID(kMisc, kHul2D, kHitPat2D);
  for(Int_t i=0; i<2; ++i){
    top_dir->Add(createTH2(mtx2d_id + i, Form("Mtx2D pattern #%d", i+1),
			   NumOfSegSCH, 0, NumOfSegSCH,
			   NumOfSegTOF, 0, NumOfSegTOF,
			   "SCH seg", "TOF seg"));
  }
  ///// Mtx3D
  Int_t mtx3d_id = getUniqueID(kMisc, kHul3D, kHitPat2D);
  for(Int_t i=0; i<NumOfSegBH2; ++i){
    top_dir->Add(createTH2(mtx3d_id+i, Form("Mtx3D pattern BH2-%d", i),
			   NumOfSegSCH, 0, NumOfSegSCH,
			   NumOfSegTOF+NumOfSegBVH, 0, NumOfSegTOF+NumOfSegBVH,
			   "SCH seg", "TOF seg"));
  }

  // // TDC TOF ---------------------------------------------------------------
  // {
  //   TList *sub_dir = new TList;
  //   sub_dir->SetName("Mtx_TOF_TDC");
  //   Int_t target_id = getUniqueID(kMtx3D, kHulTOF, kTDC, 0);
  //   for(Int_t seg=0; seg<NumOfSegTOF; ++seg){
  //     sub_dir->Add(createTH1(++target_id, Form("%s_TOF_TDC_%d", nameDetector, seg+1),
  // 			     0xff, 0, 0xff,
  // 			     "TDC [5ns/bin]", ""));
  //   }
  //   top_dir->Add(sub_dir);
  // }

  // // TDC FBH ---------------------------------------------------------------
  // {
  //   TList *sub_dir = new TList;
  //   sub_dir->SetName("Mtx_FBH_TDC");
  //   Int_t target_id = getUniqueID(kMtx3D, kHulFBH, kTDC, 0);
  //   for(Int_t seg=0; seg<NumOfSegClusteredFBH; ++seg){
  //     sub_dir->Add(createTH1(++target_id, Form("%s_FBH_TDC_%d", nameDetector, seg+1),
  // 			     0xff, 0, 0xff,
  // 			     "TDC [5ns/bin]", ""));
  //   }
  //   top_dir->Add(sub_dir);
  // }

  // // TDC SCH ---------------------------------------------------------------
  // {
  //   TList *sub_dir = new TList;
  //   sub_dir->SetName("Mtx_SCH_TDC");
  //   Int_t target_id = getUniqueID(kMtx3D, kHulSCH, kTDC, 0);
  //   for(Int_t seg=0; seg<NumOfSegSCH; ++seg){
  //     sub_dir->Add(createTH1(++target_id, Form("%s_SCH_TDC_%d", nameDetector, seg+1),
  // 			     0xff, 0, 0xff,
  // 			     "TDC [5ns/bin]", ""));
  //   }
  //   top_dir->Add(sub_dir);
  // }

  // // Hit parttern 2D (TOFxFBH) --------------------------------------------
  // {
  //   Int_t target_id = getUniqueID(kMtx3D, kHulTOFxFBH, kHitPat2D, 0);
  //   top_dir->Add(createTH2(++target_id, "Mtx_TOFxFBH", // 1 origin
  // 			   NumOfSegClusteredFBH, 0, NumOfSegClusteredFBH,
  // 			   NumOfSegTOF,          0, NumOfSegTOF,
  // 			   "Clustered FBH seg", "TOF seg"));
  //   GHist::get(target_id)->SetMarkerSize(6);
  // }

  // // Hit parttern 2D (TOFxSCH) --------------------------------------------
  // {
  //   Int_t target_id = getUniqueID(kMtx3D, kHulTOFxSCH, kHitPat2D, 0);
  //   top_dir->Add(createTH2(++target_id, "Mtx_TOFxSCH", // 1 origin
  // 			   NumOfSegSCH, 0, NumOfSegSCH,
  // 			   NumOfSegTOF, 0, NumOfSegTOF,
  // 			   "SCH seg", "TOF seg"));
  //   GHist::get(target_id)->SetMarkerSize(6);
  // }

  // // Hit parttern 2D (FBHxSCH) --------------------------------------------
  // {
  //   Int_t target_id = getUniqueID(kMtx3D, kHulFBHxSCH, kHitPat2D, 0);
  //   top_dir->Add(createTH2(++target_id, "Mtx_FBHxSCH", // 1 origin
  // 			   NumOfSegSCH,          0, NumOfSegSCH,
  // 			   NumOfSegClusteredFBH, 0, NumOfSegClusteredFBH,
  // 			   "SCH seg","Clustered FBH seg"));
  //   GHist::get(target_id)->SetMarkerSize(6);
  // }

  // // Hit parttern 2D (TOFxSCH FBH required) ----------------------------
  // {
  //   TList *sub_dir = new TList;
  //   sub_dir->SetName("Mtx2D_TOF_FBH");
  //   Int_t target_id = getUniqueID(kMtx3D, kHul2DHitPat, kHitPat2D, 0);
  //   sub_dir->Add(createTH2(++target_id, "Mtx2D_TOFxSCH", // 1 origin
  // 			   NumOfSegSCH, 0, NumOfSegSCH,
  // 			   NumOfSegTOF, 0, NumOfSegTOF,
  // 			   "SCH seg", "TOF seg"));
  //   GHist::get(target_id)->SetMarkerSize(6);
  //   top_dir->Add(sub_dir);
  // }

  // // Hit parttern 3D (TOFxSCH FBH required) ----------------------------
  // {
  //   TList *sub_dir = new TList;
  //   sub_dir->SetName("Mtx3D_TOF_FBH");
  //   Int_t target_id = getUniqueID(kMtx3D, kHul3DHitPat, kHitPat2D, 0);
  //   for(Int_t i = 0; i<NumOfSegClusteredFBH; ++i){
  //     sub_dir->Add(createTH2(++target_id, Form("Mtx3D_TOFxSCH_FBH%d",i), // 1 origin
  // 			     NumOfSegSCH, 0, NumOfSegSCH,
  // 			     NumOfSegTOF, 0, NumOfSegTOF,
  // 			     "SCH seg", "TOF seg"));
  //     GHist::get(target_id)->SetMarkerSize(6);
  //   }// for(i)
  //   top_dir->Add(sub_dir);
  // }

  // // Hit parttern 2D (Flag) --------------------------------------------
  // {
  //   Int_t target_id = getUniqueID(kMtx3D, kHul2D, kHitPat2D, 0);
  //   top_dir->Add(createTH2(++target_id, "Mtx_2D_FLAG", // 1 origin
  // 			   3, 0., 3., 3, 0., 3.,
  // 			   "Trigger Flag", "HUL Accept"));
  //   GHist::get(target_id)->SetMarkerSize(6);
  // }

  // // Hit parttern 3D (Flag) --------------------------------------------
  // {
  //   Int_t target_id = getUniqueID(kMtx3D, kHul3D, kHitPat2D, 0);
  //   top_dir->Add(createTH2(++target_id, "Mtx_3D_FLAG", // 1 origin
  // 			   3, 0., 3., 3, 0., 3.,
  // 			   "Trigger Flag", "HUL Accept"));
  //   GHist::get(target_id)->SetMarkerSize(6);
  // }

  return top_dir;
}

//_____________________________________________________________________________
TList*
HistMaker::createTPC(Bool_t flag_ps)
{
  TString strDet = CONV_STRING(kTPC);
  name_created_detectors_.push_back(strDet);
  if( flag_ps ) name_ps_files_.push_back(strDet);
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);
  {
    Int_t target_id = getUniqueID( kTPC, 0, kADC2D );
    auto title = Form( "%s_ADC2D", nameDetector );
    auto h_adc = dynamic_cast<TH2Poly*>
      ( createTH2Poly( target_id++, title, -300, 300, -300, 300 ) );
    title = Form( "%s_RMS2D", nameDetector );
    auto h_rms = dynamic_cast<TH2Poly*>
      ( createTH2Poly( target_id++, title, -300, 300, -300, 300 ) );
    title = Form( "%s_TDC2D", nameDetector );
    auto h_loc = dynamic_cast<TH2Poly*>
      ( createTH2Poly( target_id++, title, -300, 300, -300, 300 ) );
    title = Form( "%s_Hit2D", nameDetector );
    auto h_hit = dynamic_cast<TH2Poly*>
      ( createTH2Poly( target_id++, title, -300, 300, -300, 300 ) );
    Double_t X[5];
    Double_t Y[5];
    for( Int_t i=0; i<32; i++ ){
      Double_t pLength = TpcPadHelper::PadParameter[i][5];
      Double_t st      = ( 180. -
                           360./TpcPadHelper::PadParameter[i][3] *
                           TpcPadHelper::PadParameter[i][1]/2. );
      Double_t sTheta  = (-1+st/180.)*TMath::Pi();
      Double_t dTheta  = ( (360./TpcPadHelper::PadParameter[i][3]) /
                           180.*TMath::Pi() );
      Double_t cRad    = TpcPadHelper::PadParameter[i][2];
      Int_t    nPad    = TpcPadHelper::PadParameter[i][1];
      for( Int_t j=0; j<nPad; j++ ){
        X[1] = (cRad+(pLength/2.))*TMath::Cos(j*dTheta+sTheta);
        X[2] = (cRad+(pLength/2.))*TMath::Cos((j+1)*dTheta+sTheta);
        X[3] = (cRad-(pLength/2.))*TMath::Cos((j+1)*dTheta+sTheta);
        X[4] = (cRad-(pLength/2.))*TMath::Cos(j*dTheta+sTheta);
        X[0] = X[4];
        Y[1] = (cRad+(pLength/2.))*TMath::Sin(j*dTheta+sTheta);
        Y[2] = (cRad+(pLength/2.))*TMath::Sin((j+1)*dTheta+sTheta);
        Y[3] = (cRad-(pLength/2.))*TMath::Sin((j+1)*dTheta+sTheta);
        Y[4] = (cRad-(pLength/2.))*TMath::Sin(j*dTheta+sTheta);
        Y[0] = Y[4];
        for( Int_t ii=0; ii<5; ii++ ) X[ii] -=143;
        h_adc->AddBin( 5, X, Y );
        h_rms->AddBin( 5, X, Y );
        h_loc->AddBin( 5, X, Y );
        h_hit->AddBin( 5, X, Y );
      }
    }
    h_adc->SetStats( 0 );
    h_adc->SetMinimum(    0. );
    h_adc->SetMaximum( 4000. );
    h_rms->SetStats( 0 );
    h_rms->SetMinimum(    0. );
    h_rms->SetMaximum(  200. );
    h_loc->SetStats( 0 );
    h_loc->SetMinimum(    0. );
    h_loc->SetMaximum(  NumOfTimeBucket );
    top_dir->Add( h_adc );
    top_dir->Add( h_rms );
    top_dir->Add( h_loc );
    top_dir->Add( h_hit );
  }
  // ADC
  top_dir->Add( createTH1( getUniqueID( kTPC, 0, kADC ),
                           "TPC_ADC", 4000, 0, 4000 ) );
  top_dir->Add( createTH1( getUniqueID( kTPC, 0, kPede ),
                           "TPC_RMS", 1000, 0, 1000 ) );
  // TDC
  top_dir->Add( createTH1( getUniqueID( kTPC, 0, kTDC ),
                           "TPC_TDC", NumOfTimeBucket, 0, NumOfTimeBucket ) );
  top_dir->Add( createTH2( getUniqueID( kTPC, 0, kTDC2D ),
                           "TPC_Hit_ZY", 100, -300, 300,
			   NumOfTimeBucket, 0, NumOfTimeBucket,
			   "z [mm]", "tb" ) );
  top_dir->Add( createTH2( getUniqueID( kTPC, 1, kTDC2D ),
                           "TPC_Hit_XY", 100, -300, 300,
			   NumOfTimeBucket, 0, NumOfTimeBucket,
			   "x [mm]", "tb" ) );
  top_dir->Add( createTH2( getUniqueID( kTPC, 2, kTDC2D ),
                           "TPC_ZY", 100, -300, 300,
			   NumOfTimeBucket, 0, NumOfTimeBucket,
			   "z [mm]", "tb" ) );
  top_dir->Add( createTH2( getUniqueID( kTPC, 3, kTDC2D ),
                           "TPC_XY", 100, -300, 300,
			   NumOfTimeBucket, 0, NumOfTimeBucket,
			   "x [mm]", "tb" ) );
  // FADC
  top_dir->Add( createTH2( getUniqueID( kTPC, 0, kFADC ),
                           "TPC_FADC",
                           NumOfTimeBucket, 0, NumOfTimeBucket,
			   200, 0, 0x1000,
                           "Time bucket", "ADC" ) );
  // Multiplicity
  top_dir->Add( createTH1( getUniqueID( kTPC, 0, kMulti ),
                           "TPC_multiplicity", 600, 0, 6000 ) );
  top_dir->Add( createTH1( getUniqueID( kTPC, 3, kMulti ),
                           "TPC_AGET_multiplicity", 124, 0, 124,
			   "AsAdID#times4+AGETID", "Multiplicity/AGET/Event" ) );
  top_dir->Add( createTH1( getUniqueID( kTPC, 4, kMulti ),
                           "TPC_AGET_multiplicity_Max", 64, 0, 64 ) );
  // ClusterSize
  top_dir->Add( createTH2( getUniqueID( kTPC, 2, kMulti ),
                           "TPC_ClusterSize", 42, -10, 32, 10, 0, 10,
			   "Layer ID", "Cluster size") );
  // TPC-CLOCK
  top_dir->Add( createTH1( getUniqueID( kTPC, 1, kTDC ),
			   "TPC_CLOCK", 50000, 0, 2000000,
			   "TDC", "") );
  top_dir->Add( createTH1( getUniqueID( kTPC, 1, kMulti ),
			   "TPC_CLOCK_multiplicity", 10, 0, 10) );
  // TPC-BeamProfile
  //  top_dir->Add( createTH1( getUniqueID( kTPC, 2, kTDC ),
  //		 	  "TPC_BeamProfile", 34, 0, 34,
  //		 	  "Pad", "") );
  top_dir->Add( createTH1( getUniqueID( kTPC, 2, kTDC ),
			   "TPC_BeamProfile", 20, -100, 100,
			   "x [mm]", "") );
  return top_dir;
}

//_____________________________________________________________________________
TList*
HistMaker::createBVH(Bool_t flag_ps)
{
  TString strDet("BVH");
  name_created_detectors_.push_back(strDet);
  if(flag_ps)
    name_ps_files_.push_back(strDet);
  auto top_dir = new TList;
  top_dir->SetName(strDet);
  { ///// TDC
    TString strSubDir("TDC");
    auto sub_dir = new TList;
    sub_dir->SetName(strSubDir);
    const auto target_id = getUniqueID(kBVH, 0, kTDC);
    for(Int_t i=0; i<NumOfSegBVH; ++i){
      TString title = Form("%s_%s_%d", strDet.Data(), strSubDir.Data(), i+1);
      sub_dir->Add(createTH1(target_id+i, title,
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }
  { ///// Hit parttern
    TString title("BVH_hit_pattern");
    const auto target_id = getUniqueID(kBVH, 0, kHitPat);
    top_dir->Add(createTH1(target_id, title,
			   NumOfSegBVH, 0, NumOfSegBVH,
			   "Segment", ""));
  }
  { ///// Multiplicity
    TString title("BVH_multiplicity");
    const auto target_id = getUniqueID(kBVH, 0, kMulti);
    top_dir->Add(createTH1(target_id, title,
			   NumOfSegBVH, 0, NumOfSegBVH,
			   "Multiplicity", ""));
  }
  return top_dir;
}


// -------------------------------------------------------------------------
// createTriggerFlag_E07
// -------------------------------------------------------------------------
TList* HistMaker::createTriggerFlag_E07( Bool_t flag_ps )
{
  // Determine the detector name
  //TString strDet = CONV_STRING(kTriggerFlag);
  TString strDet = "TriggerFlag_E07";
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // TDC---------------------------------------------------------
  {
    // Make histogram and add it
    Int_t target_id = getUniqueID(kTriggerFlag, 1, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegTFlag; ++i){
      const char* title = NULL;
      title = Form("%s_%d", nameDetector, i+1);
      top_dir->Add(createTH1(++target_id, title, // 1 origin
			     400, 0, 4000,
			     "TDC [ch]", ""));
    }
  }

  // Hit parttern -----------------------------------------------
  {
    const char* title = "Trigger_Entry_E07";
    Int_t target_id = getUniqueID(kTriggerFlag, 1, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(++target_id, title, // 1 origin
			   20, 0, 20,
			   "Trigger flag", ""));
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}


 //_____________________________________________________________________________
TList*
HistMaker::createBeamProfileE42( Bool_t flag_ps )
{
  auto top_dir = new TList;
  const char* top_name = "BeamProfile";
  top_dir->SetName(top_name);
  for( Int_t ip=0; ip<NParticleType; ++ip ){
    auto sub_dir = new TList;
    sub_dir->SetName(ParticleName[ip]);
    int unique_id = getUniqueID(kMisc, ip, kHitPat);
    // Profile X
    for(int i = 0; i<NProfile; ++i){
      char* title = Form("%s FF %.0f X (%s)", top_name, Profiles[i],
			 ParticleName[ip].Data());
      sub_dir->Add(createTH1(unique_id++, title,
			     400,-200,200,
			     "x position [mm]", ""));
    }
    // Profile X (Fit)
    for(int i = 0; i<NProfile; ++i){
      char* title = Form("%s FF %d X Fit (%s)", top_name, (int)Profiles[i],
			 ParticleName[ip].Data());
      sub_dir->Add(createTH1(unique_id++, title,
			     400,-200,200,
			     "x position [mm]", ""));
    }
    // Profile Y
    for(int i = 0; i<NProfile; ++i){
      char* title = Form("%s FF %d Y (%s)", top_name, (int)Profiles[i],
			 ParticleName[ip].Data());
      sub_dir->Add(createTH1(unique_id++, title,
			     200,-100,100,
			     "y position [mm]", ""));
    }
    // Profile Y (Fit)
    for(int i = 0; i<NProfile; ++i){
      char* title = Form("%s FF %d Y Fit (%s)", top_name, (int)Profiles[i],
			 ParticleName[ip].Data());
      sub_dir->Add(createTH1(unique_id++, title,
			     200,-100,100,
			     "y position [mm]", ""));
    }
    // Profile XY
    for(int i = 0; i<NProfile; ++i){
      char* title = Form("%s FF %d Y%%X (%s)", top_name, (int)Profiles[i],
			 ParticleName[ip].Data());
      sub_dir->Add(createTH2(unique_id++, title,
			     100, -200, 200, 100, -100, 100,
			     "x position [mm]", "y position [mm]"));
    }
    sub_dir->Add(createTH2(unique_id++, Form("BcOut FF 0 U%%X (%s)",
					     ParticleName[ip].Data()),
			   100, -200., 200., 100, -0.2, 0.2,
			   "x position [mm]", "u0"));
    sub_dir->Add(createTH2(unique_id++, Form("BcOut FF 0 V%%Y (%s)",
					     ParticleName[ip].Data()),
			   100, -100., 100., 100, -0.2, 0.2,
			   "y0 [mm]", "v0"));
    sub_dir->Add( createTH2( getUniqueID(kMisc, ip, kHitPat2D, 1),
			     Form("BcOut_BH2 (%s)", ParticleName[ip].Data()),
			     400, -200, 200,
			     NumOfSegBH2+1, 0, NumOfSegBH2+1,
			     "x position [mm]", "Segment" ) );
    sub_dir->Add( createTH1( getUniqueID(kMisc, ip, kMulti, 1),
			     Form("BcOut_NTracks (%s)", ParticleName[ip].Data()),
			     10, 0., 10. ) );
    sub_dir->Add( createTH1( getUniqueID(kMisc, ip, kChisqr, 1),
			     Form("BcOut_Chisqr (%s)", ParticleName[ip].Data()),
			     100, 0., 20. ) );
    top_dir->Add(sub_dir);
  }
  return top_dir;
}


//_____________________________________________________________________________
TList*
HistMaker::createDCEff( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING( kDCEff );
  // name list of crearted detector
  name_created_detectors_.push_back( strDet );
  if( flag_ps ){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back( strDet );
  }

  return NULL;
}

//_____________________________________________________________________________
TList*
HistMaker::createBTOF( Bool_t flag_ps )
{
  TString det_name = "BTOF";
  auto top_dir = new TList;
  top_dir->SetName(det_name);
  const auto hid = getUniqueID(kMisc, 0, kTDC);
  top_dir->Add(createTH1(hid, det_name,
			 300, -10, 5, "[ns]", ""));
  top_dir->Add(createTH1(hid+1, "BH1-6_BH2-4",
			 600, 50000, 350000, "[ch]", ""));
  return top_dir;
}

// -------------------------------------------------------------------------
// Old functions
// -------------------------------------------------------------------------
TList* HistMaker::createBAC_SAC( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kBAC_SAC);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // ACs configuration
  const char* name_acs[] = {"BAC0a", "BAC0b", "BAC1", "BAC2", "SAC1", "SAC2"};

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kBAC_SAC, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfSegBAC_SAC; ++i){
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
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kBAC_SAC, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegBAC_SAC; ++i){
      const char* title = NULL;
      title = Form("%s_%s", name_acs[i], nameSubDir);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     //			     0x1000, 0, 0x1000,
			     1500, 0, 1500,
			     "TDC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

//TList* HistMaker::createSFV_SAC3( Bool_t flag_ps )
//{
//  // Determine the detector name
//  TString strDet = CONV_STRING(kSFV_SAC3);
//  // name list of crearted detector
//  name_created_detectors_.push_back(strDet);
//  if(flag_ps){
//    // name list which are displayed in Ps tab
//    name_ps_files_.push_back(strDet);
//  }
//
//  // Declaration of the directory
//  // Just type conversion from TString to char*
//  const char* nameDetector = strDet.Data();
//  TList *top_dir = new TList;
//  top_dir->SetName(nameDetector);
//
//  const Int_t NofLoop = 7;
//  // ADC---------------------------------------------------------
//  {
//    // Declaration of the sub-directory
//    TString strSubDir  = CONV_STRING(kADC);
//    const char* nameSubDir = strSubDir.Data();
//    TList *sub_dir = new TList;
//    sub_dir->SetName(nameSubDir);
//
//    // Make histogram and add it
//    Int_t target_id = getUniqueID(kSFV_SAC3, 0, kADC, 0);
//    for(Int_t i = 0; i<NofLoop; ++i){
//      const char* title = NULL;
//      if(i<NofLoop-1){
//	Int_t seg = i+1; // 1 origin
//	title = Form("%s_%s_%d", "SFV", nameSubDir, seg);
//      }else{
//	title = Form("%s_%s", "SAC3", nameSubDir);
//      }
//
//      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
//			     0x1000, 0, 0x1000,
//			     "ADC [ch]", ""));
//    }
//
//    // insert sub directory
//    top_dir->Add(sub_dir);
//  }
//
//  // TDC---------------------------------------------------------
//  {
//    // Declaration of the sub-directory
//    TString strSubDir  = CONV_STRING(kTDC);
//    const char* nameSubDir = strSubDir.Data();
//    TList *sub_dir = new TList;
//    sub_dir->SetName(nameSubDir);
//
//    // Make histogram and add it
//    Int_t target_id = getUniqueID(kSFV_SAC3, 0, kTDC, 0);
//    for(Int_t i = 0; i<NofLoop; ++i){
//      const char* title = NULL;
//      if(i<NofLoop-1){
//	Int_t seg = i+1; // 1 origin
//	title = Form("%s_%s_%d", "SFV", nameSubDir, seg);
//      }else{
//	title = Form("%s_%s", "SAC3", nameSubDir);
//      }
//
//      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
//			     0x1000, 0, 0x1000,
//			     "TDC [ch]", ""));
//    }
//
//    // insert sub directory
//    top_dir->Add(sub_dir);
//  }
//
//  // Hit parttern -----------------------------------------------
//  {
//    const char* title = "SFV_SAC3_hit_pattern";
//    Int_t target_id = getUniqueID(kSFV_SAC3, 0, kHitPat, 0);
//    // Add to the top directory
//    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
//			   NofLoop, 0, NofLoop,
//			   "Segment", ""));
//  }
//
//  // Return the TList pointer which is added into TGFileBrowser
//  return top_dir;
//}

//_____________________________________________________________________________
TList*
HistMaker::createGe( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kGe);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }
  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);
  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    // Make histogram and add it
    Int_t target_id = getUniqueID(kGe, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x2000, 0, 0x2000,
			     "ADC [ch]", ""));
    }
    // 2D histogram
    target_id = getUniqueID(kGe, 0, kADC2D, 0);
    sub_dir->Add(createTH2(++target_id, "Ge_ADC_2D", // 1 origin
			   NumOfSegGe, 0, NumOfSegGe,
			   0x800, 0, 0x2000,
			   "Ge segment", "ADC [ch]"));
    // HitPat
    target_id = getUniqueID(kGe, 0, kHitPat, 0);
    sub_dir->Add(createTH1(++target_id, "Ge_HitPat (using ADC)", // 1 origin
			   NumOfSegGe, 0, NumOfSegGe,
			   "Ge segment", ""));
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // ADCwTDC ---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kADCwTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);
    Int_t target_id = getUniqueID(kGe, 0, kADCwTDC, 0);
    for(Int_t i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      Int_t seg = i+1;
      title = Form("%s_ADCwTFA_%d", nameDetector, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x2000, 0, 0x2000,
			     "ADC [ch]", ""));
    }

    target_id = getUniqueID(kGe, 0, kADCwTDC, NumOfSegGe );
    for(Int_t i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      Int_t seg = i+1;
      title = Form("%s_ADCwCRM_%d", nameDetector, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x2000, 0, 0x2000,
			     "ADC [ch]", ""));
    }

    target_id = getUniqueID(kGe, 0, kADCwTDC, NumOfSegGe*2 );
    for(Int_t i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      Int_t seg = i+1;
      title = Form("%s_ADCwTFAwFlag1_SpillOn_%d", nameDetector, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x2000,
			     "ADC [ch]", ""));
    }

    target_id = getUniqueID(kGe, 0, kADCwTDC, NumOfSegGe*3 );
    for(Int_t i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      Int_t seg = i+1;
      title = Form("%s_ADCwTFAwFlag1_SpillOff_%d", nameDetector, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x2000,
			     "ADC [ch]", ""));
    }

    target_id = getUniqueID(kGe, 0, kADCwTDC, NumOfSegGe*4 );
    for(Int_t i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      Int_t seg = i+1;
      title = Form("%s_ADCwTFAwFlag2_SpillOn_%d", nameDetector, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x2000,
			     "ADC [ch]", ""));
    }

    target_id = getUniqueID(kGe, 0, kADCwTDC, NumOfSegGe*5 );
    for(Int_t i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      Int_t seg = i+1;
      title = Form("%s_ADCwTFAwFlag2_SpillOff_%d", nameDetector, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x2000,
			     "ADC [ch]", ""));
    }

    top_dir->Add(sub_dir);
  }
  // CRM---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kCRM);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kGe, 0, kCRM, 0);
    for(Int_t i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
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
    TString strSubDir  = CONV_STRING(kTFA);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kGe, 0, kTFA, 0);
    for(Int_t i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
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

  // RST---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kRST);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kGe, 0, kRST, 0);
    for(Int_t i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     2000, 0, 10000,
			     "RST [ch]", ""));
    }

    // 2D histogram
    target_id = getUniqueID(kGe, 0, kRST2D, 0);
    sub_dir->Add(createTH2(++target_id, "Ge_RST_2D", // 1 origin
			   NumOfSegGe, 0, NumOfSegGe,
			   2000, 0, 10000,
			   "Ge segment", "RST [ch]"));

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TFA_ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTFA_ADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kGe, 0, kTFA_ADC, 0);
    for(Int_t i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH2(target_id + i+1, title, // 1 origin
			     400, 0, 10000,
			     400, 0, 8000,
			     "Ge_TFA", "Ge_ADC"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // CRM_ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kCRM_ADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kGe, 0, kCRM_ADC, 0);
    for(Int_t i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH2(target_id + i+1, title, // 1 origin
                             400, 0, 10000,
                             400, 0, 8000,
                             "Ge_CRM", "Ge_ADC"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // RST_ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kRST_ADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kGe, 0, kRST_ADC, 0);
    for(Int_t i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH2(target_id + i+1, title, // 1 origin
			     200, 0, 10000,
			     200, 0, 8000,
			     "Ge_RST", "Ge_ADC"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TFA_CRM---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTFA_CRM);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kGe, 0, kTFA_CRM, 0);
    for(Int_t i = 0; i<NumOfSegGe; ++i){
      const char* title = NULL;
      Int_t seg = i+1; // 1 origin
      title = Form("%s_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH2(target_id + i+1, title, // 1 origin
			     200, 0, 10000,
			     200, 0, 10000,
			     "Ge_TFA", "Ge_CRM"));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HBX Trigger Flag---------------------------------------------------------
  // Declaration of the sub-directory
  TString strSubDir  = CONV_STRING(kADC);
  const char* nameSubDir = strSubDir.Data();
  TList *sub_dir = new TList;
  sub_dir->SetName(nameSubDir);

  // TDC---------------------------------------------------------
  {
    // Make histogram and add it
    Int_t target_id = getUniqueID(kGe, 0, kFlagTDC, 0);
    for(Int_t i = 0; i<NumOfSegHbxTrig; ++i){
      // title = Form("%s_%d", nameDetector, i+1);
      TString title = Form( "HBX_TriggerFlag %d", i );
      top_dir->Add(createTH1(++target_id, title, // 1 origin
			     400, 0, 10000,
			     "TDC [ch]", ""));
    }
  }

  // Hit parttern -----------------------------------------------
  {
    const char* title = "HBX_Trigger_Entry";
    Int_t target_id = getUniqueID(kGe, 0, kFlagHitPat, 0);
    // Add to the top directory
    auto h = createTH1(++target_id, title, // 1 origin
		       NumOfSegHbxTrig+1, 0., NumOfSegHbxTrig+1,
		       "HBX Trigger flag", "");
    // for( Int_t i=0, n=trigger::STriggerFlag.size(); i<n; ++i ){
    //   h->GetXaxis()->SetBinLabel( i+1, trigger::STriggerFlag.at(i) );
    // }
    top_dir->Add( h );
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

TList* HistMaker::createPWO( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kPWO);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kPWO, 0, kADC2D, 0);
    for(Int_t i = 0; i<NumOfBoxPWO; ++i){
      const char* title = NULL;
      Int_t box = i+1; // 1 origin
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
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // sum hist
    Int_t target_id = getUniqueID(kPWO, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfBoxPWO; ++i){
      const char* title = NULL;
      Int_t box = i+1; // 1 origin
      title = Form("%s_%ssum_Box%d", nameDetector, nameSubDir, box);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
  			     0x1000, 0, 0x1000,
  			     "TDC [ch]", ""));
    }

    target_id = getUniqueID(kPWO, 0, kTDC2D, 0);
    for(Int_t i = 0; i<NumOfBoxPWO; ++i){
      const char* title = NULL;
      Int_t box = i+1; // 1 origin
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
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kPWO, 0, kHitPat, 0);
    for(Int_t i = 0; i<NumOfBoxPWO; ++i){
      const char* title = NULL;
      Int_t box = i+1; // 1 origin
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
    TString strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kPWO, 0, kMulti, 0);
    for(Int_t i = 0; i<NumOfBoxPWO; ++i){
      const char* title = NULL;
      Int_t box = i+1; // 1 origin
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

TList* HistMaker::createPWO_E05( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kPWO);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  const Int_t box_id = 7; // PBox 8 (E-6)

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kPWO, 0, kADC, 0);
    for(Int_t i=0; i<NumOfUnitPWO[box_id]; ++i){
      sub_dir->Add(createTH1(++target_id, Form("%s_%s_%d",
					       nameDetector, nameSubDir, i+1),
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kPWO, 0, kTDC, 0);
    for(Int_t i=0; i<NumOfUnitPWO[box_id]; ++i){
      sub_dir->Add(createTH1(++target_id, Form("%s_%s_%d",
					       nameDetector, nameSubDir, i+1),
			     0x800, 0, 0x800,
			     "TDC [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // HitPat---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kHitPat);
    const char* nameSubDir = strSubDir.Data();

    Int_t target_id = getUniqueID(kPWO, 0, kHitPat, 0);
    top_dir->Add(createTH1(++target_id, Form("%s_%s", nameDetector, nameSubDir),
			   NumOfUnitPWO[box_id], 0, NumOfUnitPWO[box_id],
			   "Segment", ""));
  }

  // Multiplicity---------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kMulti);
    const char* nameSubDir = strSubDir.Data();

    Int_t target_id = getUniqueID(kPWO, 0, kMulti, 0);
    top_dir->Add(createTH1(++target_id, Form("%s_%s", nameDetector, nameSubDir),
			   NumOfUnitPWO[box_id]+1, 0, NumOfUnitPWO[box_id]+1,
			   "Multiplicity", ""));
  }

  // Return the TList pointer which is added into TGFileBrowser

  return top_dir;
}


// -------------------------------------------------------------------------
// createSFT
// -------------------------------------------------------------------------
TList* HistMaker::createSFT( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kSFT);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);
  const char* name_Layer[] = { "U", "V", "X", "XP" };
  const char* name_Layer_1[] = { "U", "V", "X" };
  //  const Int_t NumOfSegSFT;

  // TDC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSFT, 0, kTDC, 0);
    const char* sub_name = "TDC";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneSFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1( ++target_id, title , // 1 origin
			      1024, 0, 1024,
			      "TDC [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TDC (after cut) --------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kCTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSFT, 0, kTDC, 10);
    const char* sub_name = "CTDC";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneSFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     1024, 0, 1024,
			     "TDC [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSFT, 0, kADC, 0);
    const char* sub_name = "TOT";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneSFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     200, -50, 150,
			     "TOT [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT (after cut) --------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kCTOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSFT, 0, kADC, 10);
    const char* sub_name = "CTOT";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneSFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     200, -50, 150,
			     "TOT [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Hit parttern -----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kHitPattern);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSFT, 0, kHitPat, 0);
    const char* sub_name = "HitPat";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneSFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     NumOfSegSFT[i], 0, NumOfSegSFT[i],
			     "Segment", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Hit parttern (after cut) -----------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kCHitPattern);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSFT, 0, kHitPat, 10);
    const char* sub_name = "CHitPat";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneSFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     NumOfSegSFT[i], 0, NumOfSegSFT[i],
			     "Segment", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kMultiplicity);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    //    const char* title = "SFT_multiplicity";
    Int_t target_id = getUniqueID(kSFT, 0, kMulti, 0);
    const char* sub_name = "multiplicity";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneSFT-1; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer_1[i] );
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     30, 0, 30,
			     "Multiplicity", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity (after cut)------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kCMultiplicity);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    //    const char* title = "SFT_CMulti";
    Int_t target_id = getUniqueID(kSFT, 0, kMulti, 10);
    const char* sub_name = "CMulti";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneSFT-1; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s", nameDetector, sub_name, name_Layer_1[i] );
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     30, 0, 30,
			     "Multiplicity", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TDC-2D (after cut) --------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC_2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSFT, 0, kTDC2D, 0);
    const char* sub_name = "CTDC";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneSFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_2D", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegSFT[i], 0, NumOfSegSFT[i],
			     1024, 0, 1024,
			     "Segment", "TDC [ch]"));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TOT-2D (after cut) --------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTOT_2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSFT, 0, kADC2D, 0);
    const char* sub_name = "CTOT";
    // Add to the top directory
    for(Int_t i=0; i<NumOfPlaneSFT; ++i){
      const char* title = NULL;
      title = Form("%s_%s_%s_2D", nameDetector, sub_name, name_Layer[i] );
      sub_dir->Add(createTH2(++target_id, title, // 1 origin
			     NumOfSegSFT[i], 0, NumOfSegSFT[i],
			     200, -50, 150,
			     "Segment", "TOT [ch]"));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }
  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createSAC_for_E40
// -------------------------------------------------------------------------
TList* HistMaker::createSAC( Bool_t flag_ps )
{
  TString strDet = CONV_STRING(kSAC);       // Determine the detector name
  name_created_detectors_.push_back(strDet);    // name list of crearted detector
  if(flag_ps) name_ps_files_.push_back(strDet); // name list which are displayed in Ps tab

  const char* nameDetector = strDet.Data(); // Declaration of the directory
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);
  const char* Room[]={"Room1","Room2","Room3","Room4"};


  // ADC ---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kADC); // Declaration of the sub-directory
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName( nameSubDir );

    Int_t target_id = getUniqueID(kSAC, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfRoomsSAC; ++i){
      const char* title = NULL;
      title = Form("SAC_%s_%s", Room[i], nameSubDir);
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  // ADC w/TDC ---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kADCwTDC); // Declaration of the sub-directory
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName( nameSubDir );

    Int_t target_id = getUniqueID(kSAC, 0, kADCwTDC, 0);
    for( Int_t i=0; i<NumOfRoomsSAC; ++i ){
      const char* title = NULL;
      title = Form("SAC_%s_%s", Room[i], nameSubDir);
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  // TDC---------------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTDC); // Declaration of the sub-directory
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    Int_t target_id = getUniqueID(kSAC, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfRoomsSAC; ++i){
      const char* title = NULL;
      title = Form("SAC_%s_%s", Room[i], nameSubDir);
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  }

  { // Hit parttern -----------------------------------------------
    Int_t target_id = getUniqueID(kSAC, 0, kHitPat, 0);
    top_dir->Add(createTH1(++target_id, "SAC_hit_pattern",
			   NumOfRoomsSAC, 0, NumOfRoomsSAC,
			   "Room", ""));
  }

  { // Multiplicity -----------------------------------------------
    Int_t target_id = getUniqueID(kSAC, 0, kMulti, 0);
    top_dir->Add(createTH1(++target_id, "SAC_multiplicity",
			   NumOfRoomsSAC+1, 0, NumOfRoomsSAC+1,
			   "Multiplicity", ""));
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

// -------------------------------------------------------------------------
// createFHT1
// -------------------------------------------------------------------------
TList* HistMaker::createFHT1( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kFHT1);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }
  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);
  const char* UD[] ={"U","D"};

  // TDC/TOT SUM -----------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTDC_TOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    for(Int_t l=0; l<NumOfLayersFHT; ++l){
      for(Int_t v=0; v<NumOfUDStructureFHT; ++v){
	// TDC
	sub_dir->Add(createTH1(getUniqueID(kFHT1, l, kTDC, NumOfSegFHT1+1+ v*FHTOffset) ,
			       Form("%s_%sX%d_TDC", nameDetector, UD[v], l+1),
			       1024, 0, 1024,
			       "TDC [ch]", ""));
	// TOT
	sub_dir->Add(createTH1(getUniqueID(kFHT1, l, kADC, NumOfSegFHT1+1+ v*FHTOffset) ,
			       Form("%s_%sX%d_TOT", nameDetector, UD[v], l+1),
			       200, -50, 150,
			       "TOT [ch]", ""));
      }
    }
    top_dir->Add(sub_dir);
  }
  // TDC-2D --------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTDC_2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    for(Int_t l=0; l<NumOfLayersFHT; ++l){
      for(Int_t v=0; v<NumOfUDStructureFHT; ++v){
        Int_t target_id = getUniqueID(kFHT1, l, kTDC2D, v*FHTOffset);
        sub_dir->Add(createTH2(++target_id,
                               Form("FHT1_%sX%d_TDC2D", UD[v], l+1), // 1 origin
			       NumOfSegFHT1, 0, NumOfSegFHT1,
			       1024, 0, 1024,
			       "Segment", "TDC [ch]"));
      }
    }
    top_dir->Add(sub_dir);
  }
  // TOT-2D --------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTOT_2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    for(Int_t l=0; l<NumOfLayersFHT; ++l){
      for(Int_t v=0; v<NumOfUDStructureFHT; ++v){
        Int_t target_id = getUniqueID(kFHT1, l, kADC2D, v*FHTOffset);
        sub_dir->Add(createTH2(++target_id,
			       Form("FHT1_%sX%d_TOT2D", UD[v], l+1), // 1 origin
			       NumOfSegFHT1, 0, NumOfSegFHT1,
			       200, -50, 150,
			       "Segment", "TOT [ch]"));
      }
    }
    top_dir->Add(sub_dir);
  }
  // Hit parttern --------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kHitParttern);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    for(Int_t l=0; l<NumOfLayersFHT; ++l){
      for(Int_t v=0; v<NumOfUDStructureFHT; ++v){
        Int_t target_id = getUniqueID(kFHT1, l, kHitPat, v*FHTOffset);
        sub_dir->Add(createTH1(++target_id,
                               Form("FHT1_%sX%d_HitPat", UD[v], l+1), // 1 origin
			       NumOfSegFHT1, 0, NumOfSegFHT1,
			       "Segment", ""));
      }
    }
    top_dir->Add(sub_dir);
  }
  // Multiplicity -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kMultiplicity);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    for(Int_t l=0; l<NumOfLayersFHT; ++l){
      for(Int_t v=0; v<NumOfUDStructureFHT; ++v){
        Int_t target_id = getUniqueID(kFHT1, l, kMulti, v*FHTOffset);
        sub_dir->Add(createTH1(++target_id,
                               Form("FHT1_%sX%d_multiplicity", UD[v], l+1), // 1 origin
			       NumOfSegFHT1, 0, NumOfSegFHT1,
			       "Multiplicity", ""));
      }
    }
    top_dir->Add(sub_dir);
  }
  return top_dir;
}

// -------------------------------------------------------------------------
// createFHT2
// -------------------------------------------------------------------------
TList* HistMaker::createFHT2( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kFHT2);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }
  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);
  const char* UD[] ={"U","D"};

  // TDC/TOT SUM -----------------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTDC_TOT);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    for(Int_t l=0; l<NumOfLayersFHT; ++l){
      for(Int_t v=0; v<NumOfUDStructureFHT; ++v){
        // TDC
        sub_dir->Add(createTH1(getUniqueID(kFHT2, l, kTDC, NumOfSegFHT2+1+ v*FHTOffset) ,
			       Form("%s_%sX%d_TDC", nameDetector, UD[v], l+1),
			       1024, 0, 1024,
			       "TDC [ch]", ""));
        // TOT
        sub_dir->Add(createTH1(getUniqueID(kFHT2, l, kADC, NumOfSegFHT2+1+ v*FHTOffset) ,
			       Form("%s_%sX%d_TOT", nameDetector, UD[v], l+1),
			       200, -50, 150,
			       "TOT [ch]", ""));
      }
    }
    top_dir->Add(sub_dir);
  }
  // TDC-2D --------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTDC_2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    for(Int_t l=0; l<NumOfLayersFHT; ++l){
      for(Int_t v=0; v<NumOfUDStructureFHT; ++v){
        Int_t target_id = getUniqueID(kFHT2, l, kTDC2D, v*FHTOffset);
        sub_dir->Add(createTH2(++target_id,
                               Form("FHT2_%sX%d_TDC2D", UD[v], l+1), // 1 origin
			       NumOfSegFHT2, 0, NumOfSegFHT2,
			       1024, 0, 1024,
			       "Segment", "TDC [ch]"));
      }
    }
    top_dir->Add(sub_dir);
  }
  // TOT-2D --------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kTOT_2D);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    for(Int_t l=0; l<NumOfLayersFHT; ++l){
      for(Int_t v=0; v<NumOfUDStructureFHT; ++v){
        Int_t target_id = getUniqueID(kFHT2, l, kADC2D, v*FHTOffset);
        sub_dir->Add(createTH2(++target_id,
			       Form("FHT2_%sX%d_TOT2D", UD[v], l+1), // 1 origin
			       NumOfSegFHT2, 0, NumOfSegFHT2,
			       200, -50, 150,
			       "Segment", "TOT [ch]"));
      }
    }
    top_dir->Add(sub_dir);
  }
  // Hit parttern --------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kHitParttern);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    for(Int_t l=0; l<NumOfLayersFHT; ++l){
      for(Int_t v=0; v<NumOfUDStructureFHT; ++v){
        Int_t target_id = getUniqueID(kFHT2, l, kHitPat, v*FHTOffset);
        sub_dir->Add(createTH1(++target_id,
                               Form("FHT2_%sX%d_HitPat", UD[v], l+1), // 1 origin
			       NumOfSegFHT2, 0, NumOfSegFHT2,
			       "Segment", ""));
      }
    }
    top_dir->Add(sub_dir);
  }
  // Multiplicity -----------------------------------------------
  {
    TString strSubDir  = CONV_STRING(kMultiplicity);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    for(Int_t l=0; l<NumOfLayersFHT; ++l){
      for(Int_t v=0; v<NumOfUDStructureFHT; ++v){
        Int_t target_id = getUniqueID(kFHT2, l, kMulti, v*FHTOffset);
        sub_dir->Add(createTH1(++target_id,
                               Form("FHT2_%sX%d_multiplicity", UD[v], l+1), // 1 origin
			       NumOfSegFHT2, 0, NumOfSegFHT2,
			       "Multiplicity", ""));
      }
    }
    top_dir->Add(sub_dir);
  }
  return top_dir;
}
// -------------------------------------------------------------------------
// createE72E90
// -------------------------------------------------------------------------
TList* HistMaker::createE72E90( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = "E72E90";
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // E72BAC
    Int_t target_id = getUniqueID(kE72BAC, 0, kADC, 0);
    const char* title = NULL;
    title = Form("%s_BAC_%s", nameDetector, nameSubDir);

    sub_dir->Add(createTH1(++target_id, title, // 1 origin
			   0x1000, 0, 0x1000,
			   "ADC [ch]", ""));
    // E90SAC
    target_id = getUniqueID(kE90SAC, 0, kADC, 0);
    for( Int_t i=0; i<NumOfSegE90SAC; ++i ){
      const char* title = NULL;
      Int_t seg = i +1; // 1 origin
      title = Form("%s_SAC_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    // E72KVC
    const TString udsum[3] = {"U", "D", "SUM"};
    target_id = getUniqueID(kE72KVC, 0, kADC, 0);
    for( Int_t j=0; j<3; ++j ){
      for( Int_t i=0; i<NumOfSegE72KVC; ++i ){
	const char* title = NULL;
	Int_t seg = i +1; // 1 origin
	title = Form("%s_KVC_%s_%d%s", nameDetector, nameSubDir, seg, udsum[j].Data());
	sub_dir->Add(createTH1(++target_id, title, // 1 origin
			       0x1000, 0, 0x1000,
			       "ADC [ch]", ""));
      }
    }
    // E42BH2
    const TString ud[2] = {"U", "D"};
    target_id = getUniqueID(kE42BH2, 0, kADC, 0);
    for( Int_t j=0; j<2; ++j ){
      const char* title = NULL;
      Int_t seg = 5; // 1 origin
      title = Form("%s_BH2_%s_%d%s", nameDetector, nameSubDir, seg, ud[j].Data());
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }
  // ADC w/TDC ---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADCwTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // E72BAC
    Int_t target_id = getUniqueID(kE72BAC, 0, kADCwTDC, 0);
    const char* title = NULL;
    title = Form("%s_BAC_%s", nameDetector, nameSubDir);

    sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    // E90SAC
    target_id = getUniqueID(kE90SAC, 0, kADCwTDC, 0);
    for( Int_t i=0; i<NumOfSegE90SAC; ++i ){
      const char* title = NULL;
      Int_t seg = i +1; // 1 origin
      title = Form("%s_SAC_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    // E72KVC
    const TString udsum[3] = {"U", "D", "SUM"};
    target_id = getUniqueID(kE72KVC, 0, kADCwTDC, 0);
    for( Int_t j=0; j<3; ++j ){
      for( Int_t i=0; i<NumOfSegE72KVC; ++i ){
	const char* title = NULL;
	Int_t seg = i +1; // 1 origin
	title = Form("%s_KVC_%s_%d%s", nameDetector, nameSubDir, seg, udsum[j].Data());
	sub_dir->Add(createTH1(++target_id, title, // 1 origin
			       0x1000, 0, 0x1000,
			       "ADC [ch]", ""));
      }
    }
    // E42BH2
    const TString ud[2] = {"U", "D"};
    target_id = getUniqueID(kE42BH2, 0, kADCwTDC, 0);
    for( Int_t j=0; j<2; ++j ){
      const char* title = NULL;
      Int_t seg = 5; // 1 origin
      title = Form("%s_BH2_%s_%d%s", nameDetector, nameSubDir, seg, ud[j].Data());
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }

    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // TDC ----------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    // E72BAC
    Int_t target_id = getUniqueID(kE72BAC, 0, kTDC, 0);
    const char* title = NULL;
    title = Form("%s_BAC_%s", nameDetector, nameSubDir);

    sub_dir->Add(createTH1(target_id +1, title, // 1 origin
			    //			     10000, 0, 400000,
			    50000, 0, 2000000,
			    "TDC [ch]", ""));
    // E90SAC
    target_id = getUniqueID(kE90SAC, 0, kTDC, 0);
    for( Int_t i=0; i<NumOfSegE90SAC; ++i ){
      const char* title = NULL;
      Int_t seg = i +1; // 1 origin
      title = Form("%s_SAC_%s_%d", nameDetector, nameSubDir, seg);
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			    50000, 0, 2000000,
			    "TDC [ch]", ""));
    }
    // E72KVC
    const TString udsum[3] = {"U", "D", "SUM"};
    target_id = getUniqueID(kE72KVC, 0, kTDC, 0);
    for( Int_t j=0; j<3; ++j ){
      for( Int_t i=0; i<NumOfSegE72KVC; ++i ){
	const char* title = NULL;
	Int_t seg = i +1; // 1 origin
	title = Form("%s_KVC_%s_%d%s", nameDetector, nameSubDir, seg, udsum[j].Data());
	sub_dir->Add(createTH1(++target_id, title, // 1 origin
			    50000, 0, 2000000,
			    "TDC [ch]", ""));
      }
    }
    // E42BH2
    const TString udmt[3] = {"U", "D", "MT"};
    target_id = getUniqueID(kE42BH2, 0, kTDC, 0);
    for( Int_t j=0; j<2; ++j ){
      const char* title = NULL;
      Int_t seg = 5; // 1 origin
      title = Form("%s_BH2_%s_%d%s", nameDetector, nameSubDir, seg, udmt[j].Data());
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			    50000, 0, 2000000,
			    "TDC [ch]", ""));
    }
    for( Int_t i=0; i<NumOfSegE42BH2; ++i ){
      const char* title = NULL;
      Int_t seg = i +1; // 1 origin
      title = Form("%s_BH2_%s_%d%s", nameDetector, nameSubDir, seg, udmt[2].Data());
      sub_dir->Add(createTH1(++target_id, title, // 1 origin
			    50000, 0, 2000000,
			    "TDC [ch]", ""));
    }
    // T1
    target_id = getUniqueID(kT1, 0, kTDC, 0);
    title = Form("%s_T1_%s", nameDetector, nameSubDir);

    sub_dir->Add(createTH1(target_id +1, title, // 1 origin
			    //			     10000, 0, 400000,
			    50000, 0, 2000000,
			    "TDC [ch]", ""));
    // T2
    target_id = getUniqueID(kT2, 0, kTDC, 0);
    title = Form("%s_T2_%s", nameDetector, nameSubDir);

    sub_dir->Add(createTH1(target_id +1, title, // 1 origin
			    //			     10000, 0, 400000,
			    50000, 0, 2000000,
			    "TDC [ch]", ""));


    // insert sub directory
    top_dir->Add(sub_dir);
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "T1_multiplicity";
    Int_t target_id = getUniqueID(kT1, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegT1+1, 0, NumOfSegT1+1,
			   "Multiplicity", ""));
  }
  // Multiplicity -----------------------------------------------
  {
    const char* title = "T2_multiplicity";
    Int_t target_id = getUniqueID(kT2, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegT2+1, 0, NumOfSegT2+1,
			   "Multiplicity", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "E72BAC_multiplicity";
    Int_t target_id = getUniqueID(kE72BAC, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegE72BAC+1, 0, NumOfSegE72BAC+1,
			   "Multiplicity", ""));
  }
  // Hit parttern -----------------------------------------------
  {
    const char* title = "E90SAC_hit_pattern";
    Int_t target_id = getUniqueID(kE90SAC, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegE90SAC, 0, NumOfSegE90SAC,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "E90SAC6_multiplicity";
    Int_t target_id = getUniqueID(kE90SAC, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegE90SAC+1, 0, NumOfSegE90SAC+1,
			   "Multiplicity", ""));
  }
  // Multiplicity -----------------------------------------------
  {
    const char* title = "E90SAC8_multiplicity";
    Int_t target_id = getUniqueID(kE90SAC, 0, kMulti, 1);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegE90SAC+1, 0, NumOfSegE90SAC+1,
			   "Multiplicity", ""));
  }
  // Hit parttern -----------------------------------------------
  {
    const char* title = "E72KVC_hit_pattern";
    Int_t target_id = getUniqueID(kE72KVC, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegE72KVC, 0, NumOfSegE72KVC,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "E72KVC_multiplicity";
    Int_t target_id = getUniqueID(kE72KVC, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegE72KVC+1, 0, NumOfSegE72KVC+1,
			   "Multiplicity", ""));
  }
  // Hit parttern -----------------------------------------------
  {
    const char* title = "E72KVCSUM_hit_pattern";
    Int_t target_id = getUniqueID(kE72KVC, 0, kHitPat, 1);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegE72KVC, 0, NumOfSegE72KVC,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "E72KVCSUM_multiplicity";
    Int_t target_id = getUniqueID(kE72KVC, 0, kMulti, 1);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegE72KVC+1, 0, NumOfSegE72KVC+1,
			   "Multiplicity", ""));
  }

  // Hit parttern -----------------------------------------------
  {
    const char* title = "E42BH2_hit_pattern";
    Int_t target_id = getUniqueID(kE42BH2, 0, kHitPat, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegE42BH2, 0, NumOfSegE42BH2,
			   "Segment", ""));
  }

  // Multiplicity -----------------------------------------------
  {
    const char* title = "E42BH2_multiplicity";
    Int_t target_id = getUniqueID(kE42BH2, 0, kMulti, 0);
    // Add to the top directory
    top_dir->Add(createTH1(target_id + 1, title, // 1 origin
			   NumOfSegE42BH2+1, 0, NumOfSegE42BH2+1,
			   "Multiplicity", ""));
  }

  // Hit parttern -----------------------------------------------
  {
    const char* title = "E72Parasite_hit_pattern";
    Int_t target_id = getUniqueID(kE72Parasite, 0, kHitPat, 0);
    // Add to the top directory
    auto h = createTH1(target_id + 1, title, // 1 origin
		    NumOfSegE72Parasite, 0, NumOfSegE72Parasite,
		    "", "");
    for( Int_t i=0, n=e72parasite::SE72Parasite.size(); i<n; ++i ){
	    h->GetXaxis()->SetBinLabel( i+1, e72parasite::SE72Parasite.at(i) );
    }
    h->SetStats(0);
    top_dir->Add(h);
  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}


// -------------------------------------------------------------------------
// createTF_TF
// -------------------------------------------------------------------------
TList* HistMaker::createTF_TF( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kTF_TF);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);

  // ADC---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kADC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kTF_TF, 0, kADC, 0);
    for(Int_t i = 0; i<NumOfSegTF_TF; ++i){
      const char* title = NULL;
      title = Form("%s_%s", nameDetector, nameSubDir);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));
    }
    // insert sub directory
    top_dir->Add(sub_dir);
  }
 { // SUM ADCwTDC---------------------------------------------------------
    TString strSubDir  = CONV_STRING(kADCwTDC); // Declaration of the sub-directory
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kTF_TF, 0, kADCwTDC, 0);

      const char* title = NULL;
      title = Form("%s_%s", nameDetector, nameSubDir);
      sub_dir->Add(createTH1(target_id +1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "ADC [ch]", ""));

    top_dir->Add(sub_dir);
  }


  // TDC ---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kTF_TF, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegTF_TF; ++i){
      const char* title = NULL;
        title = Form("%s_%s", nameDetector, nameSubDir);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  return top_dir;

  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}


// -------------------------------------------------------------------------
// createTF_GN1
// -------------------------------------------------------------------------
TList* HistMaker::createTF_GN1( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kTF_GN1);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);


  // TDC ---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kTF_GN1, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegTF_GN1; ++i){
      const char* title = NULL;
        title = Form("%s_%s", nameDetector, nameSubDir);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  return top_dir;

  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}


// -------------------------------------------------------------------------
// createTF_GN2
// -------------------------------------------------------------------------
TList* HistMaker::createTF_GN2( Bool_t flag_ps )
{
  // Determine the detector name
  TString strDet = CONV_STRING(kTF_GN2);
  // name list of crearted detector
  name_created_detectors_.push_back(strDet);
  if(flag_ps){
    // name list which are displayed in Ps tab
    name_ps_files_.push_back(strDet);
  }

  // Declaration of the directory
  // Just type conversion from TString to char*
  const char* nameDetector = strDet.Data();
  TList *top_dir = new TList;
  top_dir->SetName(nameDetector);


  // TDC ---------------------------------------------------------
  {
    // Declaration of the sub-directory
    TString strSubDir  = CONV_STRING(kTDC);
    const char* nameSubDir = strSubDir.Data();
    TList *sub_dir = new TList;
    sub_dir->SetName(nameSubDir);

    // Make histogram and add it
    Int_t target_id = getUniqueID(kTF_GN2, 0, kTDC, 0);
    for(Int_t i = 0; i<NumOfSegTF_GN2; ++i){
      const char* title = NULL;
        title = Form("%s_%s", nameDetector, nameSubDir);
      sub_dir->Add(createTH1(target_id + i+1, title, // 1 origin
			     0x1000, 0, 0x1000,
			     "TDC [ch]", ""));
    }
    top_dir->Add(sub_dir);
  return top_dir;

  }

  // Return the TList pointer which is added into TGFileBrowser
  return top_dir;
}

#endif
