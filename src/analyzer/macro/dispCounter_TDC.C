// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispCounter_TDC( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  gStyle->SetOptStat(1111110);

  // draw RF & TAG
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(2,2);
    int base_id_1 = HistMaker::getUniqueID(kRF, 0, kTDC, 0);
    int base_id_2 = HistMaker::getUniqueID(kTAG_SF, 0, kHitPat, 0);
    int base_id_3 = HistMaker::getUniqueID(kTAG_SF, 0, kHitPat, 0) + 1;
    int base_id_4 = HistMaker::getUniqueID(kTAG_PL, 0, kHitPat, 0);
    c->cd(1);
    TH1 *h1 = (TH1*)GHist::get(base_id_1);
    h1->Draw();
    c->cd(2);
    TH1 *h2 = (TH1*)GHist::get(base_id_2);
    h2->Draw();
    c->cd(3);
    TH1 *h3 = (TH1*)GHist::get(base_id_3);
    h3->Draw();
    c->cd(4);
    TH1 *h4 = (TH1*)GHist::get(base_id_4);
    h4->Draw();
    c->Update();
  }

  // draw Veto Counter
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(2,2);
    int base_id_1 = HistMaker::getUniqueID(kU_Veto, 0, kTDC, 0);
    int base_id_2 = HistMaker::getUniqueID(kSAC, 0, kTDC, 0);
    int base_id_3 = HistMaker::getUniqueID(kE_Veto, 0, kTDC, 0);
    int base_id_4 = HistMaker::getUniqueID(kE_Veto, 0, kTDC, 0) + 1;
    c->cd(1);
    TH1 *h1 = (TH1*)GHist::get(base_id_1);
    h1->Draw();
    c->cd(2);
    TH1 *h2 = (TH1*)GHist::get(base_id_2);
    h2->Draw();
    c->cd(3);
    TH1 *h3 = (TH1*)GHist::get(base_id_3);
    h3->Draw();
    c->cd(4);
    TH1 *h4 = (TH1*)GHist::get(base_id_4);
    h4->Draw();
    c->Update();
  }

  // draw Timing Counter
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(2,2);
    int base_id_1 = HistMaker::getUniqueID(kT0, 0, kTDC, 0);
    int base_id_2 = HistMaker::getUniqueID(kT0, 0, kTDC, 0) + 1;
    int base_id_3 = HistMaker::getUniqueID(kTOF, 0, kHitPat, 0);
    c->cd(1);
    TH1 *h1 = (TH1*)GHist::get(base_id_1);
    h1->Draw();
    c->cd(2);
    TH1 *h2 = (TH1*)GHist::get(base_id_2);
    h2->Draw();
    c->cd(3);
    TH1 *h3 = (TH1*)GHist::get(base_id_3);
    h3->Draw();
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
