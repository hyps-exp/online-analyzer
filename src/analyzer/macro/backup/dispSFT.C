// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispSFT()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  ////////// SFT
  {
    const int n_hist_c1 = 8;
    const int n_hist_c2 = 8;
    const int n_hist_c3 = 8;
    const int n_hist_c4 = 6;
    const int n_hist_c5 = 8;

    int sft_id_c1[n_hist_c1] = {
      HistMaker::getUniqueID(kSFT, 0, kTDC,     1), // X
      HistMaker::getUniqueID(kSFT, 0, kTDC,     2), // X'
      HistMaker::getUniqueID(kSFT, 0, kTDC,     3), // V
      HistMaker::getUniqueID(kSFT, 0, kTDC,     4), // U
      HistMaker::getUniqueID(kSFT, 0, kTDC,    11), // X
      HistMaker::getUniqueID(kSFT, 0, kTDC,    12), // X'
      HistMaker::getUniqueID(kSFT, 0, kTDC,    13), // V
      HistMaker::getUniqueID(kSFT, 0, kTDC,    14), // U
    };

    int sft_id_c2[n_hist_c2] = {
      HistMaker::getUniqueID(kSFT, 0, kADC,     1), // X
      HistMaker::getUniqueID(kSFT, 0, kADC,     2), // X'
      HistMaker::getUniqueID(kSFT, 0, kADC,     3), // V
      HistMaker::getUniqueID(kSFT, 0, kADC,     4), // U
      HistMaker::getUniqueID(kSFT, 0, kADC,    11), // X
      HistMaker::getUniqueID(kSFT, 0, kADC,    12), // X'
      HistMaker::getUniqueID(kSFT, 0, kADC,    13), // V
      HistMaker::getUniqueID(kSFT, 0, kADC,    14), // U
    };

    int sft_id_c3[n_hist_c3] = {
      HistMaker::getUniqueID(kSFT, 0, kHitPat,  1), // X
      HistMaker::getUniqueID(kSFT, 0, kHitPat,  2), // X'
      HistMaker::getUniqueID(kSFT, 0, kHitPat,  3), // V
      HistMaker::getUniqueID(kSFT, 0, kHitPat,  4), // U
      HistMaker::getUniqueID(kSFT, 0, kHitPat, 11), // X
      HistMaker::getUniqueID(kSFT, 0, kHitPat, 12), // X'
      HistMaker::getUniqueID(kSFT, 0, kHitPat, 13), // V
      HistMaker::getUniqueID(kSFT, 0, kHitPat, 14), // U
    };

    int sft_id_c4[n_hist_c4] = {
      HistMaker::getUniqueID(kSFT, 0, kMulti,   1), // X
      HistMaker::getUniqueID(kSFT, 0, kMulti,   2), // V
      HistMaker::getUniqueID(kSFT, 0, kMulti,   3), // U
      HistMaker::getUniqueID(kSFT, 0, kMulti,  11), // X
      HistMaker::getUniqueID(kSFT, 0, kMulti,  12), // V
      HistMaker::getUniqueID(kSFT, 0, kMulti,  13), // U
    };

    int sft_id_c5[n_hist_c5] = {
      HistMaker::getUniqueID(kSFT, 0, kTDC2D,   1), // X
      HistMaker::getUniqueID(kSFT, 0, kTDC2D,   2), // X'
      HistMaker::getUniqueID(kSFT, 0, kTDC2D,   3), // V
      HistMaker::getUniqueID(kSFT, 0, kTDC2D,   4), // U
      HistMaker::getUniqueID(kSFT, 0, kADC2D,   1), // X
      HistMaker::getUniqueID(kSFT, 0, kADC2D,   2), // X'
      HistMaker::getUniqueID(kSFT, 0, kADC2D,   3), // V
      HistMaker::getUniqueID(kSFT, 0, kADC2D,   4), // U
    };

//    int sft_id_c1[n_hist_c1] = {
//      HistMaker::getUniqueID(kSFT, 0, kTDC,     3), // V
//      HistMaker::getUniqueID(kSFT, 0, kTDC,     4), // U
//      HistMaker::getUniqueID(kSFT, 0, kTDC,     1), // X
//      HistMaker::getUniqueID(kSFT, 0, kTDC,     2), // X'
//      HistMaker::getUniqueID(kSFT, 0, kTDC,    13), // V
//      HistMaker::getUniqueID(kSFT, 0, kTDC,    14), // U
//      HistMaker::getUniqueID(kSFT, 0, kTDC,    11), // X
//      HistMaker::getUniqueID(kSFT, 0, kTDC,    12), // X'
//    };
//
//    int sft_id_c2[n_hist_c2] = {
//      HistMaker::getUniqueID(kSFT, 0, kADC,     3), // V
//      HistMaker::getUniqueID(kSFT, 0, kADC,     4), // U
//      HistMaker::getUniqueID(kSFT, 0, kADC,     1), // X
//      HistMaker::getUniqueID(kSFT, 0, kADC,     2), // X'
//      HistMaker::getUniqueID(kSFT, 0, kADC,    13), // V
//      HistMaker::getUniqueID(kSFT, 0, kADC,    14), // U
//      HistMaker::getUniqueID(kSFT, 0, kADC,    11), // X
//      HistMaker::getUniqueID(kSFT, 0, kADC,    12), // X'
//    };
//
//    int sft_id_c3[n_hist_c3] = {
//      HistMaker::getUniqueID(kSFT, 0, kHitPat,  3), // V
//      HistMaker::getUniqueID(kSFT, 0, kHitPat,  4), // U
//      HistMaker::getUniqueID(kSFT, 0, kHitPat,  1), // X
//      HistMaker::getUniqueID(kSFT, 0, kHitPat,  2), // X'
//      HistMaker::getUniqueID(kSFT, 0, kHitPat, 13), // V
//      HistMaker::getUniqueID(kSFT, 0, kHitPat, 14), // U
//      HistMaker::getUniqueID(kSFT, 0, kHitPat, 11), // X
//      HistMaker::getUniqueID(kSFT, 0, kHitPat, 12), // X'
//    };
//
//    int sft_id_c4[n_hist_c4] = {
//      HistMaker::getUniqueID(kSFT, 0, kMulti,   2), // V
//      HistMaker::getUniqueID(kSFT, 0, kMulti,   3), // U
//      HistMaker::getUniqueID(kSFT, 0, kMulti,   1), // X
//      HistMaker::getUniqueID(kSFT, 0, kMulti,  12), // V
//      HistMaker::getUniqueID(kSFT, 0, kMulti,  13), // U
//      HistMaker::getUniqueID(kSFT, 0, kMulti,  11), // X
//    };
//
//    int sft_id_c5[n_hist_c5] = {
//      HistMaker::getUniqueID(kSFT, 0, kTDC2D,   3), // V
//      HistMaker::getUniqueID(kSFT, 0, kTDC2D,   4), // U
//      HistMaker::getUniqueID(kSFT, 0, kTDC2D,   1), // X
//      HistMaker::getUniqueID(kSFT, 0, kTDC2D,   2), // X'
//      HistMaker::getUniqueID(kSFT, 0, kADC2D,   3), // V
//      HistMaker::getUniqueID(kSFT, 0, kADC2D,   4), // U
//      HistMaker::getUniqueID(kSFT, 0, kADC2D,   1), // X
//      HistMaker::getUniqueID(kSFT, 0, kADC2D,   2), // X'
//    };

    // draw TDC/TOT
    {
      TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
      c->Clear();
      c->Divide(4,2);
      for(int i=0; i<n_hist_c1; i++){
	c->cd(i+1);//->SetGrid();
	TH1 *h = (TH1*)GHist::get(sft_id_c1[i]);
	h->Draw();
      }
      c->Update();
    }

    {
      TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
      c->Clear();
      c->Divide(4,2);
      for(int i=0; i<n_hist_c2; i++){
	c->cd(i+1);//->SetGrid();
	TH1 *h = (TH1*)GHist::get(sft_id_c2[i]);
	if( TString(h->GetTitle()).Contains("TOT") )
	  h->GetXaxis()->SetRangeUser(0., 110.);
	h->Draw();

	if( TString(h->GetTitle()).Contains("TOT_U") ||
	    TString(h->GetTitle()).Contains("TOT_V")) {
	  double peak = h->GetMaximum();
	  TLine *l = new TLine(65, 0, 65, peak);
	  l->SetLineColor(kRed);
	  l->Draw("same");
	} else 	if( TString(h->GetTitle()).Contains("TOT_X")) {
	  double peak = h->GetMaximum();
	  TLine *l = new TLine(67.5, 0, 67.5, peak);
	  l->SetLineColor(kRed);
	  l->Draw("same");
	}

      }
      c->Update();
    }

    // draw HitPat
    {
      TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
      c->Clear();
      c->Divide(4,2);
      for(int i=0; i<n_hist_c3; i++){
	c->cd(i+1);//->SetGrid();
	TH1 *h = (TH1*)GHist::get(sft_id_c3[i]);
	h->Draw();
      }
      c->Update();
    }

    // draw Multi
    {
      TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
      c->Clear();
      c->Divide(3,2);
      for(int i=0; i<n_hist_c4; i++){
	c->cd(i+1);//->SetGrid();
	TH1 *h = (TH1*)GHist::get(sft_id_c4[i]);
	h->Draw();
      }
      c->Update();
    }
    // draw TDC/TOT 2D
    {
      TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
      c->Clear();
      c->Divide(4,2);
      for(int i=0; i<n_hist_c5; i++){
	c->cd(i+1);//->SetGrid();
	TH1 *h = (TH1*)GHist::get(sft_id_c5[i]);
	h->Draw("colz");
      }
      c->Update();
    }
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
