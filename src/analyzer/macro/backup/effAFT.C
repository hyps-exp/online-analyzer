// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

#include "DetectorID.hh"
void effAFT()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  int n_layer = 6;

  // draw Multi with X plane efficiency AFT
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(3,3);
    int aft_mul_id = HistMaker::getUniqueID(kAFT, 0, kMulti,  1);
    for(int i = 0; i<NumOfPlaneAFT; ++i){
      if( i%4 == 0){
	c->cd(i/4+1);
	TH1 *h_wt  = (TH1*)GHist::get(aft_mul_id+i);
	h_wt->Draw();
	TH1 *hh_wt  = (TH1*)GHist::get(aft_mul_id+2*NumOfPlaneAFT+i);
	hh_wt->SetLineColor(kRed);
	hh_wt->Draw("same");

	double Nof0     = h_wt->GetBinContent(1);
	double NofTotal = h_wt->GetEntries();
	double eff      = 1. - (double)Nof0/NofTotal;

	double CNof0     = hh_wt->GetBinContent(1);
	double CNofTotal = hh_wt->GetEntries();
	double Ceff      = 1. - (double)CNof0/CNofTotal;

	double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
	double ypos  = h_wt->GetMaximum()*0.8;
	TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.2f", eff));
	text->SetTextSize(0.06);
	text->Draw();
	TLatex *ttext = new TLatex(xpos*2, ypos*0.6, Form("plane eff. %.2f", Ceff));
	ttext->SetTextSize(0.06);
	ttext->SetTextColor(kRed);
	ttext->Draw("same");
	c->Modified();
	c->Update();
      }
    }
  }

  // draw Multi with X' plane efficiency AFT
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(3,3);
    int aft_mul_id = HistMaker::getUniqueID(kAFT, 0, kMulti,  1);
    for(int i = 0; i<NumOfPlaneAFT; ++i){
      if( i%4 == 1){
	c->cd(i/4+1);
	TH1 *h_wt  = (TH1*)GHist::get(aft_mul_id+i);
	h_wt->Draw();

	TH1 *hh_wt  = (TH1*)GHist::get(aft_mul_id+2*NumOfPlaneAFT+i);
	hh_wt->SetLineColor(kRed);
	hh_wt->Draw("same");

	double Nof0     = h_wt->GetBinContent(1);
	double NofTotal = h_wt->GetEntries();
	double eff      = 1. - (double)Nof0/NofTotal;

	double CNof0     = hh_wt->GetBinContent(1);
	double CNofTotal = hh_wt->GetEntries();
	double Ceff      = 1. - (double)CNof0/CNofTotal;

	double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
	double ypos  = h_wt->GetMaximum()*0.8;
	TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.2f", eff));
	text->SetTextSize(0.06);
	text->Draw();
	TLatex *ttext = new TLatex(xpos*2, ypos*0.6, Form("plane eff. %.2f", Ceff));
	ttext->SetTextSize(0.06);
	ttext->SetTextColor(kRed);
	ttext->Draw("same");
	c->Modified();
	c->Update();
      }
    }
  }

  // draw Multi with Y plane efficiency AFT
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(3,3);
    int aft_mul_id = HistMaker::getUniqueID(kAFT, 0, kMulti,  1);
    for(int i = 0; i<NumOfPlaneAFT; ++i){
      if( i%4 == 2){
	c->cd(i/4+1);
	TH1 *h_wt  = (TH1*)GHist::get(aft_mul_id+i);
	h_wt->Draw();
	TH1 *hh_wt  = (TH1*)GHist::get(aft_mul_id+2*NumOfPlaneAFT+i);
	hh_wt->SetLineColor(kRed);
	hh_wt->Draw("same");

	double Nof0     = h_wt->GetBinContent(1);
	double NofTotal = h_wt->GetEntries();
	double eff      = 1. - (double)Nof0/NofTotal;

	double CNof0     = hh_wt->GetBinContent(1);
	double CNofTotal = hh_wt->GetEntries();
	double Ceff      = 1. - (double)CNof0/CNofTotal;

	double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
	double ypos  = h_wt->GetMaximum()*0.8;
	TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.2f", eff));
	text->SetTextSize(0.06);
	text->Draw();
	TLatex *ttext = new TLatex(xpos*2, ypos*0.6, Form("plane eff. %.2f", Ceff));
	ttext->SetTextSize(0.06);
	ttext->SetTextColor(kRed);
	ttext->Draw("same");
	c->Modified();
	c->Update();
      }
    }
  }

  // draw Multi with Y' plane efficiency AFT
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
    c->Clear();
    c->Divide(3,3);
    int aft_mul_id = HistMaker::getUniqueID(kAFT, 0, kMulti,  1);
    for(int i = 0; i<NumOfPlaneAFT; ++i){
      if( i%4 == 3){
	c->cd(i/4+1);
	TH1 *h_wt  = (TH1*)GHist::get(aft_mul_id+i);
	h_wt->Draw();
	TH1 *hh_wt  = (TH1*)GHist::get(aft_mul_id+2*NumOfPlaneAFT+i);
	hh_wt->SetLineColor(kRed);
	hh_wt->Draw("same");

	double Nof0     = h_wt->GetBinContent(1);
	double NofTotal = h_wt->GetEntries();
	double eff      = 1. - (double)Nof0/NofTotal;

	double CNof0     = hh_wt->GetBinContent(1);
	double CNofTotal = hh_wt->GetEntries();
	double Ceff      = 1. - (double)CNof0/CNofTotal;

	double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
	double ypos  = h_wt->GetMaximum()*0.8;
	TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.2f", eff));
	text->SetTextSize(0.06);
	text->Draw();
	TLatex *ttext = new TLatex(xpos*2, ypos*0.6, Form("plane eff. %.2f", Ceff));
	ttext->SetTextSize(0.06);
	ttext->SetTextColor(kRed);
	ttext->Draw("same");
	c->Modified();
	c->Update();
      }
    }
  }


  // draw Multi with plane efficiency AFT
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
    c->Clear();
    c->Divide(3,3);
    int aft_mul_id = HistMaker::getUniqueID(kAFT, 0, kMulti,  1);
    for(int i = 0; i<NumOfPlaneAFT/4; ++i){
      c->cd(i+1);
      TH1 *h_wt  = (TH1*)GHist::get(aft_mul_id+NumOfPlaneAFT+i*2);
      h_wt->Draw();

      TH1 *hh_wt  = (TH1*)GHist::get(aft_mul_id+3*NumOfPlaneAFT+i*2);
      hh_wt->SetLineColor(kRed);
      hh_wt->Draw("same");

      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double CNof0     = hh_wt->GetBinContent(1);
      double CNofTotal = hh_wt->GetEntries();
      double Ceff      = 1. - (double)CNof0/CNofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.2f", eff));
      text->SetTextSize(0.06);
      text->Draw();
      TLatex *ttext = new TLatex(xpos*2, ypos*0.6, Form("plane eff. %.2f", Ceff));
      ttext->SetTextSize(0.06);
      ttext->SetTextColor(kRed);
      ttext->Draw("same");
      c->Modified();
      c->Update();
    }
  }

  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c6");
    c->Clear();
    c->Divide(3,3);
    int aft_mul_id = HistMaker::getUniqueID(kAFT, 0, kMulti,  1);
    for(int i = 0; i<NumOfPlaneAFT/4; ++i){
      c->cd(i+1);
      TH1 *h_wt  = (TH1*)GHist::get(aft_mul_id+NumOfPlaneAFT+i*2+1);
      h_wt->Draw();

      TH1 *hh_wt  = (TH1*)GHist::get(aft_mul_id+3*NumOfPlaneAFT+i*2+1);
      hh_wt->SetLineColor(kRed);
      hh_wt->Draw("same");

      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double CNof0     = hh_wt->GetBinContent(1);
      double CNofTotal = hh_wt->GetEntries();
      double Ceff      = 1. - (double)CNof0/CNofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.2f", eff));
      text->SetTextSize(0.08);
      text->Draw();
      TLatex *ttext = new TLatex(xpos*2, ypos*0.6, Form("plane eff. %.2f", Ceff));
      ttext->SetTextSize(0.06);
      ttext->SetTextColor(kRed);
      ttext->Draw("same");
      c->Modified();
      c->Update();
    }
  }

  // draw Multi with plane efficiency AFT (when there is a hit on a plane other than the plane)
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c7");
    c->Clear();
    c->Divide(3,3);
    int aft_mul_id = HistMaker::getUniqueID(kAFT, 0, kMulti,  NumOfPlaneAFT+NumOfPlaneAFT/2+1);
    for(int i = 0; i<NumOfPlaneAFT/4; ++i){
      c->cd(i+1);
      TH1 *h_wt  = (TH1*)GHist::get(aft_mul_id+i*2);
      h_wt->Draw();

      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.2f", eff));
      text->SetTextSize(0.06);
      text->Draw();
      c->Modified();
      c->Update();
    }
  }

  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c8");
    c->Clear();
    c->Divide(3,3);
    int aft_mul_id = HistMaker::getUniqueID(kAFT, 0, kMulti,  NumOfPlaneAFT+NumOfPlaneAFT/2+1);
    for(int i = 0; i<NumOfPlaneAFT/4; ++i){
      c->cd(i+1);
      TH1 *h_wt  = (TH1*)GHist::get(aft_mul_id+i*2+1);
      h_wt->Draw();

      double Nof0     = h_wt->GetBinContent(1);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;

      double xpos  = h_wt->GetXaxis()->GetBinCenter(h_wt->GetNbinsX())*0.3;
      double ypos  = h_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.2f", eff));
      text->SetTextSize(0.08);
      text->Draw();
      c->Modified();
      c->Update();
    }
  }

  // draw Multi(w/adc) with plane efficiency AFT
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c9");
    c->Clear();
    c->Divide(3,3);
    int aft_mul_id = HistMaker::getUniqueID(kAFT, 0, kMulti,  1);
    for(int i = 0; i<NumOfPlaneAFT/4; ++i){
      c->cd(i+1);
      TH1 *hh_wt  = (TH1*)GHist::get(aft_mul_id+3*NumOfPlaneAFT+i*2);
      hh_wt->SetLineColor(kRed);
      hh_wt->Draw();
      double CNof0     = hh_wt->GetBinContent(1);
      double CNofTotal = hh_wt->GetEntries();
      double Ceff      = 1. - (double)CNof0/CNofTotal;

      double xpos  = hh_wt->GetXaxis()->GetBinCenter(hh_wt->GetNbinsX())*0.3;
      double ypos  = hh_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.2f", Ceff));
      text->SetTextSize(0.06);
      text->Draw();
      c->Modified();
      c->Update();
    }
  }

  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c10");
    c->Clear();
    c->Divide(3,3);
    int aft_mul_id = HistMaker::getUniqueID(kAFT, 0, kMulti,  1);
    for(int i = 0; i<NumOfPlaneAFT/4; ++i){
      c->cd(i+1);
      TH1 *hh_wt  = (TH1*)GHist::get(aft_mul_id+3*NumOfPlaneAFT+i*2+1);
      hh_wt->SetLineColor(kRed);
      hh_wt->Draw();
      double CNof0     = hh_wt->GetBinContent(1);
      double CNofTotal = hh_wt->GetEntries();
      double Ceff      = 1. - (double)CNof0/CNofTotal;

      double xpos  = hh_wt->GetXaxis()->GetBinCenter(hh_wt->GetNbinsX())*0.3;
      double ypos  = hh_wt->GetMaximum()*0.8;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.2f", Ceff));
      text->SetTextSize(0.06);
      text->Draw();
      c->Modified();
      c->Update();
    }

  }


  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
