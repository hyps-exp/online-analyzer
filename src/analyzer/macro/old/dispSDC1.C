// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispSDC1( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  gStyle->SetOptStat(1111110);
  int n_layer = 6;

  // draw TDC
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(3,2);
    int base_id = HistMaker::getUniqueID(kSDC1, 0, kTDC);
    int base_id_ctot = HistMaker::getUniqueID(kSDC1, 0, kTDC, 1+kTOTcutOffset);
    for( int i=0; i<n_layer; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get(base_id + i);
      h->Draw();
      TH1 *hh = (TH1*)GHist::get( base_id_ctot + i );
      if( !hh ) continue;
      hh->SetLineColor( kRed );
      hh->Draw("same");
    }
    c->Update();
  }

  // draw TDC1st
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(3,2);
    int base_id = HistMaker::getUniqueID(kSDC1, 0, kTDC2D);
    int base_id_ctot = HistMaker::getUniqueID(kSDC1, 0, kTDC2D, 1+kTOTcutOffset);
    for( int i=0; i<n_layer; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get(base_id + i);;
      if( !h ) continue;
      h->Draw();
      TH1 *hh = (TH1*)GHist::get( base_id_ctot + i );
      if( !hh ) continue;
      hh->SetLineColor( kRed );
      hh->Draw("same");
    }
    c->Update();
  }

  // draw TOT
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(3,2);
    int base_id = HistMaker::getUniqueID(kSDC1, 0, kADC);
    int base_id_ctot = HistMaker::getUniqueID(kSDC1, 0, kADC, 1+kTOTcutOffset);
    for( int i=0; i<n_layer; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get(base_id + i);;
      if( !h ) continue;
      h->Draw();
      TH1 *hh = (TH1*)GHist::get( base_id_ctot + i );
      if( !hh ) continue;
      hh->SetLineColor( kRed );
      hh->Draw("same");
      TF1 f("f", "gaus", 0., 100.);
      f.SetLineColor(kBlue);
      Double_t p = hh->GetBinCenter(hh->GetMaximumBin());
      // if(p < 30.) p = 70.;
      Double_t w = 10.;
      for(Int_t ifit=0; ifit<3; ++ifit){
	Double_t fmin = p - w;
	Double_t fmax = p + w;
	h->Fit("f", "Q", "", fmin, fmax);
	p = f.GetParameter(1);
	w = f.GetParameter(2) * 1.;
      }
      f.Draw("same");
      TLatex *text = new TLatex();
      text->SetNDC();
      text->SetTextSize(0.07);
      text->DrawLatex(0.400, 0.500, Form("%.1f", p));
    }
    c->Update();
  }

  // draw HitPat
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
    c->Clear();
    c->Divide(3,2);
    int base_id = HistMaker::getUniqueID(kSDC1, 0, kHitPat);
    int base_id_ctot = HistMaker::getUniqueID(kSDC1, 0, kHitPat, 1+kTOTcutOffset);
    for( int i=0; i<n_layer; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get(base_id + i);
      if( !h ) continue;
      h->Draw();
      TH1 *hh = (TH1*)GHist::get(base_id_ctot + i);
      if( !hh ) continue;
      hh->SetLineColor( kRed );
      hh->Draw("same");
    }
    c->Update();
  }

  // draw Multi
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
    c->Clear();
    c->Divide(3,2);
    int base_id = HistMaker::getUniqueID(kSDC1, 0, kMulti);
    for( int i=0; i<n_layer; ++i ){
      c->cd(i+1);
      TH1 *h_wot = (TH1*)GHist::get(base_id + i);//->Clone();
//      TH1 *h_wt  = (TH1*)GHist::get(base_id + i + n_layer);//->Clone();//TDCcut
      TH1 *h_wt  = (TH1*)GHist::get(base_id + i + n_layer + kTOTcutOffset);//->Clone();//TDC & TOTcut
      if( !h_wot || !h_wt ) continue;
      h_wt->SetLineColor(2);
//      h_wt->Draw();
//      h_wot->Draw("same");
      h_wt->Draw();
      h_wot->Draw("same");
//      h_wot->SetMaximum(h_wt->GetMaximum()*1.1);
    }
    c->Update();
  }

  // draw TDC2D
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c6");
    c->Clear();
    c->Divide(3,2);
    int base_id = HistMaker::getUniqueID(kSDC1, 0, kTDC2D, 21+kTOTcutOffset);
    //    int base_id_ctot = HistMaker::getUniqueID(kSDC1, 0, kTDC2D, 1+kTOTcutOffset);
    for( int i=0; i<n_layer; ++i ){
      c->cd(i+1);
      TH2 *h = (TH2*)GHist::get(base_id + i);;
      if( !h ) continue;
      h->SetStats(0);
      h->GetYaxis()->SetRangeUser(0,1500);
      h->Draw("colz");
    }
    c->Update();
  }

  // draw TDC2D wTOTCUT
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c7");
    c->Clear();
    c->Divide(3,2);
    //    int base_id = HistMaker::getUniqueID(kSDC1, 0, kTDC2D, 21+kTOTcutOffset);
    int base_id_ctot = HistMaker::getUniqueID(kSDC1, 0, kTDC2D, 31+kTOTcutOffset);
    for( int i=0; i<n_layer; ++i ){
      c->cd(i+1);
      TH2 *h = (TH2*)GHist::get(base_id_ctot + i);;
      if( !h ) continue;
      h->SetStats(0);
      h->GetYaxis()->SetRangeUser(0,1500);
      h->Draw("colz");
    }
    c->Update();
  }

  // draw TDC
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c8");
    c->Clear();
    c->Divide(3,2);
    int base_id = HistMaker::getUniqueID(kSDC1, 0, kTDC, 11);
    //    int base_id_ctot = HistMaker::getUniqueID(kSDC1, 0, kTDC, 1+kTOTcutOffset);
    for( int i=0; i<n_layer; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get(base_id + i);
      h->Draw();
    }
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
