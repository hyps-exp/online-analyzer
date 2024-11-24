// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

//_____________________________________________________________________
void
dispSSDMulti( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if( Updater::isUpdating() ){ return; }
  Updater::setUpdating(true);
  // ----------------------------------

  const int n_hist  = 8;
  int hist_id[n_hist] =
    {
      hist_id[0] = HistMaker::getUniqueID(kSSD1, 0, kMulti, 1),
      hist_id[1] = HistMaker::getUniqueID(kSSD1, 0, kMulti, 3),
      hist_id[2] = HistMaker::getUniqueID(kSSD1, 0, kMulti, 5),
      hist_id[3] = HistMaker::getUniqueID(kSSD1, 0, kMulti, 7),
      hist_id[4] = HistMaker::getUniqueID(kSSD2, 0, kMulti, 1),
      hist_id[5] = HistMaker::getUniqueID(kSSD2, 0, kMulti, 3),
      hist_id[6] = HistMaker::getUniqueID(kSSD2, 0, kMulti, 5),
      hist_id[7] = HistMaker::getUniqueID(kSSD2, 0, kMulti, 7)
    };
  int chist_id[n_hist] =
    {
      chist_id[0] = HistMaker::getUniqueID(kSSD1, 0, kMulti, 2),
      chist_id[1] = HistMaker::getUniqueID(kSSD1, 0, kMulti, 4),
      chist_id[2] = HistMaker::getUniqueID(kSSD1, 0, kMulti, 6),
      chist_id[3] = HistMaker::getUniqueID(kSSD1, 0, kMulti, 8),
      chist_id[4] = HistMaker::getUniqueID(kSSD2, 0, kMulti, 2),
      chist_id[5] = HistMaker::getUniqueID(kSSD2, 0, kMulti, 4),
      chist_id[6] = HistMaker::getUniqueID(kSSD2, 0, kMulti, 6),
      chist_id[7] = HistMaker::getUniqueID(kSSD2, 0, kMulti, 8)
    };

  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    if( c ){
      c->Clear();
      c->Divide(4,2);
      // draw SSD Multi
      for(int i=0; i<n_hist; ++i){
	c->cd(i+1)->SetGrid();
	TH1 *h = (TH1*)GHist::get( hist_id[i] );
	if( !h ) continue;
	h->Draw();
      }
      c->Update();
    }
  }

  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    if( c ){
      c->Clear();
      c->Divide(4,2);
      // draw SSD CMulti
      for(int i=0; i<n_hist; ++i){
	c->cd(i+1)->SetGrid();
	TH1 *h = (TH1*)GHist::get( chist_id[i] );
	if( !h ) continue;
	h->Draw();
      }
      c->Update();
    }
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
