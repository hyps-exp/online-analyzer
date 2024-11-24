// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispACs_E07()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const int n_hist = 4; // bac1, bac2, pvac, fac
  int tdcmax = 350e3, tdcmin = 300e3, adcmmax = 0, adcmax = 4000;
  
  int adc_id[n_hist*3] = {
    HistMaker::getUniqueID(kBAC,      0, kADC, 1),// BAC1
    HistMaker::getUniqueID(kBAC,      0, kADC, 2),// BAC2
    HistMaker::getUniqueID(kPVAC,     0, kADC, 1),// PVAC
    HistMaker::getUniqueID(kFAC,      0, kADC, 1),// FAC
    HistMaker::getUniqueID(kBAC,      0, kADC, 3),// BAC1_pi
    HistMaker::getUniqueID(kBAC,      0, kADC, 4),// BAC2_pi
    HistMaker::getUniqueID(kPVAC,     0, kADC, 2),// PVAC_pi
    HistMaker::getUniqueID(kFAC,      0, kADC, 2),// FAC_pi
    HistMaker::getUniqueID(kBAC,      0, kADC, 5),// BAC1_k
    HistMaker::getUniqueID(kBAC,      0, kADC, 6),// BAC2_k
    HistMaker::getUniqueID(kPVAC,     0, kADC, 3),// PVAC_k
    HistMaker::getUniqueID(kFAC,      0, kADC, 3),// FAC_k
  };

  int tdc_id[n_hist] = {
    HistMaker::getUniqueID(kBAC,      0, kTDC, 1),// BAC1
    HistMaker::getUniqueID(kBAC,      0, kTDC, 2),// BAC2
    HistMaker::getUniqueID(kPVAC,     0, kTDC, 1),// PVAC
    HistMaker::getUniqueID(kFAC,      0, kTDC, 1),// FAC
  };

  int adcwtdc_id[n_hist] = {
    HistMaker::getUniqueID(kBAC,      0, kADCwTDC, 1),// BAC1
    HistMaker::getUniqueID(kBAC,      0, kADCwTDC, 2),// BAC2
    HistMaker::getUniqueID(kPVAC,     0, kADCwTDC, 1),// PVAC
    HistMaker::getUniqueID(kFAC,      0, kADCwTDC, 1),// FAC
  };

  int eff_id[n_hist*2] = {
    HistMaker::getUniqueID(kBAC,      0, kHitPat, 2),// BAC1_pi
    HistMaker::getUniqueID(kBAC,      0, kHitPat, 3),// BAC2_pi
    HistMaker::getUniqueID(kPVAC,     0, kHitPat, 2),// PVAC_pi
    HistMaker::getUniqueID(kFAC,      0, kHitPat, 2),// FAC_pi
    HistMaker::getUniqueID(kBAC,      0, kHitPat, 4),// BAC1_k
    HistMaker::getUniqueID(kBAC,      0, kHitPat, 5),// BAC2_k
    HistMaker::getUniqueID(kPVAC,     0, kHitPat, 3),// PVAC_k
    HistMaker::getUniqueID(kFAC,      0, kHitPat, 3),// FAC_k
  };

  int btof_id[3] = {
    HistMaker::getUniqueID(kMisc,     0, kTDC,    1);
    HistMaker::getUniqueID(kMisc,     0, kTDC,    2);
    HistMaker::getUniqueID(kMisc,     0, kTDC,    3);
  }

  { 
    for(int i = 0; i<n_hist; ++i){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d",i+1));
      c->Clear();
      c->Divide(3,2);
      c->cd(1);
      TH1 *h1 = (TH1*)GHist::get(adc_id[i]);
      h1->GetXaxis()->SetRangeUser(adcmin,adcmax);
      h1->Draw();
      TH1 *hh1 = (TH1*)GHist::get(adcwtdc_id[i]);
      hh1->GetXaxis()->SetRangeUser(adcmin,adcmax);
      hh1->SetLineColor( kRed );
      hh1->Draw("same");
      Double_t xpos  = adcmax*0.8;
      Double_t ypos  = h1->GetMaximum()*0.9;
      auto text = new TLatex( xpos, ypos, "Red: wTDC" );
      text->SetTextSize( 0.08 );
      text->SetTextColor( kRed );
      text->Draw( "same" );

      c->cd(2);
      TH1 *h2 = (TH1*)GHist::get(adc_id[i]);
      h2->GetXaxis()->SetRangeUser(adcmin,adcmax);
      h2->Draw();
      TH1 *hh2 = (TH1*)GHist::get(adc_id[i+n_hist]);
      hh2->GetXaxis()->SetRangeUser(adcmin,adcmax);
      hh2->SetLineColor( kRed );
      hh2->Draw("same");
      xpos  = adcmax*0.8;
      ypos  = h2->GetMaximum()*0.9;
      text = new TLatex( xpos, ypos, "Red:Pi" );
      text->SetTextSize( 0.08 );
      text->SetTextColor( kRed );
      text->Draw( "same" );
      TH1 *hhh2 = (TH1*)GHist::get(adc_id[i+n_hist*2]);
      hhh2->GetXaxis()->SetRangeUser(adcmin,adcmax);
      hhh2->SetLineColor( kGreen );
      hhh2->Draw("same");
      xpos  = adcmax*0.7;
      ypos  = h2->GetMaximum()*0.8;
      text = new TLatex( xpos, ypos, "Green:K" );
      text->SetTextSize( 0.08 );
      text->SetTextColor( kGreen );
      text->Draw( "same" );

      c->cd(3);
      TH1 *h3 = (TH1*)GHist::get(tdc_id[i]);
      h3->GetXaxis()->SetRangeUser(tdcmin,tdcmax);
      h3->Draw();

      c->cd(4);
      TH1 *h4 = (TH1*)GHist::get(btof_id[1]);
      h4->GetXaxis()->SetRangeUser(-3,3);
      h4->Draw();
      TH1 *hh4 = (TH1*)GHist::get(btof_id[2]);
      hh4->GetXaxis()->SetRangeUser(-3,3);
      hh4->SetLineColor( kRed );
      hh4->Draw("same");
      xpos  = 2;
      ypos  = hh4->GetMaximum()*0.9;
      text = new TLatex( xpos, ypos, "Red:Pi" );
      text->SetTextSize( 0.08 );
      text->SetTextColor( kRed );
      text->Draw( "same" );
      TH1 *hhh4 = (TH1*)GHist::get(btof_id[3]);
      hhh4->GetXaxis()->SetRangeUser(-3,3);
      hhh4->SetLineColor( kGreen );
      hhh4->Draw("same");
      xpos  = -1;
      ypos  = hhh4->GetMaximum()*0.9;
      text = new TLatex( xpos, ypos, "Green:K" );
      text->SetTextSize( 0.08 );
      text->SetTextColor( kBrack );
      text->Draw( "same" );

      c->cd(5);
      TH1 *h5 = (TH1*)GHist::get(eff_id[i]);
      h5->GetXaxis()->SetRangeUser(0,2);
      h5->Draw();
      int a = h5->GetBinContent(0);
      int b = h5->GetBinContent(1);
      double eff = b/a ;
      xpos  = 1;
      ypos  = h5->GetMaximum()*0.8;
      text = new TLatex( xpos, ypos, Form("Pion eff.=%d",eff) );
      text->SetTextSize( 0.08 );
      text->SetTextColor( kBrack );
      text->Draw( "same" );

      c->cd(6);
      TH1 *h6 = (TH1*)GHist::get(eff_id[i+n_hist*2]);
      h6->GetXaxis()->SetRangeUser(0,2);
      h6->Draw();
      a = h6->GetBinContent(0);
      b = h6->GetBinContent(1);
      eff = b/a ;
      xpos  = 1;
      ypos  = h6->GetMaximum()*0.8;
      text = new TLatex( xpos, ypos, Form("Kaon eff.=%d",eff) );
      text->SetTextSize( 0.08 );
      text->SetTextColor( kGreen );
      text->Draw( "same" );
    }
  }
  c->Update();

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
