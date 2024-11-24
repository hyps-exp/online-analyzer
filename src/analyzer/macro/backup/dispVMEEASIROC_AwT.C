// -*- C++ -*-

#include "DetectorID.hh"

#include "UserParamMan.hh"

using namespace hddaq::gui; // for Updater

//_____________________________________________________________________________
void
dispVMEEASIROC_AwT( void )
{
  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  int drawPlane = 33;
  // draw HighGain-2D
  { //
    int vmeeasiroc_hg_2d_id   = HistMaker::getUniqueID(kVMEEASIROC, 0, kHighGain, 10);
    int vmeeasiroc_chg_2d_id  = HistMaker::getUniqueID(kVMEEASIROC, 0, kHighGain, 100);
    TH1D *h_hg[NumOfSegVMEEASIROC];
    TH1D *h_chg[NumOfSegVMEEASIROC];
    TCanvas *c;
    for( int iPlane=0; iPlane<NumOfPlaneVMEEASIROC; ++iPlane ){
      int planeId = PlaneIdOfVMEEASIROC[iPlane];
      if( planeId != drawPlane ) continue;
      TH2 *h_hg_2d  = (TH2*)GHist::get( vmeeasiroc_hg_2d_id + iPlane );
      TH2 *h_chg_2d = (TH2*)GHist::get( vmeeasiroc_chg_2d_id + iPlane );
      if( !h_hg_2d || !h_chg_2d ) continue;
      for( int iSeg = 0; iSeg < NumOfSegVMEEASIROC; ++iSeg ){
	if( iSeg%16 == 0 ){
	  c = (TCanvas*)gROOT->FindObject(Form("c%d", iSeg/16+1));
	  c->Clear();
	  c->Divide(4, 4);
	}
	c->cd(iSeg%16+1);
	h_hg[iSeg] = h_hg_2d->ProjectionY(Form("VMEEASIROC%d_Ch%d_HG", planeId, iSeg), iSeg+1, iSeg+1);
	h_hg[iSeg]->SetTitle(Form("VMEEASIROC%d_Ch%d_HG", planeId, iSeg));
	h_hg[iSeg]->Rebin(2);
	h_hg[iSeg]->GetXaxis()->SetRangeUser(700, 1100);
	h_hg[iSeg]->Draw();
	h_chg[iSeg] = h_chg_2d->ProjectionY(Form("VMEEASIROC%d_Ch%d_CHG", planeId, iSeg), iSeg+1, iSeg+1);
	h_chg[iSeg]->SetTitle(Form("VMEEASIROC%d_Ch%d_CHG", planeId, iSeg));
	h_chg[iSeg]->Rebin(2);
	h_chg[iSeg]->SetLineColor(kRed);
	h_chg[iSeg]->GetXaxis()->SetRangeUser(700, 1100);
	h_chg[iSeg]->Draw("same");

	if( iSeg%16 == 15 ) c->Update();
      }
    }
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------

}
