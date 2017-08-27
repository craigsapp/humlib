//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Aug 17 02:39:28 PDT 2015
// Last Modified: Mon Aug 17 02:39:32 PDT 2015
// Filename:      HumdrumFile.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFile.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Place-holder class to serve as interface to HumdrumFileBase,
//                HumdrumFileStructure and HumdrumFileContent.
//

#include "HumdrumFile.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// HumdrumFile::HumdrumFile -- HumdrumFile constructor.
//

HumdrumFile::HumdrumFile(void) {
	// do nothing
}

HumdrumFile::HumdrumFile(const string& filename) :
		HUMDRUMFILE_PARENT() {
	read(filename);
}


HumdrumFile::HumdrumFile(istream& contents) :
		HUMDRUMFILE_PARENT() {
	read(contents);
}



//////////////////////////////
//
// HumdrumFile::~HumdrumFile -- HumdrumFile deconstructor.
//

HumdrumFile::~HumdrumFile() {
	// do nothing
}



//////////////////////////////
//
// HumdrumFile::printXml -- Print a HumdrumFile object in XML format.
// default value: level = 0
// default value: indent = tab character
//

ostream& HumdrumFile::printXml(ostream& out, int level,
		const string& indent) {
	out << Convert::repeatString(indent, level) << "<sequence>\n";
	level++;

	out << Convert::repeatString(indent, level) << "<sequenceInfo>\n";
	level++;
	
	out << Convert::repeatString(indent, level) << "<frameCount>";
	out << getLineCount() << "</frameCount>\n";
	

	out << Convert::repeatString(indent, level) << "<tpq>";
	out << tpq() << "</tpq>\n";

	// Starting at 0 by default (single segment only).  Eventually
	// add parameter to set the starting time of the sequence, which
	// would be the duration of all previous segments before this one.
	out << Convert::repeatString(indent, level) << "<sequenceStart";
	out << Convert::getHumNumAttributes(0);
	out << "/>\n";

	out << Convert::repeatString(indent, level) << "<sequenceDuration";
	out << Convert::getHumNumAttributes(getScoreDuration());
	out << "/>\n";

	out << Convert::repeatString(indent, level) << "<trackInfo>\n";
	level++;

	out << Convert::repeatString(indent, level) << "<trackCount>";
	out << getMaxTrack() << "</trackCount>\n";

	for (int i=1; i<=getMaxTrack(); i++) {
		out << Convert::repeatString(indent, level) << "<track";
		out << " n=\"" << i << "\"";
		HumdrumToken* trackstart = getTrackStart(i);
		if (trackstart != NULL) {
			out << " dataType=\"" <<  trackstart->getDataType().substr(2) << "\"";
			out << " startId=\"" <<  trackstart->getXmlId() << "\"";
		}
		HumdrumToken* trackend = getTrackEnd(i, 0);
		if (trackend != NULL) {
			out << " endId =\"" <<  trackend->getXmlId() << "\"";
		}
		out << "/>\n";
	}

	level--;
	out << Convert::repeatString(indent, level) << "</trackInfo>\n";

	printXmlParameterInfo(out, level, "\t");

	level--;
	out << Convert::repeatString(indent, level) << "</sequenceInfo>\n";

	out << Convert::repeatString(indent, level) << "<frames>\n";
	level++;
	for (int i=0; i<getLineCount(); i++) {
		m_lines[i]->printXml(out, level, indent);
	}
	level--;
	out << Convert::repeatString(indent, level) << "</frames>\n";

	level--;
	out << Convert::repeatString(indent, level) << "</sequence>\n";

	return out;
}



//////////////////////////////
//
// HumdrumFile::printXmlParameterInfo -- Print contents of HumHash for HumdrumFile.
// default value: out = cout
// default value: level = 0
// default value: indent = "\t"
//

ostream& HumdrumFile::printXmlParameterInfo(ostream& out, int level,
		const string& indent) {
	((HumHash*)this)->printXml(out, level, indent);
	return out;
}


// END_MERGE

} // end namespace hum



