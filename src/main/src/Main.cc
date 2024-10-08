// author: Tomonori Takahashi

#include "Main.hh"

#include <iostream>
#include <algorithm>
#include <iomanip>
#include <iterator>
#include <ctime>
#include <sys/time.h>

#include <TStyle.h>
#include <TSystem.h>
#include <TThread.h>

#include <std_ostream.hh>
#include <UnpackerManager.hh>

#include "user_analyzer.hh"
//#include "DebugCounter.hh"

ClassImp(analyzer::Main)

namespace analyzer
{
  namespace
  {
    typedef hddaq::unpacker::UnpackerManager UnpackerManager;
    typedef hddaq::unpacker::GUnpacker       GUnpacker;

    void
    thread_function(void* arg)
    {
      Main::getInstance().run();
      return;
    }
  }
//_____________________________________________________________________________
Main&
Main::getInstance()
{
  static Main g_main;
  return g_main;
};

//_____________________________________________________________________________
Main::Main()
  : m_state(k_idle),
    m_argv(),
    m_thread(0),
    m_count(0),
    m_is_overwrite(false),
    m_is_batch(false),
    m_is_jsroot(false)
{
}

//_____________________________________________________________________________
Main::~Main()
{
//   if (m_thread)
//     delete m_thread;
//   m_thread = 0;
}

//_____________________________________________________________________________
void
Main::hoge() const
{
  std::cout << "#D Main::hoge()" << std::endl;
  return;
}

//_____________________________________________________________________________
const std::vector<std::string>&
Main::getArgv() const
{
  return m_argv;
}

//_____________________________________________________________________________
int
Main::getCounter() const
{
  return m_count;
}

//_____________________________________________________________________________
// void
// Main::initialize(int argc,
// 		 char* argv[])

// {
//   m_argv.assign(argv, argv+argc);
//   m_count = 0;
//   int ret = process_begin(m_argv);
//   if (ret!=0)
//     {
//       std::cout << "#D Main::initialize()\n"
// 		<< " got non-zero value from process_begin()"
// 		<< "\n  exit"
// 		<< std::endl;
//       std::exit(ret);
//     }
//   hoge();
//   return;
// }

//_____________________________________________________________________________
void
Main::initialize(const std::vector<std::string>& argV)
{
//   std::cout << "Main::initialize()" << std::endl;
//   std::copy(argV.begin(), argV.end(),
// 	    std::ostream_iterator<std::string>(std::cout, " " ));
//   std::cout << std::endl;
  m_argv = argV;
  m_count = 0;
  int ret = process_begin(m_argv);
  if (ret != 0) {
    std::cout << "#D Main::initialize()\n"
	      << " got non-zero value from process_begin()"
	      << "\n  exit"
	      << std::endl;
    std::exit(ret);
  }
  for (const auto& v : m_argv) {
    if (v.find("jsroot") != std::string::npos) {
      m_is_jsroot = true;
      break;
    }
  }
  return;
}

//_____________________________________________________________________________
bool
Main::isBatch() const
{
  return m_is_batch;
}

//_____________________________________________________________________________
bool
Main::isForceOverwrite() const
{
  return m_is_overwrite;
}

//_____________________________________________________________________________
bool
Main::isIdle() const
{
  return (m_state==k_idle);
}

//_____________________________________________________________________________
bool
Main::isJsRoot() const
{
  return m_is_jsroot;
}

//_____________________________________________________________________________
bool
Main::isRunning() const
{
  return (m_state==k_running);
}

//_____________________________________________________________________________
bool
Main::isZombie() const
{
  return (m_state==k_zombie);
}

//_____________________________________________________________________________
int
Main::join()
{
  std::cout << "#D Main::join()" << std::endl;
  return m_thread->Join();
}

//_____________________________________________________________________________
double
Main::get_dtime()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return ((double)(tv.tv_sec)+(double)(tv.tv_usec)*0.001*0.001);
}

//_____________________________________________________________________________
int
Main::run()
{
  UnpackerManager& g_unpacker = GUnpacker::get_instance();
//   if (g_unpacker.is_online())
  if (!m_is_batch)
    {
      for (;;)
	{
	  g_unpacker.initialize();
	  //	  double d3_last;
	  //	  d3_last = 0;
	  //	  double d4 = 0;
	  //	  int nevt=0;
	  for (;!g_unpacker.eof();++g_unpacker, ++m_count)
	    {
	      //	      double d0_last = get_dtime();
	      //	      if(nevt!=0)
	      //		{
	      //		  d4 += d0_last - d3_last;
	      //		}
	      //	      d3_last = d0_last;
	      if (isZombie())
		break;

	      if (isIdle())
		{
		  for(;;)
		    {
		      if (!isIdle())
			break;
		      continue;
		    }
		}

	      if (isRunning())
		{
		  // TThread::Lock();
		  //debug::ObjectCounter::Check();
		  int ret = process_event();
		  if( ret!=0 ){
		    std::cout << "#D1 analyzer::process_event() return " << ret << std::endl;
		    break;
		  }
		  // TThread::UnLock();
		}
// 	      double d1 = get_dtime();
// 	      d3 += d1 - d0;
//	      nevt++ ;
//	      if(d4>1)
//		{
//		  d4 = 0;
//		  std::cout<<"************************************************"<<std::endl;
//		  std::cout<<std::endl;
//		  std::cout<<std::endl;
//		  std::cout<<"Nevent : "<<nevt<<std::endl;
//		  getchar();
//		  std::cout<<std::endl;
//		  std::cout<<std::endl;
//		  std::cout<<std::endl;
//		  nevt = 0;
//		}
	      //	      std::cout<<"time2:" <<d1 - d0<<std::endl;
	    }

	  std::cout << "#D1 Main::run() exit loop"  << std::endl;
	  if (isZombie())
	    break;

	  if (!g_unpacker.is_online())
	    break;
	}
    }
  else
    {
      g_unpacker.initialize();
      for ( ; !g_unpacker.eof(); ++g_unpacker ){
	//debug::ObjectCounter::Check();
	int ret = process_event();
	if( ret!=0 ){
	  std::cout << "#D2 analyzer::process_event() return " << ret << std::endl;
	  break;
	}
      }
      std::cout << "#D2 Main::run() exit loop"  << std::endl;
    }
  process_end();

  std::cout << "#D Main::run() after process_end()"  << std::endl;
  return 0;
}

//_____________________________________________________________________________
void
Main::setBatchMode(bool flag)
{
  m_is_batch = flag;
  return;
}

//_____________________________________________________________________________
void
Main::setForceOverwrite(bool flag)
{
  m_is_overwrite = flag;
  return;
}

//_____________________________________________________________________________
void
Main::start()
{
  if (!m_thread)
    {
      m_thread = new TThread("MainThread",
			     &thread_function,
			     reinterpret_cast<void*>(0U));
      m_thread->Run();
    }

  m_state = k_running;

  return;
}

//_____________________________________________________________________________
void
Main::stat()
{
  // std::cout << "#D Main::stat()" << std::endl;

  static Int_t default_optstat = gStyle->GetOptStat();
  Int_t current_optstat = gStyle->GetOptStat();
  Int_t optstat = current_optstat != 0 ? 0 : default_optstat;

  hddaq::cout << "   gStyle::fOptStat "
	      << std::setw(10) << std::setfill('0') << std::right
	      << current_optstat << " -> "
	      << std::setw(10) << std::setfill('0') << std::right
	      << optstat << std::endl;
  gStyle->SetOptStat( optstat );

  // UnpackerManager& g_unpacker = GUnpacker::get_instance();
  // std::cout << "#D " << g_unpacker.get_counter()
  // 	    << " events unpacked" << std::endl;
  // TThread::Lock();
  // g_unpacker.show_event_number();
  // g_unpacker.show_summary(true);
  // TThread::UnLock();
  return;
}

//_____________________________________________________________________________
void
Main::stop()
{
  m_state = k_zombie;
  return;
}

//_____________________________________________________________________________
void
Main::suspend()
{
  m_state = k_idle;
  return;
}

}
