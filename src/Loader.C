#include <iostream>
#include <fstream>
#include <cstdint>
#include "Memory.h"
#include "String.h"
#include "Loader.h"
#include "Tools.h"

/*
 * Loader
 * Initializes the private data members
 */
Loader::Loader(int argc, char *argv[], Memory *mem)
{
   // this method is COMPLETE
   this->lastAddress = -1; // keep track of last mem byte written to for error checking
   this->mem = mem;        // memory instance
   this->inputFile = NULL;
   if (argc > 1)
      inputFile = new String(argv[1]); // input file name
}

/*
 * printErrMsg
 * Prints an error message and returns false (load failed)
 * If the line number is not -1, it also prints the line where error occurred
 *
 * which - indicates error number
 * lineNumber - number of line in input file on which error occurred (if applicable)
 * line - line on which error occurred (if applicable)
 */
bool Loader::printErrMsg(int32_t which, int32_t lineNumber, String *line)
{
   // this method is COMPLETE
   static char *errMsg[Loader::numerrs] =
       {
           (char *)"Usage: yess <file.yo>\n",                       // Loader::usage
           (char *)"Input file name must end with .yo extension\n", // Loader::badfile
           (char *)"File open failed\n",                            // Loader::openerr
           (char *)"Badly formed data record\n",                    // Loader::baddata
           (char *)"Badly formed comment record\n",                 // Loader::badcomment
       };
   if (which >= Loader::numerrs)
   {
      std::cout << "Unknown error: " << which << "\n";
   }
   else
   {
      std::cout << errMsg[which];
      if (lineNumber != -1 && line != NULL)
      {
         std::cout << "Error on line " << std::dec << lineNumber
                   << ": " << line->get_stdstr() << std::endl;
      }
   }
   return false; // load fails
}

/*
 * openFile
 * The name of the file is in the data member openFile (could be NULL if
 * no command line argument provided)
 * Checks to see if the file name is well-formed and can be opened
 * If there is an error, it prints an error message and returns false
 * by calling printErrMsg
 * Otherwise, the file is opened and the function returns true
 *
 * modifies inf data member (file handle) if file is opened
 */
bool Loader::openFile()
{
   // TODO

   // If the filename is badly formed (needs to be at least 4 characters
   // long and end with .yo) then print the Loader::badfile error message
   // and return false
   bool error;

   // If the user didn't supply a command line argument (inputFile is NULL)
   // then print the Loader::usage error message and return false
   //(Note: Loader::usage is a static const defined in Loader.h)
   if (inputFile == NULL)
   {
      printErrMsg(Loader::usage, 1, inputFile);
      return false;
   }
   int32_t filenameLength = inputFile->get_length();
   if ((filenameLength < 4) || (inputFile->isSubString(".yo", (inputFile->get_length() - 3), error) == false))
   {
      printErrMsg(Loader::badfile, -1, inputFile);
      return false;
   }

   // Open the file using an std::ifstream open
   // If the file can't be opened then print the Loader::openerr message
   // and return false
   inf.open(inputFile->get_stdstr());

   if (!inf.is_open())
   {
      // print the Loader::openerr message.
      printErrMsg(Loader::openerr, -1, inputFile);
      return false;
   }

   return true; // file name is good and file open succeeded
}

/*
 * load
 * Opens the .yo file by calling openFile.
 * Reads the lines in the file line by line and
 * loads the data bytes in data records into the Memory.
 * If a line has an error in it, then NONE of the line will be
 * loaded into memory and the load function will return false.
 *
 * Returns true if load succeeded (no errors in the input)
 * and false otherwise.
 */
bool Loader::load()
{

   if (!openFile())
      return false;
   bool error = false;

   std::string line;
   int lineNumber = 1; // needed if an error is found
   while (getline(inf, line))
   {

      String inputLine(line);
      if (isDataRecord(inputLine) && badData(inputLine))
      {
         return printErrMsg(Loader::baddata, lineNumber, &inputLine);
      }

      else if (!(isDataRecord(inputLine)) && badComment(inputLine))
      {
         return printErrMsg(Loader::badcomment, lineNumber, &inputLine);
      }
      uint32_t address = inputLine.convert2Hex(addrbegin, addrend - addrbegin + 1, error);
      int i = databegin;

      while (inputLine.isHex(i, 2, error))
      {
         uint64_t byte = inputLine.convert2Hex(i, 2, error);
         mem->putByte(byte, address, error);
         lastAddress = address;
         address++;
         i += 2;
      }
      lineNumber++;
   }
   return true;
}

bool Loader::badComment(String inputLine)
{
   bool hasError = false;
   if (!inputLine.isRepeatingChar(' ', 0, comment - 1, hasError))
      return true;
   if (!inputLine.isChar('|', comment, hasError))
      return true;
   return false;
}

bool Loader::badData(String inputLine)
{
   bool error = false;
   int i = databegin;
   int numofByte = 0;
   while (inputLine.isHex(i, 2, error))
   {
      i += 2;
      numofByte++;
   }
   if (!inputLine.isRepeatingChar(' ', i, comment - i, error))
      return true;
   if (!inputLine.isChar('|', comment, error))
      return true;
   if (!inputLine.isHex(addrbegin, addrend - addrbegin + 1, error))
      return true;
   if (!inputLine.isChar(':', 5, error))
      return true;
   if (!inputLine.isChar(' ', addrend + 2, error))
      return true;
   if (i > databegin && (inputLine.isHex(addrbegin, addrend, error)))
      return true;
   if (!inputLine.isChar(' ', comment - 1, error))
      return true;
   int64_t addy = inputLine.convert2Hex(addrbegin, 3, error);
   if (addy <= lastAddress)
      return true;
   if (addy + numofByte > Memory::size)
      return true;
   return false;
}

bool Loader::isDataRecord(String inputLine)
{
   bool error = false;
   if (!inputLine.isSubString((const char *)"0x", 0, error))
      return false;
   else
      return true;
}
