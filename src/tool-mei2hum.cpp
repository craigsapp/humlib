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



//////////////////////////////
//
// Tool_mei2hum::Tool_mei2hum --
//

Tool_mei2hum::Tool_mei2hum(void) {
	define("app|app-label=s", "app label to follow");
	define("r|recip=b", "output **recip spine");
	define("s|stems=b", "include stems in output");
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
		if (token == "") {
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
		if (token == "") {
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
	// do something here
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
			cerr << "Don't know how to process score/" << nodename << endl;
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
			cerr << "Don't know how to process scoreDef/" << nodename << endl;
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
			cerr << "Don't know how to process staffGrp/" << nodename << endl;
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
	string metercount;
	string meterunit;
	string staffnum;
	string keysig;
	string midibpm;

	for (auto atti = element.attributes_begin(); atti != element.attributes_end();
				atti++) {
		string attname = atti->name();
		if (attname == "clef.shape") {
			clefshape = atti->value();
		} else if (attname == "clef.line") {
			clefline = atti->value();
		} else if (attname == "meter.count") {
			metercount = atti->value();
		} else if (attname == "meter.unit") {
			meterunit = atti->value();
		} else if (attname == "key.sig") {
			keysig = atti->value();
		} else if (attname == "midi.bpm") {
			midibpm = atti->value();
		}
	}

	if ((!clefshape.empty()) && (!clefline.empty())) {
		staffinfo.clef = "*clef" + clefshape + clefline;
	}
	if ((!metercount.empty()) && (!meterunit.empty())) {
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
		} else {
			cerr << "Don't know how to parse section/" << nodename << endl;
		}
	}

	return starttime;
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
		cerr << "Don't know how to parse app/" << nodename << endl;
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
			cerr << "Don't know how to parse lem/" << nodename << endl;
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
			cerr << "Don't know how to parse rdg/" << nodename << endl;
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

	GridMeasure* gm = m_outdata.addMeasureToBack();
	gm->setTimestamp(starttime QUARTER_CONVERT);

	vector<HumNum> durations(children.size(), 0);

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "staff") {
			durations[i] = parseStaff(children[i], starttime);
		} else if (nodename == "fermata") {
			// handled in process processNodeStartLinks()
		} else if (nodename == "slur") {
			// handled in process processNode(Start|Stop)Links()
		} else if (nodename == "tie") {
			// handled in process processNode(Start|Stop)Links()
		} else {
			cerr << "Do not know how to parse measure/" << nodename << endl;
		}
	}

	if (durations.empty()) {
		return starttime;
	}

	HumNum firstdur = durations[0];
	int staffnumber = 0;
	for (int i=1; i<durations.size(); i++) {
		if (strcmp(children[i].name(), "staff") != 0) {
			continue;
		}
		staffnumber++;
		if (durations[i] != firstdur) {
			cerr << "Error: staves do not have same duration in measure" << endl;
			cerr << "First staff duration: " << firstdur << endl;
			cerr << "Staff " << staffnumber << "'s duration:  " << durations[i] << endl;
			break;
		}
	}

	// Check that the duration of each layer is the same here.

	gm->setTimestamp(starttime QUARTER_CONVERT);
	gm->setDuration((firstdur - starttime) QUARTER_CONVERT);

	if (strcmp(measure.attribute("right").value(), "end") == 0) {
		gm->setFinalBarlineStyle();
	}

	return durations[0];
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
	if (n == "") {
		cerr << "Warning: no staff number on staff element" << endl;
	} else {
		nnum = stoi(n);
	}
	if (nnum < 1) {
		cerr << "Error: invalid staff number: " << nnum << endl;
	}
	m_currentstaff = nnum;

	vector<HumNum> durations(children.size(), 0);

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "layer") {
			durations[i] = parseLayer(children[i], starttime);
		} else {
			cerr << "Don't know how to parse staff/" << nodename << endl;
		}
	}

	// Check that the duration of each staff is the same here.

	m_currentstaff = 0;

	return durations[0];
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
	if (n == "") {
		cerr << "Warning: no layer number on layer element" << endl;
	} else {
		nnum = stoi(n);
	}
	if (nnum < 1) {
		cerr << "Error: invalid layer number: " << nnum << endl;
	}
	m_currentlayer = nnum;

	vector<HumNum> durations(children.size(), 0);
	string dummy;

	for (int i=0; i<(int)children.size(); i++) {
		string nodename = children[i].name();
		if (nodename == "note") {
			starttime = parseNote(children[i], xml_node(NULL), dummy, starttime);
		} else if (nodename == "chord") {
			starttime = parseChord(children[i], starttime);
		} else if (nodename == "rest") {
			starttime = parseRest(children[i], starttime);
		} else if (nodename == "beam") {
			starttime = parseBeam(children[i], starttime);
		} else if (nodename == "tuplet") {
			starttime = parseTuplet(children[i], starttime);
		} else {
			cerr << "Don't know how to parse layer/" << nodename << endl;
		}
	}

	m_currentlayer = 0;

	return starttime;
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
			cerr << "Don't know how to parse tuplet/" << nodename << endl;
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

	m_beamPrefix = "L";

	xml_node lastnoterestchord;
	for (int i=children.size()-1; i>=0; i--) {
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
			m_beamPostfix = "J";
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
			cerr << "Don't know how to parse beam/" << nodename << endl;
		}
	}

	return starttime;
}



//////////////////////////////
//
// Tool_mei2hum::parseNote --
//

HumNum Tool_mei2hum::parseNote(xml_node note, xml_node chord, string& output, HumNum starttime) {
	NODE_VERIFY(note, starttime)

	HumNum duration;
	int dotcount;

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

	string articulations = getNoteArticulations(note, chord);

	string tok = recip + humpitch + articulations + m_beamPrefix + m_beamPostfix;
	m_beamPrefix.clear();
	m_beamPostfix.clear();

	processLinkedNodes(tok, note);
	processFermataAttribute(tok, note);

	if (!chord) {
		m_outdata.back()->addDataToken(tok, starttime QUARTER_CONVERT, m_currentstaff-1,
			0, m_currentlayer-1, m_staffcount);
	} else {
		output += tok;
	}

	return starttime + duration;
}



//////////////////////////////
//
// Tool_mei2hum::parseRest --
//

HumNum Tool_mei2hum::parseRest(xml_node rest, HumNum starttime) {
	NODE_VERIFY(rest, starttime)

	HumNum duration = getDuration(rest);
	int dotcount = getDotCount(rest);
	string recip = getHumdrumRecip(duration, dotcount);

	string output = recip + "r" + m_beamPrefix + m_beamPostfix;
	m_beamPrefix.clear();
	m_beamPostfix.clear();

	processLinkedNodes(output, rest);
	processFermataAttribute(output, rest);

	m_outdata.back()->addDataToken(output, starttime QUARTER_CONVERT, m_currentstaff-1,
		0, m_currentlayer-1, m_staffcount);

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
			parseFermata(output, node, nodelist[i]);
		} else if (nodename == "slur") {
			parseSlurStart(output, node, nodelist[i]);
		} else if (nodename == "tie") {
			parseTieStart(output, node, nodelist[i]);
		} else {
			cerr << "Don't know how to parse " << nodename
			     << " element in processNodeStartLinks()" << endl;
		}
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
			cerr << "Don't know how to parse " << nodename
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
		cerr << "Don't know what to do with a slur start attached to a "
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
		cerr << "Don't know what to do with a tie end attached to a "
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
		cerr << "Don't know what to do with a tie start attached to a "
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
		cerr << "Don't know what to do with a tie end attached to a "
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
		cerr << "Don't know what to do with a fermata attached to a "
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
		output = 1;
		output /= 2;
	} else if (dur == "long") {
		output = 1;
		output /= 4;
	} else if (dur == "maxima") {
		output = 1;
		output /= 8;
	} else if (isdigit(dur[0])) {
		output = 1;
		output /= stoi(dur);
	} else {
		cerr << "Unknown " << element.name() << "@dur: " << dur << endl;
		return 0;
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
			cerr << "Don't know how to parse chord/" << nodename << endl;
		}
	}

	processLinkedNodes(tok, chord);
	processFermataAttribute(tok, chord);

	m_outdata.back()->addDataToken(tok, starttime QUARTER_CONVERT, m_currentstaff-1,
		0, m_currentlayer-1, m_staffcount);

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



// END_MERGE

} // end namespace hum


