// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;
#include<algorithm>

void dispHist_e40()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // Return to the initial position
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
    c->cd();
  }

  gROOT->SetStyle("Plain");
  TPDF *pdf = new TPDF("/home/sks/PSFile/e40_2017/hist_e40.pdf", 112);

  // Title
  {
    //    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    TCanvas *c = new TCanvas("c0", "c0");
    //    pdf->NewPage();
    c->UseCurrentStyle();
    c->Divide(1,1);
    c->cd(1);
    c->GetPad(1)->Range(0,0,100,100);
    TText text;
    text.SetTextSize(0.2);
    text.SetTextAlign(22);
    TTimeStamp stamp;
    stamp.Add( -stamp.GetZoneOffset() );
    text.SetTextSize(0.1);
    text.DrawText(50.,50., stamp.AsString("s") );
    c->Update();
    c->cd();
    c->GetPad(1)->Close();
    delete c;
    c = NULL;
  }

  // BH1
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->UseCurrentStyle();
    c->Clear();
    c->Divide(2,2);
    const int n_hist = 4;
    int id[n_hist] = {0};
    id[0] = HistMaker::getUniqueID(kBH1, 0, kTDC,    6);
    id[1] = HistMaker::getUniqueID(kBH1, 0, kTDC,    8);
    id[2] = HistMaker::getUniqueID(kBH1, 0, kHitPat, 2); // CHitPat
    id[3] = HistMaker::getUniqueID(kBH1, 0, kMulti,  2); // CMulti

    for( int i = 0; i<n_hist; ++i ){
      c->cd(i+1);
      if( i == 0 || i == 1 ) gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get(id[i]);
      if( !h ) continue;
      h->GetXaxis()->UnZoom();
      h->Draw();
    }
    c->Update();
  }// BH1

  // BFT
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->UseCurrentStyle();
    c->Clear();
    c->Divide(2,2);
    const int n_hist = 4;
    int id[n_hist] = {0};
    id[0] = HistMaker::getUniqueID(kBFT, 0, kTDC,    1);
    id[1] = HistMaker::getUniqueID(kBFT, 0, kADC,    1);
    id[2] = HistMaker::getUniqueID(kBFT, 0, kHitPat, 11); // CHitPat
    id[3] = HistMaker::getUniqueID(kBFT, 0, kMulti,  11); // CMulti

    for( int i = 0; i<n_hist; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get(id[i]);
      if( !h ) continue;
      h->GetXaxis()->UnZoom();
      h->Draw();
    }
    c->Update();
  }// BFT

  // BC4-X
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->UseCurrentStyle();
    c->Clear();
    c->Divide(2,2);
    const int n_hist = 4;
    int id[n_hist] = {0};
    id[0] = HistMaker::getUniqueID(kBC4, 0, kTDC,    5);
    id[1] = HistMaker::getUniqueID(kBC4, 0, kTDC,    6);
    id[2] = HistMaker::getUniqueID(kBC4, 0, kHitPat, 6); // CHitPat
    id[3] = HistMaker::getUniqueID(kBC4, 0, kMulti,  12); // CMulti

    for( int i = 0; i<n_hist; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get(id[i]);
      if( !h ) continue;
      h->GetXaxis()->UnZoom();
      h->Draw();
    }
    c->Update();

  }// BC4-X

  pdf->Close();

  gROOT->SetStyle("Classic");

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
