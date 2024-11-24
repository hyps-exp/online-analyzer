// -*- C++ -*-

// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

//______________________________________________________________________________
void
effSSD( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const Int_t n_layer = 4;

  TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  c->Clear();
  c->Divide(4,2);

  // SSD1
  {
    Int_t base_id = HistMaker::getUniqueID(kSSD1, 0, kMulti);
    for(Int_t i = 0; i<n_layer; ++i){
      c->cd(i+1);
      TH1 *h = GHist::get(base_id + i*2+1);
      if( !h ) continue;
      h->Draw();
      Double_t Nof0     = h->GetBinContent(1);
      Double_t NofTotal = h->GetEntries();
      Double_t eff      = 1. - (Double_t)Nof0/NofTotal;
      Double_t xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.3;
      Double_t ypos  = h->GetMaximum()*0.65;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.2f", eff));
      text->SetTextSize(0.08);
      text->Draw();
    }
  }

  // SSD2
  {
    Int_t base_id = HistMaker::getUniqueID(kSSD2, 0, kMulti);
    for(Int_t i = 0; i<n_layer; ++i){
      c->cd(i+n_layer+1);
      TH1 *h = GHist::get(base_id + i*2+1);
      if( !h ) continue;
      h->Draw();
      Double_t Nof0     = h->GetBinContent(1);
      Double_t NofTotal = h->GetEntries();
      Double_t eff      = 1. - (Double_t)Nof0/NofTotal;
      Double_t xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.3;
      Double_t ypos  = h->GetMaximum()*0.65;
      TLatex *text = new TLatex(xpos, ypos, Form("plane eff. %.2f", eff));
      text->SetTextSize(0.08);
      text->Draw();
    }
  }
  c->Modified();
  c->Update();

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
