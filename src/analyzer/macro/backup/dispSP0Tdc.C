// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispSP0Tdc()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const int NumOfLayersSP0 = 8;
  const int NumOfSegSP0    = 5;

  ////////// SP0 TDC
  {
    // draw TDC
    for(int l=0; l<NumOfLayersSP0; l++){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", l/2+1));
      if(l%2==0){
	c->Clear();
	c->Divide(5,4);
      }
      for(int ud=0; ud<2; ud++){
	int base_id = HistMaker::getUniqueID(kSP0, kSP0_L1+l, kTDC, 1);
	for(int i=0; i<NumOfSegSP0; i++){
	  c->cd((l%2)*10 +ud*5 +i+1);
	  gPad->SetLogy();
	  TH1 *h = (TH1*)GHist::get(base_id +i +ud*NumOfSegSP0);
	  h->Draw();
	}
      }
      if(l%2==1) c->Update();
    }
    // draw HitPat
    {
      TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
      c->Clear();
      c->Divide(4,4);
      for(int l=0; l<NumOfLayersSP0; l++){
	for(int ud=0; ud<2; ud++){
	  c->cd(l*2 +ud +1);
	  int base_id = HistMaker::getUniqueID(kSP0, kSP0_L1+l, kHitPat, 1+ud);
	  TH1 *h = (TH1*)GHist::get(base_id);
	  h->Draw();
	}
      }
      c->Update();
    }
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
