// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

//_____________________________________________________________________
void
dispACs( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const int n_hist = 4; // BAC1, BAC2, PVAC, FAC
  
  int adc_id[n_hist] = {
    HistMaker::getUniqueID(kBAC,  0, kADC, 1),// BAC1
    HistMaker::getUniqueID(kBAC,  0, kADC, 2),// BAC2
    HistMaker::getUniqueID(kPVAC, 0, kADC, 1),// PVAC
    HistMaker::getUniqueID(kFAC,  0, kADC, 1),// FAC
  };

  int adcwtdc_id[n_hist] = {
    HistMaker::getUniqueID(kBAC,  0, kADCwTDC, 1),// BAC1
    HistMaker::getUniqueID(kBAC,  0, kADCwTDC, 2),// BAC2
    HistMaker::getUniqueID(kPVAC, 0, kADCwTDC, 1),// PVAC
    HistMaker::getUniqueID(kFAC,  0, kADCwTDC, 1),// FAC
  };

  int tdc_id[n_hist] = {
    HistMaker::getUniqueID(kBAC,  0, kTDC, 1),// BAC1
    HistMaker::getUniqueID(kBAC,  0, kTDC, 2),// BAC2
    HistMaker::getUniqueID(kPVAC, 0, kTDC, 1),// PVAC
    HistMaker::getUniqueID(kFAC,  0, kTDC, 1),// FAC
  };

  { 
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide( n_hist, 2 );
    // draw ACs ADC
    for(int i = 0; i<n_hist; ++i){
      c->cd(i+1);
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( adc_id[i] );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser(0,4000);
      h->Draw();
      TH1 *hh = (TH1*)GHist::get( adcwtdc_id[i] );
      if( !hh ) continue;
      hh->GetXaxis()->SetRangeUser(0,4000);
      hh->SetLineColor( kRed );
      hh->Draw("same");
    }
    // draw ACs TDC
    for(int i = 0; i<n_hist; ++i){
      c->cd( i+n_hist+1 );
      gPad->SetLogy();
      TH1 *h = (TH1*)GHist::get( tdc_id[i] );
      if( !h ) continue;
      h->GetXaxis()->SetRangeUser(0,4000);
      h->Draw();
    }

    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
