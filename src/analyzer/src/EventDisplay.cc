// -*- C++ -*-

// Unit is mm.

#include <bitset>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

#include <TApplication.h>
#include <TBRIK.h>
#include <TBox.h>
#include <TCanvas.h>
#include <TColor.h>
#include <TEnv.h>
#include <TFile.h>
#include <TGeometry.h>
#include <TGraph.h>
#include <TH1.h>
#include <TH2.h>
#include <TLatex.h>
#include <TMarker.h>
#include <TMarker3DBox.h>
#include <TMixture.h>
#include <TNode.h>
#include <TPad.h>
#include <TPave.h>
#include <TPaveLabel.h>
#include <TPaveText.h>
#include <TPolyLine.h>
#include <TPolyLine3D.h>
#include <TPolyMarker.h>
#include <TPolyMarker3D.h>
#include <TROOT.h>
#include <TRint.h>
#include <TRotMatrix.h>
#include <TStyle.h>
#include <TTimeStamp.h>
#include <TTRD1.h>
#include <TTRD2.h>
#include <TTUBS.h>
#include <TTUBS.h>
#include <TView.h>

#include <std_ostream.hh>
#include <DAQNode.hh>
#include <Unpacker.hh>
#include <UnpackerManager.hh>

#include "DCGeomMan.hh"
#include "DCLocalTrack.hh"
#include "DeleteUtility.hh"
#include "DetectorID.hh"
// #include "EMCAnalyzer.hh"
#include "EventDisplay.hh"
#include "Exception.hh"
#include "FuncName.hh"
#include "K18TrackD2U.hh"
#include "KuramaTrack.hh"
#include "MathTools.hh"
// #include "RMAnalyzer.hh"

// Detector Construction
#define BH2         1
#define BcOut       1
#define TARGET      1
#define KURAMA      1
#define SdcIn       1
#define SdcOut      1
#define SCH         1
#define TOF         1
#define E07Detector 0

// Additional Canvas
#define Vertex     0
#define Hist       0

ClassImp(EventDisplay);

namespace
{
  const std::string& class_name("EventDisplay");
  using namespace hddaq;
  using namespace hddaq::unpacker;

  const UnpackerManager& gUnpacker = GUnpacker::get_instance();
  const DCGeomMan&       gGeom     = DCGeomMan::GetInstance();
  // EMCAnalyzer&     gEMC  = EMCAnalyzer::GetInstance();
  // RMAnalyzer&      gRM   = RMAnalyzer::GetInstance();

  const Color_t BGColor    = kBlack;
  const Color_t FGColor    = kWhite;
  const Color_t HitColor   = kCyan;
  const Width_t TrackColor = kRed;
  const Width_t TrackWidth = 1;

  // const Double_t BeamAxis = -150.;
  const Double_t BeamAxis = -240.;

#if Vertex
  const Double_t MinX = -50.;
  const Double_t MaxX =  50.;
  const Double_t MinY = -50.;
  const Double_t MaxY =  50.;
  const Double_t MinZ = -25.;
#endif
  const Double_t MaxZ =  50.;

  inline TText*
  NewTText( Short_t align=12, Float_t tsize=0.025 )
  {
    TText *text = new TText;
    text->SetNDC();
    text->SetTextAlign(align);
    text->SetTextFont(42);
    text->SetTextSize(tsize);
    text->SetTextColor(FGColor);
    return text;
  }

  inline std::vector<TText*>
  NewTTextArray( UInt_t ntext, Short_t align=12, Float_t tsize=0.025 )
  {
    std::vector<TText*> array(ntext);
    for( Int_t i=0, n=array.size(); i<n; ++i ){
      array[i] = NewTText(align, tsize);
    }
    return array;
  }

  inline void
  ConstructionDone( const TString& name, std::ostream& ost=hddaq::cout )
  {
    const Int_t n = 20 - name.Sizeof();
    ost << " " << name << " ";
    for( Int_t i=0; i<n; ++i )
      ost << ".";
    ost << " done" << std::endl;
  }

}

//______________________________________________________________________________
EventDisplay::EventDisplay( void )
  : m_is_ready(false),
    m_text( new TLatex ),
    m_theApp(0),
    m_geometry(0),
    m_node(0),
    m_canvas(0),
    m_canvas_vertex(0),
    m_canvas_hist(0),
    m_hist_vertex_x(0),
    m_hist_vertex_y(0),
    m_hist_p(0),
    m_hist_m2(0),
    m_target_node(0),
    m_kurama_inner_node(0),
    m_kurama_outer_node(0),
    m_BH2wall_node(0),
    m_FBHwall_node(0),
    m_SCHwall_node(0),
    m_TOFwall_node(0),
    m_init_step_mark(0),
    m_kurama_step_mark(0),
    m_TargetXZ_box(0),
    m_TargetYZ_box(0),
    m_EmulsionXZ_box(0),
    m_EmulsionYZ_box(0),
    m_SSD_x_hist(0),
    m_SSD_y_hist(0),
    m_VertexPointXZ(0),
    m_VertexPointYZ(0),
    m_KuramaMarkVertexX(0),
    m_KuramaMarkVertexY(0),
    m_MissMomXZ_line(0),
    m_MissMomYZ_line(0)
{
  m_text->SetNDC();
  m_text->SetTextAlign(12);
  m_text->SetTextColor(kWhite);
  m_text->SetTextFont(42);
  m_text->SetTextSize(0.100);
}

//______________________________________________________________________________
EventDisplay::~EventDisplay( void )
{
  del::DeleteObject( m_theApp );
}

//______________________________________________________________________________
Bool_t
EventDisplay::Initialize( void )
{
  if( m_is_ready ){
    hddaq::cerr << "#W " << FUNC_NAME
		<< " already initialied" << std::endl;
    return false;
  }

  m_theApp = new TApplication( "App", 0, 0 );

#if ROOT_VERSION_CODE > ROOT_VERSION(6,4,0)
  gStyle->SetPalette(kCool);
#else
  gStyle->SetPalette(53);
#endif
  gStyle->SetNumberContours(255);

  m_geometry = new TGeometry( "evdisp","K1.8 Event Display" );

  ThreeVector worldSize( 1000., 1000., 1000. ); /*mm*/
  new TBRIK( "world", "world", "void",
	     worldSize.x(), worldSize.y(), worldSize.z() );

  m_node = new TNode( "node", "node", "world", 0., 0., 0. );
  m_geometry->GetNode("node")->SetVisibility(0);

#if BH2
  ConstructBH2();
#endif

#if TARGET
  ConstructTarget();
#endif

#if KURAMA
  ConstructKURAMA();
#endif

#if E07Detector
  ConstructCollimator();
#endif

#if BcOut
  ConstructBcOut();
#endif

#if SdcIn
  ConstructSdcIn();
#endif

#if SdcOut
  ConstructSdcOut();
#endif

#if SCH
  ConstructSCH();
#endif

#if TOF
  ConstructTOF();
#endif

  m_canvas = new TCanvas( "canvas", "K1.8 Event Display",
			  1680, 250, 1000, 800 );
  m_canvas->SetFillColor(BGColor);

  m_geometry->Draw();

  gPad->SetPhi( 180. );
  gPad->SetTheta( 1.0 );
  // gPad->SetTheta( -20. );
  gPad->GetView()->ZoomIn();

  m_canvas->Update();

#if Vertex

#if E07Detector
  ConstructFBH();
  ConstructEmulsion();
  ConstructSSD();
#endif

  m_canvas_vertex = new TCanvas( "canvas_vertex", "K1.8 Event Display (Vertex)",
				 2680, 540, 920, 500 );
  m_canvas_vertex->SetFillColor(BGColor);
  m_canvas_vertex->Divide(1,2);
  m_canvas_vertex->cd(1);
  gPad->DrawFrame( MinZ, MinX, MaxZ, MaxX );
  m_EmulsionXZ_box->Draw("L");
  m_TargetXZ_box->Draw("L");
  for( Int_t i=0; i<NumOfSegFBH; ++i ){
    m_FBHseg_box[i]->Draw("L");
  }
  m_SSD_x_hist->Draw("colz same");
  m_text->DrawLatex( 0.020, 0.920, "Vertex XZ Projection");
  gPad->Update();

  m_canvas_vertex->cd(2);
  gPad->DrawFrame( MinZ, MinY, MaxZ, MaxY );
  m_EmulsionYZ_box->Draw("L");
  m_TargetYZ_box->Draw("L");
  m_SSD_y_hist->Draw("colz same");
  m_text->DrawLatex( 0.020, 0.920, "Vertex YZ Projection");
  gPad->Update();
#endif

#if Hist
  m_canvas_hist = new TCanvas( "canvas_hist", "EventDisplay Hist",
			       400, 800 );
  m_canvas_hist->Divide(1,2);
  m_hist_p  = new TH1F( "hist1", "Momentum", 100, 0., 3. );
  m_hist_m2 = new TH1F( "hist2", "Mass Square", 100, -0.8, 2.2 );
  m_canvas_hist->cd(1)->SetGrid();
  m_hist_p->Draw();
  m_canvas_hist->cd(2)->SetGrid();
  m_hist_m2->Draw();
  // gStyle->SetOptTitle(0);
  gStyle->SetOptStat(1111110);
#endif

  ResetVisibility();

  Update();

  m_is_ready = true;
  return m_is_ready;
}

//______________________________________________________________________________
Bool_t
EventDisplay::ConstructBH2( void )
{
  const Int_t lid = gGeom.GetDetectorId("BH2");

  Double_t rotMatBH2[NumOfMaxrixElements] = {};
  Double_t BH2wallX = 118.0/2.; // X
  Double_t BH2wallY =   5.0/2.; // Z
  Double_t BH2wallZ =  60.0/2.; // Y
  // Double_t BH2wallX = 120.0/2.; // X
  // Double_t BH2wallY =   6.0/2.; // Z
  // Double_t BH2wallZ =  40.0/2.; // Y
  std::vector<Double_t> BH2SizeX =
    { 35./2., 10./2., 7./2., 7./2., 7./2., 7./2., 10./2., 35./2. }; // X
  std::vector<Double_t> BH2SizeY( NumOfSegBH2,  5./2. ); // Z
  std::vector<Double_t> BH2SizeZ( NumOfSegBH2, 60./2. ); // Y
  std::vector<Double_t> BH2PosX =
    { -41.5/2., -19./2., -10.5/2., -3.5/2., 3.5/2., 10.5/2., 19./2., 41.5/2. };
  std::vector<Double_t> BH2PosY( NumOfSegBH2, 0./2. );
  std::vector<Double_t> BH2PosZ( NumOfSegBH2, 0./2. );

  CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		 gGeom.GetRotAngle1( lid ),
		 gGeom.GetRotAngle2( lid ),
		 rotMatBH2 );

  new TRotMatrix( "rotBH2", "rotBH2", rotMatBH2 );
  const ThreeVector& BH2wallPos = gGeom.GetGlobalPosition( lid );
  new TBRIK( "BH2wall_brik", "BH2wall_brik", "void",
	     BH2wallX, BH2wallY, BH2wallZ );
  m_BH2wall_node = new TNode( "BH2wall_node", "BH2wall_node", "BH2wall_brik",
			      BH2wallPos.x(),
			      BH2wallPos.y(),
			      BH2wallPos.z(), "rotBH2", "void" );
  m_BH2wall_node->SetVisibility(0);
  m_BH2wall_node->cd();

  for( Int_t i=0; i<NumOfSegBH2; ++i ){
    new TBRIK( Form( "BH2seg_brik_%d", i ),
	       Form( "BH2seg_brik_%d", i ),
	       "void", BH2SizeX.at(i), BH2SizeY.at(i), BH2SizeZ.at(i) );
    m_BH2seg_node.push_back( new TNode( Form( "BH2seg_node_%d", i ),
					Form( "BH2seg_node_%d", i ),
					Form( "BH2seg_brik_%d", i ),
					BH2PosX.at(i), BH2PosY.at(i), BH2PosZ.at(i) ) );
  }
  m_node->cd();
  ConstructionDone(__func__);
  return true;
}

//______________________________________________________________________________
Bool_t
EventDisplay::ConstructKURAMA( void )
{
  Double_t Matrix[NumOfMaxrixElements] = {};

  Double_t inner_x = 1400.0/2.; // X
  Double_t inner_y =  800.0/2.; // Z
  Double_t inner_z =  800.0/2.; // Y

  Double_t outer_x = 2200.0/2.; // X
  Double_t outer_y =  800.0/2.; // Z
  Double_t outer_z = 1540.0/2.; // Y

  Double_t uguard_inner_x = 1600.0/2.; // X
  Double_t uguard_inner_y =  100.0/2.; // Z
  Double_t uguard_inner_z = 1940.0/2.; // Y
  Double_t uguard_outer_x =  600.0/2.; // X
  Double_t uguard_outer_y =  100.0/2.; // Z
  Double_t uguard_outer_z =  300.0/2.; // Y

  Double_t dguard_inner_x = 1600.0/2.; // X
  Double_t dguard_inner_y =  100.0/2.; // Z
  Double_t dguard_inner_z = 1940.0/2.; // Y
  Double_t dguard_outer_x = 1100.0/2.; // X
  Double_t dguard_outer_y =  100.0/2.; // Z
  Double_t dguard_outer_z = 1100.0/2.; // Y

  CalcRotMatrix( 0., 0., 0., Matrix );

  new TRotMatrix( "rotKURAMA", "rotKURAMA", Matrix );

  new TBRIK( "kurama_inner_brik", "kurama_inner_brik",
	     "void", inner_x, inner_y, inner_z );

  new TBRIK( "kurama_outer_brik", "kurama_outer_brik",
	     "void", outer_x, outer_y, outer_z );

  new TBRIK( "uguard_inner_brik", "uguard_inner_brik",
	     "void", uguard_inner_x, uguard_inner_y, uguard_inner_z );

  new TBRIK( "uguard_outer_brik", "uguard_outer_brik",
	     "void", uguard_outer_x, uguard_outer_y, uguard_outer_z );

  new TBRIK( "dguard_inner_brik", "dguard_inner_brik",
	     "void", dguard_inner_x, dguard_inner_y, dguard_inner_z );

  new TBRIK( "dguard_outer_brik", "dguard_outer_brik",
	     "void", dguard_outer_x, dguard_outer_y, dguard_outer_z );


  m_kurama_inner_node = new TNode( "kurama_inner_node",
				   "kurama_inner_node",
				   "kurama_inner_brik",
				   0., 0., 0., "rotKURAMA", "void" );
  m_kurama_outer_node = new TNode( "kurama_outer_node",
				   "kurama_outer_node",
				   "kurama_outer_brik",
				   0., 0., 0., "rotKURAMA", "void" );

  TNode *uguard_inner = new TNode( "uguard_inner_node",
				   "uguard_inner_node",
				   "uguard_inner_brik",
				   0., 0., -820.,
				   "rotKURAMA", "void" );
  TNode *uguard_outer = new TNode( "uguard_outer_node",
				   "uguard_outer_node",
				   "uguard_outer_brik",
				   0., 0., -820.,
				   "rotKURAMA", "void" );

  TNode *dguard_inner = new TNode( "dguard_inner_node",
				   "dguard_inner_node",
				   "dguard_inner_brik",
				   0., 0., 820.,
				   "rotKURAMA", "void" );
  TNode *dguard_outer = new TNode( "dguard_outer_node",
				   "dguard_outer_node",
				   "dguard_outer_brik",
				   0., 0., 820.,
				   "rotKURAMA", "void" );

  // const Color_t c = FGColor;
  const Color_t c = kGray+1;
  m_kurama_inner_node->SetLineColor(c);
  m_kurama_outer_node->SetLineColor(c);
  uguard_inner->SetLineColor(c);
  uguard_outer->SetLineColor(c);
  dguard_inner->SetLineColor(c);
  dguard_outer->SetLineColor(c);

  m_node->cd();
  ConstructionDone(__func__);
  return true;
}

//______________________________________________________________________________
Bool_t
EventDisplay::ConstructCollimator( void )
{
  Double_t Matrix[NumOfMaxrixElements] = {};

  Double_t offsetZ = -2714.4;

  Double_t inner_x = 115.0/2.; // X
  Double_t inner_y = 400.0/2.; // Z
  Double_t inner_z =  30.0/2.; // Y

  Double_t outer_x = 400.0/2.; // X
  Double_t outer_y = 400.0/2.; // Z
  Double_t outer_z = 400.0/2.; // Y

  CalcRotMatrix( 0., 0., 0., Matrix );

  new TRotMatrix( "rotCol", "rotCol", Matrix );

  new TBRIK( "col_inner_brik", "col_inner_brik",
	     "void", inner_x, inner_y, inner_z );

  new TBRIK( "col_outer_brik", "col_outer_brik",
	     "void", outer_x, outer_y, outer_z );

  TNode *col_inner = new TNode( "col_inner_node",
				"col_inner_node",
				"col_inner_brik",
				BeamAxis, 0., offsetZ, "rotCol", "void" );
  TNode *col_outer = new TNode( "col_outer_node",
				"col_outer_node",
				"col_outer_brik",
				BeamAxis, 0., offsetZ, "rotCol", "void" );

  col_inner->SetLineColor(FGColor);
  col_outer->SetLineColor(FGColor);

  m_node->cd();
  ConstructionDone(__func__);
  return true;
}

//______________________________________________________________________________
Bool_t
EventDisplay::ConstructBcOut( void )
{
  const Double_t wireL = 200.0;

  const Double_t offsetZ = -3068.4;

  // BC3 X1
  {
    const Int_t lid = gGeom.GetDetectorId("BC3-X1");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotBC3X1", "rotBC3X1", Matrix );
    new TTUBE( "BC3X1Tube", "BC3X1Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<=NumOfWireBC3; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_BC3x1_node.push_back( new TNode( Form( "BC3x1_Node_%d", wire ),
					 Form( "BC3x1_Node_%d", wire ),
					 "BC3X1Tube",
					 localPos+BeamAxis,
					 wireGlobalPos.y(),
					 wireGlobalPos.z()+offsetZ,
					 "rotBC3X1", "void" ) );
    }
  }

  // BC3 X2
  {
    const Int_t lid = gGeom.GetDetectorId("BC3-X2");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotBC3X2", "rotBC3X2", Matrix );
    new TTUBE( "BC3X2Tube", "BC3X2Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<=NumOfWireBC3; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_BC3x2_node.push_back( new TNode( Form( "BC3x2_Node_%d", wire ),
					 Form( "BC3x2_Node_%d", wire ),
					 "BC3X2Tube",
					 localPos+BeamAxis,
					 wireGlobalPos.y(),
					 wireGlobalPos.z()+offsetZ,
					 "rotBC3X2", "void" ) );
    }
  }

  // BC3 V1
  {
    const Int_t lid = gGeom.GetDetectorId("BC3-V1");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotBC3V1", "rotBC3V1", Matrix );
    new TTUBE( "BC3V1Tube", "BC3V1Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<=NumOfWireBC3; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_BC3v1_node.push_back( new TNode( Form( "BC3v1_Node_%d", wire ),
					 Form( "BC3v1_Node_%d", wire ),
					 "BC3V1Tube",
					 localPos+BeamAxis,
					 wireGlobalPos.y(),
					 wireGlobalPos.z()+offsetZ,
					 "rotBC3V1", "void" ) );
    }
  }

  // BC3 V2
  {
    const Int_t lid = gGeom.GetDetectorId("BC3-V2");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotBC3V2", "rotBC3V2", Matrix );
    new TTUBE( "BC3V2Tube", "BC3V2Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<=NumOfWireBC3; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_BC3v2_node.push_back( new TNode( Form( "BC3v2_Node_%d", wire ),
					 Form( "BC3v2_Node_%d", wire ),
					 "BC3V2Tube",
					 localPos+BeamAxis,
					 wireGlobalPos.y(),
					 wireGlobalPos.z()+offsetZ,
					 "rotBC3V2", "void" ) );
    }
  }

  // BC3 U1
  {
    const Int_t lid = gGeom.GetDetectorId("BC3-U1");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotBC3U1", "rotBC3U1", Matrix );
    new TTUBE( "BC3U1Tube", "BC3U1Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<=NumOfWireBC3; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_BC3u1_node.push_back( new TNode( Form( "BC3u1_Node_%d", wire ),
					 Form( "BC3u1_Node_%d", wire ),
					 "BC3U1Tube",
					 localPos+BeamAxis,
					 wireGlobalPos.y(),
					 wireGlobalPos.z()+offsetZ,
					 "rotBC3U1", "void" ) );
    }
  }

  // BC3 U2
  {
    const Int_t lid = gGeom.GetDetectorId("BC3-U2");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotBC3U2", "rotBC3U2", Matrix );
    new TTUBE( "BC3U2Tube", "BC3U2Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<=NumOfWireBC3; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_BC3u2_node.push_back( new TNode( Form( "BC3u2_Node_%d", wire ),
					 Form( "BC3u2_Node_%d", wire ),
					 "BC3U2Tube",
					 localPos+BeamAxis,
					 wireGlobalPos.y(),
					 wireGlobalPos.z()+offsetZ,
					 "rotBC3U2", "void" ) );
    }
  }

  // BC4 U1
  {
    const Int_t lid = gGeom.GetDetectorId("BC4-U1");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotBC4U1", "rotBC4U1", Matrix );
    new TTUBE( "BC4U1Tube", "BC4U1Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<=NumOfWireBC4; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_BC4u1_node.push_back( new TNode( Form( "BC4u1_Node_%d", wire ),
					 Form( "BC4u1_Node_%d", wire ),
					 "BC4U1Tube",
					 localPos+BeamAxis,
					 wireGlobalPos.y(),
					 wireGlobalPos.z()+offsetZ,
					 "rotBC4U1", "void" ) );
    }
  }

  // BC4 U2
  {
    const Int_t lid = gGeom.GetDetectorId("BC4-U2");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotBC4U2", "rotBC4U2", Matrix );
    new TTUBE( "BC4U2Tube", "BC4U2Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<=NumOfWireBC4; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_BC4u2_node.push_back( new TNode( Form( "BC4u2_Node_%d", wire ),
					 Form( "BC4u2_Node_%d", wire ),
					 "BC4U2Tube",
					 localPos+BeamAxis,
					 wireGlobalPos.y(),
					 wireGlobalPos.z()+offsetZ,
					 "rotBC4U2", "void" ) );
    }
  }

  // BC4 V1
  {
    const Int_t lid = gGeom.GetDetectorId("BC4-V1");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotBC4V1", "rotBC4V1", Matrix );
    new TTUBE( "BC4V1Tube", "BC4V1Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<=NumOfWireBC4; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_BC4v1_node.push_back( new TNode( Form( "BC4v1_Node_%d", wire ),
					 Form( "BC4v1_Node_%d", wire ),
					 "BC4V1Tube",
					 localPos+BeamAxis,
					 wireGlobalPos.y(),
					 wireGlobalPos.z()+offsetZ,
					 "rotBC4V1", "void" ) );
    }
  }

  // BC4 V2
  {
    const Int_t lid = gGeom.GetDetectorId("BC4-V2");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotBC4V2", "rotBC4V2", Matrix );
    new TTUBE( "BC4V2Tube", "BC4V2Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<=NumOfWireBC4; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_BC4v2_node.push_back( new TNode( Form( "BC4v2_Node_%d", wire ),
					 Form( "BC4v2_Node_%d", wire ),
					 "BC4V2Tube",
					 localPos+BeamAxis,
					 wireGlobalPos.y(),
					 wireGlobalPos.z()+offsetZ,
					 "rotBC4V2", "void" ) );
    }
  }

  // BC4 X1
  {
    const Int_t lid = gGeom.GetDetectorId("BC4-X1");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotBC4X1", "rotBC4X1", Matrix );
    new TTUBE( "BC4X1Tube", "BC4X1Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<=NumOfWireBC4; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_BC4x1_node.push_back( new TNode( Form( "BC4x1_Node_%d", wire ),
					 Form( "BC4x1_Node_%d", wire ),
					 "BC4X1Tube",
					 localPos+BeamAxis,
					 wireGlobalPos.y(),
					 wireGlobalPos.z()+offsetZ,
					 "rotBC4X1", "void" ) );
    }
  }

  // BC4 X2
  {
    const Int_t lid = gGeom.GetDetectorId("BC4-X2");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotBC4X2", "rotBC4X2", Matrix );
    new TTUBE( "BC4X2Tube", "BC4X2Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<=NumOfWireBC4; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_BC4x2_node.push_back( new TNode( Form( "BC4x2_Node_%d", wire ),
					 Form( "BC4x2_Node_%d", wire ),
					 "BC4X2Tube",
					 localPos+BeamAxis,
					 wireGlobalPos.y(),
					 wireGlobalPos.z()+offsetZ,
					 "rotBC4X2", "void" ) );
    }
  }

  ConstructionDone(__func__);
  return true;
}

//______________________________________________________________________________
Bool_t
EventDisplay::ConstructSdcIn( void )
{
  const Double_t wireL = 200.0;

  // SFT U
  {
    const Int_t lid = gGeom.GetDetectorId("SFT-U");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotSFTU", "rotSFTU", Matrix );
    new TTUBE( "SFTUTube", "SFTUTube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<=NumOfSegSFT_UV; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_SDC1v1_node.push_back( new TNode( Form( "SFTu_Node_%d", wire ),
					  Form( "SFTu_Node_%d", wire ),
					  "SFTUTube",
					  wireGlobalPos.x()+localPos,
					  wireGlobalPos.y(),
					  wireGlobalPos.z(),
					  "rotSFTU", "void" ) );
    }
  }

  // SFT V
  {
    const Int_t lid = gGeom.GetDetectorId("SFT-V");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotSFTV", "rotSFTV", Matrix );
    new TTUBE( "SFTVTube", "SFTVTube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<=NumOfSegSFT_UV; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_SDC1v1_node.push_back( new TNode( Form( "SFTv_Node_%d", wire ),
					  Form( "SFTv_Node_%d", wire ),
					  "SFTVTube",
					  wireGlobalPos.x()+localPos,
					  wireGlobalPos.y(),
					  wireGlobalPos.z(),
					  "rotSFTV", "void" ) );
    }
  }

  // SFT X
  {
    const Int_t lid = gGeom.GetDetectorId("SFT-X");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotSFTX", "rotSFTX", Matrix );
    new TTUBE( "SFTXTube", "SFTXTube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<=NumOfSegSFT_X; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_SDC1v1_node.push_back( new TNode( Form( "SFTx_Node_%d", wire ),
					  Form( "SFTx_Node_%d", wire ),
					  "SFTXTube",
					  wireGlobalPos.x()+localPos,
					  wireGlobalPos.y(),
					  wireGlobalPos.z(),
					  "rotSFTX", "void" ) );
    }
  }

  // SDC1 V1
  {
    const Int_t lid = gGeom.GetDetectorId("SDC1-V1");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotSDC1V1", "rotSDC1V1", Matrix );
    new TTUBE( "SDC1V1Tube", "SDC1V1Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<=NumOfWireSDC1; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_SDC1v1_node.push_back( new TNode( Form( "SDC1v1_Node_%d", wire ),
					  Form( "SDC1v1_Node_%d", wire ),
					  "SDC1V1Tube",
					  wireGlobalPos.x()+localPos,
					  wireGlobalPos.y(),
					  wireGlobalPos.z(),
					  "rotSDC1V1", "void" ) );
    }
  }

  // SDC1 V2
  {
    const Int_t lid = gGeom.GetDetectorId("SDC1-V2");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t Z    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotV2", "rotV2", Matrix );
    new TTUBE( "SDC1V2Tube", "SDC1V2Tube", "void", Rmin, Rmax, Z );
    for( Int_t wire=1; wire<=NumOfWireSDC1; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_SDC1v2_node.push_back( new TNode( Form( "SDC1v2_Node_%d", wire ),
					  Form( "SDC1v2_Node_%d", wire ),
					  "SDC1V2Tube",
					  wireGlobalPos.x()+localPos,
					  wireGlobalPos.y(),
					  wireGlobalPos.z(),
					  "rotV2", "void" ) );
    }
  }

  // SDC1 X1
  {
    const Int_t lid = gGeom.GetDetectorId("SDC1-X1");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t Z    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotX1", "rotX1", Matrix );
    new TTUBE( "SDC1X1Tube", "SDC1X1Tube", "void", Rmin, Rmax, Z );
    for( Int_t wire=1; wire<=NumOfWireSDC1; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_SDC1x1_node.push_back( new TNode( Form( "SDC1x1_Node_%d", wire ),
					  Form( "SDC1x1_Node_%d", wire ),
					  "SDC1X1Tube",
					  wireGlobalPos.x()+localPos,
					  wireGlobalPos.y(),
					  wireGlobalPos.z(),
					  "rotX1", "void" ) );
    }
  }

  // SDC1 X2
  {
    const Int_t lid = gGeom.GetDetectorId("SDC1-X2");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t Z    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotX2", "rotX2", Matrix );
    new TTUBE( "SDC1X2Tube", "SDC1X2Tube", "void", Rmin, Rmax, Z );
    for( Int_t wire=1; wire<=NumOfWireSDC1; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_SDC1x2_node.push_back( new TNode( Form( "SDC1x2_Node_%d", wire ),
					  Form( "SDC1x2_Node_%d", wire ),
					  "SDC1X2Tube",
					  wireGlobalPos.x()+localPos,
					  wireGlobalPos.y(),
					  wireGlobalPos.z(),
					  "rotX2", "void" ) );
    }
  }

  // SDC1 U1
  {
    const Int_t lid = gGeom.GetDetectorId("SDC1-U1");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t Z    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotU1", "rotU1", Matrix );
    new TTUBE( "SDC1U1Tube", "SDC1U1Tube", "void", Rmin, Rmax, Z );
    for( Int_t wire=1; wire<=NumOfWireSDC1; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_SDC1u1_node.push_back( new TNode( Form( "SDC1u1_Node_%d", wire ),
					  Form( "SDC1u1_Node_%d", wire ),
					  "SDC1U1Tube",
					  wireGlobalPos.x()+localPos,
					  wireGlobalPos.y(),
					  wireGlobalPos.z(),
					  "rotU1", "void" ) );
    }
  }

  // SDC1 U2
  {
    const Int_t lid = gGeom.GetDetectorId("SDC1-U2");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t Z    = wireL/TMath::Cos(gGeom.GetTiltAngle(lid)*math::Deg2Rad())/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotU2", "rotU2", Matrix );
    new TTUBE( "SDC1U2Tube", "SDC1U2Tube", "void", Rmin, Rmax, Z );
    for( Int_t wire=1; wire<=NumOfWireSDC1; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireGlobalPos = gGeom.GetGlobalPosition( lid );
      m_SDC1u2_node.push_back( new TNode( Form( "SDC1u2_Node_%d", wire ),
					  Form( "SDC1u2_Node_%d", wire ),
					  "SDC1U2Tube",
					  wireGlobalPos.x()+localPos,
					  wireGlobalPos.y(),
					  wireGlobalPos.z(),
					  "rotU2", "void" ) );
    }
  }

  ConstructionDone(__func__);
  return true;
}

//______________________________________________________________________________
Bool_t
EventDisplay::ConstructSdcOut( void )
{
  const Double_t wireLSDC3  = 1152.0;
  const Double_t wireLSDC4Y = 1920.0;
  const Double_t wireLSDC4X = 1280.0;

  // SDC3 X1
  {
    const Int_t lid = gGeom.GetDetectorId("SDC3-X1");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireLSDC3/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotX1", "rotX1", Matrix );
    new TTUBE( "SDC3X1Tube", "SDC3X1Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<= NumOfWireSDC3; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireLocalPos( localPos, 0., 0. );
      ThreeVector wireGlobalPos = gGeom.Local2GlobalPos( lid, wireLocalPos );
      m_SDC3x1_node.push_back( new TNode( Form( "SDC3x1_Node_%d", wire ),
					  Form( "SDC3x1_Node_%d", wire ),
					  "SDC3X1Tube",
					  wireGlobalPos.x(),
					  wireGlobalPos.y(),
					  wireGlobalPos.z(),
					  "rotX1", "void" ) );
    }
  }

  // SDC3 X2
  {
    const Int_t lid = gGeom.GetDetectorId("SDC3-X2");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireLSDC3/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotX2", "rotX2", Matrix );
    new TTUBE( "SDC3X2Tube", "SDC3X2Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<= NumOfWireSDC3; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireLocalPos( localPos, 0., 0. );
      ThreeVector wireGlobalPos = gGeom.Local2GlobalPos( lid, wireLocalPos );
      m_SDC3x2_node.push_back( new TNode( Form( "SDC3x2_Node_%d", wire ),
					  Form( "SDC3x2_Node_%d", wire ),
					  "SDC3X2Tube",
					  wireGlobalPos.x(),
					  wireGlobalPos.y(),
					  wireGlobalPos.z(),
					  "rotX2", "void" ) );
    }
  }

  // SDC3 Y1
  {
    const Int_t lid = gGeom.GetDetectorId("SDC3-Y1");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireLSDC3/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotY1", "rotY1", Matrix );
    new TTUBE( "SDC3Y1Tube", "SDC3Y1Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<= NumOfWireSDC3; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireLocalPos( localPos, 0., 0. );
      ThreeVector wireGlobalPos = gGeom.Local2GlobalPos( lid, wireLocalPos );
      m_SDC3y1_node.push_back( new TNode( Form( "SDC3y1_Node_%d", wire ),
					  Form( "SDC3y1_Node_%d", wire ),
					  "SDC3Y1Tube",
					  wireGlobalPos.x(),
					  wireGlobalPos.y(),
					  wireGlobalPos.z(),
					  "rotY1", "void" ) );
    }
  }

  // SDC3 Y2
  {
    const Int_t lid = gGeom.GetDetectorId("SDC3-Y2");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireLSDC3/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotY2", "rotY2", Matrix );
    new TTUBE( "SDC3Y2Tube", "SDC3Y2Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<= NumOfWireSDC3; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireLocalPos( localPos, 0., 0. );
      ThreeVector wireGlobalPos = gGeom.Local2GlobalPos( lid, wireLocalPos );
      m_SDC3y2_node.push_back( new TNode( Form( "SDC3y2_Node_%d", wire ),
					  Form( "SDC3y2_Node_%d", wire ),
					  "SDC3Y2Tube",
					  wireGlobalPos.x(),
					  wireGlobalPos.y(),
					  wireGlobalPos.z(),
					  "rotY2", "void" ) );
    }
  }

  // SDC4 Y1
  {
    const Int_t lid = gGeom.GetDetectorId("SDC4-Y1");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireLSDC4Y/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ), Matrix );
    new TRotMatrix( "rotY1", "rotY1", Matrix );
    new TTUBE( "SDC4Y1Tube", "SDC4Y1Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<= NumOfWireSDC4Y; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireLocalPos( localPos, 0., 0. );
      ThreeVector wireGlobalPos = gGeom.Local2GlobalPos( lid, wireLocalPos );
      m_SDC4y1_node.push_back( new TNode( Form( "SDC4y1_Node_%d", wire ),
					  Form( "SDC4y1_Node_%d", wire ),
					  "SDC4Y1Tube",
					  wireGlobalPos.x(),
					  wireGlobalPos.y(),
					  wireGlobalPos.z(),
					  "rotY1", "void" ) );
    }
  }

  // SDC4 Y2
  {
    const Int_t lid = gGeom.GetDetectorId("SDC4-Y2");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireLSDC4Y/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotY2", "rotY2", Matrix );
    new TTUBE( "SDC4Y2Tube", "SDC4Y2Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<= NumOfWireSDC4Y; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireLocalPos( localPos, 0., 0. );
      ThreeVector wireGlobalPos = gGeom.Local2GlobalPos( lid, wireLocalPos );
      m_SDC4y2_node.push_back( new TNode( Form( "SDC4y2_Node_%d", wire ),
					  Form( "SDC4y2_Node_%d", wire ),
					  "SDC4Y2Tube",
					  wireGlobalPos.x(),
					  wireGlobalPos.y(),
					  wireGlobalPos.z(),
					  "rotY2", "void" ) );
    }
  }

  // SDC4 X1
  {
    const Int_t lid = gGeom.GetDetectorId("SDC4-X1");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireLSDC4X/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotX1", "rotX1", Matrix );
    new TTUBE( "SDC4X1Tube", "SDC4X1Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<= NumOfWireSDC4X; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireLocalPos( localPos, 0., 0. );
      ThreeVector wireGlobalPos = gGeom.Local2GlobalPos( lid, wireLocalPos );
      m_SDC4x1_node.push_back( new TNode( Form( "SDC4x1_Node_%d", wire ),
					  Form( "SDC4x1_Node_%d", wire ),
					  "SDC4X1Tube",
					  wireGlobalPos.x(),
					  wireGlobalPos.y(),
					  wireGlobalPos.z(),
					  "rotX1", "void" ) );
    }
  }

  // SDC4 X2
  {
    const Int_t lid = gGeom.GetDetectorId("SDC4-X2");
    Double_t Rmin = 0.0;
    Double_t Rmax = 0.01;
    Double_t L    = wireLSDC4X/2.;
    Double_t Matrix[NumOfMaxrixElements] = {};
    CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		   gGeom.GetRotAngle1( lid ),
		   gGeom.GetRotAngle2( lid ),
		   Matrix );
    new TRotMatrix( "rotX2", "rotX2", Matrix );
    new TTUBE( "SDC4X2Tube", "SDC4X2Tube", "void", Rmin, Rmax, L );
    for( Int_t wire=1; wire<= NumOfWireSDC4X; ++wire ){
      Double_t localPos = gGeom.CalcWirePosition( lid, wire );
      ThreeVector wireLocalPos( localPos, 0., 0. );
      ThreeVector wireGlobalPos = gGeom.Local2GlobalPos( lid, wireLocalPos );
      m_SDC4x2_node.push_back( new TNode( Form( "SDC4x2_Node_%d", wire ),
					  Form( "SDC4x2_Node_%d", wire ),
					  "SDC4X2Tube",
					  wireGlobalPos.x(),
					  wireGlobalPos.y(),
					  wireGlobalPos.z(),
					  "rotX2", "void" ) );
    }
  }

  ConstructionDone(__func__);
  return true;
}

//______________________________________________________________________________
Bool_t
EventDisplay::ConstructEmulsion( void )
{
  Double_t EmulsionX = 100.0/2.0;
  Double_t EmulsionY =  12.0/2.0; // Z
  Double_t EmulsionZ = 100.0/2.0; // Y
  Double_t rotMatEmulsion[NumOfMaxrixElements];

  new TBRIK( "emulsion_brik", "emulsion_brik", "void",
	     EmulsionX, EmulsionY, EmulsionZ );

  CalcRotMatrix( 0., 0., 0., rotMatEmulsion );
  new TRotMatrix( "rotEmulsion", "rotEmulsion", rotMatEmulsion );

  const Int_t lid = gGeom.GetDetectorId("Emulsion");
  ThreeVector GlobalPos = gGeom.GetGlobalPosition( lid );
#if Vertex
  static const Double_t zEmulsion = gGeom.GetLocalZ("Emulsion");
  static const Double_t zTarget   = gGeom.GetLocalZ("Target");
  static const Double_t z = zEmulsion - zTarget;
  m_EmulsionXZ_box = new TBox( z, -EmulsionX, z+EmulsionY*2., EmulsionX );
  m_EmulsionXZ_box->SetFillColor(BGColor);
  m_EmulsionXZ_box->SetLineColor(FGColor);
  m_EmulsionYZ_box = new TBox( z, -EmulsionZ, z+EmulsionY*2., EmulsionZ );
  m_EmulsionYZ_box->SetFillColor(BGColor);
  m_EmulsionYZ_box->SetLineColor(FGColor);
#endif

  ConstructionDone(__func__);
  return true;
}

//______________________________________________________________________________
Bool_t
EventDisplay::ConstructTarget( void )
{
  const Int_t IdTarget = gGeom.GetDetectorId("Target");

  const Double_t TargetX = 50.0/2.0;
  const Double_t TargetY = 30.0/2.0; // Z
  const Double_t TargetZ = 30.0/2.0; // Y
  Double_t rotMatTarget[NumOfMaxrixElements];

  new TBRIK( "target_brik", "target_brik", "void",
	     TargetX, TargetY, TargetZ );

  CalcRotMatrix( 0., 0., 0., rotMatTarget );
  new TRotMatrix( "rotTarget", "rotTarget", rotMatTarget );

  const Int_t lid = IdTarget;
  ThreeVector GlobalPos = gGeom.GetGlobalPosition( lid );
  m_target_node = new TNode( "target_node", "target_node", "target_brik",
			     GlobalPos.x(), GlobalPos.y(), GlobalPos.z(),
			     "rotTarget", "void");

#if Vertex
  m_TargetXZ_box = new TBox( -TargetY, -TargetX, TargetY, TargetX );
  m_TargetXZ_box->SetFillColor(BGColor);
  m_TargetXZ_box->SetLineColor(FGColor);
  m_TargetYZ_box = new TBox( -TargetY, -TargetZ, TargetY, TargetZ );
  m_TargetYZ_box->SetFillColor(BGColor);
  m_TargetYZ_box->SetLineColor(FGColor);
#endif

  ConstructionDone(__func__);
  return true;
}

//______________________________________________________________________________
Bool_t
EventDisplay::ConstructSSD( void )
{
#if Vertex
  const Int_t NBinX = (Int_t)( (MaxY-MinY) );
  const Int_t NBinY = (Int_t)( (MaxY-MinY) );
  const Int_t NBinZ = (Int_t)( (MaxZ-MinZ)/0.5 );
  m_SSD_y_hist = new TH2F( "SSD_y_hist", "SSD-Y DeltaE", NBinZ, MinZ, MaxZ, NBinY, MinY, MaxY );
  m_SSD_x_hist = new TH2F( "SSD_x_hist", "SSD-X DeltaE", NBinZ, MinZ, MaxZ, NBinX, MinX, MaxX );
  m_SSD_y_hist->SetMaximum(20000.);
  m_SSD_x_hist->SetMaximum(20000.);
#endif

  ConstructionDone(__func__);
  return true;
}

//______________________________________________________________________________
Bool_t
EventDisplay::ConstructFBH( void )
{
  const Int_t lid = gGeom.GetDetectorId("FBH");

  Double_t rotMatFBH[NumOfMaxrixElements] = {};
  Double_t FBHwallX =  7.5/2.0*NumOfSegFBH; // X
  Double_t FBHwallY =  2.0/2.0*2.0; // Z
  Double_t FBHwallZ = 35.0/2.0; // Y

  Double_t FBHSegX =  7.5/2.0; // X
  Double_t FBHSegY =  2.0/2.0; // Z
  Double_t FBHSegZ = 35.0/2.0; // Y

  Double_t overlap = 2.5;

  CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		 gGeom.GetRotAngle1( lid ),
		 gGeom.GetRotAngle2( lid ),
		 rotMatFBH );

  new TRotMatrix( "rotFBH", "rotFBH", rotMatFBH );
  ThreeVector  FBHwallPos = gGeom.GetGlobalPosition( lid );
  new TBRIK( "FBHwall_brik", "FBHwall_brik", "void",
	     FBHwallX, FBHwallY, FBHwallZ);
  m_FBHwall_node = new TNode( "FBHwall_node", "FBHwall_node", "FBHwall_brik",
			      FBHwallPos.x(),
			      FBHwallPos.y(),
			      FBHwallPos.z(),
			      "rotFBH", "void");

  m_FBHwall_node->SetVisibility(0);
  m_FBHwall_node->cd();

  new TBRIK( "FBHseg_brik", "FBHseg_brik", "void",
	     FBHSegX, FBHSegY, FBHSegZ );
  for( Int_t i=0; i<NumOfSegFBH; i++ ){
    ThreeVector fbhSegLocalPos( (-NumOfSegFBH/2.+i)*(FBHSegX*2.-overlap)+5.0/2.,
				(-(i%2)*2+1)*(FBHSegY-1.),
				0. );
    m_FBHseg_node.push_back( new TNode( Form( "FBHseg_node_%d", i ),
					Form( "FBHseg_node_%d", i ),
					"FBHseg_brik",
					fbhSegLocalPos.x(),
					fbhSegLocalPos.y(),
					fbhSegLocalPos.z() ) );
  }

#if Vertex
  static const Double_t zTarget = gGeom.GetLocalZ("Target");
  for( Int_t i=0; i<NumOfSegFBH; i++ ){
    Double_t x = (-NumOfSegFBH/2.+i)*(FBHSegX*2.-overlap)+5.0/2.;
    Double_t z = -(i%2)*2+1 + FBHwallPos.z() - zTarget;
    m_FBHseg_box.push_back( new TBox( z-1., x-FBHSegX, z+1., x+FBHSegX ) );
    m_FBHseg_box[i]->SetFillColor(BGColor);
    m_FBHseg_box[i]->SetLineColor(FGColor);
  }
#endif

  m_node->cd();
  ConstructionDone(__func__);
  return true;
}

//______________________________________________________________________________
Bool_t
EventDisplay::ConstructSCH( void )
{
  const Int_t lid = gGeom.GetDetectorId("SCH");

  Double_t rotMatSCH[NumOfMaxrixElements] = {};
  Double_t SCHwallX =   11.5/2.0*NumOfSegSCH; // X
  Double_t SCHwallY =    2.0/2.0*2.0; // Z
  Double_t SCHwallZ =  450.0/2.0; // Y

  Double_t SCHSegX =   11.5/2.0; // X
  Double_t SCHSegY =    2.0/2.0; // Z
  Double_t SCHSegZ =  450.0/2.0; // Y

  Double_t overlap = 1.0;

  CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		 gGeom.GetRotAngle1( lid ),
		 gGeom.GetRotAngle2( lid ),
		 rotMatSCH );

  new TRotMatrix( "rotSCH", "rotSCH", rotMatSCH );
  ThreeVector  SCHwallPos = gGeom.GetGlobalPosition( lid );
  new TBRIK( "SCHwall_brik", "SCHwall_brik", "void",
	     SCHwallX, SCHwallY, SCHwallZ);
  m_SCHwall_node = new TNode( "SCHwall_node", "SCHwall_node", "SCHwall_brik",
			      SCHwallPos.x(),
			      SCHwallPos.y(),
			      SCHwallPos.z(),
			      "rotSCH", "void");

  m_SCHwall_node->SetVisibility(0);
  m_SCHwall_node->cd();


  new TBRIK( "SCHseg_brik", "SCHseg_brik", "void",
	     SCHSegX, SCHSegY, SCHSegZ );
  for( Int_t i=0; i<NumOfSegSCH; i++ ){
    ThreeVector schSegLocalPos( (-NumOfSegSCH/2.+i)*(SCHSegX*2.-overlap)+10.5/2.,
				(-(i%2)*2+1)*SCHSegY-(i%2)*2+1,
				0. );
    m_SCHseg_node.push_back( new TNode( Form( "SCHseg_node_%d", i ),
					Form( "SCHseg_node_%d", i ),
					"SCHseg_brik",
					schSegLocalPos.x(),
					schSegLocalPos.y(),
					schSegLocalPos.z() ) );
  }

  m_node->cd();
  ConstructionDone(__func__);
  return true;
}

//______________________________________________________________________________
Bool_t
EventDisplay::ConstructTOF( void )
{
  const Int_t lid = gGeom.GetDetectorId("TOF");

  Double_t rotMatTOF[NumOfMaxrixElements] = {};
  Double_t TOFwallX =   80.0/2.0*NumOfSegTOF; // X
  Double_t TOFwallY =   30.0/2.0*2.0; // Z
  Double_t TOFwallZ = 1800.0/2.0; // Y

  Double_t TOFSegX =   80.0/2.0; // X
  Double_t TOFSegY =   30.0/2.0; // Z
  Double_t TOFSegZ = 1800.0/2.0; // Y

  Double_t overlap = 5.0;

  CalcRotMatrix( gGeom.GetTiltAngle( lid ),
		 gGeom.GetRotAngle1( lid ),
		 gGeom.GetRotAngle2( lid ),
		 rotMatTOF );

  new TRotMatrix( "rotTOF", "rotTOF", rotMatTOF );
  ThreeVector  TOFwallPos = gGeom.GetGlobalPosition( lid );
  new TBRIK( "TOFwall_brik", "TOFwall_brik", "void",
	     TOFwallX, TOFwallY, TOFwallZ);
  m_TOFwall_node = new TNode( "TOFwall_node", "TOFwall_node", "TOFwall_brik",
			      TOFwallPos.x(),
			      TOFwallPos.y(),
			      TOFwallPos.z(),
			      "rotTOF", "void" );

  m_TOFwall_node->SetVisibility(0);
  m_TOFwall_node->cd();


  new TBRIK( "TOFseg_brik", "TOFseg_brik", "void",
	     TOFSegX, TOFSegY, TOFSegZ);
  for( Int_t i=0; i<NumOfSegTOF; i++ ){
    ThreeVector tofSegLocalPos( (-NumOfSegTOF/2.+i)*(TOFSegX*2.-overlap)+75./2.,
				(-(i%2)*2+1)*TOFSegY-(i%2)*2+1,
				0. );
    m_TOFseg_node.push_back( new TNode( Form( "TOFseg_node_%d", i ),
					Form( "TOFseg_node_%d", i ),
					"TOFseg_brik",
					tofSegLocalPos.x(),
					tofSegLocalPos.y(),
					tofSegLocalPos.z() ) );
  }

  m_node->cd();
  ConstructionDone(__func__);
  return true;
}

//______________________________________________________________________________
void
EventDisplay::DrawInitTrack( void )
{
  m_canvas->cd();
  if( m_init_step_mark ) m_init_step_mark->Draw();

  m_canvas->Update();

}

#if 0
//______________________________________________________________________________
void
EventDisplay::DrawHitWire( Int_t lid, Int_t hit_wire, Bool_t range_check, Bool_t tdc_check )
{
  if( hit_wire<=0 ) return;

  TString node_name;

  const TString bcout_node_name[NumOfLayersBcOut]
    = { Form( "BC3x1_Node_%d", hit_wire ),
	Form( "BC3x2_Node_%d", hit_wire ),
	Form( "BC3u1_Node_%d", hit_wire ),
	Form( "BC3u2_Node_%d", hit_wire ),
	Form( "BC3v1_Node_%d", hit_wire ),
	Form( "BC3v2_Node_%d", hit_wire ),
	Form( "BC4x1_Node_%d", hit_wire ),
	Form( "BC4x2_Node_%d", hit_wire ),
	Form( "BC4u1_Node_%d", hit_wire ),
	Form( "BC4u2_Node_%d", hit_wire ),
	Form( "BC4v1_Node_%d", hit_wire ),
	Form( "BC4v2_Node_%d", hit_wire ) };

  const TString sft_node_name[NumOfLayersSFT]
    = { Form( "SFTu_Node_%d", hit_wire ),
	Form( "SFTv_Node_%d", hit_wire ),
	Form( "SFTx_Node_%d", hit_wire ) };
  const TString sdcin_node_name[NumOfLayersSdcIn]
    = { Form( "SDC1v1_Node_%d", hit_wire ),
	Form( "SDC1v2_Node_%d", hit_wire ),
	Form( "SDC1x1_Node_%d", hit_wire ),
	Form( "SDC1x2_Node_%d", hit_wire ),
	Form( "SDC1u1_Node_%d", hit_wire ),
	Form( "SDC1u2_Node_%d", hit_wire ) };

  const TString sdcout_node_name[NumOfLayersSdcOut]
    = { Form( "SDC3x1_Node_%d", hit_wire ),
	Form( "SDC3x2_Node_%d", hit_wire ),
	Form( "SDC3y1_Node_%d", hit_wire ),
	Form( "SDC3y2_Node_%d", hit_wire ),
	Form( "SDC4y1_Node_%d", hit_wire ),
	Form( "SDC4y2_Node_%d", hit_wire ),
	Form( "SDC4x1_Node_%d", hit_wire ),
	Form( "SDC4x2_Node_%d", hit_wire ) };

  switch ( lid ) {

    // SFT-UV
  case 7: case 8:
    if( hit_wire>NumOfSegSFT_UV ) return;
    node_name = sft_node_name[lid-7];
    break;
    // SFT-X
  case 10:
    lid--;
  case 9:
    if( hit_wire>NumOfSegSFT_X ) return;
    node_name = sft_node_name[lid-7];
    break;

    // SDC1
  case 1: case 2: case 3: case 4: case 5: case 6:
    if( hit_wire>NumOfWireSDC1 ) return;
    node_name = sdcin_node_name[lid-1];
    break;

    // SDC3
  case 31: case 32: case 33: case 34:
    if( hit_wire>NumOfWireSDC3 ) return;
    node_name = sdcout_node_name[lid-31];
    break;

    // SDC4Y
  case 35: case 36:
    if( hit_wire>NumOfWireSDC4Y ) return;
    node_name = sdcout_node_name[lid-31];
    break;

    // SDC4X
  case 37: case 38:
    if( hit_wire>NumOfWireSDC4X ) return;
    node_name = sdcout_node_name[lid-31];
    break;

    // BC3
  case 113: case 114: case 115: case 116: case 117: case 118:
    if( hit_wire>NumOfWireBC3 ) return;
    node_name = bcout_node_name[lid-113];
    break;

    // BC4
  case 119: case 120: case 121: case 122: case 123: case 124:
    if( hit_wire>NumOfWireBC4 ) return;
    node_name = bcout_node_name[lid-113];
    break;

  default:
    throw Exception( FUNC_NAME+" no such plane : "+TString::Itoa(lid,10) );
  }

  TNode *node = m_geometry->GetNode( node_name );
  if( !node ){
    // hddaq::cout << "#E " << FUNC_NAME << " "
    // 		<< "no such node : " << node_name << std::endl;
    return;
  }

  node->SetVisibility(1);
  if( range_check && tdc_check )
    node->SetLineColor(HitColor);
  else if( range_check && !tdc_check )
    node->SetLineColor(28);
  else
    node->SetLineColor(kWhite);

  m_canvas->cd();
  m_geometry->Draw("same");
  m_canvas->Update();

}
#endif

//______________________________________________________________________________
void
EventDisplay::DrawHitHodoscope( Int_t lid, Int_t seg, Int_t Tu, Int_t Td )
{
  TString node_name;

  if( seg<0 ) return;

  static const Int_t IdBH2    = gGeom.GetDetectorId("BH2");
  // static const Int_t IdFBH    = gGeom.GetDetectorId("FBH");
  static const Int_t IdSCH    = gGeom.GetDetectorId("SCH");
  static const Int_t IdTOF    = gGeom.GetDetectorId("TOF");

  if( lid == IdBH2 ){
    if( seg>=NumOfSegBH2 ) return;
    node_name = Form( "BH2seg_node_%d", seg );
  }
  // else if( lid == IdFBH ){
  //   if( seg>=NumOfSegFBH ) return;
  //   node_name = Form( "FBHseg_node_%d", seg );
  // }
  else if( lid == IdSCH ){
    if( seg>=NumOfSegSCH ) return;
    node_name = Form( "SCHseg_node_%d", seg );
  }
  else if( lid == IdTOF ){
    if( seg>=NumOfSegTOF ) return;
    node_name = Form( "TOFseg_node_%d", seg );
  }
  else {
    hddaq::cout << "#E " << FUNC_NAME << " "
		<< "no such plane : " << lid << std::endl;
    return;
  }

  TNode *node = m_geometry->GetNode( node_name );
  if( !node ){
    // hddaq::cout << "#E " << FUNC_NAME << " "
    // 		<< "no such node : " << node_name << std::endl;
    return;
  }

  node->SetVisibility(1);

  if( Tu>0 && Td>0 )
    node->SetLineColor( HitColor );
  else
    node->SetLineColor( kGreen );

  m_canvas->cd();
  m_geometry->Draw("same");
  m_canvas->Update();

#if Vertex
  if( lid == IdFBH ){
    m_canvas_vertex->cd(1);
    m_FBHseg_box[seg]->SetFillColor( HitColor );
    m_canvas_vertex->Update();
  }
#endif
}

//______________________________________________________________________________
void
EventDisplay::DrawBcOutLocalTrack( DCLocalTrack *tp )
{
  static const Double_t zK18Target = gGeom.GetLocalZ("K18Target");

  static const Double_t offsetZ = -3108.4;

  Double_t z0 = gGeom.GetLocalZ("BC3-X1") - 100.;
  Double_t x0 = tp->GetX( z0 ) + BeamAxis;
  Double_t y0 = tp->GetY( z0 );
  z0 += offsetZ;

  Double_t z1 = zK18Target + 100.;
  Double_t x1 = tp->GetX( z1 ) + BeamAxis;
  Double_t y1 = tp->GetY( z1 );
  z1 += offsetZ;

  ThreeVector gPos0( x0, y0, z0 );
  ThreeVector gPos1( x1, y1, z1 );

  TPolyLine3D *p = new TPolyLine3D(2);
  p->SetLineColor(TrackColor);
  p->SetLineWidth(TrackWidth);
  p->SetPoint( 0, gPos0.x(), gPos0.y(), gPos0.z() );
  p->SetPoint( 1, gPos1.x(), gPos1.y(), gPos1.z() );
  m_BcOutTrack.push_back(p);
  m_canvas->cd();
  p->Draw();

#if Vertex
  z0 = zK18Target + MinZ;
  z1 = zK18Target;
  x0 = tp->GetX( z0 ); y0 = tp->GetY( z0 );
  x1 = tp->GetX( z1 ); y1 = tp->GetY( z1 );
  z0 -= zK18Target;
  z1 -= zK18Target;
  {
    m_canvas_vertex->cd(1);
    TPolyLine *line = new TPolyLine(2);
    line->SetPoint( 0, z0, x0 );
    line->SetPoint( 1, z1, x1 );
    line->SetLineColor(TrackColor);
    line->SetLineWidth(TrackWidth);
    line->Draw();
    m_BcOutXZ_line.push_back( line );
  }
  {
    m_canvas_vertex->cd(2);
    TPolyLine *line = new TPolyLine(2);
    line->SetPoint( 0, z0, y0 );
    line->SetPoint( 1, z1, y1 );
    line->SetLineColor(TrackColor);
    line->SetLineWidth(TrackWidth);
    line->Draw();
    m_BcOutYZ_line.push_back( line );
  }
#endif

  m_canvas->cd();
  static TText *nhit = NewTText();
  static TText *chi2 = NewTText();
  nhit->SetText( 0.150, 0.500, Form("%d", tp->GetNHit()) );
  chi2->SetText( 0.150, 0.470, Form("%.3lf", tp->GetChiSquare()) );
  nhit->Draw();
  chi2->Draw();
}

//______________________________________________________________________________
void
EventDisplay::DrawBcOutLocalTrack( Double_t x0, Double_t y0, Double_t u0, Double_t v0 )
{
  static const Double_t zK18Target = gGeom.GetLocalZ("K18Target");

  const Double_t offsetZ = -3108.4;

  Double_t z0 = offsetZ;

  Double_t z1 = zK18Target + 100.;
  Double_t x1 = x0+u0*z1;
  Double_t y1 = y0+v0*z1;
  z1 += offsetZ;

  ThreeVector gPos0( x0+BeamAxis, y0, z0 );
  ThreeVector gPos1( x1+BeamAxis, y1, z1 );

  TPolyLine3D *p = new TPolyLine3D(2);
  p->SetLineColor(TrackColor);
  p->SetLineWidth(TrackWidth);
  p->SetPoint( 0, gPos0.x(), gPos0.y(), gPos0.z() );
  p->SetPoint( 1, gPos1.x(), gPos1.y(), gPos1.z() );
  m_BcOutTrack.push_back(p);
  m_canvas->cd();
  p->Draw();
  gPad->Update();

#if Vertex
  Double_t z2 = zK18Target + MinZ;
  z1 = zK18Target;
  Double_t x2 = x0+u0*z0, y2 = y0+v0*z0;
  x1 = x0+u0*z1; y1 = y0+v0*z1;
  z2 -= zK18Target;
  z1 -= zK18Target;
  {
    TPolyLine *line = new TPolyLine(2);
    line->SetPoint( 0, z2, x2 );
    line->SetPoint( 1, z1, x1 );
    line->SetLineColor(TrackColor);
    line->SetLineWidth(TrackWidth);
    m_BcOutXZ_line.push_back( line );
    m_canvas_vertex->cd(1);
    line->Draw();
  }
  {
    m_canvas_vertex->cd(2);
    TPolyLine *line = new TPolyLine(2);
    line->SetPoint( 0, z2, y2 );
    line->SetPoint( 1, z1, y1 );
    line->SetLineColor(TrackColor);
    line->SetLineWidth(TrackWidth);
    line->Draw();
    m_BcOutYZ_line.push_back( line );
  }
  m_canvas_vertex->Update();
#endif

}

//______________________________________________________________________________
void
EventDisplay::DrawSdcInLocalTrack( DCLocalTrack *tp )
{
#if SdcIn
  Double_t x0 = tp->GetX0(), y0 = tp->GetY0();
  // static const Int_t lid = gGeom.GetDetectorId("SSD1-Y1");
  static const Int_t lid = gGeom.GetDetectorId("SFT-U");
  static const Double_t zSFTU = gGeom.GetLocalZ( lid ) - 100.;
  Double_t x1 = tp->GetX( zSFTU ), y1 = tp->GetY( zSFTU );
  // static const Int_t lid = gGeom.GetDetectorId("SDC1-X1");
  // static const Double_t zSdc1x1 = gGeom.GetLocalZ( lid ) - 100.;
  // Double_t x1 = tp->GetX( zSdc1x1 ), y1 = tp->GetY( zSdc1x1 );

  ThreeVector gPos0( x0, y0, 0. );
  ThreeVector gPos1( x1, y1, zSFTU );
  //ThreeVector gPos1( x1+BeamAxis, y1, zSdc1x1 );

  TPolyLine3D *p = new TPolyLine3D(2);
  p->SetLineColor(TrackColor);
  p->SetLineWidth(TrackWidth);
  p->SetPoint( 0, gPos0.x(), gPos0.y(), gPos0.z() );
  p->SetPoint( 1, gPos1.x(), gPos1.y(), gPos1.z() );
  m_SdcInTrack.push_back(p);
  m_canvas->cd();
  p->Draw();
  gPad->Update();
#endif

#if Vertex
  Double_t z0 = zTarget;
  Double_t z1 = zTarget + MaxZ;
  x0 = tp->GetX( z0 ); y0 = tp->GetY( z0 );
  x1 = tp->GetX( z1 ); y1 = tp->GetY( z1 );
  z0 -= zTarget;
  z1 -= zTarget;
  {
    m_canvas_vertex->cd(1);
    TPolyLine *line = new TPolyLine(2);
    line->SetPoint( 0, z0, x0 );
    line->SetPoint( 1, z1, x1 );
    line->SetLineColor(TrackColor);
    line->SetLineWidth(TrackWidth);
    line->Draw();
    m_SdcInXZ_line.push_back( line );
  }
  {
    m_canvas_vertex->cd(2);
    TPolyLine *line = new TPolyLine(2);
    line->SetPoint( 0, z0, y0 );
    line->SetPoint( 1, z1, y1 );
    line->SetLineColor(TrackColor);
    line->SetLineWidth(TrackWidth);
    line->Draw();
    m_SdcInYZ_line.push_back( line );
  }
  m_canvas_vertex->Update();
#endif

  m_canvas->cd();
  static TText *nhit   = NewTText();
  static TText *chisqr = NewTText();
  nhit->SetText( 0.150, 0.400, Form("%d", tp->GetNHit()) );
  chisqr->SetText( 0.150, 0.370, Form("%lf", tp->GetChiSquare()) );
  nhit->Draw();
  chisqr->Draw();
}

//______________________________________________________________________________
void
EventDisplay::DrawSdcOutLocalTrack( DCLocalTrack *tp )
{
#if SdcOut
  Double_t x0 = tp->GetX0(), y0 = tp->GetY0();
  Double_t zSdcOut = gGeom.GetLocalZ( "VTOF" );
  Double_t x1 = tp->GetX( zSdcOut ), y1 = tp->GetY( zSdcOut );

  ThreeVector gPos0( x0, y0, 0. );
  ThreeVector gPos1( x1, y1, zSdcOut );

  TPolyLine3D *p = new TPolyLine3D(2);
  p->SetLineColor(TrackColor);
  p->SetLineWidth(TrackWidth);
  p->SetPoint( 0, gPos0.x(), gPos0.y(), gPos0.z() );
  p->SetPoint( 1, gPos1.x(), gPos1.y(), gPos1.z() );
  m_SdcOutTrack.push_back(p);
  m_canvas->cd();
  p->Draw();
  gPad->Update();
#endif

  m_canvas->cd();
  static TText *nhit = NewTText();
  static TText *chi2 = NewTText();
  nhit->SetText( 0.150, 0.300, Form("%d", tp->GetNHit()) );
  chi2->SetText( 0.150, 0.270, Form("%.3lf", tp->GetChiSquare()) );
  nhit->Draw();
  chi2->Draw();
}

//______________________________________________________________________________
void
EventDisplay::DrawSsdHit( Int_t lid, Int_t seg, Double_t de )
{
#if Vertex
  if( seg<0 ) return;

  const Double_t localPos = gGeom.CalcWirePosition( lid, seg );
  ThreeVector wireLocalPos( localPos, 0., 0. );
  const ThreeVector& wireGlobalPos = gGeom.Local2GlobalPos( lid, wireLocalPos );

  Double_t x = wireGlobalPos.x() - BeamAxis;
  Double_t y = wireGlobalPos.y();
  Double_t z = wireGlobalPos.z() - zTarget;
  if( lid == gGeom.GetDetectorId("SSD1-Y1") ){
    if( m_SSD_y_hist )
      ((TH2F*)m_SSD_y_hist)->Fill( z, y, de );
  }
  if( lid == gGeom.GetDetectorId("SSD1-X1") ){
    if( m_SSD_x_hist )
      ((TH2F*)m_SSD_x_hist)->Fill( z, x, de );
  }
  if( lid == gGeom.GetDetectorId("SSD1-Y2") ){
    if( m_SSD_y_hist )
      ((TH2F*)m_SSD_y_hist)->Fill( z, y, de );
  }
  if( lid == gGeom.GetDetectorId("SSD1-X2") ){
    if( m_SSD_x_hist )
      ((TH2F*)m_SSD_x_hist)->Fill( z, x, de );
  }
  if( lid == gGeom.GetDetectorId("SSD2-X1") ){
    if( m_SSD_x_hist )
      ((TH2F*)m_SSD_x_hist)->Fill( z, x, de );
  }
  if( lid == gGeom.GetDetectorId("SSD2-Y1") ){
    if( m_SSD_y_hist )
      ((TH2F*)m_SSD_y_hist)->Fill( z, y, de );
  }
  if( lid == gGeom.GetDetectorId("SSD2-X2") ){
    if( m_SSD_x_hist )
      ((TH2F*)m_SSD_x_hist)->Fill( z, x, de );
  }
  if( lid == gGeom.GetDetectorId("SSD2-Y2") ){
    if( m_SSD_y_hist )
      ((TH2F*)m_SSD_y_hist)->Fill( z, y, de );
  }
#endif
}

//______________________________________________________________________________
void
EventDisplay::DrawVertex( const ThreeVector& vertex )
{
#if Vertex
  del::DeleteObject( m_VertexPointXZ );
  del::DeleteObject( m_VertexPointYZ );

  Double_t x = vertex.x();
  Double_t y = vertex.y();
  Double_t z = vertex.z();

  m_VertexPointXZ = new TMarker( z, x, 34 );
  m_VertexPointXZ->SetMarkerSize(1);
  m_VertexPointXZ->SetMarkerColor(HitColor);
  m_VertexPointYZ = new TMarker( z, y, 34 );
  m_VertexPointYZ->SetMarkerSize(1);
  m_VertexPointYZ->SetMarkerColor(HitColor);

  m_canvas_vertex->cd(1);
  m_VertexPointXZ->Draw();
  m_canvas_vertex->cd(2);
  m_VertexPointYZ->Draw();
  m_canvas_vertex->Update();
#endif

}

//______________________________________________________________________________
void
EventDisplay::DrawMissingMomentum( const ThreeVector& mom, const ThreeVector& pos )
{
#if Vertex
  if( TMath::Abs( pos.x() )>40. ) return;
  if( TMath::Abs( pos.y() )>20. ) return;
  if( TMath::Abs( pos.z() )>20. ) return;

  Double_t z0 = pos.z();
  Double_t x0 = pos.x();
  Double_t y0 = pos.y();
  Double_t u0 = mom.x()/mom.z();
  Double_t v0 = mom.y()/mom.z();
  Double_t z1 = MaxZ;
  Double_t x1 = x0 + u0*z1;
  Double_t y1 = y0 + v0*z1;
  m_canvas_vertex->cd(1);
  m_MissMomXZ_line = new TPolyLine(2);
  m_MissMomXZ_line->SetPoint( 0, z0, x0 );
  m_MissMomXZ_line->SetPoint( 1, z1, x1 );
  m_MissMomXZ_line->SetLineColor(HitColor);
  m_MissMomXZ_line->SetLineWidth(TrackWidth);
  m_MissMomXZ_line->Draw();
  m_canvas_vertex->cd(2);
  m_MissMomYZ_line = new TPolyLine(2);
  m_MissMomYZ_line->SetPoint( 0, z0, y0 );
  m_MissMomYZ_line->SetPoint( 1, z1, y1 );
  m_MissMomYZ_line->SetLineColor(HitColor);
  m_MissMomYZ_line->SetLineWidth(TrackWidth);
  m_MissMomYZ_line->Draw();
  m_canvas_vertex->Update();
#endif
}

//______________________________________________________________________________
void
EventDisplay::DrawKuramaTrack( Int_t nStep, ThreeVector *StepPoint, Int_t Polarity )
{
  del::DeleteObject( m_kurama_step_mark );

  m_kurama_step_mark = new TPolyMarker3D( nStep );
  for( Int_t i=0; i<nStep; ++i ){
    m_kurama_step_mark->SetPoint( i,
				  StepPoint[i].x(),
				  StepPoint[i].y(),
				  StepPoint[i].z() );
  }

  Color_t color = Polarity>0 ? kMagenta : kCyan;
  m_kurama_step_mark->SetMarkerSize(10);
  m_kurama_step_mark->SetMarkerColor(color);
  m_kurama_step_mark->SetMarkerStyle(6);

  m_canvas->cd();
  m_kurama_step_mark->Draw();
  m_canvas->Update();

#if Vertex
  del::DeleteObject( m_KuramaMarkVertexX );
  m_KuramaMarkVertexX = new TPolyMarker( nStep );
  for( Int_t i=0; i<nStep; ++i ){
    Double_t x = StepPoint[i].x()-BeamAxis;
    Double_t z = StepPoint[i].z()-zTarget;
    m_KuramaMarkVertexX->SetPoint( i, z, x );
  }
  m_KuramaMarkVertexX->SetMarkerSize(0.4);
  m_KuramaMarkVertexX->SetMarkerColor(color);
  m_KuramaMarkVertexX->SetMarkerStyle(8);
  m_canvas_vertex->cd(1);
  m_KuramaMarkVertexX->Draw();
  del::DeleteObject( m_KuramaMarkVertexY );
  m_KuramaMarkVertexY = new TPolyMarker( nStep );
  for( Int_t i=0; i<nStep; ++i ){
    Double_t y = StepPoint[i].y();
    Double_t z = StepPoint[i].z()-zTarget;
    m_KuramaMarkVertexY->SetPoint( i, z, y );
  }
  m_KuramaMarkVertexY->SetMarkerSize(0.4);
  m_KuramaMarkVertexY->SetMarkerColor(color);
  m_KuramaMarkVertexY->SetMarkerStyle(8);
  m_canvas_vertex->cd(2);
  m_KuramaMarkVertexY->Draw();
  m_canvas_vertex->Update();
#endif
}

//______________________________________________________________________________
void
EventDisplay::DrawTarget( void )
{
  if( !m_target_node ){
    hddaq::cout << "#E " << FUNC_NAME << " "
		<< "node is null" << std::endl;
    return;
  }

  m_target_node->SetLineColor( kMagenta );
  m_canvas->cd();
  m_canvas->Update();

}

//______________________________________________________________________________
void
EventDisplay::DrawText( void )
{
  m_canvas->cd()->Clear();

  m_geometry->Draw();

  static const int eb_id = gUnpacker.get_fe_id("k18eb");

  const UInt_t event_number = gUnpacker.get_node_header( eb_id, DAQNode::k_event_number );
  const UInt_t run_number   = gUnpacker.get_node_header( eb_id, DAQNode::k_run_number   );
  const UInt_t data_size    = gUnpacker.get_node_header( eb_id, DAQNode::k_data_size    );
  const UInt_t unix_time    = gUnpacker.get_node_header( eb_id, DAQNode::k_unix_time    );
  TTimeStamp time_stamp( unix_time );
  time_stamp.Add( -TTimeStamp::GetZoneOffset() );

  static TText *text = NewTText();
  text->SetTextSize(0.040);
  text->DrawText( 0.050, 0.950, "Run#" );
  text->DrawText( 0.270, 0.950, "Event#" );
  text->DrawText( 0.600, 0.950, "Date" );
  text->SetTextAlign(32);
  text->DrawText( 0.220, 0.950, Form("%05d", run_number ) );
  text->DrawText( 0.500, 0.950, Form("%8d", event_number ) );
  text->DrawText( 0.950, 0.950, time_stamp.AsString("s") );
  text->SetTextSize(0.025);
  text->DrawText( 0.950, 0.900, Form("%d word", data_size) );
  text->SetTextAlign(12);
  text->DrawText( 0.700, 0.900, "EB Data Size" );
  text->DrawText( 0.050, 0.530, "BcOutTracking" );
  text->DrawText( 0.070, 0.500, "NHit" );
  text->DrawText( 0.070, 0.470, "Chisqr" );
  text->DrawText( 0.050, 0.430, "SdcInTracking" );
  text->DrawText( 0.070, 0.400, "NHit" );
  text->DrawText( 0.070, 0.370, "Chisqr" );
  text->DrawText( 0.050, 0.330, "SdcOutTracking" );
  text->DrawText( 0.070, 0.300, "NHit" );
  text->DrawText( 0.070, 0.270, "Chisqr" );
  text->DrawText( 0.700, 0.800, "K18Tracking" );
  text->DrawText( 0.720, 0.770, "pK18" );
  text->DrawText( 0.720, 0.740, "Delta" );
  text->DrawText( 0.720, 0.710, "Delta 3rd" );
  text->DrawText( 0.720, 0.680, "Target XY" );
  text->DrawText( 0.720, 0.650, "Target UV" );
  text->DrawText( 0.700, 0.580, "KuramaTracking" );
  text->DrawText( 0.720, 0.550, "NHit" );
  text->DrawText( 0.720, 0.520, "Chisqr" );
  text->DrawText( 0.720, 0.490, "pKurama" );
  text->DrawText( 0.720, 0.460, "PathLength" );
  text->DrawText( 0.720, 0.430, "Polarity" );
  text->DrawText( 0.720, 0.400, "Mass Square" );
  text->DrawText( 0.720, 0.370, "Target XY" );
  text->DrawText( 0.720, 0.340, "Target UV" );
  text->DrawText( 0.700, 0.270, "Reaction" );
  text->DrawText( 0.720, 0.240, "Theta" );
  text->DrawText( 0.720, 0.210, "Vertex" );
  text->DrawText( 0.720, 0.180, "Closest Distance" );
  text->DrawText( 0.720, 0.150, "Missing Mass" );

  // text->DrawText( 0.050, 0.200, Form("EMC   (  %.2lf  %.2lf  )",
  // 				     gEMC.X(), gEMC.Y() ));
  m_canvas->Update();
}

//______________________________________________________________________________
void
EventDisplay::DrawTrigger( const std::vector<Int_t>& flag )
{
  // hddaq::cout << FUNC_NAME << std::endl;
  m_canvas->cd();
  std::bitset<NumOfSegTFlag> flag_bit;
  Int_t flag_ps = 0;
  for( Int_t i=0, n=flag.size(); i<n; ++i ){
    // hddaq::cout << std::setw(10) << trigger::STriggerFlag[i] << " "
    // 		<< flag[i] << std::endl;
    if( flag[i] <= 0 ) continue;
    flag_bit.set(i);
    if( trigger::STriggerFlag.at(i).Contains("PS") ||
	trigger::STriggerFlag.at(i).EqualTo("BH2K") ){
      if( flag_ps == 0 ){
	flag_ps = i;
      } else {
	// hddaq::cout << "#W " << FUNC_NAME << " found multi trigger : "
	// 	    << std::setw(10) << trigger::STriggerFlag[i] << std::endl;
      }
    }
  }

  std::stringstream ss; ss << flag_bit;
  TString trig_all = ss.str();
  trig_all.ReplaceAll('1', '!');
  trig_all.ReplaceAll('0', '.');
  static TText *text = NewTText();
  TString trig = flag_ps!=0 ? trigger::STriggerFlag.at(flag_ps) : "Unknown";
  text->DrawText( 0.040, 0.900,
		  Form("Trigger  %s    %s", trig_all.Data(), trig.Data()) );
  m_canvas->Update();
}

//______________________________________________________________________________
void
EventDisplay::EndOfEvent( void )
{
  del::DeleteObject( m_init_step_mark );
  del::DeleteObject( m_BcOutXZ_line );
  del::DeleteObject( m_BcOutYZ_line );
  del::DeleteObject( m_SdcInXZ_line );
  del::DeleteObject( m_SdcInYZ_line );
  del::DeleteObject( m_BcOutTrack );
  del::DeleteObject( m_SdcInTrack );
  del::DeleteObject( m_SdcOutTrack );
  del::DeleteObject( m_kurama_step_mark );
  del::DeleteObject( m_VertexPointXZ );
  del::DeleteObject( m_VertexPointYZ );
  del::DeleteObject( m_MissMomXZ_line );
  del::DeleteObject( m_MissMomYZ_line );
  del::DeleteObject( m_KuramaMarkVertexX );
  del::DeleteObject( m_KuramaMarkVertexY );

  ResetVisibility();
}

//______________________________________________________________________________
void
EventDisplay::PrintHit( TString name, Double_t y, Bool_t hit_flag )
{
  m_canvas->cd();
  static TText* text = NewTText();
  text->SetTextColor(FGColor);
  text->DrawText( 0.050, y, name );
  if( hit_flag ){
    text->SetTextColor(kGreen);
    text->DrawText( 0.150, y, "Hit" );
  } else {
    text->SetTextColor(FGColor);
    text->DrawText( 0.150, y, "No Hit" );
  }
}

//______________________________________________________________________________
void
EventDisplay::PrintK18( K18TrackD2U *tp )
{
  m_canvas->cd();
  static std::vector<TText*> text =
    NewTTextArray( nK18Tracking, 32 );
  for( Int_t i=0, n=text.size(); i<n; ++i )
    text[i]->Draw();

  text[kP3rd]->SetText( 0.950, 0.770, Form("%lf", tp->P3rd()) );
  text[kDelta]->SetText( 0.950, 0.740, Form("%lf", tp->Delta()) );
  text[kDelta3rd]->SetText( 0.950, 0.710, Form("%lf", tp->Delta3rd()) );
  text[kK18XY]->SetText( 0.950, 0.680, Form("( %.2lf %.2lf )",
					    tp->Xtgt(),
					    tp->Ytgt()));
  text[kK18UV]->SetText( 0.950, 0.650, Form("( %.3lf %.3lf )",
					    tp->Utgt(),
					    tp->Vtgt()));
  m_canvas->Update();
}

//______________________________________________________________________________
void
EventDisplay::PrintKurama( KuramaTrack *tp, Double_t m2 )
{
  m_canvas->cd();
  static std::vector<TText*> text =
    NewTTextArray( nKuramaTracking, 32 );
  for( Int_t i=0, n=text.size(); i<n; ++i )
    text[i]->Draw();

  const ThreeVector& x = tp->PrimaryPosition();
  const ThreeVector& p = tp->PrimaryMomentum();

  text[kNHit]->SetText( 0.950, 0.550, Form("%d", static_cast<int>(tp->GetNHits())) );
  text[kChiSqr]->SetText( 0.950, 0.520, Form("%lf", tp->chisqr()) );
  text[kP]->SetText( 0.950, 0.490, Form("%lf", p.Mag()) );
  text[kPath]->SetText( 0.950, 0.460, Form("%lf", tp->PathLengthToTOF()) );
  TString q = tp->Polarity()>0. ? "+" : "-";
  text[kPolarity]->SetText( 0.950, 0.430, q );
  text[kMassSqr]->SetText( 0.950, 0.400, Form("%lf", m2) );
  text[kKuramaXY]->SetText( 0.950, 0.370, Form("( %.2lf %.2lf )",
					       x.X(), x.Y()));
  text[kKuramaUV]->SetText( 0.950, 0.340, Form("( %.3lf %.3lf )",
					       p.X()/p.Z(), p.Y()/p.Z()));
  m_canvas->Update();
}

//______________________________________________________________________________
void
EventDisplay::PrintReaction( Double_t theta, const ThreeVector& vertex, Double_t close, const LorentzVector& miss )
{
  m_canvas->cd();
  static std::vector<TText*> text =
    NewTTextArray( nReaction, 32 );
  for( Int_t i=0, n=text.size(); i<n; ++i )
    text[i]->Draw();

  text[kTheta]->SetText( 0.950, 0.240, Form("%.2lf", theta));
  text[kVertex]->SetText( 0.950, 0.210,
			  Form("( %.2lf %.2lf %.2lf )",
			       vertex.X(), vertex.Y(), vertex.Z()) );
  text[kClose]->SetText( 0.950, 0.180, Form("%.5lf", close) );
  Double_t MM = miss.Mag();
  const ThreeVector& p = miss.Vect();
  text[kMissMass]->SetText( 0.950, 0.150, Form("%.5lf", MM));
  text[kMissMom]->SetText( 0.950, 0.120,
			   Form("( %.2lf %.2lf %.2lf )",
				p.X(), p.Y(), p.Z()) );
  m_canvas->Update();
}

//______________________________________________________________________________
void
EventDisplay::ResetVisibility( TNode *& node, Color_t c )
{
  if( !node ) return;
  if( c==BGColor )
    node->SetVisibility(kFALSE);
  else
    node->SetLineColor(c);
}

//______________________________________________________________________________
void
EventDisplay::ResetVisibility( std::vector<TNode*>& node, Color_t c )
{
  for( Int_t i=0, n=node.size(); i<n; ++i ){
    ResetVisibility( node[i], c );
  }
}

//______________________________________________________________________________
void
EventDisplay::ResetVisibility( void )
{
  ResetVisibility( m_BC3x1_node );
  ResetVisibility( m_BC3x2_node );
  ResetVisibility( m_BC3v1_node );
  ResetVisibility( m_BC3v2_node );
  ResetVisibility( m_BC3u1_node );
  ResetVisibility( m_BC3u2_node );
  ResetVisibility( m_BC4u1_node );
  ResetVisibility( m_BC4u2_node );
  ResetVisibility( m_BC4v1_node );
  ResetVisibility( m_BC4v2_node );
  ResetVisibility( m_BC4x1_node );
  ResetVisibility( m_BC4x2_node );
  ResetVisibility( m_SDC1v1_node );
  ResetVisibility( m_SDC1v2_node );
  ResetVisibility( m_SDC1x1_node );
  ResetVisibility( m_SDC1x2_node );
  ResetVisibility( m_SDC1u1_node );
  ResetVisibility( m_SDC1u2_node );
  ResetVisibility( m_SDC3x1_node );
  ResetVisibility( m_SDC3x2_node );
  ResetVisibility( m_SDC3y1_node );
  ResetVisibility( m_SDC3y2_node );
  ResetVisibility( m_SDC4y1_node );
  ResetVisibility( m_SDC4y2_node );
  ResetVisibility( m_SDC4x1_node );
  ResetVisibility( m_SDC4x2_node );
  ResetVisibility( m_BH2seg_node, kWhite );
  ResetVisibility( m_FBHseg_node );
  ResetVisibility( m_SCHseg_node );
  ResetVisibility( m_TOFseg_node, kWhite );
  ResetVisibility( m_target_node );

#if Vertex
  if( m_SSD_y_hist ){
    m_SSD_y_hist->Reset();
    (dynamic_cast<TH2F*>(m_SSD_y_hist))->Fill( -9999., -9999., 0.1 );
  }
  if( m_SSD_x_hist ){
    m_SSD_x_hist->Reset();
    (dynamic_cast<TH2F*>(m_SSD_x_hist))->Fill( -9999., -9999., 0.1 );
  }
  for( Int_t i=0; i<NumOfSegFBH; i++ ){
    if( m_FBHseg_box[i] )
      m_FBHseg_box[i]->SetFillColor(BGColor);
  }
#endif

}

//______________________________________________________________________________
void
EventDisplay::DrawMomentum( Double_t momentum )
{
#if Hist
  m_hist_p->Fill( momentum );
  m_canvas_hist->cd(1);
  gPad->Modified();
  gPad->Update();
#endif
}

//______________________________________________________________________________
void
EventDisplay::DrawMassSquare( Double_t mass_square )
{
#if Hist
  m_hist_m2->Fill( mass_square );
  m_canvas_hist->cd(2);
  gPad->Modified();
  gPad->Update();
#endif
}

//______________________________________________________________________________
void
EventDisplay::CalcRotMatrix( Double_t TA, Double_t RA1, Double_t RA2, Double_t *rotMat )
{
  Double_t ct0 = TMath::Cos( TA*math::Deg2Rad()  );
  Double_t st0 = TMath::Sin( TA*math::Deg2Rad()  );
  Double_t ct1 = TMath::Cos( RA1*math::Deg2Rad() );
  Double_t st1 = TMath::Sin( RA1*math::Deg2Rad() );
  Double_t ct2 = TMath::Cos( RA2*math::Deg2Rad() );
  Double_t st2 = TMath::Sin( RA2*math::Deg2Rad() );

  Double_t rotMat1[3][3], rotMat2[3][3];

  /* rotation matrix which is same as DCGeomRecord.cc */

#if 1
  /* new definition of RA2 */
  rotMat1[0][0] =  ct0*ct2+st0*st1*st2;
  rotMat1[0][1] = -st0*ct2+ct0*st1*st2;
  rotMat1[0][2] =  ct1*st2;

  rotMat1[1][0] =  st0*ct1;
  rotMat1[1][1] =  ct0*ct1;
  rotMat1[1][2] = -st1;

  rotMat1[2][0] = -ct0*st2+st0*st1*ct2;
  rotMat1[2][1] =  st0*st2+ct0*st1*ct2;
  rotMat1[2][2] =  ct1*ct2;

#else
  /* old definition of RA2 */
  rotMat1[0][0] =  ct0*ct2-st0*ct1*st2;
  rotMat1[0][1] = -st0*ct2-ct0*ct1*st2;
  rotMat1[0][2] =  st1*st2;

  rotMat1[1][0] =  ct0*st2+st0*ct1*ct2;
  rotMat1[1][1] = -st0*st2+ct0*ct1*ct2;
  rotMat1[1][2] = -st1*ct2;

  rotMat1[2][0] =  st0*st1;
  rotMat1[2][1] =  ct0*st1;
  rotMat1[2][2] =  ct1;
#endif

  /* rotation matrix which rotate -90 deg at x axis*/
  rotMat2[0][0] =  1.0;
  rotMat2[0][1] =  0.0;
  rotMat2[0][2] =  0.0;

  rotMat2[1][0] =  0.0;
  rotMat2[1][1] =  0.0;
  rotMat2[1][2] =  1.0;

  rotMat2[2][0] =  0.0;
  rotMat2[2][1] = -1.0;
  rotMat2[2][2] =  0.0;

  for (Int_t i=0; i<9; i++)
    rotMat[i]=0.0;

  for (Int_t i=0; i<3; i++) {
    for (Int_t j=0; j<3; j++) {
      for (Int_t k=0; k<3; k++) {
  	//rotMat[3*i+j] += rotMat1[i][k]*rotMat2[k][j];
  	rotMat[i+3*j] += rotMat1[i][k]*rotMat2[k][j];
      }
    }
  }

}

//______________________________________________________________________________
void
EventDisplay::GetCommand( void ) const
{
  static Int_t stat   = 0;
  static Int_t Nevent = 0;
  static Int_t ev     = 0;

  if ( stat==1 && Nevent > 0 && ev<Nevent ) {
    ev++;
    return;
  }
  if ( ev==Nevent ) {
    stat=0;
    ev=0;
  }

  if ( stat == 0 ) {
    hddaq::cout << "q|n|p>" << std::endl;
    char ch;
    std::cin >> ch;
    if ( ch != '\n' )
      while( getchar() != '\n' );

    switch (ch) {
    case 'q':
      m_theApp->Terminate();
    case 'n':
      stat = 1;
      do {
	std::cout << "event#>";
	Int_t val = 0;
	std::cin >> val;
	Nevent = val;
      } while ( Nevent<=0 );
      hddaq::cout << "Continue " << Nevent << "event" << std::endl;
      break;
    case 'p':
      m_theApp->Run(kTRUE);
      break;
    }
  }
}

//______________________________________________________________________________
void
EventDisplay::Run( Bool_t flag )
{
  hddaq::cout << FUNC_NAME << " TApplication is running" << std::endl;

  m_theApp->Run(flag);
}

//______________________________________________________________________________
void
EventDisplay::SetStyle( void )
{
  gStyle->SetAxisColor(FGColor, "xyz");
  gStyle->SetLabelColor(FGColor, "xyz");
  gStyle->SetLabelFont(42);
  gStyle->SetTickLength(0.00, "xyz");
  gStyle->SetTitleColor(FGColor, "xyz");
  gStyle->SetPadBorderSize(0);
  gStyle->SetPadLeftMargin(0.050);
  gStyle->SetPadTopMargin(0.050);
  gStyle->SetTextColor(FGColor);
  gStyle->SetFrameLineColor(FGColor);
  gStyle->SetLineColor(FGColor);
}

//______________________________________________________________________________
void
EventDisplay::Update( void )
{
  m_canvas->Update();
}
