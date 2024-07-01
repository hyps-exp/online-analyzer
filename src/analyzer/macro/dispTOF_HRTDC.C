#include "DetectorID.hh"

// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

#include "UserParamMan.hh"

void dispTOF_HRTDC()
{
  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------


  //TOF TDC gate range
  static const unsigned int tdc_min = gUser.GetParameter("TdcTOF", 0);
  static const unsigned int tdc_max = gUser.GetParameter("TdcTOF", 1);

  // draw TDC

  for (Int_t n=0; n< NumOfSegTOF_HRTDC/8; n++){
    TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", n+1));
    c->Clear();
    c->Divide(3,3);
    int tdc_id = HistMaker::getUniqueID( kTOF_HRTDC, 0, kTDC, 0 );
    for( int i=0; i<8; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get( tdc_id + i + n*8);
      // h->GetXaxis()->SetRangeUser( tdc_min, tdc_max );
      if( h ) h->Draw();
    }
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
