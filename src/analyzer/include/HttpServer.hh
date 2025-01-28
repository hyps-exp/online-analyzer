// -*- C++ -*-

#ifndef HTTP_SERVER_HH
#define HTTP_SERVER_HH

#include <TObject.h>

class TCanvas;
class TH1;
class THttpServer;
class TMacro;
class TObject;

//_____________________________________________________________________________
class HttpServer : public TObject
{
public:
  static HttpServer& GetInstance(void);
  ~HttpServer(void);

private:
  HttpServer(void);
  HttpServer(const HttpServer&);
  HttpServer& operator =(const HttpServer&);

private:
  THttpServer*       m_server;
  Int_t              m_port;
  std::vector<TH1*>  m_th1_list;

public:
  void CreateItem(TString name, TString desc);
  void Hide(TString dir);
  void Open(void);
  void MakePs(Int_t runno);
  void Register(TObject *obj);
  void Register(TList *list, TList *parent=nullptr);
  void Register(TMacro *macro);
  void Register(TString dir, TString command);
  void ResetAll(void);
  void SetPort(Int_t port){m_port = port;}
  void SetItemField(TString dir, TString key, TString val);

  ClassDef(HttpServer,0);
};

//______________________________________________________________________________
inline HttpServer&
HttpServer::GetInstance(void)
{
  static HttpServer g_instance;
  return g_instance;
}

#endif
