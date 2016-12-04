//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Tue Dec 11 16:09:32 PST 2012
// Last Modified: Tue Dec 11 16:09:38 PST 2012
// Last Modified: Fri Mar 11 21:26:18 PST 2016 Changed to STL
// Last Modified: Fri Dec  2 19:25:41 PST 2016 Moved to humlib
// Filename:      HumdrumFileStream.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileStream.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Higher-level functions for processing Humdrum files.
//                Inherits HumdrumFileStreamBasic and adds rhythmic and other
//                types of analyses to the HumdrumFileStream class.
//

#include "HumdrumFileStream.h"
#include "HumRegex.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// HumdrumFileStream::HumdrumFileStream --
//

HumdrumFileStream::HumdrumFileStream(void) {
   curfile = -1;
}

HumdrumFileStream::HumdrumFileStream(char** list) {
   curfile = -1;
   setFileList(list);
}

HumdrumFileStream::HumdrumFileStream(const vector<string>& list) {
   curfile = -1;
   setFileList(list);
}

HumdrumFileStream::HumdrumFileStream(Options& options) {
   curfile = -1;
   vector<string> list;
   options.getArgList(list);
   setFileList(list);
}



//////////////////////////////
//
// HumdrumFileStream::clear -- reset the contents of the class.
//

void HumdrumFileStream::clear(void) {
   curfile = 0;
   filelist.resize(0);
   universals.resize(0);
   newfilebuffer.resize(0);
}



//////////////////////////////
//
// HumdrumFileStream::setFileList --
//

int HumdrumFileStream::setFileList(char** list) {
   filelist.reserve(1000);
   filelist.resize(0);
   int i = 0;
   while (list[i] != NULL) {
      filelist.push_back(list[i]);
      i++;
   }
   return i;
}


int HumdrumFileStream::setFileList(const vector<string>& list) {
   filelist = list;
   return (int)list.size();
}



//////////////////////////////
//
// HumdrumFileStream::read -- alias for getFile.
//

int HumdrumFileStream::read(HumdrumFile& infile) {
   return getFile(infile);
}



//////////////////////////////
//
// HumdrumFileStream::eof -- returns true if there is no more segements
//     to read from the input source(s).
//

int HumdrumFileStream::eof(void) {
   istream* newinput = NULL;

   // Read HumdrumFile contents from:
   // (1) Current ifstream if open
   // (2) Next filename if ifstream is done
   // (3) cin if no ifstream open and no filenames

   // (1) Is an ifstream open?, then yes, there is more data to read.
   if (instream.is_open() && !instream.eof()) {
      return 0;
   } 

   // (1b) Is the URL data buffer open?
   else if (urlbuffer.str() != "") {
      return 0;
   }

   // (2) If ifstream is closed but there is a file to be processed,
   // load it into the ifstream and start processing it immediately.
   else if ((filelist.size() > 0) && (curfile < (int)filelist.size()-1)) {
      return 0;
   } else {
      // no input fstream open and no list of files to process, so
      // start (or continue) reading from standard input.
      if (curfile < 0) {
         // but only read from cin if no files have previously been read
         newinput = &cin;
      }
      if ((newinput != NULL) && newinput->eof()) {
         return 1;
      }
   }

   return 1;
}



//////////////////////////////
//
// HumdrumFileStream::getFile -- fills a HumdrumFile class with content
//    from the input stream or next input file in the list.  Returns
//    true if content was extracted, fails if there is no more HumdrumFiles
//    in the input stream.
//

int HumdrumFileStream::getFile(HumdrumFile& infile) {
   infile.clear();
   istream* newinput;

restarting:

   newinput = NULL;

   if (urlbuffer.eof()) {
      // If the URL buffer is at its end, clear the buffer.
      urlbuffer.str("");
   }

   // Read HumdrumFile contents from:
   // (1) Current ifstream if open
   // (2) Next filename if ifstream is done
   // (3) cin if no ifstream open and no filenames

   // (1) Is an ifstream open? 
   if (instream.is_open() && !instream.eof()) {
      newinput = &instream;
   } 

   // (1b) Is the URL data buffer open?
   else if (urlbuffer.str() != "") {
      urlbuffer.clear();
      newinput = &urlbuffer;
   }

   // (2) If ifstream is closed but there is a file to be processed,
   // load it into the ifstream and start processing it immediately.
   else if (((int)filelist.size() > 0) && (curfile < (int)filelist.size()-1)) {
      curfile++;
      if (instream.is_open()) {
         instream.close();
      }
      if (strstr(filelist[curfile].c_str(), "://") != NULL) {
         // The next file to read is a URL/URI, so buffer the
         // data from the internet and start reading that instead
         // of reading from a file on the hard disk.
         fillUrlBuffer(urlbuffer, filelist[curfile].c_str());
         infile.setFilename(filelist[curfile].c_str());
         goto restarting;
      }
      instream.open(filelist[curfile].c_str());
      infile.setFilename(filelist[curfile].c_str());
      if (!instream.is_open()) {
         // file does not exist or cannot be opened close
         // the file and try luck with next file in the list
         // (perhaps given an error or warning?).
         infile.setFilename("");
         instream.close();
         goto restarting;
      }
      newinput = &instream;
   } else {
      // no input fstream open and no list of files to process, so
      // start (or continue) reading from standard input.
      if (curfile < 0) {
         // but only read from cin if no files have previously been read
         newinput = &cin;
      }
   }

   // At this point the newinput istream is set to read from the given
   // file or from standard input, so start reading Humdrum content.
   // If there is "newfilebuffer" content, then set the filename of the
   // HumdrumFile to that value. 

   if (newfilebuffer.size() > 0) {
      // store the filename for the current HumdrumFile being read:
      HumRegex hre;
      if (hre.search(newfilebuffer, 
            R"(^!!!!SEGMENT\s*([+-]?\d+)?\s*:\s*(.*)\s*$)")) {
         if (hre.getMatchLength(1) > 0) {
            infile.setSegmentLevel(atoi(hre.getMatch(1).c_str()));
         } else {
            infile.setSegmentLevel(0);
         }
         infile.setFilename(hre.getMatch(2));
      } else if ((curfile >=0) && (curfile < (int)filelist.size()) 
            && (filelist.size() > 0)) {
         infile.setFilename(filelist[curfile].c_str());
      } else {
         // reading from standard input, but no name.
      }
   }

   if (newinput == NULL) {
      // something strange happened, or no more files to read.
      return 0;
   }

   stringstream buffer;
   int foundUniversalQ = 0;

   // Start reading the input stream.  If !!!!SEGMENT: universal comment
   // is found, then store that line in newfilebuffer and return the
   // newly read HumdrumFile.  If other universal comments are found, then
   // overwrite the old universal comments here.
  
   int addedFilename = 0;
   //int searchName = 0;
   int dataFoundQ = 0;
   int starstarFoundQ = 0;
   int starminusFoundQ = 0;
   if (newfilebuffer.size() < 4) {
      //searchName = 1;
   }
   char templine[123123] = {0};

   if (newinput->eof()) {
      if (curfile < (int)filelist.size()-1) {
         curfile++;
         goto restarting;
      }
      // input stream is close and there is no more files to process.
      return 0;
   }

   istream& input = *newinput;

   // if the previous line from the last read starts with "**"
   // then treat it as part of the current file.
   if ((newfilebuffer.size() > 1) && 
       (strncmp(newfilebuffer.c_str(), "**", 2)) == 0) {
      buffer << newfilebuffer << "\n";
      newfilebuffer = "";
      starstarFoundQ = 1;
   }

   while (!input.eof()) {
      input.getline(templine, 123123, '\n');
      if ((!dataFoundQ) && 
            (strncmp(templine, "!!!!SEGMENT", strlen("!!!!SEGMENT")) == 0)) {
         string tempstring;
         tempstring = templine;
			HumRegex hre;
         if (hre.search(tempstring, 
               R"(^!!!!SEGMENT\s*([+-]?\d+)?\s*:\s*(.*)\s*$)")) {
            if (hre.getMatchLength(1) > 0) {
               infile.setSegmentLevel(atoi(hre.getMatch(1).c_str()));
            } else {
               infile.setSegmentLevel(0);
            }
            infile.setFilename(hre.getMatch(2));
         }
         continue;
      }

      if (strncmp(templine, "**", 2) == 0) {
         if (starstarFoundQ == 1) {
            newfilebuffer = templine;
            // already found a **, so this one is defined as a file
            // segment.  Exit from the loop and process the previous
            // content, waiting until the next read for to start with
            // this line.
            break;
         }
         starstarFoundQ = 1;   
      }

      if (input.eof() && (strcmp(templine, "") == 0)) {
         // No more data coming from current stream, so this is 
         // the end of the HumdrumFile.  Break from the while loop
         // and then store the read contents of the stream in the
         // HumdrumFile.
         break;
      } 
      // (1) Does the line start with "!!!!SEGMENT"?  If so, then
      // this is either the name of the current or next file to process.
      // (1a) this is the name of the current file to process if no
      // data has yet been found, 
      // (1b) or a name is being actively searched for.
      if (strncmp(templine, "!!!!SEGMENT", strlen("!!!!SEGMENT")) == 0) {
         newfilebuffer = templine;
         if (dataFoundQ) {
            // this new filename is for the next chunk to process in the
            // current file stream, not this one, so stop reading the 
            // HumdrumFile content and send what has already been read back
            // out with new contents. 
            break;
         }  else {
            // !!!!SEGMENT: came before any real data was read, so
            // it is most likely the name of the current file
            // (i.e., it comes at the start of the file stream and
            // is the name of the first HumdrumFile in the stream).
            HumRegex hre;
            if (hre.search(newfilebuffer, 
                  R"(^!!!!SEGMENT\s*([+-]?\d+)?\s:\s*(.*)\s*$)")) {
               if (hre.getMatchLength(1) > 0) {
                  infile.setSegmentLevel(atoi(hre.getMatch(1).c_str()));
               } else {
                  infile.setSegmentLevel(0);
               }
               infile.setFilename(hre.getMatch(2));
            }
            continue;
         }
      }
      int len = (int)strlen(templine);
      if ((len > 4) && (strncmp(templine, "!!!!", 4) == 0) && 
            (templine[4] != '!') && (dataFoundQ == 0)) {
         // This is a universal comment.  Should it be appended 
         // to the list or should the currnet list be erased and
         // this record placed into the first entry?
         if (foundUniversalQ) {
            // already found a previous universal, so append.
            universals.push_back(templine);
         } else {
            // new universal comment, to delete all previous
            // universal comments and store this one.
            universals.reserve(1000);
            universals.resize(1);
            universals[0] = templine;
            foundUniversalQ = 1;
         }
         continue;
      }

      if (strncmp(templine, "*-", 2) == 0) {
         starminusFoundQ = 1;
      }

      // if before first ** in a data file or after *-, and the line 
      // does not start with '!' or '*', then assume that it is a file 
      // name which should be added to the file list to read.
      if (((starminusFoundQ == 1) || (starstarFoundQ == 0)) 
            && (templine[0] != '*') && (templine[0] != '!')) {
         if ((templine[0] != '\0') && (templine[0] != ' ')) {
            // The file can only be added once in this manner
            // so that infinite loops are prevented.
            int found = 0;
            for (int mm=0; mm<(int)filelist.size(); mm++) {
               if (strcmp(filelist[mm].c_str(), templine) == 0) {
                  found = 1;
               }
            }
            if (!found) {
               filelist.push_back(templine);
               addedFilename = 1;
            }
            continue;
         }
      }

      dataFoundQ = 1; // found something other than universal comments
      // should empty lines be treated somewhat as universal comments?

      // store the data line for later parsing into HumdrumFile record:
      buffer << templine << "\n";
   }

   if (dataFoundQ == 0) {
      // never found anything for some strange reason.
      if (addedFilename) {
         goto restarting;
      }
      return 0;
   }

   // Arriving here means that reading of the data stream is complete.
   // The string stream variable "buffer" contains the HumdrumFile
   // content, so send it to the HumdrumFile variable.  Also, prepend
   // Universal comments (demoted into Global comments) at the start
   // of the data stream (maybe allow for postpending Universal comments
   // in the future).
   stringstream contents;
   for (int i=0; i<(int)universals.size(); i++) {
      contents << &(universals[i][1]) << "\n";
   }
   buffer << ends;
   contents << buffer.str();
   infile.read(contents);
   return 1;
}


//////////////////////////////
//
// HumdrumFileStream::fillUrlBuffer --
//


void HumdrumFileStream::fillUrlBuffer(stringstream& uribuffer,
		const string& uriname) {
	#ifndef USING_URI
   	uribuffer.str(""); // empty any contents in buffer
   	uribuffer.clear(); // reset error flags in buffer
		string webaddress = HumdrumFileBase::getUriToUrlMapping(uriname);
		readStringFromHttpUri(uribuffer, webaddress);
	#endif
}


// END_MERGE

} // end namespace hum



