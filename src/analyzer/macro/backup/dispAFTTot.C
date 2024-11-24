// -*- C++ -*-

#include "DetectorID.hh"
#include "UserParamMan.hh"

using namespace hddaq::gui; // for Updater

//_____________________________________________________________________________
void
dispAFTTot( void )
{
  //  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------
  const auto& gUser = UserParamMan::GetInstance();
  const Double_t TotRef = gUser.GetParameter( "TotRefAFT" );

  // draw TOT for each MPPC vias
  {

    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(2, 2);
    Int_t aft_tot_id  = HistMaker::getUniqueID(kAFT, 0, kTOT, 1);
    TH1* h[4];
    for(Int_t i=0; i<4; i++){
      c->cd(i+1);
      h[i] = (TH1*)GHist::get(aft_tot_id + kUorD*NumOfPlaneAFT + i);
      h[i]->Draw();
      TF1 f("f", "gaus", 0., 100.);
      f.SetLineColor(kRed);
      Double_t p = TotRef;
      // Double_t p = h[i]->GetBinCenter(h[i]->GetMaximumBin());
      // if(p < 30.) p = TotRef;
      Double_t w = 10.;
      for(Int_t ifit=0; ifit<3; ++ifit){
	Double_t fmin = p - w;
	Double_t fmax = p + w;
	h[i]->Fit("f", "Q", "", fmin, fmax);
	p = f.GetParameter(1);
	w = f.GetParameter(2) * 1.;
      }
      TLatex *text = new TLatex();
      text->SetNDC();
      text->SetTextSize(0.07);
      text->DrawLatex(0.500, 0.700, Form("%.1f", p));
      text->DrawLatex(0.500, 0.630, Form("Ref : %.1f", TotRef));
      auto l = new TLine( TotRef, 0, TotRef, h[i]->GetMaximum() );
      l->SetLineColor(kRed);
      l->Draw();
    }
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
