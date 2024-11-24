// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispPWO_E05()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  int n_seg = 14;

  ////////// PWO ADC
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(4,4);
    int base_id = HistMaker::getUniqueID(kPWO, 0, kADC, 1);
    for(int i=0; i<n_seg; i++){
      c->cd(i+1)->SetLogy();
      TH1 *h = (TH1*)GHist::get(base_id + i);
      if(!h) continue;
      h->Draw();
    }
    c->Update();
  }

  ////////// PWO TDC
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(4,4);
    int base_id = HistMaker::getUniqueID(kPWO, 0, kTDC, 1);
    for(int i=0; i<n_seg; i++){
      c->cd(i+1)->SetLogy();
      TH1 *h = (TH1*)GHist::get(base_id + i);
      if(!h) continue;
      h->Draw();
    }
    c->Update();
  }

  ////////// PWO HitPat/Multi
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(2,2);
    int hist_id[2] = {
      HistMaker::getUniqueID(kPWO, 0, kHitPat, 1),
      HistMaker::getUniqueID(kPWO, 0, kMulti,  1)
    };
    for(int i=0; i<2; i++){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get(hist_id[i]);
      if(!h) continue;
      h->Draw();
    }
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
