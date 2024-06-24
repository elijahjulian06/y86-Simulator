#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <string.h>
#include "Memory.h"
#include "String.h"
#include "Loader.h"
#include "Tools.h"


/* 
 * Loader
 * Initializes the private data members
*/
Loader::Loader(int argc, char * argv[], Memory * mem)
{
   //this method is COMPLETE
   this->lastAddress = -1;   //keep track of last mem byte written to for error checking
   this->mem = mem;          //memory instance
   this->inputFile = NULL;   
   if (argc > 1) inputFile = new String(argv[1]);  //input file name
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
bool Loader::printErrMsg(int32_t which, int32_t lineNumber, String * line)
{
	//this method is COMPLETE
	static char * errMsg[Loader::numerrs] = 
   	{
    	(char *) "Usage: yess <file.yo>\n",                       //Loader::usage
    	(char *) "Input file name must end with .yo extension\n", //Loader::badfile
    	(char *) "File open failed\n",                            //Loader::openerr
    	(char *) "Badly formed data record\n",                    //Loader::baddata
    	(char *) "Badly formed comment record\n",                 //Loader::badcomment
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
	return false; //load fails
}

/**
 * openFile
 * The name of the file is in the data member openFile (could be NULL if
 * no command line argument provided)
 * Checks to see if the file name is well-formed and can be opened
 * If there is an error, it prints an error message and returns false
 * by calling printErrMsg
 * Otherwise, the file is opened and the function returns true
 *
 * * modifies inf data member (file handle) if file is opened
**/
bool Loader::openFile()
{
	bool error;
	//TODO

	//If the user didn't supply a command line argument (inputFile is NULL)
	//then print the Loader::usage emakerror message and return false
	//(Note: Loader::usage is a static const defined in Loader.h)
   	if(inputFile == NULL)
    	{
        	printErrMsg(usage, -1, NULL);
        	return false;
    	}
	//If the filename ais badly formed (needs to be at least 4 characters
	//long and end with .yo) then print the Loader::badfile error message 
	//and return false  
	if(inputFile -> get_length() < 4 || !(inputFile -> isSubString(".yo", inputFile -> get_length() - 3, error)))
    	{
        	printErrMsg(badfile, -1, NULL);  //fix later
        	return false;
   	}
	//Open the file using an std::ifstream open
	//If the file can't be openxsd then print the Loader::openerr message 
	//and return falses
	//std::ifstream file(inputFile -> get_stdstr());
	inf.open(inputFile->get_stdstr(), std::ifstream::in);
    	if(!(inf.is_open()))
    	{
    		printErrMsg(openerr,-1,NULL);
    		return false;
    	}
    	return true;  //file name is good and file open succeeded
}

/**
 * load 
 * Opens the .yo file by calling openFile.
 * Reads the lines in the file line by line and
 * loads the data bytes in data records into the Memory.
 * If a line has an error in it, then NONE of the line will be
 * loaded into memory and the load function will return false.
 *
 * Returns true if load succeeded (no errors in the input) 
 * and false otherwise.
**/    
bool Loader::load()
{
	bool error = false;
  	if (!openFile()) return false;

   	std::string line;
   	int lineNumber = 1;  //needed if an error is found
   	while (getline(inf, line))
   	{
    		//create a String to contain the std::string
    		//Now, all accesses to the input line MUST be via your
    		//String class methods
    		String inputLine(line);
      		//TODO
      		//printf("Current line = %s\n", inputLine);
      		//Note: there are two kinds of records: data and comment
      		// A data record begins with a "0x"
      		//
      		//If the line is a data record with errors
      		//then print the Loader::baddata error message and return false
		if(isData(inputLine, error))
		{
			if(isDataError(inputLine))
			{
				error = true;
				printErrMsg(baddata, lineNumber, &inputLine);
				return false;
			}
		}
      		//If the line is a comment record with errors
      		//then print the Loader::badcomment error message and return false
		else if(isCommentError(inputLine, error))
	  	{
            error = true;
			printErrMsg(badcomment, lineNumber, &inputLine);
			return false;
		}
     	 	//Otherwise, load any data on the line into
      		//memory
                if(isData(inputLine, error))
                {
                       int32_t addy;
                       //If the line is a data record with errors
                       //then print the Loader::baddata error message and return false
                       addy = inputLine.convert2Hex(2, 3, error); // grab address
                       //calculate if their is room in memory 
                       int indexLeft = mem->size - addy;
                       int dataLength = 0;
                       //calculating length of data
                       for(int i = databegin; !inputLine.isChar(' ', i , error); i++)
                       {  
                            dataLength++;
                       }
                       dataLength /= 2;
                       if(indexLeft < dataLength) //if datalength is greater than its not
                       {
                            printErrMsg(baddata, lineNumber, &inputLine);
                            return false;
                       }
                       if(addy <= lastAddress) //makes sure current addr is greater than prev 
                       {
                            printErrMsg(baddata, lineNumber, &inputLine);
                            return false;
                       }
                       //all error checking passed
                       for(int i = databegin; !inputLine.isChar(' ', i , error); i+=2)
                       {       
                             uint32_t val = inputLine.convert2Hex(i, 2, error);
                             mem->putByte(val, addy, error);
                             lastAddress = addy;
                             addy++;
                       }
                 }
				 //Don't do all of this work in this method!
                 //Break the work up into multiple single purpose methods
                 //increment the line number for next iteration
                 lineNumber++;
      
        }
	return true;  //load succeeded
}

//Add helper methods definitions here and the declarations to Loader.h
//In your code, be sure to use the static const variables defined in 
//Loader.h to grab specific fields from the input line.


/** isData
 *
 * Checks whether a given line is an inputLine
 * returns true if line contains '0x'
 * returns false if line does not contain '0x'
 * 
 * @param line - the current line
 * @param error - set to true if there is an error.
 * 
*/
bool Loader::isData(String line, bool &error)
{
	//do we need to account for if there is no 0x but 0x is somewhere else in the inputLine???
	return line.isSubString("0x", 0, error);
}

bool Loader::isDataError(String data)
{
	bool error = false;
	int dataM = 0;
	if(!(data.isHex(addrbegin, 3, error)))
	{
		return true;
	}
	if(!(data.isChar(':', 5, error)))
	{
		return true;
   }
   if(!(data.isChar(' ', 6, error)))
   {
		return true;
   }
   if(!(data.isChar(' ', 27, error)))
   {
      return true;
   }
   for(int i = databegin; i < comment - 1; i++)
   {
      //find how many bytes 0-10 and then find how many spaces proceed up till comment
		if(data.isHex(i, 1, error))
		{
			dataM++;
		}
   }
   if(dataM % 2 != 0)
   {
      return true;
   }
   //check to see if the number of bytes we have are hex
   if(!(data.isHex(databegin, dataM, error)))
   {
		return true;
   }
   if(!(data.isRepeatingChar(' ', ((databegin + dataM) + 1), comment - ((databegin + dataM) + 1), error)))
   {
      return true;
   }
   if(!(data.isChar('|', 28, error)))
   {
      return true;
   }
   //check to see if the empty bytes are spaces up until comment 
   return false;
}


/** isCommentError
 * 
 * returns true if there is comment error 
 * comment errors:
 * if col 28 on a line of code does not contain a '|' it is a comment error
 * checks to make sure that lines 0-27 are spaces 
 * 
 * @param line - set to true if there is an error.
 * @param error - set to true if there is an error.
*/
bool Loader::isCommentError(String line, bool &error)
{
   if(!(line.isChar('|', 28, error)))
   {  
      error = true;
      return true;
   }
   //also check to make sure that lines 0-27 are spaces 
   if(!(line.isRepeatingChar(' ', 0, 27, error)))
   {
       error = true;
       return true;
   }
   return false;
}
