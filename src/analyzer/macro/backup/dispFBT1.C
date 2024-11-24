// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispFBT1( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const Int_t FBTOffset = 200;

  std::vector< std::vector<Int_t> > hid = {
    {
      HistMaker::getUniqueID(kFBT1, 0, kTDC,   NumOfSegFBT1+1),
      HistMaker::getUniqueID(kFBT1, 0, kTDC,   FBTOffset+NumOfSegFBT1+1),
      HistMaker::getUniqueID(kFBT1, 1, kTDC,   NumOfSegFBT1+1),
      HistMaker::getUniqueID(kFBT1, 1, kTDC,   FBTOffset+NumOfSegFBT1+1),
      HistMaker::getUniqueID(kFBT1, 0, kTDC2D, 1),
      HistMaker::getUniqueID(kFBT1, 0, kTDC2D, FBTOffset+1),
      HistMaker::getUniqueID(kFBT1, 1, kTDC2D, 1),
      HistMaker::getUniqueID(kFBT1, 1, kTDC2D, FBTOffset+1)
    },
    {
      HistMaker::getUniqueID(kFBT1, 0, kADC,   NumOfSegFBT1+1),
      HistMaker::getUniqueID(kFBT1, 0, kADC,   FBTOffset+NumOfSegFBT1+1),
      HistMaker::getUniqueID(kFBT1, 1, kADC,   NumOfSegFBT1+1),
      HistMaker::getUniqueID(kFBT1, 1, kADC,   FBTOffset+NumOfSegFBT1+1),
      HistMaker::getUniqueID(kFBT1, 0, kADC2D, 1),
      HistMaker::getUniqueID(kFBT1, 0, kADC2D, FBTOffset+1),
      HistMaker::getUniqueID(kFBT1, 1, kADC2D, 1),
      HistMaker::getUniqueID(kFBT1, 1, kADC2D, FBTOffset+1)
    },
    {
      HistMaker::getUniqueID(kFBT1, 0, kHitPat, 1),
      HistMaker::getUniqueID(kFBT1, 0, kHitPat, FBTOffset+1),
      HistMaker::getUniqueID(kFBT1, 1, kHitPat, 1),
      HistMaker::getUniqueID(kFBT1, 1, kHitPat, FBTOffset+1),
      HistMaker::getUniqueID(kFBT1, 0, kMulti,  1),
      HistMaker::getUniqueID(kFBT1, 0, kMulti,  FBTOffset+1),
      HistMaker::getUniqueID(kFBT1, 1, kMulti,  1),
      HistMaker::getUniqueID(kFBT1, 1, kMulti,  FBTOffset+1),
    },
    {
      HistMaker::getUniqueID(kFBT1, 0, kHitPat, 1),
      HistMaker::getUniqueID(kFBT1, 0, kHitPat, FBTOffset+1),
      HistMaker::getUniqueID(kFBT1, 1, kHitPat, 1),
      HistMaker::getUniqueID(kFBT1, 1, kHitPat, FBTOffset+1),
      HistMaker::getUniqueID(kFBT1, 0, kMulti,  1),
      HistMaker::getUniqueID(kFBT1, 0, kMulti,  FBTOffset+1),
      HistMaker::getUniqueID(kFBT1, 1, kMulti,  1),
      HistMaker::getUniqueID(kFBT1, 1, kMulti,  FBTOffset+1),
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
      if( h ) h->Draw("colz");
    }
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
