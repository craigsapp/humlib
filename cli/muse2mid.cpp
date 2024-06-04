//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Wed May 29 18:46:42 PDT 2024
// Last Modified: Wed May 29 20:15:54 PDT 2024
// Filename:      cli/muse2mid.cpp
// URL:           https://github.com/craigsapp/g/blob/master/src/muse2mid.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab
//
// Description:   Converter from MuseData to MIDI files.
//

#include "Convert.h"
#include "MidiFile.h"
#include "MuseDataSet.h"
#include "MuseData.h"
#include "Options.h"

#include <iostream>

using namespace std;
using namespace hum;
using namespace smf;

void setTempo        (MidiFile& midiout);
void setTrackNames   (MidiFile& midiout, MuseDataSet& mds);
void setTpq          (MidiFile& midiout, MuseDataSet& mds);
void processData     (MidiFile& midiout, MuseDataSet& mds);
void convertPartData (MidiEventList& outlist, MuseData& md, int channel);

// Global variables:
int Tpq = 120;

Options options;
bool graceQ = true;   // Used with -G option
bool tieQ   = false;  // Todo: add -T option to attack tied notes


///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	options.define("t|tempo=d:120", "Set tempo of MIDI file");
	options.define("o|output=s",    "Save MIDI file to given filename");
	options.define("G|no-grace=b",  "Do not translate grace notes");
	options.process(argc, argv);
	graceQ = !options.getBoolean("no-grace");
	MuseDataSet mds;
	int status = 1;
	if (options.getArgCount() == 0) {
		status = mds.readString(cin);
		if (!status) {
			cerr << "Problem reading input string " << endl;
		}
	} else {
		for (int i=0; i<options.getArgCount(); i++) {
			MuseData* md;
			md = new MuseData;
			status &= md->readFile(options.getArg(i+1));
			if (!status) {
				cerr << "Problem reading " << options.getArg(i+1) << endl;
			}
			mds.appendPart(md);
		}
	}

	MidiFile midiout;
	setTpq(midiout, mds);
	setTempo(midiout);
	midiout.absoluteTicks(); // time information stored as absolute time rather than delta time
	processData(midiout, mds);

	setTrackNames(midiout, mds);
	midiout.sortTracks();
	midiout.deltaTicks();
	if (options.getBoolean("output")) {
		midiout.write(options.getString("output"));
	} else {
		cout << midiout;
	}

	return !status;
}


///////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// setTrackNames -- set the track names in the MIDI file to the instrument names
//     in the MuseData file(s).
//

void setTrackNames(MidiFile& midiout, MuseDataSet& mds) {
	for (int i=0; i<mds.getFileCount(); i++) {
		string name = mds[i].getPartName();
		if (name.empty()) {
			name = "track " + to_string(i+1);
		}
		MidiEvent trackName;
		trackName.makeTrackName(name);
		trackName.tick = 0;
		midiout[i+1].push_back(trackName);
	}
}



//////////////////////////////
//
// setTempo -- set the tempo of the MIDI file (in quarter notes per minute).
//

void setTempo(MidiFile& midiout) {
	if (options.getBoolean("tempo")) {
		double tempo = options.getDouble("tempo");
		MidiEvent mm;
		mm.setTempo(tempo);
		mm.tick = 0;
		midiout[0].push_back(mm);
	}
}



////////////////////
//
// setTpq -- Set the Ticks-per-quarter parameter in the MIDI file header.
//     Will have a problem if the Q: parameter in a MuseData part changes
//     after the initial $ record.
//

void setTpq(MidiFile& midiout, MuseDataSet& mds) {
	vector<int> tpqs;
	for (int i=0; i<mds.getFileCount(); i++) {
		int tpq = mds[i].getInitialTpq();
		tpqs.push_back(tpq);
	}

	// Make Tpq LCM of all individual track ticks.
	Tpq = Convert::getLcm(tpqs);
	// Add extra ticks so that grace notes are maximum 16th notes:
	midiout.setTicksPerQuarterNote(Tpq);
}



//////////////////////////////
//
// processData -- Place each MuseData part in a separate track;
//

void processData(MidiFile& midiout, MuseDataSet& mds) {
	int tracks = mds.getFileCount();
	midiout.addTrack(tracks);  // Add a tracks for each part
	for (int i=0; i<mds.getFileCount(); i++) {
		// basic channel assignment (channels limited to 16 in MIDI):
		int channel = i;
		if (channel >= 9) {
			// skip over percussion channel
			channel++;
		}
		channel %= 16;
		convertPartData(midiout[i+1], mds[i], channel);
	}
}



//////////////////////////////
//
// convertPartData -- Convert a MuseData part file into a MIDI track.
//

void convertPartData(MidiEventList& outlist, MuseData& md, int channel) {
	int velocity = 64;
	MidiEvent meon;
	MidiEvent meoff;
	for (int i=0; i<md.getLineCount(); i++) {
		if (!md[i].isAnyNote()) {
			continue;
		}
		HumNum startTime = md[i].getQStamp();
		HumNum duration = md[i].getNoteDuration();
		HumNum endTime = startTime + duration;
		startTime *= Tpq;
		endTime   *= Tpq;
		if (startTime == endTime) {
			if (graceQ) {
				continue;
			}
			// Give grace notes a short duration:
			startTime -= 1;
			if (startTime < 0) {
				startTime = 0;
				endTime += 1;
			}
		}
		int midiPitch = Convert::base40ToMidiNoteNumber(md[i].getBase40());
		// Deal with tied notes here (if tieQ is true).
		meon.makeNoteOn(channel, midiPitch, velocity);
		meoff.makeNoteOff(channel, midiPitch, velocity);
		meon.tick = (int)startTime.getFloat();
		meoff.tick = (int)endTime.getFloat();
		outlist.push_back(meon);
		outlist.push_back(meoff);
	}
}



