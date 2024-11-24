/ -*- C++ -*-

#include "DetectorID.hh"

#include "UserParamMan.hh"

using namespace hddaq::gui; // for Updater

//_____________________________________________________________________________
void
dispVMEEASIROC1D( void )
{

  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // draw TDC-1D
  { // aft01
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(3, 5);
    int vmeeasiroc_t_2d_id = HistMaker::getUniqueID(kVMEEASIROC, 0, kTDC2D,   1);
    for( int i=0; i<NumOfPlaneVMEEASIROC; ++i ){
      int planeId = PlaneIdOfVMEEASIROC[i];
      if( i >= 12 ) break;
      c->cd(i+1);
      TH2 *h = (TH2*)GHist::get( vmeeasiroc_t_2d_id + i );
      if( !h ) continue;
      TH1D *hh =  h->ProjectionY(Form("VMEEASIROC_TDC_%d_1D", planeId), 1, NumOfSegVMEEASIROC);
      hh->SetTitle(Form("VMEEASIROC_TDC_%d_1D", planeId));
      hh->Draw();
    }
    c->Update();
  }

  // draw TDC-1D
  { // aft02
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(3, 5);
    int vmeeasiroc_t_2d_id = HistMaker::getUniqueID(kVMEEASIROC, 0, kTDC2D,   1);
    for( int i=0; i<NumOfPlaneVMEEASIROC; ++i ){
      int planeId = PlaneIdOfVMEEASIROC[i];
      if( i < 12 ) continue;
      c->cd((i-12)+1);
      TH2 *h = (TH2*)GHist::get( vmeeasiroc_t_2d_id + i );
      if( !h ) continue;
      TH1D *hh =  h->ProjectionY(Form("VMEEASIROC_TDC_%d_1D", planeId), 1, NumOfSegVMEEASIROC);
      hh->SetTitle(Form("VMEEASIROC_TDC_%d_1D", planeId));
      hh->Draw();
    }
    c->Update();
  }

  const auto& gUser = UserParamMan::GetInstance();
  const Double_t TotRef = gUser.GetParameter( "TotRefVMEEASIROC" );
  // draw TOT-1D
  { // aft01
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(3, 5);
    int vmeeasiroc_tot_2d_id = HistMaker::getUniqueID(kVMEEASIROC, 0, kTOT2D,   1);
    for( int i=0; i<NumOfPlaneVMEEASIROC; ++i ){
      int planeId = PlaneIdOfVMEEASIROC[i];
      if( i >= 12 ) break;
      c->cd(i+1);
      TH2 *h = (TH2*)GHist::get( vmeeasiroc_tot_2d_id + i );
      if( !h ) continue;
      TH1D *hh =  h->ProjectionY(Form("VMEEASIROC_TOT_%d_1D", planeId), 1, NumOfSegVMEEASIROC);
      hh->SetTitle(Form("VMEEASIROC_TOT_%d_1D", planeId));
      hh->Draw();

      Double_t peak = hh->GetMaximum();
      auto l = new TLine( TotRef, 0, TotRef, peak );
      l->SetLineColor( kRed );
      l->Draw( "same" );

      TLatex *text = new TLatex();
      text->SetNDC();
      text->SetTextSize(0.08);
      if( i < 9 ){
	if( i%3 == 0 || i%3 == 1 ) text->DrawLatex(0.75, 0.60, "AFT-X1-3");
	if( i%3 == 2 ) text->DrawLatex(0.75, 0.60, "AFT-Y1-3");
      }
      else {
	if( i%3 == 0 || i%3 == 1 ) text->DrawLatex(0.75, 0.60, "AFT-X4-6");
	if( i%3 == 2 ) text->DrawLatex(0.75, 0.60, "AFT-Y4-6");
      }
    }
    c->Update();
  }

  // draw TOT-1D
  { // aft02
    TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
    c->Clear();
    c->Divide(3, 5);
    int vmeeasiroc_tot_2d_id = HistMaker::getUniqueID(kVMEEASIROC, 0, kTOT2D,   1);
    for( int i=0; i<NumOfPlaneVMEEASIROC; ++i ){
      int planeId = PlaneIdOfVMEEASIROC[i];
      if( i < 12 ) continue;
      c->cd((i-12)+1);
      TH2 *h = (TH2*)GHist::get( vmeeasiroc_tot_2d_id + i );
      if( !h ) continue;
      TH1D *hh =  h->ProjectionY(Form("VMEEASIROC_TOT_%d_1D", planeId), 1, NumOfSegVMEEASIROC);
      hh->SetTitle(Form("VMEEASIROC_TOT_%d_1D", planeId));
      hh->Draw();

      Double_t peak = hh->GetMaximum();
      auto l = new TLine( TotRef, 0, TotRef, peak );
      l->SetLineColor( kRed );
      l->Draw( "same" );

      TLatex *text = new TLatex();
      text->SetNDC();
      text->SetTextSize(0.08);
      if( i < 18 ){
	if( i%3 == 0 || i%3 == 1 ) text->DrawLatex(0.75, 0.60, "AFT-X4-6");
	if( i%3 == 2 ) text->DrawLatex(0.75, 0.60, "AFT-Y4-6");
      }
      else {
	if( i%3 == 0 || i%3 == 1 ) text->DrawLatex(0.75, 0.60, "AFT-X7-9");
	if( i%3 == 2 ) text->DrawLatex(0.75, 0.60, "AFT-Y7-9");
      }

    }
    c->Update();
  }

  // draw HighGain-1D
  { // aft01
    TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
    c->Clear();
    c->Divide(3, 5);
    int vmeeasiroc_hg_2d_id  = HistMaker::getUniqueID(kVMEEASIROC, 0, kHighGain, 11);
    for( int i=0; i<NumOfPlaneVMEEASIROC; ++i ){
      int planeId = PlaneIdOfVMEEASIROC[i];
      if( i >= 12 ) break;
      c->cd(i+1);
      TH2 *h = (TH2*)GHist::get( vmeeasiroc_hg_2d_id + i );
      if( !h ) continue;
      TH1D *hh =  h->ProjectionY(Form("VMEEASIROC_HighGain_%d_1D", planeId), 1, NumOfSegVMEEASIROC);
      hh->SetTitle(Form("VMEEASIROC_HighGain_%d_1D", planeId));
      hh->Draw();

      // double stddev_mean = 0.;
      // for( int iSeg = 0; iSeg < NumOfSegVMEEASIROC; ++iSeg ){
      // 	TH1I *h_seg  = (TH1I*)h->ProjectionY(Form("h_seg_%d", iSeg), iSeg+1, iSeg+1);
      // 	stddev_mean += h_seg->GetStdDev();
      // 	if( iSeg == NumOfSegVMEEASIROC/2-1 ){
      // 	  stddev_mean /= NumOfSegVMEEASIROC/2.;
      // 	  double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.1;
      // 	  double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
      // 	  TLatex *text = new TLatex(xpos, ypos, Form("1st half stddev %.2f", stddev_mean));
      // 	  text->SetTextSize(0.08);
      // 	  text->Draw();
      // 	  stddev_mean = 0.;
      // 	  continue;
      // 	}
      // 	else if( iSeg == NumOfSegVMEEASIROC-1 ){
      // 	  stddev_mean /= NumOfSegVMEEASIROC/2.;
      // 	  double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.6;
      // 	  double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
      // 	  TLatex *text = new TLatex(xpos, ypos, Form("2nd half stddev %.2f", stddev_mean));
      // 	  text->SetTextSize(0.08);
      // 	  text->Draw();
      // 	}
      // }
    }
    c->Update();
  }

  // draw HighGain-1D
  { // aft02
    TCanvas *c = (TCanvas*)gROOT->FindObject("c6");
    c->Clear();
    c->Divide(3, 5);
    int vmeeasiroc_hg_2d_id  = HistMaker::getUniqueID(kVMEEASIROC, 0, kHighGain, 11);
    for( int i=0; i<NumOfPlaneVMEEASIROC; ++i ){
      int planeId = PlaneIdOfVMEEASIROC[i];
      if( i < 12 ) continue;
      c->cd((i-12)+1);
      TH2 *h = (TH2*)GHist::get( vmeeasiroc_hg_2d_id + i );
      if( !h ) continue;
      TH1D *hh =  h->ProjectionY(Form("VMEEASIROC_HighGain_%d_1D", planeId), 1, NumOfSegVMEEASIROC);
      hh->SetTitle(Form("VMEEASIROC_HighGain_%d_1D", planeId));
      hh->Draw();

      // double stddev_mean = 0.;
      // for( int iSeg = 0; iSeg < NumOfSegVMEEASIROC; ++iSeg ){
      // 	TH1I *h_seg  = (TH1I*)h->ProjectionY(Form("h_seg_%d", iSeg), iSeg+1, iSeg+1);
      // 	stddev_mean += h_seg->GetStdDev();
      // 	if( iSeg == NumOfSegVMEEASIROC/2-1 ){
      // 	  stddev_mean /= NumOfSegVMEEASIROC/2.;
      // 	  double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.1;
      // 	  double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
      // 	  TLatex *text = new TLatex(xpos, ypos, Form("1st half stddev %.2f", stddev_mean));
      // 	  text->SetTextSize(0.08);
      // 	  text->Draw();
      // 	  stddev_mean = 0.;
      // 	  continue;
      // 	}
      // 	else if( iSeg == NumOfSegVMEEASIROC-1 ){
      // 	  stddev_mean /= NumOfSegVMEEASIROC/2.;
      // 	  double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.6;
      // 	  double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
      // 	  TLatex *text = new TLatex(xpos, ypos, Form("2nd half stddev %.2f", stddev_mean));
      // 	  text->SetTextSize(0.08);
      // 	  text->Draw();
      // 	}
      // }
    }
    c->Update();
  }

  // draw LowGain-1D
  { // aft01
    TCanvas *c = (TCanvas*)gROOT->FindObject("c7");
    c->Clear();
    c->Divide(3, 5);
    int vmeeasiroc_lg_2d_id  = HistMaker::getUniqueID(kVMEEASIROC, 0, kLowGain, 11);
    for( int i=0; i<NumOfPlaneVMEEASIROC; ++i ){
      int planeId = PlaneIdOfVMEEASIROC[i];
      if( i >= 12 ) break;
      c->cd(i+1);
      TH2 *h = (TH2*)GHist::get( vmeeasiroc_lg_2d_id + i );
      if( !h ) continue;
      TH1D *hh =  h->ProjectionY(Form("VMEEASIROC_LowGain_%d_1D", planeId), 1, NumOfSegVMEEASIROC);
      hh->SetTitle(Form("VMEEASIROC_LowGain_%d_1D", planeId));
      hh->Draw();

      // double stddev_mean = 0.;
      // for( int iSeg = 0; iSeg < NumOfSegVMEEASIROC; ++iSeg ){
      // 	TH1I *h_seg  = (TH1I*)h->ProjectionY(Form("h_seg_%d", iSeg), iSeg+1, iSeg+1);
      // 	stddev_mean += h_seg->GetStdDev();
      // 	if( iSeg == NumOfSegVMEEASIROC/2-1 ){
      // 	  stddev_mean /= NumOfSegVMEEASIROC/2.;
      // 	  double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.1;
      // 	  double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
      // 	  TLatex *text = new TLatex(xpos, ypos, Form("1st half stddev %.2f", stddev_mean));
      // 	  text->SetTextSize(0.08);
      // 	  text->Draw();
      // 	  stddev_mean = 0.;
      // 	  continue;
      // 	}
      // 	else if( iSeg == NumOfSegVMEEASIROC-1 ){
      // 	  stddev_mean /= NumOfSegVMEEASIROC/2.;
      // 	  double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.6;
      // 	  double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
      // 	  TLatex *text = new TLatex(xpos, ypos, Form("2nd half stddev %.2f", stddev_mean));
      // 	  text->SetTextSize(0.08);
      // 	  text->Draw();
      // 	}
      // }
    }
    c->Update();
  }

  // draw LowGain-1D
  { // aft02
    TCanvas *c = (TCanvas*)gROOT->FindObject("c8");
    c->Clear();
    c->Divide(3, 5);
    int vmeeasiroc_lg_2d_id  = HistMaker::getUniqueID(kVMEEASIROC, 0, kLowGain, 11);
    for( int i=0; i<NumOfPlaneVMEEASIROC; ++i ){
      int planeId = PlaneIdOfVMEEASIROC[i];
      if( i < 12 ) continue;
      c->cd((i-12)+1);
      TH2 *h = (TH2*)GHist::get( vmeeasiroc_lg_2d_id + i );
      if( !h ) continue;
      TH1D *hh =  h->ProjectionY(Form("VMEEASIROC_LowGain_%d_1D", planeId), 1, NumOfSegVMEEASIROC);
      hh->SetTitle(Form("VMEEASIROC_LowGain_%d_1D", planeId));
      hh->Draw();

      // double stddev_mean = 0.;
      // for( int iSeg = 0; iSeg < NumOfSegVMEEASIROC; ++iSeg ){
      // 	TH1I *h_seg  = (TH1I*)h->ProjectionY(Form("h_seg_%d", iSeg), iSeg+1, iSeg+1);
      // 	stddev_mean += h_seg->GetStdDev();
      // 	if( iSeg == NumOfSegVMEEASIROC/2-1 ){
      // 	  stddev_mean /= NumOfSegVMEEASIROC/2.;
      // 	  double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.1;
      // 	  double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
      // 	  TLatex *text = new TLatex(xpos, ypos, Form("1st half stddev %.2f", stddev_mean));
      // 	  text->SetTextSize(0.08);
      // 	  text->Draw();
      // 	  stddev_mean = 0.;
      // 	  continue;
      // 	}
      // 	else if( iSeg == NumOfSegVMEEASIROC-1 ){
      // 	  stddev_mean /= NumOfSegVMEEASIROC/2.;
      // 	  double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.6;
      // 	  double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
      // 	  TLatex *text = new TLatex(xpos, ypos, Form("2nd half stddev %.2f", stddev_mean));
      // 	  text->SetTextSize(0.08);
      // 	  text->Draw();
      // 	}
      // }
    }
    c->Update();
  }


  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------

}
