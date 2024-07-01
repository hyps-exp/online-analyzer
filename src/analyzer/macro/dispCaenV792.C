// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

#include "UserParamMan.hh"

void dispCaenV792()
{
  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // Draw ADC
  for (Int_t n =0; n<4; n++){
    TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", n+1));
    c->Clear();
    c->Divide(3,3);
    Int_t adc_id = HistMaker::getUniqueID(kCaenV792, 0, kADC, 0);
    for( Int_t i=0; i<8; ++i ){
      c->cd(i+1);
      // gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( adc_id + i + n*8);
      if( !h ) continue;
      // h->GetXaxis()->SetRangeUser(0, 4096);
      h->Draw();
    }
    c->Update();
  }
}
