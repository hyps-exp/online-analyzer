// -*- C++ -*-

// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispMsT()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // draw TDC 1st harf
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(4,3);
    int base_id = HistMaker::getUniqueID(kMsT, 0, kTDC, 1);
    for( int i=0; i<NumOfSegTOF/2; ++i ){
      c->cd(i+1);
      TH1 *h1 = (TH1*)GHist::get(base_id +i);
      if( !h1 ) continue;
      h1->GetXaxis()->SetRangeUser(180000,210000);
      h1->Draw();
    }
    c->Update();
  }

  // draw TDC 2nd harf
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(4,3);
    int base_id = HistMaker::getUniqueID(kMsT, 0, kTDC, 1 +NumOfSegTOF/2);
    for( int i=0; i<NumOfSegTOF/2; ++i ){
      c->cd(i+1);
      TH1 *h1 = (TH1*)GHist::get(base_id + i);
      if( !h1 ) continue;
      h1->GetXaxis()->SetRangeUser(180000,210000);
      h1->Draw();
    }
    c->Update();
  }

  // draw HitPat with TOF & SCH
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(2,2);
    const int n_hist = 4;
    int hist_id[n_hist] = {
      HistMaker::getUniqueID(kMsT, 0, kHitPat, 0),
      HistMaker::getUniqueID(kMsT, 0, kHitPat, 1),
      HistMaker::getUniqueID(kTOF, 0, kHitPat),
      HistMaker::getUniqueID(kSCH, 0, kHitPat)
    };
    for( int i=0; i<n_hist; ++i ){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get(hist_id[i]);
      if( h ) h->Draw();
    }
    c->Update();
  }

  // draw HitPat & Flag
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
    c->Clear();
    c->Divide(2,2);
    int base_id = HistMaker::getUniqueID(kMsT, 0, kHitPat, 0);
    c->cd(1);
    GHist::get(base_id)->Draw();
    c->cd(2);
    base_id = HistMaker::getUniqueID(kMsT, 0, kTDC2D);
    GHist::get(base_id)->Draw("colz");
    c->cd(3);
    base_id = HistMaker::getUniqueID(kMsT, 0, kHitPat, 2);
    GHist::get(base_id)->Draw("");
    c->cd(4);
    base_id = HistMaker::getUniqueID(kMsT, 0, kHitPat2D,0);
    GHist::get(base_id)->Draw("text");
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
