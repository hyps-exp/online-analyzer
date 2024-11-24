// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispE03targeting_y2020()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  gStyle->SetOptStat(0);
  
  TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  c->Clear();
  c->Divide(2,2);


  c->cd(1);
  //gPad->SetLogy();
  {
    TH1 *h = (TH1*)GHist::get(HistMaker::getUniqueID(kMisc, 0, kTDC)+12);
    h->GetXaxis()->SetRangeUser(-150,150);
    h->Draw();
    TH1 *hpi = (TH1*)GHist::get(HistMaker::getUniqueID(kMisc, 0, kTDC)+14);
    hpi->SetLineColor( kBlue );
    hpi->Draw("same");
    TH1 *hk = (TH1*)GHist::get(HistMaker::getUniqueID(kMisc, 0, kTDC)+16);
    hk->SetLineColor( kGreen );
    hk->Draw("same");
    TH1 *hp = (TH1*)GHist::get(HistMaker::getUniqueID(kMisc, 0, kTDC)+18);
    hp->SetLineColor( kRed );
    hp->Draw("same");
    TText *t1 = new TText(0.7,0.85,"Blue: pion");
    t1->SetNDC(1);
    t1->SetTextColor(kBlue);
    t1->SetTextSize(0.04);
    t1->Draw("same");
    TText *t2 = new TText(0.7,0.78,"Green: kaon");
    t2->SetNDC(1);
    t2->SetTextColor(kGreen);
    t2->SetTextSize(0.04);
    t2->Draw("same");
    TText *t3 = new TText(0.7,0.71,"Red: p-bar");
    t3->SetNDC(1);
    t3->SetTextColor(kRed);
    t3->SetTextSize(0.04);
    t3->Draw("same");
    TLine *l1 = new TLine(-30,0,-30,1000000);
    l1->SetLineColor(kBlue);
    l1->SetLineStyle(2);
    l1->SetLineWidth(2);
    l1->Draw("same");
    TLine *l2 = new TLine(30,0,30,1000000);
    l2->SetLineColor(kBlue);
    l2->SetLineStyle(2);
    l2->SetLineWidth(2);
    l2->Draw("same");
  }

  c->cd(3);
  //gPad->SetLogy();
  {
    TH1 *h = (TH1*)GHist::get(HistMaker::getUniqueID(kMisc, 0, kTDC)+13);
    h->GetXaxis()->SetRangeUser(-50,50);
    h->Draw();
    TH1 *hpi = (TH1*)GHist::get(HistMaker::getUniqueID(kMisc, 0, kTDC)+15);
    hpi->SetLineColor( kBlue );
    hpi->Draw("same");
    TH1 *hk = (TH1*)GHist::get(HistMaker::getUniqueID(kMisc, 0, kTDC)+17);
    hk->SetLineColor( kGreen );
    hk->Draw("same");
    TH1 *hp = (TH1*)GHist::get(HistMaker::getUniqueID(kMisc, 0, kTDC)+19);
    hp->SetLineColor( kRed );
    hp->Draw("same");
    TText *t1 = new TText(0.7,0.85,"Blue: pion");
    t1->SetNDC(1);
    t1->SetTextColor(kBlue);
    t1->SetTextSize(0.04);
    t1->Draw("same");
    TText *t2 = new TText(0.7,0.78,"Green: kaon");
    t2->SetNDC(1);
    t2->SetTextColor(kGreen);
    t2->SetTextSize(0.04);
    t2->Draw("same");
    TText *t3 = new TText(0.7,0.71,"Red: p-bar");
    t3->SetNDC(1);
    t3->SetTextColor(kRed);
    t3->SetTextSize(0.04);
    t3->Draw("same");
    TLine *l1 = new TLine(-7,0,-7,1000000);
    l1->SetLineColor(kBlue);
    l1->SetLineStyle(2);
    l1->SetLineWidth(2);
    l1->Draw("same");
    TLine *l2 = new TLine(7,0,7,1000000);
    l2->SetLineColor(kBlue);
    l2->SetLineStyle(2);
    l2->SetLineWidth(2);
    l2->Draw("same");
  }

  c->cd(2);
  //gPad->SetLogy();
  {
    TH2 *hpi = (TH2*)GHist::get(HistMaker::getUniqueID(kMisc, 0, kTDC)+20);
    hpi->GetXaxis()->SetRangeUser(-70,70);
    hpi->SetMarkerColor( kBlue );
    hpi->Draw("");
    TH2 *hk = (TH2*)GHist::get(HistMaker::getUniqueID(kMisc, 0, kTDC)+21);
    hk->SetMarkerColor( kGreen );
    hk->Draw("same");
    TH2 *hp = (TH2*)GHist::get(HistMaker::getUniqueID(kMisc, 0, kTDC)+22);
    hp->SetMarkerColor( kRed );
    hp->Draw("same");
    TText *t1 = new TText(0.7,0.85,"Blue: pion");
    t1->SetNDC(1);
    t1->SetTextColor(kBlue);
    t1->SetTextSize(0.04);
    t1->Draw("same");
    TText *t2 = new TText(0.7,0.78,"Green: kaon");
    t2->SetNDC(1);
    t2->SetTextColor(kGreen);
    t2->SetTextSize(0.04);
    t2->Draw("same");
    TText *t3 = new TText(0.7,0.71,"Red: p-bar");
    t3->SetNDC(1);
    t3->SetTextColor(kRed);
    t3->SetTextSize(0.04);
    t3->Draw("same");
    TLine *l1 = new TLine(-30,-7,-30,7);
    l1->SetLineColor(kBlue);
    l1->SetLineStyle(2);
    l1->SetLineWidth(2);
    l1->Draw("same");
    TLine *l2 = new TLine(30,-7,30,7);
    l2->SetLineColor(kBlue);
    l2->SetLineStyle(2);
    l2->SetLineWidth(2);
    l2->Draw("same");
    TLine *l3 = new TLine(-30,7,30,7);
    l3->SetLineColor(kBlue);
    l3->SetLineStyle(2);
    l3->SetLineWidth(2);
    l3->Draw("same");
    TLine *l4 = new TLine(-30,-7,30,-7);
    l4->SetLineColor(kBlue);
    l4->SetLineStyle(2);
    l4->SetLineWidth(2);
    l4->Draw("same");

    //check ratio: kaon in target//
    { // pi
      int Entry = hpi->GetEntries();
      int Integral = hpi->Integral( hpi->GetXaxis()->FindBin(-30),
				    hpi->GetXaxis()->FindBin(30),
				    hpi->GetYaxis()->FindBin(-7),
				    hpi->GetYaxis()->FindBin(7) );
      double ratio = ((double)Integral/(double)Entry)*100;
      std::stringstream oss;
      oss.str("");
      oss << "Inside TGT [pi]: " << std::fixed << std::setprecision(2) << ratio << "%";
      TText *tr = new TText(0.13,0.22,oss.str().c_str());
      tr->SetNDC(1);
      tr->SetTextColor(kBlue);
      tr->SetTextSize(0.05);
      tr->Draw("same");
    }
    { // k
      int Entry = hk->GetEntries();
      int Integral = hk->Integral( hk->GetXaxis()->FindBin(-30),
				   hk->GetXaxis()->FindBin(30),
				   hk->GetYaxis()->FindBin(-7),
				   hk->GetYaxis()->FindBin(7) );
      double ratio = ((double)Integral/(double)Entry)*100;
      std::stringstream oss;
      oss.str("");
      oss << "Inside TGT [k]: " << std::fixed << std::setprecision(2) << ratio << "%";
      TText *tr = new TText(0.13,0.15,oss.str().c_str());
      tr->SetNDC(1);
      tr->SetTextColor(kGreen);
      tr->SetTextSize(0.05);
      tr->Draw("same");
    }
  }

  c->cd(4);
  {
    TH1 *h = (TH1*)GHist::get(HistMaker::getUniqueID(kMisc,  0, kTDC)+0);
    h->GetXaxis()->SetRangeUser(-6,2);
    h->Draw();
    TH1 *hpi = (TH1*)GHist::get(HistMaker::getUniqueID(kMisc, 0, kTDC)+1);
    hpi->SetLineColor( kBlue );
    hpi->Draw("same");
    TH1 *hk = (TH1*)GHist::get(HistMaker::getUniqueID(kMisc, 0, kTDC)+2);
    hk->SetLineColor( kGreen );
    hk->Draw("same");
    TH1 *hp = (TH1*)GHist::get(HistMaker::getUniqueID(kMisc, 0, kTDC)+3);
    hp->SetLineColor( kRed );
    hp->Draw("same");
    TLatex *t1 = new TLatex(0.72,0.86,"#pi");
    t1->SetNDC(1);
    t1->SetTextColor(kBlue);
    t1->SetTextSize(0.04);
    t1->Draw("same");
    TText *t2 = new TText(0.55,0.86,"K");
    t2->SetNDC(1);
    t2->SetTextColor(kGreen);
    t2->SetTextSize(0.04);
    t2->Draw("same");
    TText *t3 = new TText(0.20,0.86,"p-bar");
    t3->SetNDC(1);
    t3->SetTextColor(kRed);
    t3->SetTextSize(0.04);
    t3->Draw("same");
  }
 

  c->Update();

  c->cd(0);

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
