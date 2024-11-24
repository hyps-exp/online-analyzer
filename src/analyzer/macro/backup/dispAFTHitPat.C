// -*- C++ -*-

#include "DetectorID.hh"
#include "UserParamMan.hh"

using namespace hddaq::gui; // for Updater

//_____________________________________________________________________________
void
dispAFTHitPat( void )
{
  //  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------



  { // draw Hitpattern X-U
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    int aft_hit_id = HistMaker::getUniqueID( kAFT, 0, kHitPat, 1 );
    int aft_chit_id = HistMaker::getUniqueID( kAFT, 0, kHitPat, 101 );
    c->Clear();
    c->Divide(6, 3);
    for( int l = 0; l < NumOfPlaneAFT; l++ ){
      if( l%4 != 0 && l%4 != 1 ) continue;
      c->cd(l/2+1+l%2);
      TH1 *h = (TH1*)GHist::get( aft_hit_id+l );
      if( !h ) continue;
      h->SetMinimum(0);
      h->Draw();
      TH1 *hh = (TH1*)GHist::get( aft_chit_id+l );
      if( !hh ) continue;
      hh->SetLineColor(kRed);
      hh->Draw("same");
      TH1 *hhh = (TH1*)GHist::get( aft_chit_id+(kUorD+1)*NumOfPlaneAFT+kUorD+l );
      if( !hhh ) continue;
      hhh->SetLineColor(kBlue);
      hhh->Draw("same");
    }
    c->Update();
  }

  { // draw Hitpattern X-D
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    int aft_hit_id = HistMaker::getUniqueID( kAFT, 0, kHitPat, NumOfPlaneAFT+1 );
    int aft_chit_id = HistMaker::getUniqueID( kAFT, 0, kHitPat, NumOfPlaneAFT+101 );
    c->Clear();
    c->Divide(6, 3);
    for( int l = 0; l < NumOfPlaneAFT; l++ ){
      if( l%4 != 0 && l%4 != 1 ) continue;
      c->cd(l/2+1+l%2);
      TH1 *h = (TH1*)GHist::get( aft_hit_id+l );
      if( !h ) continue;
      h->SetMinimum(0);
      h->Draw();
      TH1 *hh = (TH1*)GHist::get( aft_chit_id+l );
      if( !hh ) continue;
      hh->SetLineColor(kRed);
      hh->Draw("same");
      TH1 *hhh = (TH1*)GHist::get( aft_chit_id+(kUorD+1)*NumOfPlaneAFT+kUorD+l );
      if( !hhh ) continue;
      hhh->SetLineColor(kBlue);
      hhh->Draw("same");
    }
    c->Update();
  }

  { // draw Hitpattern Y-U
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    int aft_hit_id = HistMaker::getUniqueID( kAFT, 0, kHitPat, 1 );
    int aft_chit_id = HistMaker::getUniqueID( kAFT, 0, kHitPat, 101 );
    c->Clear();
    c->Divide(6, 3);
    for( int l = 0; l < NumOfPlaneAFT; l++ ){
      if( l%4 != 2 && l%4 != 3 ) continue;
      c->cd(l/2+l%2);
      TH1 *h = (TH1*)GHist::get( aft_hit_id+l );
      if( !h ) continue;
      h->SetMinimum(0);
      h->Draw();
      TH1 *hh = (TH1*)GHist::get( aft_chit_id+l );
      if( !hh ) continue;
      hh->SetLineColor(kRed);
      hh->Draw("same");
      TH1 *hhh = (TH1*)GHist::get( aft_chit_id+(kUorD+1)*NumOfPlaneAFT+kUorD+l );
      if( !hhh ) continue;
      hhh->SetLineColor(kBlue);
      hhh->Draw("same");
    }
    c->Update();
  }

  { // draw Hitpattern Y-D
    TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
    int aft_hit_id = HistMaker::getUniqueID( kAFT, 0, kHitPat, NumOfPlaneAFT+1 );
    int aft_chit_id = HistMaker::getUniqueID( kAFT, 0, kHitPat, NumOfPlaneAFT+101 );
    c->Clear();
    c->Divide(6, 3);
    for( int l = 0; l < NumOfPlaneAFT; l++ ){
      if( l%4 != 2 && l%4 != 3 ) continue;
      c->cd(l/2+l%2);
      TH1 *h = (TH1*)GHist::get( aft_hit_id+l );
      if( !h ) continue;
      h->SetMinimum(0);
      h->Draw();
      TH1 *hh = (TH1*)GHist::get( aft_chit_id+l );
      if( !hh ) continue;
      hh->SetLineColor(kRed);
      hh->Draw("same");
      TH1 *hhh = (TH1*)GHist::get( aft_chit_id+(kUorD+1)*NumOfPlaneAFT+kUorD+l );
      if( !hhh ) continue;
      hhh->SetLineColor(kBlue);
      hhh->Draw("same");
    }
    c->Update();
  }


  {  // draw Hitpattern X-UDcoin
    TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
    int aft_chit_id = HistMaker::getUniqueID( kAFT, 0, kHitPat, kUorD*NumOfPlaneAFT+101 );
    c->Clear();
    c->Divide(6, 3);
    for( int l = 0; l < NumOfPlaneAFT; l++ ){
      if( l%4 != 0 && l%4 != 1 ) continue;
      c->cd(l/2+1+l%2);
      TH1 *h = (TH1*)GHist::get( aft_chit_id+l );
      h->SetMinimum(0);
      if( h ) h->Draw();
      TH1 *hh = (TH1*)GHist::get( aft_chit_id+3*NumOfPlaneAFT+kUorD+l );
      if( !hh ) continue;
      hh->SetLineColor(kBlue);
      hh->Draw("same");
    }
    c->Update();
  }

  {  // draw Hitpattern Y-UDcoin
    TCanvas *c = (TCanvas*)gROOT->FindObject("c6");
    int aft_chit_id = HistMaker::getUniqueID( kAFT, 0, kHitPat, kUorD*NumOfPlaneAFT+101 );
    c->Clear();
    c->Divide(6, 3);
    for( int l = 0; l < NumOfPlaneAFT; l++ ){
      if( l%4 != 2 && l%4 != 3 ) continue;
      c->cd(l/2+l%2);
      TH1 *h = (TH1*)GHist::get( aft_chit_id+l );
      h->SetMinimum(0);
      if( h ) h->Draw();
      TH1 *hh = (TH1*)GHist::get( aft_chit_id+3*NumOfPlaneAFT+kUorD+l );
      if( !hh ) continue;
      hh->SetLineColor(kBlue);
      hh->Draw("same");
    }
    c->Update();
  }

  { // draw Hitpattern 2-Dim
    int aft_chit_id = HistMaker::getUniqueID( kAFT, 0, kHitPat, (kUorD+1)*NumOfPlaneAFT+101 );
    for( int i = 0; i < 2; i++ ){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", i+7));
      c->Clear();
      TH2 *h = (TH2*)GHist::get( aft_chit_id+i );
      if( h ) h->Draw("colz1");
      c->Update();
    }
  }




  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
