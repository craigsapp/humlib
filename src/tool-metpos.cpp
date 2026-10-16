//
// Programmer:    Alexander Morgan
// Creation Date: Fri Oct 16 2026
// Filename:      tool-metpos.cpp
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Append a **metpos spine giving the position of each data
//                line in the metric hierarchy (1 for the start of the
//                measure, larger numbers for weaker positions).  Based on
//                the Humdrum Toolkit metpos command by David Huron
//                (metpos.awk).  The original numbers the records of input
//                processed by timebase; here the metric position is
//                calculated from the duration of each line from the
//                barline, which gives the same values without timebase.
//                When a measure is longer than its time signature, the
//                barlines give the length of the measure, and the original
//                (*omet) or modern (*met) mensuration sign gives its duple
//                or triple organization.  Music without any meter, or with
//                measures of irregular length, is measured from each
//                (possibly hidden) barline with a semibreve pulse.
//

#include "tool-metpos.h"

#include <cstdlib>
#include <regex>

using namespace std;

namespace hum {

// START_MERGE

/////////////////////////////////
//
// Tool_metpos::Tool_metpos -- Set the recognized options for the tool.
//

Tool_metpos::Tool_metpos(void) {
	define("m|meter=s:", "initial meter signature, such as 3/4");
}



/////////////////////////////////
//
// Tool_metpos::run -- Do the main work of the tool.
//

bool Tool_metpos::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}



bool Tool_metpos::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	return run(infile, out);
}



bool Tool_metpos::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	if (hasAnyText()) {
		getAllText(out);
	} else {
		out << infile;
	}
	return status;
}



bool Tool_metpos::run(HumdrumFile& infile) {
	initialize();
	processFile(infile);
	return !hasError();
}



//////////////////////////////
//
// Tool_metpos::getMetricPositions -- Metric position of every line of a
//     file, as it would be printed in the **metpos spine, without printing
//     anything: positions is indexed by line, with 0 for non-data lines and
//     data lines that have no metric position (including the lines after
//     an error, which stays available from the error text).
//

void Tool_metpos::getMetricPositions(HumdrumFile& infile, vector<int>& positions) {
	initialize();
	processFile(infile);
	positions = m_lineValues;
	m_humdrum_text.str("");
}



//////////////////////////////
//
// Tool_metpos::initialize --
//

void Tool_metpos::initialize(void) {
	m_meterOption = getString("meter");
	m_signatureDuration = 0;
	m_signatureBeats = 0;
	m_measureDuration = 0;
	m_binaryQ = false;
	m_pulseQ = false;
	m_template = NULL;
	m_omet.clear();
	m_met.clear();
	m_firstBarYet = false;
	m_firstInterpretationYet = false;
	m_barStart = 0;
	m_barEnd = 0;
	m_nextBarTime.clear();
	m_anacrusis.clear();
	m_infile = NULL;
}



//////////////////////////////
//
// Tool_metpos::processFile -- Process the input one record at a time.
//     When an error stops the analysis, the records collected before the
//     first barline are still printed, as in the original.
//

void Tool_metpos::processFile(HumdrumFile& infile) {
	m_infile = &infile;
	m_lineValues.assign(infile.getLineCount(), 0);
	if (getBoolean("meter")) {
		if (!regex_match(m_meterOption, regex("^[1-9]+/[1-9]+[.]*$"))) {
			m_error_text << "metpos: ERROR: Invalid meter specified: "
			             << m_meterOption << endl;
			return;
		}
	}

	// Time of the next barline (or the end of the music) after each line.
	m_nextBarTime.resize(infile.getLineCount());
	HumNum next = infile.getScoreDuration();
	for (int i=infile.getLineCount()-1; i>=0; i--) {
		m_nextBarTime[i] = next;
		if (infile[i].isBarline()) {
			next = infile[i].getDurationFromStart();
		}
	}

	bool ok = true;
	if (!m_meterOption.empty()) {
		ok = setMeter(m_meterOption, 0);
	}

	for (int i=0; ok && (i<infile.getLineCount()); i++) {
		ok = processLine(infile, i);
	}

	if (!m_anacrusis.empty()) {
		printAnacrusis(infile.getScoreDuration());
	}
}



//////////////////////////////
//
// Tool_metpos::processLine -- Returns false when an error stops the analysis.
//

bool Tool_metpos::processLine(HumdrumFile& infile, int index) {
	const string& line = infile[index];
	vector<string> fields;
	size_t start = 0;
	while (true) {
		size_t tab = line.find('\t', start);
		if (tab == string::npos) {
			fields.push_back(line.substr(start));
			break;
		}
		fields.push_back(line.substr(start, tab - start));
		start = tab + 1;
	}

	if (line.compare(0, 2, "!!") == 0) {
		emit(line);
		return true;
	}
	if (line.compare(0, 1, "!") == 0) {
		emit(line + "\t!");
		return true;
	}
	if (line.compare(0, 1, "*") == 0) {
		return processInterpretation(infile, index, fields);
	}
	if (fields[0].find('=') != string::npos) {
		processBarline(infile, index, fields);
		return true;
	}
	return processData(infile, index);
}



//////////////////////////////
//
// Tool_metpos::processInterpretation --
//

bool Tool_metpos::processInterpretation(HumdrumFile& infile, int index,
		const vector<string>& fields) {
	static const regex tandemRe("tb[0-9]|M[0-9]");
	const string& line = infile[index];

	for (const string& field : fields) {
		if ((field.compare(0, 6, "*omet(") == 0) && (field.back() == ')')) {
			m_omet = field.substr(6, field.size() - 7);
		} else if ((field.compare(0, 5, "*met(") == 0) && (field.back() == ')')) {
			m_met = field.substr(5, field.size() - 6);
		}
	}

	bool allExclusive = true;  // every field is **name
	bool exclusiveOrNull = true;  // every field is **name or *
	bool allNull = true;  // every field is *
	bool allTerminators = true;  // every field is *-
	for (const string& field : fields) {
		bool exclusive = (field.size() > 2) && (field.compare(0, 2, "**") == 0);
		bool null = (field == "*");
		if (!exclusive) {
			allExclusive = false;
		}
		if (!(exclusive || null)) {
			exclusiveOrNull = false;
		}
		if (!null) {
			allNull = false;
		}
		if (field != "*-") {
			allTerminators = false;
		}
	}

	if (allExclusive) {
		emit(line + "\t**metpos");
		m_firstInterpretationYet = true;
		m_firstBarYet = false;
		printAnacrusis(infile[index].getDurationFromStart());
		m_omet.clear();
		m_met.clear();
	} else if (exclusiveOrNull && !allNull) {
		// A new exclusive interpretation after spine additions.
		if (!m_firstInterpretationYet) {
			emit(line + "\t**metpos");
			m_firstInterpretationYet = true;
		} else {
			emit(line + "\t*");
		}
	} else if (allTerminators) {
		emit(line + "\t*-");
		m_firstInterpretationYet = false;
		m_firstBarYet = false;
		printAnacrusis(infile[index].getDurationFromStart());
	} else if (regex_search(line, tandemRe)) {
		return processTandem(infile, index, fields);
	} else {
		emit(line + "\t*");
	}
	return true;
}



//////////////////////////////
//
// Tool_metpos::processTandem -- Store meter (*M) interpretations, copying
//     the first meter or timebase (*tb) interpretation into the **metpos
//     spine.  The original printed the line once for every spine that
//     contained one (so usually several times), and dropped lines such as
//     *MM120 that contained none; here each line is printed once.
//

bool Tool_metpos::processTandem(HumdrumFile& infile, int index,
		const vector<string>& fields) {
	static const regex timebaseRe("^\\*tb[0-9]+$");
	static const regex meterRe("^\\*M(([1-9]+((\\+)([1-9]+))*(\\/)([0-9]+)(\\.)*)|(\\?)|(X))$");
	int lineNumber = index + 1;

	string suffix;
	string meter;
	for (int j=0; j<(int)fields.size(); j++) {
		bool timebaseQ = regex_match(fields[j], timebaseRe);
		bool meterQ = !timebaseQ && regex_match(fields[j], meterRe);
		if (meterQ) {
			if (meter.empty()) {
				meter = fields[j];
			} else if (fields[j] != meter) {
				m_error_text << "metpos: ERROR: Different meter signatures in "
				             << "concurrent spines at line: " << lineNumber << endl;
				return false;
			}
		}
		if ((timebaseQ || meterQ) && suffix.empty()) {
			suffix = fields[j];
		}
	}
	emit(infile[index] + "\t" + (suffix.empty() ? "*" : suffix));

	if (!meter.empty()) {
		return setMeter(meter.substr(2), lineNumber);
	}
	return true;
}



//////////////////////////////
//
// Tool_metpos::processBarline -- The **metpos spine copies the barline
//     of the first spine.  A final barline (==) does not need a complete
//     measure, and ends the measure numbering so that following data are
//     treated as a new anacrusis.
//

void Tool_metpos::processBarline(HumdrumFile& infile, int index,
		const vector<string>& fields) {
	HumNum time = infile[index].getDurationFromStart();
	bool finalQ = fields[0].find("==") != string::npos;
	if (!m_firstBarYet) {
		printAnacrusis(time);
		m_firstBarYet = !finalQ;
	} else if (time - m_barStart < m_measureDuration) {
		if (!finalQ) {
			m_warning_text << "metpos: WARNING: Unexpected barline encountered in line "
			               << (index + 1) << ".\n                 Measure is shorter "
			               << "than the meter signature." << endl;
		} else {
			m_firstBarYet = false;
			printAnacrusis(time);
		}
	} else if (finalQ) {
		m_firstBarYet = false;
		printAnacrusis(time);
	}
	m_humdrum_text << infile[index] << "\t" << fields[0] << "\n";
	m_barStart = time;
	m_barEnd = m_nextBarTime.at(index);
}



//////////////////////////////
//
// Tool_metpos::processData --
//

bool Tool_metpos::processData(HumdrumFile& infile, int index) {
	if (!m_firstBarYet) {
		m_anacrusis.emplace_back(infile[index], index);
		return true;
	}
	updateMeasureHierarchy(m_barEnd - m_barStart);
	HumNum position = infile[index].getDurationFromStart() - m_barStart;
	if ((m_measureDuration > 0) && (position >= m_measureDuration)) {
		m_error_text << "metpos: ERROR: Measure is longer than the meter signature "
		             << "in input line: " << (index + 1) << endl;
		return false;
	}
	m_humdrum_text << infile[index] << "\t" << getPositionText(index, position) << "\n";
	return true;
}



//////////////////////////////
//
// Tool_metpos::emit -- Print a non-data record, or store it until the first
//     barline when it comes before the first barline.
//

void Tool_metpos::emit(const string& text) {
	if (m_firstBarYet) {
		m_humdrum_text << text << "\n";
	} else {
		m_anacrusis.emplace_back(text, -1);
	}
}



//////////////////////////////
//
// Tool_metpos::printAnacrusis -- Print the stored records.  Data records
//     are aligned with the end of a measure that ends at endTime.  When
//     they last longer than the time signature (or there is none), they
//     form a measure of their own instead, which starts with the first
//     stored data record (see updateMeasureHierarchy()).
//

void Tool_metpos::printAnacrusis(HumNum endTime) {
	HumNum measureStart = -1;
	for (auto& record : m_anacrusis) {
		if (record.second >= 0) {
			measureStart = (*m_infile)[record.second].getDurationFromStart();
			break;
		}
	}
	if ((m_signatureDuration > 0) && (endTime - measureStart <= m_signatureDuration)) {
		useHierarchy(m_signatureBeats, m_signatureDuration);
		measureStart = -1;
	} else {
		updateMeasureHierarchy(endTime - measureStart);
	}
	for (auto& record : m_anacrusis) {
		if (record.second < 0) {
			m_humdrum_text << record.first << "\n";
			continue;
		}
		HumNum start = (*m_infile)[record.second].getDurationFromStart();
		HumNum position;
		if (measureStart >= 0) {
			position = start - measureStart;
		} else {
			position = m_measureDuration - (endTime - start);
		}
		m_humdrum_text << record.first << "\t" << getPositionText(record.second, position) << "\n";
	}
	m_anacrusis.clear();
}



//////////////////////////////
//
// Tool_metpos::setMeter -- Set the meter for getMetricPosition(), such as
//     "3/4" or "4/2" (without "*M").  Meters with 2, 3, 4, 6, 9 or 12 beats
//     are handled; other meters give no metric positions, and irregular
//     meters (5, 7, 10, 11 or more than 12 beats) are an error.
//

bool Tool_metpos::setMeter(const string& meter, int lineNumber) {
	m_signatureDuration = 0;
	m_signatureBeats = 0;
	useHierarchy(0, 0);

	size_t slash = meter.find('/');
	if (slash == string::npos) {
		return true;
	}
	string numerator = meter.substr(0, slash);
	string denominator = meter.substr(meter.rfind('/') + 1);
	if (numerator.find_first_not_of("0123456789") != string::npos) {
		return true;
	}
	int top = atoi(numerator.c_str());
	int bottom = atoi(denominator.c_str());
	if (denominator.find('.') != string::npos) {
		// Double-dotted denominators are not handled correctly (as in the
		// original).
		top *= 3;
		bottom *= 2;
	}

	if ((top == 5) || (top == 7) || (top == 10) || (top == 11) || (top >= 13)) {
		m_error_text << "metpos: ERROR:  Cannot handle irregular meter: " << meter
		             << " in line " << lineNumber << "." << endl;
		return false;
	}
	if (bottom <= 0) {
		return true;
	}
	m_signatureDuration = HumNum(top * 4, bottom);
	m_signatureBeats = top;
	useHierarchy(m_signatureBeats, m_signatureDuration);
	return true;
}



//////////////////////////////
//
// Tool_metpos::useHierarchy -- Make getMetricPosition() use a measure of
//     the given duration (in quarter notes) organized like a meter with the
//     given number of beats: binary for 2 or 4 beats, or the template of
//     the original for 3, 6, 9 or 12 beats.  A binary measure whose length
//     is not a power of two (such as 5 semibreves) uses a semibreve pulse
//     instead.  Other values give no metric positions.
//

void Tool_metpos::useHierarchy(int beats, HumNum duration) {
	static const vector<int> simpleTriple      = makeTemplate(96,  7, {2, 4, 8, 16, 32});
	static const vector<int> compoundDuple     = makeTemplate(96,  7, {2, 4, 8, 16, 48});
	static const vector<int> compoundTriple    = makeTemplate(108, 6, {2, 4, 12, 36});
	static const vector<int> compoundQuadruple = makeTemplate(96,  7, {2, 4, 8, 24, 48});

	m_measureDuration = duration;
	m_binaryQ = false;
	m_pulseQ = false;
	m_template = NULL;
	switch (beats) {
		case 2:
		case 4:
			if (duration.isPowerOfTwo()) {
				m_binaryQ = true;
			} else {
				usePulse(duration);
			}
			break;
		case 3:  m_template = &simpleTriple;        break;
		case 6:  m_template = &compoundDuple;       break;
		case 9:  m_template = &compoundTriple;      break;
		case 12: m_template = &compoundQuadruple;   break;
	}
}



//////////////////////////////
//
// Tool_metpos::updateMeasureHierarchy -- Choose the hierarchy of a measure
//     with the given length from the barlines.  The time signature is used
//     unless the measure is longer than it (or there is no time signature,
//     or it has no length, such as *M?).  Then the measure has the length
//     from the barlines, and its organization comes from the original
//     mensuration sign (*omet), then the modern one (*met), then the time
//     signature, and otherwise a semibreve pulse.
//

void Tool_metpos::updateMeasureHierarchy(HumNum length) {
	if ((length <= 0) || ((m_signatureDuration > 0) && (length <= m_signatureDuration))) {
		useHierarchy(m_signatureBeats, m_signatureDuration);
		return;
	}
	int beats = getSignBeats(m_omet);
	if (beats == 0) {
		beats = getSignBeats(m_met);
	}
	if (beats == 0) {
		beats = m_signatureBeats;
	}
	if (beats == 0) {
		usePulse(length);
		return;
	}
	useHierarchy(beats, length);
}



//////////////////////////////
//
// Tool_metpos::usePulse -- Make getMetricPosition() use a semibreve pulse
//     in a measure of the given duration (in quarter notes), for measures
//     without a regular metric organization.
//

void Tool_metpos::usePulse(HumNum duration) {
	m_measureDuration = duration;
	m_binaryQ = false;
	m_pulseQ = true;
	m_template = NULL;
}



//////////////////////////////
//
// Tool_metpos::getSignBeats -- Organization of a measure implied by a
//     mensuration sign (the text inside *omet() or *met()), given as the
//     number of beats of the equivalent meter for useHierarchy():
//        3 for proportions (signs with a 3) and tempus perfectum (O)
//        9 for tempus perfectum with prolatio maior (O.)
//        6 for prolatio maior (C.)
//        2 for C, C| and c (duple at every level)
//     Returns 0 for other signs, which do not give the organization.
//

int Tool_metpos::getSignBeats(const string& sign) {
	bool threeQ   = sign.find('3') != string::npos;
	bool perfectQ = sign.find_first_of("Oo") != string::npos;
	bool dotQ     = sign.find('.') != string::npos;
	bool cQ       = sign.find_first_of("Cc") != string::npos;
	if (threeQ) {
		return 3;
	}
	if (perfectQ) {
		return dotQ ? 9 : 3;
	}
	if (cQ) {
		return dotQ ? 6 : 2;
	}
	return 0;
}



//////////////////////////////
//
// Tool_metpos::getGridPosition -- Metric position of a time (in quarter
//     notes) from the start of the measure in the meter given to setMeter():
//     1 for the start of the measure, and larger numbers for weaker
//     positions.  Returns 0 when the position is not in the metric
//     hierarchy, such as a triplet in a duple meter (see
//     getMetricPosition() for those).
//
//     The measure is the one from setMeter(), or, inside the score, the
//     current measure (see updateMeasureHierarchy()).
//
//     In 2-beat and 4-beat simple meters, the hierarchy is binary: a
//     position that is a/b of the measure (in lowest terms) is
//     1 + log2(b).  Other meters look up the position in a template of the
//     measure with 96 (or 108 for 9 beats) slots.  With a semibreve pulse
//     (see usePulse()), every semibreve after the start of the measure is
//     2, other minims 3, other crotchets 4, and so on, as in a 4/2 measure.
//

int Tool_metpos::getGridPosition(HumNum position) {
	if (m_measureDuration <= 0) {
		return 0;
	}
	HumNum fraction = position / m_measureDuration;
	if ((fraction < 0) || (fraction >= 1)) {
		return 0;
	}
	if (m_pulseQ) {
		if (position == 0) {
			return 1;
		}
		// Find the largest power-of-two duration that position is a
		// multiple of: 2^exponent quarter notes.
		int numerator = position.getNumerator();
		int denominator = position.getDenominator();
		int exponent = 0;
		while (denominator > 1) {
			if (denominator % 2) {
				return 0;
			}
			denominator /= 2;
			exponent--;
		}
		while ((numerator % 2 == 0) && (exponent < 2)) {
			numerator /= 2;
			exponent++;
		}
		// A semibreve (2^2 quarter notes) or more is 2.
		return 4 - exponent;
	}
	if (m_binaryQ) {
		int denominator = fraction.getDenominator();
		int value = 1;
		while (denominator > 1) {
			if (denominator % 2) {
				return 0;
			}
			denominator /= 2;
			value++;
		}
		return value;
	}
	if (m_template) {
		HumNum slot = fraction * ((int)m_template->size() - 2);
		if (!slot.isInteger()) {
			return 0;
		}
		return m_template->at(slot.getNumerator() + 1);
	}
	return 0;
}



//////////////////////////////
//
// Tool_metpos::getMetricPosition -- Metric position of a time (in quarter
//     notes) from the start of the measure (see getGridPosition()).
//     Positions off the metric hierarchy that belong to a triplet get the
//     metric position of the duple note value that the triplet replaces,
//     which is 3/2 of the triplet note value: in 4/4, the inner notes of
//     half-note triplets get 2 like half notes, those of quarter-note
//     triplets 3 like quarter notes, and those of eighth-note triplets 4
//     like eighth notes.  A position can belong to triplets of several
//     sizes (2/3 of a quarter note is in both eighth-note and quarter-note
//     triplets), so the triplet note value is the largest one that fits
//     the position and is no longer than duration, the shortest note value
//     starting at that time (0 for no limit).  Other positions, such as
//     quintuplets or triplets within triplets, return 0.
//

int Tool_metpos::getMetricPosition(HumNum position, HumNum duration) {
	int value = getGridPosition(position);
	if ((value > 0) || (position <= 0)) {
		return value;
	}

	// The position is n/(3 * 2^x) quarter notes with n not a multiple of 3.
	int numerator = position.getNumerator();
	int denominator = position.getDenominator();
	if (denominator % 3) {
		return 0;
	}
	denominator /= 3;
	int x = 0;
	while (denominator > 1) {
		if (denominator % 2) {
			return 0;
		}
		denominator /= 2;
		x++;
	}

	// Triplet note value 2^k/3 quarter notes: the position must be a
	// multiple of it, and it must be no longer than duration.
	int k = -x;
	while (numerator % 2 == 0) {
		numerator /= 2;
		k++;
	}
	if (duration > 0) {
		while (HumNum(1 << max(k, 0), 3 << max(-k, 0)) > duration) {
			k--;
			if (k < -16) {
				return 0;
			}
		}
	}

	// The duple note value replaced by the triplet: 2^(k-1) quarter notes.
	HumNum replaced = (k >= 1) ? HumNum(1 << (k - 1), 1) : HumNum(1, 1 << (1 - k));
	return getGridPosition(replaced);
}



//////////////////////////////
//
// Tool_metpos::getShortestDuration -- Shortest note or rest value (as
//     written, not including ties) starting on a data line, or 0 when
//     there is none.
//

HumNum Tool_metpos::getShortestDuration(HumdrumFile& infile, int index) {
	HumNum output = 0;
	for (int j=0; j<infile[index].getFieldCount(); j++) {
		HTp token = infile.token(index, j);
		if (!token->isKern() || token->isNull()) {
			continue;
		}
		HumNum duration = token->getDuration();
		if ((duration > 0) && ((output == 0) || (duration < output))) {
			output = duration;
		}
	}
	return output;
}



//////////////////////////////
//
// Tool_metpos::getPositionText -- Metric position of the data line at index
//     (position quarter notes from the start of its measure) as printed in
//     the **metpos spine, or "." when there is none.  The value is also
//     stored for getMetricPositions().
//

string Tool_metpos::getPositionText(int index, HumNum position) {
	int value = getMetricPosition(position, getShortestDuration(*m_infile, index));
	m_lineValues.at(index) = max(value, 0);
	if (value <= 0) {
		return ".";
	}
	return to_string(value);
}



//////////////////////////////
//
// Tool_metpos::makeTemplate -- Metric hierarchy of a measure divided into
//     size equal slots (indexed from 1), as built by the original.  The
//     finest slots get base, odd slots get base-1, and the slot after each
//     multiple of divisors[d] (d >= 1) gets base-1-d.  Slot 1 is 1.  The
//     vector has two extra entries, and 0 marks unset entries.
//

vector<int> Tool_metpos::makeTemplate(int size, int base, const vector<int>& divisors) {
	vector<int> output(size + 2, 0);
	for (int i=1; i<=size; i++) {
		output[i] = base;
	}
	for (int i=2; i<=size; i+=2) {
		output[i-1] = base - 1;
	}
	for (int d=1; d<(int)divisors.size(); d++) {
		for (int i=1; i<=size; i++) {
			if (i % divisors[d] == 0) {
				output[i+1] = base - 1 - d;
			}
		}
	}
	output[1] = 1;
	return output;
}

// END_MERGE

} // end namespace hum


