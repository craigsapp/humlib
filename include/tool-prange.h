//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 18 11:23:42 PDT 2005
// Last Modified: Sat Jun  8 17:37:39 PDT 2024 (ported to humlib)
// Filename:      include/tool-prange.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-prange.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   C++ implementation of the Humdrum Toolkit prange command.
//

#ifndef _TOOL_PRANGE_H
#define _TOOL_PRANGE_H

#include "HumTool.h"
#include "HumdrumFileSet.h"

#include <map>
#include <ostream>
#include <string>
#include <sstream>
#include <vector>

namespace hum {

// START_MERGE


class _VoiceInfo {
	public:
		std::vector<std::vector<double>>   diatonic;
		std::vector<double>                midibins;
		std::string                        name;      // name for instrument name of spine
		std::string                        abbr;      // abbreviation for instrument name of spine
		int                                track;     // track number for spine
		bool                               kernQ;     // is spine a **kern spine?
		double                             hpos;      // horiz. position on system for pitch range data for spine
		std::vector<int>                   diafinal;  // finalis note diatonic pitch (4=middle-C octave)
		std::vector<int>                   accfinal;  // finalis note accidental (0 = natural)
		std::vector<std::string>           namfinal;  // name of voice for finalis note (for "all" display)
		int                                index = -1;

	public:
		                _VoiceInfo        (void);
		void            clear             (void);
		std::ostream&   print             (std::ostream& out);

};



class Tool_prange : public HumTool {
	public:
		         Tool_prange       (void);
		        ~Tool_prange       () {};

		bool        run               (HumdrumFileSet& infiles);
		bool        run               (HumdrumFile& infile);
		bool        run               (const std::string& indata, std::ostream& out);
		bool        run               (HumdrumFile& infile, std::ostream& out);

	protected:
		void        processFile         (HumdrumFile& infile);
		void        initialize          (void);

		void        mergeAllVoiceInfo           (std::vector<_VoiceInfo>& voiceInfo);
		void        getVoiceInfo                (std::vector<_VoiceInfo>& voiceInfo, HumdrumFile& infile);
		std::string getHand                     (HTp sstart);
		void        fillHistograms              (std::vector<_VoiceInfo>& voiceInfo, HumdrumFile& infile);
		void        printFilenameBase           (std::ostream& out, const std::string& filename);
		void        printReferenceRecords       (std::ostream& out, HumdrumFile& infile);
		void        printScoreEncodedText       (std::ostream& out, const std::string& strang);
		void        printXmlEncodedText         (std::ostream& out, const std::string& strang);
		void        printScoreFile              (std::ostream& out, std::vector<_VoiceInfo>& voiceInfo, HumdrumFile& infile);
		void        printKeySigCompression      (std::ostream& out, int keysig, int extra);
		void        assignHorizontalPosition    (std::vector<_VoiceInfo>& voiceInfo, int minval, int maxval);
		int         getKeySignature             (HumdrumFile& infile);
		void        printScoreVoice             (std::ostream& out, _VoiceInfo& voiceInfo, double maxvalue);
		void        printDiatonicPitchName      (std::ostream& out, int base7, int acc);
		std::string getDiatonicPitchName        (int base7, int acc);
		void        printHtmlStringEncodeSimple (std::ostream& out, const std::string& strang);
		std::string getNoteTitle                (double value, int diatonic, int acc);
		int         getDiatonicInterval         (int note1, int note2);
		int         getTopQuartile              (std::vector<double>& midibins);
		int         getBottomQuartile           (std::vector<double>& midibins);
		double      getMaxValue                 (std::vector<std::vector<double>>& bins);
		double      getVpos                     (double base7);
		int         getStaffBase7               (int base7);
		int         getMaxDiatonicIndex         (std::vector<std::vector<double>>& diatonic);
		int         getMinDiatonicIndex         (std::vector<std::vector<double>>& diatonic);
		int         getMinDiatonicAcc           (std::vector<std::vector<double>>& diatonic, int index);
		int         getMaxDiatonicAcc           (std::vector<std::vector<double>>& diatonic, int index);
		std::string getTitle                    (void);
		void        clearHistograms             (std::vector<std::vector<double> >& bins, int start);
		void        printAnalysis               (std::ostream& out, std::vector<double>& midibins);
		int         getMedian12                 (std::vector<double>& midibins);
		double      getMean12                   (std::vector<double>& midibins);
		int         getTessitura                (std::vector<double>& midibins);
		double      countNotesInRange           (std::vector<double>& midibins, int low, int high);
		void        printPercentile             (std::ostream& out, std::vector<double>& midibins, double percentile);
		void        getRange                    (int& rangeL, int& rangeH, const std::string& rangestring);
		void        mergeFinals                 (std::vector<_VoiceInfo>& voiceInfo,
		                                         std::vector<std::vector<int>>& diafinal,
		                                         std::vector<std::vector<int>>& accfinal);
		void        getInstrumentNames          (std::vector<std::string>& nameByTrack,
		                                         std::vector<int>& kernSpines, HumdrumFile& infile);
		void        printEmbeddedScore          (std::ostream& out, std::stringstream& scoredata, HumdrumFile& infile);
		void        prepareRefmap               (HumdrumFile& infile);
		int         getMaxStaffPosition         (std::vector<_VoiceInfo>& voiceinfo);
		int         getPrangeId                 (HumdrumFile& infile);
		void        processStrandNotes          (HTp sstart, HTp send,
		                                         std::vector<std::vector<std::pair<HTp, int>>>& trackInfo);
		void        fillMidiInfo                (HumdrumFile& infile);
		void        doExtremaMarkup             (HumdrumFile& infile);
		void        applyMarkup                 (std::vector<std::pair<HTp, int>>& notelist,
		                                         const std::string& mark);

	private:

		bool m_accQ         = false; // for --acc option
		bool m_addFractionQ = false; // for --fraction option
		bool m_allQ         = false; // for --all option
		bool m_debugQ       = false; // for --debug option
		bool m_defineQ      = false; // for --score option (use text macro)
		bool m_diatonicQ    = false; // for --diatonic option
		bool m_durationQ    = false; // for --duration option
		bool m_embedQ       = false; // for --embed option
		bool m_fillOnlyQ    = false; // for --fill option
		bool m_finalisQ     = false; // for --finalis option
		bool m_hoverQ       = false; // for --hover option
		bool m_instrumentQ  = false; // for --instrument option
		bool m_keyQ         = true;  // for --no-key option
		bool m_listQ        = false;
		bool m_localQ       = false; // for --local-maximum option
		bool m_normQ        = false; // for --norm option
		bool m_notitleQ     = false; // for --no-title option
		bool m_percentileQ  = false; // for --percentile option
		bool m_pitchQ       = false; // for --pitch option
		bool m_printQ       = false; // for --print option
		bool m_quartileQ    = false; // for --quartile option
		bool m_rangeQ       = false; // for --range option
		bool m_reverseQ     = false; // for --reverse option
		bool m_scoreQ       = false; // for --score option
		bool m_titleQ       = false; // for --title option
		bool m_extremaQ     = false; // for --extrema option

		std::string m_highMark = "🌸";
		std::string m_lowMark  = "🟢";

		double m_percentile = 50.0;  // for --percentile option
		std::string m_title;         // for --title option

		int m_rangeL;                // for --range option
		int m_rangeH;                // for --range option

		std::map<std::string, std::string> m_refmap;

		//   track       >midi       >tokens        <token, subtoken>
		std::vector<std::vector<std::vector<std::pair<HTp, int>>>> m_trackMidi;

		// m_trackToKernIndex: mapping from track to **kern index
		std::vector<int> m_trackToKernIndex;

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_PRANGE_H */



