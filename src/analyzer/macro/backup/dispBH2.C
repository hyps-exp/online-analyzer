#include "DetectorID.hh"

// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

#include "UserParamMan.hh"

void dispBH2()
{
  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------
  //
  // TDC gate range
  static const unsigned int tdc_min = gUser.GetParameter("TdcBH2", 0);
  static const unsigned int tdc_max = gUser.GetParameter("TdcBH2", 1);

  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    TCanvas *c2 = (TCanvas*)gROOT->FindObject("c2");
    TCanvas *c3 = (TCanvas*)gROOT->FindObject("c3");
    TCanvas *c4 = (TCanvas*)gROOT->FindObject("c4");
    c->Clear();
    c->Divide(4,2);
    c2->Clear();
    c2->Divide(4,2);
    c3->Clear();
    c3->Divide(4,2);
    c4->Clear();
    c4->Divide(4,2);

    int icanvas = 0;
    int icanvas3 = 0;
    for( int ud=0; ud<NumOfSegBH2; ++ud ){
      TH1 *h  = NULL;
      TH1 *hh = NULL;
      // draw ADC
      c->cd( ++icanvas );
      gPad->SetLogy();
      h = (TH1*)GHist::get( HistMaker::getUniqueID(kBH2, 0, kADC, ud+1) );
      if( h ){
	h->GetXaxis()->SetRangeUser(0,2000);
	h->Draw();
      }
      hh = (TH1*)GHist::get( HistMaker::getUniqueID(kBH2, 0, kADCwTDC, ud+1) );
      if( hh ){
	hh->GetXaxis()->SetRangeUser(0,2000);
	hh->SetLineColor( kRed );
	hh->Draw("same");
      }
    }
    for( int ud=0; ud<NumOfSegBH2; ++ud ){
      TH1 *h  = NULL;
      // draw TDC
      c2->cd( ++icanvas3 );
      //      gPad->SetLogy();
      h = (TH1*)GHist::get( HistMaker::getUniqueID(kBH2, 0, kTDC, ud+1) );
      if( h ){
	//h->GetXaxis()->SetRangeUser(0,2000);
	h->GetXaxis()->SetRangeUser(tdc_min,tdc_max);
	h->Draw();
      }
    }
    c->Update();
    c->cd(0);
    c2->Update();
    c2->cd(0);

    icanvas = 0;
    icanvas3 = 0;
    for( int ud=0; ud<NumOfSegBH2; ++ud ){
      TH1 *h  = NULL;
      TH1 *hh = NULL;
      // draw ADC
      c3->cd( ++icanvas );
      gPad->SetLogy();
      h = (TH1*)GHist::get( HistMaker::getUniqueID(kBH2, 0, kADC, ud+1+NumOfSegBH2) );
      if( h ){
	h->GetXaxis()->SetRangeUser(0,2000);
	h->Draw();
      }
      hh = (TH1*)GHist::get( HistMaker::getUniqueID(kBH2, 0, kADCwTDC, ud+1+NumOfSegBH2) );
      if( hh ){
	hh->GetXaxis()->SetRangeUser(0,2000);
	hh->SetLineColor( kRed );
	hh->Draw("same");
      }
    }
    for( int ud=0; ud<NumOfSegBH2; ++ud ){
      TH1 *h  = NULL;
      // draw TDC
      c4->cd( ++icanvas3 );
      //      gPad->SetLogy();
      h = (TH1*)GHist::get( HistMaker::getUniqueID(kBH2, 0, kTDC, ud+1+NumOfSegBH2) );
      if( h ){
	//h->GetXaxis()->SetRangeUser(0,2000);
	h->GetXaxis()->SetRangeUser(tdc_min,tdc_max);
	h->Draw();
      }
    }
    c3->Update();
    c3->cd(0);
    c4->Update();
    c4->cd(0);


  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
