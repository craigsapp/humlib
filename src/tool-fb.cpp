//
// Programmer:    Wolfgang Drescher <drescher.wolfgang@gmail.com>
// Creation Date: Sun Nov 27 2022 00:25:34 CET
// Filename:      tool-fb.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-fb.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Add figured bass numbers from **kern spines.
// Documentation: https://doc.verovio.humdrum.org/filter/fb
//

#include "tool-fb.h"
#include "Convert.h"
#include "HumRegex.h"
#include <regex>
#include <cmath>

using namespace std;

namespace hum {

// START_MERGE

//////////////////////////////
//
// Tool_fb::Tool_fb -- Set the recognized options for the tool.
//

Tool_fb::Tool_fb(void) {
	define("c|compound=b",               "Output reasonable figured bass numbers within octave");
	define("a|accidentals|accid|acc=b",  "Display accidentals in front of the numbers");
	define("b|base|base-track=i:1",      "Number of the base kern track (compare with -k)");
	define("i|intervallsatz=b",          "Display numbers under their voice instead of under the base staff");
	define("o|sort|order=b",             "Sort figured bass numbers by size");
	define("l|lowest=b",                 "Use lowest note as base note");
	define("n|normalize=b",              "Remove number 8 and doubled numbers; adds -co");
	define("r|reduce|abbreviate|abbr=b", "Use abbreviated figures; adds -nco");
	define("t|ties=b",                   "Hide numbers without attack or changing base (needs -i)");
	define("f|figuredbass=b",            "Shortcut for -acorn3");
	define("3|hide-three=b",             "Hide number 3 if it has an accidental");
	define("m|negative=b",               "Show negative numbers");
	define("above=b",                    "Show numbers above the staff (**fba)");
	define("rate=s:",                    "Rate to display the numbers (use a **recip value, e.g. 4, 4.)");
	define("k|kern-tracks=s",            "Process only the specified kern spines");
	define("s|spine-tracks|spine|spines|track|tracks=s", "Process only the specified spines");
}



//////////////////////////////
//
// Tool_fb::run -- Do the main work of the tool.
//

bool Tool_fb::run(HumdrumFileSet &infiles) {
	bool status = true;
	for (int i = 0; i < infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}

bool Tool_fb::run(const string &indata, ostream &out) {
	HumdrumFile infile(indata);
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

bool Tool_fb::run(HumdrumFile &infile, ostream &out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}

bool Tool_fb::run(HumdrumFile &infile) {
	initialize();
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_fb::initialize -- 
//

void Tool_fb::initialize(void) {
	m_compoundQ      = getBoolean("compound");
	m_accidentalsQ   = getBoolean("accidentals");
	m_baseTrackQ     = getInteger("base");
	m_intervallsatzQ = getBoolean("intervallsatz");
	m_sortQ          = getBoolean("sort");
	m_lowestQ        = getBoolean("lowest");
	m_normalizeQ     = getBoolean("normalize");
	m_reduceQ        = getBoolean("reduce");
	m_attackQ        = getBoolean("ties");
	m_figuredbassQ   = getBoolean("figuredbass");
	m_hideThreeQ     = getBoolean("hide-three");
	m_showNegativeQ  = getBoolean("negative");
	m_aboveQ         = getBoolean("above");
	m_rateQ          = getString("rate");

	if (getBoolean("spine-tracks")) {
		m_spineTracks = getString("spine-tracks");
	} else if (getBoolean("kern-tracks")) {
		m_kernTracks = getString("kern-tracks");
	}

	if (m_normalizeQ) {
		m_compoundQ = true;
		m_sortQ = true;
	}

	if (m_reduceQ) {
		m_normalizeQ = true;
		m_compoundQ = true;
		m_sortQ = true;
	}

	if (m_figuredbassQ) {
		m_reduceQ = true;
		m_normalizeQ = true;
		m_compoundQ = true;
		m_sortQ = true;
		m_accidentalsQ = true;
		m_hideThreeQ = true;
	}
}



//////////////////////////////
//
// Tool_fb::processFile -- 
//

void Tool_fb::processFile(HumdrumFile& infile) {

	NoteGrid grid(infile);

	vector<FiguredBassNumber*> numbers;

	vector<HTp> kernspines = infile.getKernSpineStartList();

	int maxTrack = infile.getMaxTrack();

	// Do nothing if base track not withing kern track range
	if (m_baseTrackQ < 1 || m_baseTrackQ > maxTrack) {
		return;
	}

	m_selectedKernSpines.resize(maxTrack + 1); // +1 is needed since track=0 is not used
	// By default, process all tracks:
	fill(m_selectedKernSpines.begin(), m_selectedKernSpines.end(), true);
	// Otherwise, select which **kern track, or spine tracks to process selectively:

	// Calculate which input spines to process based on -s or -k option:
	if (!m_kernTracks.empty()) {
		vector<int> ktracks = Convert::extractIntegerList(m_kernTracks, maxTrack);
		fill(m_selectedKernSpines.begin(), m_selectedKernSpines.end(), false);
		for (int i=0; i<(int)ktracks.size(); i++) {
			int index = ktracks[i] - 1;
			if ((index < 0) || (index >= (int)kernspines.size())) {
				continue;
			}
			int track = kernspines.at(ktracks[i] - 1)->getTrack();
			m_selectedKernSpines.at(track) = true;
		}
	} else if (!m_spineTracks.empty()) {
		infile.makeBooleanTrackList(m_selectedKernSpines, m_spineTracks);
	}

	vector<vector<int>> lastNumbers = {};
	lastNumbers.resize((int)grid.getVoiceCount());
	vector<vector<int>> currentNumbers = {};

	// Interate through the NoteGrid and fill the numbers vector with
	// all generated FiguredBassNumbers
	for (int i=0; i<(int)grid.getSliceCount(); i++) {
		currentNumbers.clear();
		currentNumbers.resize((int)grid.getVoiceCount());

		// Reset usedBaseKernTrack
		int usedBaseKernTrack = m_baseTrackQ;

		// Overwrite usedBaseKernTrack with the lowest voice index of the lowest pitched note
		if (m_lowestQ) {
			int lowestNotePitch = 99999;
			for (int k=0; k<(int)grid.getVoiceCount(); k++) {
				NoteCell* checkCell = grid.cell(k, i);
				HTp currentToken = checkCell->getToken();
				int initialTokenTrack = currentToken->getTrack();

				// Handle spine splits
				do {
					HTp resolvedToken = currentToken->resolveNull();
					
					int lowest = getLowestBase40Pitch(resolvedToken->getBase40Pitches());

					if (abs(lowest) < lowestNotePitch) {
						lowestNotePitch = abs(lowest);
						usedBaseKernTrack = k + 1;
					}

					HTp nextToken = currentToken->getNextField();
					if (nextToken && (initialTokenTrack == nextToken->getTrack())) {
						currentToken = nextToken;
					} else {
						// Break loop if nextToken is not the same track as initialTokenTrack
						break;
					}
				} while (currentToken);
			}
		}

		NoteCell* baseCell = grid.cell(usedBaseKernTrack - 1, i);

		// Ignore grace notes
		if (baseCell->getToken()->getOwner()->getDuration() == 0) {
			continue;
		}

		string keySignature = getKeySignature(infile, baseCell->getLineIndex());

		// Hide numbers if they do not match rhythmic position of --rate
		if (!m_rateQ.empty()) {
			// Get time signatures
			vector<pair<int, HumNum>> timeSigs;
			infile.getTimeSigs(timeSigs, baseCell->getToken()->getTrack());
			// Ignore numbers if they don't fit
			if (hideNumbersForTokenLine(baseCell->getToken(), timeSigs[baseCell->getLineIndex()])) {
				continue;
			}
		}


		HTp currentToken = baseCell->getToken();
		int initialTokenTrack = baseCell->getToken()->getTrack();
		int lowestBaseNoteBase40Pitch = 9999;

		// Handle spine splits
		do {
			HTp resolvedToken = currentToken->resolveNull();
			
			int lowest = getLowestBase40Pitch(resolvedToken->getBase40Pitches());

			// Ignore if base is a rest or silent note
			if ((lowest != 0) && (lowest != -1000) && (lowest != -2000)) {
				if(abs(lowest) < lowestBaseNoteBase40Pitch) {
					lowestBaseNoteBase40Pitch = abs(lowest);
				}
			}

			HTp nextToken = currentToken->getNextField();
			if (nextToken && (initialTokenTrack == nextToken->getTrack())) {
				currentToken = nextToken;
			} else {
				// Break loop if nextToken is not the same track as initialTokenTrack
				break;
			}
		} while (currentToken);

		// Ignore if base is a rest or silent note
		if ((lowestBaseNoteBase40Pitch == 0) || (lowestBaseNoteBase40Pitch == -1000) || (lowestBaseNoteBase40Pitch == -2000) || (lowestBaseNoteBase40Pitch == 9999)) {
			continue;
		}

		// Interate through each voice
		for (int j=0; j<(int)grid.getVoiceCount(); j++) {
			NoteCell* targetCell = grid.cell(j, i);

			// Ignore voice if track is not active by --kern-tracks or --spine-tracks
			if (m_selectedKernSpines.at(targetCell->getToken()->getTrack()) == false) {
				continue;
			}

			HTp currentToken = targetCell->getToken();
			int initialTokenTrack = targetCell->getToken()->getTrack();
			vector<FiguredBassNumber*> chordNumbers = {};

			// Handle spine splits
			do {
				HTp resolvedToken = currentToken->resolveNull();
				
				for (int subtokenBase40: resolvedToken->getBase40Pitches()) {

					// Ignore if target is a rest or silent note
					if ((subtokenBase40 == 0) || (subtokenBase40 == -1000) || (subtokenBase40 == -2000)) {
						continue;
					}
					
					// Ignore if same pitch as base voice
					if ((abs(lowestBaseNoteBase40Pitch) == abs(subtokenBase40)) && (baseCell->getToken()->getTrack() == initialTokenTrack)) {
						continue;
					}

					// Create FiguredBassNumber
					FiguredBassNumber* number = createFiguredBassNumber(abs(lowestBaseNoteBase40Pitch), abs(subtokenBase40), targetCell->getVoiceIndex(), targetCell->getLineIndex(), targetCell->isAttack(), keySignature);

					currentNumbers[j].push_back(number->m_number);
					chordNumbers.push_back(number);
				}

				HTp nextToken = currentToken->getNextField();
				if (nextToken && (initialTokenTrack == nextToken->getTrack())) {
						currentToken = nextToken;
				} else {
					// Break loop if nextToken is not the same track as initialTokenTrack
					break;
				}
			} while (currentToken);

			// Sort chord numbers by size
			sort(chordNumbers.begin(), chordNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
				return a->m_number > b->m_number;
			});

			// Then add to numbers vector
			for (FiguredBassNumber*  num: chordNumbers) {
				if (lastNumbers[j].size() != 0) {
					// If a number belongs to a sustained note but the base note did change
					// the new numbers need to be displayable
					num->m_baseOfSustainedNoteDidChange = !num->m_isAttack && std::find(lastNumbers[j].begin(), lastNumbers[j].end(), num->m_number) == lastNumbers[j].end();
				}
				numbers.push_back(num);
			}
		}
		
		// Set current numbers as the new last numbers
		lastNumbers = currentNumbers;
	}

	string exinterp = m_aboveQ ? "**fba" : "**fb";

	if (m_intervallsatzQ) {
		// Create **fb spine for each voice
		for (int voiceIndex = 0; voiceIndex < grid.getVoiceCount(); voiceIndex++) {
			vector<string> trackData = getTrackDataForVoice(voiceIndex, numbers, infile.getLineCount());
			if (voiceIndex + 1 < grid.getVoiceCount()) {
				int trackIndex = kernspines[voiceIndex + 1]->getTrack();
				infile.insertDataSpineBefore(trackIndex, trackData, ".", exinterp);
			} else {
				infile.appendDataSpine(trackData, ".", exinterp);
			}
		}
	} else {
		// Create **fb spine and bind it to the base voice
		vector<string> trackData = getTrackData(numbers, infile.getLineCount());
		if (m_baseTrackQ < grid.getVoiceCount()) {
			int trackIndex = kernspines[m_baseTrackQ]->getTrack();
			infile.insertDataSpineBefore(trackIndex, trackData, ".", exinterp);
		} else {
			infile.appendDataSpine(trackData, ".", exinterp);
		}
	}

	// Enables usage in verovio (`!!!filter: fb`)
	m_humdrum_text << infile;
}



//////////////////////////////
//
// Tool_fb::hideNumbersForTokenLine -- Checks if rhythmic position of line should display numbers
//

bool Tool_fb::hideNumbersForTokenLine(HTp token, pair<int, HumNum> timeSig) {
	// Get note duration from --rate option
	HumNum rateDuration = Convert::recipToDuration(m_rateQ);
	if (rateDuration.toFloat() != 0) {
		double timeSigBarDuration = timeSig.first * Convert::recipToDuration(to_string(timeSig.second.getInteger())).toFloat();
		double durationFromBarline = token->getDurationFromBarline().toFloat();
		// Handle upbeats
		if (token->getBarlineDuration().toFloat() < timeSigBarDuration) {
			// Fix durationFromBarline when current bar duration is shorter than
			// the bar duration of the time signature
			durationFromBarline = timeSigBarDuration - token->getDurationToBarline().toFloat();
		}
		// Checks if rhythmic position is divisible by rateDuration
		return fmod(durationFromBarline, rateDuration.toFloat()) != 0;
	}
	return false;
}



//////////////////////////////
//
// Tool_fb::getTrackData -- Create **fb spine data with formatted numbers for all voices
//

vector<string> Tool_fb::getTrackData(const vector<FiguredBassNumber*>& numbers, int lineCount) {
	vector<string> trackData;
	trackData.resize(lineCount);

	for (int i = 0; i < lineCount; i++) {
		vector<FiguredBassNumber*> sliceNumbers = filterFiguredBassNumbersForLine(numbers, i);
		if (sliceNumbers.size() > 0) {
			trackData[i] = formatFiguredBassNumbers(sliceNumbers);
		}
	}

	return trackData;
}



//////////////////////////////
//
// Tool_fb::getTrackDataForVoice -- Create **fb spine data with formatted numbers for passed voiceIndex
//

vector<string> Tool_fb::getTrackDataForVoice(int voiceIndex, const vector<FiguredBassNumber*>& numbers, int lineCount) {
	vector<string> trackData;
	trackData.resize(lineCount);

	for (int i = 0; i < lineCount; i++) {
		vector<FiguredBassNumber*> sliceNumbers = filterFiguredBassNumbersForLineAndVoice(numbers, i, voiceIndex);
		if (sliceNumbers.size() > 0) {
			trackData[i] = formatFiguredBassNumbers(sliceNumbers);
		}
	}

	return trackData;
}



//////////////////////////////
//
// Tool_fb::createFiguredBassNumber -- Create FiguredBassNumber from a NoteCell.
//    The figured bass number (num) is calculated with a base and target NoteCell
//    as well as a passed key signature.
//

FiguredBassNumber* Tool_fb::createFiguredBassNumber(int basePitchBase40, int targetPitchBase40, int voiceIndex, int lineIndex, bool isAttack, string keySignature) {

	// Calculate figured bass number
	int baseDiatonicPitch   = Convert::base40ToDiatonic(basePitchBase40);
	int targetDiatonicPitch = Convert::base40ToDiatonic(targetPitchBase40);
	int diff        = abs(targetDiatonicPitch) - abs(baseDiatonicPitch);
	int num;

	if ((baseDiatonicPitch == 0) || (targetDiatonicPitch == 0)) {
		num = 0;
	} else if (diff == 0) {
		num = 1;
	} else if (diff > 0) {
		num = diff + 1;
	} else {
		num = diff - 1;
	}

	// Transform key signature to lower case
	transform(keySignature.begin(), keySignature.end(), keySignature.begin(), [](unsigned char c) {
		return tolower(c);
	});

	char targetPitchName = Convert::kernToDiatonicLC(Convert::base40ToKern(targetPitchBase40));
	int targetAccidNr = Convert::base40ToAccidental(targetPitchBase40);
	string targetAccid;
	for (int i=0; i<abs(targetAccidNr); i++) {
		targetAccid += (targetAccidNr < 0 ? "-" : "#");
	}

	char basePitchName = Convert::kernToDiatonicLC(Convert::base40ToKern(basePitchBase40));
	int baseAccidNr = Convert::base40ToAccidental(basePitchBase40);
	string baseAccid;
	for (int i=0; i<abs(baseAccidNr); i++) {
		baseAccid += (baseAccidNr < 0 ? "-" : "#");
	}

	string accid = targetAccid;
	bool showAccid = false;

	// Show accidentals when they are not included in the key signature
	if ((targetAccidNr != 0) && (keySignature.find(targetPitchName + targetAccid) == std::string::npos)) {
		showAccid = true;
	}

	// Show natural accidentals when they are alterations of the key signature
	if ((targetAccidNr == 0) && (keySignature.find(targetPitchName + targetAccid) != std::string::npos)) {
		accid = "n";
		showAccid = true;
	}

	// Show accidentlas when pitch class of base and target is equal but alteration is different
	if (basePitchName == targetPitchName) {
		if (baseAccidNr == targetAccidNr) {
			showAccid = false;
		} else {
			accid = (targetAccidNr == 0) ? "n" : targetAccid;
			showAccid = true;
		}
	}

	FiguredBassNumber* number = new FiguredBassNumber(num, accid, showAccid, voiceIndex, lineIndex, isAttack, m_intervallsatzQ);

	return number;
}



//////////////////////////////
//
// Tool_fb::filterNegativeNumbers -- Hide negative numbers if m_showNegativeQ if not true
//

vector<FiguredBassNumber*> Tool_fb::filterNegativeNumbers(vector<FiguredBassNumber*> numbers) {

	vector<FiguredBassNumber*> filteredNumbers;
	
	bool mQ = m_showNegativeQ;
	copy_if(numbers.begin(), numbers.end(), back_inserter(filteredNumbers), [mQ](FiguredBassNumber* num) {
		return mQ ? true : (num->m_number > 0);
	});

	return filteredNumbers;
}



//////////////////////////////
//
// Tool_fb::filterFiguredBassNumbersForLine -- Find all FiguredBassNumber objects for a slice (line index) of the music.
//

vector<FiguredBassNumber*> Tool_fb::filterFiguredBassNumbersForLine(vector<FiguredBassNumber*> numbers, int lineIndex) {

	vector<FiguredBassNumber*> filteredNumbers;

	// filter numbers with passed lineIndex
	copy_if(numbers.begin(), numbers.end(), back_inserter(filteredNumbers), [lineIndex](FiguredBassNumber* num) {
		return num->m_lineIndex == lineIndex;
	});

	// sort by voiceIndex
	sort(filteredNumbers.begin(), filteredNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
		return a->m_voiceIndex > b->m_voiceIndex; 
	});

	return filterNegativeNumbers(filteredNumbers);
}



//////////////////////////////
//
// Tool_fb::filterFiguredBassNumbersForLineAndVoice --
//

vector<FiguredBassNumber*> Tool_fb::filterFiguredBassNumbersForLineAndVoice(vector<FiguredBassNumber*> numbers, int lineIndex, int voiceIndex) {

	vector<FiguredBassNumber*> filteredNumbers;

	// filter numbers with passed lineIndex and passed voiceIndex
	copy_if(numbers.begin(), numbers.end(), back_inserter(filteredNumbers), [lineIndex, voiceIndex](FiguredBassNumber* num) {
		return (num->m_lineIndex == lineIndex) && (num->m_voiceIndex == voiceIndex);
	});

	// sort by voiceIndex (probably not needed here)
	sort(filteredNumbers.begin(), filteredNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
		return a->m_voiceIndex > b->m_voiceIndex; 
	});

	return filterNegativeNumbers(filteredNumbers);
}



//////////////////////////////
//
// Tool_fb::formatFiguredBassNumbers -- Create a **fb data record string out of the passed FiguredBassNumber objects
//

string Tool_fb::formatFiguredBassNumbers(const vector<FiguredBassNumber*>& numbers) {

	vector<FiguredBassNumber*> formattedNumbers;

	// Normalize numbers (remove 8 and 1, sort by size, remove duplicate numbers)
	if (m_normalizeQ) {
		bool aQ = m_accidentalsQ;
		// remove 8 and 1 but keep them if they have an accidental
		copy_if(numbers.begin(), numbers.end(), back_inserter(formattedNumbers), [aQ](FiguredBassNumber* num) {
			return ((num->getNumberWithinOctave() != 8) && (num->getNumberWithinOctave() != 1)) || (aQ && num->m_showAccidentals);
		});
		// sort by size
		sort(formattedNumbers.begin(), formattedNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
			return a->getNumberWithinOctave() < b->getNumberWithinOctave();
		});
		// remove duplicate numbers
		formattedNumbers.erase(unique(formattedNumbers.begin(), formattedNumbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) {
			return a->getNumberWithinOctave() == b->getNumberWithinOctave();
		}), formattedNumbers.end());
	} else {
		formattedNumbers = numbers;
	}

	// Hide numbers if they have no attack
	if (m_intervallsatzQ && m_attackQ) {
		vector<FiguredBassNumber*> attackNumbers;
		copy_if(formattedNumbers.begin(), formattedNumbers.end(), back_inserter(attackNumbers), [](FiguredBassNumber* num) {
			return num->m_isAttack || num->m_baseOfSustainedNoteDidChange;
		});
		formattedNumbers = attackNumbers;
	}

	// Analysze before sorting
	if (m_compoundQ) {
		formattedNumbers = analyzeChordNumbers(formattedNumbers);
	}

	// Sort numbers by size
	if (m_sortQ) {
		bool cQ = m_compoundQ;
		sort(formattedNumbers.begin(), formattedNumbers.end(), [cQ](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
			// sort by getNumberWithinOctave if compoundQ is true otherwise sort by number
			return (cQ) ? a->getNumberWithinOctave() > b->getNumberWithinOctave() : a->m_number > b->m_number;
		});
	}

	if (m_reduceQ) {
		// Overwrite formattedNumbers with abbreviated numbers
		formattedNumbers = getAbbreviatedNumbers(formattedNumbers);
	}

	// join numbers
	string str = "";
	bool first = true;
	for (FiguredBassNumber* number: formattedNumbers) {
		string num = number->toString(m_compoundQ, m_accidentalsQ, m_hideThreeQ);
		if (num.length() > 0) {
			if (!first) str += " ";
			first = false;
			str += num;
		}
	}
	return str;
}



//////////////////////////////
//
// Tool_fb::getAbbreviatedNumbers -- Get abbreviated figured bass numbers
//    If no abbreviation is found all numbers will be shown

vector<FiguredBassNumber*> Tool_fb::getAbbreviatedNumbers(const vector<FiguredBassNumber*>& numbers) {

	vector<FiguredBassNumber*> abbreviatedNumbers;

	string numberString = getNumberString(numbers);

	// Check if an abbreviation exists for passed numbers
	auto it = find_if(FiguredBassAbbreviationMapping::s_mappings.begin(), FiguredBassAbbreviationMapping::s_mappings.end(), [&numberString](const FiguredBassAbbreviationMapping& abbr) {
		return abbr.m_str == numberString;
	});

	if (it != FiguredBassAbbreviationMapping::s_mappings.end()) {
		const FiguredBassAbbreviationMapping& abbr = *it;
		bool aQ = m_accidentalsQ;
		// Store numbers to display by the abbreviation mapping in abbreviatedNumbers
		copy_if(numbers.begin(), numbers.end(), back_inserter(abbreviatedNumbers), [&abbr, aQ](FiguredBassNumber* num) {
			const vector<int>& nums = abbr.m_numbers;
			// Show numbers if they are part of the abbreviation mapping or if they have an accidental
			return (find(nums.begin(), nums.end(), num->getNumberWithinOctave()) != nums.end()) || (num->m_showAccidentals && aQ);
		});

		return abbreviatedNumbers;
	}

	return numbers;
}



//////////////////////////////
//
// Tool_fb::analyzeChordNumbers -- Analyze chord numbers and improve them
//    Set m_convert2To9 to true when a 3 is included in the chord numbers.

vector<FiguredBassNumber*> Tool_fb::analyzeChordNumbers(const vector<FiguredBassNumber*>& numbers) {

	vector<FiguredBassNumber*> analyzedNumbers = numbers;

	// Check if compound numbers 3 is withing passed numbers (chord)
	auto it = find_if(analyzedNumbers.begin(), analyzedNumbers.end(), [](FiguredBassNumber* number) {
		return number->getNumberWithinOctave() == 3;
	});
	if (it != analyzedNumbers.end()) {
		for (auto &number : analyzedNumbers) {  
			number->m_convert2To9 = true;
		}
	}

	return analyzedNumbers;
}



//////////////////////////////
//
// Tool_fb::getNumberString -- Get only the numbers (without accidentals) of passed FiguredBassNumbers
//

string Tool_fb::getNumberString(vector<FiguredBassNumber*> numbers) {
	// Sort numbers by size
	sort(numbers.begin(), numbers.end(), [](FiguredBassNumber* a, FiguredBassNumber* b) -> bool { 
		return a->getNumberWithinOctave() > b->getNumberWithinOctave();
	});
	// join numbers
	string str = "";
	bool first = true;
	for (FiguredBassNumber* nr: numbers) {
		int num = nr->getNumberWithinOctave();
		if (num > 0) {
			if (!first) str += " ";
			first = false;
			str += to_string(num);
		}	
	}
	return str;
}



//////////////////////////////
//
// Tool_fb::getKeySignature -- Get the key signature for a line index of the input file
//

string Tool_fb::getKeySignature(HumdrumFile& infile, int lineIndex) {
	string keySignature = "";
	[&] {
		for (int i = 0; i < infile.getLineCount(); i++) {
			if (i > lineIndex) {
				return;
			}
			HLp line = infile.getLine(i);
			for (int j = 0; j < line->getFieldCount(); j++) {
				if (line->token(j)->isKeySignature()) {
					keySignature = line->getTokenString(j);
				}
			}
		}
	}();
	return keySignature;
}



//////////////////////////////
//
// Tool_fb::getLowestBase40Pitch -- Get lowest base 40 pitch that is not a rest or silent
//    TODO: Handle negative values and sustained notes
//

int Tool_fb::getLowestBase40Pitch(vector<int> base40Pitches) {
	vector<int> filteredBase40Pitches;
	copy_if(base40Pitches.begin(), base40Pitches.end(), std::back_inserter(filteredBase40Pitches), [](int base40Pitch) {
		// Ignore if base is a rest or silent note
		return (base40Pitch != -1000) && (base40Pitch != -2000) && (base40Pitch != 0);
	});

	if (filteredBase40Pitches.size() == 0) {
		return -2000;
	}

	return *min_element(begin(filteredBase40Pitches), end(filteredBase40Pitches));
}


//////////////////////////////
//
// FiguredBassNumber::FiguredBassNumber -- Constructor
//

FiguredBassNumber::FiguredBassNumber(int num, string accid, bool showAccid, int voiceIdx, int lineIdx, bool isAtk, bool intervallsatz) {
	m_number          = num;
	m_accidentals     = accid;
	m_voiceIndex      = voiceIdx;
	m_lineIndex       = lineIdx;
	m_showAccidentals = showAccid;
	m_isAttack        = isAtk;
	m_intervallsatz   = intervallsatz;
}



//////////////////////////////
//
// FiguredBassNumber::toString -- Convert FiguredBassNumber to a string (accidental + number)
//

string FiguredBassNumber::toString(bool compoundQ, bool accidentalsQ, bool hideThreeQ) {
	int num = (compoundQ) ? getNumberWithinOctave() : m_number;
	string accid = (accidentalsQ && m_showAccidentals) ? m_accidentals : "";
	if (((num == 3) || (num == -3)) && accidentalsQ && m_showAccidentals && hideThreeQ) {
		return accid;
	}
	if (num > 0) {
		return accid + to_string(num);
	}
	if (num < 0) {
		return accid + "~" + to_string(abs(num));
	}
	return "";
}



//////////////////////////////
//
// FiguredBassNumber::getNumberWithinOctave -- Get a reasonable figured bass number
//    Replace 0 with 7 and -7
//    Replace 1 with 8 and -8
//    Replace 2 with 9 if it is a suspension of the ninth
//    Allow 1 (unisono) in intervallsatz

int FiguredBassNumber::getNumberWithinOctave(void) {
	int num = m_number % 7;

	// Replace 0 with 7 and -7
	if ((abs(m_number) > 0) && (m_number % 7 == 0)) {
		return m_number < 0 ? -7 : 7;
	}

	// Replace 1 with 8 and -8
	if (abs(num) == 1) {
		// Allow unisono in intervallsatz
		if (m_intervallsatz) {
			if (abs(m_number) == 1) {
				return 1;
			}
		}
		return m_number < 0 ? -8 : 8;
	}

	// Replace 2 with 9 if m_convert2To9 is true (e.g. when a 3 is included in the chord numbers)
	if (m_convert2To9 && (num == 2)) {
		return 9;
	}

	return num;
}



//////////////////////////////
//
// FiguredBassAbbreviationMapping::FiguredBassAbbreviationMapping -- Constructor
//    Helper class to store the mappings for abbreviate figured bass numbers
//

FiguredBassAbbreviationMapping::FiguredBassAbbreviationMapping(string s, vector<int> n) {
	m_str = s;
	m_numbers = n;
}



//////////////////////////////
//
// FiguredBassAbbreviationMapping::s_mappings -- Mapping to abbreviate figured bass numbers
//

const vector<FiguredBassAbbreviationMapping> FiguredBassAbbreviationMapping::s_mappings = {
	FiguredBassAbbreviationMapping("3", {}),
	FiguredBassAbbreviationMapping("5", {}),
	FiguredBassAbbreviationMapping("5 3", {}),
	FiguredBassAbbreviationMapping("6 3", {6}),
	FiguredBassAbbreviationMapping("5 4", {4}),
	FiguredBassAbbreviationMapping("7 5 3", {7}),
	FiguredBassAbbreviationMapping("7 3", {7}),
	FiguredBassAbbreviationMapping("7 5", {7}),
	FiguredBassAbbreviationMapping("6 5 3", {6, 5}),
	FiguredBassAbbreviationMapping("6 4 3", {4, 3}),
	FiguredBassAbbreviationMapping("6 4 2", {4, 2}),
	FiguredBassAbbreviationMapping("9 5 3", {9}),
	FiguredBassAbbreviationMapping("9 5", {9}),
	FiguredBassAbbreviationMapping("9 3", {9}),
};

// END_MERGE

} // end namespace hum
