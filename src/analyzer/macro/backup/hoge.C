// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void hoge()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  TCanvas *c = (TCanvas*)gROOT->FindObject(gPad->GetName());
  c->Clear();
  c->Divide(4,3);
  int base_id = HistMaker::getUniqueID(kBH1, 0, kADC, 1);
  for(int i = 0; i<11; ++i){
    c->cd(i+1);
    GHist::get(base_id + i)->Draw();
  }

  c->Update();
  c->cd(0);

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
