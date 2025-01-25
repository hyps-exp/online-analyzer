// -*- C++ -*-

#include <iostream>

#include <TCanvas.h>
#include <TFolder.h>
#include <TH1.h>
#include <THttpServer.h>
#include <TKey.h>
#include <TMacro.h>
#include <TObject.h>
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
HttpServer::MakePs(void)
{
  // static auto& gJsRoot = analyzer::JsRootUpdater::getInstance();
  gSystem->Sleep(5000);
  TString ps_path("/home/sks/PSFile/pro/default.ps");
  TPostScript ps(ps_path, 112);
  const auto& g_unpacker = hddaq::unpacker::GUnpacker::get_instance();
  Int_t runno = g_unpacker.get_root()->get_run_number();
  ps.NewPage();
  {
    static TCanvas c1("cps", "cps", 700, 500);
    c1.Clear();
    c1.cd(1);
    c1.GetPad(1)->Range(0,0,100,100);
    TText text;
    text.SetTextSize(0.2);
    text.SetTextAlign(22);
    text.DrawText(50.,65., Form("Run# %d", runno));
    // TTimeStamp stamp;
    // stamp.Add(-stamp.GetZoneOffset());
    // text.SetTextSize(0.1);
    // text.DrawText(50.,40., stamp.AsString("s"));
    c1.Update();
    c1.GetPad(1)->Close();
  }
  gSystem->Setenv("SLEEP_JSROOT", "0");
  return;
  TIter canvas_iterator(gROOT->GetListOfCanvases());
  while(true){
    Int_t i = 0;
    std::cout << "#D HttpServer::MakePs()" << i++ << std::endl;
    auto c1 = dynamic_cast<TCanvas*>(canvas_iterator.Next());
    if(!c1) break;
    c1->Modified();
    c1->Update();
  }
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
