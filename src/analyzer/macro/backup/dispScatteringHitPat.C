// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispScatteringHitPat()
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
  TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
  c->Clear();
  c->Divide(3,3);

  TH1* h = NULL;
  TH1* hh = NULL;

  int base_id = HistMaker::getUniqueID(kSDC1, 0, kHitPat);
  int base_id_ctot = HistMaker::getUniqueID(kSDC1, 0, kHitPat, 1+kTOTcutOffset);
  c->cd(1);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();
  hh = (TH1*)GHist::get(base_id_ctot);
  hh->SetMinimum(0);
  hh->SetLineColor( kRed );
  hh->Draw("same");

  base_id = HistMaker::getUniqueID(kSDC2, 0, kHitPat);
  base_id_ctot = HistMaker::getUniqueID(kSDC2, 0, kHitPat, 1+kTOTcutOffset);
  c->cd(2);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();
  hh = (TH1*)GHist::get(base_id_ctot);
  hh->SetMinimum(0);
  hh->SetLineColor( kRed );
  hh->Draw("same");

  base_id = HistMaker::getUniqueID(kSDC3, 0, kHitPat);
  base_id_ctot = HistMaker::getUniqueID(kSDC3, 0, kHitPat, 11);
  c->cd(3);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();
  hh = (TH1*)GHist::get(base_id_ctot);
  hh->SetMinimum(0);
  hh->SetLineColor( kRed );
  hh->Draw("same");

  base_id = HistMaker::getUniqueID(kSDC4, 0, kHitPat);
  base_id_ctot = HistMaker::getUniqueID(kSDC4, 0, kHitPat, 11);
  c->cd(4);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();
  hh = (TH1*)GHist::get(base_id_ctot);
  hh->SetMinimum(0);
  hh->SetLineColor( kRed );
  hh->Draw("same");


  base_id = HistMaker::getUniqueID(kSDC5, 0, kHitPat);
  base_id_ctot = HistMaker::getUniqueID(kSDC5, 0, kHitPat, 11);
  c->cd(5);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();
  hh = (TH1*)GHist::get(base_id_ctot);
  hh->SetMinimum(0);
  hh->SetLineColor( kRed );
  hh->Draw("same");

  c->cd(6);
  base_id = HistMaker::getUniqueID(kTOF, 0, kHitPat);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();

  c->cd(7);
  base_id = HistMaker::getUniqueID(kAC1, 0, kHitPat);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();

  c->cd(8);
  base_id = HistMaker::getUniqueID(kWC, 0, kHitPat);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();

  c->cd(9);
  base_id = HistMaker::getUniqueID(kSFV, 0, kHitPat);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();


  c->Update();

  c->cd(0);
}

  gROOT->SetStyle("Classic");

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
