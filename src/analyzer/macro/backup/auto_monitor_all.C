// Updater belongs to the namespace hddaq::gui
using namespace hddaq::gui;

#include<unistd.h>
#include<vector>

void initialize(std::vector<std::vector<int > >& parent);
void draw(TCanvas* c, std::vector<int>& id);

void auto_monitor_all()
{
  // You must write these lines for the thread safe
  // ----------------------------------
  if(Updater::isUpdating()){return;}
  Updater::setUpdating(true);
  // ----------------------------------

  std::vector<std::vector<int > > parent;
  initialize(parent);

  TCanvas *c = (TCanvas*)gROOT->FindObject("c1");
  c->Clear();
  c->cd(0);
  int n = parent.size();
  int loop = 0;
  for(;;){
    for(int i = 0; i<n; ++i){
      std::vector<int>& id = parent[i];
      draw(c, id);
    }

    ++loop;
    if(loop == 6){
      Updater::getInstance().resetAll();
      loop = 0;
      sleep(10);
    }
  }

  // You must write these lines for the thread safe
  // ----------------------------------
  Updater::setUpdating(false);
  // ----------------------------------
}

void initialize(std::vector<std::vector<int > >& parent)
{
  std::vector<int> id;

  // BH1
  id.push_back(HistMaker::getUniqueID(kBH1, 0, kHitPat));
  id.push_back(HistMaker::getUniqueID(kBH1, 0, kMulti));
  parent.push_back(id);
  id.clear();

  // BFT
  id.push_back(HistMaker::getUniqueID(kBFT, 0, kHitPat));
  id.push_back(HistMaker::getUniqueID(kBFT, 0, kTDC));
  id.push_back(HistMaker::getUniqueID(kBFT, 0, kADC));
  id.push_back(HistMaker::getUniqueID(kBFT, 0, kMulti));
  parent.push_back(id);
  id.clear();

  // BC3 TDC
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kBC3, 0, kTDC)+i);
  }
  parent.push_back(id);
  id.clear();

  // BC3 HitPat
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kBC3, 0, kHitPat)+i);
  }
  parent.push_back(id);
  id.clear();

  // BC3 Multi
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kBC3, 0, kMulti)+i);
  }
  parent.push_back(id);
  id.clear();

  // BC4 TDC
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kBC4, 0, kTDC)+i);
  }
  parent.push_back(id);
  id.clear();

  // BC4 HitPat
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kBC4, 0, kHitPat)+i);
  }
  parent.push_back(id);
  id.clear();

  // BC4 Multi
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kBC4, 0, kMulti)+i);
  }
  parent.push_back(id);
  id.clear();

  // BH2
  id.push_back(HistMaker::getUniqueID(kBH2, 0, kHitPat));
  id.push_back(HistMaker::getUniqueID(kBH2, 0, kMulti));
  parent.push_back(id);
  id.clear();

  // BAC12, SAC1
  id.push_back(HistMaker::getUniqueID(kBAC_SAC, 0, kADC)+2);
  id.push_back(HistMaker::getUniqueID(kBAC_SAC, 0, kADC)+3);
  id.push_back(HistMaker::getUniqueID(kBAC_SAC, 0, kADC)+4);
  id.push_back(HistMaker::getUniqueID(kBAC_SAC, 0, kTDC)+2);
  id.push_back(HistMaker::getUniqueID(kBAC_SAC, 0, kTDC)+3);
  id.push_back(HistMaker::getUniqueID(kBAC_SAC, 0, kTDC)+4);
  parent.push_back(id);
  id.clear();

  // Ge
  id.push_back(HistMaker::getUniqueID(kGe, 0, kHitPat));
  id.push_back(HistMaker::getUniqueID(kGe, 0, kCRM2D));
  parent.push_back(id);
  id.clear();

  // PWO 1
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kPWO, 0, kHitPat)+i);
  }
  parent.push_back(id);
  id.clear();

  // PWO 2
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kPWO, 0, kHitPat)+i+6);
  }
  parent.push_back(id);
  id.clear();

  // PWO 3
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kPWO, 0, kHitPat)+i+12);
  }
  parent.push_back(id);
  id.clear();

  // PWO 4
  for(int i = 0; i<4; ++i){
    id.push_back(HistMaker::getUniqueID(kPWO, 0, kHitPat)+i+18);
  }
  parent.push_back(id);
  id.clear();

  // SP0 1
  id.push_back(HistMaker::getUniqueID(kSP0, kSP0_L1, kHitPat));
  id.push_back(HistMaker::getUniqueID(kSP0, kSP0_L1, kHitPat)+1);
  id.push_back(HistMaker::getUniqueID(kSP0, kSP0_L2, kHitPat));
  id.push_back(HistMaker::getUniqueID(kSP0, kSP0_L2, kHitPat)+1);
  parent.push_back(id);
  id.clear();

  // SP0 2
  id.push_back(HistMaker::getUniqueID(kSP0, kSP0_L3, kHitPat));
  id.push_back(HistMaker::getUniqueID(kSP0, kSP0_L3, kHitPat)+1);
  id.push_back(HistMaker::getUniqueID(kSP0, kSP0_L4, kHitPat));
  id.push_back(HistMaker::getUniqueID(kSP0, kSP0_L4, kHitPat)+1);
  parent.push_back(id);
  id.clear();

  // SP0 3
  id.push_back(HistMaker::getUniqueID(kSP0, kSP0_L5, kHitPat));
  id.push_back(HistMaker::getUniqueID(kSP0, kSP0_L5, kHitPat)+1);
  id.push_back(HistMaker::getUniqueID(kSP0, kSP0_L6, kHitPat));
  id.push_back(HistMaker::getUniqueID(kSP0, kSP0_L6, kHitPat)+1);
  parent.push_back(id);
  id.clear();

  // SP0 4
  id.push_back(HistMaker::getUniqueID(kSP0, kSP0_L7, kHitPat));
  id.push_back(HistMaker::getUniqueID(kSP0, kSP0_L7, kHitPat)+1);
  id.push_back(HistMaker::getUniqueID(kSP0, kSP0_L8, kHitPat));
  id.push_back(HistMaker::getUniqueID(kSP0, kSP0_L8, kHitPat)+1);
  parent.push_back(id);
  id.clear();

  // SDC2 TDC
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kSDC2, 0, kTDC)+i);
  }
  parent.push_back(id);
  id.clear();

  // SDC2 HitPat
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kSDC2, 0, kHitPat)+i);
  }
  parent.push_back(id);
  id.clear();

  // SDC2 Multi
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kSDC2, 0, kMulti)+i);
  }
  parent.push_back(id);
  id.clear();

  // HDC TDC
  for(int i = 0; i<4; ++i){
    id.push_back(HistMaker::getUniqueID(kHDC, 0, kTDC)+i);
  }
  parent.push_back(id);
  id.clear();

  // HDC HitPat
  for(int i = 0; i<4; ++i){
    id.push_back(HistMaker::getUniqueID(kHDC, 0, kHitPat)+i);
  }
  parent.push_back(id);
  id.clear();

  // HDC Multi
  for(int i = 0; i<4; ++i){
    id.push_back(HistMaker::getUniqueID(kHDC, 0, kMulti)+i);
  }
  parent.push_back(id);
  id.clear();

  // SDC3 TDC
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kSDC3, 0, kTDC)+i);
  }
  parent.push_back(id);
  id.clear();

  // SDC3 HitPat
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kSDC3, 0, kHitPat)+i);
  }
  parent.push_back(id);
  id.clear();

  // SDC3 Multi
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kSDC3, 0, kMulti)+i);
  }
  parent.push_back(id);
  id.clear();

  // SDC4 TDC
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kSDC4, 0, kTDC)+i);
  }
  parent.push_back(id);
  id.clear();

  // SDC4 HitPat
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kSDC4, 0, kHitPat)+i);
  }
  parent.push_back(id);
  id.clear();

  // SDC4 Multi
  for(int i = 0; i<6; ++i){
    id.push_back(HistMaker::getUniqueID(kSDC4, 0, kMulti)+i);
  }
  parent.push_back(id);
  id.clear();

  // TOF
  id.push_back(HistMaker::getUniqueID(kTOF, 0, kHitPat));
  id.push_back(HistMaker::getUniqueID(kTOF, 0, kMulti));
  parent.push_back(id);
  id.clear();

  // SFV SAC3
  id.push_back(HistMaker::getUniqueID(kSFV_SAC3, 0, kHitPat));
  parent.push_back(id);
  id.clear();

  // LC
  id.push_back(HistMaker::getUniqueID(kLC, 0, kHitPat));
  id.push_back(HistMaker::getUniqueID(kLC, 0, kMulti));
  parent.push_back(id);
  id.clear();
}

void draw(TCanvas* c, std::vector<int>& id)
{
  c->cd(0);
  c->Clear();
  if(id.size()==2){
    c->Divide(2,1);
  }else if(3 <= id.size() && id.size() <= 4){
    c->Divide(2,2);
  }else if(5 <= id.size() && id.size() <= 6){
    c->Divide(3,2);
  }

  for(int i = 0; i<id.size(); ++i){
    c->cd(i+1);
    TH1* tmp = GHist::get(id[i]);
    tmp->SetMinimum(0);
    tmp->Draw("colz");
  }

  c->cd(0);
  c->Modified();
  c->Update();
  sleep(6);
}
