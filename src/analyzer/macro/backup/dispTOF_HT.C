// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispTOF_HT()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // TDC gate range
  const UserParamMan& gUser = UserParamMan::GetInstance();
  static const unsigned int tdc_min = gUser.GetParameter("TOFHT_TDC", 0);
  static const unsigned int tdc_max = gUser.GetParameter("TOFHT_TDC", 1);

  // draw TDC-HT TDC
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(4,4);
    int tdc_id = HistMaker::getUniqueID( kTOF_HT, 0, kTDC, 1 );
    for( int i=0; i<NumOfSegHtTOF; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( tdc_id + i );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser(tdc_min-100, tdc_max+100);
      h->Draw();
    }
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
