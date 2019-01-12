//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Oct  5 23:16:26 PDT 2015
// Last Modified: Mon Oct  5 23:16:29 PDT 2015
// Filename:      HumdrumFileContent-tie.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumdrumFileContent-tie.cpp
// Syntax:        C++11
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Links tie starting/continuing/ending points to each other.
//

#include "HumdrumFileContent.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE



//////////////////////////////
//
// HumdrumFileContent::analyzeKernTies -- Link start and ends of
//    ties to each other.
//

bool HumdrumFileContent::analyzeKernTies(void) {
	vector<pair<HTp, int>> linkedtiestarts;
	vector<pair<HTp, int>> linkedtieends;

	vector<HTp> kernspines;
	getSpineStartList(kernspines, "**kern");
	bool output = true;
	string linkSignifier = m_signifiers.getKernLinkSignifier();
	output = analyzeKernTies(linkedtiestarts, linkedtieends, linkSignifier);
	createLinkedTies(linkedtiestarts, linkedtieends);
	return output;
}


//
// Could be generalized to allow multiple grand-staff pairs by limiting
// the search spines for linking (probably with *part indications).
// Currently all spines are checked for linked ties.
//

bool HumdrumFileContent::analyzeKernTies(
		vector<pair<HTp, int>>& linkedtiestarts,
		vector<pair<HTp, int>>& linkedtieends,
		string& linkSignifier) {

	// Use this in the future to limit to grand-staff search (or 3 staves for organ):
	// vector<vector<HTp> > tracktokens;
	// this->getTrackSeq(tracktokens, spinestart, OPT_DATA | OPT_NOEMPTY);

	// Only analyzing linked ties for now (others ties are handled without analysis in 
	// the hum2mei converter, for example.
	if (linkSignifier.empty()) {
		return true;
	}

	string lstart  = linkSignifier + "[";
	string lmiddle = linkSignifier + "_";
	string lend    = linkSignifier + "]";

	vector<pair<HTp, int>> startdatabase(400);

	for (int i=0; i<(int)startdatabase.size(); i++) {
		startdatabase[i].first  = NULL;
		startdatabase[i].second = -1;
	}

	HumdrumFileContent& infile = *this;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp tok = infile.token(i, j);
			if (!tok->isKern()) {
				continue;
			}
			if (!tok->isData()) {
				continue;
			}
			if (tok->isNull()) {
				continue;
			}
			if (tok->isRest()) {
				continue;
			}
			int scount = tok->getSubtokenCount();
			int b40;
			for (int k=0; k<scount; k++) {
				int index = k;
				if (scount == 1) {
					index = -1;
				}
				std::string tstring = tok->getSubtoken(k);
				if (tstring.find(lstart) != std::string::npos) {
					b40 = Convert::kernToBase40(tstring);
					startdatabase[b40].first  = tok;
					startdatabase[b40].second = index;
					// linkedtiestarts.push_back(std::make_pair(tok, index));
				}
				if (tstring.find(lend) != std::string::npos) {
					b40 = Convert::kernToBase40(tstring);
					if (startdatabase.at(b40).first) {
						linkedtiestarts.push_back(startdatabase[b40]);
						linkedtieends.push_back(std::make_pair(tok, index));
						startdatabase[b40].first  = NULL;
						startdatabase[b40].second = -1;
					}
				}
				if (tstring.find(lmiddle) != std::string::npos) {
					b40 = Convert::kernToBase40(tstring);
					if (startdatabase[b40].first) {
						linkedtiestarts.push_back(startdatabase[b40]);
						linkedtieends.push_back(std::make_pair(tok, index));
					}
					startdatabase[b40].first  = tok;
					startdatabase[b40].second = index;
					// linkedtiestarts.push_back(std::make_pair(tok, index));
					// linkedtieends.push_back(std::make_pair(tok, index));
				}
			}
		}
	}

	return true;
}



//////////////////////////////
//
// HumdrumFileContent::createLinkedTies --
//

void HumdrumFileContent::createLinkedTies(vector<pair<HTp, int>>& linkstarts,
	vector<pair<HTp, int>>& linkends) {
   int max = (int)linkstarts.size();
   if ((int)linkends.size() < max) {
      max = (int)linkends.size();
   }
   if (max == 0) {
      // nothing to do
      return;
   }

   for (int i=0; i<max; i++) {
      linkTieEndpoints(linkstarts[i].first, linkstarts[i].second,
				linkends[i].first, linkends[i].second);
   }

}


//////////////////////////////
//
// HumdrumFileContent::linkTieEndpoints --
//

void HumdrumFileContent::linkTieEndpoints(HTp tiestart,
		int startindex, HTp tieend, int endindex) {

   string durtag   = "tieDuration";
   string starttag = "tieStart";
   string endtag   = "tieEnd";
	string startnum = "tieStartSubtokenNumber";
	string endnum   = "tieEndSubtokenNumber";

	int startnumber = startindex + 1;
	int endnumber   = endindex + 1;

	if (tiestart->isChord()) {
		if (startnumber > 0) {
			durtag   += to_string(startnumber);
			endnum   += to_string(startnumber);
			endtag   += to_string(startnumber);
		}
	}
	if (tieend->isChord()) {
		if (endnumber > 0) {
			starttag += to_string(endnumber);
			startnum += to_string(endnumber);
		}
	}

   tiestart->setValue("auto", endtag, tieend);
   tiestart->setValue("auto", "id", tiestart);
	if (endnumber > 0) {
		tiestart->setValue("auto", endnum, to_string(endnumber));
	}

   tieend->setValue("auto", starttag, tiestart);
   tieend->setValue("auto", "id", tieend);
	if (startnumber > 0) {
		tieend->setValue("auto", startnum, to_string(startnumber));
	}

   HumNum duration = tieend->getDurationFromStart()
         - tiestart->getDurationFromStart();
   tiestart->setValue("auto", durtag, duration);
}



// END_MERGE

} // end namespace hum



