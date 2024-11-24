// -*- C++ -*-

#include "DetectorID.hh"
#include "UserParamMan.hh"

using namespace hddaq::gui; // for Updater

//_____________________________________________________________________________
void
dispAFT_Eff_Hitpat_Yadc( void )
{
  //  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------


  // draw Y Plane 8-11seg 1D HG UorD
  {
    int aft_hg_2d_id     = HistMaker::getUniqueID( kAFT, 0, kHighGain2D, 0 );
    TH1D *h_hg[4];
    TCanvas *c;
    for( int ud=0; ud<2; ud++ ){
	for( int iPlane=0; iPlane<NumOfPlaneAFT; ++iPlane ){
	  int planeId = iPlane/4+1;
	  TH2 *h_hg_2d  = (TH2*)GHist::get( ++aft_hg_2d_id);
	  if( !h_hg_2d) continue;
	  if(ud==0){
	    if( iPlane%12 == 0 ){
	      c = (TCanvas*)gROOT->FindObject(Form("c%d", iPlane/12+1));
	      c->Clear();
	      c->Divide(6, 4);
	    }
	    if( iPlane%4 == 2 || iPlane%4 == 3){
	      for( int iSeg = 0; iSeg < 4; ++iSeg ){
		if( iPlane%4 == 2){
		  c->cd(6*iSeg+2*((iPlane/4)%3)+1);
		  gPad->SetLogy();
		  h_hg[iSeg] = h_hg_2d->ProjectionY(Form("AFT_HG_%dU_Y0_seg%d", planeId, iSeg+8), iSeg+8, iSeg+8);
		  h_hg[iSeg]->SetTitle(Form("AFT_HG_%dU_Y0_seg%d", planeId, iSeg+8));
		  h_hg[iSeg]->Draw();
		}
		if( iPlane%4 == 3){
		  c->cd(6*iSeg+2*((iPlane/4)%3)+2);
		  gPad->SetLogy();
		  h_hg[iSeg] = h_hg_2d->ProjectionY(Form("AFT_HG_%dU_Y1_seg%d", planeId, iSeg+8), iSeg+8, iSeg+8);
		  h_hg[iSeg]->SetTitle(Form("AFT_HG_%dU_Y1_seg%d", planeId, iSeg+8));
		  h_hg[iSeg]->Draw();
		}
	      }
	    }
	    if( iPlane%12 == 11 ) c->Update();
	  }
	  if(ud==1){
	    if( iPlane%12 == 0 ){
	      c = (TCanvas*)gROOT->FindObject(Form("c%d", iPlane/12+4));
	      c->Clear();
	      c->Divide(6, 4);
	    }
	    if( iPlane%4 == 2 || iPlane%4 == 3){
	      for( int iSeg = 0; iSeg < 4; ++iSeg ){
		if( iPlane%4 == 2){
		  c->cd(6*iSeg+2*((iPlane/4)%3)+1);
		  gPad->SetLogy();
		  h_hg[iSeg] = h_hg_2d->ProjectionY(Form("AFT_HG_%dD_Y0_seg%d", planeId, iSeg+8), iSeg+8, iSeg+8);
		  h_hg[iSeg]->SetTitle(Form("AFT_HG_%dD_Y0_seg%d", planeId, iSeg+8));
		  h_hg[iSeg]->Draw();
		}
		if( iPlane%4 == 3){
		  c->cd(6*iSeg+2*((iPlane/4)%3)+2);
		  gPad->SetLogy();
		  h_hg[iSeg] = h_hg_2d->ProjectionY(Form("AFT_HG_%dD_Y1_seg%d", planeId, iSeg+8), iSeg+8, iSeg+8);
		  h_hg[iSeg]->SetTitle(Form("AFT_HG_%dD_Y1_seg%d", planeId, iSeg+8));
		  h_hg[iSeg]->Draw();
		}
	      }
	    }
	    if( iPlane%12 == 11 ) c->Update();
	  }
	}
    }
  }

  // draw Multi(w/adc) with X plane efficiency AFT
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c7");
    c->Clear();
    c->Divide(3,3);
    int aft_mul_id = HistMaker::getUniqueID(kAFT, 0, kMulti,  1);
    for(int i = 0; i<NumOfPlaneAFT/4; ++i){
      c->cd(i+1);
      TH1 *hh_wt  = (TH1*)GHist::get(aft_mul_id+3*NumOfPlaneAFT+i*2);
      hh_wt->SetLineColor(kRed);
      hh_wt->Draw();
      double CNof0     = hh_wt->GetBinContent(1);
      double CNofTotal = hh_wt->GetEntries();
      double Ceff      = 1. - (double)CNof0/CNofTotal;

      double xpos  = hh_wt->GetXaxis()->GetBinCenter(hh_wt->GetNbinsX())*0.3;
      double ypos  = hh_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.2f", Ceff));
      text->SetTextSize(0.06);
      text->Draw();
      c->Modified();
      c->Update();
    }
  }

  // draw Multi(w/adc) with Y plane efficiency AFT
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c8");
    c->Clear();
    c->Divide(3,3);
    int aft_mul_id = HistMaker::getUniqueID(kAFT, 0, kMulti,  1);
    for(int i = 0; i<NumOfPlaneAFT/4; ++i){
      c->cd(i+1);
      TH1 *hh_wt  = (TH1*)GHist::get(aft_mul_id+3*NumOfPlaneAFT+i*2+1);
      hh_wt->SetLineColor(kRed);
      hh_wt->Draw();
      double CNof0     = hh_wt->GetBinContent(1);
      double CNofTotal = hh_wt->GetEntries();
      double Ceff      = 1. - (double)CNof0/CNofTotal;

      double xpos  = hh_wt->GetXaxis()->GetBinCenter(hh_wt->GetNbinsX())*0.3;
      double ypos  = hh_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.2f", Ceff));
      text->SetTextSize(0.06);
      text->Draw();
      c->Modified();
      c->Update();
    }
  }

  {  // draw Hitpattern X-UDcoin
    TCanvas *c = (TCanvas*)gROOT->FindObject("c9");
    int aft_chit_id = HistMaker::getUniqueID( kAFT, 0, kHitPat, kUorD*NumOfPlaneAFT+101 );
    c->Clear();
    c->Divide(6, 3);
    for( int l = 0; l < NumOfPlaneAFT; l++ ){
      if( l%4 != 0 && l%4 != 1 ) continue;
      c->cd(l/2+1+l%2);
      // TH1 *h = (TH1*)GHist::get( aft_chit_id+l );
      // h->SetMinimum(0);
      // if( h ) h->Draw();
      // TH1 *hh = (TH1*)GHist::get( aft_chit_id+3*NumOfPlaneAFT+kUorD+l );
      // if( !hh ) continue;
      // hh->SetLineColor(kBlue);
      // hh->Draw("same");
      TH1 *h = (TH1*)GHist::get( aft_chit_id+l );
      TH1 *hh = (TH1*)GHist::get( aft_chit_id+3*NumOfPlaneAFT+kUorD+l );
      h->SetMinimum(0);
      hh->SetLineColor(kBlue);
      if( hh ) hh->Draw();
      if( !h ) continue;
      h->Draw("same");
    }
    c->Update();
  }

  {  // draw Hitpattern Y-UDcoin
    TCanvas *c = (TCanvas*)gROOT->FindObject("c10");
    int aft_chit_id = HistMaker::getUniqueID( kAFT, 0, kHitPat, kUorD*NumOfPlaneAFT+101 );
    c->Clear();
    c->Divide(6, 3);
    for( int l = 0; l < NumOfPlaneAFT; l++ ){
      if( l%4 != 2 && l%4 != 3 ) continue;
      c->cd(l/2+l%2);
      // TH1 *h = (TH1*)GHist::get( aft_chit_id+l );
      // h->SetMinimum(0);
      // if( h ) h->Draw();
      // TH1 *hh = (TH1*)GHist::get( aft_chit_id+3*NumOfPlaneAFT+kUorD+l );
      // if( !hh ) continue;
      // hh->SetLineColor(kBlue);
      // hh->Draw("same");

      TH1 *h = (TH1*)GHist::get( aft_chit_id+l );
      TH1 *hh = (TH1*)GHist::get( aft_chit_id+3*NumOfPlaneAFT+kUorD+l );
      h->SetMinimum(0);
      hh->SetLineColor(kBlue);
      if( hh ) hh->Draw();
      if( !h ) continue;
      h->Draw("same");
    }
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
