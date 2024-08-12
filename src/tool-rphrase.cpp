//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 15 23:33:44 PDT 2024
// Last Modified: Sun Aug  4 03:54:55 PDT 2024
// Filename:      tool-rphrase.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-rphrase.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Calculate duration of notes as phrases delimited by rests.
//

#include "tool-rphrase.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_rphrase::Tool_rphrase -- Set the recognized options for the tool.
//

Tool_rphrase::Tool_rphrase(void) {
	define("a|average=b",            "calculate average length of rest-phrases by score");
	define("A|all-average=b",        "calculate average length of rest-phrases for all scores");
	define("B|no-breath=b",          "ignore breath interpretations");
	define("c|composite|collapse=b", "collapse all voices into single part");
	define("d|duration-unit=d:2.0",  "duration units, default: 2.0 (minims/half notes)");
	define("f|filename=b",           "include filename in output analysis");
	define("F|full-filename=b",      "include full filename location in output analysis");
	define("I|no-info=b",            "do not display summary info");
	define("l|longa=b",              "display minim length of longas");
	define("m|b|measure|barline=b",  "include barline numbers in output analysis");
	define("mark=b",                 "mark starts of phrases in score");
	define("s|sort=b",               "sort phrases by short to long length");
	define("S|reverse-sort=b",       "sort phrases by long to short length");
	define("u|url-type=s",           "URL type (jrp, 1520s) for hyperlink");
	define("z|squeeze=b",            "squeeze notation");
	define("close=b",                "close details element initially");
}



/////////////////////////////////
//
// Tool_rphrase::run -- Do the main work of the tool.
//

bool Tool_rphrase::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_rphrase::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_rphrase::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}


bool Tool_rphrase::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_rphrase::finally --
//

void Tool_rphrase::finally(void) {
	if (!m_markQ) {
		if (m_allAverageQ) {
			if (m_compositeQ) {
				double average = m_sumComposite / m_pcountComposite;
				m_free_text << "Composite average phrase length: " << average << " minims" << endl;
			} else {
				double average = m_sum / m_pcount;
				m_free_text << "All average phrase length: " << average << " minims" << endl;
			}
		}
	}
}



//////////////////////////////
//
// Tool_rphrase::initialize --
//

void Tool_rphrase::initialize(void) {
	m_barlineQ      = getBoolean("measure");
	m_allAverageQ   = getBoolean("all-average");
	m_breathQ       = !getBoolean("no-breath");
	m_compositeQ    = getBoolean("collapse");
	m_filenameQ     = getBoolean("filename");
	m_fullFilenameQ = getBoolean("full-filename");
	m_urlType       = getString("url-type");
	m_longaQ        = getBoolean("longa");
	#ifndef __EMSCRIPTEN__
		m_markQ         = getBoolean("mark");
		m_averageQ      = getBoolean("average");
	#else
		m_markQ         = !getBoolean("mark");
		m_averageQ      = !getBoolean("average");
	#endif
	m_sortQ         = getBoolean("sort");
	m_reverseSortQ  = getBoolean("reverse-sort");
	m_durUnit       = getDouble("duration-unit");
	m_infoQ         = !getDouble("no-info");
	m_squeezeQ      = getBoolean("squeeze");
	m_closeQ        = getBoolean("close");
}



//////////////////////////////
//
// Tool_rphrase::processFile --
//

void Tool_rphrase::processFile(HumdrumFile& infile) {
	if (m_filenameQ) {
		m_filename = infile.getFilename();
		HumRegex hre;
		hre.replaceDestructive(m_filename, "", ".*\\/");
		hre.replaceDestructive(m_filename, "", "\\.krn$");
	} else if (m_fullFilenameQ) {
		m_filename = infile.getFilename();
	}
	vector<HTp> kernStarts = infile.getKernSpineStartList();
	vector<Tool_rphrase::VoiceInfo> voiceInfo(kernStarts.size());
	Tool_rphrase::VoiceInfo compositeInfo;

	if (m_compositeQ) {
		fillCompositeInfo(compositeInfo, infile);
	} else {
		fillVoiceInfo(voiceInfo, kernStarts, infile);
	}

	if (m_longaQ) {
		markLongaDurations(infile);
	}

	if ((!m_allAverageQ) && (!m_markQ)) {
		if (m_line == 1) {
			if (m_compositeQ) {
				m_free_text << "Filename";
				if (!m_urlType.empty()) {
					m_free_text << "\tVHV";
				}
				m_free_text << "\tVoice";
				m_free_text << "\tComp seg count";
				if (m_averageQ) {
					m_free_text << "\tAvg comp seg dur";
				}
				m_free_text << "\tComposite seg durs";
				m_free_text << endl;
			} else {
				m_free_text << "Filename";
				if (!m_urlType.empty()) {
					m_free_text << "\tVHV";
				}
				m_free_text << "\tVoice";
				m_free_text << "\tSounding dur";
				m_free_text << "\tResting dur";
				m_free_text << "\tTotal dur";
				m_free_text << "\tSeg count";
				if (m_averageQ) {
					m_free_text << "\tSeg dur average";
				}
				m_free_text << "\tSegment durs";
				m_free_text << endl;
			}
		}
		if (m_compositeQ) {
			if (m_compositeQ) {
				m_line++;
			}
			printVoiceInfo(compositeInfo);
		} else {
			printVoiceInfo(voiceInfo);
		}
	}

	if (m_markQ) {
		outputMarkedFile(infile, voiceInfo, compositeInfo);
		if (m_squeezeQ) {
			m_humdrum_text << "!!!verovio: evenNoteSpacing" << endl;
		}
	}

}



//////////////////////////
//
// Tool_rphrase::markLongaDuratios --
//

void Tool_rphrase::markLongaDurations(HumdrumFile& infile) {
	string longrdf;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].hasSpines()) {
			continue;
		}
		if (!infile[i].isReferenceRecord()) {
			continue;
		}
		string key = infile[i].getReferenceKey();
		if (key != "RDF**kern") {
			continue;
		}
		string value = infile[i].getReferenceValue();
		HumRegex hre;
		if (hre.search(value, "^\\s*([^\\s=]+)\\s*=.*long")) {
			longrdf = hre.getMatch(1);
			break;
		}
	}

	if (longrdf.empty()) {
		return;
	}

	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].hasSpines()) {
			continue;
		}
		if (!infile[i].isData()) {
			continue;
		}
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->find(longrdf) != string::npos) {
				HumNum duration = token->getTiedDuration();
				stringstream value;
				value.str("");
				value << duration.getFloat() / m_durUnit;
				token->setValue("auto", "rphrase-longa", value.str());
			}
		}
	}
}



//////////////////////////////
//
// Tool_rphrase::outputMarkedFile --
//

void Tool_rphrase::outputMarkedFile(HumdrumFile& infile, vector<Tool_rphrase::VoiceInfo>& voiceInfo,
		Tool_rphrase::VoiceInfo& compositeInfo) {
	m_free_text.clear();
	m_free_text.str("");
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			m_humdrum_text << infile[i] << endl;
		} else {
			printDataLine(infile, i);
		}
	}

	if (m_infoQ) {
		printEmbeddedVoiceInfo(voiceInfo, compositeInfo, infile);
	}
}



//////////////////////////////
//
// Tool_rphrase::printDataLine --
//

void Tool_rphrase::printDataLine(HumdrumFile& infile, int index) {

	bool hasLonga = false;
	if (m_longaQ) {
		for (int j=0; j<infile[index].getFieldCount(); j++) {
			HTp token = infile.token(index, j);
			if (!token->isKern()) {
				continue;
			}
			string lotext = token->getValue("auto", "rphrase-longa");
			if (!lotext.empty()) {
				hasLonga = true;
				break;
			}
		}
	}


	bool hasLo = false;
	for (int j=0; j<infile[index].getFieldCount(); j++) {
		HTp token = infile.token(index, j);
		if (!token->isKern()) {
			continue;
		}
		string lotext = token->getValue("auto", "rphrase-start");
		if (!lotext.empty()) {
			hasLo = true;
			break;
		}
	}

	// search for composite phrase info
	bool hasGlo = false;
	if (infile[index].isData()) {
		string glotext = infile[index].getValue("auto", "rphrase-composite-start");
		if (!glotext.empty()) {
			hasGlo = true;
		}
	}

	if (hasGlo) {
		string glotext = infile[index].getValue("auto", "rphrase-composite-start");
		m_humdrum_text << "!!LO:TX:b:B:color=" << m_compositeLengthColor << ":t=" << glotext << endl;
	}

	if (hasLonga) {
		for (int j=0; j<infile[index].getFieldCount(); j++) {
			HTp token = infile.token(index, j);
			if (!token->isKern()) {
				m_humdrum_text << "!";
			} else {
				string value = token->getValue("auto", "rphrase-longa");
				if (value.empty()) {
					m_humdrum_text << "!";
				} else {
					m_humdrum_text << "!LO:TX:a:B:color=silver:t=" << value;
				}
			}
			if (j < infile[index].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << endl;
	}

	if (hasLo) {
		for (int j=0; j<infile[index].getFieldCount(); j++) {
			HTp token = infile.token(index, j);
			if (!token->isKern()) {
				m_humdrum_text << "!";
			} else {
				string value = token->getValue("auto", "rphrase-start");
				if (value.empty()) {
					m_humdrum_text << "!";
				} else {
					m_humdrum_text << "!LO:TX:a:B:color=" << m_voiceLengthColor << ":t=" << value;
				}
			}
			if (j < infile[index].getFieldCount() - 1) {
				m_humdrum_text << "\t";
			}
		}
		m_humdrum_text << endl;
	}

	m_humdrum_text << infile[index] << endl;
}



//////////////////////////////
//
// Tool_rphrase::getCompositeStates --
//

void Tool_rphrase::getCompositeStates(vector<int>& noteStates, HumdrumFile& infile) {
	noteStates.resize(infile.getLineCount());
	fill(noteStates.begin(), noteStates.end(), -1);
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isData()) {
			continue;
		}
		int value = 0;
		for (int j=0; j<infile[i].getFieldCount(); j++) {
			HTp token = infile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isRest()) {
				continue;
			} else if (token->isNull()) {
				HTp resolve = token->resolveNull();
				if (!resolve) {
					continue;
				} else if (resolve->isRest()) {
					continue;
				} else {
					value = 1;
					break;
				}
			} else {
				value = 1;
				break;
			}
		}
		noteStates[i] = value;
	}
}



//////////////////////////////
//
// Tool_rphrase::printVoiceInfo --
//

void Tool_rphrase::printVoiceInfo(vector<Tool_rphrase::VoiceInfo>& voiceInfo) {
	for (int i=(int)voiceInfo.size() - 1; i>=0; i--) {
		if (!m_compositeQ) {
			m_line++;
		}
		printVoiceInfo(voiceInfo[i]);
	}
}


//////////////////////////////
//
// Tool_rphrase::printHyperlink --
//

void Tool_rphrase::printHyperlink(const string& urlType) {
	string command = "rphrase";
	string options;
	options += "l";
	options+= "z";
	if (m_compositeQ) {
		options += "c";
	}
	if (m_sortQ) {
		options += "s";
	} else if (m_reverseSortQ) {
		options += "S";
	}
	if (!options.empty()) {
		command += "%20-";
		command += options;
	}

	if (urlType == "jrp") {
		m_free_text << "=HYPERLINK(\"https://verovio.humdrum.org/?file=jrp/\" & ";
		m_free_text << "LEFT(A" << m_line << ", 3) & \"/\" & A" << m_line << " & ";
		m_free_text << "\".krn&filter=" << command << "&k=ey\", LEFT(A" << m_line;
		m_free_text << ", FIND(\"-\", A" << m_line << ") - 1))";
	} else if (urlType == "1520s") {
		m_free_text << "=HYPERLINK(\"https://verovio.humdrum.org/?file=1520s/\" & A";
		m_free_text << m_line << " & \".krn&filter=" << command << "&k=ey\", LEFT(A";
		m_free_text << m_line << ", FIND(\"-\", A" << m_line << ") - 1))";
	}
}



//////////////////////////////
//
// Tool_rphrase::printVoiceInfo --
//

void Tool_rphrase::printVoiceInfo(Tool_rphrase::VoiceInfo& voiceInfo) {
	if (m_filenameQ) {
		m_free_text << m_filename << "\t";
	}
	if (!m_urlType.empty()) {
		printHyperlink(m_urlType);
		m_free_text << "\t";
	}
	m_free_text << voiceInfo.name << "\t";

	if (!m_compositeQ) {
		double sounding = 0.0;
		double resting = 0.0;
		for (int i=0; i<(int)voiceInfo.phraseDurs.size(); i++) {
			if (voiceInfo.phraseDurs[i] > 0.0) {
				sounding += voiceInfo.phraseDurs[i];
			}
			if (voiceInfo.restsBefore[i] > 0.0) {
				resting += voiceInfo.restsBefore[i];
			}
		}
		double total = sounding + resting;
		// double sounding_percent = int (sounding/total * 100.0 + 0.5);
		// double resting_percent = int (sounding/total * 100.0 + 0.5);

		m_free_text << twoDigitRound(sounding) << "\t";
		m_free_text << twoDigitRound(resting) << "\t";
		m_free_text << twoDigitRound(total) << "\t";
	}

	m_free_text << voiceInfo.phraseDurs.size() << "\t";

	if (m_averageQ) {
		double sum = 0;
		int count = 0;
		for (int i=0; i<(int)voiceInfo.phraseDurs.size(); i++) {
			count++;
			sum += voiceInfo.phraseDurs.at(i);
		}
		m_free_text << int(sum / count * 100.0 + 0.5)/100.0 << "\t";
	}

	if (m_sortQ || m_reverseSortQ) {
		vector<pair<double, int>> sortList;
		for (int i=0; i<(int)voiceInfo.phraseDurs.size(); i++) {
			sortList.emplace_back(voiceInfo.phraseDurs[i], i);
		}
		if (m_sortQ) {
			sort(sortList.begin(), sortList.end(),
				[](const std::pair<double, int>& a, const std::pair<double, int>& b) {
					return a.first < b.first;
			});
		} else if (m_reverseSortQ) {
			sort(sortList.begin(), sortList.end(),
				[](const std::pair<double, int>& a, const std::pair<double, int>& b) {
					return a.first > b.first;
			});
		}

		for (int i=0; i<(int)sortList.size(); i++) {
			int ii = sortList[i].second;
			if (m_barlineQ) {
				m_free_text << "m" << voiceInfo.barStarts.at(ii) << ":";
			}
			m_free_text << twoDigitRound(voiceInfo.phraseDurs.at(ii));
			if (i < (int)sortList.size() - 1) {
				m_free_text << " ";
			}
		}
	} else {
		for (int i=0; i<(int)voiceInfo.phraseDurs.size(); i++) {
			if (voiceInfo.restsBefore.at(i) > 0) {
				m_free_text << "r:" << twoDigitRound(voiceInfo.restsBefore.at(i)) << " ";
			} else if (i > 0) {
				// force display r:0 for section boundaries.
				m_free_text << "r:" << twoDigitRound(voiceInfo.restsBefore.at(i)) << " ";
			}
			if (m_barlineQ) {
				m_free_text << "m" << voiceInfo.barStarts.at(i) << ":";
			}
			m_free_text << twoDigitRound(voiceInfo.phraseDurs.at(i));
			if (i < (int)voiceInfo.phraseDurs.size() - 1) {
				m_free_text << " ";
			}
		}
	}

	m_free_text << endl;
}



//////////////////////////////
//
// Tool_rphrase::printEmbeddedVoiceInfo --
//

void Tool_rphrase::printEmbeddedVoiceInfo(vector<Tool_rphrase::VoiceInfo>& voiceInfo, Tool_rphrase::VoiceInfo& compositeInfo, HumdrumFile& infile) {

	m_humdrum_text << "!!@@BEGIN: PREHTML" << endl;;

	m_humdrum_text << "!!@SCRIPT:" << endl;
	m_humdrum_text << "!!   function rphraseGotoMeasure(measure) {" << endl;
	m_humdrum_text << "!!      let target = `svg .measure.m-${measure}`;" << endl;
	m_humdrum_text << "!!      let element = document.querySelector(target);" << endl;
	m_humdrum_text << "!!      if (element) {" << endl;
	m_humdrum_text << "!!         element.scrollIntoViewIfNeeded({ behavior: 'smooth' });" << endl;
	m_humdrum_text << "!!     }" << endl;
	m_humdrum_text << "!!   }" << endl;

	m_humdrum_text << "!!@CONTENT:\n";
	if (m_compositeQ) {
		m_humdrum_text << "!!<style> .PREHTML .composite ul .rest { color: #ccc; } </style>" << endl;
		m_humdrum_text << "!!<style> .PREHTML .composite ul .measure { color: #ccc; } </style>" << endl;
		m_humdrum_text << "!!<style> .PREHTML .composite ul .length { cursor: pointer; font-weight: bold; color: " << m_compositeLengthColor << "; } </style>" << endl;
	} else {
		m_humdrum_text << "!!<style> .PREHTML table.rphrase .rest { color: #ccc; } </style>" << endl;
		m_humdrum_text << "!!<style> .PREHTML table.rphrase .measure { color: #ccc; } </style>" << endl;
		m_humdrum_text << "!!<style> .PREHTML table.rphrase .length { cursor: pointer; font-weight: bold; color: " << m_voiceLengthColor << "; } </style>" << endl;
		m_humdrum_text << "!!<style> .PREHTML table.rphrase { border-collapse: collapse; </style>" << endl;
		m_humdrum_text << "!!<style> .PREHTML table.rphrase th, .PREHTML table.rphrase td { vertical-align: top; padding-right: 10px; } </style>" << endl;
		m_humdrum_text << "!!<style> .PREHTML table.rphrase tr { border-bottom: 1px solid #ccc; } </style>" << endl;
		m_humdrum_text << "!!<style> .PREHTML table.rphrase tr th:last-child, .PREHTML table.rphrase tr td:last-child { padding-right: 0; } </style>" << endl;

		m_humdrum_text << "!!<style> .PREHTML table.rphrase th.average { text-align: right; } </style>" << endl;
		m_humdrum_text << "!!<style> .PREHTML table.rphrase th.segments { text-align: right; } </style>" << endl;
		m_humdrum_text << "!!<style> .PREHTML table.rphrase th.sounding { text-align: right; } </style>" << endl;
		m_humdrum_text << "!!<style> .PREHTML table.rphrase th.resting { text-align: right; } </style>" << endl;
		m_humdrum_text << "!!<style> .PREHTML table.rphrase td.average { text-align: right; } </style>" << endl;
		m_humdrum_text << "!!<style> .PREHTML table.rphrase td.segments { text-align: right; } </style>" << endl;
		m_humdrum_text << "!!<style> .PREHTML table.rphrase td.sounding { text-align: right; } </style>" << endl;
		m_humdrum_text << "!!<style> .PREHTML table.rphrase td.resting { text-align: right; } </style>" << endl;
	}
	m_humdrum_text << "!!<style> .PREHTML details { position: relative; padding-left: 20px; } </style>" << endl;
	m_humdrum_text << "!!<style> .PREHTML summary { font-size: 1.5rem; cursor: pointer; list-style: none; } </style>" << endl;
	m_humdrum_text << "!!<style> .PREHTML summary::before { content: '▶'; display: inline-block; width: 2em; margin-left: -1.5em; text-align: center; } </style>" << endl;
	m_humdrum_text << "!!<style> .PREHTML details[open] summary::before { content: '▼'; } </style>" << endl;

	if (m_compositeQ) {
		m_humdrum_text << "!!<details";
		if (!m_closeQ) {
			m_humdrum_text << " open";
		}
		m_humdrum_text << "><summary>Composite rest phrasing</summary>\n";
	} else {
		m_humdrum_text << "!!<details";
		if (!m_closeQ) {
			m_humdrum_text << " open";
		}
		m_humdrum_text << "><summary>Voice rest phrasing</summary>\n";
	}
	if (m_compositeQ) {
		printEmbeddedCompositeInfo(compositeInfo, infile);
	} else {
		if (voiceInfo.size() > 0) {
			m_humdrum_text << "!!<table class='rphrase'>" << endl;
			m_humdrum_text << "!!<tr><th class='voice'>Voice</th><th class='sounding'>Sounding</th><th class='resting'>Resting</th><th class='segments'>Segments</th><th class='average'>Average</th><th class='segment-durations'>Segment durations</th></tr>" << endl;
			for (int i=(int)voiceInfo.size() - 1; i>=0; i--) {
				printEmbeddedIndividualVoiceInfo(voiceInfo[i], infile);
			}
			m_humdrum_text << "!!</table>" << endl;
			printEmbeddedVoiceInfoSummary(voiceInfo, infile);
		}
	}
	m_humdrum_text << "!!</details>" << endl;
	m_humdrum_text << "!!@@END: PREHTML" << endl;
}



//////////////////////////////
//
// Tool_rphrase::printEmbeddedCompositeInfo --
//

void Tool_rphrase::printEmbeddedCompositeInfo(Tool_rphrase::VoiceInfo& compositeInfo, HumdrumFile& infile) {

	m_humdrum_text << "!!<div class='composite'>" << endl;
	m_humdrum_text << "!!<ul>" << endl;
	m_humdrum_text << "!!<li>Composite segment count: " << compositeInfo.phraseDurs.size() << "</li>" << endl;

	if (!compositeInfo.phraseDurs.empty()) {
		m_humdrum_text << "!!<li>Composite segment duration";
		if (compositeInfo.phraseDurs.size() != 1) {
			m_humdrum_text << "s";
		}
		m_humdrum_text << ": ";
		if (m_sortQ || m_reverseSortQ) {
			vector<pair<double, int>> sortList;
			for (int i=0; i<(int)compositeInfo.phraseDurs.size(); i++) {
				sortList.emplace_back(compositeInfo.phraseDurs[i], i);
			}
			if (m_sortQ) {
				sort(sortList.begin(), sortList.end(),
					[](const std::pair<double, int>& a, const std::pair<double, int>& b) {
						return a.first < b.first;
				});
			} else if (m_reverseSortQ) {
				sort(sortList.begin(), sortList.end(),
					[](const std::pair<double, int>& a, const std::pair<double, int>& b) {
						return a.first > b.first;
				});
			}

			for (int i=0; i<(int)sortList.size(); i++) {
				int ii = sortList[i].second;
				if (m_barlineQ) {
					m_humdrum_text << "m" << compositeInfo.barStarts.at(ii) << ":";
				}
				m_humdrum_text << "<span class='length' title='measure " << compositeInfo.barStarts.at(ii)
				               << "' onclick='rphraseGotoMeasure(" << compositeInfo.barStarts.at(ii)
				               << ")' >" << twoDigitRound(compositeInfo.phraseDurs.at(ii)) << "</span>";
				if (i < (int)sortList.size() - 1) {
					m_humdrum_text << " ";
				}
			}
		} else {
			for (int i=0; i<(int)compositeInfo.phraseDurs.size(); i++) {
				if (compositeInfo.restsBefore.at(i) > 0) {
					m_humdrum_text << "<span title='inter-phrase rest' class='rest'>" << twoDigitRound(compositeInfo.restsBefore.at(i)) << "</span> ";
				} else if (i > 0) {
					// force display r:0 for section boundaries.
					m_humdrum_text << "<span title='inter-phrase rest' class='rest'>" << twoDigitRound(compositeInfo.restsBefore.at(i)) << "</span> ";
				}
				if (m_barlineQ) {
					m_humdrum_text << "<span class='measure'>m" << compositeInfo.barStarts.at(i) << ":</span>";
				}
				m_humdrum_text << "<span class='length' title='measure " << compositeInfo.barStarts.at(i)
				               << "' onclick='rphraseGotoMeasure(" << compositeInfo.barStarts.at(i)
				               << ")' >" << twoDigitRound(compositeInfo.phraseDurs.at(i)) << "</span>";
				if (i < (int)compositeInfo.phraseDurs.size() - 1) {
					m_humdrum_text << " ";
				}
			}
		}
		m_humdrum_text << "</li>" << endl;

		if (m_averageQ && (compositeInfo.phraseDurs.size() > 1)) {
			double sum = 0;
			int count = 0;
			for (int i=0; i<(int)compositeInfo.phraseDurs.size(); i++) {
				count++;
				sum += compositeInfo.phraseDurs.at(i);
			}
			double average = int(sum / count * 100.0 + 0.5)/100.0;
			m_humdrum_text << "!!<li>Average composite segment durations: " << average << "</li>" << endl;
		}

		m_humdrum_text << "!!<li>Voices: " << getVoiceInfo(infile) << "</li>" << endl;

		if (m_durUnit != 2.0) {
			m_humdrum_text << "!!<li>Duration unit: " << m_durUnit << "</li>" << endl;
		}
	}

	m_humdrum_text << "!!</ul>" << endl;
	m_humdrum_text << "!!</div>" << endl;
}



//////////////////////////////
//
// Tool_rphrase::getVoiceInfo --
//

string Tool_rphrase::getVoiceInfo(HumdrumFile& infile) {
	vector<HTp> kspines = infile.getKernSpineStartList();
	string vcount = to_string(kspines.size());
	string ocount;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (infile[i].isData()) {
			break;
		}
		if (infile[i].isReferenceRecord()) {
			string key = infile[i].getReferenceKey();
			if (key == "voices") {
				ocount = infile[i].getReferenceValue();
			}
		}
	}

	if (ocount.empty()) {
		return vcount;
	}

	if (ocount != vcount) {
		string output = ocount;
		output += "(";
		output += vcount;
		output += ")";
		return output;
	} else {
		return vcount;
	}
}



//////////////////////////////
//
// Tool_rphrase::printEmbeddedVoiceInfoSummary --
//

void Tool_rphrase::printEmbeddedVoiceInfoSummary(vector<Tool_rphrase::VoiceInfo>& voiceInfo, HumdrumFile& infile) {
	m_humdrum_text << "!!<ul>" << endl;

	double total = 0.0;
	for (int i=0; i<(int)voiceInfo[0].phraseDurs.size(); i++) {
		if (voiceInfo[0].phraseDurs[i] > 0.0) {
			total += voiceInfo[0].phraseDurs[i];
		}
		if (voiceInfo[0].restsBefore[i] > 0.0) {
			total += voiceInfo[0].restsBefore[i];
		}
	}
	m_humdrum_text << "!!<li>Score duration: " << twoDigitRound(total) << "</li>" << endl;

	int countSum = 0;
	for (int i=0; i<(int)voiceInfo.size(); i++) {
		countSum += (int)voiceInfo[i].phraseDurs.size();
	}
	m_humdrum_text << "!!<li>Total segments: " << countSum << "</li>" << endl;

	double averageCount = countSum / (double)voiceInfo.size();
	averageCount = (int)(averageCount * 10 + 0.5) / 10.0;
	m_humdrum_text << "!!<li>Average voice segments: " << averageCount << "</li>" << endl;

	double durSum = 0.0;
	for (int i=0; i<(int)voiceInfo.size(); i++) {
		for (int j=0; j<(int)voiceInfo[i].phraseDurs.size(); j++) {
			durSum += voiceInfo[i].phraseDurs[j];
		}
	}
	double averageDur = durSum / countSum;
	averageDur = (int)(averageDur * 10 + 0.5) / 10.0;
	m_humdrum_text << "!!<li>Average segment duration: " << averageDur << "</li>" << endl;

	m_humdrum_text << "!!<li>Voices: " << getVoiceInfo(infile) << "</li>" << endl;

	m_humdrum_text << "!!</ul>" << endl;
}



//////////////////////////////
//
// Tool_rphrase::printEmbeddedIndividualVoiceInfo --
//

void Tool_rphrase::printEmbeddedIndividualVoiceInfo(Tool_rphrase::VoiceInfo& voiceInfo, HumdrumFile& infile) {
	m_humdrum_text << "!!<tr>" << endl;

	m_humdrum_text << "!!<td class='voice'>" << voiceInfo.name << "</td>" << endl;

	double sounding = 0.0;
	double resting = 0.0;
	for (int i=0; i<(int)voiceInfo.phraseDurs.size(); i++) {
		if (voiceInfo.phraseDurs[i] > 0.0) {
			sounding += voiceInfo.phraseDurs[i];
		}
		if (voiceInfo.restsBefore[i] > 0.0) {
			resting += voiceInfo.restsBefore[i];
		}
	}
	double total = sounding + resting;
	double spercent = int(sounding/total * 100.0 + 0.5);
	double rpercent = int(resting/total * 100.0 + 0.5);
	m_humdrum_text << "!!<td class='sounding'>" << sounding << "(" << spercent << "%)</td>" << endl;
	m_humdrum_text << "!!<td class='resting'>" << resting << "(" << rpercent << "%)</td>" << endl;

	// Segment count
	m_humdrum_text << "!!<td class='segments'>";
	m_humdrum_text << voiceInfo.phraseDurs.size();
	m_humdrum_text << "</td>" << endl;

	// Segment duration average
	m_humdrum_text << "!!<td class='average'>";
	double sum = 0;
	int count = 0;
	for (int i=0; i<(int)voiceInfo.phraseDurs.size(); i++) {
		count++;
		sum += voiceInfo.phraseDurs.at(i);
	}
	double average = int(sum / count * 100.0 + 0.5)/100.0;
	m_humdrum_text << average;
	m_humdrum_text << "</td>" << endl;

	// Segments
	m_humdrum_text << "!!<td class='segment-durations'>";
	if (m_sortQ || m_reverseSortQ) {
		vector<pair<double, int>> sortList;
		for (int i=0; i<(int)voiceInfo.phraseDurs.size(); i++) {
			sortList.emplace_back(voiceInfo.phraseDurs[i], i);
		}
		if (m_sortQ) {
			sort(sortList.begin(), sortList.end(),
				[](const std::pair<double, int>& a, const std::pair<double, int>& b) {
					return a.first < b.first;
			});
		} else if (m_reverseSortQ) {
			sort(sortList.begin(), sortList.end(),
				[](const std::pair<double, int>& a, const std::pair<double, int>& b) {
					return a.first > b.first;
			});
		}
		for (int i=0; i<(int)sortList.size(); i++) {
			int ii = sortList[i].second;
			if (m_barlineQ) {
				m_humdrum_text << "<span class='measure'>m" << voiceInfo.barStarts.at(ii) << ":</span>";
			}
			m_humdrum_text << "<span class='length' title='measure " << voiceInfo.barStarts.at(ii)
			               << ")' >" << twoDigitRound(voiceInfo.phraseDurs.at(ii)) << "</span>";
			if (i < (int)sortList.size() - 1) {
				m_humdrum_text << " ";
			}
		}
	} else {
		for (int i=0; i<(int)voiceInfo.phraseDurs.size(); i++) {
			if (voiceInfo.restsBefore.at(i) > 0) {
				m_humdrum_text << "<span title='inter-phrase rest' class='rest'>" << twoDigitRound(voiceInfo.restsBefore.at(i)) << "</span> ";
			} else if (i > 0) {
				// force display r:0 for section boundaries.
				m_humdrum_text << "<span title='inter-phrase rest' class='rest'>" << twoDigitRound(voiceInfo.restsBefore.at(i)) << "</span> ";
			}
			if (m_barlineQ) {
				m_humdrum_text << "<span class='measure'>m" << voiceInfo.barStarts.at(i) << ":</span>";
			}
			m_humdrum_text << "<span class='length' title='measure " << voiceInfo.barStarts.at(i)
			               << "' onclick='rphraseGotoMeasure(" << voiceInfo.barStarts.at(i)
			               << ")' >" << twoDigitRound(voiceInfo.phraseDurs.at(i)) << "</span>";
			if (i < (int)voiceInfo.phraseDurs.size() - 1) {
				m_humdrum_text << " ";
			}
		}
	}

	m_humdrum_text << "</td>" << endl;
	m_humdrum_text << "!!</tr>" << endl;
}



//////////////////////////////
//
// Tool_rphrase::fillCompositeInfo --
//

void Tool_rphrase::fillCompositeInfo(Tool_rphrase::VoiceInfo& compositeInfo, HumdrumFile& infile) {
	compositeInfo.name = getCompositeLabel(infile);
	vector<int> noteStates;
	getCompositeStates(noteStates, infile);

	bool inPhraseQ       = false;
	int currentBarline   = 0;
	int startBarline     = 1;
	HumNum startTime     = 0;
	HumNum restBefore    = 0;
	HumNum startTimeRest = 0;
	HumNum scoreDur      = infile.getScoreDuration();
	HTp phraseStartTok   = NULL;

	for (int i=0; i<infile.getLineCount(); i++) {

		// Split phrases at double barlines (medial cadences):
		if (infile[i].isBarline()) {
			HTp token = infile.token(i, 0);
			if (token->find("||") != string::npos) {
				HumNum tdur = token->getDurationFromStart();
				if (tdur != scoreDur) {
					// Only process if double barline is not at the end of the score.

					if (inPhraseQ) {
						// In phrase, so continue if still notes, otherwise
						// if a rest, then record the currently active phrase
						// that has ended.

						// ending a phrase
						HumNum endTime = infile[i].getDurationFromStart();
						HumNum duration = endTime - startTime;
						startTime = -1;
						inPhraseQ = false;
						double value = duration.getFloat() / m_durUnit;
						compositeInfo.phraseDurs.push_back(value);
						compositeInfo.barStarts.push_back(startBarline);
						compositeInfo.phraseStartToks.push_back(phraseStartTok);
						phraseStartTok = NULL;
						m_sumComposite += duration.getFloat() / m_durUnit;
						m_pcountComposite++;
						double rvalue = restBefore.getFloat() / m_durUnit;
						compositeInfo.restsBefore.push_back(rvalue);

						// record rest start
						startTimeRest = endTime;
					} else {
						// Not in phrase, so not splitting a rest region.
						// This case should be rare (starting a medial cadence
						// with rests and potentially starting new section with rests.
					}

				}
			}
		}

		if (infile[i].isBarline()) {
			HTp token = infile.token(i, 0);
			HumRegex hre;
			if (hre.search(token, "(\\d+)")) {
				currentBarline = hre.getMatchInt(1);
				continue;
			}
		}


		if (!infile[i].isData()) {
			continue;
		}

		if (inPhraseQ) {
			// In phrase, so continue if still notes, otherwise
			// if a rest, then record the currently active phrase
			// that has ended.
			if (noteStates[i] == 0) {
				// ending a phrase
				HumNum endTime = infile[i].getDurationFromStart();
				HumNum duration = endTime - startTime;
				startTime = -1;
				inPhraseQ = false;
				double value = duration.getFloat() / m_durUnit;
				compositeInfo.phraseDurs.push_back(value);
				compositeInfo.barStarts.push_back(startBarline);
				compositeInfo.phraseStartToks.push_back(phraseStartTok);
				phraseStartTok = NULL;
				m_sumComposite += duration.getFloat() / m_durUnit;
				m_pcountComposite++;
				double rvalue = restBefore.getFloat() / m_durUnit;
				compositeInfo.restsBefore.push_back(rvalue);
				// record rest start
				startTimeRest = endTime;
			} else {
				// continuing a phrase, so do nothing
			}
		} else {
			// Not in phrase, so continue if rest; otherwise,
			// if a note, then record a phrase start.
			if (noteStates[i] == 0) {
				// continuing a non-phrase, so do nothing
			} else {
				// starting a phrase
				startTime = infile[i].getDurationFromStart();
				startBarline = currentBarline;
				inPhraseQ = true;
				// check if there are rests before the phrase
				// The rest duration will be stored when the
				// end of the next phrase is encountered.
				if (startTimeRest >= 0) {
					restBefore = startTime - startTimeRest;
				} else {
					restBefore = 0;
				}
				phraseStartTok = infile.token(i, 0);
			}
		}

	}

	if (inPhraseQ) {
		// process last phrase
		HumNum endTime = infile.getScoreDuration();
		HumNum duration = endTime - startTime;
		double value = duration.getFloat() / m_durUnit;
		compositeInfo.phraseDurs.push_back(value);
		compositeInfo.barStarts.push_back(startBarline);
		compositeInfo.phraseStartToks.push_back(phraseStartTok);
		m_sumComposite += duration.getFloat() / m_durUnit;
		m_pcountComposite++;
		double rvalue = restBefore.getFloat() / m_durUnit;
		compositeInfo.restsBefore.push_back(rvalue);
	}

	if (m_markQ) {
		markCompositePhraseStartsInScore(infile, compositeInfo);
	}
}



//////////////////////////////
//
// Tool_rphrase::getCompositeLabel --
//

string Tool_rphrase::getCompositeLabel(HumdrumFile& infile) {
	string voices;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isReferenceRecord()) {
			continue;
		}
		string key = infile[i].getReferenceKey();
		if (key != "voices") {
			continue;
		}
		voices = infile[i].getReferenceValue();
		break;
	}

	if (voices.empty()) {
		return "composite";
	}

	vector<HTp> kstarts = infile.getKernSpineStartList();

	string output = "composite ";
	output += voices;


	HumRegex hre;

	if (hre.search(voices, "^\\d+$")) {
		int vint = stoi(voices);
		if (vint != (int)kstarts.size()) {
			output += "(";
			output += to_string(kstarts.size());
			output += ")";
		}
	} else {
		output += "(";
		output += to_string(kstarts.size());
		output += ")";
	}

	return output;
}



//////////////////////////////
//
// Tool_rphrase::fillVoiceInfo --
//

void Tool_rphrase::fillVoiceInfo(vector<Tool_rphrase::VoiceInfo>& voiceInfo,
		vector<HTp>& kstarts, HumdrumFile& infile) {
	for (int i=0; i<(int)kstarts.size(); i++) {
		fillVoiceInfo(voiceInfo.at(i), kstarts.at(i), infile);
	}
}


void Tool_rphrase::fillVoiceInfo(Tool_rphrase::VoiceInfo& voiceInfo, HTp& kstart, HumdrumFile& infile) {
	HTp current = kstart;

	bool inPhraseQ       = false;
	int currentBarline   = 0;
	int startBarline     = 1;
	HumNum startTime     = 0;

	HumNum restBefore    = 0;
	HumNum startTimeRest = 0;

	HumNum scoreDur = infile.getScoreDuration();
	HTp phraseStartTok = NULL;

	while (current) {

		// Split phrases at double barlines (medial cadences):
		if (infile[current->getLineIndex()].isBarline()) {
			HTp token = infile.token(current->getLineIndex(), 0);
			if (token->find("||") != string::npos) {
				HumNum tdur = token->getDurationFromStart();
				if (tdur != scoreDur) {
					// Only process if double barline is not at the end of the score.

					if (inPhraseQ) {
						// In phrase, so continue if still notes, otherwise
						// if a rest, then record the currently active phrase
						// that has ended.

						HumNum endTime = current->getDurationFromStart();
						HumNum duration = endTime - startTime;
						startTime = -1;
						inPhraseQ = false;
						double value = duration.getFloat() / m_durUnit;
						voiceInfo.phraseDurs.push_back(value);
						voiceInfo.barStarts.push_back(startBarline);
						voiceInfo.phraseStartToks.push_back(phraseStartTok);
						phraseStartTok = NULL;
						m_sum += duration.getFloat() / m_durUnit;
						m_pcount++;
						double rvalue = restBefore.getFloat() / m_durUnit;
						voiceInfo.restsBefore.push_back(rvalue);

						// record rest start
						startTimeRest = endTime;
					} else {
						// Not in phrase, so not splitting a rest region.
						// This case should be rare (starting a medial cadence
						// with rests and potentially starting new section with rests.
					}

				}
			}
		}

		if (current->isBarline()) {
			HumRegex hre;
			if (hre.search(current, "(\\d+)")) {
				currentBarline = hre.getMatchInt(1);
				current = current->getNextToken();
				continue;
			}
		}

		if (current->isInstrumentName()) {
			voiceInfo.name = current->substr(3);
		}
		if (!(current->isData() || (m_breathQ && (*current == "*breath")))) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull()) {
			current = current->getNextToken();
			continue;
		}

		if (inPhraseQ) {
			// In phrase, so continue if still notes, otherwise
			// if a rest, then record the currently active phrase
			// that has ended.
			if (current->isRest() || (*current == "*breath")) {
				// ending a phrase
				HumNum endTime = current->getDurationFromStart();
				HumNum duration = endTime - startTime;
				startTime = -1;
				inPhraseQ = false;
				double value = duration.getFloat() / m_durUnit;
				voiceInfo.phraseDurs.push_back(value);
				voiceInfo.barStarts.push_back(startBarline);
				voiceInfo.phraseStartToks.push_back(phraseStartTok);
				phraseStartTok = NULL;
				m_sum += duration.getFloat() / m_durUnit;
				m_pcount++;
				double rvalue = restBefore.getFloat() / m_durUnit;
				voiceInfo.restsBefore.push_back(rvalue);
				// record rest start
				startTimeRest = endTime;
			} else {
				// continuing a phrase, so do nothing
			}
		} else {
			// Not in phrase, so continue if rest; otherwise,
			// if a note, then record a phrase start.
			if (current->isRest() || (*current == "*breath")) {
				// continuing a non-phrase, so do nothing
			} else {
				// starting a phrase
				startTime = current->getDurationFromStart();
				startBarline = currentBarline;
				inPhraseQ = true;
				// check if there are rests before the phrase
				// The rest duration will be stored when the
				// end of the next phrase is encountered.
				if (startTimeRest >= 0) {
					restBefore = startTime - startTimeRest;
				} else {
					restBefore = 0;
				}
				phraseStartTok = current;
			}
		}

		current = current->getNextToken();
	}
	if (inPhraseQ) {
		// process last phrase
		HumNum endTime = kstart->getLine()->getOwner()->getScoreDuration();
		HumNum duration = endTime - startTime;
		double value = duration.getFloat() / m_durUnit;
		voiceInfo.phraseDurs.push_back(value);
		voiceInfo.barStarts.push_back(startBarline);
		voiceInfo.phraseStartToks.push_back(phraseStartTok);
		m_sum += duration.getFloat() / m_durUnit;
		m_pcount++;
		double rvalue = restBefore.getFloat() / m_durUnit;
		voiceInfo.restsBefore.push_back(rvalue);
	}

	if (m_markQ) {
		markPhraseStartsInScore(infile, voiceInfo);
	}
}



//////////////////////////////
//
// Tool_rphrase::markCompositePhraseStartsInScore --
//

void Tool_rphrase::markCompositePhraseStartsInScore(HumdrumFile& infile, Tool_rphrase::VoiceInfo& compositeInfo) {
	stringstream buffer;
	for (int i=0; i<(int)compositeInfo.phraseStartToks.size(); i++) {
		HTp tok = compositeInfo.phraseStartToks.at(i);
		string measure = "";
		if (m_barlineQ) {
			measure = to_string(compositeInfo.barStarts.at(i));
		}
		double duration = compositeInfo.phraseDurs.at(i);
		buffer.str("");
		if (!measure.empty()) {
			buffer << "m" << measure << "&colon;";
		}
		buffer << twoDigitRound(duration);
		int lineIndex = tok->getLineIndex();
		infile[lineIndex].setValue("auto", "rphrase-composite-start", buffer.str());
	}
}



//////////////////////////////
//
// Tool_rphrase::twoDigitRound --
//

double Tool_rphrase::twoDigitRound(double input) {
	return int(input * 100.0 + 0.499999) / 100.0;
}



//////////////////////////////
//
// Tool_rphrase::markPhraseStartsInScore --
//

void Tool_rphrase::markPhraseStartsInScore(HumdrumFile& infile, Tool_rphrase::VoiceInfo& voiceInfo) {
	stringstream buffer;
	for (int i=0; i<(int)voiceInfo.phraseStartToks.size(); i++) {
		HTp tok = voiceInfo.phraseStartToks.at(i);
		string measure = "";
		if (m_barlineQ) {
			measure = to_string(voiceInfo.barStarts.at(i));
		}
		double duration = voiceInfo.phraseDurs.at(i);
		buffer.str("");
		if (!measure.empty()) {
			buffer << "m" << measure << "&colon;";
		}
		buffer << duration;
		tok->setValue("auto", "rphrase-start", buffer.str());
	}
}


// END_MERGE

} // end namespace hum



