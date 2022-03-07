//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jul 11 06:34:40 PDT 2020
// Last Modified: Sat Jul 11 06:34:43 PDT 2020
// Filename:      HumdrumFileContent-barline.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-barline.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Analyze barlines.  This is used to identify if all barlines
//                on a line are the same style or not.  This information is used
//                to convert the music notation into MEI data, with a hybrid
//                system of measure/barline elements when the barlines
//                are the same or not.
//

#include "HumdrumFileContent.h"

using namespace std;

namespace hum {

// START_MERGE



//////////////////////////////
//
// HumdrumFileContent::analyzeBarlines --
//

void HumdrumFileContent::analyzeBarlines(void) {
	if (m_analyses.m_barlines_analyzed) {
		// Maybe allow forcing reanalysis.
		return;
	}
	m_analyses.m_slurs_analyzed = true;
	m_analyses.m_barlines_different = false;

	string baseline;
	string comparison;
	bool baseQ;

	HumdrumFileContent& infile = *this;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			continue;
		}
		bool allSame = true;
		int fieldcount = infile[i].getFieldCount();
		if (fieldcount <= 1) {
			continue;
		}
		baseQ = false;
		baseline = "";
		comparison = "";
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile[i].token(j);
			int subtrack = token->getSubtrack();
			if (subtrack > 1) {
				// ignore secondary barlines in subspines.
				continue;
			}
			if (!token->isStaff()) {
				// don't check non-staff barlines
				continue;
			}
			if (!baseQ) {
				baseline = "";
				for (int k=0; k<(int)token->size(); k++) {
					if (isdigit(token->at(k))) {
						// ignore barnumbers
						// maybe ignore fermatas
						continue;
					}
					if (token->at(k) == ';') {
						// ignore fermatas in comparison
						continue;
					}
					baseline += token->at(k);
				}
				baseQ = true;
			} else {
				comparison = "";
				for (int k=0; k<(int)token->size(); k++) {
					if (isdigit(token->at(k))) {
						// ignore barnumbers;
						// maybe ignore fermatas
						continue;
					}
					if (token->at(k) == ';') {
						// ignore fermatas in comparison
						continue;
					}
					comparison += token->at(k);
				}
				if (comparison != baseline) {
					allSame = false;
					break;
				}
			}
		}

		if (hasDataStraddle(i)) {
				infile[i].setValue("auto", "straddlingData", 1);
		} else {
				infile[i].setValue("auto", "straddlingData", 0);
		}

		if (!allSame) {
			infile[i].setValue("auto", "barlinesDifferent", 1);
			m_analyses.m_barlines_different = true;
		} else {
			infile[i].setValue("auto", "barlinesDifferent", 0);
		}
	}
}



//////////////////////////////
//
// HumdrumFileContent::hasDataStraddle -- Returns true if the next
//    data line after a barline has null tokens on isStaff() tokens.
//    If there are no data lines after the barline, then it will
//    return false;
//

bool HumdrumFileContent::hasDataStraddle(int line) {
	HumdrumFileContent& infile = *this;
	if (!infile[line].isBarline()) {
		return false;
	}
	for (int i=line+1; i<infile.getLineCount(); i++) {
		if (infile[i].isInterpretation()) {
			HTp token = infile.token(i, 0);
			if (*token == "*-") {
				return false;
			}
		}
		if (!infile[i].isData()) {
			continue;
		}
		if (infile[i].isGraceLine()) {
			continue;
		}

		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isStaff()) {
				continue;
			}
			if (token->isNull()) {
				return true;
			}
		}
		break;
	}
	return false;
}



//////////////////////////////
//
// HumdrumFileContent::hasDifferentBarlines --
//

bool HumdrumFileContent::hasDifferentBarlines(void) {
	if (!m_analyses.m_barlines_analyzed) {
		analyzeBarlines();
	}
	return m_analyses.m_barlines_different;
}




// END_MERGE

} // end namespace hum



