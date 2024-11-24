// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void effCFT( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // draw Multi with plane efficiency BC3
  for( Int_t i=0; i<3; ++i ){
    TCanvas *c = (TCanvas*)gROOT->FindObject( Form("c%d", i+1));
    c->Clear();
    c->Divide(3,2);
    Int_t base_id = HistMaker::getUniqueID(kCFT, 0, kMulti, 10*i);
    for( Int_t l=0; l<NumOfLayersCFT; ++l ){
      c->cd(l+1);
      TH1 *h_wt = GHist::get(base_id+l+1);
      h_wt->Draw();
      Double_t Nof0     = h_wt->GetBinContent(1);
      Double_t NofTotal = h_wt->GetEntries();
      Double_t eff      = 1. - Nof0/NofTotal;
      static TLatex *text = new TLatex;
      text->SetNDC();
      text->SetTextSize(0.08);
      text->DrawLatex(0.3, 0.7, Form("plane eff. %.2f", eff));
    }
    c->Modified();
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
