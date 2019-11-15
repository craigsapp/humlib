//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 29 11:38:01 CEST 2019
// Last Modified: Sat Aug  3 17:48:04 EDT 2019
// Filename:      tool-humdiff.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-humdiff.h
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Interface for humdiff (similarity matrix) tool.
//

#ifndef _TOOL_HUMDIFF_H_INCLUDED
#define _TOOL_HUMDIFF_H_INCLUDED

#include "HumTool.h"
#include "HumdrumFile.h"
#include "HumdrumFileSet.h"

#include <iostream>

namespace hum {

// START_MERGE

// A TimePoint records the event times in a file.  These are positions of note attacks
// in the file.  The "index" variable keeps track of the line in the original file
// (for the first position in index), and other positions in index keep track of the
// equivalent line position of the timepoint in other file(s) that are being compared.
class TimePoint {
	public:
		// file: pointers to the file in which the index refers to
		vector<HumdrumFile*> file;

		// index :: A list of indexes for the lines at which the given timestamp
		// occurs in each file.  The first index is for the reference work.
		vector<int> index;

		// timestamp :: The duration from the start of the score to given time in score.
		HumNum timestamp = -1;

		// measure :: The measure number in which the timestamp occurs.
		int measure = -1;

		void clear(void) {
			file.clear();
			index.clear();
			timestamp = -1;
			measure = -1;
		}
};


// NotePoint is a note from a score that will be matches hopefully to an
// equivalent note in another score.
class NotePoint {
	public:
		HTp         token          = NULL;   // Humdrum token that contains note
		string      subtoken;                // string that represents not (token may be chord)
		int         subindex       = -1;     // subtoken index of note (in chord)
		int         measure        = -1;     // measure number that note is found
		HumNum      measurequarter = -1;     // distance from start of measure to note
		int         track          = -1;     // track that note is from
		int         layer          = -1;     // layer that note is in
		HumNum      duration       = -1;     // duration (tied) of note
		int         b40            = -1;     // b40 pitch of note
		int         processed      = 0;      // has note been processed/matched
		int         sourceindex    = -1;     // source file index for note
		int         tpindex        = -1;     // timepoint index of note in source
		vector<int> matched;       // indexes to the location of the note in TimePoint list.
		                           // the index indicate which score the match is related to,
		                           // and a value of -1 means there is no equivalent timepoint.
		void clear(void) {
			token = NULL;
			subtoken = "";
			subindex = -1;
			measure = -1;
			measurequarter = -1;
			track = -1;
			layer = -1;
			duration = -1;
			b40 = -1;
			processed = 0;
			sourceindex = -1;
			tpindex = -1;
			matched.clear();
		}
};


// Function declarations:

class Tool_humdiff : public HumTool {
	public:
		         Tool_humdiff       (void);

		bool     run                (HumdrumFileSet& infiles);

	protected:
		void     compareFiles       (HumdrumFile& reference, HumdrumFile& alternate);

		void     compareTimePoints  (vector<vector<TimePoint>>& timepoints, HumdrumFile& reference, HumdrumFile& alternate);
		void     extractTimePoints  (vector<TimePoint>& points, HumdrumFile& infile);
		void     printTimePoints    (vector<TimePoint>& timepoints);
		void     compareLines       (HumNum minval, vector<int>& indexes, vector<vector<TimePoint>>& timepoints, vector<HumdrumFile*> infiles);
		void     getNoteList        (vector<NotePoint>& notelist, HumdrumFile& infile, int line, int measure, int sourceindex, int tpindex);
		int      findNoteInList     (NotePoint& np, vector<NotePoint>& nps);
		void     printNotePoints    (vector<NotePoint>& notelist);
		void     markNote           (NotePoint& np);

	private:
		int m_marked = 0;


};

ostream& operator<<(ostream& out, TimePoint& tp);
ostream& operator<<(ostream& out, NotePoint& np);


// END_MERGE

} // end namespace hum

#endif /* _TOOL_HUMDIFF_H_INCLUDED */



