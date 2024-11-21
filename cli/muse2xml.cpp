//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Thu May 30 10:21:57 PDT 2024
// Last Modified: Thu May 30 11:12:11 PDT 2024
// Filename:      cli/muse2xml.cpp
// URL:           https://github.com/craigsapp/g/blob/master/src/muse2xml.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Converter from MuseData to MusicXML.
//
// Reference:     https://wiki.ccarh.org/images/9/9f/Stage2-specs.html
// Reference:     https://www.w3.org/2021/06/musicxml40
//

#include "Convert.h"
#include "MuseDataSet.h"
#include "MuseData.h"
#include "Options.h"

#include "pugiconfig.hpp"
#include "pugixml.hpp"

#include <iostream>

using namespace std;
using namespace hum;
using namespace pugi;

void initializeXmlDocument(xml_document& xmlout);
void preparePartList      (xml_document& xmlout, MuseDataSet& mds);
void convertPartData      (xml_document& xmlout, MuseData& md, int partIndex);
int  convertPartMeasure   (xml_node& partxml, MuseData& md, int index, int& tpq);
void convertNoteOrRest    (xml_node& partxml, MuseData& md, int index, int tpq);
void convertAttributes    (xml_node& partxml, MuseData& md, int index, int& tpq);

 
Options options;


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	options.process(argc, argv);
	MuseDataSet mds;
	bool output = true;
	if (options.getArgCount() == 0) {
		mds.readString(cin);
	} else {
		for (int i=0; i<options.getArgCount(); i++) {
			MuseData* md;
			md = new MuseData;
			output &= md->readFile(options.getArg(i+1));
			mds.appendPart(md);
		}
	}

	xml_document xmlout;
	initializeXmlDocument(xmlout);
	preparePartList(xmlout, mds);
	for (int i=0; i<mds.getFileCount(); i++) {
		convertPartData(xmlout, mds[i], i);
	}

	xmlout.print(cout);

	return !output;
}


///////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// convertPartData -- Convert a MuseData part file into a MIDI track.
//

void convertPartData(xml_document& xmlout, MuseData& md, int partIndex) {
	xml_node score_partwise = xmlout.document_element();
	xml_node partxml = score_partwise.append_child("part");
	string partId = "P" + to_string(partIndex + 1);
	partxml.append_attribute("id") = partId.c_str();
	int tpq = 0;

	for (int i=0; i<md.getLineCount(); i++) {
		i = convertPartMeasure(partxml, md, i, tpq) - 1;
	}
}



//////////////////////////////
//
// convertPartMeasure -- Convert a single measure of a part.  Returns index
//     for start of next measure.
//

int convertPartMeasure(xml_node& partxml, MuseData& md, int index, int& tpq) {
	if (!md.measureHasData(index)) {
		int nextMeasureIndex = md.getNextMeasureIndex(index);
		return nextMeasureIndex;
	}
	string measureNumber = md[index].getMeasureNumber();
	xml_node measurexml = partxml.append_child("measure");
	if (!measureNumber.empty()) {
		measurexml.append_attribute("number") = measureNumber.c_str();
	}
	
	for (int i=index+1; i<md.getLineCount(); i++) {
		if (md[i].isAnyNoteOrRest()) {
			convertNoteOrRest(measurexml, md, i, tpq);
		} else if (md[i].isAttributes()) {
			convertAttributes(measurexml, md, i, tpq);
		} else if (md[i].isMeasure()) {
			// Add barline if not plain
			return i;
		}
	}
	return md.getLineCount();
}



//////////////////////////////
//
// convertAttributes --
//

void convertAttributes(xml_node& partxml, MuseData& md, int index, int& tpq) {
	map<string, string> attributes;
	md[index].getAttributeMap(attributes);
	if (attributes.empty()) {
		return;
	}

	xml_node attributesxml = partxml.append_child("attributes");

	// <divisions> (number of duration ticks in quarter note)
	if (!attributes["Q"].empty()) {
		tpq = stoi(attributes["Q"]);
		xml_node divisionsxml = attributesxml.append_child("divisions");
		divisionsxml.append_child(pugi::node_pcdata).set_value(attributes["Q"].c_str());
	}

	// <key> (key signature)
	// https://www.w3.org/2021/06/musicxml40/musicxml-reference/elements/key

	// <time> (time signature)

	// <staves> (number of staves in part)

	// <clef>
	// Reference: https://www.w3.org/2021/06/musicxml40/musicxml-reference/elements/clef
	if (!attributes["C"].empty()) {
		string kclef = Convert::museClefToKernClef(attributes["C"]);
		HumRegex hre;
		if (hre.search(kclef, "([FGC])")) {
			string letter = hre.getMatch(1);
			xml_node clefxml = attributesxml.append_child("clef");

			// <clef><sign> (required)
			xml_node signxml = clefxml.append_child("sign");
			signxml.append_child(pugi::node_pcdata).set_value(letter.c_str());

			// <clef><line> (optional)
			if (hre.search(kclef, "(\\d)")) {
				string line = hre.getMatch(1);
				xml_node linexml = clefxml.append_child("line");
				linexml.append_child(pugi::node_pcdata).set_value(line.c_str());
			}

			// <clef><clef-octave-change> (optional)
			if (hre.search(kclef, "(v+)")) {
				xml_node cocxml = clefxml.append_child("clef-octave-change");
				string value = hre.getMatch(1);
				string number = to_string(-(int)value.size());
				cocxml.append_child(pugi::node_pcdata).set_value(number.c_str());
			} else if (hre.search(kclef, "(\\^+)")) {
				xml_node cocxml = clefxml.append_child("clef-octave-change");
				string value = hre.getMatch(1);
				string number = to_string((int)value.size());
				cocxml.append_child(pugi::node_pcdata).set_value(number.c_str());
			}
		}
	}
}


//////////////////////////////
//
// convertNoteOrRest --
//

void convertNoteOrRest(xml_node& measurexml, MuseData& md, int index, int tpq) {
	xml_node notexml = measurexml.append_child("note");
	if (md[index].isAnyRest()) {
		// <rest>
		notexml.append_child("rest");
	} 
	if (md[index].isChordNote()) {
		// <chord>
		notexml.append_child("chord");
	}
	if (md[index].isGraceNote()) {
		// <grace>
		notexml.append_child("grace");
	}

	// <pitch>
	if (md[index].isAnyNote()) {
		xml_node pitchxml = notexml.append_child("pitch");
		string pc         = md[index].getPitchClassString();
		pc.resize(1);
		string accidental = to_string(md[index].getAccidental());
		string octave     = to_string(md[index].getOctave());

		// <pitch><step>
		xml_node stepxml = pitchxml.append_child("step");
		stepxml.append_child(pugi::node_pcdata).set_value(pc.c_str());
		// <pitch><alter>
		if (accidental != "0") {
			xml_node alterxml = pitchxml.append_child("alter");
			alterxml.append_child(pugi::node_pcdata).set_value(accidental.c_str());
		}
		// <pitch><octave>
		xml_node octavexml = pitchxml.append_child("octave");
		octavexml.append_child(pugi::node_pcdata).set_value(octave.c_str());
	}

	// <duration>
	if (!md[index].isGraceNote()) {
		int ticks = md[index].getTickDuration();
		HumNum newdur = ticks;
		string duration = to_string(newdur.getNumerator());
		xml_node durationxml = notexml.append_child("duration");
		durationxml.append_child(pugi::node_pcdata).set_value(duration.c_str());
	}

	// <voice>

	// <type> (visual note/rest duration)
	if (md[index].graphicNoteTypeQ()) {
		int value = md[index].getGraphicNoteType();
		xml_node typexml = notexml.append_child("type");
		// Reference: https://www.w3.org/2021/06/musicxml40/musicxml-reference/data-types/note-type-value
		switch (value) {
			case 1024:
				// not possible in MuseData
				break;
			case 512:
				typexml.append_child(pugi::node_pcdata).set_value("512th");
				break;
			case 256:
				typexml.append_child(pugi::node_pcdata).set_value("256th");
				break;
			case 128:
				typexml.append_child(pugi::node_pcdata).set_value("128th");
				break;
			case 64:
				typexml.append_child(pugi::node_pcdata).set_value("64th");
				break;
			case 32:
				typexml.append_child(pugi::node_pcdata).set_value("32nd");
				break;
			case 16:
				typexml.append_child(pugi::node_pcdata).set_value("16th");
				break;
			case 8:
				typexml.append_child(pugi::node_pcdata).set_value("eighth");
				break;
			case 4:
				typexml.append_child(pugi::node_pcdata).set_value("quarter");
				break;
			case 2:
				typexml.append_child(pugi::node_pcdata).set_value("half");
				break;
			case 1:
				typexml.append_child(pugi::node_pcdata).set_value("whole");
				break;
			case 0:
				typexml.append_child(pugi::node_pcdata).set_value("breve");
				break;
			case -1:
				typexml.append_child(pugi::node_pcdata).set_value("long");
				break;
			case -2:
				typexml.append_child(pugi::node_pcdata).set_value("maxima");
				break;
		}
		// <type@symbol-size>
	}

	// <dot>
	int dots = md[index].getDotCount();
	if (dots > 0) {
		for (int i=0; i<dots; i++) {
			notexml.append_child("dot");
		}
	}
	
	// <accidental> (visual accidental)
	if (md[index].isAnyNote() && md[index].notatedAccidentalQ()) {
		int accidental = md[index].getNotatedAccidental();
		xml_node accidentalxml = notexml.append_child("accidental");
		switch (accidental) {
			case  2:
				accidentalxml.append_child(pugi::node_pcdata).set_value("double-sharp");
				break;
			case  1:
				accidentalxml.append_child(pugi::node_pcdata).set_value("sharp");
				break;
			case  0:
				accidentalxml.append_child(pugi::node_pcdata).set_value("natural");
				break;
			case -1:
				accidentalxml.append_child(pugi::node_pcdata).set_value("flat");
				break;
			case -2:
				accidentalxml.append_child(pugi::node_pcdata).set_value("double-flat");
				break;
		}
	}

	// <stem>
	if (md[index].isAnyNote()) {
		int stemdir = md[index].getStemDirection();
		if (stemdir) {
			xml_node stemxml = notexml.append_child("stem");
			if (stemdir > 0) {
				stemxml.append_child(pugi::node_pcdata).set_value("up");
			} else {
				stemxml.append_child(pugi::node_pcdata).set_value("down");
			}
		}
	}

	// <staff>

}



//////////////////////////////
//
// preparePartList -- Generate <part-list>.
//

void preparePartList(xml_document& xmlout, MuseDataSet& mds) {
	xml_node score_partwise = xmlout.document_element();

	// Add part list
	pugi::xml_node part_list = score_partwise.append_child("part-list");

	HumRegex hre;
	for (int i=0; i<mds.getFileCount(); i++) {
		pugi::xml_node score_part = part_list.append_child("score-part");
		string partId = "P" + to_string(i+1);
		score_part.append_attribute("id") = partId.c_str();
		pugi::xml_node part_name = score_part.append_child("part-name");
		string partName = mds[i].getPartName();
		// Escape quotation marks in part name:
		hre.replaceDestructive(partName, "&quot;", "\"", "g");
		part_name.append_child(pugi::node_pcdata).set_value(partName.c_str());
	}
}



//////////////////////////////
//
// initializeXmlDocument --
//

void initializeXmlDocument(xml_document& xmlout) {
    // Add XML declaration:
    pugi::xml_node declaration = xmlout.append_child(pugi::node_declaration);
    declaration.append_attribute("version") = "1.0";
    declaration.append_attribute("encoding") = "UTF-8";

    // Create the root element (score-partwise):
    pugi::xml_node score_partwise = xmlout.append_child("score-partwise");
    score_partwise.append_attribute("version") = "4.0";
}



