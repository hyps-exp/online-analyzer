// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

#include "UserParamMan.hh"

void dispCaenV1725()
{
  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // Draw Waveform
  for (Int_t n =0; n<2; n++){
    TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", n+1));
    c->Clear();
    c->Divide(3,3);
    Int_t fadc_id = HistMaker::getUniqueID(kCaenV1725, 0, kFADC, 0);
    for( Int_t i=0; i<8; ++i ){
      c->cd(i+1);
      gPad->SetLogz();
      TH1 *h = (TH1*)GHist::get( fadc_id + i + n*8);
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser(0,512);
      h->Draw("colz");
    }
    c->Update();
  }
}
