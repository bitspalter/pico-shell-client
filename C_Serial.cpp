/////////////////////////////////////////////////////////////////////
// [ C_Serial_Class_Source ]
/////////////////////////////////////////////////////////////////////

#include "C_Serial.hpp"

/////////////////////////////////////////////////////////////////////
// [ open ]
/////////////////////////////////////////////////////////////////////
int C_Serial::open(std::string sPath, int baud){

   if(bOpen) return(C_SERIAL_READY);

   hPort = ::open(sPath.c_str(), O_RDWR | O_NOCTTY | O_SYNC);

   if(hPort < 0){

      std::cerr << "C_Serial::open Path: " << sPath.c_str() 
                << " [ Error ] : " << strerror(errno) 
                << std::endl;

      return(C_SERIAL_ERROR);
   }

   ///////////////////////////////////////////

   struct termios tty;

   if(tcgetattr(hPort, &tty) != 0){

      std::cerr << "C_Serial::open Error from tcgetattr: " 
                << strerror(errno) 
                << std::endl;

      ::close(hPort);
      hPort = C_SERIAL_NOT_FOUND;

      return(C_SERIAL_ERROR);
   }

   cfsetospeed(&tty, baud);
   cfsetispeed(&tty, baud);

   tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit characters
   tty.c_iflag &= ~IGNBRK; // disable break processing
   tty.c_lflag = 0;        // no signaling chars, no echo, no
                           // canonical processing
   tty.c_oflag = 0;        // no remapping, no delays

   tty.c_cc[VMIN]  = 1;    // read doesn't block
   tty.c_cc[VTIME] = 5;    // 0.5 seconds read timeout

   tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

   tty.c_cflag |= (CLOCAL | CREAD);   // ignore modem controls,
                                      // enable reading
   tty.c_cflag &= ~(PARENB | PARODD); // shut off parity
   tty.c_cflag &= ~CSTOPB;
   tty.c_cflag &= ~CRTSCTS;

   if(tcsetattr(hPort, TCSANOW, &tty) != 0){

      std::cerr << "C_Serial::open Error from tcsetattr: " 
                << strerror(errno) 
                << std::endl;

      ::close(hPort);
      hPort = C_SERIAL_NOT_FOUND;

      return(C_SERIAL_ERROR);
   }

   ///////////////////////////////////////////

   bOpen   = true;
   sDevice = sPath;

   return(C_SERIAL_READY);
}

/////////////////////////////////////////////////////////////////////
// [ close ]
/////////////////////////////////////////////////////////////////////
void C_Serial::close(){

   if(!bOpen) return;

   ::close(hPort);

   hPort   = C_SERIAL_NOT_FOUND;
   sDevice = "";
   bOpen   = false;
}

/////////////////////////////////////////////////////////////////////
// [ write ]
/////////////////////////////////////////////////////////////////////
int C_Serial::write(const char* pbuffer, size_t cBuffer){

   if(!bOpen) return(C_SERIAL_ERROR);

   int result = ::write(hPort, pbuffer, cBuffer);

   if(result < cBuffer){
      
      std::cerr << "C_Serial::write ERROR: " 
                << strerror(errno) 
                << std::endl;

      return(C_SERIAL_ERROR);
   } 

   return(C_SERIAL_READY);
}

/////////////////////////////////////////////////////////////////////
// [ read ]
/////////////////////////////////////////////////////////////////////
int C_Serial::read(char* pbuffer, size_t* pcBuffer){

   if(!bOpen) return(C_SERIAL_ERROR);

   *pcBuffer = ::read(hPort, pbuffer, *pcBuffer);

   if(*pcBuffer < 0){

      std::cerr << "C_Serial::read ERROR: " 
                << strerror(errno) 
                << std::endl;

      return(C_SERIAL_ERROR);
   }

   return(C_SERIAL_READY);
}