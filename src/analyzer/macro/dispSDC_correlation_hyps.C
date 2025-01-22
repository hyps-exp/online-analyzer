// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispSDC_correlation_hyps( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  gStyle->SetOptStat(1111110);
  gStyle->SetOptLogz(1);
  Int_t NumOfPairPlane = 2;
  
  // draw SDCIn
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(2,2);
    int sdc0_id = HistMaker::getUniqueID(kSDC0, 0, kCorr, 20);
    int sdc1_id = HistMaker::getUniqueID(kSDC1, 0, kCorr, 20);
    for( int i=0; i<NumOfPairPlane; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get(sdc0_id + i);
      h->Draw();
      gStyle->SetStatY(0.6);
    }
    for( int i=0; i<NumOfPairPlane; ++i ){
      c->cd(i+3);
      TH1 *hh = (TH1*)GHist::get(sdc1_id + i);
      hh->Draw();
      gStyle->SetStatY(0.6);
    }
    c->Update();
  }

  // draw SDCOut
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(2,2);
    int sdc2_id = HistMaker::getUniqueID(kSDC2, 0, kCorr, 20);
    int sdc3_id = HistMaker::getUniqueID(kSDC3, 0, kCorr, 20);
    for( int i=0; i<NumOfPairPlane; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get(sdc2_id + i);
      h->Draw();
      gStyle->SetStatY(0.6);
    }
    for( int i=0; i<NumOfPairPlane; ++i ){
      c->cd(i+3);
      TH1 *hh = (TH1*)GHist::get(sdc3_id + i);
      hh->Draw();
      gStyle->SetStatY(0.6);
    }
    c->Update();
  }
  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
