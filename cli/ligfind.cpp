//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Jun 13 11:45:47 PDT 2020
// Last Modified: Sat Jun 13 11:45:50 PDT 2020
// Filename:      cli/ligfind.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/cli/ligfind.cpp
// Syntax:        C++11
// vim:           ts=3 noexpandtab nowrap
//
// Description:   Find cases where there are two semi-breves (whole notes)
//                in a row, with lyric text only on the first note.
//                This is for the Tasso in Music Project to find locations
//                where ligature marks should probably be added.
// 
// To do:         Maybe add an option to check whether or not there is already
//                a ligature mark on the two notes.
//

/* Example output: (starting measure/part name/filename)

19	Quinto	Tam1020468a-Io_che_fin_a_quel_punto_altro_non_volsi--Marotta_1600
1	Tenore	Tco0806a-Piange_sospira_e_quando_i_caldi_raggi--Monteverdi_1603
2	Tenore	Tco0806a-Piange_sospira_e_quando_i_caldi_raggi--Monteverdi_1603
7	Basso	Trm0025c-Come_vivro_ne_le_mie_pene_Amore--Billi_1602
24	Alto	Trm0048a-Amor_lalma_mallaccia--Meldert_1575
87	Quinto	Trm0099a-Geloso_amante_apro_millocchi_e_giro--Luzzaschi_1576
93	Quinto	Trm0099a-Geloso_amante_apro_millocchi_e_giro--Luzzaschi_1576
58	Sesto	Trm0248a-Vita_de_la_mia_vita--Marenzio_1584
80	Sesto	Trm0248a-Vita_de_la_mia_vita--Marenzio_1584
49	Alto	Trm0255a-Mentre_in_grembo_a_la_madre_un_giorno--Giovannelli_1599
44	Tenore	Trm0256d-Amor_che_qui_dintorno--Nanino_1599
82	Basso	Trm0378h-Nel_dolce_seno_de_la_bella_Clori--Luzzaschi_1604
97	Basso	Trm0378h-Nel_dolce_seno_de_la_bella_Clori--Luzzaschi_1604
28	Basso	Tsg12065a-Segue_egli_la_vittoria_e_la_trafitta--Massaino_1587
9	Alto	Tsg12066a--Amico_hai_vinto_io_ti_perdon_perdona--Massaino_1587
16	Alto	Tsg12096c-Giunto_a_la_tomba_ove_al_suo_spirto_vivo--Ricci_1597
20	[Basso continuo]	Tsg12096g-Giunto_a_la_tomba_ove_al_suo_spirto_vivo--DIndia_1618
29	[Canto]	Tsg16060e-La_tral_sangue_e_le_morti_egro_giacente--DIndia_1609
32	[Canto]	Tsg19107c-Ma_che_squallido_e_scuro_anco_mi_piaci--DIndia_1609
9	Basso continuo	Tsg20128b-Si_volse_Armida_e_l_rimiro_improvviso--Eredi_1629

*/

#include "humlib.h"
#include <iostream>

using namespace hum;
using namespace std;

void  processFile            (HumdrumFile& infile);
void  processPart            (HumdrumFile& infile, HTp partstart);
void  analyzeBarNumbers      (HumdrumFile& infile);
HTp   getNoteList            (vector<HTp>& notelist, HTp partstart);
void  printLigatureCandidate (HTp partname, HTp starting, HTp ending);
bool  hasText                (HTp token);

vector<int> m_barnum;   // measure number of current line in Humdrum file.

///////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
	Options options;
	options.process(argc, argv);
	HumdrumFileStream instream(options);
	HumdrumFile infile;
	while (instream.read(infile)) {
		processFile(infile);
	}
	return 0;
}



//////////////////////////////
//
// processFile --
//

void processFile(HumdrumFile& infile) {
	analyzeBarNumbers(infile);
	vector<HTp> sstarts;
	sstarts = infile.getKernSpineStartList();
	for (int i=0; i<(int)sstarts.size(); i++) {
		processPart(infile, sstarts[i]);
	}
}



//////////////////////////////
//
// analyzeBarNumbers -- Create an index of the measure number
//    that each line of the input file occurs in.
//

void analyzeBarNumbers(HumdrumFile& infile) {
	m_barnum.clear();
	m_barnum.resize(infile.getLineCount());
	int current = -1;
	HumRegex hre;
	for (int i=0; i<infile.getLineCount(); i++) {
		if (!infile[i].isBarline()) {
			m_barnum.at(i) = current;
			continue;
		}
		if (hre.search(infile[i].token(0), "=(\\d+)")) {
			current = hre.getMatchInt(1);
		}
		m_barnum.at(i) = current;
	}
}



//////////////////////////////
//
// processPart -- Create a list of notes, and then
//    search for two whole notes where the first whole note
//    has a syllable and the second one has a null syllable.
//
//    Two whole notes may be mean dotted whole notes in
//    triple mensurations, or could even means 2/3rd of a whole
//    note in coloration, so a range of durations for whole notes
//    is considered, from 8/3 to 6 (units of duration are in quarter
//    notes).
//

void processPart(HumdrumFile& infile, HTp partstart) {
	vector<HTp> notelist;
	HTp partname = getNoteList(notelist, partstart);
	vector<HumNum> durations(notelist.size());
	for (int i=0; i<(int)notelist.size(); i++) {
		durations[i] = notelist[i]->getTiedDuration();
	}

	HumNum mindur(8, 3);
	HumNum maxdur(6, 1);
	for (int i=0; i<(int)durations.size()-1; i++) {
		if (notelist[i]->isRest()) {
			continue;
		}
		if (notelist[i+1]->isRest()) {
			continue;
		}
		if ((durations[i] >= mindur) && (durations[i] <= maxdur) &&
		    (durations[i+1] >= mindur) && (durations[i+1] <= maxdur)) {
			// exclude cases where the pitch is repeated
			int pitch1 = Convert::kernToMidiNoteNumber(notelist[i]);
			int pitch2 = Convert::kernToMidiNoteNumber(notelist[i+1]);
			if (pitch1 == pitch2) {
				continue;
			}
			bool hastext1 = hasText(notelist[i]);
			bool hastext2 = hasText(notelist[i+1]);
			if (hastext1 && !hastext2) {
				printLigatureCandidate(partname, notelist[i], notelist[i+1]);
			}
		}
	}
}



//////////////////////////////
//
// hasText -- return true if the given note has non-null
//   **text content in a field to the right of it before the
//   next **kern spine.  Spine splits in **kern data can cause
//   problems, but there should be no spine splits in data that
//   will be analyze with this program.
//

bool hasText(HTp token) {
	HTp current = token->getNextFieldToken();
	while (current) {
		if (current->isKern()) {
			break;
		}
		if (current->isDataType("**text")) {
			if (current->isNull()) {
				return false;
			} else {
				return true;
			}
		}
		current = current->getNextFieldToken();
	}
	return false;
}



//////////////////////////////
//
// printLigatureCandidate -- Print pairs of notes that are probably
//    written as a ligature in the original notation.
//

void printLigatureCandidate(HTp partname, HTp starting, HTp ending) {
	HumdrumFile* infile = starting->getOwner()->getOwner();
	cout << m_barnum[starting->getLineIndex()];
	if (partname) {
		cout << "\t" << partname->substr(3);
	} else {
		cout << "\t";
	}
	cout << "\t" << infile->getFilenameBase();
	cout << endl;
}



//////////////////////////////
//
// getNoteList -- Get a melodic list of notes in a part
//   (ignoring any spine splits).  Secondary tied notes are
//    not stored.
//

HTp getNoteList(vector<HTp>& notelist, HTp partstart) {
	HumdrumFile* infile = partstart->getOwner()->getOwner();
	notelist.clear();
	notelist.reserve(infile->getLineCount());
	HTp current = partstart->getNextToken();
	HTp partname = NULL;
	while (current) {
		if (current->isInterpretation()) {
			if (current->compare(0, 3, "*I\"") == 0) {
				partname = current;
			}
		}
		if (current->isData() && current->isNoteAttack() && !current->isNull()) {
			notelist.push_back(current);
		}
		current = current->getNextToken();
	}
	return partname;
}



