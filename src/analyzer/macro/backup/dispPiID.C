// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispPiID()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  int n_seg = 16;

  // draw HighGain 
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(8,4);
    int adc_id     = HistMaker::getUniqueID(kPiID, 0, kHighGain,1);
    int adcwtdc_id = HistMaker::getUniqueID(kPiID, 0, kADCwTDC,1);
    for( int i=0; i<NumOfSegPiID; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( adc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser(0,2000);
      h->Draw();
      TH1 *hh = (TH1*)GHist::get( adcwtdc_id + i );
      if( !hh ) continue;
      hh->GetXaxis()->SetRangeUser(0,2000);
      hh->SetLineColor( kRed );
      hh->Draw("same");
    }
    c->Update();
  }

//  {
//    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
//    c->Clear();
//    c->Divide(4,4);
//    int adc_id     = HistMaker::getUniqueID(kPiID, 0, kHighGain);
//    int adcwtdc_id = HistMaker::getUniqueID(kPiID, 0, kADCwTDC);
//    for( int i=0; i<n_seg; ++i ){
//      c->cd(i+1);
//      gPad->SetLogy();
//      TH1 *h = (TH1*)GHist::get( adc_id + i+16 );
//      if( !h ) continue;
//      h->GetXaxis()->SetRangeUser(0,2000);
//      h->Draw();
//      TH1 *hh = (TH1*)GHist::get( adcwtdc_id + i );
//      if( !hh ) continue;
//      hh->GetXaxis()->SetRangeUser(0,2000);
//      hh->SetLineColor( kRed );
//      hh->Draw("same");
//    }
//    c->Update();
//  }

  // draw LowGain 
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(8,4);
    int adc_id     = HistMaker::getUniqueID(kPiID, 0, kLowGain,1);
    int adcwtdc_id = HistMaker::getUniqueID(kPiID, 0, kADCwTDC,NumOfSegPiID+1);
    for( int i=0; i<NumOfSegPiID; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( adc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser(0,2000);
      h->Draw();
      TH1 *hh = (TH1*)GHist::get( adcwtdc_id + i );
      if( !hh ) continue;
      hh->GetXaxis()->SetRangeUser(0,2000);
      hh->SetLineColor( kRed );
      hh->Draw("same");
    }
    c->Update();
  }

//  {
//    TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
//    c->Clear();
//    c->Divide(4,4);
//    int adc_id     = HistMaker::getUniqueID(kPiID, 0, kLowGain);
//    int adcwtdc_id = HistMaker::getUniqueID(kPiID, 0, kADCwTDC+NumOfSegPiID);
//    for( int i=0; i<n_seg; ++i ){
//      c->cd(i+1);
//      gPad->SetLogy();
//      TH1 *h = (TH1*)GHist::get( adc_id + i+16 );
//      if( !h ) continue;
//      h->GetXaxis()->SetRangeUser(0,2000);
//      h->Draw();
//      TH1 *hh = (TH1*)GHist::get( adcwtdc_id + i );
//      if( !hh ) continue;
//      hh->GetXaxis()->SetRangeUser(0,2000);
//      hh->SetLineColor( kRed );
//      hh->Draw("same");
//    }
//    c->Update();
//  }

  // draw TDC 
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(8,4);
    int tdc_id = HistMaker::getUniqueID(kPiID, 0, kTDC,1);
    for( int i=0; i<NumOfSegPiID; ++i ){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( tdc_id + i );
      if( !h ) continue;
      h->Draw();
    }
    c->Update();
  }

//  {
//    TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
//    c->Clear();
//    c->Divide(4,4);
//    int tdc_id = HistMaker::getUniqueID(kPiID, 0, kTDC);
//    for( int i=0; i<n_seg; ++i ){
//      c->cd(i+1);
//      gPad->SetLogy();
//      TH1 *h = (TH1*)GHist::get( tdc_id + i + 16 );
//      if( !h ) continue;
//      h->Draw();
//    }
//    c->Update();
//  }

  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
    c->Clear();
    c->Divide(2,2);
    int hitpat_id = HistMaker::getUniqueID(kPiID, 0, kHitPat);
    int multi_id = HistMaker::getUniqueID(kPiID, 0, kMulti);
    for( int i=0; i<4; ++i ){
     if(i <= 1){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( hitpat_id + i);
      if( !h ) continue;
      h->Draw();
     }
     if(i > 1){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( multi_id + i -2);
      if( !h ) continue;
      h->Draw();
     }
    }
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
