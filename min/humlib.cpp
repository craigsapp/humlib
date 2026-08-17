//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Aug  8 12:24:49 PDT 2015
// Last Modified: Mon Aug 17 10:00:26 PDT 2026
// Filename:      min/humlib.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/min/humlib.cpp
// Syntax:        C++11
// vim:           ts=3
//
// Description:   Source file for humlib library.
//
/*
https://github.com/craigsapp/humlib
Copyright (c) 2015-2021 Craig Stuart Sapp
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   and the following disclaimer in the documentation and/or other materials
   provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "humlib.h"

namespace hum {



/////////////////////////////////
//
// Tool_texture::Tool_texture -- Set the recognized options for the tool.
//

Tool_texture::Tool_texture(void) {
	define("1|file=b", "Display only first verse for each part");
}



/////////////////////////////////
//
// Tool_texture::run -- Do the main work of the tool.
//

bool Tool_texture::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_texture::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_texture::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}



bool Tool_texture::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_texture::initialize --  Initializations that only have to be done once
//    for all HumdrumFile segments.
//

void Tool_texture::initialize(void) {
	m_fileQ       = getBoolean("file");
}


/////////////////////////////
//
// Tool_texture::processfile --
//

void Tool_texture::processFile(HumdrumFile& infile) {

}





class Tool_texture : public HumTool {
	class VoiceState {
		public:
		string file:qname;
		int monophony = false;
		int chordal   = false;
		int polyphony = false;
		void addToken(HTp token);
	};

	public:
		         Tool_texture       (void);
		        ~Tool_texture       () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void     processFile       (HumdrumFile& infile);
		void     initialize        (void);

	private:
		bool     m_fileQ     = false;

};


} // end namespace hum
