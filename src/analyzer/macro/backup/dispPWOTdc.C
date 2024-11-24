// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispPWOTdc()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  int n_seg = 22;
  ////////// PWO TDC SUM
  {
    for(int ud=0; ud<2; ud++){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", ud+1));
      c->Clear();
      c->Divide(3,4);
      int base_id = HistMaker::getUniqueID(kPWO, 0, kTDC, 1);
      for(int i=0; i<n_seg/2; i++){
	c->cd(i+1);
	TH1 *h = (TH1*)GHist::get(base_id + i + ud*n_seg/2);
	h->Draw();
      }
      c->Update();
    }
  }
  ////////// PWO TDC 2D
  {
    for(int ud=0; ud<2; ud++){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", ud+3));
      c->Clear();
      c->Divide(3,4);
      int base_id = HistMaker::getUniqueID(kPWO, 0, kTDC2D, 1);
      for(int i=0; i<n_seg/2; i++){
	c->cd(i+1);
	TH2 *h2 = (TH2*)GHist::get(base_id + i + ud*n_seg/2);
	h2->Draw("colz");
      }
      c->Update();
    }
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
