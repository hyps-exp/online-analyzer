#include "DetectorID.hh"

// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

#include "UserParamMan.hh"

void dispBH2_E42()
{
  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------
  //
  // TDC gate range
  static const unsigned int tdc_min = gUser.GetParameter("BH2_E42_TDC_FPGA", 0);
  static const unsigned int tdc_max = gUser.GetParameter("BH2_E42_TDC_FPGA", 1);

  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    TCanvas *c3 = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(4,4);
    c3->Clear();
    c3->Divide(4,4);
    int icanvas = 0;
    int icanvas3 = 0;
    for( int ud=0; ud<NumOfSegBH2_E42; ++ud ){
      TH1 *h  = NULL;
      TH1 *hh = NULL;
      // draw ADC
      c->cd( ++icanvas );
      gPad->SetLogy();
      h = (TH1*)GHist::get( HistMaker::getUniqueID(kBH2_E42, 0, kADC, ud+1) );
      if( h ){
	h->GetXaxis()->SetRangeUser(0,0x1000);
	h->Draw();
      }
      hh = (TH1*)GHist::get( HistMaker::getUniqueID(kBH2_E42, 0, kADCwTDC, ud+1) );
      if( hh ){
	hh->GetXaxis()->SetRangeUser(0,0x1000);
	hh->SetLineColor( kRed );
	hh->Draw("same");
      }
    }
    for( int ud=0; ud<NumOfSegBH2_E42; ++ud ){
      TH1 *h  = NULL;
      // draw TDC_FPGA
      c3->cd( ++icanvas3 );
      //      gPad->SetLogy();
      h = (TH1*)GHist::get( HistMaker::getUniqueID(kBH2_E42, 0, kTDC, ud+1) );
      if( h ){
	//h->GetXaxis()->SetRangeUser(0,2000);
	h->GetXaxis()->SetRangeUser(tdc_min,tdc_max);
	h->Draw();
      }
    }
    c->Update();
    c->cd(0);
    c3->Update();
    c3->cd(0);
  }

  {
    TCanvas *c2 = (TCanvas*)gROOT->FindObject("c2");
    TCanvas *c4 = (TCanvas*)gROOT->FindObject("c4");
    c2->Clear();
    c4->Clear();
    c2->Divide(4,4);
    c4->Divide(4,4);
    int icanvas2 = 0;
    int icanvas4 = 0;
    for( int ud=NumOfSegBH2_E42; ud<NumOfSegBH2_E42*2; ++ud ){
      TH1 *h  = NULL;
      TH1 *hh = NULL;
      // draw ADC
      c2->cd( ++icanvas2 );
      gPad->SetLogy();
      h = (TH1*)GHist::get( HistMaker::getUniqueID(kBH2_E42, 0, kADC, ud+1) );
      if( h ){
	h->GetXaxis()->SetRangeUser(0,0x1000);
	h->Draw();
      }
      hh = (TH1*)GHist::get( HistMaker::getUniqueID(kBH2_E42, 0, kADCwTDC, ud+1) );
      if( hh ){
	hh->GetXaxis()->SetRangeUser(0,0x1000);
	hh->SetLineColor( kRed );
	hh->Draw("same");
      }
    }
    for( int ud=NumOfSegBH2_E42; ud<NumOfSegBH2_E42*2; ++ud ){
      TH1 *h  = NULL;
      // draw TDC_FPGA
      c4->cd( ++icanvas4 );
      //      gPad->SetLogy();
      h = (TH1*)GHist::get( HistMaker::getUniqueID(kBH2_E42, 0, kTDC, ud+1) );
      if( h ){
	//h->GetXaxis()->SetRangeUser(0,2000);
	h->GetXaxis()->SetRangeUser(tdc_min,tdc_max);
	h->Draw();
      }
    }
    c2->Update();
    c2->cd(0);
    c4->Update();
    c4->cd(0);
  }


  {
    TCanvas *c5 = (TCanvas*)gROOT->FindObject("c5");
    TCanvas *c6 = (TCanvas*)gROOT->FindObject("c6");
    c5->Clear();
    c5->Divide(2,2);
    c6->Clear();
    c6->Divide(2,2);
    int icanvas5 = 0;
    int icanvas6 = 0;
    for( int itr=0; itr<3; ++itr ){
      TH1 *h1  = NULL;
      TH1 *h2 = NULL;
      TH1 *h3 = NULL;
      // draw Hitpat

      c5->cd( ++icanvas5 );
      if(itr==2)
      c5->cd( icanvas5+1 );
      //      gPad->SetLogy();
      //unbias (raw hitpat)
      h1 = (TH1*)GHist::get( HistMaker::getUniqueID(kBH2_E42, 0, kHitPat, itr*3+1))->Clone();
      if( h1 ){
	h1->Draw();
      }
      //cor hitpat
      h2 = (TH1*)GHist::get( HistMaker::getUniqueID(kBH2_E42, 0, kHitPat, itr*3+2))->Clone();
      if( h2 ){
	h2->SetLineColor( kRed );
	h2->Draw("same");
      }
      //Max hitpat
      h3 = (TH1*)GHist::get( HistMaker::getUniqueID(kBH2_E42, 0, kHitPat, itr*3+3))->Clone();
      if( h3 ){
	h3->SetLineColor( kGreen );
	h3->Draw("same");
      }
    }

    for( int itr=0; itr<3; ++itr ){
      TH1 *h1  = NULL;
      TH1 *h2 = NULL;
      // draw Multiplicity
      c6->cd( ++icanvas6 );
      if(itr==2)
	c6->cd( icanvas6+1 );
      //      gPad->SetLogy();
      //unbias (raw multi)
      h1 = (TH1*)GHist::get( HistMaker::getUniqueID(kBH2_E42, 0, kMulti, itr*2+1))->Clone();
      h2 = (TH1*)GHist::get( HistMaker::getUniqueID(kBH2_E42, 0, kMulti, itr*2+2))->Clone();
      if( h1 ){
	h1->GetYaxis()->SetRangeUser(0, 1.2*h2->GetMaximum());
	h1->Draw();
      }
      //cor multi

      if( h2 ){
	h2->SetLineColor( kRed );
	h2->Draw("same");
      }
    }




    c5->Update();
    c5->cd(0);
    c6->Update();
    c6->cd(0);
  }




  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
