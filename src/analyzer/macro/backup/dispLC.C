// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispLC()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // TDC gate range
  const UserParamMan& gUser = UserParamMan::GetInstance();
  static const unsigned int tdc_min = gUser.GetParameter("LC_TDC", 0);
  static const unsigned int tdc_max = gUser.GetParameter("LC_TDC", 1);

  TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  c->Clear();
  c->Divide(7,4);
  int  base_id = HistMaker::getUniqueID(kLC, 0, kTDC, 1);
  for(int i = 0; i<NumOfSegLC; ++i){
    c->cd(i+1);
    TH1 *h = GHist::get(base_id + i);
    if(!h) continue;
    h->GetXaxis()->SetRangeUser(tdc_min-100, tdc_max+100);
    h->Draw();
  }

  c->Update();

  c->cd(0);

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
