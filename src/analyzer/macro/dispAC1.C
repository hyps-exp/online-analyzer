// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispAC1()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  int n_seg = 22;



  // draw ADC
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(2,2);
    int base_id = HistMaker::getUniqueID(kAC1, 0, kADC);
    int adcwtdc_id = HistMaker::getUniqueID(kAC1, 0 , kADCwTDC);
    for(int i = 0; i<4; ++i){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get(base_id + i);
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser( 0 , 4096 );
      h->Draw();
      TH1 *hh =(TH1*)GHist::get(adcwtdc_id + i);
      if( !hh ) continue;
      hh->GetXaxis()->SetRangeUser( 0 , 4096 );
      hh->SetLineColor( kRed );
      hh->Draw("same");

      }
    c->Update();
  }
  // draw TDC
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(6,4);
    int base_id = HistMaker::getUniqueID(kAC1, 0, kTDC);
    for(int i = 0; i<n_seg; ++i){
      c->cd(i+1);
      GHist::get(base_id + i)->Draw();
    }

    c->Update();
  }
  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
