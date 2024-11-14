// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void effSdcOut()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  int n_layer = 6;

  // draw Multi with plane efficiency SDC3
  TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  c->Clear();
  c->Divide(3,2);
  int base_id = HistMaker::getUniqueID(kSDC3, 0, kMulti);
  for(int i = 0; i<n_layer; ++i){
    c->cd(i+1);
    TH1 *h_wt  = GHist::get(base_id + i + n_layer);
    h_wt->Draw();

    double Nof0     = h_wt->GetBinContent(1);
    double NofTotal = h_wt->GetEntries();
    double eff      = 1. - (double)Nof0/NofTotal;

    double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
    double ypos  = h_wt->GetMaximum()*0.8;
    TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.2f", eff));
    text->SetTextSize(0.08);
    text->Draw();
  }

  c->Modified();
  c->Update();

  // draw Multi with plane efficiency SDC4
  c = (TCanvas*)gROOT->FindObject("c2");
  c->Clear();
  c->Divide(3,2);
  int base_id = HistMaker::getUniqueID(kSDC4, 0, kMulti);
  for(int i = 0; i<n_layer; ++i){
    c->cd(i+1);
    TH1 *h_wt  = GHist::get(base_id + i + n_layer);
    h_wt->Draw();

    double Nof0     = h_wt->GetBinContent(1);
    double NofTotal = h_wt->GetEntries();
    double eff      = 1. - (double)Nof0/NofTotal;

    double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
    double ypos  = h_wt->GetMaximum()*0.8;
    TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.2f", eff));
    text->SetTextSize(0.08);
    text->Draw();
  }

  c->Modified();
  c->Update();

  c->cd(0);

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
