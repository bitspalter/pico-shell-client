//////////////////////////////////////////////////////////////////////////////////
// [ C_Serial_Header ]
//////////////////////////////////////////////////////////////////////////////////
//
// 
// [::Last modi: 19.05.25 L.ey (µ~)::]
//
//
#ifndef _C_SERIAL_H_
  #define _C_SERIAL_H_

  #include <iostream>
  #include <string>
  #include <cstring>
  #include <fcntl.h>  // open()
  #include <unistd.h> // close()
  #include <termios.h>
  #include <errno.h>

/////////////////////////////////////////////////////////////////////////
// CONST
/////////////////////////////////////////////////////////////////////////

   const int C_SERIAL_ERROR     =  0;
   const int C_SERIAL_READY     =  1;
   const int C_SERIAL_NOT_FOUND = -1;

/////////////////////////////////////////////////////////////////////////
// CLASS
/////////////////////////////////////////////////////////////////////////

class C_Serial {

   public:

      C_Serial(){};
     ~C_Serial(){};

      int  open(std::string sPath, int baud);
      void close();

      int write(const char* buffer, size_t size);
      int read(char* pbuffer, size_t* pcBuffer);

      /////////////////////////

      bool get_bOpen(){return(bOpen);};

      std::string get_sDevice(){return(sDevice);};

   private:

      int  hPort = {C_SERIAL_NOT_FOUND};
      bool bOpen = {false};

      std::string sDevice = {""};
};

#endif // _C_SERIAL_H_