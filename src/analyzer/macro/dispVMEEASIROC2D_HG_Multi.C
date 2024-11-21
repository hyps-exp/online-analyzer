// -*- C++ -*-

#include "DetectorID.hh"

#include "UserParamMan.hh"

using namespace hddaq::gui; // for Updater

//_____________________________________________________________________________
void
dispVMEEASIROC2D_HG_Multi( void )
{
  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  Int_t nplane[5] = {18, 19, 19, 18, 5};
  { // aft01
    Int_t first = 0;
    for( Int_t j=0; j<5; j++ ){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", j+1));
      c->Clear();
      c->Divide(4, 5);
      int vmeeasiroc_hg_2d_id = HistMaker::getUniqueID(kVMEEASIROC, 0, kHighGain,   10);
      for( int i=0; i<NumOfPlaneVMEEASIROC; ++i ){
	if( !(first<=i && i<(first+nplane[j])) )continue;
	c->cd(i-first+1);
	TH2 *h = (TH2*)GHist::get( vmeeasiroc_hg_2d_id + i );
	if( !h ) continue;
	h->Draw("colz");

	double stddev_mean = 0.;
	for( int iSeg = 0; iSeg < NumOfSegVMEEASIROC; ++iSeg ){
	  TH1I *h_seg  = (TH1I*)h->ProjectionY(Form("h_seg_%d", iSeg), iSeg+1, iSeg+1);
	  stddev_mean += h_seg->GetStdDev();
	  if( iSeg == NumOfSegVMEEASIROC/2-1 ){
	    stddev_mean /= NumOfSegVMEEASIROC/2.;
	    double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.1;
	    double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
	    TLatex *text = new TLatex(xpos, ypos, Form("%.2f", stddev_mean));
	    text->SetTextSize(0.16);
	    text->Draw();
	    stddev_mean = 0.;
	    continue;
	  }
	  else if( iSeg == NumOfSegVMEEASIROC-1 ){
	    stddev_mean /= NumOfSegVMEEASIROC/2.;
	    double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.6;
	    double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
	    TLatex *text = new TLatex(xpos, ypos, Form("%.2f", stddev_mean));
	    text->SetTextSize(0.16);
	    text->Draw();
	  }
	}
      }
      first += nplane[j];
      c->Update();
    }
  }

  { // aft01
    Int_t first = 0;
    for( Int_t j=0; j<5; j++ ){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", j+6));
      c->Clear();
      c->Divide(4, 5);
      int vmeeasiroc_multihit_2d_id = HistMaker::getUniqueID(kVMEEASIROC, 0, kMultiHitTdc,   20);
      for( int i=0; i<NumOfPlaneVMEEASIROC; ++i ){
	if( !(first<=i && i<(first+nplane[j])) )continue;
	c->cd(i-first+1);
	TH2 *h = (TH2*)GHist::get( vmeeasiroc_multihit_2d_id + i );
	if( !h ) continue;
	h->Draw("colz");

	double eff_mean = 0.;
	for( int iSeg = 0; iSeg < NumOfSegVMEEASIROC; ++iSeg ){
	  TH1I *h_seg  = (TH1I*)h->ProjectionY(Form("h_seg_%d", iSeg), iSeg+1, iSeg+1);
	  double Nof0     = h_seg->GetBinContent(1);
	  double NofTotal = h_seg->GetEntries();
	  eff_mean += 1. - (double)Nof0/NofTotal;
	  if( iSeg == NumOfSegVMEEASIROC/2-1 ){
	    eff_mean /= NumOfSegVMEEASIROC/2.;
	    double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.1;
	    double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
	    TLatex *text = new TLatex(xpos, ypos, Form("%.2f", eff_mean));
	    text->SetTextSize(0.16);
	    text->Draw();
	    eff_mean = 0.;
	    continue;
	  }
	  else if( iSeg == NumOfSegVMEEASIROC-1 ){
	    eff_mean /= NumOfSegVMEEASIROC/2.;
	    double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.6;
	    double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
	    TLatex *text = new TLatex(xpos, ypos, Form("%.2f", eff_mean));
	    text->SetTextSize(0.16);
	    text->Draw();
	  }
	}
      }
      first += nplane[j];
      c->Update();
    }
  }


  // // draw HighGain-2D
  // { // aft01
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
  //   c->Clear();
  //   c->Divide(3, 5);
  //   int vmeeasiroc_hg_2d_id  = HistMaker::getUniqueID(kVMEEASIROC, 0, kHighGain, 11);
  //   for( int i=0; i<NumOfPlaneVMEEASIROC; ++i ){
  //     if( i >= 12 ) break;
  //     c->cd(i+1);
  //     TH2 *h = (TH2*)GHist::get( vmeeasiroc_hg_2d_id + i );
  //     if( !h ) continue;
  //     // h->GetYaxis()->SetRangeUser(700, 1100);
  //     h->Draw("colz");

  //     double stddev_mean = 0.;
  //     for( int iSeg = 0; iSeg < NumOfSegVMEEASIROC; ++iSeg ){
  //   	TH1I *h_seg  = (TH1I*)h->ProjectionY(Form("h_seg_%d", iSeg), iSeg+1, iSeg+1);
  // 	stddev_mean += h_seg->GetStdDev();
  //   	if( iSeg == NumOfSegVMEEASIROC/2-1 ){
  //   	  stddev_mean /= NumOfSegVMEEASIROC/2.;
  //   	  double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.1;
  //   	  double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
  //   	  TLatex *text = new TLatex(xpos, ypos, Form("1st half stddev %.2f", stddev_mean));
  //   	  text->SetTextSize(0.08);
  //   	  text->Draw();
  //   	  stddev_mean = 0.;
  //   	  continue;
  //   	}
  //   	else if( iSeg == NumOfSegVMEEASIROC-1 ){
  //   	  stddev_mean /= NumOfSegVMEEASIROC/2.;
  //   	  double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.6;
  //   	  double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
  //   	  TLatex *text = new TLatex(xpos, ypos, Form("2nd half stddev %.2f", stddev_mean));
  //   	  text->SetTextSize(0.08);
  //   	  text->Draw();
  //   	}
  //     }
  //   }
  //   c->Update();
  // }

  // // draw MultiHitTdc-2D
  // { // aft01
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c9");
  //   c->Clear();
  //   c->Divide(3, 5);
  //   int vmeeasiroc_multihit_2d_id = HistMaker::getUniqueID(kVMEEASIROC, 0, kMultiHitTdc, 21);
  //   for( int i=0; i<NumOfPlaneVMEEASIROC; ++i ){
  //     if( i >= 12 ) break;
  //     c->cd(i+1);
  //     TH2 *h = (TH2*)GHist::get( vmeeasiroc_multihit_2d_id + i );
  //     if( !h ) continue;
  //     h->Draw("colz");

    //   double eff_mean = 0.;
    //   for( int iSeg = 0; iSeg < NumOfSegVMEEASIROC; ++iSeg ){
    // 	TH1I *h_seg  = (TH1I*)h->ProjectionY(Form("h_seg_%d", iSeg), iSeg+1, iSeg+1);
    // 	double Nof0     = h_seg->GetBinContent(1);
    // 	double NofTotal = h_seg->GetEntries();
    // 	eff_mean += 1. - (double)Nof0/NofTotal;
    // 	if( iSeg == NumOfSegVMEEASIROC/2-1 ){
    // 	  eff_mean /= NumOfSegVMEEASIROC/2.;
    // 	  double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.1;
    // 	  double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
    // 	  TLatex *text = new TLatex(xpos, ypos, Form("1st half eff. %.2f", eff_mean));
    // 	  text->SetTextSize(0.08);
    // 	  text->Draw();
    // 	  eff_mean = 0.;
    // 	  continue;
    // 	}
    // 	else if( iSeg == NumOfSegVMEEASIROC-1 ){
    // 	  eff_mean /= NumOfSegVMEEASIROC/2.;
    // 	  double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.6;
    // 	  double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
    // 	  TLatex *text = new TLatex(xpos, ypos, Form("2nd half eff. %.2f", eff_mean));
    // 	  text->SetTextSize(0.08);
    // 	  text->Draw();
    // 	}
    //   }
    // }
  //   c->Update();
  // }


  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------

}
