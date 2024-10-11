//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Nov 25 14:18:01 PST 2000
// Last Modified: Fri Sep  2 10:16:09 CEST 2016 (added to humlib)
// Filename:      HumInstrument.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/HumInstrument.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Special enumeration class for processing Humdrum
//                instrument names.
//

#include "HumInstrument.h"

#include <cstring>

using namespace std;

namespace hum {

// START_MERGE

typedef unsigned long long TEMP64BITFIX;

// declare static variables
vector<_HumInstrument> HumInstrument::m_data;
int HumInstrument::m_classcount = 0;


//////////////////////////////
//
// HumInstrument::HumInstrument --
//

HumInstrument::HumInstrument(void) {
	if (m_classcount == 0) {
		initialize();
	}
	m_classcount++;
	m_index = -1;
}



//////////////////////////////
//
// HumInstrument::HumInstrument --
//

HumInstrument::HumInstrument(const string& Hname) {
	if (m_classcount == 0) {
		initialize();
	}

	m_index = find(Hname);
}



//////////////////////////////
//
// HumInstrument::~HumInstrument --
//

HumInstrument::~HumInstrument() {
	m_index = -1;
}



/////////////////////////////
//
// HumInstrument::getGM --
//

int HumInstrument::getGM(void) {
	if (m_index > 0) {
		return m_data[m_index].gm;
	} else {
		return -1;
	}
}



/////////////////////////////
//
// HumInstrument::getGM --
//

int HumInstrument::getGM(const string& Hname) {
	int tindex;
	if (Hname.compare(0, 2, "*I") == 0) {
		tindex = find(Hname.substr(2));
	} else {
		tindex = find(Hname);
	}

	if (tindex > 0) {
		return m_data[tindex].gm;
	} else {
		return -1;
	}
}



//////////////////////////////
//
// HumInstrument::getName --
//

string HumInstrument::getName(void) {
	if (m_index > 0) {
		return m_data[m_index].name;
	} else {
		return "";
	}
}



//////////////////////////////
//
// HumInstrument::getName --
//

string HumInstrument::getName(const string& Hname) {
	int tindex;
	if (Hname.compare(0, 2, "*I") == 0) {
		tindex = find(Hname.substr(2));
	} else{
		tindex = find(Hname);
	}
	if (tindex > 0) {
		return m_data[tindex].name;
	} else {
		return "";
	}
}



//////////////////////////////
//
// HumInstrument::getHumdrum --
//

string HumInstrument::getHumdrum(void) {
	if (m_index > 0) {
		return m_data[m_index].humdrum;
	} else {
		return "";
	}
}



//////////////////////////////
//
// HumInstrument::setGM --
//

int HumInstrument::setGM(const string& Hname, int aValue) {
	if (aValue < 0 || aValue > 127) {
		return 0;
	}
	int rindex = find(Hname);
	if (rindex > 0) {
		m_data[rindex].gm = aValue;
	} else {
		afi(Hname.c_str(), aValue, Hname.c_str());
		sortData();
	}
	return rindex;
}



//////////////////////////////
//
// HumInstrument::setHumdrum --
//

void HumInstrument::setHumdrum(const string& Hname) {
	if (Hname.compare(0, 2, "*I") == 0) {
		m_index = find(Hname.substr(2));
	} else {
		m_index = find(Hname);
	}
                                             }



//////////////////////////////////////////////////////////////////////////
//
// private functions
//


//////////////////////////////
//
// HumInstrument::initialize --
//
void HumInstrument::initialize(void) {
   m_data.reserve(500);

	// List has to be sorted by first parameter.  Maybe put in map.
   afi("accor",   GM_ACCORDION,             "accordion");
   afi("alto",    GM_RECORDER,              "alto");
   afi("anvil",   GM_TINKLE_BELL,           "anvil");
   afi("archl",   GM_ACOUSTIC_GUITAR_NYLON, "archlute");
   afi("armon",   GM_HARMONICA,             "harmonica");
   afi("arpa",    GM_ORCHESTRAL_HARP,       "harp");
   afi("bagpI",   GM_BAGPIPE,               "bagpipe (Irish)");
   afi("bagpS",   GM_BAGPIPE,               "bagpipe (Scottish)");
   afi("banjo",   GM_BANJO,                 "banjo");
   afi("bansu",   GM_FLUTE,                 "bansuri");
   afi("barit",   GM_CHOIR_AAHS,            "baritone");
   afi("baset",   GM_CLARINET,              "bassett horn");
   afi("bass",    GM_CHOIR_AAHS,            "bass");
   afi("bdrum",   GM_TAIKO_DRUM,            "bass drum");
   afi("bguit",   GM_ELECTRIC_BASS_FINGER,  "electric bass guitar");
   afi("biwa",    GM_FLUTE,                 "biwa");
   afi("bongo",   GM_TAIKO_DRUM,            "bongo");
   afi("brush",   GM_BREATH_NOISE,          "brush");
   afi("bscan",   GM_CHOIR_AAHS,            "basso cantante");
   afi("bspro",   GM_CHOIR_AAHS,            "basso profondo");
   afi("bugle",   GM_TRUMPET,               "bugle");
   afi("calam",   GM_OBOE,                  "chalumeau");
   afi("calpe",   GM_LEAD_CALLIOPE,         "calliope");
   afi("calto",   GM_CHOIR_AAHS,            "contralto");
   afi("campn",   GM_TUBULAR_BELLS,         "bell");
   afi("cangl",   GM_ENGLISH_HORN,          "english horn");
   afi("canto",   GM_CHOIR_AAHS,            "canto");
   afi("caril",   GM_TUBULAR_BELLS,         "carillon");
   afi("castr",   GM_CHOIR_AAHS,            "castrato");
   afi("casts",   GM_WOODBLOCKS,            "castanets");
   afi("cbass",   GM_CONTRABASS,            "contrabass");
   afi("cello",   GM_CELLO,                 "violoncello");
   afi("cemba",   GM_HARPSICHORD,           "harpsichord");
   afi("cetra",   GM_VIOLIN,                "cittern");
   afi("chain",   GM_TINKLE_BELL,           "chains");
   afi("chcym",   GM_REVERSE_CYMBAL,        "China cymbal");
   afi("chime",   GM_TUBULAR_BELLS,         "chimes");
   afi("chlma",   GM_BASSOON,               "alto shawm");
   afi("chlms",   GM_BASSOON,               "soprano shawm");
   afi("chlmt",   GM_BASSOON,               "tenor shawm");
   afi("clap",    GM_GUNSHOT,               "hand clapping");
   afi("clara",   GM_CLARINET,              "alto clarinet");
   afi("clarb",   GM_CLARINET,              "bass clarinet");
   afi("clarp",   GM_CLARINET,              "piccolo clarinet");
   afi("clars",   GM_CLARINET,              "clarinet");
   afi("clave",   GM_AGOGO,                 "claves");
   afi("clavi",   GM_CLAVI,                 "clavichord");
   afi("clest",   GM_CELESTA,               "celesta");
   afi("clrno",   GM_TRUMPET,               "clarino");
   afi("colsp",   GM_FLUTE,                 "coloratura soprano");
   afi("conga",   GM_TAIKO_DRUM,            "conga");
   afi("cor",     GM_FRENCH_HORN,           "horn");
   afi("cornm",   GM_BAGPIPE,               "French bagpipe");
   afi("corno",   GM_TRUMPET,               "cornett");
   afi("cornt",   GM_TRUMPET,               "cornet");
   afi("coro",    GM_CHOIR_AAHS,            "chorus");
   afi("crshc",   GM_REVERSE_CYMBAL,        "crash cymbal");
   afi("ctenor",  GM_CHOIR_AAHS,            "counter-tenor");
   afi("ctina",   GM_ACCORDION,             "concertina");
   afi("drmsp",   GM_FLUTE,                 "dramatic soprano");
   afi("drum",    GM_SYNTH_DRUM,            "drum");
   afi("drumP",   GM_SYNTH_DRUM,            "small drum");
   afi("dulc",    GM_DULCIMER,              "dulcimer");
   afi("eguit",   GM_ELECTRIC_GUITAR_CLEAN, "electric guitar");
   afi("fag_c",   GM_BASSOON,               "contrabassoon");
   afi("fagot",   GM_BASSOON,               "bassoon");
   afi("false",   GM_RECORDER,              "falsetto");
   afi("fdrum",   GM_TAIKO_DRUM,            "frame drum");
   afi("feme",    GM_CHOIR_AAHS,            "female voice");
   afi("fife",    GM_BLOWN_BOTTLE,          "fife");
   afi("fingc",   GM_REVERSE_CYMBAL,        "finger cymbal");
   afi("flt",     GM_FLUTE,                 "flute");
   afi("flt_a",   GM_FLUTE,                 "alto flute");
   afi("flt_b",   GM_FLUTE,                 "bass flute");
   afi("fltda",   GM_RECORDER,              "alto recorder");
   afi("fltdb",   GM_RECORDER,              "bass recorder");
   afi("fltdn",   GM_RECORDER,              "sopranino recorder");
   afi("fltds",   GM_RECORDER,              "soprano recorder");
   afi("fltdt",   GM_RECORDER,              "tenor recorder");
   afi("flugh",   GM_FRENCH_HORN,           "flugelhorn");
   afi("forte",   GM_HONKYTONK_PIANO,       "fortepiano");
   afi("gen",     GM_ACOUSTIC_GRAND_PIANO,  "generic instrument");
   afi("genB",    GM_ACOUSTIC_GRAND_PIANO,  "generic bass instrument");
   afi("genT",    GM_ACOUSTIC_GRAND_PIANO,  "generic treble instrument");
   afi("glock",   GM_GLOCKENSPIEL,          "glockenspiel");
   afi("gong",    GM_REVERSE_CYMBAL,        "gong");
   afi("guitr",   GM_ACOUSTIC_GUITAR_NYLON, "guitar");
   afi("hammd",   GM_DRAWBAR_ORGAN,         "Hammond electronic organ");
   afi("hbell",   GM_TINKLE_BELL,           "handbell");
   afi("hbell",   GM_TINKLE_BELL,           "handbell");
   afi("heck",    GM_BASSOON,               "heckelphone");
   afi("heltn",   GM_CHOIR_AAHS,            "Heldentenor");
   afi("hichi",   GM_OBOE,                  "hichiriki");
   afi("hurdy",   GM_LEAD_CALLIOPE,         "hurdy-gurdy");
   afi("kitv",    GM_VIOLIN,                "kit violin");
   afi("klav",    GM_ACOUSTIC_GRAND_PIANO,  "keyboard");
   afi("kokyu",   GM_FIDDLE,                "kokyu");
   afi("komun",   GM_KOTO,                  "komun'go");
   afi("koto",    GM_KOTO,                  "koto");
   afi("kruma",   GM_TRUMPET,               "alto crumhorn");
   afi("krumb",   GM_TRUMPET,               "bass crumhorn");
   afi("krums",   GM_TRUMPET,               "soprano crumhorn");
   afi("krumt",   GM_TRUMPET,               "tenor crumhorn");
   afi("lion",    GM_AGOGO,                 "lion's roar");
   afi("liuto",   GM_ACOUSTIC_GUITAR_NYLON, "lute");
   afi("lyrsp",   GM_FLUTE,                 "lyric soprano");
   afi("lyrtn",   GM_FRENCH_HORN,           "lyric tenor");
   afi("male",    GM_CHOIR_AAHS,            "male voice");
   afi("mando",   GM_ACOUSTIC_GUITAR_NYLON, "mandolin");
   afi("marac",   GM_AGOGO,                 "maracas");
   afi("marim",   GM_MARIMBA,               "marimba");
   afi("mbari",   GM_CHOIR_AAHS,            "high baritone");
   afi("mezzo",   GM_CHOIR_AAHS,            "mezzo soprano");
   afi("nfant",   GM_CHOIR_AAHS,            "child's voice");
   afi("nokan",   GM_SHAKUHACHI,            "nokan");
   afi("oboe",    GM_OBOE,                  "oboe");
   afi("oboeD",   GM_ENGLISH_HORN,          "oboe d'amore");
   afi("ocari",   GM_OCARINA,               "ocarina");
   afi("ondes",   GM_PAD_SWEEP,             "ondes Martenot");
   afi("ophic",   GM_TUBA,                  "ophicleide");
   afi("organ",   GM_CHURCH_ORGAN,          "pipe organ");
   afi("oud",     GM_ACOUSTIC_GUITAR_NYLON, "oud");
   afi("paila",   GM_AGOGO,                 "timbales");
   afi("panpi",   GM_PAN_FLUTE,             "panpipe");
   afi("pbell",   GM_TUBULAR_BELLS,         "bell plate");
   afi("pguit",   GM_ACOUSTIC_GUITAR_NYLON, "Portuguese guitar");
   afi("physh",   GM_REED_ORGAN,            "physharmonica");
   afi("piano",   GM_ACOUSTIC_GRAND_PIANO,  "pianoforte");
   afi("piatt",   GM_REVERSE_CYMBAL,        "cymbals");
   afi("picco",   GM_PICCOLO,               "piccolo");
   afi("pipa",    GM_ACOUSTIC_GUITAR_NYLON, "Chinese lute");
   afi("porta",   GM_TANGO_ACCORDION,       "portative organ");
   afi("psalt",   GM_CLAVI,                 "psaltery");
   afi("qin",     GM_CLAVI,                 "qin");
   afi("quinto",  GM_CHOIR_AAHS,            "quinto");
   afi("quitr",   GM_ACOUSTIC_GUITAR_NYLON, "gittern");
   afi("rackt",   GM_TRUMPET,               "racket");
   afi("ratl",    GM_WOODBLOCKS,            "rattle");
   afi("rebec",   GM_ACOUSTIC_GUITAR_NYLON, "rebec");
   afi("recit",   GM_CHOIR_AAHS,            "recitativo");
   afi("reedo",   GM_REED_ORGAN,            "reed organ");
   afi("rhode",   GM_ELECTRIC_PIANO_1,      "Fender-Rhodes electric piano");
   afi("ridec",   GM_REVERSE_CYMBAL,        "ride cymbal");
   afi("sarod",   GM_SITAR,                 "sarod");
   afi("sarus",   GM_TUBA,                  "sarrusophone");
   afi("saxA",    GM_ALTO_SAX,              "alto saxophone");
   afi("saxB",    GM_BARITONE_SAX,          "bass saxophone");
   afi("saxC",    GM_BARITONE_SAX,          "contrabass saxophone");
   afi("saxN",    GM_SOPRANO_SAX,           "sopranino saxophone");
   afi("saxR",    GM_BARITONE_SAX,          "baritone saxophone");
   afi("saxS",    GM_SOPRANO_SAX,           "soprano saxophone");
   afi("saxT",    GM_TENOR_SAX,             "tenor saxophone");
   afi("sbell",   GM_TINKLE_BELL,           "sleigh bells");
   afi("sdrum",   GM_SYNTH_DRUM,            "snare drum (kit)");
   afi("shaku",   GM_SHAKUHACHI,            "shakuhachi");
   afi("shami",   GM_SHAMISEN,              "shamisen");
   afi("sheng",   GM_SHANAI,                "sheng");
   afi("sho",     GM_SHANAI,                "sho");
   afi("siren",   GM_FX_SCI_FI,             "siren");
   afi("sitar",   GM_SITAR,                 "sitar");
   afi("slap",    GM_GUNSHOT,               "slapstick");
   afi("soprn",   GM_CHOIR_AAHS,            "soprano");
   afi("spshc",   GM_REVERSE_CYMBAL,        "splash cymbal");
   afi("steel",   GM_STEEL_DRUMS,           "steel-drum");
   afi("stim",    GM_SEASHORE,              "Sprechstimme");
   afi("stimA",   GM_SEASHORE,              "Sprechstimme, alto");
   afi("stimB",   GM_SEASHORE,              "Sprechstimme, bass");
   afi("stimC",   GM_SEASHORE,              "Sprechstimme, contralto");
   afi("stimR",   GM_SEASHORE,              "Sprechstimme, baritone");
   afi("stimS",   GM_SEASHORE,              "Sprechstimme, soprano");
   afi("strdr",   GM_AGOGO,                 "string drum");
   afi("sxhA",    GM_ALTO_SAX,              "alto saxhorn");
   afi("sxhB",    GM_BARITONE_SAX,          "bass saxhorn");
   afi("sxhC",    GM_BARITONE_SAX,          "contrabass saxhorn");
   afi("sxhR",    GM_BARITONE_SAX,          "baritone saxhorn");
   afi("sxhS",    GM_SOPRANO_SAX,           "soprano saxhorn");
   afi("sxhT",    GM_TENOR_SAX,             "tenor saxhorn");
   afi("synth",   GM_ELECTRIC_PIANO_2,      "keyboard synthesizer");
   afi("tabla",   GM_MELODIC_DRUM,          "tabla");
   afi("tambn",   GM_TINKLE_BELL,           "tambourine");
   afi("tambu",   GM_MELODIC_DRUM,          "tambura");
   afi("tanbr",   GM_MELODIC_DRUM,          "tanbur");
   afi("tblok",   GM_WOODBLOCKS,            "temple blocks");
   afi("tdrum",   GM_SYNTH_DRUM,            "tenor drum");
   afi("tenor",   GM_CHOIR_AAHS,            "tenor");
   afi("timpa",   GM_MELODIC_DRUM,          "timpani");
   afi("tiorb",   GM_ACOUSTIC_GUITAR_NYLON, "theorbo");
   afi("tom",     GM_TAIKO_DRUM,            "tom-tom drum");
   afi("trngl",   GM_TINKLE_BELL,           "triangle");
   afi("tromb",   GM_TROMBONE,              "bass trombone");
   afi("tromp",   GM_TRUMPET,               "trumpet");
   afi("tromt",   GM_TROMBONE,              "tenor trombone");
   afi("tuba",    GM_TUBA,                  "tuba");
   afi("tubaB",   GM_TUBA,                  "bass tuba");
   afi("tubaC",   GM_TUBA,                  "contrabass tuba");
   afi("tubaT",   GM_TUBA,                  "tenor tuba");
   afi("tubaU",   GM_TUBA,                  "subcontra tuba");
   afi("ukule",   GM_ACOUSTIC_GUITAR_NYLON, "ukulele");
   afi("vibra",   GM_VIBRAPHONE,            "vibraphone");
   afi("vina",    GM_SITAR,                 "vina");
   afi("viola",   GM_VIOLA,                 "viola");
   afi("violb",   GM_CONTRABASS,            "bass viola da gamba");
   afi("viold",   GM_VIOLA,                 "viola d'amore");
   afi("violn",   GM_VIOLIN,                "violin");
   afi("violp",   GM_VIOLIN,                "piccolo violin");
   afi("viols",   GM_VIOLIN,                "treble viola da gamba");
   afi("violt",   GM_CELLO,                 "tenor viola da gamba");
   afi("vox",     GM_CHOIR_AAHS,            "generic voice");
   afi("wblok",   GM_WOODBLOCKS,            "woodblock");
   afi("xylo",    GM_XYLOPHONE,             "xylophone");
   afi("zithr",   GM_CLAVI,                 "zither");
   afi("zurna",   GM_ACOUSTIC_GUITAR_NYLON, "zurna");

}



//////////////////////////////
//
// HumInstrument::afi --
//

void HumInstrument::afi(const char* humdrum_name, int midinum,
		const char* EN_name) {
	_HumInstrument x;
	x.name = EN_name;
	x.humdrum = humdrum_name;
	x.gm = midinum;

	m_data.push_back(x);
}



//////////////////////////////
//
// HumInstrument::find --
//

int HumInstrument::find(const string& Hname) {
	void* searchResult;
	_HumInstrument key;
	key.humdrum = Hname;
	key.name = "";
	key.gm = 0;

	searchResult = bsearch(&key, m_data.data(),
			m_data.size(), sizeof(_HumInstrument),
			&data_compare_by_humdrum_name);

	if (searchResult == NULL) {
		return -1;
	} else {
		return (int)(((TEMP64BITFIX)(searchResult)) - ((TEMP64BITFIX)(m_data.data())))/
			sizeof(_HumInstrument);
	}
}


//////////////////////////////
//
// HumInstrument::data_compare_by_humdrum_name --
//

int HumInstrument::data_compare_by_humdrum_name(const void* a,
		const void* b) {
	_HumInstrument& valuea = *((_HumInstrument*)a);
	_HumInstrument& valueb = *((_HumInstrument*)b);
	return strcmp(valuea.humdrum.c_str(), valueb.humdrum.c_str());
}



//////////////////////////////
//
// HumInstrument::sortData --
//

void HumInstrument::sortData(void) {
	qsort(m_data.data(), m_data.size(), sizeof(_HumInstrument),
		&HumInstrument::data_compare_by_humdrum_name);
}


// END_MERGE

} // end namespace hum



