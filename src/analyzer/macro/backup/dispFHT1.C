// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispFHT1( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const Int_t FHTOffset = 200;

  std::vector< std::vector<Int_t> > hid = {
    {
      HistMaker::getUniqueID(kFHT1, 0, kTDC,   NumOfSegFHT1+1),
      HistMaker::getUniqueID(kFHT1, 0, kTDC,   FHTOffset+NumOfSegFHT1+1),
      HistMaker::getUniqueID(kFHT1, 1, kTDC,   NumOfSegFHT1+1),
      HistMaker::getUniqueID(kFHT1, 1, kTDC,   FHTOffset+NumOfSegFHT1+1),
      HistMaker::getUniqueID(kFHT1, 0, kTDC2D, 1),
      HistMaker::getUniqueID(kFHT1, 0, kTDC2D, FHTOffset+1),
      HistMaker::getUniqueID(kFHT1, 1, kTDC2D, 1),
      HistMaker::getUniqueID(kFHT1, 1, kTDC2D, FHTOffset+1)
    },
    {
      HistMaker::getUniqueID(kFHT1, 0, kADC,   NumOfSegFHT1+1),
      HistMaker::getUniqueID(kFHT1, 0, kADC,   FHTOffset+NumOfSegFHT1+1),
      HistMaker::getUniqueID(kFHT1, 1, kADC,   NumOfSegFHT1+1),
      HistMaker::getUniqueID(kFHT1, 1, kADC,   FHTOffset+NumOfSegFHT1+1),
      HistMaker::getUniqueID(kFHT1, 0, kADC2D, 1),
      HistMaker::getUniqueID(kFHT1, 0, kADC2D, FHTOffset+1),
      HistMaker::getUniqueID(kFHT1, 1, kADC2D, 1),
      HistMaker::getUniqueID(kFHT1, 1, kADC2D, FHTOffset+1)
    },
    {
      HistMaker::getUniqueID(kFHT1, 0, kHitPat, 1),
      HistMaker::getUniqueID(kFHT1, 0, kHitPat, FHTOffset+1),
      HistMaker::getUniqueID(kFHT1, 1, kHitPat, 1),
      HistMaker::getUniqueID(kFHT1, 1, kHitPat, FHTOffset+1),
      HistMaker::getUniqueID(kFHT1, 0, kMulti,  1),
      HistMaker::getUniqueID(kFHT1, 0, kMulti,  FHTOffset+1),
      HistMaker::getUniqueID(kFHT1, 1, kMulti,  1),
      HistMaker::getUniqueID(kFHT1, 1, kMulti,  FHTOffset+1),
    },
  };

  //
  for( Int_t i=0, n=hid.size(); i<n; ++i ){
    TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", i+1));
    c->Clear();
    c->Divide(4,2);
    for( Int_t j=0, m=hid.at(i).size(); j<m; ++j ){
      c->cd(j+1);//->SetGrid();
      TH1 *h = (TH1*)GHist::get( hid.at(i).at(j) );
      if( !h ) continue;
      if( TString(h->GetTitle()).Contains("TOT") )
	h->GetXaxis()->SetRangeUser(0., 100.);
      h->Draw("colz");

      if( TString(h->GetTitle()).Contains("TOT") && TString(h->GetTitle()).Contains("UX")) {
	double peak = h->GetMaximum();
	TLine *l = new TLine(58, 0, 58, peak);
	l->SetLineColor(kRed);
	l->Draw("same");
      } else if( TString(h->GetTitle()).Contains("TOT") && TString(h->GetTitle()).Contains("DX")) {
	double peak = h->GetMaximum();
	TLine *l = new TLine(60, 0, 60, peak);
	l->SetLineColor(kRed);
	l->Draw("same");
      }


    }
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
