// -*- C++ -*-

using hddaq::gui::Updater;

//_____________________________________________________________________________
void
dispMtx2D( void )
{
  if( Updater::isUpdating() )
    return;
  Updater::setUpdating( true );

  const auto& gMatrix = MatrixParamMan::GetInstance();

  for( Int_t i=0; i<2; ++i ){
    auto c = dynamic_cast<TCanvas*>( gROOT->FindObject( Form( "c%d", i+1 ) ) );
    if( !c ) continue;
    c->Clear();
    c->cd(0);
    Int_t mtx2d_id  = HistMaker::getUniqueID( kMisc, kHul2D, kHitPat2D );
    Int_t tofsch_id = HistMaker::getUniqueID( kCorrelation, 0, 0, 3 );
    auto hmtx2d  = dynamic_cast<TH2*>( GHist::get( mtx2d_id + i ) );
    auto htofsch = dynamic_cast<TH2*>( GHist::get( tofsch_id ) );
    if( !hmtx2d ) continue;
    if( !htofsch ) continue;
    hmtx2d->Reset();
    hmtx2d->SetLineWidth( 1 );
    hmtx2d->SetLineColor( kRed );
    for( Int_t i_tof=0; i_tof<NumOfSegTOF; ++i_tof ){
      for( Int_t i_sch=0; i_sch<NumOfSegSCH; ++i_sch ){
	Bool_t hul2d_flag = gMatrix.IsAccept( i_tof, i_sch );
	if( !hul2d_flag ) hmtx2d->Fill( i_sch, i_tof );
      } // for(i_sch)
    } // for(i_tof)

    hmtx2d->GetXaxis()->SetRangeUser(0, NumOfSegSCH);
    hmtx2d->GetYaxis()->SetRangeUser(0, NumOfSegTOF);
    hmtx2d->SetStats( 0 );
    hmtx2d->Draw( "box" );

    htofsch->GetXaxis()->SetRangeUser(0, NumOfSegSCH);
    htofsch->GetYaxis()->SetRangeUser(0, NumOfSegTOF);
    htofsch->Draw( "same colz" );

    gPad->SetGridx();
    gPad->SetGridy();

    c->Update();
  }

  Updater::setUpdating( false );
}
