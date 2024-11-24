// -*- C++ -*-

#include "DetectorID.hh"
#include "UserParamMan.hh"

using namespace hddaq::gui; // for Updater

//_____________________________________________________________________________
void
fitWC( void )
{
  if( Updater::isUpdating() )
    return;
  Updater::setUpdating( true );

  const auto& gUser = UserParamMan::GetInstance();
  TLatex tex;
  tex.SetNDC();
  tex.SetTextSize(0.14);
  Double_t xpos = 0.15, ypos = 0.75;

  auto c1 = dynamic_cast<TCanvas*>( gROOT->FindObject("c1") );
  c1->Clear();
  c1->Divide( 3, 1 );
  Int_t icanvas = 0;
  Int_t id = 0;
  TH1* h1 = nullptr;
  TF1 *f = new TF1("f", "gaus");
  f->SetLineColor(kBlue);
  for( Int_t i=0; i<NumOfSegWC; ++i ){
    for( Int_t ud=0; ud<2; ++ud ){
      if( i == NumOfSegWC-1 && ud == 1 )
	continue;
      // ADC w TDC
      //c1->cd( ++icanvas )->SetLogy();
      c1->cd( ++icanvas );

      id = HistMaker::getUniqueID( kWC, 0, kADCwTDC, i+ud*NumOfSegWC+1 );
      h1 = dynamic_cast<TH1*>( GHist::get( id )->Clone());
      if( h1 ){
	h1->GetXaxis()->SetRangeUser( 0, 0x1000 );
	Double_t rms = h1->GetRMS();
	Double_t max = h1->GetBinCenter(h1->GetMaximumBin());
	// if(max>200.)
	//   h1->Fit("f", "Q", "", max - 3.*rms, max + 3.*rms);
	// else
	//   h1->Fit("f", "L", "", 0., 2000.);

	h1->Fit("f", "L", "", 100., 1000.);

	Double_t mean = f->GetParameter("Mean");
	Double_t sigma = f->GetParameter("Sigma");
	h1->Draw( );
	tex.DrawLatex(xpos, ypos, Form("%.2lf, %.2lf", mean, sigma));
      }
    }
  }

  c1->Update();
  c1->cd(0);

  Updater::setUpdating( false );
}
