//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed Sep 13 14:58:26 PDT 2017
// Last Modified: Thu Sep 21 13:10:56 PDT 2017
// Filename:      mei2hum.cpp
// URL:           https://github.com/craigsapp/mei2hum/blob/master/src/mei2hum.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Convert an MEI file into a Humdrum file.
//

#include "tool-mei2hum.h"
#include "HumGrid.h"
#include "HumRegex.h"

#include <string.h>
#include <stdlib.h>

#include <algorithm>

using namespace std;
using namespace pugi;

namespace hum {

// START_MERGE


#define QUARTER_CONVERT * 4
#define ELEMENT_DEBUG_STATEMENT(X)
// #define ELEMENT_DEBUG_STATEMENT(X)  cerr << #X << endl;


#define NODE_VERIFY(ELEMENT, RETURNVALUE)        \
	if (!ELEMENT) {                               \
		return RETURNVALUE;                        \
	}                                             \
	if (strcmp(ELEMENT.name(), #ELEMENT) != 0) {  \
		return RETURNVALUE;                        \
	}                                             \
	ELEMENT_DEBUG_STATEMENT(ELEMENT)

#define MAKE_CHILD_LIST(VARNAME, ELEMENT)        \
	vector<xml_node> VARNAME;                     \
	getChildrenVector(VARNAME, ELEMENT);


#define DKHTP "Don't know how to process "

#define CURRLOC " in measure " << m_currentMeasure


//////////////////////////////
//
// Tool_mei2hum::Tool_mei2hum --
//

Tool_mei2hum::Tool_mei2hum(void) {
	define("app|app-label=s", "app label to follow");
	define("r|recip=b", "output **recip spine");
	define("s|stems=b", "include stems in output");

	m_maxverse.resize(m_maxstaff);
	fill(m_maxverse.begin(), m_maxverse.end(), 0);

	m_measureDuration.resize(m_maxstaff);
	fill(m_measureDuration.begin(), m_measureDuration.end(), 0);

	m_currentMeterUnit.resize(m_maxstaff);
	fill(m_currentMeterUnit.begin(), m_currentMeterUnit.end(), 4);

	m_hasDynamics.resize(m_maxstaff);
	fill(m_hasDynamics.begin(), m_hasDynamics.end(), false);

	
}



//////////////////////////////
//
// Tool_mei2hum::convert -- Convert an MEI file into
//     Humdrum content.
//

bool Tool_mei2hum::convertFile(ostream& out, const char* filename) {
	xml_document doc;
	auto result = doc.load_file(filename);
	if (!result) {
		cerr << "\nXML file [" << filename << "] has syntax errors\n";
		cerr << "Error description:\t" << result.description() << "\n";
		cerr << "Error offset:\t" << result.offset << "\n\n";
		exit(1);
	}

	return convert(out, doc);
}


bool Tool_mei2hum::convert(ostream& out, istream& input) {
	string s(istreambuf_iterator<char>(input), {});
	return convert(out, s.c_str());
}


bool Tool_mei2hum::convert(ostream& out, const char* input) {
	xml_document doc;
	auto result = doc.load(input);
	if (!result) {
		cout << "\nXML content has syntax errors\n";
		cout << "Error description:\t" << result.description() << "\n";
		cout << "Error offset:\t" << result.offset << "\n\n";
		exit(1);
	}

	return convert(out, doc);
}



bool Tool_mei2hum::convert(ostream& out, xml_document& doc) {
	initialize();

	bool status = true; // for keeping track of problems in conversion process.

	buildIdLinkMap(doc);

	auto score = doc.select_node("/mei/music/body/mdiv/score").node();

	if (!score) {
		cerr << "Cannot find score, so cannot convert ";
		cerr << "MEI file to Humdrum" << endl;
		return false;
	}

	m_staffcount = extractStaffCount(score);

	if (m_recipQ) {
		m_outdata.enableRecipSpine();
	}

	HumNum systemstamp = 0;  // timestamp for music.
	systemstamp = parseScore(score, systemstamp);

	m_outdata.removeRedundantClefChanges();
	// m_outdata.removeSibeliusIncipit();

	// set the duration of the last slice

	HumdrumFile outfile;


	// Report verse counts for each staff to HumGrid:
	for (int i=0; i<m_maxverse.size(); i++) {
		if (m_maxverse[i] == 0) {
			continue;
		}
		m_outdata.setVerseCount(i, 0, m_maxverse[i]);
	}

	// Report dynamic presence for each staff to HumGrid:
	for (int i=0; i<m_hasDynamics.size(); i++) {
		if (m_hasDynamics[i] == false) {
			continue;
		}
		m_outdata.setDynamicsPresent(i);
	}

	m_outdata.transferTokens(outfile);

	addHeaderRecords(outfile, doc);
	addExtMetaRecords(outfile, doc);
	addFooterRecords(outfile, doc);

	for (int i=0; i<outfile.getLineCount(); i++) {
		outfile[i].createLineFromTokens();
	}
	out << outfile;

	return status;
}



//////////////////////////////
//
// Tool_mei2hum::addExtMetaRecords --
//

void Tool_mei2hum::addExtMetaRecords(HumdrumFile& outfile, xml_document& doc) {
	pugi::xpath_node_set metaframes = doc.select_nodes("/mei/meiHead/extMeta/frames/metaFrame");
	double starttime;
	string starttimevalue;
	string token;
	xml_node node;
	xml_node timenode;

	// place header reference records, assumed to be time sorted
	for (int i=(int)metaframes.size()-1; i>=0; i--) {
		node = metaframes[i].node();
		timenode = node.select_node("./frameInfo/startTime").node();
		starttimevalue = timenode.attribute("float").value();
		if (starttimevalue == "") {
			starttime = 0.0;
		} else {
			starttime = stof(starttimevalue);
		}
		if (starttime > 0.0) {
			continue;
		}
		token = node.attribute("token").value();
		if (token.empty()) {
			continue;
		}
		outfile.insertLine(0, token);
		if (token.find("!!!RDF**kern: < = below") != string::npos) {
			m_belowQ = false;
		}
		if (token.find("!!!RDF**kern: > = above") != string::npos) {
			m_aboveQ = false;
		}
	}

	// place footer reference records, assumed to be time sorted
	for (int i=0; i<(int)metaframes.size(); i++) {
		node = metaframes[i].node();
		timenode = node.select_node("./frameInfo/startTime").node();
		starttimevalue = timenode.attribute("float").value();
		if (starttimevalue == "") {
			starttime = 0.0;
		} else {
			starttime = stof(starttimevalue);
		}
		if (starttime == 0.0) {
			continue;
		}
		token = node.attribute("token").value();
		if (token.empty()) {
			continue;
		}
		outfile.appendLine(token);
		if (token.find("!!!RDF**kern: < = below") != string::npos) {
			m_belowQ = false;
		}
		if (token.find("!!!RDF**kern: > = above") != string::npos) {
			m_aboveQ = false;
		}
	}

}



//////////////////////////////
//
// Tool_mei2hum::addHeaderRecords --
//

void Tool_mei2hum::addHeaderRecords(HumdrumFile& outfile, xml_document& doc) {

	// title is at /mei/meiHead/fileDesc/titleStmt/title
	string title = cleanReferenceRecordText(doc.select_node("/mei/meiHead/fileDesc/titleStmt/title").node().child_value());

	// composer is at /mei/meiHead/fileDesc/titleStmt/respStmt/persName@role="creator"
	string composer = cleanReferenceRecordText(doc.select_node("/mei/meiHead/fileDesc/titleStmt/respStmt/persName[@role='creator']").node().child_value());

	// lyricist is at /mei/meiHead/fileDesc/titleStmt/respStmt/persName@role="lyricist"
	string lyricist = cleanReferenceRecordText(doc.select_node("/mei/meiHead/fileDesc/titleStmt/respStmt/persName[@role='lyricist']").node().child_value());

	if (!title.empty()) {
		outfile.insertLine(0, "!!!OTL: " + title);
	}
	if (!lyricist.empty()) {
		outfile.insertLine(0, "!!!LYR: " + lyricist);
	}
	if (!composer.empty()) {
		outfile.insertLine(0, "!!!COM: " + composer);
	}
}



//////////////////////////////
//
// Tool_mei2hum::addFooterRecords --
//

void Tool_mei2hum::addFooterRecords(HumdrumFile& outfile, xml_document& doc) {
	if (m_aboveQ) {
		outfile.appendLine("!!!RDF**kern: > = above");
	}
	if (m_belowQ) {
		outfile.appendLine("!!!RDF**kern: < = below");
	}
	if (m_editorialAccidentalQ) {
		outfile.appendLine("!!!RDF**kern: i = editorial accidental");
	}
}



//////////////////////////////
//
// Tool_mei2hum::extractStaffCount -- Count the number of staves in the score.
//

int Tool_mei2hum::extractStaffCount(xml_node element) {
	auto measure = element.select_node("//measure").node();
	if (!measure) {
		return 0;
	}

	int count = 0;
	for (xml_node child : measure.children()) {
		string nodename = child.name();
		if (nodename == "staff") {
			count++;
		}
	}
	return count;
}



///////////////////////////////////
//
// Tool_mei2hum::parseScore -- Convert an MEI <score> element into Humdrum data.
//

HumNum Tool_mei2hum::parseScore(xml_node score, HumNum starttime) {
	NODE_VERIFY(score, starttime)
	MAKE_CHILD_LIST(children, score);

	for (xml_node item : children) {
		string nodename = item.name();
		if (nodename == "scoreDef") {
			parseScoreDef(item, starttime);
		} else if (nodename == "section") {
			starttime = parseSection(item, starttime);
		} else {
			cerr << DKHTP << score.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::parseScoreDef -- Process a <scoreDef> element in an MEI file.
//

void Tool_mei2hum::parseScoreDef(xml_node scoreDef, HumNum starttime) {
	NODE_VERIFY(scoreDef, )
	MAKE_CHILD_LIST(children, scoreDef);

	if (m_scoreDef.global.timestamp == starttime QUARTER_CONVERT) {
		m_scoreDef.clear();
	}
	m_scoreDef.global.timestamp = starttime QUARTER_CONVERT;

	fillWithStaffDefAttributes(m_scoreDef.global, scoreDef);

	for (xml_node item : children) {
		string nodename = item.name();
		if (nodename == "staffGrp") {
		    processStaffGrp(item, starttime);
		} else if (nodename == "staffDef") {
		    processStaffDef(item, starttime);
		} else {
			cerr << DKHTP << scoreDef.name() << "/" << nodename << CURRLOC << endl;
		}
	}

}



//////////////////////////////
//
// Tool_mei2hum::processStaffGrp -- Process a <staffGrp> element in an MEI file.
//

void Tool_mei2hum::processStaffGrp(xml_node staffGrp, HumNum starttime) {
	NODE_VERIFY(staffGrp, )
	MAKE_CHILD_LIST(children, staffGrp);

	for (xml_node item : children) {
		string nodename = item.name();
		if (nodename == "staffGrp") {
		    processStaffGrp(item, starttime);
		} else if (nodename == "staffDef") {
		    processStaffDef(item, starttime);
		} else {
			cerr << DKHTP << staffGrp.name() << "/" << nodename << CURRLOC << endl;
		}
	}

}



//////////////////////////////
//
// Tool_mei2hum::processStaffDef -- Process a <staffDef> element in an MEI file.
//

void Tool_mei2hum::processStaffDef(xml_node staffDef, HumNum starttime) {
	NODE_VERIFY(staffDef, )

	string staffnum = staffDef.attribute("n").value();
	if (staffnum.empty()) {
		// no staffDef@n so cannot process.
		return;
	}

	int num = stoi(staffnum);
	if (num < 1) {
		// to small
		return;
	}
	if (num > 1000) {
		// too large
		return;
	}

	m_scoreDef.minresize(num);
	m_scoreDef.staves[num-1].clear();
	m_scoreDef.staves[num-1] = m_scoreDef.global;

	fillWithStaffDefAttributes(m_scoreDef.staves[num-1], staffDef);

	// see leaky memory note below for why there are separate
	// variables for clef, keysig, etc.
	string clef = m_scoreDef.staves[num-1].clef;
	string keysig = m_scoreDef.staves[num-1].keysig;
	string timesig = m_scoreDef.staves[num-1].timesig;
	string midibpm = m_scoreDef.staves[num-1].midibpm;


	// Incorporate clef into HumGrid:
	if (clef.empty()) {
		clef = m_scoreDef.global.clef;
	}
	if (!clef.empty()) {
		// Need to store in next measure (fix later).  If there are no measures then
		// create one.
		if (m_outdata.empty()) {
			/* GridMeasure* gm = */ m_outdata.addMeasureToBack();
		}
		// m_scoreDef.staves[num-1].keysig disappears after this line, so some
		// leaky memory is likey to happen here.
		m_outdata.back()->addClefToken(clef, starttime QUARTER_CONVERT, num-1,
				0, 0, m_staffcount);
	}

	// Incorporate key signature into HumGrid:
	if (keysig.empty()) {
		keysig = m_scoreDef.global.keysig;
	}
	if (!keysig.empty()) {
		// Need to store in next measure (fix later).  If there are no measures then
		// create one.
		if (m_outdata.empty()) {
			/* GridMeasure* gm = */ m_outdata.addMeasureToBack();
		}
		m_outdata.back()->addKeySigToken(keysig, starttime QUARTER_CONVERT, num-1,
				0, 0, m_staffcount);
	}

	// Incorporate time signature into HumGrid:
	if (timesig.empty()) {
		timesig = m_scoreDef.global.timesig;
	}
	if (!timesig.empty()) {
		// Need to store in next measure (fix later).  If there are no measures then
		// create one.
		if (m_outdata.empty()) {
			/* GridMeasure* gm = */ m_outdata.addMeasureToBack();
		}
		m_outdata.back()->addTimeSigToken(timesig, starttime QUARTER_CONVERT, num-1,
				0, 0, m_staffcount);
	}

	// Incorporate tempo into HumGrid:
	if (midibpm.empty()) {
		midibpm = m_scoreDef.global.midibpm;
	}
	if (!midibpm.empty()) {
		// Need to store in next measure (fix later).  If there are no measures then
		// create one.
		if (m_outdata.empty()) {
			/* GridMeasure* gm = */ m_outdata.addMeasureToBack();
		}
		m_outdata.back()->addTempoToken(midibpm, starttime QUARTER_CONVERT, num-1,
				0, 0, m_staffcount);
	}

}



//////////////////////////////
//
// Tool_mei2hum::fillWithStaffDefAttributes --
//

void Tool_mei2hum::fillWithStaffDefAttributes(mei_staffDef& staffinfo, xml_node element) {

	string clefshape;
	string clefline;
	string clefdis;
	string clefdisplace;
	string metercount;
	string meterunit;
	string staffnum;
	string keysig;
	string midibpm;

	string nodename = element.name();

	int nnum = 0;  // For staffnumber of element is staffDef.

	for (auto atti = element.attributes_begin(); atti != element.attributes_end();
				atti++) {
		string attname = atti->name();
		if (attname == "clef.shape") {
			clefshape = atti->value();
		} else if (attname == "clef.line") {
			clefline = atti->value();
		} else if (attname == "clef.dis") {
			clefdis = atti->value();
		} else if (attname == "clef.displace") {
			clefdisplace = atti->value();
		} else if (attname == "meter.count") {
			metercount = atti->value();
		} else if (attname == "meter.unit") {
			meterunit = atti->value();
		} else if (attname == "key.sig") {
			keysig = atti->value();
		} else if (attname == "midi.bpm") {
			midibpm = atti->value();
		} else if (attname == "n") {
			nnum = atoi(atti->value());
		}
	}
	if (nnum < 1) {
		nnum = 1;
	}

	if ((!clefshape.empty()) && (!clefline.empty())) {
		staffinfo.clef = makeHumdrumClef(clefshape, clefline, clefdis, clefdisplace);
	}
	if ((!metercount.empty()) && (!meterunit.empty())) {
		HumNum meterduration = stoi(metercount) * 4 / stoi(meterunit);
		if (nodename == "scoreDef") {
			for (int i=0; i<m_measureDuration.size(); i++) {
				m_measureDuration.at(i) = meterduration;
				m_currentMeterUnit.at(i) = stoi(meterunit);
			}
		} else if (nodename == "staffDef") {
			if (nnum > 0) {
				m_measureDuration.at(nnum-1) = meterduration;
				m_currentMeterUnit.at(nnum-1) = stoi(meterunit);
			}
		} else {
			cerr << DKHTP << element.name() << "@meter.count/@meter.unit" << CURRLOC << endl;
		}

		staffinfo.timesig = "*M" + metercount + "/" + meterunit;
	}
	if (!keysig.empty()) {
		int count = stoi(keysig);
		int accid = 0;
		if (keysig.find("s") != string::npos) {
			accid = +1;
		} else if (keysig.find("f") != string::npos) {
			accid = -1;
		}

		if (accid > 0) {
			switch (count) {
				case 1: staffinfo.keysig = "*k[f#]";             break;
				case 2: staffinfo.keysig = "*k[f#c#]";           break;
				case 3: staffinfo.keysig = "*k[f#c#g#]";         break;
				case 4: staffinfo.keysig = "*k[f#c#g#d#]";       break;
				case 5: staffinfo.keysig = "*k[f#c#g#d#a#]";     break;
				case 6: staffinfo.keysig = "*k[f#c#g#d#a#e#]";   break;
				case 7: staffinfo.keysig = "*k[f#c#g#d#a#e#b#]"; break;
			}
		} else if (accid < 0) {
			switch (count) {
				case 1: staffinfo.keysig = "*k[b-]";             break;
				case 2: staffinfo.keysig = "*k[b-e-]";           break;
				case 3: staffinfo.keysig = "*k[b-e-a-]";         break;
				case 4: staffinfo.keysig = "*k[b-e-a-d-]";       break;
				case 5: staffinfo.keysig = "*k[b-e-a-d-g-]";     break;
				case 6: staffinfo.keysig = "*k[b-e-a-d-g-c-]";   break;
				case 7: staffinfo.keysig = "*k[b-e-a-d-g-c-f-]"; break;
			}
		}
	}
	if (!midibpm.empty()) {
		staffinfo.midibpm = "*MM" + midibpm;
	}

}



//////////////////////////////
//
// Tool_mei2hum::parseSection -- Process a <section> element in an MEI file.
//

HumNum Tool_mei2hum::parseSection(xml_node section, HumNum starttime) {
	NODE_VERIFY(section, starttime);
	MAKE_CHILD_LIST(children, section);

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "section") {
			starttime = parseSection(children[i], starttime);
		} else if (nodename == "measure") {
			starttime = parseMeasure(children[i], starttime);
		} else if (nodename == "app") {
			starttime = parseApp(children[i], starttime);
		} else if (nodename == "sb") {
			parseSb(children[i], starttime);
		} else {
			cerr << DKHTP << section.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	return starttime;
}


//////////////////////////////
//
// Tool_mei2hum::parseSb -- System (line) break in the music.
//

void Tool_mei2hum::parseSb(xml_node sb, HumNum starttime) {
	NODE_VERIFY(sb, );
	MAKE_CHILD_LIST(children, sb);

	// There should be no children of sb (at least any that are currently known)
	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		cerr << DKHTP << sb.name() << "/" << nodename << CURRLOC << endl;
	}

	m_outdata.back()->appendGlobalLayout("!!LO:LB", starttime QUARTER_CONVERT);
}



//////////////////////////////
//
// Tool_mei2hum::parseApp --
//

HumNum Tool_mei2hum::parseApp(xml_node app, HumNum starttime) {
	NODE_VERIFY(app, starttime);
	MAKE_CHILD_LIST(children, app);

	if (children.empty()) {
		return starttime;
	}

	xml_node target = children[0];
	if (!m_appLabel.empty()) {
		string testlabel;
		for (int i=0; i<(int)children.size(); i++) {
			testlabel = children[i].attribute("label").value();
			if (testlabel == m_appLabel) {
				target = children[i];
				break;
			}
		}
	}

	// Only following the first element in app list for now.
	string nodename = target.name();
	if (nodename == "lem") {
		starttime = parseLem(target, starttime);
	} else if (nodename == "rdg") {
		starttime = parseRdg(target, starttime);
	} else {
		cerr << DKHTP << app.name() << "/" << nodename << CURRLOC << endl;
	}

	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::parseLem -- Process a <lem> element in an MEI file.
//

HumNum Tool_mei2hum::parseLem(xml_node lem, HumNum starttime) {
	NODE_VERIFY(lem, starttime);
	MAKE_CHILD_LIST(children, lem);

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "section") {
			starttime = parseSection(children[i], starttime);
		} else if (nodename == "measure") {
			starttime = parseMeasure(children[i], starttime);
		} else {
			cerr << DKHTP << lem.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::parseRdg -- Process a <rdg> element in an MEI file.
//

HumNum Tool_mei2hum::parseRdg(xml_node rdg, HumNum starttime) {
	NODE_VERIFY(rdg, starttime);
	MAKE_CHILD_LIST(children, rdg);

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "section") {
			starttime = parseSection(children[i], starttime);
		} else if (nodename == "measure") {
			starttime = parseMeasure(children[i], starttime);
		} else {
			cerr << DKHTP << rdg.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::parseMeasure -- Process an MEI measure element.
//

HumNum Tool_mei2hum::parseMeasure(xml_node measure, HumNum starttime) {
	NODE_VERIFY(measure, starttime);
	MAKE_CHILD_LIST(children, measure);

	string n = measure.attribute("n").value();
	int nnum = 0;
	if (n.empty()) {
		// cerr << "Warning: no measure number on measure element" << endl;
	} else {
		nnum = stoi(n);
	}
	if (nnum < 0) {
		cerr << "Error: invalid measure number: " << nnum << endl;
	}
	m_currentMeasure = nnum;

	GridMeasure* gm = m_outdata.addMeasureToBack();
	gm->setTimestamp(starttime QUARTER_CONVERT);

	vector<HumNum> durations;
	
	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "staff") {
			durations.push_back(parseStaff(children[i], starttime) - starttime);
		} else if (nodename == "fermata") {
			// handled in process processNodeStartLinks()
		} else if (nodename == "slur") {
			// handled in process processNode(Start|Stop)Links()
		} else if (nodename == "tie") {
			// handled in process processNode(Start|Stop)Links()
		} else if (nodename == "arpeg") {
			// handled in process processNode(Start|Stop)Links()
		} else if (nodename == "dynam") {
			parseDynam(children[i], starttime);
		} else if (nodename == "dir") {
			parseDir(children[i], starttime);
		} else {
			cerr << DKHTP << measure.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	// Check that the duration of each layer is the same here.

	if (durations.empty()) {
		return starttime;
	}

	bool allequal = true;
	for (int i=1; i<durations.size(); i++) {
		if (durations[i] != durations[0]) {
			allequal = false;
			break;
		}
	}

	HumNum measuredur = durations[0];
	HumNum targetDur = m_measureDuration.at(0);
	if (!allequal) {
		measuredur = targetDur;
		for (int i=0; i<durations.size(); i++) {
			if (durations[i] == targetDur) {
				continue;
			}
			if (durations[i] < targetDur) {
				std::ostringstream message;
				message << "Error: measure " << m_currentMeasure;
				message << " staff " << i+1 << " is underfilled: ";
				message << (durations[i] QUARTER_CONVERT).getFloat();
				message << " quarter notes instead of ";
				message << (targetDur QUARTER_CONVERT).getFloat() << ".";
				cerr << message.str() << endl;
				m_outdata.back()->addGlobalComment("!!" + message.str(), starttime QUARTER_CONVERT);
			} else if (durations[i] > targetDur) {
				std::ostringstream message;
				message << "Error: measure " << m_currentMeasure;
				message << " staff " << i+1 << " is overfilled: ";
				message << (durations[i] QUARTER_CONVERT).getFloat();
				message << " quarter notes instead of ";
				message << (targetDur QUARTER_CONVERT).getFloat() << ".";
				cerr << message.str() << endl;
				m_outdata.back()->addGlobalComment("!!" + message.str(), starttime QUARTER_CONVERT);
			}
		}
	}

	gm->setTimestamp(starttime QUARTER_CONVERT);
	gm->setDuration(measuredur QUARTER_CONVERT);
	gm->setTimeSigDur(m_measureDuration[0]);

	string rightstyle = measure.attribute("right").value();
	if (rightstyle == "") {
		// do nothing
	} else if (rightstyle == "end") {
		gm->setFinalBarlineStyle();
	} else if (rightstyle == "rptend") {
		gm->setRepeatBackwardStyle();
	}


	return starttime + measuredur;
}



//////////////////////////////
//
// Tool_mei2hum::parseStaff -- Process an MEI staff element.
//

HumNum Tool_mei2hum::parseStaff(xml_node staff, HumNum starttime) {
	NODE_VERIFY(staff, starttime);
	MAKE_CHILD_LIST(children, staff);

	string n = staff.attribute("n").value();
	int nnum = 0;
	if (n.empty()) {
		cerr << "Warning: no staff number on staff element" << endl;
	} else {
		nnum = stoi(n);
	}
	if (nnum < 1) {
		cerr << "Error: invalid staff number: " << nnum << endl;
	}
	m_currentStaff = nnum;

	if (m_maxStaffInFile < m_currentStaff) {
		m_maxStaffInFile = m_currentStaff;
	}

	vector<HumNum> durations;

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "layer") {
			durations.push_back(parseLayer(children[i], starttime) - starttime);
		} else {
			cerr << DKHTP << staff.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	// Check that the duration of each layer is the same here.

	if (durations.empty()) {
		return starttime;
	}

	bool allequal = true;
	for (int i=1; i<durations.size(); i++) {
		if (durations[i] != durations[0]) {
			allequal = false;
			break;
		}
	}

	HumNum staffdur = durations[0];

	HumNum targetDur = m_measureDuration.at(m_currentStaff-1) / 4;;
	if (!allequal) {
		staffdur = targetDur;
		for (int i=0; i<durations.size(); i++) {
			if (durations[i] QUARTER_CONVERT == targetDur) {
				continue;
			}
			if (durations[i] < targetDur) {
				std::ostringstream message;
				message << "Error: measure " << m_currentMeasure;
				message << " staff " << m_currentStaff;
				message << " layer " << i+1;
				message << " is underfilled: ";
				message << (durations[i] QUARTER_CONVERT).getFloat();
				message << " quarter notes instead of ";
				message << (targetDur QUARTER_CONVERT).getFloat() << ".";
				cerr << message.str() << endl;
				m_outdata.back()->addGlobalComment("!!" + message.str(), starttime QUARTER_CONVERT);
			} else if (durations[i] > targetDur) {
				std::ostringstream message;
				message << "Error: measure " << m_currentMeasure;
				message << " staff " << m_currentStaff;
				message << " layer " << i+1;
				message << " is overfilled: ";
				message << (durations[i] QUARTER_CONVERT).getFloat();
				message << " quarter notes instead of ";
				message << (targetDur QUARTER_CONVERT).getFloat() << ".";
				cerr << message.str() << endl;
				m_outdata.back()->addGlobalComment("!!" + message.str(), starttime QUARTER_CONVERT);
			}
		}
	}

	m_currentStaff = 0;

	return starttime + staffdur;
}



//////////////////////////////
//
// Tool_mei2hum::parseLayer --
//

HumNum Tool_mei2hum::parseLayer(xml_node layer, HumNum starttime) {
	NODE_VERIFY(layer, starttime)
	MAKE_CHILD_LIST(children, layer);


	string n = layer.attribute("n").value();
	int nnum = 0;
	if (n.empty()) {
		cerr << "Warning: no layer number on layer element" << endl;
	} else {
		nnum = stoi(n);
	}
	if (nnum < 1) {
		cerr << "Error: invalid layer number: " << nnum << endl;
	}
	m_currentLayer = nnum;

	HumNum  starting = starttime;
	string dummy;

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "note") {
			starttime = parseNote(children[i], xml_node(NULL), dummy, starttime);
		} else if (nodename == "chord") {
			starttime = parseChord(children[i], starttime);
		} else if (nodename == "rest") {
			starttime = parseRest(children[i], starttime);
		} else if (nodename == "space") {
			starttime = parseRest(children[i], starttime);
		} else if (nodename == "mRest") {
			starttime = parseMRest(children[i], starttime);
		} else if (nodename == "beam") {
			starttime = parseBeam(children[i], starttime);
		} else if (nodename == "tuplet") {
			starttime = parseTuplet(children[i], starttime);
		} else if (nodename == "clef") {
			parseClef(children[i], starttime);
		} else {
			cerr << DKHTP << layer.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	m_currentLayer = 0;
	return starttime;
// ggg
}


//////////////////////////////
//
// Tool_mei2hum::parseTuplet --
//

HumNum Tool_mei2hum::parseTuplet(xml_node tuplet, HumNum starttime) {
	NODE_VERIFY(tuplet, starttime)
	MAKE_CHILD_LIST(children, tuplet);

	string num = tuplet.attribute("num").value();
	string numbase = tuplet.attribute("numbase").value();

	HumNum newfactor = 1;

	if (numbase == "") {
		cerr << "Warning: tuplet@numbase is empty" << endl;
	} else {
		newfactor = stoi(numbase);
	}

	if (num == "") {
		cerr << "Warning: tuplet@num is empty" << endl;
	} else {
		newfactor /= stoi(num);
	}

	m_tupletfactor *= newfactor;

	string stored_beamPostfix;
	if (m_beamPostfix != "") {
		stored_beamPostfix = m_beamPostfix;
		m_beamPostfix.clear();
	}

	xml_node lastnoterestchord;
	for (int i=(int)children.size() - 1; i>=0; i--) {
		string nodename = children[i].name();
		if (nodename == "note") {
			lastnoterestchord = children[i];
			break;
		} else if (nodename == "rest") {
			lastnoterestchord = children[i];
			break;
		} else if (nodename == "chord") {
			lastnoterestchord = children[i];
			break;
		}
	}

	string dummy;
	for (int i=0; i<(int)children.size(); i++) {
		if (children[i] == lastnoterestchord) {
			m_beamPostfix = stored_beamPostfix;
		}
		string nodename = children[i].name();
		if (nodename == "note") {
			starttime = parseNote(children[i], xml_node(NULL), dummy, starttime);
		} else if (nodename == "rest") {
			starttime = parseRest(children[i], starttime);
		} else if (nodename == "chord") {
			starttime = parseChord(children[i], starttime);
		} else if (nodename == "beam") {
			starttime = parseBeam(children[i], starttime);
		} else {
			cerr << DKHTP << tuplet.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	m_tupletfactor /= newfactor;

	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::parseBeam --
//

HumNum Tool_mei2hum::parseBeam(xml_node beam, HumNum starttime) {
	NODE_VERIFY(beam, starttime)
	MAKE_CHILD_LIST(children, beam);

	bool isvalid = beamIsValid(children);

	if (isvalid) {
		m_beamPrefix = "L";
	}

	xml_node lastnoterestchord;
	for (int i=(int)children.size()-1; i>=0; i--) {
		string nodename = children[i].name();
		if (nodename == "note") {
			lastnoterestchord = children[i];
			break;
		} else if (nodename == "rest") {
			lastnoterestchord = children[i];
			break;
		} else if (nodename == "chord") {
			lastnoterestchord = children[i];
			break;
		} else if (nodename == "tuplet") {
			lastnoterestchord = children[i];
			break;
		}
	}

	string dummy;
	for (int i=0; i<(int)children.size(); i++) {
		if (children[i] == lastnoterestchord) {
			if (isvalid) {
				m_beamPostfix = "J";
			}
		}
		string nodename = children[i].name();
		if (nodename == "note") {
			starttime = parseNote(children[i], xml_node(NULL), dummy, starttime);
		} else if (nodename == "rest") {
			starttime = parseRest(children[i], starttime);
		} else if (nodename == "chord") {
			starttime = parseChord(children[i], starttime);
		} else if (nodename == "tuplet") {
			starttime = parseTuplet(children[i], starttime);
		} else {
			cerr << DKHTP << beam.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::beamIsValid --
//

bool Tool_mei2hum::beamIsValid(vector<xml_node>& beamlist) {
	for (int i=0; i<(int)beamlist.size(); i++) {
		string nodename = beamlist[i].name();
		if (nodename != "note") {
			continue;
		}
		string grace = beamlist[i].attribute("grace").value();
		if (!grace.empty()) {
			continue;
		}
		string dur = beamlist[i].attribute("dur").value();
		if (dur.empty()) {
			// strange, but skip
			continue;
		}
		if (isdigit(dur[0])) {
			if (stoi(dur) <= 4) {
				return false;
			}
		} else {
			// "breve", "long", "maxima", junk, etc.
			return false;
		}
	}

	return true;
}



//////////////////////////////
//
// Tool_mei2hum::parseNote --
//

HumNum Tool_mei2hum::parseNote(xml_node note, xml_node chord, string& output, HumNum starttime) {
	NODE_VERIFY(note, starttime)
	MAKE_CHILD_LIST(children, note);

	HumNum duration;
	int dotcount;

	string grace = note.attribute("grace").value();
	if (!grace.empty()) {
		// grace note so currently ignore.
		cerr << "Warning: currently ignoring grace notes." << endl;
 		return starttime;
	}

	if (chord) {
		duration = getDuration(chord);
		dotcount = getDotCount(chord);
		// maybe allow different durations on notes in a chord if the first one is the
		// same as the duration of the chord.
	} else {
		duration = getDuration(note);
		dotcount = getDotCount(note);
	}

	string recip = getHumdrumRecip(duration, dotcount);
	string humpitch = getHumdrumPitch(note);
	string editorial = getEditorialAccidental(children);
	string cautionary = getCautionaryAccidental(children);
	if (!editorial.empty()) {
		humpitch += editorial;
	}
	if (!cautionary.empty()) {
		humpitch += cautionary;
	}

	string articulations = getNoteArticulations(note, chord);

	string stemdir = note.attribute("stem.dir").value();
	if (stemdir == "up") {
		stemdir = "/";
	} else if (stemdir == "down") {
		stemdir = "\\";
	} else {
		stemdir = "";
	}

	string tok = recip + humpitch + articulations + stemdir + m_beamPrefix + m_beamPostfix;
	m_beamPrefix.clear();
	m_beamPostfix.clear();

	m_fermata = false;
	processLinkedNodes(tok, note);
	if (!m_fermata) {
		processFermataAttribute(tok, note);
	}

	GridSlice* dataslice = NULL;

	if (!chord) {
		dataslice = m_outdata.back()->addDataToken(tok, starttime QUARTER_CONVERT,
				m_currentStaff-1, 0, m_currentLayer-1, m_staffcount);
	} else {
		output += tok;
	}

	bool hasverse = false;

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if ((nodename == "verse") && (dataslice != NULL)) {
			hasverse = true;
			parseVerse(children[i], dataslice->at(m_currentStaff-1)->at(0));
		} else if (nodename == "artic") {
			// handled elsewhere: don't do anything
		} else if (nodename == "accid") {
			// handled elsewhere: don't do anything
		} else {
			cerr << DKHTP << note.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	if (!hasverse) {
		string attsyl = note.attribute("syl").value();
		if (!attsyl.empty()) {
			parseSylAttribute(attsyl, dataslice->at(m_currentStaff-1)->at(0));
		}
	}

	return starttime + duration;
}



//////////////////////////////
//
// Tool_mei2hum::parseSylAttribute -- 
//

void Tool_mei2hum::parseSylAttribute(const string& attsyl, GridStaff* staff) {
	vector<string> pieces(1);
	int length = (int)attsyl.size();
	if (length == 0) {
		return;
	}
	if (length == 1) {
		pieces[0] += attsyl;
	} else {
		for (int i=0; i<length-2; i++) {
			if ((attsyl[i] == '/') && (attsyl[i+1] == '/')) {
				pieces.emplace_back("");
				i++;
			} else {
				pieces.back() += attsyl[i];
			}
		}
		if ((attsyl[length-1] != '/') && (attsyl[length-2] != '/')) {
			pieces.back() += attsyl[length-2];
			pieces.back() += attsyl.back();
		}
	}

	if ((pieces.size() == 1) && (pieces[0].empty())) {
		return;
	}

	for (int i=0; i<(int)pieces.size(); i++) {
		pieces[i] = cleanVerseText(pieces[i]);
	}

	for (int i=0; i<(int)pieces.size(); i++) {
		if (pieces[i].empty()) {
			continue;
		}
		staff->setVerse(i, pieces[i]);
		reportVerseNumber(i+1, m_currentStaff-1);
	}
}



//////////////////////////////
//
// Tool_mei2hum::getEditorialAccidental --
//

string Tool_mei2hum::getEditorialAccidental(vector<xml_node>& children) {
	string output;
	if (children.empty()) {
		return output;
	}

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename != "accid") {
			continue;
		}
		string function = children[i].attribute("func").value();
		if (function != "edit") {
			continue;
		}
		string accid = children[i].attribute("accid").value();
		if (accid.empty()) {
			continue;
		}
		if (accid == "n") {
			output = "ni";
		} else if (accid == "s") {
			output = "#i";
		} else if (accid == "f") {
			output = "-i";
		} else if (accid == "ff") {
			output = "--i";
		} else if (accid == "ss") {
			output = "##i";
		} else if (accid == "x") {
			output = "##i";
		} else if (accid == "nf") {
			output = "-i";
		} else if (accid == "ns") {
			output = "#i";
		} else {
			cerr << "Don't know how to interpret " << accid << " accidental" << endl;
		}
		m_editorialAccidentalQ = true;
		break;
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::getCautionaryAccidental --
// Such as:
//     <accid accid="n" func="caution" />
//

string Tool_mei2hum::getCautionaryAccidental(vector<xml_node>& children) {
	string output;
	if (children.empty()) {
		return output;
	}

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename != "accid") {
			continue;
		}
		string function = children[i].attribute("func").value();
		if (function != "caution") {
			continue;
		}
		string accid = children[i].attribute("accid").value();
		if (accid.empty()) {
			continue;
		}
		if (accid == "n") {
			output = "n";
		} else if (accid == "s") {
			output = "#X";
		} else if (accid == "f") {
			output = "-X";
		} else if (accid == "ff") {
			output = "--X";
		} else if (accid == "ss") {
			output = "##X";
		} else if (accid == "x") {
			output = "##X";
		} else if (accid == "nf") {
			output = "-X";
		} else if (accid == "ns") {
			output = "#X";
		} else {
			cerr << "Don't know how to interpret " << accid << " accidental" << endl;
		}
		break;
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::parseMRest -- Full-measure rest.
//

HumNum Tool_mei2hum::parseMRest(xml_node mrest, HumNum starttime) {
	HumNum duration = m_measureDuration.at(m_currentStaff-1);
	duration /= 4;
	int dotcount = 0;
	string recip = getHumdrumRecip(duration, dotcount);
	if (recip.find('%') != string::npos) {
		string recip2 = getHumdrumRecip(duration, 1);
		if (recip2.find('%') == string::npos) {
			recip = recip2;
			dotcount = 1;
		} else {
			string recip3 = getHumdrumRecip(duration, 2);
			if (recip2.find('%') == string::npos) {
				recip = recip3;
				dotcount = 2;
			}
		}
	}
	string tok = recip + "r";
	// Add fermata on whole-measure rest if needed.	

	// Deal here with calculating number of dots needed for
	// measure duration.

	m_outdata.back()->addDataToken(tok, starttime QUARTER_CONVERT,
			m_currentStaff-1, 0, m_currentLayer-1, m_staffcount);

	return starttime + duration; // convert to whole-note units
}



//////////////////////////////
//
// Tool_mei2hum::parseRest --
//

HumNum Tool_mei2hum::parseRest(xml_node rest, HumNum starttime) {
	if (!rest) {
		return starttime;
	}
	string nodename = rest.name();
	if (!((nodename == "rest") || (nodename == "space"))) {
		return starttime;
	}
	if (nodename == "rest") {
		ELEMENT_DEBUG_STATEMENT(rest)
	} else if (nodename == "space") {
		ELEMENT_DEBUG_STATEMENT(space)
	}

	HumNum duration = getDuration(rest);
	int dotcount = getDotCount(rest);
	string recip = getHumdrumRecip(duration, dotcount);
	string invisible;
	if (nodename == "space") {
		invisible = "yy";
	}

	string output = recip + "r" + invisible + m_beamPrefix + m_beamPostfix;
	m_beamPrefix.clear();
	m_beamPostfix.clear();

	processLinkedNodes(output, rest);
	processFermataAttribute(output, rest);

	m_outdata.back()->addDataToken(output, starttime QUARTER_CONVERT,
			m_currentStaff-1, 0, m_currentLayer-1, m_staffcount);

	return starttime + duration;
}



//////////////////////////////
//
// Tool_mei2hum::getNoteArticulations --
//

string Tool_mei2hum::getNoteArticulations(xml_node note, xml_node chord) {

	string attribute_artic = note.attribute("artic").value();

	vector<xml_node> element_artic;
	for (pugi::xml_node artic : note.children("artic")) {
		element_artic.push_back(artic);
	}

	string chord_attribute_artic;
	vector<xml_node> chord_element_artic;

	if (chord) {
		chord_attribute_artic = chord.attribute("artic").value();
	}
	for (pugi::xml_node artic : chord.children("artic")) {
		chord_element_artic.push_back(artic);
	}

	string output;

	output += getHumdrumArticulation("\\bstacc\\b", "'", attribute_artic, element_artic,
			chord_attribute_artic, chord_element_artic);
	output += getHumdrumArticulation("\\bacc\\b", "^", attribute_artic, element_artic,
			chord_attribute_artic, chord_element_artic);
	output += getHumdrumArticulation("\\bmarc\\b", "^^", attribute_artic, element_artic,
			chord_attribute_artic, chord_element_artic);
	output += getHumdrumArticulation("\\bstacciss\\b", "`", attribute_artic, element_artic,
			chord_attribute_artic, chord_element_artic);
	output += getHumdrumArticulation("\\bten\\b", "~", attribute_artic, element_artic,
			chord_attribute_artic, chord_element_artic);

	return output;
}



///////////////////////////////
//
// Tool_mei2hum::getHumdrumArticulation --
//

string Tool_mei2hum::getHumdrumArticulation(const string& tag, const string& humdrum,
		const string& attribute_artic, vector<xml_node>& element_artic,
		const string& chord_attribute_artic, vector<xml_node>& chord_element_artic) {
	HumRegex hre;
	string output;

	// First check artic attributes:
	if (hre.search(attribute_artic, tag)) {
		output = humdrum;
		return output;
	}

	// If the note is attached to a chord, search the chord:
	if (hre.search(chord_attribute_artic, tag)) {
		output = humdrum;
		return output;
	}

	// Now search for artic elements in the note:
	for (int i=0; i<(int)element_artic.size(); i++) {
		string nodename = element_artic[i].name();
		if (nodename != "artic") {
			continue;
		}
		string artic = element_artic[i].attribute("artic").value();
		if (hre.search(artic, tag)) {
			output = humdrum;
			output += setPlacement(element_artic[i].attribute("place").value());
			return output;
		}
	}

	// And then in the chord:
	for (int i=0; i<(int)chord_element_artic.size(); i++) {
		string nodename = chord_element_artic[i].name();
		if (nodename != "artic") {
			continue;
		}
		string artic = chord_element_artic[i].attribute("artic").value();
		if (hre.search(artic, tag)) {
			output = humdrum;
			output += setPlacement(chord_element_artic[i].attribute("place").value());
			return output;
		}
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::setPlacement --
//

string Tool_mei2hum::setPlacement(const string& placement) {
	if (placement == "above") {
		m_aboveQ = true;
		return ">";
	} else if (placement == "below") {
		m_belowQ = true;
		return "<";
	}
	return "";
}



//////////////////////////////
//
// Tool_mei2hum::processFermataAttribute -- @fermata="(above|below)"
//

void Tool_mei2hum::processFermataAttribute(string& output, xml_node node) {
	string fermata = node.attribute("fermata").value();
	if (fermata.empty()) {
		return;
	}
	if (fermata == "above") {
		output += ';';
	} else if (fermata == "below") {
		output += ";<";
		m_belowQ = true;
	}
}



//////////////////////////////
//
// Tool_mei2hum::processLinkedNodes --
//

void Tool_mei2hum::processLinkedNodes(string& output, xml_node node) {
	string id = node.attribute("xml:id").value();
	if (!id.empty()) {
		auto found = m_startlinks.find(id);
		if (found != m_startlinks.end()) {
			processNodeStartLinks(output, node, (*found).second);
		}
		found = m_stoplinks.find(id);
		if (found != m_stoplinks.end()) {
			processNodeStopLinks(output, node, (*found).second);
		}
	}
}



//////////////////////////////
//
// Tool_mei2hum::getDotCount --
//

int Tool_mei2hum::getDotCount(xml_node node) {
	string dots = node.attribute("dots").value();
	int dotcount = 0;
	if (dots != "") {
		dotcount = stoi(dots);
	}
	return dotcount;
}



//////////////////////////////
//
// Tool_mei2hum::processNodeStartLinks --
//

void Tool_mei2hum::processNodeStartLinks(string& output, xml_node node,
		vector<xml_node>& nodelist) {
	for (int i=0; i<(int)nodelist.size(); i++) {
		string nodename = nodelist[i].name();
		if (nodename == "fermata") {
			m_fermata = true; // used to disable note@fermata duplications
			parseFermata(output, node, nodelist[i]);
		} else if (nodename == "slur") {
			parseSlurStart(output, node, nodelist[i]);
		} else if (nodename == "tie") {
			parseTieStart(output, node, nodelist[i]);
		} else if (nodename == "arpeg") {
			parseArpeg(output, node, nodelist[i]);
		} else {
			cerr << DKHTP << nodename
			     << " element in processNodeStartLinks()" << endl;
		}
	}
}



//////////////////////////////
//
// Tool_mei2hum::parseArpeg -- Only handles single chord arpeggiation for now
//    (ignores @endid).
//

void Tool_mei2hum::parseArpeg(string& output, xml_node node, xml_node arpeg) {
	NODE_VERIFY(arpeg, )

	if (strcmp(arpeg.attribute("endid").value(), "") != 0) {
		cerr << "Warning: multi-note arpeggios are not yet handled in the converter." << endl;
	}

	string nodename = node.name();
	if (nodename == "note") {
		output += ':';
	} else if (nodename == "chord") {
		string temp = output;
		output.clear();
		for (int i=0; i<(int)temp.size(); i++) {
			if (temp[i] == ' ') {
				output += ": ";
			} else {
				output += temp[i];
			}
		}
		output += ':';
	} else {
		cerr << DKHTP << "an arpeggio attached to a "
		     << nodename << " element" << endl;
		return;
	}

}



//////////////////////////////
//
// Tool_mei2hum::processNodeStopLinks --
//

void Tool_mei2hum::processNodeStopLinks(string& output, xml_node node,
		vector<xml_node>& nodelist) {
	for (int i=0; i<(int)nodelist.size(); i++) {
		string nodename = nodelist[i].name();
		if (nodename == "slur") {
			parseSlurStop(output, node, nodelist[i]);
		} else if (nodename == "tie") {
			parseTieStop(output, node, nodelist[i]);
		} else {
			cerr << DKHTP << nodename
			     << " element in processNodeStopLinks()" << endl;
		}
	}
}



//////////////////////////////
//
// Tool_mei2hum::parseSlurStart --
//

void Tool_mei2hum::parseSlurStart(string& output, xml_node node, xml_node slur) {
	NODE_VERIFY(slur, )
	string nodename = node.name();
	if (nodename == "note") {
		output = "(" + setPlacement(slur.attribute("curvedir").value()) + output;
	} else if (nodename == "chord") {
		output = "(" + setPlacement(slur.attribute("curvedir").value()) + output;
	} else {
		cerr << DKHTP << "a slur start attached to a "
		     << nodename << " element" << endl;
		return;
	}

}



//////////////////////////////
//
// Tool_mei2hum::parseSlurStop --
//

void Tool_mei2hum::parseSlurStop(string& output, xml_node node, xml_node slur) {
	NODE_VERIFY(slur, )
	string nodename = node.name();
	if (nodename == "note") {
		output += ")";
	} else if (nodename == "chord") {
		output += ")";
	} else {
		cerr << DKHTP << "a tie end attached to a "
		     << nodename << " element" << endl;
		return;
	}
}



//////////////////////////////
//
// Tool_mei2hum::parseTieStart -- Need to deal with chords later.
//

void Tool_mei2hum::parseTieStart(string& output, xml_node node, xml_node tie) {
	NODE_VERIFY(tie, )

	string id = node.attribute("xml:id").value();
	if (!id.empty()) {
		auto found = m_stoplinks.find(id);
		if (found != m_stoplinks.end()) {
			for (auto item : (*found).second) {
				if (strcmp(tie.attribute("startid").value(), item.attribute("endid").value()) == 0) {
					// deal with tie middles in parseTieStop().
					return;
				}
			}
		}
	}

	string nodename = node.name();
	if (nodename == "note") {
		output = "[" + output;
	} else {
		cerr << DKHTP << "a tie start attached to a "
		     << nodename << " element" << endl;
		return;
	}
}



//////////////////////////////
//
// Tool_mei2hum::parseTieStop -- Need to deal with chords later.
//

void Tool_mei2hum::parseTieStop(string& output, xml_node node, xml_node tie) {
	NODE_VERIFY(tie, )

	string id = node.attribute("xml:id").value();
	if (!id.empty()) {
		auto found = m_startlinks.find(id);
		if (found != m_startlinks.end()) {
			for (auto item : (*found).second) {
				if (strcmp(tie.attribute("endid").value(), item.attribute("startid").value()) == 0) {
					output += "_";
					return;
				}
			}
		}
	}

	string nodename = node.name();
	if (nodename == "note") {
		output += "]";
	} else {
		cerr << DKHTP << "a tie end attached to a "
		     << nodename << " element" << endl;
		return;
	}
}



//////////////////////////////
//
// Tool_mei2hum::parseFermata -- deal with a fermata attached to something.
///     output is a Humdrum token string (maybe have it as a HumdrumToken object).
//

void Tool_mei2hum::parseFermata(string& output, xml_node node, xml_node fermata) {
	NODE_VERIFY(fermata, )

	string nodename = node.name();
	if (nodename == "note") {
		output += ';';
	} else if (nodename == "chord") {
		output += ';';
	} else if (nodename == "rest") {
		output += ';';
	} else {
		cerr << DKHTP << "a fermata attached to a "
		     << nodename << " element" << endl;
		return;
	}

}



//////////////////////////////
//
// Tool_mei2hum::getHumdrumRecip --
//

string Tool_mei2hum::getHumdrumRecip(HumNum duration, int dotcount) {
	string output;

	if (dotcount > 0) {
		// remove dots from duration
		int top = (1 << (dotcount+1)) - 1;
		int bot = 1 << dotcount;
		HumNum dotfactor(bot, top);
		duration *= dotfactor;
	}

	if (duration.getNumerator() == 1) {
		output = to_string(duration.getDenominator());
	} else if ((duration.getNumerator() == 2) && (duration.getDenominator() == 1)) {
		// breve symbol:
		output = "0";
	} else if ((duration.getNumerator() == 4) && (duration.getDenominator() == 1)) {
		// long symbol:
		output = "00";
	} else if ((duration.getNumerator() == 8) && (duration.getDenominator() == 1)) {
		// maxima symbol:
		output = "000";
	} else {
		output = to_string(duration.getDenominator());
		output += "%";
		output += to_string(duration.getNumerator());
	}

	for (int i=0; i<dotcount; i++) {
		output += '.';
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::getHumdrumPitch --
//

string Tool_mei2hum::getHumdrumPitch(xml_node note) {
	string pname = note.attribute("pname").value();
	string accidvis = note.attribute("accid").value();
	string accidges = note.attribute("accid.ges").value();

	int octnum = 4;
	string oct = note.attribute("oct").value();
	if (oct == "") {
		cerr << "Empty octave" << endl;
	} else if (isdigit(oct[0])) {
		octnum = stoi(oct);
	} else {
		cerr << "Unknown octave value: " << oct << endl;
	}

	if (pname == "") {
		cerr << "Empty pname" << endl;
		return "x";
	}

	string output;
	if (octnum < 4) {
		char val = toupper(pname[0]);
		int count = 4 - octnum;
		for (int i=0; i<count; i++) {
			output += val;
		}
	} else {
		char val = pname[0];
		int count = octnum - 3;
		for (int i=0; i<count; i++) {
			output += val;
		}
	}

	if (accidges != "") {
		if (accidges == "n") {
			// do nothing;
		} else if (accidges == "f") {
			output += "-";
		} else if (accidges == "s") {
			output += "#";
		} else if (accidges == "ff") {
			output += "--";
		} else if (accidges == "ss") {
			output += "##";
		} else if (accidges == "x") {
			output += "##";
		}
	} else if (accidvis != "") {
		if (accidvis == "n") {
			// do nothing;
		} else if (accidvis == "f") {
			output += "-";
		} else if (accidvis == "s") {
			output += "#";
		} else if (accidvis == "ff") {
			output += "--";
		} else if (accidvis == "ss") {
			output += "##";
		} else if (accidvis == "x") {
			output += "##";
		}
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::getDuration --
//

HumNum Tool_mei2hum::getDuration(xml_node element) {
	string dur = element.attribute("dur").value();
	if (dur == "") {
		return 0;
	}

	HumNum output;
	if (dur == "breve") {
		output = 2;
	} else if (dur == "long") {
		output = 4;
	} else if (dur == "maxima") {
		output = 8;
	} else if (isdigit(dur[0])) {
		output = 1;
		output /= stoi(dur);
	} else {
		cerr << "Unknown " << element.name() << "@dur: " << dur << endl;
		return 0;
	}

	if (output == 0) {
		cerr << "Error: zero duration for note" << endl;
	}

	int dotcount;
	string dots = element.attribute("dots").value();
	if (dots == "") {
		dotcount = 0;
	} else if (isdigit(dots[0])) {
		dotcount = stoi(dots);
	} else {
		cerr << "Unknown " << element.name() << "@dotcount: " << dur << endl;
		return 0;
	}

	if (dotcount > 0) {
		int top = (1 << (dotcount+1)) - 1;
		int bot = 1 << dotcount;
		HumNum dotfactor(top, bot);
		output *= dotfactor;
	}

	// add a correction for the tuplet factor which is currently active.
	if (m_tupletfactor != 1) {
		output *= m_tupletfactor;
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::parseVerse -- 
//

void Tool_mei2hum::parseVerse(xml_node verse, GridStaff* staff) {
	NODE_VERIFY(verse, )
	MAKE_CHILD_LIST(children, verse);

	string n = verse.attribute("n").value();
	int nnum = 1;
	if (n.empty()) {
		cerr << "Warning: no layer number on layer element" << endl;
	} else {
		nnum = stoi(n);
	}
	if (nnum < 1) {
		cerr << "Warning: invalid layer number: " << nnum << endl;
		cerr << "Setting it to 1." << endl;
		nnum = 1;
	}

	string versetext;
	int sylcount = 0;

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "syl") {
			if (sylcount > 0) {
				versetext += " ";
			}
			sylcount++;
			versetext += parseSyl(children[i]);
		} else {
			cerr << DKHTP << verse.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	if (versetext == "") {
		// nothing to store
		return;
	}

	staff->setVerse(nnum-1, versetext);
	reportVerseNumber(nnum, m_currentStaff-1);

	return;
}



//////////////////////////////
//
// Tool_mei2hum::reportVerseNumber --
//

void Tool_mei2hum::reportVerseNumber(int pmax, int staffindex) {
	if (staffindex < 0) {
		return;
	}
	if (staffindex >= (int)m_maxverse.size()) {
		return;
	}
	if (m_maxverse.at(staffindex) < pmax) {
		m_maxverse[staffindex] = pmax;
	}
}



//////////////////////////////
//
// Tool_mei2hum::parseSyl --
//

string Tool_mei2hum::parseSyl(xml_node syl) {
	NODE_VERIFY(syl, "")
	MAKE_CHILD_LIST(children, syl);

	string text = syl.child_value();
	for (int i=0; i<(int)text.size(); i++) {
		if (text[i] == '_') {
			text[i] = ' ';
		}
	}

	string wordpos = syl.attribute("wordpos").value();
	if (wordpos == "i") {
		text = text + "-";
	} else if (wordpos == "m") {
		text = "-" + text + "-";
	} else if (wordpos == "t") {
		text = "-" + text;
	}

	return text;
}



//////////////////////////////
//
// Tool_mei2hum::parseClef --
//
//

void Tool_mei2hum::parseClef(xml_node clef, HumNum starttime) {
	NODE_VERIFY(clef, )

	string shape = clef.attribute("shape").value();
	string line = clef.attribute("line").value();
	string clefdis = clef.attribute("clef.dis").value();
	string clefdisplace = clef.attribute("clef.dis.place").value();

	string tok = makeHumdrumClef(shape, line, clefdis, clefdisplace);

	m_outdata.back()->addClefToken(tok, starttime QUARTER_CONVERT,
			m_currentStaff-1, 0, 0, m_staffcount);

}



//////////////////////////////
//
// Tool_mei2hum::makeHumdrumClef --
//
// Example:
//     <clef shape="G" line="2" clef.dis="8" clef.dis.place="below" />
//

string Tool_mei2hum::makeHumdrumClef(const string& shape,
		const string& line, const string& clefdis, const string& clefdisplace) {
	string output = "*clef" + shape;
	if (!clefdis.empty()) {
		int number = stoi(clefdis);
		int count = 0;
		if (number == 8) {
			count = 1;
		} else if (number == 15) {
			count = 2;
		}
		if (clefdisplace != "above") {
			count = -count;
		}
		switch (count) {
			case 1: output += "^"; break;
			case 2: output += "^^"; break;
			case -1: output += "v"; break;
			case -2: output += "vv"; break;
		}
	}
	output += line;
	return output;
}



//////////////////////////////
//
// Tool_mei2hum::parseChord --
//

HumNum Tool_mei2hum::parseChord(xml_node chord, HumNum starttime) {
	NODE_VERIFY(chord, starttime)
	MAKE_CHILD_LIST(children, chord);

	string tok;
	int counter = 0;
	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "note") {
			counter++;
			if (counter > 1) {
				tok += " ";
			}
			parseNote(children[i], chord, tok, starttime);
		} else if (nodename == "artic") {
			// This is handled within parseNote();
		} else {
			cerr << DKHTP << chord.name() << "/" << nodename << CURRLOC << endl;
		}
	}

	m_fermata = false;
	processLinkedNodes(tok, chord);
	if (!m_fermata) {
		processFermataAttribute(tok, chord);
	}

	m_outdata.back()->addDataToken(tok, starttime QUARTER_CONVERT, m_currentStaff-1,
		0, m_currentLayer-1, m_staffcount);

	HumNum duration = getDuration(chord);
	return starttime + duration;
}



//////////////////////////////
//
// Tool_mei2hum::getChildrenVector -- Return a list of all children elements
//   of a given element.  Pugixml does not allow random access, but storing
//   them in a vector allows that possibility.
//

void Tool_mei2hum::getChildrenVector(vector<xml_node>& children,
		xml_node parent) {
	children.clear();
	for (xml_node child : parent.children()) {
		children.push_back(child);
	}
}



//////////////////////////////
//
// Tool_mei2hum::initialize -- Setup for the tool, mostly parsing command-line
//   (input) options.
//

void Tool_mei2hum::initialize(void) {
	m_recipQ   = getBoolean("recip");
	m_stemsQ   = getBoolean("stems");
	m_appLabel = getString("app-label");
}



//////////////////////////////
//
// Tool_mei2hum::buildIdLinkMap -- Build table of startid and endid links between elemements.
//
// Reference: https://pugixml.org/docs/samples/traverse_walker.cpp
//

void Tool_mei2hum::buildIdLinkMap(xml_document& doc) {
	class linkmap_walker : public pugi::xml_tree_walker {
		public:
			virtual bool for_each(pugi::xml_node& node) {
				xml_attribute startid = node.attribute("startid");
				xml_attribute endid = node.attribute("endid");
				if (startid) {

					string value = startid.value();
					if (!value.empty()) {
						if (value[0] == '#') {
							value = value.substr(1, string::npos);
						}
					}
					if (!value.empty()) {
						(*startlinks)[value].push_back(node);
					}

				}
				if (endid) {

					string value = endid.value();
					if (!value.empty()) {
						if (value[0] == '#') {
							value = value.substr(1, string::npos);
						}
					}
					if (!value.empty()) {
						(*stoplinks)[value].push_back(node);
					}

				}
				return true; // continue traversal
			}

			map<string, vector<xml_node>>* startlinks = NULL;
			map<string, vector<xml_node>>* stoplinks = NULL;
	};

	m_startlinks.clear();
	m_stoplinks.clear();
	linkmap_walker walker;
	walker.startlinks = &m_startlinks;
	walker.stoplinks = &m_stoplinks;
	doc.traverse(walker);
}



//////////////////////////////
//
// Tool_mei2hum::parseDir -- Meter cannot change in middle of measure.
//     Need to implement @startid version.
//
// Example:
//    <dir xml:id="dir-L408F3" place="below" staff="1" tstamp="2.0">con espressione</dir>
//
// or with normal font specified:
//    <dir xml:id="dir-L7F1" staff="1" tstamp="2.000000">
//       <rend xml:id="rend-0000001696821523" fontstyle="normal">test</rend>
//    </dir>
//
// bold font:
//   <dir xml:id="dir-L25F3" place="above" staff="1" tstamp="3.000000">
//      <rend xml:id="rend-0000001714819172" fontstyle="normal" fontweight="bold">comment</rend>
//   </dir>
//

void Tool_mei2hum::parseDir(xml_node dir, HumNum starttime) {
	NODE_VERIFY(dir, )
	MAKE_CHILD_LIST(children, dir);

	string font = "i";  // italic by default in verovio

	string placement = ""; // a = above, b = below

	string place = dir.attribute("place").value();
	if (place == "above") {
		placement = "a:";
	}
	// Below is the default in Humdrum layout commands.

	string text;

	if (!children.empty()) { // also includes the above text node, but only looking at <rend>.
		int count = 0;
		for (int i=0; i<(int)children.size(); i++) {
			string nodename = children[i].name();
			if (nodename == "rend") {
				if (count) {
					text += " ";
				}
				count++;
				text += children[i].child_value();
				if (strcmp(children[i].attribute("fontstyle").value(), "normal") == 0) {
					font = "";  // normal is default in Humdrum layout
				}
				if (strcmp(children[i].attribute("fontweight").value(), "bold") == 0) {
					font += "B";  // normal is default in Humdrum layout
				}
			} else if (nodename == "") {
				// text node
				if (count) {
					text += " ";
				}
				count++;
				text += children[i].value();
			} else {
				cerr << DKHTP << dir.name() << "/" << nodename << CURRLOC << endl;
			}
		}
	}

	if (text.empty()) {
		return;
	}

	string message = "!LO:TX:";
	message += placement;
	if (!font.empty()) {
		message += font + ":";
	}
	message += "t=" + cleanDirText(text);

	string ts = dir.attribute("tstamp").value();
	if (ts.empty()) {
		cerr << "Error: no timestamp on dir element and can't currently processes with @startid." << endl;
		return;
	}

	int staffnum = dir.attribute("staff").as_int();
	if (staffnum == 0) {
		cerr << "Error: staff number required on dir element in measure " << m_currentMeasure  << endl;
		return;
	}
	double meterunit = m_currentMeterUnit[staffnum - 1];

	double tsd = (stof(ts)-1) * 4.0 / meterunit;
	GridMeasure* gm = m_outdata.back();
	double tsm = gm->getTimestamp().getFloat();
	bool foundslice = false;
	GridSlice* gs;
	for (auto gsit = gm->begin(); gsit != gm->end(); gsit++) {
		gs = *gsit;
		if (!gs->isDataSlice()) {
			continue;
		}
		double gsts = gs->getTimestamp().getFloat();
		double difference = (gsts-tsm) - tsd;
		if (!(fabs(difference) < 0.0001)) {
			continue;
		}
		// GridVoice* voice = gs->at(staffnum-1)->at(0)->at(0);
		// HTp token = voice->getToken();
		// if (token != NULL) {
		// 	token->setValue("LO", "TX", "t", text);	
		// } else {
		// 	cerr << "Strange null-token error while inserting dir element." << endl;
		// }
		foundslice = true;

		// Found data line which should prefixed with a layout line
		// should be done with HumHash post-processing, but do it manually for now.
	
		auto previousit = gsit;
		previousit--;
		if (previousit == gm->end()) {
			previousit = gsit;
		}
		auto previous = *previousit;
		if (previous->isLayoutSlice()) {
			GridVoice* voice = previous->at(staffnum-1)->at(0)->at(0);
			HTp tok = voice->getToken();
			if ((tok == NULL) || tok->isNull()) {
				tok->setText(message);
				break;
			}
		}

		// Insert a layout slice in front of current data slice.
		GridSlice* ngs = new GridSlice(gm, gs->getTimestamp(), SliceType::Layouts, m_maxStaffInFile);
		int parti = staffnum - 1;
		int staffi = 0;
		int voicei = 0;
		ngs->addToken(message, parti, staffi, voicei);
		gm->insert(gsit, ngs);

// ggg

		break;
	}
	if (!foundslice) {
		cerr << "Warning: dir elements not occuring at note/rest times are not yet supported" << endl;
	}
}



//////////////////////////////
//
// Tool_mei2hum::cleanDirText -- convert ":" to "&colon;".
//     Remove tabs and newlines, and trim spaces.  Maybe allow
//     newlines using "\n" and allow font changes in the future.
//     Do accents later perhaps or monitor for UTF-8.
//

string Tool_mei2hum::cleanDirText(const string& input) {
	string output;
	output.reserve(input.size() + 8);
	bool foundstart = false;
	for (int i=0; i<(int)input.size(); i++) {
		if ((!foundstart) && std::isspace(input[i])) {
			continue;
		}
		foundstart = true;
		if (input[i] == ':') {
			output += "&colon;";
		} else if (input[i] == '\t') {
			output += ' ';
		} else if (input[i] == '\n') {
			output += ' ';
		} else {
			output += input[i];
		}
	}
	while ((!output.empty()) && (output.back() == ' ')) {
		output.pop_back();
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::cleanVerseText -- 
//     Remove tabs and newlines, and trim spaces. 
//     Do accents later perhaps or monitor for UTF-8.
//

string Tool_mei2hum::cleanVerseText(const string& input) {
	string output;
	output.reserve(input.size() + 8);
	bool foundstart = false;
	for (int i=0; i<(int)input.size(); i++) {
		if ((!foundstart) && std::isspace(input[i])) {
			continue;
		}
		foundstart = true;
		if (input[i] == '\t') {
			output += ' ';
		} else if (input[i] == '\n') {
			output += ' ';
		} else {
			output += input[i];
		}
	}
	while ((!output.empty()) && (output.back() == ' ')) {
		output.pop_back();
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::cleanReferenceRecordText -- convert ":" to "&colon;".
//     Remove tabs and newlines, and trim spaces.  Maybe allow
//     newlines using "\n" and allow font changes in the future.
//     Do accents later perhaps or monitor for UTF-8.
//

string Tool_mei2hum::cleanReferenceRecordText(const string& input) {
	string output;
	output.reserve(input.size() + 8);
	bool foundstart = false;
	char lastchar = '\0';
	for (int i=0; i<(int)input.size(); i++) {
		if ((!foundstart) && std::isspace(input[i])) {
			continue;
		}
		foundstart = true;
		if (input[i] == '\n') {
			if (lastchar != ' ') {
				output += ' ';
			}
			lastchar = ' ';
		} else if (input[i] == '\t') {
			if (lastchar != ' ') {
				output += ' ';
			}
			lastchar = ' ';
		} else {
			output += input[i];
			lastchar = input[i];
		}
	}
	while ((!output.empty()) && (output.back() == ' ')) {
		output.pop_back();
	}

	return output;
}



//////////////////////////////
//
// Tool_mei2hum::parseDynam --
//
// Example:
//     <dynam staff="1" tstamp="1.000000">p</dynam>
//

void Tool_mei2hum::parseDynam(xml_node dynam, HumNum starttime) {
	NODE_VERIFY(dynam, )

	string text = dynam.child_value();
	// maybe check for valid text content here.
	if (text.empty()) {
		return;
	}

	string startid = dynam.attribute("startid").value();

	int staffnum = dynam.attribute("staff").as_int();
	if (staffnum == 0) {
		cerr << "Error: staff number required on dynam element" << endl;
		return;
	}
	double meterunit = m_currentMeterUnit[staffnum - 1];

	if (!startid.empty()) {
		// Dynamic is (or at least should) be attached directly
		// do a note, so it is handled elsewhere.
		cerr << "Warning DYNAMIC " << text << " is not yet processed." << endl;
		return;
	}

	string ts = dynam.attribute("tstamp").value();
	if (ts.empty()) {
		cerr << "Error: no timestamp on dynam element" << endl;
		return;
	}
	double tsd = (stof(ts)-1) * 4.0 / meterunit;
	GridMeasure* gm = m_outdata.back();
	double tsm = gm->getTimestamp().getFloat();
	bool foundslice = false;
	for (auto gs : *gm) {
		if (!gs->isDataSlice()) {
			continue;
		}
		double gsts = gs->getTimestamp().getFloat();
		double difference = (gsts-tsm) - tsd;
		if (!(fabs(difference) < 0.0001)) {
			continue;
		}
		GridPart* part = gs->at(staffnum-1);
		part->setDynamics(text);
		m_outdata.setDynamicsPresent(staffnum-1);
		foundslice = true;
		break;
	}
	if (!foundslice) {
		cerr << "Warning: dynamics not attched to system events are not yet supported" << endl;
	}

}



// END_MERGE

} // end namespace hum


