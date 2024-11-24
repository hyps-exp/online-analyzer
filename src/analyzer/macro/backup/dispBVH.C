// -*- C++ -*-

using hddaq::gui::Updater;

#include "UserParamMan.hh"

//_____________________________________________________________________________
void
dispBVH()
{
  if(Updater::isUpdating())
    return;
  Updater::setUpdating(true);

  const auto& gUser = UserParamMan::GetInstance();
  const auto tdc_min = gUser.GetParameter("TdcBVH", 0);
  const auto tdc_max = gUser.GetParameter("TdcBVH", 1);

  { ///// TDC
    auto c1 = dynamic_cast<TCanvas*>(gROOT->FindObject("c1"));
    c1->Clear();
    c1->Divide(2, 2);
    auto tdc_hid = HistMaker::getUniqueID(kBVH, 0, kTDC);
    for(Int_t i=0; i<NumOfSegBVH; ++i){
      c1->cd(i+1);
      auto h1 = dynamic_cast<TH1*>(GHist::get(tdc_hid+i));
      if(!h1) continue;
      h1->GetXaxis()->SetRangeUser(tdc_min, tdc_max);
      h1->Draw();
    }
    c1->Update();
  }

  Updater::setUpdating(false);
}
