//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Oct 16 16:08:05 PDT 2016
// Last Modified: Sun Oct 16 16:08:08 PDT 2016
// Filename:      HumGrid.h
// URL:           https://github.com/craigsapp/hum2ly/blob/master/include/HumGrid.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   HumGrid is an intermediate container for converting from
//                MusicXML syntax into Humdrum syntax.
//

#ifndef _HUMGRID_H
#define _HUMGRID_H

#include "GridMeasure.h"
#include "GridSlice.h"

#include <vector>

namespace hum {

// START_MERGE

class HumGrid : public std::vector<GridMeasure*> {
	public:
		HumGrid(void);
		~HumGrid();
		void enableRecipSpine           (void);
		bool transferTokens             (HumdrumFile& outfile, int startbarnum = 0);
		int  getHarmonyCount            (int partindex);
		int  getDynamicsCount           (int partindex);
		int  getFiguredBassCount        (int partindex);
		int  getVerseCount              (int partindex, int staffindex);
		bool hasDynamics                (int partindex);
		bool hasFiguredBass             (int partindex);
		void setDynamicsPresent         (int partindex);
		void setFiguredBassPresent      (int partindex);
		void setHarmonyPresent          (int partindex);
		void setVerseCount              (int partindex, int staffindex, int count);
		void reportVerseCount           (int partindex, int staffindex, int count);
		void setHarmonyCount            (int partindex, int count);
		void removeRedundantClefChanges (void);
		void removeSibeliusIncipit      (void);
		bool hasPickup                  (void);
		GridMeasure*  addMeasureToBack  (void);
		int  getPartCount               (void);
		int  getStaffCount              (int partindex);
		void deleteMeasure              (int index);
		void setPartName                (int index, const string& name);
		std::string getPartName         (int index);
		void addInvisibleRestsInFirstTrack(void);
		void setPartStaffDimensions     (std::vector<std::vector<GridSlice*>>& nextevent,
		                                 GridSlice* startslice);
		void addInvisibleRest           (std::vector<std::vector<GridSlice*>>& nextevent,
		                                 int index, int p, int s);
		void cleanTempos                (void);
		void cleanTempos                (GridSlice* slice);

	protected:
		void calculateGridDurations        (void);
		void insertExclusiveInterpretationLine (HumdrumFile& outfile);
		void insertDataTerminationLine     (HumdrumFile& outfile);
		void appendMeasureLine             (HumdrumFile& outfile,
		                                    GridSlice& slice);
		void insertPartIndications         (HumdrumFile& outfile);
		void insertStaffIndications        (HumdrumFile& outfile);
		void insertPartNames               (HumdrumFile& outfile);
		void addNullTokens                 (void);
		void addNullTokensForGraceNotes    (void);
		void addNullTokensForClefChanges   (void);
		void addNullTokensForLayoutComments(void);

		void fillInNullTokensForGraceNotes(GridSlice* graceslice, GridSlice* lastnote,
		                                   GridSlice* nextnote);
		void fillInNullTokensForLayoutComments(GridSlice* layoutslice, GridSlice* lastnote,
		                                   GridSlice* nextnote);
		void fillInNullTokensForClefChanges (GridSlice* clefslice,
		                                    GridSlice* lastnote, GridSlice* nextnote);
		void adjustClefChanges             (void);
		bool buildSingleList               (void);
		void extendDurationToken           (int slicei, int parti,
		                                    int staffi, int voicei);
		GridVoice* getGridVoice(int slicei, int parti, int staffi, int voicei);
		void addMeasureLines               (void);
		void addLastMeasure                (void);
		bool manipulatorCheck              (void);
		GridSlice* manipulatorCheck        (GridSlice* ice1, GridSlice* ice2);
		void cleanupManipulators           (void);
		void cleanManipulator              (std::vector<GridSlice*>& newslices,
		                                    GridSlice* curr);
		GridSlice* checkManipulatorExpand  (GridSlice* curr);
		GridSlice* checkManipulatorContract(GridSlice* curr);
		void transferMerges                (GridStaff* oldstaff,
		                                    GridStaff* oldlaststaff,
		                                    GridStaff* newstaff,
		                                    GridStaff* newlaststaff, int pindex,
		                                    int sindex);
		void transferOtherParts            (GridSlice* oldline, GridSlice* newline, int maxpart);
		void insertExInterpSides           (HumdrumLine* line, int part,
		                                    int staff);
		void insertSideTerminals           (HumdrumLine* line, int part,
		                                    int staff);
		void insertSidePartInfo            (HumdrumLine* line, int part,
		                                    int staff);
		void insertSideStaffInfo           (HumdrumLine* line, int part,
		                                    int staff, int staffnum);
		void insertSideNullInterpretations (HumdrumLine* line,
		                                    int part, int staff);
		void getMetricBarNumbers           (std::vector<int>& barnums);
		string  createBarToken             (int m, int barnum,
		                                    GridMeasure* measure);
		string getBarStyle                 (GridMeasure* measure);
		void adjustExpansionsInStaff       (GridSlice* newmanip, GridSlice* curr,
		                                    int p, int s);
		void transferNonDataSlices         (GridMeasure* output, GridMeasure* input);
		string extractMelody               (GridMeasure* measure);
		void insertMelodyString            (GridMeasure* measure, const string& melody);
		GridVoice* createVoice             (const string& tok, const string& post, HumNum duration, int pindex, int sindex);
		HTp createHumdrumToken             (const string& tok, int pindex, int sindex);
		GridSlice* getNextSpinedLine       (const GridMeasure::iterator& it, int measureindex);
		void matchVoices                   (GridSlice* current, GridSlice* last);
		void adjustVoices                  (GridSlice* curr, GridSlice* newmanip, int partsplit);
		void createMatchedVoiceCount       (GridStaff* snew, GridStaff* sold, int p, int s);

	private:
		std::vector<GridSlice*>       m_allslices;
		std::vector<std::vector<int>> m_verseCount;
		std::vector<int>              m_harmonyCount;
		bool                          m_pickup;
		std::vector<bool>             m_dynamics;
		std::vector<bool>             m_figured_bass;
		std::vector<bool>             m_harmony;

		std::vector<std::string>      m_partnames;

		// options:
		bool m_recip;               // include **recip spine in output
		bool m_musicxmlbarlines;    // use measure numbers from <measure> element

};

ostream& operator<<(ostream& out, HumGrid& grid);


// END_MERGE

} // end namespace hum

#endif /* _HUMGRID_H */



