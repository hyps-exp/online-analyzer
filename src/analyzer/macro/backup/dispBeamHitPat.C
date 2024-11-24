// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispBeamHitPat()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // Return to the initial position
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
    c->cd();
  }

{
  TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  c->Clear();
  c->Divide(3,2);

  TH1* h = NULL;

  // beam
  c->cd(1);
  int base_id = HistMaker::getUniqueID(kBH1, 0, kHitPat);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();

  c->cd(2);
  base_id = HistMaker::getUniqueID(kBFT, 0, kHitPat, 1);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();

  c->cd(3);
  base_id = HistMaker::getUniqueID(kBH2, 0, kHitPat);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();
  c->Update();


  TH1* hh = NULL;


  base_id = HistMaker::getUniqueID(kBC3, 0, kHitPat);
  int base_id_ctot = HistMaker::getUniqueID(kBC3, 0, kHitPat, 1+kTOTcutOffset);
  c->cd(4);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();
  hh = (TH1*)GHist::get(base_id_ctot);
  hh->SetMinimum(0);
  hh->SetLineColor( kRed );
  hh->Draw("same");


  base_id = HistMaker::getUniqueID(kBC4, 0, kHitPat);
  base_id_ctot = HistMaker::getUniqueID(kBC4, 0, kHitPat, 1+kTOTcutOffset);
  c->cd(5);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();
  hh = (TH1*)GHist::get(base_id_ctot);
  hh->SetMinimum(0);
  hh->SetLineColor( kRed );
  hh->Draw("same");

  c->cd(6);
  base_id = HistMaker::getUniqueID(kBAC, 0, kMulti);
  {
    TH1 *h_wt = GHist::get(base_id);
    h_wt->Draw();
    double Nof0     = h_wt->GetBinContent(1);
    double NofTotal = h_wt->GetEntries();
    double eff      = 1. - (double)Nof0/NofTotal;

    double xpos     = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
    double ypos     = h_wt->GetMaximum()*0.8;
    TLatex *text    = new TLatex(xpos, ypos, Form("eff. %.4f", eff));
    text->SetTextSize(0.08);
    text->Draw();
  }
  c->Update();

  c->cd(0);
}
  gROOT->SetStyle("Classic");

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
