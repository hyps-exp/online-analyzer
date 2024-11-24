// -*- C++ -*-

using hddaq::gui::Updater;

//_____________________________________________________________________________
void
dispMatrix()
{
  if(Updater::isUpdating())
    return;
  Updater::setUpdating(true);

  const auto& gMatrix = MatrixParamMan::GetInstance();
  // Mtx2D
  for(Int_t i=0; i<2; ++i){
    auto c = dynamic_cast<TCanvas*>(gROOT->FindObject(Form("c%d", i+1)));
    if(!c) continue;
    c->Clear();
    c->cd(0);
    Int_t mtx2d_id  = HistMaker::getUniqueID(kMisc, kHul2D, kHitPat2D);
    Int_t tofsch_id = HistMaker::getUniqueID(kCorrelation, 1, 0);
    auto hmtx2d  = dynamic_cast<TH2*>(GHist::get(mtx2d_id + i));
    auto htofsch = dynamic_cast<TH2*>(GHist::get(tofsch_id));
    if(!hmtx2d) continue;
    if(!htofsch) continue;
    hmtx2d->Reset();
    hmtx2d->SetLineWidth(1);
    hmtx2d->SetLineColor(kRed);
    for(Int_t i_tof=0; i_tof<NumOfSegTOF; ++i_tof){
      for(Int_t i_sch=0; i_sch<NumOfSegSCH; ++i_sch){
        if(i == 0 && !gMatrix.IsAccept2D1(i_tof, i_sch))
          hmtx2d->Fill(i_sch, i_tof);
        else if(i == 1 && !gMatrix.IsAccept2D2(i_tof, i_sch))
          hmtx2d->Fill(i_sch, i_tof);
      } // for(i_sch)
    } // for(i_tof)

    hmtx2d->GetXaxis()->SetRangeUser(0, NumOfSegSCH);
    hmtx2d->GetYaxis()->SetRangeUser(0, NumOfSegTOF);
    hmtx2d->SetStats(0);
    hmtx2d->Draw("box");

    htofsch->GetXaxis()->SetRangeUser(0, NumOfSegSCH);
    htofsch->GetYaxis()->SetRangeUser(0, NumOfSegTOF);
    htofsch->Draw("same");

    gPad->SetGridx();
    gPad->SetGridy();

    c->Update();
  }
  // Mtx3D
  auto c3 = dynamic_cast<TCanvas*>(gROOT->FindObject("c3"));
  if(c3){
    c3->Clear();
    c3->Divide(4, 2);
    for(Int_t i=0; i<NumOfSegBH2; ++i){
      c3->cd(i+1);
      Int_t mtx3d_id  = HistMaker::getUniqueID(kMisc, kHul3D, kHitPat2D);
      Int_t tofsch_id = HistMaker::getUniqueID(kCorrelation, 2, 0);
      auto hmtx3d  = dynamic_cast<TH2*>(GHist::get(mtx3d_id + i));
      auto htofsch = dynamic_cast<TH2*>(GHist::get(tofsch_id + i));
      if(!hmtx3d) continue;
      if(!htofsch) continue;
      hmtx3d->Reset();
      hmtx3d->SetLineWidth(1);
      hmtx3d->SetLineColor(kRed);
      for(Int_t i_tof=0; i_tof<NumOfSegTOF; ++i_tof){
        for(Int_t i_sch=0; i_sch<NumOfSegSCH; ++i_sch){
          Bool_t hul3d_flag = gMatrix.IsAccept3D(i_tof, i_sch, i);
          if(hul3d_flag) hmtx3d->Fill(i_sch, i_tof);
        } // for(i_sch)
      } // for(i_tof)

      hmtx3d->GetXaxis()->SetRangeUser(0, NumOfSegSCH);
      hmtx3d->GetYaxis()->SetRangeUser(0, NumOfSegTOF);
      hmtx3d->SetStats(0);
      hmtx3d->Draw("box");

      htofsch->GetXaxis()->SetRangeUser(0, NumOfSegSCH);
      htofsch->GetYaxis()->SetRangeUser(0, NumOfSegTOF);
      htofsch->Draw("same");

      gPad->SetGridx();
      gPad->SetGridy();

      c3->Update();
    }
  }
  Updater::setUpdating(false);
}
