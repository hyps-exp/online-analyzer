// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispGeAdc_60Co_1330()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  gStyle->SetStatH(.45);
  const int n_seg = 32;
  const int hmin = 1350, hmax = 1550;

  double slot[n_seg]  = {};
  double FWHM[n_seg]  = {};
  double eFWHM[n_seg] = {};

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
    for(int ud=0; ud<2; ud++){
      TCanvas *c = (TCanvas*)gROOT->FindObject(Form("c%d", ud+1));
      c->Clear();
      c->Divide(4,4);
      int base_id = HistMaker::getUniqueID(kGe, 0, kADC, 1);
      for(int i=0; i<n_seg/2; i++){
	int seg = i + ud*n_seg/2;
	slot[seg] = seg + 1;
	c->cd(i+1);
	//gPad->SetLogy();
	TH1 *h = (TH1*)GHist::get(base_id +seg)->Clone();
	h->GetXaxis()->SetRangeUser(hmin, hmax);
	double center = h->GetBinCenter(h->GetMaximumBin());
	double param[5] = { 2., center, 2., 0., 0. };
	h->Fit("f0", "Q", "", center-width, center+width);
	f0->GetParameters(param);
	FWHM[seg]  = f0->GetParameter(2) * 2.35482;
	eFWHM[seg] = f0->GetParError(2)  * 2.35482;
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
	    FWHM[seg]  = f1->GetParameter(2) * 2.35482;
	    eFWHM[seg] = f1->GetParError(2)  * 2.35482;
	    min = param[1] - param[2]*20.;
	    max = param[1] + param[2]*20.;
	  }
	}
	h->GetXaxis()->SetRangeUser(param[1]-width*1.5, param[1]+width*2.5);
	if(h->GetEntries()>0){
	  t_name.DrawLatex(0.53, 0.56, "FWHM@1.33MeV");
	  t_val.DrawLatex(0.54, 0.44, Form("%.2lf #pm %.2lf",
					   FWHM[seg], eFWHM[seg]));
	}else{
	  FWHM[seg] = -1.; eFWHM[seg] = 0.;
	}
      }
      c->Update();
    }
  }
  ////////// Ge ADC FWHM % Slot
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
    c->Clear();
    c->SetGrid();
    gStyle->SetEndErrorSize(0);
    TGraph *gr = new TGraphErrors(n_seg, slot, FWHM, 0, eFWHM );
    gr->SetTitle("Ge ADC @ 1330keV (60Co)");
    gr->GetXaxis()->SetTitle("slot");
    gr->GetXaxis()->SetRangeUser(0, n_seg+1);
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
