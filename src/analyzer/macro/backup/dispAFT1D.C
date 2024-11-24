// -*- C++ -*-

#include "DetectorID.hh"
#include "UserParamMan.hh"

using namespace hddaq::gui; // for Updater

//_____________________________________________________________________________
void
dispAFT1D( void )
{
  //  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // draw TDC UorD
  {
    int aft_t_id     = HistMaker::getUniqueID( kAFT, 0, kTDC, 0 );
    for( int ud=0; ud<2; ud++ ){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", ud+1));
      c->Clear();
      c->Divide(8, 5);
      for( int i=0; i<NumOfPlaneAFT; ++i ){
	c->cd(i+1);
	TH1 *h = (TH1*)GHist::get( ++aft_t_id );
	if( !h ) continue;
	h->Draw();
      }
      for( int i = 0; i < 4; i++ ){
	c->cd(NumOfPlaneAFT+i+1);
	const TString layer_name = NameOfPlaneAFT[i%4];
	const char* title = NULL;
	if( ud == 0 ) title = Form("TDC_#U_%s", layer_name.Data());
	if( ud == 1 ) title = Form("TDC_#D_%s", layer_name.Data());
	double xpos = 0.1;
	double ypos = 0.5;
	TLatex *text = new TLatex(xpos, ypos, title);
	text->SetTextSize(0.15);
	text->Draw();
      }
      c->Update();
    }
  }

  // draw TOT UorD
  {
    int aft_tot_id     = HistMaker::getUniqueID( kAFT, 0, kTOT, 0 );
    int aft_ctot_id    = HistMaker::getUniqueID( kAFT, 0, kTOT, 100 );
    for( int ud=0; ud<2; ud++ ){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", ud+3));
      c->Clear();
      c->Divide(8, 5);
      for( int i=0; i<NumOfPlaneAFT; ++i ){
	c->cd(i+1);
	TH1 *h = (TH1*)GHist::get( ++aft_tot_id );
	if( !h ) continue;
	h->SetTitleSize(0.05, "T");
	h->Draw();
	TH1 *hh = (TH1*)GHist::get( ++aft_ctot_id );
	if( !hh ) continue;
	// hh->GetXaxis()->SetRangeUser( 0, 4096 );
	hh->SetLineColor( kRed );
	hh->Draw("same");
      }
      for( int i = 0; i < 4; i++ ){
	c->cd(NumOfPlaneAFT+i+1);
	const TString layer_name = NameOfPlaneAFT[i%4];
	const char* title = NULL;
	if( ud == 0 ) title = Form("TOT_#U_%s", layer_name.Data());
	if( ud == 1 ) title = Form("TOT_#D_%s", layer_name.Data());
	double xpos = 0.1;
	double ypos = 0.5;
	TLatex *text = new TLatex(xpos, ypos, title);
	text->SetTextSize(0.15);
	text->Draw();
      }
      c->Update();
    }
  }

  // draw HighGain UorD
  {
    int aft_hg_id     = HistMaker::getUniqueID( kAFT, 0, kHighGain, 0 );
    int aft_chg_id    = HistMaker::getUniqueID( kAFT, 0, kHighGain, 100 );
    for( int ud=0; ud<2; ud++ ){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", ud+5));
      c->Clear();
      c->Divide(8, 5);
      for( int i=0; i<NumOfPlaneAFT; ++i ){
	c->cd(i+1);
	gPad->SetLogy();
	TH1 *h = (TH1*)GHist::get( ++aft_hg_id );
	if( !h ) continue;
	h->Rebin(4);
	h->Draw();
	TH1 *hh = (TH1*)GHist::get( ++aft_chg_id );
	if( !hh ) continue;
	hh->Rebin(4);
	hh->SetLineColor( kRed );
	hh->Draw("same");
	for( int i = 0; i < 4; i++ ){
	  c->cd(NumOfPlaneAFT+i+1);
	  const TString layer_name = NameOfPlaneAFT[i%4];
	  const char* title = NULL;
	  if( ud == 0 ) title = Form("HG_#U_%s", layer_name.Data());
	  if( ud == 1 ) title = Form("HG_#D_%s", layer_name.Data());
	  double xpos = 0.1;
	  double ypos = 0.5;
	  TLatex *text = new TLatex(xpos, ypos, title);
	  text->SetTextSize(0.15);
	  text->Draw();
	}
      }
      c->Update();
    }
  }

  // draw LowGain UorD
  {
    int aft_lg_id     = HistMaker::getUniqueID( kAFT, 0, kLowGain, 0 );
    int aft_clg_id    = HistMaker::getUniqueID( kAFT, 0, kLowGain, 100 );
    for( int ud=0; ud<2; ud++ ){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", ud+7));
      c->Clear();
      c->Divide(8, 5);
      for( int i=0; i<NumOfPlaneAFT; ++i ){
	c->cd(i+1);
	gPad->SetLogy();
	TH1 *h = (TH1*)GHist::get( ++aft_lg_id );
	if( !h ) continue;
	h->Rebin(4);
	h->Draw();
	TH1 *hh = (TH1*)GHist::get( ++aft_clg_id );
	if( !hh ) continue;
	hh->Rebin(4);
	hh->SetLineColor( kRed );
	hh->Draw("same");
	for( int i = 0; i < 4; i++ ){
	  c->cd(NumOfPlaneAFT+i+1);
	  const TString layer_name = NameOfPlaneAFT[i%4];
	  const char* title = NULL;
	  if( ud == 0 ) title = Form("LG_#U_%s", layer_name.Data());
	  if( ud == 1 ) title = Form("LG_#D_%s", layer_name.Data());
	  double xpos = 0.1;
	  double ypos = 0.5;
	  TLatex *text = new TLatex(xpos, ypos, title);
	  text->SetTextSize(0.15);
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
