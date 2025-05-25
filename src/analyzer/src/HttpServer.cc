// -*- C++ -*-

#include <fstream>
#include <iostream>

#include <TCanvas.h>
#include <TFolder.h>
#include <TH1.h>
#include <THttpServer.h>
#include <TKey.h>
#include <TMacro.h>
#include <TObject.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TPostScript.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TText.h>
#include <TTimeStamp.h>

#include <Unpacker.hh>
#include <UnpackerManager.hh>

#include "JsRootUpdater.hh"

#include "FuncName.hh"
#include "HttpServer.hh"

ClassImp(HttpServer);

//_____________________________________________________________________________
HttpServer::HttpServer( void )
  : TObject(),
    m_server(0),
    m_port(8080),
    m_th1_list()
{
}

//_____________________________________________________________________________
HttpServer::~HttpServer( void )
{
}

//_____________________________________________________________________________
void
HttpServer::CreateItem( TString name, TString desc )
{
  m_server->CreateItem( name, desc );
}

//_____________________________________________________________________________
void
HttpServer::Hide( TString dir )
{
  m_server->Hide( dir );
}

//_____________________________________________________________________________
void
HttpServer::MakePs(Int_t runno)
{
  // static auto& gJsRoot = analyzer::JsRootUpdater::getInstance();
  // gSystem->Sleep(5000);
  // const auto& g_unpacker = hddaq::unpacker::GUnpacker::get_instance();
  // Int_t runno = g_unpacker.get_root()->get_run_number();
  TString ps_path(Form("/misc/subdata/psfile/run%05d.pdf", runno));
  std::ifstream comment_txt("/misc/rawdata/misc/comment.txt");
  TString line;
  TString comment;
  while(line.ReadLine(comment_txt)){
    if(line.Contains("START") && line.Contains(Form("%05d", runno))){
      auto tokens = line.Tokenize(" \t");
      if(tokens && tokens->GetEntries() >= 8){
	for(Int_t i=7; i<tokens->GetEntries(); ++i){
	  if(!comment.IsNull()) comment += " ";
	  comment += ((TObjString*)tokens->At(i))->GetString();
	}
      }
    }
  }
  std::cout << "#D HttpServer::MakePs() open " << ps_path << std::endl;
  {
    TCanvas c1("cps", "cps", 700, 500);
    TText text;
    text.SetTextSize(0.2);
    text.SetTextAlign(22);
    text.SetNDC(1);
    text.DrawText(0.5, 0.65, Form("Run# %d", runno));
    TTimeStamp stamp;
    stamp.Add(-stamp.GetZoneOffset());
    text.SetTextSize(0.04);
    text.DrawText(0.5, 0.45, comment);
    text.SetTextSize(0.08);
    text.DrawText(0.5, 0.32, stamp.AsString("s"));
    c1.Modified();
    c1.Update();
    c1.Print(ps_path + "(");
  }
  TIter next(gROOT->GetListOfCanvases());
  TCanvas* c1;
  while((c1 = dynamic_cast<TCanvas*>(next()))){
    std::cout << c1->GetName() << std::endl;
    c1->Modified();
    c1->Update();
    c1->Print(ps_path);
  }
  TCanvas c2;
  c2.Print(ps_path + "]");
  std::cout << "#D HttpServer::MakePs() done" << std::endl;
}

//_____________________________________________________________________________
void
HttpServer::MakePsPre(Int_t runno)
{
  // static auto& gJsRoot = analyzer::JsRootUpdater::getInstance();
  // gSystem->Sleep(5000);
  // const auto& g_unpacker = hddaq::unpacker::GUnpacker::get_instance();
  // Int_t runno = g_unpacker.get_root()->get_run_number();
  TString ps_path(Form("/home/sks/user/haratani/psfile/now/pdf/auto00000.pdf"));
  std::ifstream comment_txt("/misc/rawdata/misc/comment.txt");
  TString line;
  TString comment;
  while(line.ReadLine(comment_txt)){
    if(line.Contains("START") && line.Contains(Form("%05d", runno))){
      auto tokens = line.Tokenize(" \t");
      if(tokens && tokens->GetEntries() >= 8){
	for(Int_t i=7; i<tokens->GetEntries(); ++i){
	  if(!comment.IsNull()) comment += " ";
	  comment += ((TObjString*)tokens->At(i))->GetString();
	}
      }
    }
  }
  std::cout << "#D HttpServer::MakePsPre() open " << ps_path << std::endl;
  {
    TCanvas c1("cps", "cps", 700, 500);
    TText text;
    text.SetTextSize(0.2);
    text.SetTextAlign(22);
    text.SetNDC(1);
    text.DrawText(0.5, 0.65, Form("auto# 00000"));
    TTimeStamp stamp;
    stamp.Add(-stamp.GetZoneOffset());
    text.SetTextSize(0.04);
    text.DrawText(0.5, 0.45, comment);
    text.SetTextSize(0.08);
    text.DrawText(0.5, 0.32, stamp.AsString("s"));
    c1.Modified();
    c1.Update();
    c1.Print(ps_path + "(");
  }
  TIter next(gROOT->GetListOfCanvases());
  TCanvas* c1;
  while((c1 = dynamic_cast<TCanvas*>(next()))){
    std::cout << c1->GetName() << std::endl;
    c1->Modified();
    c1->Update();
    c1->Print(ps_path);
  }
  TCanvas c2;
  c2.Print(ps_path + "]");
  std::cout << "#D HttpServer::MakePsPre() done" << std::endl;
}

//_____________________________________________________________________________
void
HttpServer::Open( void )
{
  m_server = new THttpServer(Form("http:%d?loopback?thrds=5", m_port));
  m_server->Restrict("/", "allow=all");
  m_server->SetReadOnly(kTRUE);
  m_server->SetJSROOT("https://root.cern.ch/js/latest/");
  m_server->RegisterCommand("/Reset", "HttpServer::GetInstance().ResetAll()");
  m_server->RegisterCommand("/Restart", "gSystem->Exit(0)");
  // m_server->RegisterCommand("/MakePs", "HttpServer::GetInstance().MakePs()");
  std::cout << "#D HttpServer::Open()" << std::endl
	    << "   Port : " << m_port << std::endl;
}

//_____________________________________________________________________________
void
HttpServer::Register( TObject *obj )
{
  if( !obj ||
      TString(obj->ClassName()).Contains("TCanvas") )
    return;

  if( TString(obj->ClassName()).Contains("TMacro") )
    Register((TMacro*)obj);

  m_server->Register( "/", obj );
}

//_____________________________________________________________________________
void
HttpServer::Register( TList *list, TList *parent )
{
  if( !list ) return;

  //  std::cout << "List : " << list->GetName() << std::endl;

  TString parent_dir = ( parent ) ? parent->GetName() : "";

  TIter itr(list);
  TObject *obj = 0;
  while( ( obj=itr.Next() ) ){
    TString class_name(obj->ClassName());
    //    std::cout << "  Obj : " << obj->GetName() << std::endl;
    if( class_name.Contains("TList") ){
      Register((TList*)obj, list);
    }
    if( class_name.Contains("TH1") ||
	class_name.Contains("TH2") ||
	class_name.Contains("TH3") ){
      m_server->Register("/"+parent_dir+"/"+list->GetName(), obj);
      m_th1_list.push_back( dynamic_cast<TH1*>(obj) );
    }
  }
}

//_____________________________________________________________________________
void
HttpServer::Register( TMacro *macro )
{
  std::cerr << "#W HttpServer::Register(TMacro) is not supported"
  	    << std::endl;
  // if( !macro ) return;
  // macro->Load();
  // m_server->RegisterCommand( Form("/%s", macro->GetName()),
  // 			     Form("%s()", macro->GetName()) );
  // std::cout << "  Macro : " << macro->GetName() << std::endl;
  // m_server->Register("/", macro);
}

//_____________________________________________________________________________
void
HttpServer::Register( TString dir, TString command )
{
  m_server->RegisterCommand( dir, command );
}

//_____________________________________________________________________________
void
HttpServer::ResetAll( void )
{
  std::cout << "#D HttpServer::ResetAll()" << std::endl;
  for( auto&& h : m_th1_list )
    h->Reset();
}

//_____________________________________________________________________________
void
HttpServer::SetItemField( TString dir, TString key, TString val )
{
  m_server->SetItemField( dir, key, val );
}
