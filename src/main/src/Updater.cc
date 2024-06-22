// Author: Tomonori Takahashi

#include "Updater.hh"

#include <iostream>
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <exception>
#include <stdexcept>

#include <TROOT.h>
#include <TFolder.h>
#include <TList.h>
#include <TColor.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TROOT.h>
#include <TThread.h>
#include <TMutex.h>
#include <TPaveStats.h>
#include <TSystem.h>

#include "Main.hh"
#include "lexical_cast.hh"
#include "Controller.hh"

ClassImp(hddaq::gui::Updater)

namespace hddaq
{
  namespace gui
  {

    namespace
    {
      using analyzer::Main;

      //_______________________________________________________________________
      void
      thread_function(void* arg)
      {
	Updater::getInstance().run();
	return;
      }

      //_______________________________________________________________________
      void
      reset_all_hist(TFolder* f)
      {
	if (!f)
	  return;

	TCollection* c = f->GetListOfFolders();
	if (!c)
	  return;
	TIterator* itr = c->MakeIterator();
	for (TObject* obj = itr->Next(); obj; obj = itr->Next())
	  {
	    const std::string& class_name = obj->ClassName();
// 	    std::cout << "#D reset_all_hist() " << class_name
// 		      << " " << obj->GetName() << std::endl;
	    if (class_name=="TFolder")
	      reset_all_hist(dynamic_cast<TFolder*>(obj));

	    if (0==class_name.find("TH1"))
	      {
		TH1* h = dynamic_cast<TH1*>(obj);
 		h->Reset();
	      }
	    else if(0==class_name.find("TH2"))
	      {
		TH2* h = dynamic_cast<TH2*>(obj);
 		h->Reset();
	      }
	    else if(0==class_name.find("TH3"))
	      {
		TH3* h = dynamic_cast<TH3*>(obj);
 		h->Reset();
	      }
	  }

	return;
      }

      //_______________________________________________________________________
      bool
      reset_hist(TPad* pad)
      {
	if (!pad)
	  return false;
	bool logx = pad->GetLogx();
	bool logy = pad->GetLogy();
	bool logz = pad->GetLogz();

	TIter primitive_iterator = (pad->GetListOfPrimitives());
	for (;;)
	  {
	    TObject* obj = primitive_iterator.Next();
	    if (!obj)
	      return false;
	    const std::string class_name = obj->ClassName();
// 	    std::cout << "#D Updater::reset()\n"
// 		      << "  " << class_name << std::endl;
	    if (class_name=="TPad")
	      reset_hist(dynamic_cast<TPad*>(obj));

	    if (0==class_name.find("TH1"))
	      {
		TH1* h = dynamic_cast<TH1*>(obj);
		h->Reset();
	      }
	    else if(0==class_name.find("TH2"))
	      {
		TH2* h = dynamic_cast<TH2*>(obj);
		h->Reset();
	      }
	    else if(0==class_name.find("TH3"))
	      {
		TH3* h = dynamic_cast<TH3*>(obj);
		h->Reset();
	      }
	  }
	pad->SetLogx(logx);
	pad->SetLogy(logy);
	pad->SetLogz(logz);
	return true;
      }

    }


//______________________________________________________________________________
Updater&
Updater::getInstance()
{
  static Updater g_updater;
  return g_updater;
}

//______________________________________________________________________________
Updater::Updater()
  : m_refresh_interval(1),
    m_mode(k_clicked),
    m_state(k_idle),
    m_thread(0),
    m_during_update(false)
    //    m_mutex(new TMutex)
{
}

//______________________________________________________________________________
Updater::~Updater()
{
  //  if (m_mutex)
  //    delete m_mutex;
  //  m_mutex = 0;
}

//______________________________________________________________________________
bool
Updater::isUpdating()
{
  TThread::Lock();
  Updater& g = Updater::getInstance();
  bool ret = g.m_during_update;
  TThread::UnLock();
  return ret;
}

//______________________________________________________________________________
void
Updater::hoge() const
{
  std::cout << "#D Updater::hoge() " << std::endl;
  return;
}

//______________________________________________________________________________
int
Updater::getCounter() const
{
  return Main::getInstance().getCounter();
}

//______________________________________________________________________________
double
Updater::getInterval() const
{
  return m_refresh_interval;
}

//______________________________________________________________________________
bool
Updater::isIdle() const
{
  return (m_state==k_idle);
}

//______________________________________________________________________________
bool
Updater::isRunning() const
{
  return (m_state==k_running);
}

//______________________________________________________________________________
bool
Updater::isZombie() const
{
  return (m_state==k_zombie);
}

//______________________________________________________________________________
int
Updater::join()
{
  std::cout << "#D Updater::join()" << std::endl;
  return m_thread->Join();
}

//______________________________________________________________________________
// int
// Updater::lock()
// {
//   Updater& g = getInstance();
//   int ret =g.m_mutex->Lock();
//   return ret;
// }
//______________________________________________________________________________
void
Updater::refresh()
{
  TThread::Lock();
  TIter canvas_iterator(gROOT->GetListOfCanvases());
  for (;;)
    {
      TCanvas* canvas = dynamic_cast<TCanvas*>(canvas_iterator.Next());
      if (!canvas){ break; }

      canvas->UseCurrentStyle();
      canvas->Modified();
      canvas->Update();
    }

  TThread::UnLock();
  return;
}

//______________________________________________________________________________
void
Updater::reset()
{
  TThread::Lock();
  TIter canvas_iterator(gROOT->GetListOfCanvases());
  for (;;)
    {
      TCanvas* canvas = dynamic_cast<TCanvas*>(canvas_iterator.Next());
      if (!canvas)
	break;
//       std::cout << "#D canvas = " << canvas->GetName() << std::endl;
      bool logx = canvas->GetLogx();
      bool logy = canvas->GetLogy();
      bool logz = canvas->GetLogz();

      TIter pad_iterator(canvas->GetListOfPrimitives());
      for (TPad* pad = canvas;
	   reset_hist(pad);
	   pad=dynamic_cast<TPad*>(pad_iterator.Next()));

      canvas->UseCurrentStyle();

      canvas->SetLogx(logx);
      canvas->SetLogy(logy);
      canvas->SetLogz(logz);

      canvas->Modified();
      canvas->Update();
    }
  TThread::UnLock();
  return;
}

//______________________________________________________________________________
void
Updater::resetAll()
{
  TThread::Lock();
  std::cout << "#D Updater::resetAll()" << std::endl;
  TList* l = gROOT->GetListOfBrowsables();
  TFolder* root_folder = dynamic_cast<TFolder*>(l->FindObject("root"));
  if (!root_folder)
    {
      std::cerr << "#E Updater::resetAll() cannot find the TFolder \"root\""
		<< std::endl;
      return;
    }
  root_folder->FindObject("ROOT Memory");
  TFolder* root_memory
    = dynamic_cast<TFolder*>(root_folder->FindObject("ROOT Memory"));
  if (!root_memory)
    {
      std::cerr << "#E Updater::resetAll() cannot find the TFolder"
		<< " \"ROOT Memory\""
		<< std::endl;
    }
  reset_all_hist(root_memory);
  TThread::UnLock();
  return;
}

//______________________________________________________________________________
int
Updater::run()
{
  Main& g_main = Main::getInstance();
  for (;;)
    {
      // std::cout << __func__ << " " << __LINE__ << std::endl;
      // gSystem->ProcessEvents();
      if (g_main.isZombie())
	{
// 	  std::cout << "#D Updater detects end of main" << std::endl;
	  m_state = k_zombie;
	  break;
	}
      if (g_main.isIdle())
	{
// 	  std::cout << "#D Updater detects idling of Main" << std::endl;
	  m_state = k_idle;
	  ::usleep(1);
	}

      if (g_main.isRunning())
	{
// 	  std::cout << "#D Updater detects running of Main" << std::endl;
	  m_state = k_running;
	}

      if (isRunning())
	{
// 	  if (loop%100==0)
// 	    std::cout << "#D Updater::run() loop = " << loop << std::endl;
// 	  ++loop;

	  if (wait()<0)
	    continue;

	  update();
	}
    }
  std::cout << "#D Updater exited loop" << std::endl;
  return 0;
}

//______________________________________________________________________________
void
Updater::setInterval(double interval)
{
  static std::vector<std::string> mode;
  if (mode.empty())
    {
      mode.resize(k_n_mode);
      mode[k_seconds] = "seconds";
      mode[k_events]  = "events";
      mode[k_clicked] = "clicked";
    }

  std::string interval_str = d2a(interval);
  std::cout << "#D Updater::set_interval() "
	    << ((m_mode==k_clicked) ? "" : interval_str)
	    << " " << mode[m_mode] << std::endl;
  m_refresh_interval = interval;
  return;
}

//______________________________________________________________________________
void
Updater::setUpdateMode(int mode)
{
  m_mode = mode;
  return;
}

//______________________________________________________________________________
void
Updater::setUpdating(bool flag)
{
  TThread::Lock();
  Updater& g = Updater::getInstance();
  g.m_during_update = flag;
  TThread::UnLock();
}

//______________________________________________________________________________
void
Updater::start()
{
  TThread::Lock();
  //  m_mutex->Lock();
  if (!m_thread)
    {
      m_thread = new TThread("UpdaterThread",
			     &thread_function,
			     reinterpret_cast<void*>(0U));
      m_thread->Run();
    }
  m_state = k_running;
  //  m_mutex->UnLock();
  TThread::UnLock();
  return;
}

//______________________________________________________________________________
void
Updater::stop()
{
  //  m_mutex->Lock();
  TThread::Lock();
  m_state = k_idle;
  //  m_mutex->UnLock();
  TThread::UnLock();
  return;
}

//______________________________________________________________________________
// int
// Updater::unlock()
// {
//   Updater& g = getInstance();
//   int ret = g.m_mutex->UnLock();
//   return ret;
// }

//______________________________________________________________________________
void
Updater::update()
{
  if(this->isUpdating()){return;}
  this->setUpdating(true);

  TColor* c = gROOT->GetColor(kRed);
  Controller& g_controller = Controller::getInstance();
  g_controller.disableCommand(Controller::k_refresh);
  g_controller.setIntervalTextColor(c);

  TIter canvas_iterator(gROOT->GetListOfCanvases());
  while( true )
    {
      //      std::cout << "#D " << i <<  " Updater::update()" << std::endl;
      TCanvas* canvas = dynamic_cast<TCanvas*>(canvas_iterator.Next());
      if (!canvas){ break; }


      //      std::cout << "#C " << i << " " << canvas->GetName() << std::endl;

      TIter pad_iterator(canvas->GetListOfPrimitives());
      while( true )
	{
	  TPad* pad = dynamic_cast<TPad*>(pad_iterator.Next());
	  if (!pad){ break; }
	  //	  std::cout << "#pad " << pad->GetName() << std::endl;
	  pad->Modified();
	  pad->Update();
	}

      canvas->Modified();
      canvas->Update();
    }

  g_controller.enableCommand(Controller::k_refresh);
  g_controller.setIntervalTextColor();

  this->setUpdating(false);

  return;
}

//______________________________________________________________________________
int
Updater::wait()
{

  const int n_events = m_refresh_interval;
  int ret = 0;
  switch (m_mode)
    {
    case k_seconds:
      {
//       std::cout << "#D Updater::wait() " << m_refresh_interval
// 		<< " seconds" << std::endl;
      if (m_refresh_interval>=0)
	{
	  ::usleep(static_cast<unsigned int>
		   (m_refresh_interval * 1.0e6));
	  ret = 0;
	  break;
	}
      }
    case k_events:
      {
//       std::cout << "#D Updater::wait() " << n_events
// 		<< " events" << std::endl;
      if (n_events>0)
	{
	  if (0==(Main::getInstance().getCounter() & n_events))
	    ret = 0;
	  else
	    ret = -1;
	  break;
	}
      }
    case k_clicked:
      {
	TColor* c = gROOT->GetColor(kGray);
	Controller::getInstance().setIntervalTextColor(c);

//       std::cout << "#D Updater::wait() clicked" << std::endl;

      }
    default:
      {
	::sleep(1);
	ret = -1;
	break;
      }
    }

  return ret;
}

  }
}
