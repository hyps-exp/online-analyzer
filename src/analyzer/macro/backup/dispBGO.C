// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispBGO()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const int NumOfSegBGO = 48;
  const int NumOfSlot = 4;

  ////////// BGO TDC
  {
    for(int slot=0; slot<NumOfSlot; slot++){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", slot+1));
      c->Clear();
      c->Divide(3,4);
      int base_id = HistMaker::getUniqueID(kBGO, 0, kTDC, 1);
      for(int i=0; i<NumOfSegBGO/NumOfSlot; i++){
	c->cd(i+1);
	TH1 *h = (TH1*)GHist::get(base_id + i + slot*NumOfSegBGO/NumOfSlot);
	h->Draw();
      }
      c->Update();
    }
  }

  ////////// BGO TDC 2D
  {

      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", 5));
      c->Clear();
      int base_id = HistMaker::getUniqueID(kBGO, 0, kTDC2D, 1);
      TH2 *h2 = (TH2*)GHist::get(base_id);
      h2->Draw("colz");
      c->Update();
  }


  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
