// -*- C++ -*-

#include "DetectorID.hh"
#include "UserParamMan.hh"

using namespace hddaq::gui; // for Updater

//_____________________________________________________________________________
void
dispAFT2D( void )
{
  //  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // draw TDC-2D UorD
  {
    int aft_t_2d_id     = HistMaker::getUniqueID( kAFT, 0, kTDC2D, 0 );
    for( int ud=0; ud<2; ud++ ){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", ud+1));
      c->Clear();
      c->Divide(8, 5);
      for( int i=0; i<NumOfPlaneAFT; ++i ){
	c->cd(i+1);
	TH2 *h = (TH2*)GHist::get( ++aft_t_2d_id );
	if( !h ) continue;
	h->Draw("colz");
      }
      for( int i = 0; i < 4; i++ ){
	c->cd(NumOfPlaneAFT+i+1);
	const TString layer_name = NameOfPlaneAFT[i%4];
	const char* title = NULL;
	if( ud == 0 ) title = Form("TDC_#U_%s_2D", layer_name.Data());
	if( ud == 1 ) title = Form("TDC_#D_%s_2D", layer_name.Data());
	double xpos = 0.1;
	double ypos = 0.5;
	TLatex *text = new TLatex(xpos, ypos, title);
	text->SetTextSize(0.12);
	text->Draw();
      }
      c->Update();
    }
  }

  // draw TOT-2D UorD
  {
    int aft_tot_2d_id     = HistMaker::getUniqueID( kAFT, 0, kTOT2D, 0 );
    int aft_ctot_2d_id    = HistMaker::getUniqueID( kAFT, 0, kTOT2D, 100 );
    for( int ud=0; ud<2; ud++ ){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", ud+3));
      c->Clear();
      c->Divide(8, 5);
      for( int i=0; i<NumOfPlaneAFT; ++i ){
	c->cd(i+1);
	TH2 *h = (TH2*)GHist::get( ++aft_tot_2d_id );
	if( !h ) continue;
	h->Draw("colz");
	// TH2 *hh = (TH2*)GHist::get( ++aft_ctot_2d_id );
	// if( !hh ) continue;
	// // hh->GetXaxis()->SetRangeUser( 0, 4096 );
	// hh->SetLineColor( kRed );
	// hh->Draw("same");
	for( int i = 0; i < 4; i++ ){
	  c->cd(NumOfPlaneAFT+i+1);
	  const TString layer_name = NameOfPlaneAFT[i%4];
	  const char* title = NULL;
	  if( ud == 0 ) title = Form("TOT_#U_%s_2D", layer_name.Data());
	  if( ud == 1 ) title = Form("TOT_#D_%s_2D", layer_name.Data());
	  double xpos = 0.1;
	  double ypos = 0.5;
	  TLatex *text = new TLatex(xpos, ypos, title);
	  text->SetTextSize(0.12);
	  text->Draw();
	}
      }
      c->Update();
    }
  }

  // draw HighGain UorD
  {
    int aft_hg_2d_id     = HistMaker::getUniqueID( kAFT, 0, kHighGain2D, 0 );
    int aft_chg_2d_id    = HistMaker::getUniqueID( kAFT, 0, kHighGain2D, 100 );
    for( int ud=0; ud<2; ud++ ){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", ud+5));
      c->Clear();
      c->Divide(8, 5);
      for( int i=0; i<NumOfPlaneAFT; ++i ){
	c->cd(i+1);
	TH2 *h = (TH2*)GHist::get( ++aft_hg_2d_id );
	if( !h ) continue;
	h->Draw("colz");
	// TH2 *hh = (TH2*)GHist::get( ++aft_chg_2d_id );
	// if( !hh ) continue;
	// // hh->GetXaxis()->SetRangeUser( 0, 4096 );
	// hh->SetLineColor( kRed );
	// hh->Draw("same");
	for( int i = 0; i < 4; i++ ){
	  c->cd(NumOfPlaneAFT+i+1);
	  const TString layer_name = NameOfPlaneAFT[i%4];
	  const char* title = NULL;
	  if( ud == 0 ) title = Form("HG_#U_%s_2D", layer_name.Data());
	  if( ud == 1 ) title = Form("HG_#D_%s_2D", layer_name.Data());
	  double xpos = 0.1;
	  double ypos = 0.5;
	  TLatex *text = new TLatex(xpos, ypos, title);
	  text->SetTextSize(0.12);
	  text->Draw();
	}
      }
      c->Update();
    }
  }

  // draw LowGain UorD
  {
    int aft_lg_2d_id     = HistMaker::getUniqueID( kAFT, 0, kLowGain2D, 0 );
    int aft_clg_2d_id    = HistMaker::getUniqueID( kAFT, 0, kLowGain2D, 100 );
    for( int ud=0; ud<2; ud++ ){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", ud+7));
      c->Clear();
      c->Divide(8, 5);
      for( int i=0; i<NumOfPlaneAFT; ++i ){
	c->cd(i+1);
	TH2 *h = (TH2*)GHist::get( ++aft_lg_2d_id );
	if( !h ) continue;
	h->Draw("colz");
	// TH2 *hh = (TH2*)GHist::get( ++aft_clg_2d_id );
	// if( !hh ) continue;
	// // hh->GetXaxis()->SetRangeUser( 0, 4096 );
	// hh->SetLineColor( kRed );
	// hh->Draw("same");
	for( int i = 0; i < 4; i++ ){
	  c->cd(NumOfPlaneAFT+i+1);
	  const TString layer_name = NameOfPlaneAFT[i%4];
	  const char* title = NULL;
	  if( ud == 0 ) title = Form("LG_#U_%s_2D", layer_name.Data());
	  if( ud == 1 ) title = Form("LG_#D_%s_2D", layer_name.Data());
	  double xpos = 0.1;
	  double ypos = 0.5;
	  TLatex *text = new TLatex(xpos, ypos, title);
	  text->SetTextSize(0.12);
	  text->Draw();
	}
      }
      c->Update();
    }
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
