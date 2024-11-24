// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispBeamProfile_e05()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const int n_hist = 5;
  TLatex tex;
  tex.SetNDC();
  tex.SetTextSize(0.14);
  double xpos = 0.15, ypos = 0.75;

  const double fit_width[2] = { 100., 100. };

  const double ff[n_hist] = { 200., 400., 580., 800., 1000.
			      /*-100, 100*/ };
  double   rms[2][n_hist];
  double sigma[2][n_hist];
    
  // XY position (dispRMS)
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(5,2);
    int base_id = HistMaker::getUniqueID(kMisc, 0, kHitPat);
    for(int xy=0; xy<2; xy++){
      for(int i=0; i<n_hist; i++){
	c->cd(i+1+xy*n_hist);
	TH1 *h = (TH1*)GHist::get(base_id +i +xy*n_hist)->Clone();
	h->GetXaxis()->SetRangeUser(-200,200);
	rms[xy][i] = h->GetRMS();
	h->Draw();
	tex.DrawLatex(xpos, ypos, Form("%.2lf", rms[xy][i]));
      }
    }
    c->Update();
  }
  // XY position (dispSigma)
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->Divide(5,2);
    int base_id = HistMaker::getUniqueID(kMisc, 0, kHitPat);
    TF1 *f = new TF1("f", "gaus");
    f->SetLineColor(kBlue);
    for(int xy=0; xy<2; xy++){
      for(int i=0; i<n_hist; i++){
	c->cd(i+1+xy*n_hist);
	TH1 *h = (TH1*)GHist::get(base_id +i +xy*n_hist)->Clone();
	h->GetXaxis()->SetRangeUser(-200,200);
	double max = h->GetBinCenter(h->GetMaximumBin());
	h->Fit("f", "Q", "", max-fit_width[xy], max+fit_width[xy]);
	sigma[xy][i] = f->GetParameter("Sigma");
	h->Draw();
	tex.DrawLatex(xpos, ypos, Form("%.2lf", sigma[xy][i]));
      }    
    }
    c->Update();
  }
  // X(RMS)%FFpos & X(sigma)%FFpos
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->Divide(2,1);
    {
      c->cd(1)->SetGrid();
      TGraph *gr = new TGraph(n_hist, ff, rms[0] );
      gr->SetTitle("BeamProfile X (RMS)");
      gr->GetXaxis()->SetTitle("FF+[mm]");
      gr->GetYaxis()->SetTitle("[mm]  ");
      gr->GetYaxis()->SetTitleOffset(1.2);
      gr->SetMarkerStyle(8);
      gr->SetMarkerSize(4);
      gr->SetMarkerColor(kRed);
      gr->Draw("AP");
      c->cd(2)->SetGrid();
      TGraph *gs = new TGraph(n_hist, ff, sigma[0] );
      gs->SetTitle("BeamProfile X (sigma)");
      gs->GetXaxis()->SetTitle("FF+[mm]");
      gs->GetYaxis()->SetTitle("[mm]  ");
      gs->GetYaxis()->SetTitleOffset(1.2);
      gs->SetMarkerStyle(8);
      gs->SetMarkerSize(4);
      gs->SetMarkerColor(kBlue);
      gs->Draw("AP");
    }
    c->Update();
  }
  // Y(RMS)%FFpos & Y(sigma)%FFpos
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
    c->Clear();
    c->Divide(2,1);
    {
      c->cd(1)->SetGrid();
      TGraph *gr = new TGraph(n_hist, ff, rms[1] );
      gr->SetTitle("BeamProfile Y (RMS)");
      gr->GetXaxis()->SetTitle("FF+[mm]");
      gr->GetYaxis()->SetTitle("[mm]  ");
      gr->GetYaxis()->SetTitleOffset(1.2);
      gr->SetMarkerStyle(8);
      gr->SetMarkerSize(4);
      gr->SetMarkerColor(kRed);
      gr->Draw("AP");
      c->cd(2)->SetGrid();
      TGraph *gs = new TGraph(n_hist, ff, sigma[1] );
      gs->SetTitle("BeamProfile Y (sigma)");
      gs->GetXaxis()->SetTitle("FF+[mm]");
      gs->GetYaxis()->SetTitle("[mm]  ");
      gs->GetYaxis()->SetTitleOffset(1.2);
      gs->SetMarkerStyle(8);
      gs->SetMarkerSize(4);
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
