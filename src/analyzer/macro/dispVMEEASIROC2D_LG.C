// -*- C++ -*-

#include "DetectorID.hh"

#include "UserParamMan.hh"

using namespace hddaq::gui; // for Updater

//_____________________________________________________________________________
void
dispVMEEASIROC2D_LG( void )
{
  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  Int_t nplane[5] = {18, 19, 19, 18, 5};
  { // aft01
    Int_t first = 0;
    for( Int_t j=0; j<5; j++ ){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", j+1));
      c->Clear();
      c->Divide(4, 5);
      int vmeeasiroc_lg_2d_id = HistMaker::getUniqueID(kVMEEASIROC, 0, kLowGain,   10);
      for( int i=0; i<NumOfPlaneVMEEASIROC; ++i ){
	if( !(first<=i && i<(first+nplane[j])) )continue;
	c->cd(i-first+1);
	TH2 *h = (TH2*)GHist::get( vmeeasiroc_lg_2d_id + i );
	if( !h ) continue;
	h->Draw("colz");

	double stddev_mean = 0.;
	for( int iSeg = 0; iSeg < NumOfSegVMEEASIROC; ++iSeg ){
	  TH1I *h_seg  = (TH1I*)h->ProjectionY(Form("h_seg_%d", iSeg), iSeg+1, iSeg+1);
	  stddev_mean += h_seg->GetStdDev();
	  if( iSeg == NumOfSegVMEEASIROC/2-1 ){
	    stddev_mean /= NumOfSegVMEEASIROC/2.;
	    double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.1;
	    double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
	    TLatex *text = new TLatex(xpos, ypos, Form("%.2f", stddev_mean));
	    text->SetTextSize(0.16);
	    text->Draw();
	    stddev_mean = 0.;
	    continue;
	  }
	  else if( iSeg == NumOfSegVMEEASIROC-1 ){
	    stddev_mean /= NumOfSegVMEEASIROC/2.;
	    double xpos  = h->GetXaxis()->GetBinCenter(h->GetNbinsX())*0.6;
	    double ypos  = h->GetYaxis()->GetBinCenter(h->GetNbinsY())*0.5;
	    TLatex *text = new TLatex(xpos, ypos, Form("%.2f", stddev_mean));
	    text->SetTextSize(0.16);
	    text->Draw();
	  }
	}
      }
      first += nplane[j];
      c->Update();
    }
  }


  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------

}
