// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispBeamProfile_e42_Kbeam()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------


  const Int_t n_hist = 5;
  const Int_t NumHist=(3*n_hist+1);//X, Y, XY, U
  //  const Int trig =1; //0: unbias, 1: K-beam, 2: pi-beam
  const Int_t trig =1; //0: unbias, 1: K-beam, 2: pi-beam


  TLatex tex;
  tex.SetNDC();
  tex.SetTextSize(0.14);
  Double_t xpos = 0.15, ypos = 0.75;

  const Double_t fit_width[2] = { 50., 50. };

  //const Double_t ff[] = { 200., 500., 680, 1000., 1200. };
  const Double_t ff[] = { -600., -300., 0, 300., 600. };
  Double_t   rms[2][n_hist];
  Double_t sigma[2][n_hist];
  Double_t   mean[2][n_hist];
  Double_t mean_fit[2][n_hist];

  // XY position (dispRMS)
  {
    TCanvas *c = dynamic_cast<TCanvas*>(gROOT->FindObject("c1"));
    c->Clear();
    c->Divide(5,2);

    Int_t base_id = HistMaker::getUniqueID(kMisc, 0, kHitPat);

    for( Int_t xy=0; xy<2; ++xy ){
      for(Int_t i=0; i<n_hist; i++){
	c->cd(i+1+xy*n_hist);
	TH1 *h = (TH1*)GHist::get(base_id +i +xy*n_hist + (NumHist*trig))->Clone();
	h->GetXaxis()->SetRangeUser(-200,200);
	rms[xy][i] = h->GetRMS();
	mean[xy][i] = h->GetMean();
	h->Draw();
	tex.DrawLatex(xpos, ypos, Form("%.2lf", rms[xy][i]));
      }
    }
    c->Update();
  }

  // XY position (dispSigma)
  {
    TCanvas *c = dynamic_cast<TCanvas*>(gROOT->FindObject("c2"));
    c->Clear();
    c->Divide(5,2);
    Int_t base_id = HistMaker::getUniqueID(kMisc, 0, kHitPat);
    TF1 *f = new TF1("f", "gaus");
    f->SetLineColor(kBlue);
    for( Int_t xy=0; xy<2; ++xy ){
      for( Int_t i=0; i<n_hist; ++i ){
	c->cd(i+1+xy*n_hist);
	TH1 *h = (TH1*)GHist::get(base_id +i +xy*n_hist + (NumHist*trig))->Clone();
	h->GetXaxis()->SetRangeUser(-200,200);
	// Double_t max = h->GetBinCenter(h->GetMaximumBin());
	Double_t max = 0.;
	h->Fit("f", "Q", "", max-fit_width[xy], max+fit_width[xy]);
	sigma[xy][i] = f->GetParameter("Sigma");
	mean_fit[xy][i] = f->GetParameter("Mean");
	h->Draw();
	tex.DrawLatex(xpos, ypos, Form("%.2lf", sigma[xy][i]));
      }
    }
    c->Update();
  }

  // X(Mean)%FFpos & X(Mean_fit)%FFpos
  {
    TCanvas *c = dynamic_cast<TCanvas*>(gROOT->FindObject("c3"));
    c->Clear();
    c->Divide(2,2);
    {
      c->cd(1)->SetGrid();
      TGraph *gr = new TGraph(n_hist, ff, mean[0] );
      gr->SetTitle("BeamProfile X (Mean)");
      gr->GetXaxis()->SetTitle("FF+[mm]");
      gr->GetYaxis()->SetTitle("[mm]  ");
      gr->GetYaxis()->SetTitleOffset(1.2);
      gr->SetMarkerStyle(8);
      gr->SetMarkerSize(3);
      gr->SetMarkerColor(kRed);
      gr->Draw("AP");
      c->cd(2)->SetGrid();
      TGraph *gs = new TGraph(n_hist, ff, mean_fit[0] );
      gs->SetTitle("BeamProfile X (Mean_fit)");
      gs->GetXaxis()->SetTitle("FF+[mm]");
      gs->GetYaxis()->SetTitle("[mm]  ");
      gs->GetYaxis()->SetTitleOffset(1.2);
      gs->SetMarkerStyle(8);
      gs->SetMarkerSize(3);
      gs->SetMarkerColor(kBlue);
      gs->Draw("AP");
    }
    {
      c->cd(3)->SetGrid();
      TGraph *gr = new TGraph(n_hist, ff, mean[1] );
      gr->SetTitle("BeamProfile Y (Mean)");
      gr->GetXaxis()->SetTitle("FF+[mm]");
      gr->GetYaxis()->SetTitle("[mm]  ");
      gr->GetYaxis()->SetTitleOffset(1.2);
      gr->SetMarkerStyle(8);
      gr->SetMarkerSize(3);
      gr->SetMarkerColor(kRed);
      gr->Draw("AP");
      c->cd(4)->SetGrid();
      TGraph *gs = new TGraph(n_hist, ff, mean_fit[1] );
      gs->SetTitle("BeamProfile Y (Mean_fit)");
      gs->GetXaxis()->SetTitle("FF+[mm]");
      gs->GetYaxis()->SetTitle("[mm]  ");
      gs->GetYaxis()->SetTitleOffset(1.2);
      gs->SetMarkerStyle(8);
      gs->SetMarkerSize(3);
      gs->SetMarkerColor(kBlue);
      gs->Draw("AP");
    }
    c->Update();
  }


  // X(RMS)%FFpos & X(sigma)%FFpos
  {
    TCanvas *c = dynamic_cast<TCanvas*>(gROOT->FindObject("c4"));
    c->Clear();
    c->Divide(2,2);
    {
      c->cd(1)->SetGrid();
      TGraph *gr = new TGraph(n_hist, ff, rms[0] );
      gr->SetTitle("BeamProfile X (RMS)");
      gr->GetXaxis()->SetTitle("FF+[mm]");
      gr->GetYaxis()->SetTitle("[mm]  ");
      gr->GetYaxis()->SetTitleOffset(1.2);
      gr->SetMarkerStyle(8);
      gr->SetMarkerSize(3);
      gr->SetMarkerColor(kRed);
      gr->Draw("AP");
      c->cd(2)->SetGrid();
      TGraph *gs = new TGraph(n_hist, ff, sigma[0] );
      gs->SetTitle("BeamProfile X (sigma)");
      gs->GetXaxis()->SetTitle("FF+[mm]");
      gs->GetYaxis()->SetTitle("[mm]  ");
      gs->GetYaxis()->SetTitleOffset(1.2);
      gs->SetMarkerStyle(8);
      gs->SetMarkerSize(3);
      gs->SetMarkerColor(kBlue);
      gs->Draw("AP");
    }
    {
      c->cd(3)->SetGrid();
      TGraph *gr = new TGraph(n_hist, ff, rms[1] );
      gr->SetTitle("BeamProfile Y (RMS)");
      gr->GetXaxis()->SetTitle("FF+[mm]");
      gr->GetYaxis()->SetTitle("[mm]  ");
      gr->GetYaxis()->SetTitleOffset(1.2);
      gr->SetMarkerStyle(8);
      gr->SetMarkerSize(3);
      gr->SetMarkerColor(kRed);
      gr->Draw("AP");
      c->cd(4)->SetGrid();
      TGraph *gs = new TGraph(n_hist, ff, sigma[1] );
      gs->SetTitle("BeamProfile Y (sigma)");
      gs->GetXaxis()->SetTitle("FF+[mm]");
      gs->GetYaxis()->SetTitle("[mm]  ");
      gs->GetYaxis()->SetTitleOffset(1.2);
      gs->SetMarkerStyle(8);
      gs->SetMarkerSize(3);
      gs->SetMarkerColor(kBlue);
      gs->Draw("AP");
    }
    c->Update();
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
