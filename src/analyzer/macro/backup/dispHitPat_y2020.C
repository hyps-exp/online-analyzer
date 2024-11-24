// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispHitPat_y2020()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // Return to the initial position
  //{
  //  TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
  //  c->cd();
  //}

  gROOT->SetStyle("Plain");
  // TPDF *pdf = new TPDF("/home/sks/PSFile/pro/hitpat_e40.pdf", 112);

  // Title
  /*
  {
    //    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    TCanvas *c = new TCanvas("c0", "c0");
    //    pdf->NewPage();
    c->UseCurrentStyle();
    c->Divide(1,1);
    c->cd(1);
    c->GetPad(1)->Range(0,0,100,100);
    TText text;
    text.SetTextSize(0.2);
    text.SetTextAlign(22);
    TTimeStamp stamp;
    stamp.Add( -stamp.GetZoneOffset() );
    text.SetTextSize(0.1);
    text.DrawText(50.,50., stamp.AsString("s") );
    c->Update();
    c->cd();
    c->GetPad(1)->Close();
    delete c;
    c = NULL;
  }
  */

{
  // draw ADC
  TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  c->Clear();
  c->Divide(3,2);

  TH1* h = NULL;
  int base_id;

  // beam
  c->cd(1);
  base_id = HistMaker::getUniqueID(kBH1, 0, kHitPat);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();

  c->cd(2);
  base_id = HistMaker::getUniqueID(kBFT, 0, kHitPat);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();

  c->cd(3);
  base_id = HistMaker::getUniqueID(kBFT, 0, kHitPat,2);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();

  c->cd(4);
  base_id = HistMaker::getUniqueID(kBC3, 0, kHitPat, 1);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();

  c->cd(5);
  base_id = HistMaker::getUniqueID(kBC4, 0, kHitPat, 5);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();

  c->cd(6);
  base_id = HistMaker::getUniqueID(kBH2, 0, kHitPat,1);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();
  c->Update();

  c->cd(0);
}

  // pdf->Close();

  gROOT->SetStyle("Classic");

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
