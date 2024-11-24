// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void effFBT( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // draw Multi with plane efficiency FBT
  {
    const Int_t n_layer = 8;
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(4,2);
    Int_t base_id[n_layer] = {
      HistMaker::getUniqueID(kFBT1, 0, kMulti, 1),
      HistMaker::getUniqueID(kFBT1, 0, kMulti, 201),
      HistMaker::getUniqueID(kFBT1, 1, kMulti, 1),
      HistMaker::getUniqueID(kFBT1, 1, kMulti, 201),
      HistMaker::getUniqueID(kFBT2, 0, kMulti, 1),
      HistMaker::getUniqueID(kFBT2, 0, kMulti, 201),
      HistMaker::getUniqueID(kFBT2, 1, kMulti, 1),
      HistMaker::getUniqueID(kFBT2, 1, kMulti, 201)
    };
    for( Int_t i=0; i<n_layer; ++i ){
      c->cd(i+1);
      TH1 *h_wt = GHist::get(base_id[i]);
      h_wt->Draw();

      double Nof0     = h_wt->GetBinContent(1);
      double Nof1     = h_wt->GetBinContent(2);
      double Nof2     = h_wt->GetBinContent(3);
      double NofTotal = h_wt->GetEntries();
      double eff      = 1. - (double)Nof0/NofTotal;
      double eff1     = (double)Nof1/NofTotal;
      double eff2     = (double)Nof2/NofTotal;

      TLatex *text = new TLatex;
      TLatex *text_ratio = new TLatex;
      text->SetNDC();
      text->SetTextSize(0.07);
      text->DrawLatex(0.3, 0.7, Form("plane eff. %.2f", eff));
      text_ratio->SetNDC();
      text_ratio->SetTextSize(0.055);
      text_ratio->DrawLatex(0.3, 0.6, Form("eff1 : eff2 = 1 : %.3f", eff2/eff1));

      text->Draw();
      text_ratio->Draw();

    }
    c->Modified();
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
