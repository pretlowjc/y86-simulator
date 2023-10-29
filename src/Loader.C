#include <iostream>
#include <fstream>
#include <cstdint>
#include "Memory.h"
#include "String.h"
#include "Loader.h"


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
		int32_t addressBegin = Loader::addrbegin;
    	int32_t addressEnd = Loader::addrend;
        int32_t dataBegin = Loader::databegin;

   if (!openFile()) return false;

   std::string line;
   int lineNumber = 1; // needed if an error is found
   while (getline(inf, line))
   {
      // create a String to contain the std::string
      // Now, all accesses to the input line MUST be via your
      // String class methods
      String inputLine(line);
    //   // TODO

	
	bool error = false;
	uint64_t data = 0;

	  
	//    if(!Loader::isaHex(line,addrbegin,addrend,error)){ useless check I think
			
	//    }
		// I dont know if im suppose to use inputLine or line.
		if(!Loader::badData(inputLine)){
			printErrMsg(Loader::baddata,lineNumber,&inputLine); // unsure on &inputLine
		}
			else if (!Loader::badComment(inputLine)){
				printErrMsg(Loader::badcomment,lineNumber, &inputLine); // unsure on &input
			}
		
		else{
		// memory address check here last check needed we need to check for that next address 
		//line is less than the current, also need to check if address is out of bounds.
		uint32_t address = inputLine.convert2Hex(addressBegin, addressEnd, error); // should be address 0xhhh
		data = inputLine.convert2Hex(dataBegin, inputLine.get_length(), error); // should be data up until end of line.
		for (int i = 0; i < 8; i++) {				// loop could be wrong?
            uint8_t byte = static_cast<uint8_t>((data >> (i * 8)) & 0xFF);      
            mem->putByte(byte, address + i, error);   // nothing writing to my sdump
         }
		 lineNumber++;
		}
	// moved loader hints down
      
   }
   return true; // load succeeded
}

	//COMMENTS
      // Note: there are two kinds of records: data and comment
      //       A data record begins with a "0x"
	  

      //
      // If the line is a data record with errors
      // then print the Loader::baddata error message and return false
      //
      // If the line is a comment record with errors
      // then print the Loader::badcomment error message and return false
      //
      // Otherwise, load any data on the line into
      // memory
      //
      // Don't do all of this work in this method!
      // Break the work up into multiple single purpose methods

      // increment the line number for next iteration

// Add helper methods definitions here and the declarations to Loader.h
// In your code, be sure to use the static const variables defined in
// Loader.h to grab specific fields from the input line.

/*
  Suggested Helper Methods:
  badComment
  badData
  loadMem ...maybe?
*/

bool Loader::badComment(String inputLine){
	
	bool hasError = false;
	if (inputLine.isSubString(" ", 0, hasError) == true){
		if(inputLine.isChar('|', Loader::comment, hasError) == false){
			return true;
		}
		
	}


	return false;
}

	bool Loader::badData(String inputLine){
		bool error = false;
		if (inputLine.isChar('0' ,0, error)) {
			if(inputLine.isChar('x', 1, error)){
				if(inputLine.isHex(2,4,error)){
					if(inputLine.isChar(':', 5, error)){
						if(inputLine.isChar(' ',6,error)){
							return true; // good data record following a "0xhhh: " format
						}

					}
				}

			}
    }




		return false;
		

	

	}

bool Loader::isaHex(String inputLine,int32_t sIdx, int32_t len, bool &error){
	if (inputLine.convert2Hex(sIdx, len, error) == false){
		return false;
	}

	


	return true;


	
}

