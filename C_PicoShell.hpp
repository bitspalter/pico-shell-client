//////////////////////////////////////////////////////////////////////////////////
// [ C_PicoShell_Header ]
//////////////////////////////////////////////////////////////////////////////////
//
// 
// [::Last modi: 20.05.25 L.ey (µ~)::]
//
//
#ifndef _C_PICOSHELL_H_
  #define _C_PICOSHELL_H_

  #include <iostream>
  #include <sstream>
  #include <fstream>
  #include <iomanip>
  #include <string>
  #include <array>
  #include <vector>
  #include <thread>
  #include <algorithm>
  #include <chrono>
  #include <map>

  using namespace std::chrono_literals;

  #include "C_Serial.hpp"

/////////////////////////////////////////////////////////////////////////
// Packets
/////////////////////////////////////////////////////////////////////////
//
// Type 1: |SOH|STX|Command|ETX|NUL|
// Type 2: |SOH|STX|Command|Path|ETX|NUL|
// Type 3: |SOH|STX|Command|HXS|SON|Path|EON|Data|ETX|NUL|
//
/////////////////////////////////////////////////////////////////////////
// CONST
/////////////////////////////////////////////////////////////////////////

  const int C_PICOSHELL_ERROR     =  0;
  const int C_PICOSHELL_READY     =  1;
  const int C_PICOSHELL_NOT_FOUND = -1;

  #define CPS_DATACHUNK 0x70

  #define CPS_REBOOT   '1'
  #define CPS_RB2USB   '2'
  #define CPS_PICOID   '3'
  #define CPS_CALLBACK '4'

  //////////////////////////////////////////
  // ascii codes //////////////////////

  #define NUL 0x00 // 
  #define SOH 0x01 // start of heading
  #define STX 0x02 // start of text
  #define ETX 0x03 // end of text
  #define EOT 0x04 // end of transmission
  #define ENQ 0x05 // enquiry
  #define ACK 0x06 // acknowledge
  #define BEL 0x07 // bell

  ///// MY OWN /////////////

  #define PWD 0x08 // print working directory
  #define CWD 0x09 // change working directory
  #define LST 0x0A // list working directory
  #define MKD 0x0B // make directory
  #define MKF 0x0C // make file
  #define RDF 0x0D // read file
  #define WRF 0x0E // write file
  #define SON 0x0F // start of name
  #define LFD 0x10 // line feed
  #define EON 0x11 // end of name
  #define CLF 0x12 // clear file
  #define HXS 0x13 // hex string
  #define COM 0x14 // Commando

  ///// MY OWN /////////////

  // ascii codes //////////////////////

  #define NAK 0x15 // negative acknowledge
  #define SYN 0x16 // synchronize
  #define CAN 0x18 // cancel

  #define ESC 0x1B // escape
  #define MSG 0x1C // message
  #define DEL 0x7F // delete

///////////////////////////////////////////////////
// from ff.h for error2string();

typedef enum {
	FR_OK = 0,              /* (0) Succeeded */
	FR_DISK_ERR,            /* (1) A hard error occurred in the low level disk I/O layer */
	FR_INT_ERR,             /* (2) Assertion failed */
	FR_NOT_READY,           /* (3) The physical drive cannot work */
	FR_NO_FILE,             /* (4) Could not find the file */
	FR_NO_PATH,             /* (5) Could not find the path */
	FR_INVALID_NAME,        /* (6) The path name format is invalid */
	FR_DENIED,              /* (7) Access denied due to prohibited access or directory full */
	FR_EXIST,               /* (8) Access denied due to prohibited access */
	FR_INVALID_OBJECT,      /* (9) The file/directory object is invalid */
	FR_WRITE_PROTECTED,     /* (10) The physical drive is write protected */
	FR_INVALID_DRIVE,       /* (11) The logical drive number is invalid */
	FR_NOT_ENABLED,         /* (12) The volume has no work area */
	FR_NO_FILESYSTEM,       /* (13) There is no valid FAT volume */
	FR_MKFS_ABORTED,        /* (14) The f_mkfs() aborted due to any problem */
	FR_TIMEOUT,             /* (15) Could not get a grant to access the volume within defined period */
	FR_LOCKED,              /* (16) The operation is rejected according to the file sharing policy */
	FR_NOT_ENOUGH_CORE,     /* (17) LFN working buffer could not be allocated */
	FR_TOO_MANY_OPEN_FILES,	/* (18) Number of open files > FF_FS_LOCK */
	FR_INVALID_PARAMETER    /* (19) Given parameter is invalid */
} FRESULT;

/////////////////////////////////////////////////////////////////////////

 enum ECOMMAND {
    COM_HELP = 0,
    COM_LS,
    COM_PWD,
    COM_CD,
    COM_RM,
    COM_CLF,
    COM_CAT,
    COM_ULF,
    COM_DLF,
    COM_MKDIR,
    COM_TOUCH,
    COM_RBP2USB,
    COM_RBP
 };

 static const std::map<const char*, int> MCommand = {
    {"help",    COM_HELP},
    {"ls",      COM_LS},
    {"pwd",     COM_PWD},
    {"cd",      COM_CD},
    {"rm",      COM_RM},
    {"clf",     COM_CLF},
    {"cat",     COM_CAT},
    {"ulf",     COM_ULF},
    {"dlf",     COM_DLF},
    {"mkdir",   COM_MKDIR},
    {"touch",   COM_TOUCH},
    {"rbp2usb", COM_RBP2USB},
    {"rbp",     COM_RBP}
 };

 //the following are LINUX and MacOS ONLY terminal color codes

 #define cursorXY(x,y) printf("\033[%d;%dH",(x),(y))

 #define TESTT       "\033[0;0H"

 #define CLEAR       "\033[2J"

 #define RESET       "\033[0m"
 #define BOLD        "\033[1m"
 #define UNDERLINE   "\033[4m"
 #define BLINK       "\033[5m"
 #define INVERTED    "\033[7m"
 #define HIDDEN      "\033[8m"

 #define BLACK       "\033[30m"
 #define RED         "\033[31m"
 #define GREEN       "\033[32m"
 #define YELLOW      "\033[33m"
 #define BLUE        "\033[34m"
 #define MAGENTA     "\033[35m"
 #define CYAN        "\033[36m"
 #define WHITE       "\033[37m"

 #define BGBLACK     "\033[40m"
 #define BGRED       "\033[41m"
 #define BGGREEN     "\033[42m"
 #define BGYELLOW    "\033[43m"
 #define BGBLUE      "\033[44m"
 #define BGMAGENTA   "\033[45m"
 #define BGCYAN      "\033[46m"
 #define BGWHITE     "\033[47m"

/////////////////////////////////////////////////////////////////////////
// CLASS
/////////////////////////////////////////////////////////////////////////

class C_PicoShell {

   public:

      C_PicoShell(C_Serial* pSerial);
     ~C_PicoShell();

      int start();
      int stop();

      int command(std::string sCommand);
      
   private:

      int listen();
      int running();

      std::string hex2ascii(std::string SHex);
      std::string ascii2hex(std::string SAscii);

      std::string error2string(u_int error);

      int command2number(std::string sCommand);
      int command2packet(int nCommand, std::string sParam);

      void help();

      int upload(std::string sPath);
      int download(std::string sPath);

      /////////////

      C_Serial* pCSerial = {nullptr};

      bool bReady = {false};

      /////////////

      std::thread* pTRecive = {nullptr};
      bool bListen = {false};
      void recive();

      /////////////

      std::thread* pTRunning = {nullptr};
      bool bRunning = {false};
      void run();

      //////////////
      
      bool bUpload   = {false};
      bool bDownload = {false};

      std::ofstream ofile;
};

#endif // _C_PICOSHELL_H_