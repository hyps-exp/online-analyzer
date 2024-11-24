// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispKtune202006()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  gStyle->SetOptStat(0);
  
  // draw BAC ADC & TDC
  TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  c->Clear();
  c->Divide(2,2);

  c->cd(1);
  //gPad->SetLogy();
  {
    TH1 *h = (TH1*)GHist::get(HistMaker::getUniqueID(kMisc, 0, kTDC)+7);
    h->GetXaxis()->SetRangeUser(0,2000);
    h->Draw();
    TH1 *hh = (TH1*)GHist::get(HistMaker::getUniqueID(kMisc, 0, kTDC)+8);
    hh->SetLineColor( kRed );
    hh->Draw("same");
    TText *t1 = new TText(0.65,0.85,"Red: w/TDC");
    t1->SetNDC(1);
    t1->SetTextColor(kRed);
    t1->SetTextSize(0.04);
    t1->Draw("same");
  }

  c->cd(2);
  {
    TH1 *h = (TH1*)GHist::get(HistMaker::getUniqueID(kBAC,  0, kTDC, 1));
    h->GetXaxis()->SetRangeUser(250e3,350e3);
    h->Draw();
  }

  c->cd(3);
  {
    TH1 *h = (TH1*)GHist::get(HistMaker::getUniqueID(kMisc,  0, kTDC)+0);
    h->GetXaxis()->SetRangeUser(-6,2);
    h->Draw();
    TLatex *t1 = new TLatex(0.72,0.86,"#pi");
    t1->SetNDC(1);
    t1->SetTextColor(kBlue);
    t1->SetTextSize(0.04);
    t1->Draw("same");
    TText *t2 = new TText(0.55,0.86,"K");
    t2->SetNDC(1);
    t2->SetTextColor(kGreen);
    t2->SetTextSize(0.04);
    t2->Draw("same");
    TText *t3 = new TText(0.20,0.86,"p-bar");
    t3->SetNDC(1);
    t3->SetTextColor(kRed);
    t3->SetTextSize(0.04);
    t3->Draw("same");
  }

  c->cd(4);
  {
    TH1 *h = (TH1*)GHist::get(HistMaker::getUniqueID(kTriggerFlag,  0, kHitPat));
    h->GetXaxis()->SetRangeUser(0,32);
    h->Draw();
    // TText *t1 = new TText(0.73,0.96,"E03 Kin[PS]: ch24");
    // t1->SetNDC(1);
    // t1->SetTextColor(kBlue);
    // t1->SetTextSize(0.04);
    // t1->Draw("same");
    // TText *t2 = new TText(0.73,0.91,"BH2[PS]: ch18");
    // t2->SetNDC(1);
    // t2->SetTextColor(kBlack);
    // t2->SetTextSize(0.04);
    // t2->Draw("same");
    // TLine *l1 = new TLine(24.5,0,24.5,1000000);
    // l1->SetLineColor(kBlue);
    // l1->SetLineStyle(2);
    // l1->Draw("same");
    // TLine *l2 = new TLine(18.5,0,18.5,1000000);
    // l2->SetLineColor(kBlack);
    // l2->SetLineStyle(2);
    // l2->Draw("same");
  }

  c->Update();

  c->cd(0);

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
