#include "HistMaker.hh"
#include "DetectorID.hh"

// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

//_____________________________________________________________________
void
dispTAGMulti( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  int TAG_SF_base_id = HistMaker::getUniqueID(kTAG_SF, 0, kMulti, 0);
  int TAG_PL_base_id = HistMaker::getUniqueID(kTAG_PL, 0, kMulti, 0);
  TCanvas *c = (TCanvas*)gROOT->FindObject( Form("c%d", 1) );
  c->Clear();
  c->Divide(2, 2);

  //draw TAG_SF_Multi
  for( int i=0; i<NumOfLayersTAG_SF + 1; ++i ){
    c->cd(i+1);
    TH1 *h;
    if (i < NumOfLayersTAG_SF){
      h = GHist::get(TAG_SF_base_id + i);
    } else {
      h = GHist::get(TAG_PL_base_id);
    }
    h->Draw();
    Double_t total_hit = h->GetEntries();
    Double_t no_hit  = h->GetBinContent(1);
    Double_t one_hit = h->GetBinContent(2);
    TLatex *multi = new TLatex();
    multi->SetTextSize(0.07);
    multi->DrawLatexNDC(0.5, 0.6, Form("0 hit Eff. = %4.1f", (no_hit/total_hit)*100));
    multi->DrawLatexNDC(0.5, 0.5, Form("1 hit Eff. = %4.1f", (one_hit/total_hit)*100));
  }
  c->Update();

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
