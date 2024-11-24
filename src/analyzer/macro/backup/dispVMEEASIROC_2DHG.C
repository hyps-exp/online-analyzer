// -*- C++ -*-

#include "DetectorID.hh"

#include "UserParamMan.hh"

using namespace hddaq::gui; // for Updater

//_____________________________________________________________________________
void
dispVMEEASIROC_2DHG( void )
{
  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------


  // draw HighGain-2D
  { // aft01
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(3, 3);
    int vmeeasiroc_hg_2d_id  = HistMaker::getUniqueID(kVMEEASIROC, 0, kHighGain, 11);
    for( int i=0; i<NumOfPlaneVMEEASIROC; ++i ){
      if( i >= 9 ) break;
      c->cd(i+1);
      TH2 *h = (TH2*)GHist::get( vmeeasiroc_hg_2d_id + i );
      if( !h ) continue;
      h->GetYaxis()->SetRangeUser(700, 1100);
      gPad->SetLogz();
      h->Draw("colz");

    }
    c->Update();
  }

  // draw HighGain-2D
  { // aft02
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(3, 3);
    int vmeeasiroc_hg_2d_id  = HistMaker::getUniqueID(kVMEEASIROC, 0, kHighGain, 11);
    for( int i=0; i<NumOfPlaneVMEEASIROC; ++i ){
      //if( i < 12 ) continue;
      if( i < 9  ) continue;
      if( i >= 18 ) break;
      c->cd((i-9)+1);
      TH2 *h = (TH2*)GHist::get( vmeeasiroc_hg_2d_id + i );
      if( !h ) continue;
      h->GetYaxis()->SetRangeUser(700, 1100);
      gPad->SetLogz();
      h->Draw("colz");

    }
    c->Update();
  }

  // draw HighGain-2D
  { // aft03
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(3, 3);
    int vmeeasiroc_hg_2d_id  = HistMaker::getUniqueID(kVMEEASIROC, 0, kHighGain, 11);
    for( int i=0; i<NumOfPlaneVMEEASIROC; ++i ){
      if( i < 18 ) continue;
      c->cd((i-18)+1);
      TH2 *h = (TH2*)GHist::get( vmeeasiroc_hg_2d_id + i );
      if( !h ) continue;
      h->GetYaxis()->SetRangeUser(700, 1100);
      gPad->SetLogz();
      h->Draw("colz");
    }
    c->Update();
  }


  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------

}
