// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void clear_all_canvas( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const Int_t n_canvas = 10;
  for( Int_t i=0; i<n_canvas; ++i ){
    TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", i+1));
    if( !c ) continue;
    c->Clear();
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
