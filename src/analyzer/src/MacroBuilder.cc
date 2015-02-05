#include"MacroBuilder.hh"
#include"HistMaker.hh"
#include"Main.hh"

#include<vector>
#include<string>

#include<TMacro.h>

// Get execute path ---------------------------------------------
std::string getMacroPath()
{
  const std::vector<std::string> argv 
    = analyzer::Main::getInstance().getArgv();
  std::string str_process_name = argv[0];
  int n = str_process_name.find("bin");
  return str_process_name.substr(0, n) + "src/analyzer/macro/";
}

// hoge ---------------------------------------------------------
// This is just an exsample of macros
TObject* hoge()
{
  std::string path = getMacroPath() + "hoge.C";
  TMacro* m = new TMacro(path.c_str());
  m->SetName("hoge");
  return m;
}

// clear_canvas --------------------------------------------------
TObject* clear_canvas()
{
  std::string path = getMacroPath() + "clear_canvas.C";
  TMacro* m = new TMacro(path.c_str());
  m->SetName("clear_canvas");
  return m;
}

// split22 -------------------------------------------------------
TObject* split22()
{
  std::string path = getMacroPath() + "split22.C";
  TMacro* m = new TMacro(path.c_str());
  m->SetName("split22");
  return m;
}

// split32 -------------------------------------------------------
TObject* split32()
{
  std::string path = getMacroPath() + "split32.C";
  TMacro* m = new TMacro(path.c_str());
  m->SetName("split32");
  return m;
}

// split33 -------------------------------------------------------
TObject* split33()
{
  std::string path = getMacroPath() + "split33.C";
  TMacro* m = new TMacro(path.c_str());
  m->SetName("split33");
  return m;
}

// dispBH1 -------------------------------------------------------
TObject* dispBH1()
{
  std::string path = getMacroPath() + "dispBH1.C";
  TMacro* m = new TMacro(path.c_str());
  m->SetName("dispBH1");
  return m;
}

// dispBH2 -------------------------------------------------------
TObject* dispBH2()
{
  std::string path = getMacroPath() + "dispBH2.C";
  TMacro* m = new TMacro(path.c_str());
  m->SetName("dispBH2");
  return m;
}

// dispACs_SFV -------------------------------------------------------
TObject* dispACs_SFV()
{
  std::string path = getMacroPath() + "dispACs_SFV.C";
  TMacro* m = new TMacro(path.c_str());
  m->SetName("dispACs_SFV");
  return m;
}

// dispTOF -------------------------------------------------------
TObject* dispTOF()
{
  std::string path = getMacroPath() + "dispTOF.C";
  TMacro* m = new TMacro(path.c_str());
  m->SetName("dispTOF");
  return m;
}

// dispLC -------------------------------------------------------
TObject* dispLC()
{
  std::string path = getMacroPath() + "dispLC.C";
  TMacro* m = new TMacro(path.c_str());
  m->SetName("dispLC");
  return m;
}

// dispBC3 -------------------------------------------------------
TObject* dispBC3()
{
  std::string path = getMacroPath() + "dispBC3.C";
  TMacro* m = new TMacro(path.c_str());
  m->SetName("dispBC3");
  return m;
}

// dispBC4 -------------------------------------------------------
TObject* dispBC4()
{
  std::string path = getMacroPath() + "dispBC4.C";
  TMacro* m = new TMacro(path.c_str());
  m->SetName("dispBC4");
  return m;
}

// dispSDC2 -------------------------------------------------------
TObject* dispSDC2()
{
  std::string path = getMacroPath() + "dispSDC2.C";
  TMacro* m = new TMacro(path.c_str());
  m->SetName("dispSDC2");
  return m;
}

// dispHDC -------------------------------------------------------
TObject* dispHDC()
{
  std::string path = getMacroPath() + "dispHDC.C";
  TMacro* m = new TMacro(path.c_str());
  m->SetName("dispHDC");
  return m;
}

// dispSDC3 -------------------------------------------------------
TObject* dispSDC3()
{
  std::string path = getMacroPath() + "dispSDC3.C";
  TMacro* m = new TMacro(path.c_str());
  m->SetName("dispSDC3");
  return m;
}

// dispSDC4 -------------------------------------------------------
TObject* dispSDC4()
{
  std::string path = getMacroPath() + "dispSDC4.C";
  TMacro* m = new TMacro(path.c_str());
  m->SetName("dispSDC4");
  return m;
}

// dispHitPat -----------------------------------------------------
TObject* dispHitPat()
{
  std::string path = getMacroPath() + "dispHitPat.C";
  TMacro* m = new TMacro(path.c_str());
  m->SetName("dispHitPat");
  return m;
}
