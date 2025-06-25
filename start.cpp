#include <errno.h>
#include <iostream>
#include <string>

/////////////////

#include "C_Serial.hpp"
#include "C_PicoShell.hpp"
#include "C_INotify.hpp"

C_Serial CSerial;

C_PicoShell CShell(&CSerial);

C_INotify CINotify;

void NCallback(struct inotify_event* pEvent);

std::string sFile = "ttyACM0";
std::string sPath = "/dev/";

/////////////////////////////////////////////////////////////////////
// [ main ]
/////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]){

   int opt;

   while((opt = getopt(argc, argv, "d:ab")) != -1){
      switch(opt){
         case 'd': sFile = optarg; break;
         case '?': break;
      }
   }

   ///////////////////////////////////////////////////////////////

   if(CINotify.init(NCallback) == C_INOTIFY_READY){

      if(CINotify.add_path(std::string("/dev"), 
                           IN_MODIFY|IN_CREATE|IN_DELETE) == C_INOTIFY_READY){

         if(CINotify.start() == C_INOTIFY_READY){
            std::cout << "CINotify Active" << std::endl;
         }
      }
   }

   ///////////////////////////////////////////////////////////////

   sPath += sFile;

   if(CSerial.open(sPath, B9600) != C_SERIAL_READY){
      //std::cout << "CSerial.open - ERROR " << std::endl;
   }else{

      if(CShell.start() != C_PICOSHELL_READY){
         //std::cout << "CShell.start - ERROR " << std::endl;
      }else{


      }
   }

   //////////////

   std::cout << CLEAR;

   cursorXY(0, 0);

   std::cout << GREEN << "PicoShell 0.1" << RESET << std::endl;

   //////////////

   while(true){

      std::string sBuffer = "";

      std::getline(std::cin >> std::ws, sBuffer);

      CShell.command(sBuffer);
   }

   //////////////

   CShell.stop();
   CSerial.close();

   return(0);
}

/////////////////////////////////////////////////////////////////////
// [ NCallback ]
/////////////////////////////////////////////////////////////////////
void NCallback(struct inotify_event* pEvent){

   if(pEvent == nullptr || !pEvent->len) return;

/*    std::cout << "NCallback" << std::endl;

   std::cout << "name:   " << pEvent->name   << std::endl;
   std::cout << "cookie: " << pEvent->cookie << std::endl;
   std::cout << "mask:   " << pEvent->mask   << std::endl;
   std::cout << "wd:     " << pEvent->wd     << std::endl; */

   if(pEvent->mask & IN_CREATE){

      if(!(pEvent->mask & IN_ISDIR)){

         if(sFile.compare(pEvent->name) == 0){

            std::cout << GREEN << "Pico is added" << RESET << std::endl;

            std::this_thread::sleep_for(1000ms);

            if(CSerial.open(sPath, B9600) == C_SERIAL_READY){

               if(CShell.start() == C_PICOSHELL_READY){

               }
            }
         }
      }

   }else 
   if(pEvent->mask & IN_DELETE){

      if(!(pEvent->mask & IN_ISDIR)){

         if(sFile.compare(pEvent->name) == 0){

            std::cout << RED << "Pico is removed" << RESET << std::endl;

            CShell.stop();
            CSerial.close();
         }
      }

   }else 
   if(pEvent->mask & IN_MODIFY){

      if(pEvent->mask & IN_ISDIR){

      }
   }

}