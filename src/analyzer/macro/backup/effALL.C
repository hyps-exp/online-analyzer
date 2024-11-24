// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void effALL( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  {
    Int_t n_layer = 6;
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(6,4);
    Int_t base_id = HistMaker::getUniqueID(kBH1, 0, kMulti);
    c->cd(1);
    {
      TH1 *h_wt = GHist::get(base_id);
      h_wt->Draw();
      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("eff. %.4f", eff));
      text->SetTextSize(0.08);
      text->Draw();
    }

    base_id = HistMaker::getUniqueID(kBFT, 0, kMulti);
    c->cd(2);
    {
      TH1 *h_wt = GHist::get(base_id);
      h_wt->Draw();
      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("eff. %.4f", eff));
      text->SetTextSize(0.08);
      text->Draw();
    }


    base_id = HistMaker::getUniqueID(kBH2, 0, kMulti);
    c->cd(3);
    {
      TH1 *h_wt = GHist::get(base_id);
      h_wt->Draw();
      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("eff. %.4f", eff));
      text->SetTextSize(0.08);
      text->Draw();
    }

    base_id = HistMaker::getUniqueID(kBAC, 0, kMulti);
    c->cd(4);
    {
      TH1 *h_wt = GHist::get(base_id);
      h_wt->Draw();
      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("eff. %.4f", eff));
      text->SetTextSize(0.08);
      text->Draw();
    }

    base_id = HistMaker::getUniqueID(kBC3, 0, kMulti);
    for( Int_t i=0; i<n_layer; ++i ){
      c->cd(i+7);
      TH1 *h_wt = GHist::get(base_id +i +n_layer);
      h_wt->Draw();

      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.4f", eff));
      text->SetTextSize(0.08);
      text->Draw();
    }
    c->Modified();
    c->Update();

  // draw Multi with plane efficiency BC4

    base_id = HistMaker::getUniqueID(kBC4, 0, kMulti);
    for( Int_t i=0; i<n_layer; ++i ){
      c->cd(i+13);
      TH1 *h_wt = GHist::get(base_id +i +n_layer);
      h_wt->Draw();

      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.4f", eff));
      text->SetTextSize(0.08);
      text->Draw();
    }

    c->Modified();
    c->Update();


    base_id = HistMaker::getUniqueID(kTOF, 0, kMulti);
    c->cd(19);
    {
      TH1 *h_wt = GHist::get(base_id);
      h_wt->Draw();
      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("eff. %.4f", eff));
      text->SetTextSize(0.08);
      text->Draw();
    }
    c->Modified();
    c->Update();

    base_id = HistMaker::getUniqueID(kAC1, 0, kMulti);
    c->cd(20);
    {
      TH1 *h_wt = GHist::get(base_id);
      h_wt->Draw();
      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("eff. %.4f", eff));
      text->SetTextSize(0.08);
      text->Draw();
    }
    c->Modified();
    c->Update();


    base_id = HistMaker::getUniqueID(kWC, 0, kMulti);
    c->cd(21);
    {
      TH1 *h_wt = GHist::get(base_id);
      h_wt->Draw();
      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("eff. %.4f", eff));
      text->SetTextSize(0.08);
      text->Draw();
    }
    c->Modified();
    c->Update();


    base_id = HistMaker::getUniqueID(kSFV, 0, kMulti);
    c->cd(22);
    {
      TH1 *h_wt = GHist::get(base_id);
      h_wt->Draw();
      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("eff. %.4f", eff));
      text->SetTextSize(0.08);
      text->Draw();
    }
    c->Modified();
    c->Update();

  }

  // draw Multi with plane efficiency SDC1
  {
    Int_t n_layer = 6;
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(6,4);
    Int_t base_id = HistMaker::getUniqueID(kSDC1, 0, kMulti);
    for( Int_t i=0; i<n_layer; ++i ){
      c->cd(i+1);
      TH1 *h_wt = GHist::get(base_id +i +n_layer);
      h_wt->Draw();

      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.4f", eff));
      text->SetTextSize(0.08);
      text->Draw();
    }
    c->Modified();
    c->Update();

  // draw Multi with plane efficiency SDC2
    n_layer = 4;
    base_id = HistMaker::getUniqueID(kSDC2, 0, kMulti);
    for( Int_t i=0; i<n_layer; ++i ){
      c->cd(i+7);
      TH1 *h = GHist::get(base_id +i);
      h->Draw();
      TH1 *h_wt = GHist::get(base_id +i +n_layer);
      if( !h_wt ) continue;
      h_wt->SetLineColor(kBlue);
      h_wt->Draw("same");
      double Nof0     = h->GetBinContent(1);
      double NofTotal = h->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;
      double Nof0_wt     = h_wt->GetBinContent(1);
      double NofTotal_wt = h_wt->GetEntries();
      double eff_wt      = 1. - (double)Nof0_wt/NofTotal_wt;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.5;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.4f(%.4f)", eff, eff_wt));
      text->SetTextSize(0.08);
      text->Draw();
    }

    c->Modified();
    c->Update();
  // draw Multi with plane efficiency SDC3

    base_id = HistMaker::getUniqueID(kSDC3, 0, kMulti);
    for( Int_t i=0; i<n_layer; ++i ){
      c->cd(i+11);
      TH1 *h_wt = GHist::get(base_id +i +n_layer);
      h_wt->Draw();

      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.4f", eff));
      text->SetTextSize(0.08);
      text->Draw();
    }

    c->Modified();
    c->Update();

    // draw Multi with plane efficiency SDC4
    base_id = HistMaker::getUniqueID(kSDC4, 0, kMulti);
    for( Int_t i=0; i<n_layer; ++i ){
      c->cd(i+15);
      TH1 *h_wt = GHist::get(base_id +i +n_layer);
      h_wt->Draw();

      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.4f", eff));
      text->SetTextSize(0.08);
      text->Draw();
    }
    c->Modified();
    c->Update();

    // draw Multi with plane efficiency SDC5
    base_id = HistMaker::getUniqueID(kSDC5, 0, kMulti);
    for( Int_t i=0; i<n_layer; ++i ){
      c->cd(i+19);
      TH1 *h_wt = GHist::get(base_id +i +n_layer);
      h_wt->Draw();

      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.4f", eff));
      text->SetTextSize(0.08);
      text->Draw();
    }
    c->Modified();
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
