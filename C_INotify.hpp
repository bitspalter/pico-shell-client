//////////////////////////////////////////////////////////////////////////////////
// [ C_PicoShell_Header ]
//////////////////////////////////////////////////////////////////////////////////
//
// 
// [::Last modi: 23.05.25 L.ey (µ~)::]
//
//
#ifndef _C_INOTIFY_H_
  #define _C_INOTIFY_H_

  #include <iostream>
  #include <string>
  #include <cstring>
  #include <array>
  #include <vector>
  #include <thread>

  #include <sys/types.h>
  #include <sys/inotify.h>
  #include <errno.h>
  #include <unistd.h> // read()
  
/////////////////////////////////////////////////////////////////////////
// CONST
/////////////////////////////////////////////////////////////////////////

  const int C_INOTIFY_ERROR     =  0;
  const int C_INOTIFY_READY     =  1;
  const int C_INOTIFY_NOT_FOUND = -1;

  #define EVENT_SIZE (sizeof(struct inotify_event))
  #define BUF_LEN    (1024 * (EVENT_SIZE + 16))

/////////////////////////////////////////////////////////////////////////
// CLASS
/////////////////////////////////////////////////////////////////////////

class C_INotify {

   public:

      C_INotify(){};
     ~C_INotify(){close();};

      int init(void (*pCallback)(struct inotify_event*));
      int add_path(std::string sPath, uint32_t mask);

      int start();
      int stop();

      int close();

   private:

      int fd = {C_INOTIFY_NOT_FOUND};

      std::vector<int> VWatchlist;

      std::thread* pTRunning = {nullptr};
      bool bRunning = {false};
      void run();

      ////////////////////////////////////

      void (*pCallback)(struct inotify_event*);
};

#endif //_C_INOTIFY_H_