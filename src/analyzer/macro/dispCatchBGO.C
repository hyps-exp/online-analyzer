// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispCatchBGO()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // draw FADC  1-12
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(4,3);
    Int_t id = HistMaker::getUniqueID(kBGO, 0, kFADC);
    for( Int_t i=0; i<NumOfSegBGO/2; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( 80, 200 );
      h->Draw("colz");
      gPad->SetLogz();
    }
    c->Update();
  }

  // draw FADC 13-24
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(4,3);
    Int_t id = HistMaker::getUniqueID(kBGO, 0, kFADC, 13);
    for( Int_t i=0; i<NumOfSegBGO/2; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( 80, 200 );
      h->Draw("colz");
      gPad->SetLogz();
    }
    c->Update();
  }

  // draw ADC  1-12
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(4,3);
    Int_t id = HistMaker::getUniqueID(kBGO, 0, kADC);
    for( Int_t i=0; i<NumOfSegBGO/2; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( id + i );
      if( !h ) continue;
      h->Draw();
    }
    id = HistMaker::getUniqueID(kBGO, 0, kADCwTDC);
    for( Int_t i=0; i<NumOfSegBGO/2; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( id + i );
      if( !h ) continue;
      h->SetLineColor(kRed);
      h->Draw("same");
    }
    c->Update();
  }

  // draw ADC 13-24
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
    c->Clear();
    c->Divide(4,3);
    Int_t id = HistMaker::getUniqueID(kBGO, 0, kADC, 13);
    for( Int_t i=0; i<NumOfSegBGO/2; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( id + i );
      if( !h ) continue;
      h->Draw("colz");
    }
    id = HistMaker::getUniqueID(kBGO, 0, kADCwTDC, 13);
    for( Int_t i=0; i<NumOfSegBGO/2; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( id + i );
      if( !h ) continue;
      h->SetLineColor(kRed);
      h->Draw("same");
    }
    c->Update();
  }

  // draw TDC  1-12
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
    c->Clear();
    c->Divide(4,3);
    Int_t tdc_id     = HistMaker::getUniqueID(kBGO, 0, kTDC);
    for( Int_t i=0; i<NumOfSegBGO/2; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( tdc_id + i );
      if( !h ) continue;
      h->Draw();
    }
    c->Update();
  }

  // draw TDC 13-24
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c6");
    c->Clear();
    c->Divide(4,3);
    Int_t tdc_id     = HistMaker::getUniqueID(kBGO, 0, kTDC, 13);
    for( Int_t i=0; i<NumOfSegBGO/2; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( tdc_id + i );
      if( !h ) continue;
      h->Draw();
    }
    c->Update();
  }

  // draw 2D
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c7");
    c->Clear();
    c->Divide( 2, 2 );
    Int_t id = HistMaker::getUniqueID(kBGO, 0, kADC2D);
    for( Int_t i=0; i<3; ++i ){
      c->cd( i + 1 );
      TH1 *h = (TH1*)GHist::get( id + i );
      if( !h ) continue;
      h->Draw("colz");
    }
    id = HistMaker::getUniqueID(kBGO, 0, kTDC2D);
    TH1 *h = (TH1*)GHist::get( id );
    c->cd(4);
    if( h )
      h->Draw("colz");
    c->Update();
  }

  // draw clk
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c8");
    c->Clear();
    c->Divide(4,2);
    Int_t clk_id = HistMaker::getUniqueID(kBGO, 0, kTDC, 25);
    for( Int_t i=0; i<NumOfSegBGO_T; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( clk_id + i );
      if( !h ) continue;
      h->Draw();
    }
    Int_t h_id     = HistMaker::getUniqueID(kBGO, 0, kHitPat, 1);
    TH1 *h1 = (TH1*)GHist::get( h_id );
    c->cd(5);
    h1->Draw();

    h_id     = HistMaker::getUniqueID(kBGO, 0, kHitPat, 2);
    TH1 *h2 = (TH1*)GHist::get( h_id );
    c->cd(6);
    h2->Draw();

    h_id     = HistMaker::getUniqueID(kBGO, 0, kMulti, 1);
    TH1 *h3 = (TH1*)GHist::get( h_id );
    c->cd(7);
    h3->Draw();

    h_id     = HistMaker::getUniqueID(kBGO, 0, kMulti, 2);
    TH1 *h4 = (TH1*)GHist::get( h_id );
    c->cd(8);
    h4->Draw();

    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
