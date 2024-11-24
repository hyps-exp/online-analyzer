// -*- C++ -*-

using hddaq::gui::Updater;

//_____________________________________________________________________________
void
dispBH2Fit( void )
{
  if( Updater::isUpdating() )
    return;
  Updater::setUpdating( true );

  static const auto& gUser = UserParamMan::GetInstance();
  static const auto tdc_min = gUser.GetParameter( "TdcBH2", 0 );
  static const auto tdc_max = gUser.GetParameter( "TdcBH2", 1 );

  {
    auto c1 = dynamic_cast<TCanvas*>( gROOT->FindObject("c1") );
    if( !c1 )
      return;
    c1->Clear();
    c1->Divide( 3, 2 );
    const Int_t n = 5; // NumOfSegBH2;
    Int_t icanvas1 = 0;
    for( Int_t seg=0; seg<n; ++seg ){
      TF1 fit = TF1( "fit", "gaus" );
      fit.SetLineColor( kRed );
      c1->cd( ++icanvas1 );
      // gPad->SetLogy();
      auto h1  = GHist::get( HistMaker::getUniqueID(kBH2, 0, kTDC, seg+1) );
      auto h2 = h1; //->Clone();
      if( h2 ){
	h2->GetXaxis()->SetRangeUser( tdc_min, tdc_max );
        // Double_t max  = h1->GetMaximumBin();
        Double_t max  =  h2->GetXaxis()->GetBinCenter( h1->GetMaximumBin() );
	Double_t range = 100.;
	// h1->Fit("fit","IQ","",tdc_min,tdc_max);
	h2->Fit( "fit", "IQ", "", max-range, max+range );
	h2->Draw();
        Double_t mean = fit.GetParameter(1);
        // Double_t xpos  = h1->GetXaxis()->GetBinCenter( h1->GetMaximumBin() );
        Double_t xpos  = tdc_min+1000;
        Double_t ypos  = h1->GetMaximum()*0.7;
        auto text = new TLatex( xpos, ypos, Form( "Mean. %.2f", mean ) );
        text->SetTextSize( 0.08 );
        text->Draw( "same" );
      }
    }
    c1->Update();
    c1->cd(0);
  }

  Updater::setUpdating( false );
}
