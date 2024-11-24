// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispHitPat()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  // Return to the initial position
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c5");
    c->cd();
  }

  int n_layerbc3 = 6;
  int n_layerbc4 = 6;
  int n_layersdc1 = 6;
  int n_layersdc2 = 4;
  int n_layersdc3 = 4;
  int n_layersdc4 = 4;
  int n_layersdc5 = 4;


  // gROOT->SetStyle("Plain");
  // // TPDF *pdf = new TPDF("/home/sks/PSFile/pro/hitpat_e40.pdf", 112);

  // // Title
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
{
  TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  c->Clear();
  c->Divide(6,4);

  TH1* h = NULL;

  // beam
  c->cd(1);
  int base_id = HistMaker::getUniqueID(kBH1, 0, kHitPat);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();

  c->cd(2);
  base_id = HistMaker::getUniqueID(kBFT, 0, kHitPat, 1);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();

  c->cd(3);
  base_id = HistMaker::getUniqueID(kBFT, 0, kHitPat, 2);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();

  c->cd(4);
  base_id = HistMaker::getUniqueID(kBH2, 0, kHitPat);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();
  c->Update();

  c->cd(5);
  base_id = HistMaker::getUniqueID(kBAC, 0, kHitPat);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();
  c->Update();

  TH1* hh = NULL;

  base_id = HistMaker::getUniqueID(kBC3, 0, kHitPat);
  int base_id_ctot = HistMaker::getUniqueID(kBC3, 0, kHitPat, 1+kTOTcutOffset);
  for(int i=0; i<n_layerbc3; i++){
    c->cd(i+7);
    h = (TH1*)GHist::get(base_id + i);
    h->SetMinimum(0);
    if( !h ) continue;
    h->Draw();
    hh = (TH1*)GHist::get(base_id_ctot + i);
    hh->SetMinimum(0);
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }


  base_id = HistMaker::getUniqueID(kBC4, 0, kHitPat);
  base_id_ctot = HistMaker::getUniqueID(kBC4, 0, kHitPat, 1+kTOTcutOffset);
  for(int i=0; i<n_layerbc4; i++){
    c->cd(i+7+n_layerbc3);
    h = (TH1*)GHist::get(base_id + i);
    h->SetMinimum(0);
    if( !h ) continue;
    h->Draw();
    hh = (TH1*)GHist::get(base_id_ctot + i);
    hh->SetMinimum(0);
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }

  c->cd(19);
  base_id = HistMaker::getUniqueID(kTOF, 0, kHitPat);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();

  c->cd(20);
  base_id = HistMaker::getUniqueID(kAC1, 0, kHitPat);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();

  c->cd(21);
  base_id = HistMaker::getUniqueID(kWC, 0, kHitPat);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();

  c->cd(22);
  base_id = HistMaker::getUniqueID(kSFV, 0, kHitPat);
  h = (TH1*)GHist::get(base_id);
  h->SetMinimum(0);
  h->Draw();


  c->Update();

  c->cd(0);
}

{
  TCanvas *c2 = (TCanvas*)gROOT->FindObject("c2");
  c2->Clear();
  c2->Divide(6,4);

  TH1* h = NULL;
  TH1* hh = NULL;

  int base_id = HistMaker::getUniqueID(kSDC1, 0, kHitPat);
  int base_id_ctot = HistMaker::getUniqueID(kSDC1, 0, kHitPat, 1+kTOTcutOffset);
  for(int i=0; i<n_layersdc1; i++){
    c2->cd(i+1);
    h = (TH1*)GHist::get(base_id + i);
    h->SetMinimum(0);
    if( !h ) continue;
    h->Draw();
    hh = (TH1*)GHist::get(base_id_ctot + i);
    hh->SetMinimum(0);
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }


  base_id = HistMaker::getUniqueID(kSDC2, 0, kHitPat);
  base_id_ctot = HistMaker::getUniqueID(kSDC2, 0, kHitPat, 1+kTOTcutOffset);
  for(int i=0; i<n_layersdc2; i++){
    c2->cd(i+7);
    h = (TH1*)GHist::get(base_id + i);
    h->SetMinimum(0);
    if( !h ) continue;
    h->Draw();
    hh = (TH1*)GHist::get(base_id_ctot + i);
    hh->SetMinimum(0);
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }

  base_id = HistMaker::getUniqueID(kSDC3, 0, kHitPat);
  base_id_ctot = HistMaker::getUniqueID(kSDC3, 0, kHitPat, 11);
  for(int i=0; i<n_layersdc3; i++){
    c2->cd(i+7+n_layersdc2);
    h = (TH1*)GHist::get(base_id + i);
    h->SetMinimum(0);
    if( !h ) continue;
    h->Draw();
    hh = (TH1*)GHist::get(base_id_ctot + i);
    hh->SetMinimum(0);
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }

  base_id = HistMaker::getUniqueID(kSDC4, 0, kHitPat);
  base_id_ctot = HistMaker::getUniqueID(kSDC4, 0, kHitPat, 11);
  for(int i=0; i<n_layersdc4; i++){
    c2->cd(i+7+n_layersdc2+n_layersdc3);
    h = (TH1*)GHist::get(base_id + i);
    h->SetMinimum(0);
    if( !h ) continue;
    h->Draw();
    hh = (TH1*)GHist::get(base_id_ctot + i);
    hh->SetMinimum(0);
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }


  base_id = HistMaker::getUniqueID(kSDC5, 0, kHitPat);
  base_id_ctot = HistMaker::getUniqueID(kSDC5, 0, kHitPat, 11);
  for(int i=0; i<n_layersdc5; i++){
    c2->cd(i+7+n_layersdc2+n_layersdc3+n_layersdc4);
    h = (TH1*)GHist::get(base_id + i);
    h->SetMinimum(0);
    if( !h ) continue;
    h->Draw();
    hh = (TH1*)GHist::get(base_id_ctot + i);
    hh->SetMinimum(0);
    if( !hh ) continue;
    hh->SetLineColor( kRed );
    hh->Draw("same");
  }

  c2->Update();


  c2->cd(0);

}

  gROOT->SetStyle("Classic");

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}
