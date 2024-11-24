// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;
#include<algorithm>

void dispBeamProfile_e42_2021()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const int n_hist = 6;
  TLatex tex;
  tex.SetNDC();
  tex.SetTextSize(0.14);
  double xpos = 0.15, ypos = 0.75;

  bool flag_line[2][n_hist] =
    {
      {false, false, false, false, false, false},
      {false, false, false, false, false, false}
    };

  double pos_line_x[n_hist][2] =
    {
      {-60, 60}, {-39, 39}, {-20, 20}, {-20, 20}, {-20, 20}, {-30, 30}
    };

  double pos_line_y[n_hist][2] =
    {
      {-45, 45}, {-27, 27}, {-20, 20}, {-20, 20}, {-20, 20}, {-15, 15},
    };

  const double fit_width[2] = { 50., 50. };

  const double ff[n_hist] = {-400, -200, 0, 200, 400, 600};
  double   rms[2][n_hist];
  double sigma[2][n_hist];

  double   mean[2][n_hist];
  double gaus_mean[2][n_hist];

  // Return to the initial position
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
    c->cd();
  }

  //gROOT->SetStyle("Plain");
  //TPDF *pdf = new TPDF("/home/sks/PSFile/e40_2017/beamprofile_e40.pdf", 112);

  // Title
  // {
  //   //    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  //   TCanvas *c = new TCanvas("c0", "c0");
  //   //    pdf->NewPage();
  //   c->UseCurrentStyle();
  //   c->Divide(1,1);
  //   c->cd(1);
  //   c->GetPad(1)->Range(0,0,100,100);
  //   TText text;
  //   text.SetTextSize(0.2);
  //   text.SetTextAlign(22);
  //   TTimeStamp stamp;
  //   stamp.Add( -stamp.GetZoneOffset() );
  //   text.SetTextSize(0.1);
  //   text.DrawText(50.,50., stamp.AsString("s") );
  //   c->Update();
  //   c->cd();
  //   c->GetPad(1)->Close();
  //   delete c;
  //   c = NULL;
  // }

  // X position (dispRMS)
  {
    TCanvas *c = dynamic_cast<TCanvas*>(gROOT->FindObject("c1"));
    c->Clear();
    c->UseCurrentStyle();
    c->Divide(4,2);

    TLine *line[n_hist][2];
    int base_id = HistMaker::getUniqueID(kMisc, 0, kHitPat);
    for(int i=0; i<n_hist; i++){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get(base_id +i)->Clone();
      h->GetXaxis()->SetRangeUser(-100,100);
      rms[0][i] = h->GetRMS();
      mean[0][i] = h->GetMean();
      h->Draw();
      tex.DrawLatex(xpos, ypos, Form("%.2lf", rms[0][i]));
      double ymax = h->GetMaximum();
      for(int j = 0; j<2; j++){
	line[i][j] = new TLine(pos_line_x[i][j], 0,
			       pos_line_x[i][j], ymax
			       );
	line[i][j]->SetLineColor(4);
	line[i][j]->SetLineStyle(2);
	line[i][j]->SetLineWidth(2);
	if(flag_line[0][i]) line[i][j]->Draw();
      }// for(j)
    }// for(i)
    c->Update();
    //    c->Print("~/PSFile/pro/beamprofile_e40.ps(");
  }
  // Y position (dispRMS)
  {
    TCanvas *c = dynamic_cast<TCanvas*>(gROOT->FindObject("c2"));
    c->Clear();
    c->UseCurrentStyle();
    c->Divide(4,2);

    TLine *line[n_hist][2];
    int base_id = HistMaker::getUniqueID(kMisc, 0, kHitPat);
    for(int i=0; i<n_hist; i++){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get(base_id +i + n_hist)->Clone();
      h->GetXaxis()->SetRangeUser(-60,60);
      rms[1][i] = h->GetRMS();
      mean[1][i] = h->GetMean();
      h->Draw();
      tex.DrawLatex(xpos, ypos, Form("%.2lf", rms[1][i]));
      double ymax = h->GetMaximum();
      for(int j = 0; j<2; j++){
	line[i][j] = new TLine(pos_line_y[i][j], 0,
			       pos_line_y[i][j], ymax
			       );
	line[i][j]->SetLineColor(4);
	line[i][j]->SetLineStyle(2);
	line[i][j]->SetLineWidth(2);
	if(flag_line[1][i]) line[i][j]->Draw();
      }// for(j)
    }// for(i)
    c->Update();
    //    c->Print("~/PSFile/pro/beamprofile_e40.ps");
  }
  // X position (dispSigma)
  {
    TCanvas *c = dynamic_cast<TCanvas*>(gROOT->FindObject("c3"));
    c->Clear();
    c->UseCurrentStyle();
    c->Divide(4,2);
    int base_id = HistMaker::getUniqueID(kMisc, 0, kHitPat);
    TF1 *f = new TF1("f", "gaus");
    f->SetLineColor(kBlue);
    for(int i=0; i<n_hist; i++){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get(base_id +i)->Clone();
      h->GetXaxis()->SetRangeUser(-100,100);
      // double max = h->GetBinCenter(h->GetMaximumBin());
      double max = 0.;
      h->Fit("f", "Q", "", max-fit_width[0], max+fit_width[0]);
      sigma[0][i] = f->GetParameter("Sigma");
      gaus_mean[0][i] = f->GetParameter("Mean");
      h->Draw();
      tex.DrawLatex(xpos, ypos, Form("%.2lf", sigma[0][i]));
    }// for(i)
    c->Update();
    //    c->Print("~/PSFile/pro/beamprofile_e40.ps");
  }
  // Y position (dispSigma)
  {
    TCanvas *c = dynamic_cast<TCanvas*>(gROOT->FindObject("c4"));
    c->Clear();
    c->UseCurrentStyle();
    c->Divide(4,2);
    int base_id = HistMaker::getUniqueID(kMisc, 0, kHitPat);
    TF1 *f = new TF1("f", "gaus");
    f->SetLineColor(kBlue);
    for(int i=0; i<n_hist; i++){
      c->cd(i+1);
      TH1 *h = (TH1*)GHist::get(base_id +i +n_hist)->Clone();
      h->GetXaxis()->SetRangeUser(-60,60);
      // double max = h->GetBinCenter(h->GetMaximumBin());
      double max = 0.;
      h->Fit("f", "Q", "", max-fit_width[1], max+fit_width[1]);
      sigma[1][i] = f->GetParameter("Sigma");
      gaus_mean[1][i] = f->GetParameter("Mean");
      h->Draw();
      tex.DrawLatex(xpos, ypos, Form("%.2lf", sigma[1][i]));
    }// for(i)
    c->Update();
    //    c->Print("~/PSFile/pro/beamprofile_e40.ps");
  }
  // X(RMS)%FFpos & X(sigma)%FFpos
  {
    TCanvas *c = dynamic_cast<TCanvas*>(gROOT->FindObject("c5"));
    c->Clear();
    c->UseCurrentStyle();
    c->Divide(2,2);
    {
      // X (RMS, sigma)
      double ymax_rms = *std::max_element(rms[0], rms[0] + n_hist-1);
      double ymax_gaus = *std::max_element(sigma[0], sigma[0] + n_hist-1);
      double ymax = ymax_rms > ymax_gaus? ymax_rms : ymax_gaus;

      double ymin_rms = *std::min_element(rms[0], rms[0] + n_hist-1);
      double ymin_gaus = *std::min_element(sigma[0], sigma[0] + n_hist-1);
      double ymin = ymin_rms < ymin_gaus? ymin_rms : ymin_gaus;

      c->cd(1)->SetGrid();
      TGraph *gr = new TGraph(n_hist, ff, rms[0] );
      gr->SetTitle("BeamProfile X (RMS=Red, Sigma=Blue)");
      gr->GetXaxis()->SetTitle("From FF [mm]");
      gr->GetYaxis()->SetTitle("[mm]  ");
      gr->GetYaxis()->SetTitleOffset(1.2);
      gr->SetMarkerStyle(8);
      gr->SetMarkerSize(2);
      gr->SetMarkerColor(kRed);
      gr->SetMaximum(ymax+0.1*fabs(ymax));
      gr->SetMinimum(ymin-0.1*fabs(ymin));
      gr->Draw("AP");
      TGraph *gs = new TGraph(n_hist, ff, sigma[0] );
      gs->SetTitle("BeamProfile X (sigma)");
      gs->GetXaxis()->SetTitle("From FF [mm]");
      gs->GetYaxis()->SetTitle("[mm]  ");
      gs->GetYaxis()->SetTitleOffset(1.2);
      gs->SetMarkerStyle(8);
      gs->SetMarkerSize(2);
      gs->SetMarkerColor(kBlue);
      gs->Draw("P");
      TLine *l1 = new TLine(0,-100,0,100);
      l1->SetLineColor(kBlue);
      l1->SetLineStyle(2);
      l1->SetLineWidth(2);
      l1->Draw("same");
    }
    {
      // Y (RMS, sigma)
      double ymax_rms = *std::max_element(rms[1], rms[1] + n_hist-1);
      double ymax_gaus = *std::max_element(sigma[1], sigma[1] + n_hist-1);
      double ymax = ymax_rms > ymax_gaus? ymax_rms : ymax_gaus;

      double ymin_rms = *std::min_element(rms[1], rms[1] + n_hist-1);
      double ymin_gaus = *std::min_element(sigma[1], sigma[1] + n_hist-1);
      double ymin = ymin_rms < ymin_gaus? ymin_rms : ymin_gaus;


      c->cd(2)->SetGrid();
      TGraph *gr = new TGraph(n_hist, ff, rms[1] );
      gr->SetTitle("BeamProfile Y (RMS=Red, Sigma=Blue)");
      gr->GetXaxis()->SetTitle("From FF [mm]");
      gr->GetYaxis()->SetTitle("[mm]  ");
      gr->GetYaxis()->SetTitleOffset(1.2);
      gr->SetMarkerStyle(8);
      gr->SetMarkerSize(2);
      gr->SetMarkerColor(kRed);
      gr->SetMaximum(ymax+0.1*fabs(ymax));
      gr->SetMinimum(ymin-0.1*fabs(ymin));
      gr->Draw("AP");
      TGraph *gs = new TGraph(n_hist, ff, sigma[1] );
      gs->SetTitle("BeamProfile Y (sigma)");
      gs->GetXaxis()->SetTitle("From FF [mm]");
      gs->GetYaxis()->SetTitle("[mm]  ");
      gs->GetYaxis()->SetTitleOffset(1.2);
      gs->SetMarkerStyle(8);
      gs->SetMarkerSize(2);
      gs->SetMarkerColor(kBlue);
      gs->Draw("P");
      TLine *l1 = new TLine(0,-100,0,100);
      l1->SetLineColor(kBlue);
      l1->SetLineStyle(2);
      l1->SetLineWidth(2);
      l1->Draw("same");
    }
    {
      // X (Mean)
      double ymax_mean = *std::max_element(mean[0], mean[0] + n_hist-1);
      double ymax_gaus = *std::max_element(gaus_mean[0], gaus_mean[1] + n_hist-1);
      double ymax = ymax_mean > ymax_gaus? ymax_mean : ymax_gaus;

      double ymin_mean = *std::min_element(mean[0], mean[0] + n_hist-1);
      double ymin_gaus = *std::min_element(gaus_mean[0], gaus_mean[0] + n_hist-1);
      double ymin = ymin_mean < ymin_gaus? ymin_mean : ymin_gaus;

      c->cd(3)->SetGrid();
      TGraph *gm = new TGraph(n_hist, ff, mean[0] );
      gm->SetTitle("BeamProfile X (Mean=Red, Gaus Mean=Blue)");
      gm->GetXaxis()->SetTitle("From FF [mm]");
      gm->GetYaxis()->SetTitle("[mm]  ");
      gm->GetYaxis()->SetTitleOffset(1.2);
      gm->SetMarkerStyle(8);
      gm->SetMarkerSize(2);
      gm->SetMarkerColor(kRed);
      gm->SetMaximum(ymax+0.1*fabs(ymax)+0.5);
      gm->SetMinimum(ymin-0.1*fabs(ymin)-0.5);
      gm->Draw("AP");
      TGraph *ggm = new TGraph(n_hist, ff, gaus_mean[0] );
      ggm->SetTitle("BeamProfile X (Mean, Gaus Mean)");
      ggm->GetXaxis()->SetTitle("From FF [mm]");
      ggm->GetYaxis()->SetTitle("[mm]  ");
      ggm->GetYaxis()->SetTitleOffset(1.2);
      ggm->SetMarkerStyle(8);
      ggm->SetMarkerSize(2);
      ggm->SetMarkerColor(kBlue);
      ggm->Draw("P");
      TLine *l1 = new TLine(0,-100,0,100);
      l1->SetLineColor(kBlue);
      l1->SetLineStyle(2);
      l1->SetLineWidth(2);
      l1->Draw("same");
    }
    {
      // Y (Mean)
      c->cd(4)->SetGrid();
      double ymax_mean = *std::max_element(mean[1], mean[1] + n_hist-1);
      double ymax_gaus = *std::max_element(gaus_mean[1], gaus_mean[1] + n_hist-1);
      double ymax = ymax_mean > ymax_gaus? ymax_mean : ymax_gaus;

      double ymin_mean = *std::min_element(mean[1], mean[1] + n_hist-1);
      double ymin_gaus = *std::min_element(gaus_mean[1], gaus_mean[1] + n_hist-1);
      double ymin = ymin_mean < ymin_gaus? ymin_mean : ymin_gaus;


      TGraph *gm = new TGraph(n_hist, ff, mean[1] );
      gm->SetTitle("BeamProfile Y (Mean=Red, Gaus Mean=Blue)");
      gm->GetXaxis()->SetTitle("From FF [mm]");
      gm->GetYaxis()->SetTitle("[mm]  ");
      gm->GetYaxis()->SetTitleOffset(1.2);
      gm->SetMarkerStyle(8);
      gm->SetMarkerSize(2);
      gm->SetMarkerColor(kRed);
      gm->SetMaximum(ymax+0.1*fabs(ymax)+0.5);
      gm->SetMinimum(ymin-0.1*fabs(ymin)-0.5);
      gm->Draw("AP");
      TGraph *ggm = new TGraph(n_hist, ff, gaus_mean[1] );
      ggm->SetTitle("BeamProfile Y (Mean=Red, Gaus Mean=Blue)");
      ggm->GetXaxis()->SetTitle("From FF [mm]");
      ggm->GetYaxis()->SetTitle("[mm]  ");
      ggm->GetYaxis()->SetTitleOffset(1.2);
      ggm->SetMarkerStyle(8);
      ggm->SetMarkerSize(2);
      ggm->SetMarkerColor(kBlue);
      ggm->Draw("P");
      TLine *l1 = new TLine(0,-100,0,100);
      l1->SetLineColor(kBlue);
      l1->SetLineStyle(2);
      l1->SetLineWidth(2);
      l1->Draw("same");
    }

    c->cd(0);
    c->Update();
    //    c->Print("~/PSFile/pro/beamprofile_e40.ps");
    //    c->Print("~/PSFile/pro/beamprofile_e40.ps)");
  }

  //pdf->Close();

  gROOT->SetStyle("Classic");

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
