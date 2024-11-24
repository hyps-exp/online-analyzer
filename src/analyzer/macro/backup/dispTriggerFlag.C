// -*- C++ -*-

// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

//_____________________________________________________________________________
void
dispTriggerFlag( void )
{
  // for thread safe
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);

  int n_seg = 16;
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(4,4);
    int  base_id = HistMaker::getUniqueID(kTriggerFlag, 0, kTDC, 1);
    for(int i = 0; i<n_seg; ++i){
      gPad->SetLogy();
      c->cd(i+1);
      GHist::get(base_id + i)->Draw();
    }
    c->Update();
  }

  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(4,4);
    int  base_id = HistMaker::getUniqueID(kTriggerFlag, 0, kTDC, 17);
    for(int i = 0; i<n_seg; ++i){
      gPad->SetLogy();
      c->cd(i+1);
      if( i+n_seg == trigger::NTriggerFlag )
	break;
      GHist::get(base_id + i)->Draw();
    }
    c->Update();
  }

  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    int  base_id = HistMaker::getUniqueID(kTriggerFlag, 0, kHitPat, 1);
    GHist::get(base_id)->Draw();
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
