//////////////////////////////////////////////////////////////////////////////////
// [ C_PicoShell_Class_Source ]
//////////////////////////////////////////////////////////////////////////////////

#include "C_PicoShell.hpp"

//////////////////////////////////////////////////////////////////////////////////
// [ C_PicoShell ]
//////////////////////////////////////////////////////////////////////////////////
C_PicoShell::C_PicoShell(C_Serial* pSerial){

   if(pSerial == nullptr) return;

   pCSerial = pSerial;

   bReady = true;
}

//////////////////////////////////////////////////////////////////////////////////
// [ ~C_PicoShell ]
//////////////////////////////////////////////////////////////////////////////////
C_PicoShell::~C_PicoShell(){

}

//////////////////////////////////////////////////////////////////////////////////
// [ start ]
//////////////////////////////////////////////////////////////////////////////////
int C_PicoShell::start(){

   if(listen() != C_PICOSHELL_READY) 
      return(C_PICOSHELL_ERROR);

   return(C_PICOSHELL_READY);
}

//////////////////////////////////////////////////////////////////////////////////
// [ stop ]
//////////////////////////////////////////////////////////////////////////////////
int C_PicoShell::stop(){

   if(bListen){

      bListen = false;

      std::this_thread::sleep_for(100ms);

      delete pTRecive;

      pTRecive = nullptr;
   }

   if(bDownload){

      bDownload = false;

      ofile.close();
   }

   return(C_PICOSHELL_READY); 
}

//////////////////////////////////////////////////////////////////////////////////
// [ listen ]
//////////////////////////////////////////////////////////////////////////////////
int C_PicoShell::listen(){

   if(bListen) return(C_PICOSHELL_READY);

   bListen = true;

   pTRecive = new std::thread(&C_PicoShell::recive, this);

   if(pTRecive){

      pTRecive->detach();
      return(C_PICOSHELL_READY);

   }else{

      bListen = false;
      return(C_PICOSHELL_ERROR);
   } 
}

//////////////////////////////////////////////////////////////////////////////////
// [ recive ]
//////////////////////////////////////////////////////////////////////////////////
void C_PicoShell::recive(){

   std::array<char, 1024> aBuffer;
   std::string sBuffer = "";

   while(bListen){

      size_t nbytes = aBuffer.size();

      if(pCSerial->read(aBuffer.data(), &nbytes) != C_SERIAL_READY){
         std::cout << "C_PicoShell::recive - ERROR" << std::endl;
         continue;
      }

      if(!nbytes) return; // TEST

      if((int)aBuffer[2] == HXS){

         std::string stemp = std::string(aBuffer.data(), nbytes);

         sBuffer += stemp.substr(3, nbytes - 4);

         if(nbytes != 116){

            std::string sText = hex2ascii(sBuffer);

            if(bDownload){

               ofile.write(sText.c_str(), sText.size());

               bDownload = false;

               ofile.close();

            }else{

               std::cout << GREEN << sText << RESET << std::endl;
            }

            sBuffer.clear();
         }

      }else{

         std::string sBuffer = std::string(aBuffer.data(), nbytes);

         if(sBuffer.length() > 2 && sBuffer.compare(2, 5, "Error") == 0){

            std::string sNumber = sBuffer.substr(sBuffer.length() - 2, 2);

            u_long error = stoul(sNumber);

            std::cout << RED << "error: " << error << " - " << error2string(error) << RESET << std::endl;

         }else{

            sBuffer.erase(std::remove(sBuffer.begin(), sBuffer.end(), '\n'), sBuffer.cend());

            if(bUpload){
               std::cout << "+";
            }else{

               size_t pos;

               if((pos = sBuffer.find("[directory]")) != std::string::npos){

                  std::cout << YELLOW << sBuffer.substr(0, pos) << RESET << std::endl;

               }else
               if((pos = sBuffer.find("[writable file]")) != std::string::npos){

                  std::cout << CYAN << std::left << std::setw(15) << sBuffer.substr(0, pos);

                  std::cout << WHITE << std::left << std::setfill(' ') << std::setw(10) << " ";
                  
                  size_t posSize;

                  if((posSize = sBuffer.find("[size=")) != std::string::npos){

                     std::string sSize = sBuffer.substr(posSize + 6);

                     sSize.pop_back();
                     sSize.pop_back();

                     std::cout << GREEN << std::right << std::setfill(' ') << std::setw(10) << sSize << WHITE << " byte";
                  }

                  std::cout << RESET << std::endl;

               }else{

                  std::cout << BOLD BGBLUE WHITE << sBuffer << RESET << std::endl;
               }
            }
         }
      }
   }
}

//////////////////////////////////////////////////////////////////////////////////
// [ command ]
//////////////////////////////////////////////////////////////////////////////////
int C_PicoShell::command(std::string sCommand){

   if(!sCommand.length()) return(C_PICOSHELL_NOT_FOUND);

   std::string sParam = "";

   size_t found = sCommand.find(' ');

   if(found != std::string::npos){

      if(sCommand.length() > found + 1)
         sParam = sCommand.substr(found + 1, sCommand.length() - found);

      sCommand = sCommand.substr(0, found);
   }

   ///////////////////////////////////////////

   int Result = command2number(sCommand);

   if(Result == C_PICOSHELL_NOT_FOUND){
      std::cout << RED << "Unknown Command: " << sCommand << RESET << std::endl;
      return(C_PICOSHELL_NOT_FOUND);
   }else
   if(Result == ECOMMAND::COM_HELP){
      help();
      return(C_PICOSHELL_READY);
   }

   ///////////////////////////////////////////////

   return(command2packet(Result, sParam));
}

//////////////////////////////////////////////////////////////////////////////////
// [ command2number ]
//////////////////////////////////////////////////////////////////////////////////
int C_PicoShell::command2number(std::string sCommand){

   if(!sCommand.length()) return(C_PICOSHELL_NOT_FOUND);

   for(auto& Command : MCommand)
      if(sCommand.compare(Command.first) == 0)
         return(Command.second);

   return(C_PICOSHELL_NOT_FOUND);
}

//////////////////////////////////////////////////////////////////////////////////
// [ command2packet ]
//////////////////////////////////////////////////////////////////////////////////
int C_PicoShell::command2packet(int nCommand, std::string sParam){

   std::array<char, 1024> aBuffer;
   int p = 0;

   aBuffer[p++] = SOH;
   aBuffer[p++] = STX;

   bool bParam = false;

   switch(nCommand){

      case ECOMMAND::COM_LS:      aBuffer[p++] = LST; break;
      case ECOMMAND::COM_PWD:     aBuffer[p++] = PWD; break;

      case ECOMMAND::COM_CD:      aBuffer[p++] = CWD; bParam = true; break;
      case ECOMMAND::COM_RM:      aBuffer[p++] = DEL; bParam = true; break;
      case ECOMMAND::COM_CLF:     aBuffer[p++] = CLF; bParam = true; break;
      case ECOMMAND::COM_CAT:     aBuffer[p++] = RDF; bParam = true; break;
      case ECOMMAND::COM_MKDIR:   aBuffer[p++] = MKD; bParam = true; break;
      case ECOMMAND::COM_TOUCH:   aBuffer[p++] = MKF; bParam = true; break;

      case ECOMMAND::COM_RBP2USB: aBuffer[p++] = COM; aBuffer[p++] = CPS_RB2USB; break;
      case ECOMMAND::COM_RBP:     aBuffer[p++] = COM; aBuffer[p++] = CPS_REBOOT; break;

      case ECOMMAND::COM_ULF:     return(upload(sParam));
      case ECOMMAND::COM_DLF:     return(download(sParam));

      default: return(C_PICOSHELL_ERROR); break;
   }

   if(bParam && sParam.length()){
      p += sprintf(&aBuffer[p], "%s", sParam.c_str());
   }else
   if(bParam){
      std::cout << RED << "ERROR - Commando need a parameter" << RESET << std::endl;
      return(C_PICOSHELL_ERROR);
   }

   aBuffer[p++] = ETX;
   aBuffer[p++] = NUL;

   if(pCSerial->write(aBuffer.data(), p) != C_SERIAL_READY){
      std::cout << "pCSerial->write - ERROR" << std::endl;
      return(C_PICOSHELL_ERROR);
   }

   return(C_PICOSHELL_READY);
}

//////////////////////////////////////////////////////////////////////////////////
// [ hex2ascii ]
//////////////////////////////////////////////////////////////////////////////////
std::string C_PicoShell::hex2ascii(std::string shex){

   std::string sAscii = "";

   for(size_t i = 0; i < shex.length(); i += 2){

      std::string sPart = shex.substr(i, 2);

      char ch;

      try{
         ch = stoul(sPart, nullptr, 16);
      }catch(const std::exception& e){
         std::cerr << "C_PicoShell::hex2ascii - ERROR: " << e.what() << '\n';
         continue;
      }

      sAscii += ch;
   }

   return(sAscii);
}

//////////////////////////////////////////////////////////////////////////////////
// [ ascii2hex ]
//////////////////////////////////////////////////////////////////////////////////
std::string C_PicoShell::ascii2hex(std::string SAscii){

   std::stringstream ss;
   ss << std::hex;

   for(int i(0); i < SAscii.length(); ++i)
      ss << std::setw(2) << std::setfill('0') << (int)SAscii[i];

   return(ss.str());
}

//////////////////////////////////////////////////////////////////////////////////
// [ download ]
//////////////////////////////////////////////////////////////////////////////////
int C_PicoShell::download(std::string sPath){

   ofile.open(sPath, std::ios::out | std::ios::binary);

   if(!ofile.good() || !ofile.is_open()){ 
      std::cout << "error: open file: " << sPath << std::endl;
      return(C_PICOSHELL_ERROR);
   }

   /////////////////////////////////////////////////

   std::size_t found = sPath.find_last_of("/");
   std::string sFile = sPath.substr(found + 1);

   bDownload = true;

   std::array<char, 1024> aBuffer;
   int p = 0;

   aBuffer[p++] = SOH;
   aBuffer[p++] = STX;
   aBuffer[p++] = RDF;

   p += sprintf(&aBuffer[p], "%s", sFile.data());

   aBuffer[p++] = ETX;
   aBuffer[p++] = NUL;

   if(pCSerial->write(aBuffer.data(), p) != C_SERIAL_READY){
      bDownload = false;
      ofile.close();
      return(C_PICOSHELL_ERROR);
   }

   return(C_PICOSHELL_READY);
}

//////////////////////////////////////////////////////////////////////////////////
// [ upload ]
//////////////////////////////////////////////////////////////////////////////////
int C_PicoShell::upload(std::string sPath){

   std::ifstream file(sPath, std::ios::in | std::ios::binary);

   if(!file.good() || file.eof() || !file.is_open()){ 
      std::cout << "error: open file" << std::endl;
      return(C_PICOSHELL_ERROR);
   }

   /////////////////////////////////////////////////

   file.seekg(0, std::ios_base::beg);
   std::ifstream::pos_type begin_pos = file.tellg();
   file.seekg(0, std::ios_base::end);

   int Size = static_cast<int>(file.tellg() - begin_pos);

   file.seekg(0, std::ios_base::beg);

   ////////////////////////////////////////////////

   std::vector<char> vBuffer;

   vBuffer.resize(Size + 1);

   file.read(vBuffer.data(), Size);

   vBuffer[Size] = 0x00; // TEST

   ///////////////////////////////////////////////

   std::string temp = vBuffer.data();
   std::string shex = ascii2hex(temp);

   ///////////////////////////////////////////////

   std::size_t found = sPath.find_last_of("/");
   std::string sFile = sPath.substr(found + 1);

   ///////////////////////////////////////////////

   bUpload = true;

   std::array<char, 1024> aBuffer;
   int p = 0;

   int cSFile = shex.length();

   int cParts = cSFile / CPS_DATACHUNK;
   int cRest  = cSFile % CPS_DATACHUNK;

   aBuffer[p++] = SON;
   p += sprintf(&aBuffer[p], "%s", sFile.data());
   aBuffer[p++] = EON;
   aBuffer[p++] = NUL;

   std::string nhex = ascii2hex(aBuffer.data());

   /////////////////////

   for(int n = 0; n < cParts; n++){

      std::string part = shex.substr(n * CPS_DATACHUNK, CPS_DATACHUNK);

      p = 0;

      aBuffer[p++] = SOH;
      aBuffer[p++] = STX;
      aBuffer[p++] = WRF;
      aBuffer[p++] = HXS;

      p += sprintf(&aBuffer[p], "%02x", nhex.length() + part.length());
      p += sprintf(&aBuffer[p], "%s", nhex.c_str());
      p += sprintf(&aBuffer[p], "%s", part.c_str());

      aBuffer[p++] = ETX;
      aBuffer[p++] = 0;

      if(pCSerial->write(aBuffer.data(), p) != C_SERIAL_READY){
         file.close();
         bUpload = false;
         return(C_PICOSHELL_ERROR);
      }

      std::this_thread::sleep_for(20ms);
   }

   /////////////////////

   if(cRest){

      std::string part = shex.substr(cParts * CPS_DATACHUNK, cRest);

      p = 0;

      aBuffer[p++] = SOH;
      aBuffer[p++] = STX;
      aBuffer[p++] = WRF;
      aBuffer[p++] = HXS;

      p += sprintf(&aBuffer[p], "%02x", nhex.length() + cRest);
      p += sprintf(&aBuffer[p],   "%s", nhex.c_str());
      p += sprintf(&aBuffer[p],   "%s", part.c_str());

      aBuffer[p++] = ETX;
      aBuffer[p++] = 0;

      if(pCSerial->write(aBuffer.data(), p) != C_SERIAL_READY){
         file.close();
         bUpload = false;
         return(C_PICOSHELL_ERROR);
      }

      std::this_thread::sleep_for(20ms);
   }

   bUpload = false;

   ///////////////////////////////////////////////

   file.close();

   return(C_PICOSHELL_READY);
}

//////////////////////////////////////////////////////////////////////////////////
// [ error2string ]
//////////////////////////////////////////////////////////////////////////////////
std::string C_PicoShell::error2string(u_int error){

   std::string sResult = "Succeeded";

   switch(error){
      case FR_DISK_ERR:            sResult = "A hard error occurred in the low level disk I/O layer"; break;
      case FR_INT_ERR:             sResult = "Assertion failed"; break;
      case FR_NOT_READY:           sResult = "The physical drive cannot work"; break;
      case FR_NO_FILE:             sResult = "Could not find the file"; break;
      case FR_NO_PATH:             sResult = "Could not find the path"; break;
      case FR_INVALID_NAME:        sResult = "The path name format is invalid"; break;
      case FR_DENIED:              sResult = "Access denied due to prohibited access or directory full"; break;
      case FR_EXIST:               sResult = "Access denied due to prohibited access"; break;
      case FR_INVALID_OBJECT:      sResult = "The file/directory object is invalid"; break;
      case FR_WRITE_PROTECTED:     sResult = "The physical drive is write protected"; break;
      case FR_INVALID_DRIVE:       sResult = "The logical drive number is invalid "; break;
      case FR_NOT_ENABLED:         sResult = "The volume has no work area"; break;
      case FR_NO_FILESYSTEM:       sResult = "There is no valid FAT volume "; break;
      case FR_MKFS_ABORTED:        sResult = "The f_mkfs() aborted due to any problem"; break;
      case FR_TIMEOUT:             sResult = "Could not get a grant to access the volume within defined period"; break;
      case FR_LOCKED:              sResult = "The operation is rejected according to the file sharing policy"; break;
      case FR_NOT_ENOUGH_CORE:     sResult = "LFN working buffer could not be allocated"; break;
      case FR_TOO_MANY_OPEN_FILES: sResult = "Number of open files > FF_FS_LOCK "; break;
      case FR_INVALID_PARAMETER:   sResult = "Given parameter is invalid"; break;
      default: break;
   }

   return(sResult);
}

//////////////////////////////////////////////////////////////////////////////////
// [ help ]
//////////////////////////////////////////////////////////////////////////////////
void C_PicoShell::help(){

   std::cout << "PicoShell 0.1 - help" << std::endl << std::endl;

   std::cout << "Commands:" << std::endl;

   std::cout << "rbp        - reboot pico"              << std::endl;
   std::cout << "rbp2usb    - reboot pico as usb drive" << std::endl;

   std::cout << std::endl 
             << "only available if a data storage is attached to the pico" 
             << std::endl << std::endl;

   std::cout << "ls         - list working directory"   << std::endl;
   std::cout << "pwd        - print working directory"  << std::endl;
   std::cout << "cd path    - change working directory" << std::endl;
   std::cout << "mkdir path - make directory"           << std::endl;
   std::cout << "touch file - make file"                << std::endl;
   std::cout << "rm path    - remove file or folder"    << std::endl;
   std::cout << "clf file   - clear file"               << std::endl;
   std::cout << "cat file   - display file content"     << std::endl;
   std::cout << "ulf file   - upload file"              << std::endl;
   std::cout << "dlf file   - download file"            << std::endl;

}