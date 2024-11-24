// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void effSdcInOut( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // draw Multi with plane efficiency SDC1
  {
    const Int_t n_layer = 6;
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(3,2);
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
  }

  // draw Multi with plane efficiency SDC2
  {
    const Int_t n_layer = 4;
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(2,2);
    Int_t base_id = HistMaker::getUniqueID(kSDC2, 0, kMulti);
    Int_t base_id_ctot = HistMaker::getUniqueID(kSDC2, 0, kMulti, 1+kTOTcutOffset);
    for( Int_t i=0; i<n_layer; ++i ){
      c->cd(i+1);
      TH1 *h = GHist::get(base_id +i);
      h->Draw();
      TH1 *h_wt = GHist::get(base_id +i +n_layer);
      if( !h_wt ) continue;
      h_wt->SetLineColor(kBlue);
      h_wt->Draw("same");
      TH1 *h_wt_ctot = GHist::get(base_id_ctot +i +n_layer);
      if( !h_wt_ctot ) continue;
      //h_wt_ctot->SetStats(1);
      h_wt_ctot->SetLineColor(kRed);
      h_wt_ctot->Draw("same");
      double Nof0     = h->GetBinContent(1);
      double NofTotal = h->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;
      double Nof0_wt     = h_wt->GetBinContent(1);
      double NofTotal_wt = h_wt->GetEntries();
      double eff_wt      = 1. - (double)Nof0_wt/NofTotal_wt;
      double Nof0_wt_ctot     = h_wt_ctot->GetBinContent(1);
      double NofTotal_wt_ctot = h_wt_ctot->GetEntries();
      double eff_wt_ctot      = 1. - (double)Nof0_wt_ctot/NofTotal_wt_ctot;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff.\n %.4f(%.4f)", eff, eff_wt));
      text->SetTextSize(0.08);
      text->Draw();
      double ypos_ctot  = 500;
      TLatex *text_ctot = new TLatex(xpos, ypos_ctot, Form("                 [%.4f]", eff_wt_ctot));
      text_ctot->SetTextSize(0.08);
      text_ctot->Draw();
    }
    c->Modified();
    c->Update();
  }

  // draw Multi with plane efficiency SDC3
  {
    const Int_t n_layer = 4;
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(2,2);
    Int_t base_id = HistMaker::getUniqueID(kSDC3, 0, kMulti);
    for( Int_t i=0; i<n_layer; ++i ){
      c->cd(i+1);
      TH1 *h_wt = GHist::get(base_id +i +n_layer);
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
  }

    // draw Multi with plane efficiency SDC4
  {
    const Int_t n_layer = 4;
    TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
    c->Clear();
    c->Divide(2,2);
    Int_t base_id = HistMaker::getUniqueID(kSDC4, 0, kMulti);
    for( Int_t i=0; i<n_layer; ++i ){
      c->cd(i+1);
      TH1 *h_wt = GHist::get(base_id +i +n_layer);
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
  }

    // draw Multi with plane efficiency SDC5
  {
    const Int_t n_layer = 4;
    TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
    c->Clear();
    c->Divide(2,2);
    Int_t base_id = HistMaker::getUniqueID(kSDC5, 0, kMulti);
    for( Int_t i=0; i<n_layer; ++i ){
      c->cd(i+1);
      TH1 *h_wt = GHist::get(base_id +i +n_layer);
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
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
