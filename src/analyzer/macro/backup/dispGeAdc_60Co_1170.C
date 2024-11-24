// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispGeAdc_60Co_1170()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  gStyle->SetStatH(.45);
  const int NumOfSegGe = 16;
  const int hmin = 5200, hmax = 5800;

  double seg[NumOfSegGe]  = {};
  double FWHM[NumOfSegGe]  = {};
  double eFWHM[NumOfSegGe] = {};

  TF1 *f0 = new TF1("f0", "gaus");
  TF1 *f1 = new TF1("f1", "gaus(0) + pol1(3)");
  f0->SetLineColor(kRed); f1->SetLineColor(kGreen);

  TLatex t_name, t_val;
  t_name.SetNDC(); t_val.SetNDC();
  t_name.SetTextSize(0.07); t_val.SetTextSize(0.1);

  ////////// Ge ADC at 1.17MeV
  {
    const double width = 20.;
    // draw ADC

      TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
      c->Clear();
      c->Divide(4,4);
      int base_id = HistMaker::getUniqueID(kGe, 0, kADC, 1);
      for(int i=0; i<NumOfSegGe; i++){
	seg[i] = i;

	c->cd(i+1);
	//gPad->SetLogy();
	TH1 *h = (TH1*)GHist::get(base_id + i)->Clone();
	h->GetXaxis()->SetRangeUser(hmin, hmax);
	double center = h->GetBinCenter(h->GetMaximumBin());
	double param[5] = { 2., center, 2., 0., 0. };
	h->Fit("f0", "Q", "", center-width, center+width);
	f0->GetParameters(param);
	FWHM[i]  = f0->GetParameter(2) * 2.35482;
	eFWHM[i] = f0->GetParError(2)  * 2.35482;
	double min = param[1] - param[2]*20.;
	double max = param[1] + param[2]*20.;
	f1->SetParameters(param);
	f1->SetParLimits(0, 0., 10000000.);
	f1->SetParLimits(1, hmin, hmax);
	f1->SetParLimits(2, 0., 100.);
	for(int it=0; it<5; it++){
	  if(param[2]<20.){
	    h->Fit("f1", "Q", "", min, max);
	    f1->GetParameters(param);
	    FWHM[i]  = f1->GetParameter(2) * 2.35482;
	    eFWHM[i] = f1->GetParError(2)  * 2.35482;
	    min = param[1] - param[2]*20.;
	    max = param[1] + param[2]*20.;
	  }
	}
	h->GetXaxis()->SetRangeUser(param[1]-width*1.5, param[1]+width*2.5);
	if(h->GetEntries()>0){
	  t_name.DrawLatex(0.53, 0.56, "FWHM@1.17MeV");
	  t_val.DrawLatex(0.54, 0.44, Form("%.2lf #pm %.2lf",
 					   FWHM[i], eFWHM[i]));
	}else{
	  FWHM[i] = -1.; eFWHM[i] = 0.;
	}
      }
      c->Update();
  }

  ////////// Ge ADC FWHM % Slot
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
    c->Clear();
    c->SetGrid();
    gStyle->SetEndErrorSize(0);
    TGraph *gr = new TGraphErrors(NumOfSegGe, seg, FWHM, 0, eFWHM );
    gr->SetTitle("Ge ADC @ 1170keV (60Co)");
    gr->GetXaxis()->SetTitle("seg");
    gr->GetXaxis()->SetRangeUser(0, NumOfSegGe+1);
    gr->GetYaxis()->SetTitle("FWHM [ch]  ");
    gr->GetYaxis()->SetTitleOffset(1.2);
    gr->GetYaxis()->SetRangeUser(0, 19);
    gr->SetMarkerStyle(8);
    gr->SetMarkerSize(4);
    gr->SetMarkerColor(kGreen);
    gr->SetLineWidth(4);
    gr->Draw("AP");
    c->Update();
  }

  const int hmin2 = 5900, hmax2 = 6500;
  ////////// Ge ADC at 1.33MeV
  {
    const double width = 20.;
    // draw ADC

      TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
      c->Clear();
      c->Divide(4,4);
      int base_id = HistMaker::getUniqueID(kGe, 0, kADC, 1);
      for(int i=0; i<NumOfSegGe; i++){
	seg[i] = i;

	c->cd(i+1);
	//gPad->SetLogy();
	TH1 *h = (TH1*)GHist::get(base_id + i)->Clone();
	h->GetXaxis()->SetRangeUser(hmin2, hmax2);
	double center = h->GetBinCenter(h->GetMaximumBin());
	double param[5] = { 2., center, 2., 0., 0. };
	h->Fit("f0", "Q", "", center-width, center+width);
	f0->GetParameters(param);
	FWHM[i]  = f0->GetParameter(2) * 2.35482;
	eFWHM[i] = f0->GetParError(2)  * 2.35482;
	double min = param[1] - param[2]*20.;
	double max = param[1] + param[2]*20.;
	f1->SetParameters(param);
	f1->SetParLimits(0, 0., 10000000.);
	f1->SetParLimits(1, hmin2, hmax2);
	f1->SetParLimits(2, 0., 100.);
	for(int it=0; it<5; it++){
	  if(param[2]<20.){
	    h->Fit("f1", "Q", "", min, max);
	    f1->GetParameters(param);
	    FWHM[i]  = f1->GetParameter(2) * 2.35482;
	    eFWHM[i] = f1->GetParError(2)  * 2.35482;
	    min = param[1] - param[2]*20.;
	    max = param[1] + param[2]*20.;
	  }
	}
	h->GetXaxis()->SetRangeUser(param[1]-width*1.5, param[1]+width*2.5);
	if(h->GetEntries()>0){
	  t_name.DrawLatex(0.53, 0.56, "FWHM@1.33MeV");
	  t_val.DrawLatex(0.54, 0.44, Form("%.2lf #pm %.2lf",
 					   FWHM[i], eFWHM[i]));
	}else{
	  FWHM[i] = -1.; eFWHM[i] = 0.;
	}
      }
      c->Update();
  }

  ////////// Ge ADC FWHM % Slot
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
    c->Clear();
    c->SetGrid();
    gStyle->SetEndErrorSize(0);
    TGraph *gr = new TGraphErrors(NumOfSegGe, seg, FWHM, 0, eFWHM );
    gr->SetTitle("Ge ADC @ 1330keV (60Co)");
    gr->GetXaxis()->SetTitle("seg");
    gr->GetXaxis()->SetRangeUser(0, NumOfSegGe+1);
    gr->GetYaxis()->SetTitle("FWHM [ch]  ");
    gr->GetYaxis()->SetTitleOffset(1.2);
    gr->GetYaxis()->SetRangeUser(0, 19);
    gr->SetMarkerStyle(8);
    gr->SetMarkerSize(4);
    gr->SetMarkerColor(kGreen);
    gr->SetLineWidth(4);
    gr->Draw("AP");
    c->Update();
  }


  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
