// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispVme01()
{
  const UserParamMan& gUser = UserParamMan::GetInstance();
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const int n_v792    = 5;
  const int n_ch_v792 = 32;
  const int offset    = 16;
  const int x_max     = 300;

  // Draw v792
  {
    for(int mod = 0; mod<n_v792; ++mod){
      {
	// 1-16ch
	TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", 2*mod+1));
	c->Clear();
	c->Divide(4,4);

	for( int i=0; i<n_ch_v792/2; ++i ){
	  c->cd(i+1);
	  gPad->SetLogy();
	  TH1 *h = (TH1*)gDirectory->Get(Form("h%d-%d", mod, i));
	  if( !h ) continue;
	  h->GetXaxis()->SetRangeUser( 0, x_max );
	  h->Draw();
	}
	c->Update();
      }

      {
	// 17-32ch
	TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", 2*mod+2));
	c->Clear();
	c->Divide(4,4);

	for( int i=0; i<n_ch_v792/2; ++i ){
	  c->cd(i+1);
	  gPad->SetLogy();
	  TH1 *h = (TH1*)gDirectory->Get(Form("h%d-%d", mod, i+offset));
	  if( !h ) continue;
	  h->GetXaxis()->SetRangeUser( 0, x_max );
	  h->Draw();
	}
	c->Update();
      }
    }
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
