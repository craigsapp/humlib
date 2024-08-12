//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Mon Jul 15 23:31:37 PDT 2024
// Last Modified: Mon Jul 15 23:31:40 PDT 2024
// Filename:      tool-rphrase.h
// URL:           https://github.com/craigsapp/humlib/blob/master/include/tool-rphrase.h
// Syntax:        C++11; humlib
// vim:           ts=3 noexpandtab
//
// Description:   Calculate duration of notes as phrases delimited by rests.
//

#ifndef _TOOL_RPHRASE_H
#define _TOOL_RPHRASE_H

#include "HumTool.h"
#include "HumdrumFile.h"

#include <ostream>
#include <string>
#include <vector>

namespace hum {

// START_MERGE

class Tool_rphrase : public HumTool {
	public:

	class VoiceInfo {
		public:
			std::string name;
			std::vector<double> restsBefore;
			std::vector<double> phraseDurs;
			std::vector<int>    barStarts;
			std::vector<HTp>    phraseStartToks;
	};

		         Tool_rphrase      (void);
		        ~Tool_rphrase      () {};

		bool     run               (HumdrumFileSet& infiles);
		bool     run               (HumdrumFile& infile);
		bool     run               (const std::string& indata, std::ostream& out);
		bool     run               (HumdrumFile& infile, std::ostream& out);
		void     finally           (void);

	protected:
		void     initialize        (void);
		void     processFile       (HumdrumFile& infile);
		void     fillVoiceInfo     (std::vector<Tool_rphrase::VoiceInfo>& voiceInfo, std::vector<HTp>& kstarts, HumdrumFile& infile);
		void     fillVoiceInfo     (Tool_rphrase::VoiceInfo& voiceInfo, HTp& kstart, HumdrumFile& infile);
		void     fillCompositeInfo  (Tool_rphrase::VoiceInfo& collapseInfo, HumdrumFile& infile);
		void     printVoiceInfo    (std::vector<Tool_rphrase::VoiceInfo>& voiceInfo);
		void     printVoiceInfo    (Tool_rphrase::VoiceInfo& voiceInfo);

		void     printEmbeddedVoiceInfo(std::vector<Tool_rphrase::VoiceInfo>& voiceInfo, Tool_rphrase::VoiceInfo& collapseInfo, HumdrumFile& infile);
		void     printEmbeddedIndividualVoiceInfo(Tool_rphrase::VoiceInfo& voiceInfo, HumdrumFile& infile);
		void     printEmbeddedCompositeInfo(Tool_rphrase::VoiceInfo& compositeInfo, HumdrumFile& infile);

		void     getCompositeStates(std::vector<int>& noteStates, HumdrumFile& infile);
		std::string getCompositeLabel(HumdrumFile& infile);
		void     markPhraseStartsInScore(HumdrumFile& infile, Tool_rphrase::VoiceInfo& voiceInfo);
		void     markCompositePhraseStartsInScore(HumdrumFile& infile, Tool_rphrase::VoiceInfo& collapseInfo);
		void     outputMarkedFile  (HumdrumFile& infile, std::vector<Tool_rphrase::VoiceInfo>& voiceInfo,
		                            Tool_rphrase::VoiceInfo& compositeInfo);
		void     printDataLine     (HumdrumFile& infile, int index);
		void     markLongaDurations(HumdrumFile& infile);
		std::string getVoiceInfo(HumdrumFile& infile);
		void     printEmbeddedVoiceInfoSummary(std::vector<Tool_rphrase::VoiceInfo>& voiceInfo, HumdrumFile& infile);
		double   twoDigitRound(double input);
		void     printHyperlink(const std::string& urlType);

	private:
		bool        m_averageQ        = false; // for -a option
		bool        m_allAverageQ     = false; // for -A option
		bool        m_breathQ         = true;  // for -B option
		bool        m_barlineQ        = false; // for -m option
		bool        m_compositeQ      = false; // for -c option
		bool        m_longaQ          = false; // for -l option
		bool        m_filenameQ       = false; // for -f option
		bool        m_fullFilenameQ   = false; // for -F option
		std::string m_filename;                // for -f or -F option
		bool        m_sortQ           = false; // for -s option
		bool        m_reverseSortQ    = false; // for -S option
		int         m_pcount          = 0;     // for -a option
		double      m_sum             = 0.0;   // for -a option
		int         m_pcountComposite = 0;     // for -c option
		double      m_sumComposite    = 0.0;   // for -c option
		bool        m_markQ           = false; // for --mark option
		double      m_durUnit         = 2.0;   // for -d option
		bool        m_infoQ           = false; // for -i option (when --mark is active)
		bool        m_squeezeQ        = false; // for -z option
		bool        m_closeQ          = false; // for --close option
		std::string m_urlType;                 // for -u option

		int         m_line = 1;
		std::string m_voiceLengthColor     = "crimson";
		std::string m_compositeLengthColor = "limegreen";

};

// END_MERGE

} // end namespace hum

#endif /* _TOOL_RPHRASE_H */



