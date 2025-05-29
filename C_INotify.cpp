#include "C_INotify.hpp"

/////////////////////////////////////////////////////////////////////
// [ init ]
/////////////////////////////////////////////////////////////////////
int C_INotify::init(void (*fptr)(struct inotify_event*)){

   if(fd != C_INOTIFY_NOT_FOUND) return(C_INOTIFY_READY);

   if(fptr == nullptr) return(C_INOTIFY_ERROR);

   fd = inotify_init();

   if(fd == C_INOTIFY_NOT_FOUND){

      std::cerr << "C_INotify::init Error: " 
                << strerror(errno) 
                << std::endl;

      return(C_INOTIFY_ERROR);
   } 

   pCallback = fptr;

   return(C_INOTIFY_READY);
}

/////////////////////////////////////////////////////////////////////
// [ add_path ]
/////////////////////////////////////////////////////////////////////
int C_INotify::add_path(std::string sPath, uint32_t mask){

   if(fd == C_INOTIFY_NOT_FOUND || !sPath.length()) 
      return(C_INOTIFY_ERROR);

   int wd = inotify_add_watch(fd, sPath.c_str(), mask);

   if(wd == C_INOTIFY_NOT_FOUND){

      std::cerr << "C_INotify::add_path Error: " 
                << strerror(errno) 
                << std::endl;

      return(C_INOTIFY_ERROR);
   }

   VWatchlist.push_back(wd);

   return(C_INOTIFY_READY);
}

/////////////////////////////////////////////////////////////////////
// [ start ]
/////////////////////////////////////////////////////////////////////
int C_INotify::start(){

   if(bRunning) return(C_INOTIFY_READY);

   bRunning = true;

   pTRunning = new std::thread(&C_INotify::run, this);

   if(pTRunning){

      pTRunning->detach();
      return(C_INOTIFY_READY);

   }else{

      bRunning = false;
      return(C_INOTIFY_ERROR);
   }

   return(C_INOTIFY_READY);
}

/////////////////////////////////////////////////////////////////////
// [ stop ]
/////////////////////////////////////////////////////////////////////
int C_INotify::stop(){

   if(bRunning){

      bRunning = false;

      delete pTRunning;

      pTRunning = nullptr;
   }

   return(C_INOTIFY_READY);
}

/////////////////////////////////////////////////////////////////////
// [ run ]
/////////////////////////////////////////////////////////////////////
int C_INotify::close(){

   stop();

   if(fd == C_INOTIFY_NOT_FOUND) return(C_INOTIFY_READY);

   for(auto& wd : VWatchlist){
      inotify_rm_watch(fd, wd);
   }

   ::close(fd);

   fd = C_INOTIFY_NOT_FOUND;

   return(C_INOTIFY_READY);
}

/////////////////////////////////////////////////////////////////////
// [ run ]
/////////////////////////////////////////////////////////////////////
void C_INotify::run(){

   std::array<char, BUF_LEN> aBuffer;

   while(bRunning){

      int index = 0;

      int length = read(fd, aBuffer.data(), BUF_LEN);

      if(length < 0){

        std::cerr << "C_INotify::run Error: " 
                    << strerror(errno) 
                    << std::endl;
        continue;
      }

      while(length > index){

         struct inotify_event* event = (struct inotify_event*) &aBuffer[index];

         pCallback(event);

         if(event->mask & IN_CREATE){
            
            if(event->mask & IN_ISDIR){

            }else{

            }

         }else 
         if(event->mask & IN_DELETE){

            if(event->mask & IN_ISDIR){

            }else{

            }

         }else 
         if(event->mask & IN_MODIFY){

            if(event->mask & IN_ISDIR){

            }else{

            }

         }

         index += EVENT_SIZE + event->len;
      }
   }
}