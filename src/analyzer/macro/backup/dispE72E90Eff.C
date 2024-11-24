// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

#include "UserParamMan.hh"

void dispE72E90Eff()
{

  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(3,2);
    Int_t icanvas = 0;
    { // BAC
      Int_t base_id = HistMaker::getUniqueID(kE72BAC, 0, kMulti);
      c->cd( ++icanvas );
      TH1 *h = (TH1*)GHist::get( base_id );
      h->Draw();

      double Nof0 = h->GetBinContent(1);
      double NofTotal = h->GetEntries();
      double eff = 1. - (double)Nof0/NofTotal;

      double xpos = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.3;
      double ypos = h->GetMaximum()*0.8;

      TLatex *text = new TLatex(xpos, ypos, Form("eff. %.2f",eff));
      text->SetTextSize(0.08);
      text->Draw();
    }
    { // SAC
      Int_t base_id = HistMaker::getUniqueID(kE90SAC, 0, kMulti);
      c->cd( ++icanvas );
      TH1 *h = (TH1*)GHist::get( base_id );
      h->Draw();

      double Nof0 = h->GetBinContent(1);
      double NofTotal = h->GetEntries();
      double eff = 1. - (double)Nof0/NofTotal;

      double xpos = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.3;
      double ypos = h->GetMaximum()*0.8;

      TLatex *text = new TLatex(xpos, ypos, Form("eff. %.2f",eff));
      text->SetTextSize(0.08);
      text->Draw();
    }
    { // SAC
      Int_t base_id = HistMaker::getUniqueID(kE90SAC, 0, kMulti);
      c->cd( ++icanvas );
      TH1 *h = (TH1*)GHist::get( base_id + 1 );
      h->Draw();

      double Nof0 = h->GetBinContent(1);
      double NofTotal = h->GetEntries();
      double eff = 1. - (double)Nof0/NofTotal;

      double xpos = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.3;
      double ypos = h->GetMaximum()*0.8;

      TLatex *text = new TLatex(xpos, ypos, Form("eff. %.2f",eff));
      text->SetTextSize(0.08);
      text->Draw();
    }
    { // KVC
      Int_t base_id = HistMaker::getUniqueID(kE72KVC, 0, kMulti);
      c->cd( ++icanvas );
      TH1 *h = (TH1*)GHist::get( base_id );
      h->Draw();

      double Nof0 = h->GetBinContent(1);
      double NofTotal = h->GetEntries();
      double eff = 1. - (double)Nof0/NofTotal;

      double xpos = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.3;
      double ypos = h->GetMaximum()*0.8;

      TLatex *text = new TLatex(xpos, ypos, Form("eff. %.2f",eff));
      text->SetTextSize(0.08);
      text->Draw();
    }
    { // KVC
      Int_t base_id = HistMaker::getUniqueID(kE72KVC, 0, kMulti);
      c->cd( ++icanvas );
      TH1 *h = (TH1*)GHist::get( base_id + 1 );
      h->Draw();

      double Nof0 = h->GetBinContent(1);
      double NofTotal = h->GetEntries();
      double eff = 1. - (double)Nof0/NofTotal;

      double xpos = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.3;
      double ypos = h->GetMaximum()*0.8;

      TLatex *text = new TLatex(xpos, ypos, Form("eff. %.2f",eff));
      text->SetTextSize(0.08);
      text->Draw();
    }
    { // BH2
      Int_t base_id = HistMaker::getUniqueID(kE42BH2, 0, kMulti);
      c->cd( ++icanvas );
      TH1 *h = (TH1*)GHist::get( base_id );
      h->Draw();

      double Nof0 = h->GetBinContent(1);
      double NofTotal = h->GetEntries();
      double eff = 1. - (double)Nof0/NofTotal;

      double xpos = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.3;
      double ypos = h->GetMaximum()*0.8;

      TLatex *text = new TLatex(xpos, ypos, Form("eff. %.2f",eff));
      text->SetTextSize(0.08);
      text->Draw();
    }
    c->Update();
  }
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->cd();
    Int_t base_id = HistMaker::getUniqueID(kE72Parasite, 0, kHitPat);
    TH1 *h = (TH1*)GHist::get( base_id );
    h->Draw();

    TLatex *text[NumOfSegE72Parasite];
    double NofT1 = h->GetBinContent(1);
    for( Int_t i=0; i<NumOfSegE72Parasite; ++i ) {
      double xpos = h->GetXaxis()->GetBinLowEdge(i+1);
      double ypos = h->GetBinContent(i+1)*1.2;
      double eff = 0;
      if(NofT1) {
	eff = (double)h->GetBinContent(i+1)/NofT1;
      }
      text[i] = new TLatex(xpos, ypos, Form("%.2f",eff));
      text[i]->SetTextSize(0.05);
      text[i]->Draw();
    }
    c->Update();
  }
  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
