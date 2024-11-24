// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

void dispFBT( void )
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  const int n_seg_1     = 48;
  const int n_seg_2     = 64;
  const int FBTOffset   = 200;
  int fbt1_1_id[6];
  int fbt1_2_id[6];
  int fbt2_1_id[6];
  int fbt2_2_id[6];
  fbt1_1_id[] = {
//    HistMaker::getUniqueID(kFBT1, l, kTDC,    1),
//    HistMaker::getUniqueID(kFBT1, l, kADC,    1),
    HistMaker::getUniqueID(kFBT1, 0, kTDC,    n_seg_1+1),
    HistMaker::getUniqueID(kFBT1, 0, kADC,    n_seg_1+1),
    HistMaker::getUniqueID(kFBT1, 0, kHitPat, 1),
    HistMaker::getUniqueID(kFBT1, 0, kTDC2D,  1),
    HistMaker::getUniqueID(kFBT1, 0, kADC2D,  1),
    HistMaker::getUniqueID(kFBT1, 0, kMulti,  1)
  };
  fbt1_2_id[] = {
//    HistMaker::getUniqueID(kFBT1, l, kTDC,    FBTOffset+1),
//    HistMaker::getUniqueID(kFBT1, l, kADC,    FBTOffset+1),
    HistMaker::getUniqueID(kFBT1, 1, kTDC,    FBTOffset+n_seg_1+1),
    HistMaker::getUniqueID(kFBT1, 1, kADC,    FBTOffset+n_seg_1+1),
    HistMaker::getUniqueID(kFBT1, 1, kHitPat, FBTOffset+1),
    HistMaker::getUniqueID(kFBT1, 1, kTDC2D,  FBTOffset+1),
    HistMaker::getUniqueID(kFBT1, 1, kADC2D,  FBTOffset+1),
    HistMaker::getUniqueID(kFBT1, 1, kMulti,  FBTOffset+1)
  };

  fbt2_1_id[] = {
//    HistMaker::getUniqueID(kFBT2, l, kTDC,    1),
//    HistMaker::getUniqueID(kFBT2, l, kADC,    1),
    HistMaker::getUniqueID(kFBT2, 0, kTDC,    n_seg_1+1),
    HistMaker::getUniqueID(kFBT2, 0, kADC,    n_seg_1+1),
    HistMaker::getUniqueID(kFBT2, 0, kHitPat, 1),
    HistMaker::getUniqueID(kFBT2, 0, kTDC2D,  1),
    HistMaker::getUniqueID(kFBT2, 0, kADC2D,  1),
    HistMaker::getUniqueID(kFBT2, 0, kMulti,  1)
  }  
  fbt2_2_id[] = {
//    HistMaker::getUniqueID(kFBT2, l, kTDC,    FBTOffset+1),
//    HistMaker::getUniqueID(kFBT2, l, kADC,    FBTOffset+1),
    HistMaker::getUniqueID(kFBT2, 1, kTDC,    FBTOffset+n_seg_2+1),
    HistMaker::getUniqueID(kFBT2, 1, kADC,    FBTOffset+n_seg_2+1),
    HistMaker::getUniqueID(kFBT2, 1, kHitPat, FBTOffset+1),
    HistMaker::getUniqueID(kFBT2, 1, kTDC2D,  FBTOffset+1),
    HistMaker::getUniqueID(kFBT2, 1, kADC2D,  FBTOffset+1),
    HistMaker::getUniqueID(kFBT2, 1, kMulti,  FBTOffset+1)
  };


  // // Draw TDC UP
  // {
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  //   c->Clear();
  //   c->Divide(4,4);
  //   for( int i=0; i<n_seg; ++i ){
  //     c->cd(i+1)->SetGrid();
  //     TH1 *h = (TH1*)GHist::get( fbt1_id_c1[0]+i );
  //     h->Draw();
  //   }
  //   c->Update();
  // }

  // // Draw TDC DOWN
  // {
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c2");
  //   c->Clear();
  //   c->Divide(4,4);
  //   for( int i=0; i<n_seg; ++i ){
  //     c->cd(i+1)->SetGrid();
  //     TH1 *h = (TH1*)GHist::get( fbt1_id_c1[0]+n_seg+i );
  //     h->Draw();
  //   }
  //   c->Update();
  // }

  // // Draw TOT UP
  // {
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c3");
  //   c->Clear();
  //   c->Divide(4,4);
  //   for( int i=0; i<n_seg; ++i ){
  //     c->cd(i+1)->SetGrid();
  //     TH1 *h = (TH1*)GHist::get( fbt1_id_c1[1]+i );
  //     h->Draw();
  //   }
  //   c->Update();
  // }

  // // Draw TOT DOWN
  // {
  //   TCanvas *c = (TCanvas*)gROOT->FindObject("c4");
  //   c->Clear();
  //   c->Divide(4,4);
  //   for( int i=0; i<n_seg; ++i ){
  //     c->cd(i+1)->SetGrid();
  //     TH1 *h = (TH1*)GHist::get( fbt1_id_c1[1]+n_seg+i );
  //     h->Draw();
  //   }
  //   c->Update();
  // }

  //
  {
    TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
    c->Clear();
    c->Divide(3,2);
    for( int l=0; l<2; ++l ){
    for( int i=0; i<3; ++i ){
      c->cd(l*3+i+1)->SetGrid();
      TH1 *h = (TH1*)GHist::get( fbt1_id[l][i] );
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
