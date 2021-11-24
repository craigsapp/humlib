//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Feb 29 20:10:49 PST 2020
// Last Modified: Sat Feb 29 20:10:52 PST 2020
// Filename:      Convert-reference.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-reference.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Conversions related to reference records.
//

#include "Convert.h"
#include "HumRegex.h"

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// Convert::getReferenceKeyMeaning --
//

string Convert::getReferenceKeyMeaning(HTp token) {
	string text = token->getText();
	return Convert::getReferenceKeyMeaning(text);
}

string Convert::getReferenceKeyMeaning(const string& token) {
	string key;
	string keybase;
	string translation;
	string language;
	string number;
	HumRegex hre;
	if (hre.search(token, "^!!!+\\s*([^:]+)\\s*:")) {
		key = hre.getMatch(1);
	}
	if (key.empty()) {
		return "";
	}
	if (islower(key[0])) {
		// non-standard reference record.
		return "";
	}

	// extract language information
	if (hre.search(key, "^([^@])+@@([^@]+)$")) {
		key      = hre.getMatch(1);
		language = hre.getMatch(2);
	} else if (hre.search(key, "^([^@])+@([^@]+)$")) {
		key         = hre.getMatch(1);
		translation = hre.getMatch(2);
	}

	// extract number qualifier
	if (hre.search(key, "^(.*)(\\d+)$")) {
		key     = hre.getMatch(1);
		number  = hre.getMatch(2);
	}

	if (key.empty()) {
		return "";
	}

	string meaning;
	switch (key[0]) {
		case 'A':	// analytic information
			if      (key == "ACO") { meaning = "Collection designation"; }
			else if (key == "AFR") { meaning = "Form designation"; }
			else if (key == "AGN") { meaning = "genre designation"; }
			else if (key == "AST") { meaning = "Syle/period"; }
			else if (key == "AMD") { meaning = "Mode classification"; }
			else if (key == "AMT") { meaning = "Meter classification"; }
			else if (key == "AIN") { meaning = "Instrumentation"; }
			else if (key == "ARE") { meaning = "Geographical region of origin"; }
			else if (key == "ARL") { meaning = "Origin coordinates"; }
			break;

		case 'C':	// composer-base reference records
			if      (key == "COM") { meaning = "Composer"; }
			else if (key == "CDT") { meaning = "Composer's dates"; }
			else if (key == "CNT") { meaning = "Composer's nationality"; }
			else if (key == "COA") { meaning = "Attributed composer"; }
			else if (key == "COS") { meaning = "Suspected composer"; }
			else if (key == "COL") { meaning = "Composer's stage name"; }
			else if (key == "COC") { meaning = "Composer's corporate name"; }
			else if (key == "CBL") { meaning = "Composer's birth location"; }
			else if (key == "CDL") { meaning = "Composer's death location"; }
			break;

		case 'E':	// electronic editor
			if      (key == "EED") { meaning = "Electronic editor"; }
			else if (key == "ENC") { meaning = "Electronic encoder"; }
			else if (key == "END") { meaning = "Electronic encoding date"; }
			else if (key == "EMD") { meaning = "Modification description"; }
			else if (key == "EEV") { meaning = "Electronic edition version"; }
			else if (key == "EFL") { meaning = "Electronic file number"; }
			else if (key == "EST") { meaning = "Encoding status"; }
			break;

		case 'G':	// group information
			if      (key == "GTL") { meaning = "Group title"; }
			else if (key == "GAW") { meaning = "Associated work"; }
			else if (key == "GCO") { meaning = "Collection designation"; }
			break;

		case 'H':	// historical information
			if      (key == "HAO") { meaning = "Aural history"; }
			else if (key == "HTX") { meaning = "Vocal text translation"; }
			break;

		case 'L':	// lyricist/librettis/arranger/orchestrator reference records
			if      (key == "LYR") { meaning = "Lyricist"; }
			else if (key == "LIB") { meaning = "Librettist"; }
			else if (key == "LOR") { meaning = "Orchestrator"; }
			break;

		case 'M':	// performance information
			if      (key == "MPN") { meaning = "Performer"; }
			else if (key == "MPS") { meaning = "Suspected performer"; }
			else if (key == "MGN") { meaning = "Performance group name"; }
			else if (key == "MRD") { meaning = "Performance date"; }
			else if (key == "MLC") { meaning = "Performance location"; }
			else if (key == "MCN") { meaning = "Conductor"; }
			else if (key == "MPD") { meaning = "Premier date"; }
			break;

		case 'O':	// work (opus) information
			if      (key == "OTL") { meaning = "Work title"; }
			else if (key == "OTP") { meaning = "Popular title"; }
			else if (key == "OTA") { meaning = "Alternative title"; }
			else if (key == "OPR") { meaning = "Parent-work title"; }
			else if (key == "OAC") { meaning = "Act number"; }
			else if (key == "OSC") { meaning = "Scene number"; }
			else if (key == "OMV") { meaning = "Movement number"; }
			else if (key == "OMD") { meaning = "Movement designation"; }
			else if (key == "OPS") { meaning = "Opus number"; }
			else if (key == "ONM") { meaning = "Work number in opus"; }
			else if (key == "OVM") { meaning = "Volume number"; }
			else if (key == "ODE") { meaning = "Dedicatee"; }
			else if (key == "OCO") { meaning = "Commission"; }
			else if (key == "OCL") { meaning = "Collector"; }
			else if (key == "OCL") { meaning = "Free-form note"; }
			else if (key == "OCY") { meaning = "Composition country"; }
			else if (key == "OPC") { meaning = "Composition city"; }
			break;

		case 'P':	// publication information
			if      (key == "PUB") { meaning = "Publication status"; }
			else if (key == "PPR") { meaning = "First publisher"; }
			else if (key == "PTL") { meaning = "Publication title"; }
			else if (key == "PDT") { meaning = "Publication date"; }
			else if (key == "PPP") { meaning = "Publication location"; }
			else if (key == "PC#") { meaning = "Publication catalog number"; }
			else if (key == "SCT") { meaning = "Scholarly catalog abbreviation and number"; }
			else if (key == "SCA") { meaning = "Scholarly catalog unabbreviated name"; }
			else if (key == "SMS") { meaning = "Manuscript source name"; }
			else if (key == "SML") { meaning = "Manuscript location"; }
			else if (key == "SMA") { meaning = "Manuscript access"; }
			break;

		case 'R':
			// recording information
			if      (key == "RTL") { meaning = "Recording Title"; }
			else if (key == "RMM") { meaning = "Manufacturer"; }
			else if (key == "RC#") { meaning = "Catalog number"; }
			else if (key == "RRD") { meaning = "Recording release date"; }
			else if (key == "RLC") { meaning = "Recording location"; }
			else if (key == "RNP") { meaning = "Record producer"; }
			else if (key == "RDT") { meaning = "Recording date"; }
			else if (key == "RT#") { meaning = "Recording track number"; }
			// representation information
			else if (key == "RLN") { meaning = "ASCII language setting"; }
			else if (key == "RDF") { meaning = "User-defined signifiers"; }
			else if (key == "RDT") { meaning = "Encoding date"; }
			else if (key == "RNB") { meaning = "Encoding note"; }
			else if (key == "RWG") { meaning = "Encoding warning"; }
			break;

		case 'T':	// translator
			if      (key == "TRN") { meaning = "Translator"; }
			break;

		case 'V':	// version
			if      (key == "VTS") { meaning = "Data checksum"; }
			break;

		case 'Y':	// copyright information
			if      (key == "YEP") { meaning = "Publisher of electronic edition"; }
			else if (key == "YEC") { meaning = "Electronic edition copyright"; }
			else if (key == "YER") { meaning = "Electronic edition release year"; }
			else if (key == "YEM") { meaning = "Copyright message"; }
			else if (key == "YEC") { meaning = "Country of copyright"; }
			else if (key == "YOR") { meaning = "Original document"; }
			else if (key == "YOO") { meaning = "Original edition owner"; }
			else if (key == "YOY") { meaning = "Original edition copyright year"; }
			else if (key == "YOE") { meaning = "Original edition editor"; }
			break;
	}

	if (!number.empty()) {
		meaning += " #" + number;
	}

	if (!language.empty()) {
		meaning += ", original language " + Convert::getLanguageName(language);
	} else if (!translation.empty()) {
		meaning += ", translated into " + Convert::getLanguageName(language);
	}

	return meaning;
}



//////////////////////////////
//
// Convert::getLanguageName --
//

string Convert::getLanguageName(const string& abbreviation) {
	string code;
	for (int i=0; i<(int)abbreviation.size(); i++) {
		if (abbreviation[i] == '@') {
			continue;
		}
		code.push_back(tolower(abbreviation[i]));
	}

	if (code.size() == 2) {
		// ISO 639-1 language codes

		if (code == "aa") { return "Afar"; }
		if (code == "ab") { return "Abkhazian"; }
		if (code == "ae") { return "Avestan"; }
		if (code == "af") { return "Afrikaans"; }
		if (code == "ak") { return "Akan"; }
		if (code == "am") { return "Amharic"; }
		if (code == "an") { return "Aragonese"; }
		if (code == "ar") { return "Arabic"; }
		if (code == "as") { return "Assamese"; }
		if (code == "av") { return "Avaric"; }
		if (code == "ay") { return "Aymara"; }
		if (code == "az") { return "Azerbaijani"; }
		if (code == "ba") { return "Bashkir"; }
		if (code == "be") { return "Belarusian"; }
		if (code == "bg") { return "Bulgarian"; }
		if (code == "bh") { return "Bihari languages"; }
		if (code == "bi") { return "Bislama"; }
		if (code == "bm") { return "Bambara"; }
		if (code == "bn") { return "Bengali"; }
		if (code == "bo") { return "Tibetan"; }
		if (code == "br") { return "Breton"; }
		if (code == "bs") { return "Bosnian"; }
		if (code == "ca") { return "Catalan"; }
		if (code == "ce") { return "Chechen"; }
		if (code == "ch") { return "Chamorro"; }
		if (code == "co") { return "Corsican"; }
		if (code == "cr") { return "Cree"; }
		if (code == "cs") { return "Czech"; }
		if (code == "cs") { return "Czech"; }
		if (code == "cu") { return "Church Slavic"; }
		if (code == "cv") { return "Chuvash"; }
		if (code == "cy") { return "Welsh"; }
		if (code == "cy") { return "Welsh"; }
		if (code == "da") { return "Danish"; }
		if (code == "de") { return "German"; }
		if (code == "dv") { return "Divehi"; }
		if (code == "dz") { return "Dzongkha"; }
		if (code == "ee") { return "Ewe"; }
		if (code == "el") { return "Greek, Modern (1453-)"; }
		if (code == "en") { return "English"; }
		if (code == "eo") { return "Esperanto"; }
		if (code == "es") { return "Spanish"; }
		if (code == "et") { return "Estonian"; }
		if (code == "eu") { return "Basque"; }
		if (code == "eu") { return "Basque"; }
		if (code == "fa") { return "Persian"; }
		if (code == "ff") { return "Fulah"; }
		if (code == "fi") { return "Finnish"; }
		if (code == "fj") { return "Fijian"; }
		if (code == "fo") { return "Faroese"; }
		if (code == "fr") { return "French"; }
		if (code == "fy") { return "Western Frisian"; }
		if (code == "ga") { return "Irish"; }
		if (code == "gd") { return "Gaelic"; }
		if (code == "gl") { return "Galician"; }
		if (code == "gn") { return "Guarani"; }
		if (code == "gu") { return "Gujarati"; }
		if (code == "gv") { return "Manx"; }
		if (code == "ha") { return "Hausa"; }
		if (code == "he") { return "Hebrew"; }
		if (code == "hi") { return "Hindi"; }
		if (code == "ho") { return "Hiri Motu"; }
		if (code == "hr") { return "Croatian"; }
		if (code == "ht") { return "Haitian"; }
		if (code == "hu") { return "Hungarian"; }
		if (code == "hy") { return "Armenian"; }
		if (code == "hz") { return "Herero"; }
		if (code == "ia") { return "Interlingua"; }
		if (code == "id") { return "Indonesian"; }
		if (code == "ie") { return "Interlingue"; }
		if (code == "ig") { return "Igbo"; }
		if (code == "ii") { return "Sichuan Yi"; }
		if (code == "ik") { return "Inupiaq"; }
		if (code == "io") { return "Ido"; }
		if (code == "is") { return "Icelandic"; }
		if (code == "it") { return "Italian"; }
		if (code == "iu") { return "Inuktitut"; }
		if (code == "ja") { return "Japanese"; }
		if (code == "jv") { return "Javanese"; }
		if (code == "ka") { return "Georgian"; }
		if (code == "kg") { return "Kongo"; }
		if (code == "ki") { return "Kikuyu"; }
		if (code == "kj") { return "Kuanyama"; }
		if (code == "kk") { return "Kazakh"; }
		if (code == "kl") { return "Greenlandic"; }
		if (code == "km") { return "Central Khmer"; }
		if (code == "kn") { return "Kannada"; }
		if (code == "ko") { return "Korean"; }
		if (code == "kr") { return "Kanuri"; }
		if (code == "ks") { return "Kashmiri"; }
		if (code == "ku") { return "Kurdish"; }
		if (code == "kv") { return "Komi"; }
		if (code == "kw") { return "Cornish"; }
		if (code == "ky") { return "Kirghiz"; }
		if (code == "la") { return "Latin"; }
		if (code == "lb") { return "Luxembourgish"; }
		if (code == "lg") { return "Ganda"; }
		if (code == "li") { return "Limburgan"; }
		if (code == "ln") { return "Lingala"; }
		if (code == "lo") { return "Lao"; }
		if (code == "lt") { return "Lithuanian"; }
		if (code == "lu") { return "Luba-Katanga"; }
		if (code == "lv") { return "Latvian"; }
		if (code == "mg") { return "Malagasy"; }
		if (code == "mh") { return "Marshallese"; }
		if (code == "mi") { return "Maori"; }
		if (code == "mk") { return "Macedonian"; }
		if (code == "mk") { return "Macedonian"; }
		if (code == "ml") { return "Malayalam"; }
		if (code == "mn") { return "Mongolian"; }
		if (code == "mr") { return "Marathi"; }
		if (code == "ms") { return "Malay"; }
		if (code == "mt") { return "Maltese"; }
		if (code == "my") { return "Burmese"; }
		if (code == "my") { return "Burmese"; }
		if (code == "na") { return "Nauru"; }
		if (code == "nb") { return "Bokmål, Norwegian"; }
		if (code == "nd") { return "Ndebele, North"; }
		if (code == "ne") { return "Nepali"; }
		if (code == "ng") { return "Ndonga"; }
		if (code == "nl") { return "Dutch"; }
		if (code == "nl") { return "Dutch"; }
		if (code == "nn") { return "Norwegian Nynorsk"; }
		if (code == "no") { return "Norwegian"; }
		if (code == "nr") { return "Ndebele, South"; }
		if (code == "nv") { return "Navajo"; }
		if (code == "ny") { return "Chichewa"; }
		if (code == "oc") { return "Occitan (post 1500)"; }
		if (code == "oj") { return "Ojibwa"; }
		if (code == "om") { return "Oromo"; }
		if (code == "or") { return "Oriya"; }
		if (code == "os") { return "Ossetian"; }
		if (code == "pa") { return "Panjabi"; }
		if (code == "pi") { return "Pali"; }
		if (code == "pl") { return "Polish"; }
		if (code == "ps") { return "Pushto"; }
		if (code == "pt") { return "Portuguese"; }
		if (code == "qu") { return "Quechua"; }
		if (code == "rm") { return "Romansh"; }
		if (code == "rn") { return "Rundi"; }
		if (code == "ro") { return "Romanian"; }
		if (code == "ru") { return "Russian"; }
		if (code == "rw") { return "Kinyarwanda"; }
		if (code == "sa") { return "Sanskrit"; }
		if (code == "sc") { return "Sardinian"; }
		if (code == "sd") { return "Sindhi"; }
		if (code == "se") { return "Northern Sami"; }
		if (code == "sg") { return "Sango"; }
		if (code == "si") { return "Sinhala"; }
		if (code == "sl") { return "Slovenian"; }
		if (code == "sm") { return "Samoan"; }
		if (code == "sn") { return "Shona"; }
		if (code == "so") { return "Somali"; }
		if (code == "sq") { return "Albanian"; }
		if (code == "sr") { return "Serbian"; }
		if (code == "ss") { return "Swati"; }
		if (code == "st") { return "Sotho, Southern"; }
		if (code == "su") { return "Sundanese"; }
		if (code == "sv") { return "Swedish"; }
		if (code == "sw") { return "Swahili"; }
		if (code == "ta") { return "Tamil"; }
		if (code == "te") { return "Telugu"; }
		if (code == "tg") { return "Tajik"; }
		if (code == "th") { return "Thai"; }
		if (code == "ti") { return "Tigrinya"; }
		if (code == "tk") { return "Turkmen"; }
		if (code == "tl") { return "Tagalog"; }
		if (code == "tn") { return "Tswana"; }
		if (code == "to") { return "Tonga (Tonga Islands)"; }
		if (code == "tr") { return "Turkish"; }
		if (code == "ts") { return "Tsonga"; }
		if (code == "tt") { return "Tatar"; }
		if (code == "tw") { return "Twi"; }
		if (code == "ty") { return "Tahitian"; }
		if (code == "ug") { return "Uighur"; }
		if (code == "uk") { return "Ukrainian"; }
		if (code == "ur") { return "Urdu"; }
		if (code == "uz") { return "Uzbek"; }
		if (code == "ve") { return "Venda"; }
		if (code == "vi") { return "Vietnamese"; }
		if (code == "vo") { return "Volapük"; }
		if (code == "wa") { return "Walloon"; }
		if (code == "wo") { return "Wolof"; }
		if (code == "xh") { return "Xhosa"; }
		if (code == "yi") { return "Yiddish"; }
		if (code == "yo") { return "Yoruba"; }
		if (code == "za") { return "Zhuang"; }
		if (code == "zh") { return "Chinese"; }
		if (code == "zu") { return "Zulu"; }

	} else if (code.size() == 3) {
		// ISO 639-2 language codes

		if (code == "aar") { return "Afar"; }
		if (code == "abk") { return "Abkhazian"; }
		if (code == "ace") { return "Achinese"; }
		if (code == "ach") { return "Acoli"; }
		if (code == "ada") { return "Adangme"; }
		if (code == "ady") { return "Adyghe"; }
		if (code == "afa") { return "Afro-Asiatic languages"; }
		if (code == "afh") { return "Afrihili"; }
		if (code == "afr") { return "Afrikaans"; }
		if (code == "ain") { return "Ainu"; }
		if (code == "aka") { return "Akan"; }
		if (code == "akk") { return "Akkadian"; }
		if (code == "alb") { return "Albanian"; }
		if (code == "ale") { return "Aleut"; }
		if (code == "alg") { return "Algonquian languages"; }
		if (code == "alt") { return "Southern Altai"; }
		if (code == "amh") { return "Amharic"; }
		if (code == "ang") { return "English, Old (ca.450-1100)"; }
		if (code == "anp") { return "Angika"; }
		if (code == "apa") { return "Apache languages"; }
		if (code == "ara") { return "Arabic"; }
		if (code == "arc") { return "Aramaic (700-300 BCE)"; }
		if (code == "arg") { return "Aragonese"; }
		if (code == "arm") { return "Armenian"; }
		if (code == "arn") { return "Mapudungun"; }
		if (code == "arp") { return "Arapaho"; }
		if (code == "art") { return "Artificial languages"; }
		if (code == "arw") { return "Arawak"; }
		if (code == "asm") { return "Assamese"; }
		if (code == "ast") { return "Asturian"; }
		if (code == "ath") { return "Athapascan languages"; }
		if (code == "aus") { return "Australian languages"; }
		if (code == "ava") { return "Avaric"; }
		if (code == "ave") { return "Avestan"; }
		if (code == "awa") { return "Awadhi"; }
		if (code == "aym") { return "Aymara"; }
		if (code == "aze") { return "Azerbaijani"; }
		if (code == "bad") { return "Banda languages"; }
		if (code == "bai") { return "Bamileke languages"; }
		if (code == "bak") { return "Bashkir"; }
		if (code == "bal") { return "Baluchi"; }
		if (code == "bam") { return "Bambara"; }
		if (code == "ban") { return "Balinese"; }
		if (code == "baq") { return "Basque"; }
		if (code == "baq") { return "Basque"; }
		if (code == "bas") { return "Basa"; }
		if (code == "bat") { return "Baltic languages"; }
		if (code == "bej") { return "Beja"; }
		if (code == "bel") { return "Belarusian"; }
		if (code == "bem") { return "Bemba"; }
		if (code == "ben") { return "Bengali"; }
		if (code == "ber") { return "Berber languages"; }
		if (code == "bho") { return "Bhojpuri"; }
		if (code == "bih") { return "Bihari languages"; }
		if (code == "bik") { return "Bikol"; }
		if (code == "bin") { return "Bini"; }
		if (code == "bis") { return "Bislama"; }
		if (code == "bla") { return "Siksika"; }
		if (code == "bnt") { return "Bantu languages"; }
		if (code == "bod") { return "Tibetan"; }
		if (code == "bos") { return "Bosnian"; }
		if (code == "bra") { return "Braj"; }
		if (code == "bre") { return "Breton"; }
		if (code == "btk") { return "Batak languages"; }
		if (code == "bua") { return "Buriat"; }
		if (code == "bug") { return "Buginese"; }
		if (code == "bul") { return "Bulgarian"; }
		if (code == "bur") { return "Burmese"; }
		if (code == "bur") { return "Burmese"; }
		if (code == "byn") { return "Blin"; }
		if (code == "cad") { return "Caddo"; }
		if (code == "cai") { return "Central American Indian languages"; }
		if (code == "car") { return "Galibi Carib"; }
		if (code == "cat") { return "Catalan"; }
		if (code == "cau") { return "Caucasian languages"; }
		if (code == "ceb") { return "Cebuano"; }
		if (code == "cel") { return "Celtic languages"; }
		if (code == "ces") { return "Czech"; }
		if (code == "ces") { return "Czech"; }
		if (code == "cha") { return "Chamorro"; }
		if (code == "chb") { return "Chibcha"; }
		if (code == "che") { return "Chechen"; }
		if (code == "chg") { return "Chagatai"; }
		if (code == "chi") { return "Chinese"; }
		if (code == "chk") { return "Chuukese"; }
		if (code == "chm") { return "Mari"; }
		if (code == "chn") { return "Chinook jargon"; }
		if (code == "cho") { return "Choctaw"; }
		if (code == "chp") { return "Chipewyan"; }
		if (code == "chr") { return "Cherokee"; }
		if (code == "chu") { return "Church Slavic"; }
		if (code == "chv") { return "Chuvash"; }
		if (code == "chy") { return "Cheyenne"; }
		if (code == "cmc") { return "Chamic languages"; }
		if (code == "cnr") { return "Montenegrin"; }
		if (code == "cop") { return "Coptic"; }
		if (code == "cor") { return "Cornish"; }
		if (code == "cos") { return "Corsican"; }
		if (code == "cpe") { return "Creoles and pidgins, English based"; }
		if (code == "cpf") { return "Creoles and pidgins, French-based"; }
		if (code == "cpp") { return "Creoles and pidgins, Portuguese-based"; }
		if (code == "cre") { return "Cree"; }
		if (code == "crh") { return "Crimean Tatar"; }
		if (code == "crp") { return "Creoles and pidgins"; }
		if (code == "csb") { return "Kashubian"; }
		if (code == "cus") { return "Cushitic languages"; }
		if (code == "cym") { return "Welsh"; }
		if (code == "cym") { return "Welsh"; }
		if (code == "cze") { return "Czech"; }
		if (code == "cze") { return "Czech"; }
		if (code == "dak") { return "Dakota"; }
		if (code == "dan") { return "Danish"; }
		if (code == "dar") { return "Dargwa"; }
		if (code == "day") { return "Land Dayak languages"; }
		if (code == "del") { return "Delaware"; }
		if (code == "den") { return "Slave (Athapascan)"; }
		if (code == "deu") { return "German"; }
		if (code == "dgr") { return "Dogrib"; }
		if (code == "din") { return "Dinka"; }
		if (code == "div") { return "Divehi"; }
		if (code == "doi") { return "Dogri"; }
		if (code == "dra") { return "Dravidian languages"; }
		if (code == "dsb") { return "Lower Sorbian"; }
		if (code == "dua") { return "Duala"; }
		if (code == "dum") { return "Dutch, Middle (ca.1050-1350)"; }
		if (code == "dut") { return "Dutch"; }
		if (code == "dut") { return "Dutch"; }
		if (code == "dyu") { return "Dyula"; }
		if (code == "dzo") { return "Dzongkha"; }
		if (code == "efi") { return "Efik"; }
		if (code == "egy") { return "Egyptian (Ancient)"; }
		if (code == "eka") { return "Ekajuk"; }
		if (code == "ell") { return "Greek, Modern (1453-)"; }
		if (code == "elx") { return "Elamite"; }
		if (code == "eng") { return "English"; }
		if (code == "enm") { return "English, Middle (1100-1500)"; }
		if (code == "epo") { return "Esperanto"; }
		if (code == "est") { return "Estonian"; }
		if (code == "eus") { return "Basque"; }
		if (code == "eus") { return "Basque"; }
		if (code == "ewe") { return "Ewe"; }
		if (code == "ewo") { return "Ewondo"; }
		if (code == "fan") { return "Fang"; }
		if (code == "fao") { return "Faroese"; }
		if (code == "fas") { return "Persian"; }
		if (code == "fat") { return "Fanti"; }
		if (code == "fij") { return "Fijian"; }
		if (code == "fil") { return "Filipino"; }
		if (code == "fin") { return "Finnish"; }
		if (code == "fiu") { return "Finno-Ugrian languages"; }
		if (code == "fon") { return "Fon"; }
		if (code == "fra") { return "French"; }
		if (code == "fre") { return "French"; }
		if (code == "frm") { return "French, Middle (ca.1400-1600)"; }
		if (code == "fro") { return "French, Old (842-ca.1400)"; }
		if (code == "frr") { return "Northern Frisian"; }
		if (code == "frs") { return "Eastern Frisian"; }
		if (code == "fry") { return "Western Frisian"; }
		if (code == "ful") { return "Fulah"; }
		if (code == "fur") { return "Friulian"; }
		if (code == "gaa") { return "Ga"; }
		if (code == "gay") { return "Gayo"; }
		if (code == "gba") { return "Gbaya"; }
		if (code == "gem") { return "Germanic languages"; }
		if (code == "geo") { return "Georgin"; }
		if (code == "ger") { return "German"; }
		if (code == "gez") { return "Geez"; }
		if (code == "gil") { return "Gilbertese"; }
		if (code == "gla") { return "Gaelic"; }
		if (code == "gle") { return "Irish"; }
		if (code == "glg") { return "Galician"; }
		if (code == "glv") { return "Manx"; }
		if (code == "gmh") { return "German, Middle High (ca.1050-1500)"; }
		if (code == "goh") { return "German, Old High (ca.750-1050)"; }
		if (code == "gon") { return "Gondi"; }
		if (code == "gor") { return "Gorontalo"; }
		if (code == "got") { return "Gothic"; }
		if (code == "grb") { return "Grebo"; }
		if (code == "grc") { return "Greek, Ancient (to 1453)"; }
		if (code == "gre") { return "Greek"; }
		if (code == "grn") { return "Guarani"; }
		if (code == "gsw") { return "Swiss German"; }
		if (code == "guj") { return "Gujarati"; }
		if (code == "gwi") { return "Gwich'in"; }
		if (code == "hai") { return "Haida"; }
		if (code == "hat") { return "Haitian"; }
		if (code == "hau") { return "Hausa"; }
		if (code == "haw") { return "Hawaiian"; }
		if (code == "heb") { return "Hebrew"; }
		if (code == "her") { return "Herero"; }
		if (code == "hil") { return "Hiligaynon"; }
		if (code == "him") { return "Himachali languages"; }
		if (code == "hin") { return "Hindi"; }
		if (code == "hit") { return "Hittite"; }
		if (code == "hmn") { return "Hmong"; }
		if (code == "hmo") { return "Hiri Motu"; }
		if (code == "hrv") { return "Croatian"; }
		if (code == "hsb") { return "Upper Sorbian"; }
		if (code == "hun") { return "Hungarian"; }
		if (code == "hup") { return "Hupa"; }
		if (code == "hye") { return "Armenian"; }
		if (code == "iba") { return "Iban"; }
		if (code == "ibo") { return "Igbo"; }
		if (code == "ice") { return "Icelandic"; }
		if (code == "ido") { return "Ido"; }
		if (code == "iii") { return "Sichuan Yi"; }
		if (code == "ijo") { return "Ijo languages"; }
		if (code == "iku") { return "Inuktitut"; }
		if (code == "ile") { return "Interlingue"; }
		if (code == "ilo") { return "Iloko"; }
		if (code == "ina") { return "Interlingua)"; }
		if (code == "inc") { return "Indic languages"; }
		if (code == "ind") { return "Indonesian"; }
		if (code == "ine") { return "Indo-European languages"; }
		if (code == "inh") { return "Ingush"; }
		if (code == "ipk") { return "Inupiaq"; }
		if (code == "ira") { return "Iranian languages"; }
		if (code == "iro") { return "Iroquoian languages"; }
		if (code == "isl") { return "Icelandic"; }
		if (code == "ita") { return "Italian"; }
		if (code == "jav") { return "Javanese"; }
		if (code == "jbo") { return "Lojban"; }
		if (code == "jpn") { return "Japanese"; }
		if (code == "jpr") { return "Judeo-Persian"; }
		if (code == "jrb") { return "Judeo-Arabic"; }
		if (code == "kaa") { return "Kara-Kalpak"; }
		if (code == "kab") { return "Kabyle"; }
		if (code == "kac") { return "Kachin"; }
		if (code == "kal") { return "Greenlandic"; }
		if (code == "kam") { return "Kamba"; }
		if (code == "kan") { return "Kannada"; }
		if (code == "kar") { return "Karen languages"; }
		if (code == "kas") { return "Kashmiri"; }
		if (code == "kat") { return "Georgian"; }
		if (code == "kau") { return "Kanuri"; }
		if (code == "kaw") { return "Kawi"; }
		if (code == "kaz") { return "Kazakh"; }
		if (code == "kbd") { return "Kabardian"; }
		if (code == "kha") { return "Khasi"; }
		if (code == "khi") { return "Khoisan languages"; }
		if (code == "khm") { return "Central Khmer"; }
		if (code == "kho") { return "Khotanese"; }
		if (code == "kik") { return "Kikuyu"; }
		if (code == "kin") { return "Kinyarwanda"; }
		if (code == "kir") { return "Kirghiz"; }
		if (code == "kmb") { return "Kimbundu"; }
		if (code == "kok") { return "Konkani"; }
		if (code == "kom") { return "Komi"; }
		if (code == "kon") { return "Kongo"; }
		if (code == "kor") { return "Korean"; }
		if (code == "kos") { return "Kosraean"; }
		if (code == "kpe") { return "Kpelle"; }
		if (code == "krc") { return "Karachay-Balkar"; }
		if (code == "krl") { return "Karelian"; }
		if (code == "kro") { return "Kru languages"; }
		if (code == "kru") { return "Kurukh"; }
		if (code == "kua") { return "Kuanyama"; }
		if (code == "kum") { return "Kumyk"; }
		if (code == "kur") { return "Kurdish"; }
		if (code == "kut") { return "Kutenai"; }
		if (code == "lad") { return "Ladino"; }
		if (code == "lah") { return "Lahnda"; }
		if (code == "lam") { return "Lamba"; }
		if (code == "lao") { return "Lao"; }
		if (code == "lat") { return "Latin"; }
		if (code == "lav") { return "Latvian"; }
		if (code == "lez") { return "Lezghian"; }
		if (code == "lim") { return "Limburgan"; }
		if (code == "lin") { return "Lingala"; }
		if (code == "lit") { return "Lithuanian"; }
		if (code == "lol") { return "Mongo"; }
		if (code == "loz") { return "Lozi"; }
		if (code == "ltz") { return "Luxembourgish"; }
		if (code == "lua") { return "Luba-Lulua"; }
		if (code == "lub") { return "Luba-Katanga"; }
		if (code == "lug") { return "Ganda"; }
		if (code == "lui") { return "Luiseno"; }
		if (code == "lun") { return "Lunda"; }
		if (code == "luo") { return "Luo (Kenya and Tanzania)"; }
		if (code == "lus") { return "Lushai"; }
		if (code == "mac") { return "Macedonian"; }
		if (code == "mac") { return "Macedonian"; }
		if (code == "mad") { return "Madurese"; }
		if (code == "mag") { return "Magahi"; }
		if (code == "mah") { return "Marshallese"; }
		if (code == "mai") { return "Maithili"; }
		if (code == "mak") { return "Makasar"; }
		if (code == "mal") { return "Malayalam"; }
		if (code == "man") { return "Mandingo"; }
		if (code == "mao") { return "Maori"; }
		if (code == "map") { return "Austronesian languages"; }
		if (code == "mar") { return "Marathi"; }
		if (code == "mas") { return "Masai"; }
		if (code == "may") { return "Malay"; }
		if (code == "mdf") { return "Moksha"; }
		if (code == "mdr") { return "Mandar"; }
		if (code == "men") { return "Mende"; }
		if (code == "mga") { return "Irish, Middle (900-1200)"; }
		if (code == "mic") { return "Mi'kmaq"; }
		if (code == "min") { return "Minangkabau"; }
		if (code == "mis") { return "Uncoded languages"; }
		if (code == "mkd") { return "Macedonian"; }
		if (code == "mkd") { return "Macedonian"; }
		if (code == "mkh") { return "Mon-Khmer languages"; }
		if (code == "mlg") { return "Malagasy"; }
		if (code == "mlt") { return "Maltese"; }
		if (code == "mnc") { return "Manchu"; }
		if (code == "mni") { return "Manipuri"; }
		if (code == "mno") { return "Manobo languages"; }
		if (code == "moh") { return "Mohawk"; }
		if (code == "mon") { return "Mongolian"; }
		if (code == "mos") { return "Mossi"; }
		if (code == "mri") { return "Maori"; }
		if (code == "msa") { return "Malay"; }
		if (code == "mul") { return "Multiple languages"; }
		if (code == "mun") { return "Munda languages"; }
		if (code == "mus") { return "Creek"; }
		if (code == "mwl") { return "Mirandese"; }
		if (code == "mwr") { return "Marwari"; }
		if (code == "mya") { return "Burmese"; }
		if (code == "mya") { return "Burmese"; }
		if (code == "myn") { return "Mayan languages"; }
		if (code == "myv") { return "Erzya"; }
		if (code == "nah") { return "Nahuatl languages"; }
		if (code == "nai") { return "North American Indian languages"; }
		if (code == "nap") { return "Neapolitan"; }
		if (code == "nau") { return "Nauru"; }
		if (code == "nav") { return "Navajo"; }
		if (code == "nbl") { return "Ndebele, South"; }
		if (code == "nde") { return "Ndebele, North"; }
		if (code == "ndo") { return "Ndonga"; }
		if (code == "nds") { return "Low German"; }
		if (code == "nep") { return "Nepali"; }
		if (code == "new") { return "Nepal Bhasa"; }
		if (code == "nia") { return "Nias"; }
		if (code == "nic") { return "Niger-Kordofanian languages"; }
		if (code == "niu") { return "Niuean"; }
		if (code == "nld") { return "Dutch"; }
		if (code == "nld") { return "Dutch"; }
		if (code == "nno") { return "Norwegian Nynorsk"; }
		if (code == "nob") { return "Bokmål, Norwegian"; }
		if (code == "nog") { return "Nogai"; }
		if (code == "non") { return "Norse, Old"; }
		if (code == "nor") { return "Norwegian"; }
		if (code == "nqo") { return "N'Ko"; }
		if (code == "nso") { return "Pedi"; }
		if (code == "nub") { return "Nubian languages"; }
		if (code == "nwc") { return "Classical Newari"; }
		if (code == "nya") { return "Chichewa"; }
		if (code == "nym") { return "Nyamwezi"; }
		if (code == "nyn") { return "Nyankole"; }
		if (code == "nyo") { return "Nyoro"; }
		if (code == "nzi") { return "Nzima"; }
		if (code == "oci") { return "Occitan (post 1500)"; }
		if (code == "oji") { return "Ojibwa"; }
		if (code == "ori") { return "Oriya"; }
		if (code == "orm") { return "Oromo"; }
		if (code == "osa") { return "Osage"; }
		if (code == "oss") { return "Ossetian"; }
		if (code == "ota") { return "Turkish, Ottoman (1500-1928)"; }
		if (code == "oto") { return "Otomian languages"; }
		if (code == "paa") { return "Papuan languages"; }
		if (code == "pag") { return "Pangasinan"; }
		if (code == "pal") { return "Pahlavi"; }
		if (code == "pam") { return "Pampanga"; }
		if (code == "pan") { return "Panjabi"; }
		if (code == "pap") { return "Papiamento"; }
		if (code == "pau") { return "Palauan"; }
		if (code == "peo") { return "Persian, Old (ca.600-400 B.C.)"; }
		if (code == "per") { return "Persian"; }
		if (code == "phi") { return "Philippine languages"; }
		if (code == "phn") { return "Phoenician"; }
		if (code == "pli") { return "Pali"; }
		if (code == "pol") { return "Polish"; }
		if (code == "pon") { return "Pohnpeian"; }
		if (code == "por") { return "Portuguese"; }
		if (code == "pra") { return "Prakrit languages"; }
		if (code == "pro") { return "Provençal, Old (to 1500)"; }
		if (code == "pus") { return "Pushto"; }
		if (code == "que") { return "Quechua"; }
		if (code == "raj") { return "Rajasthani"; }
		if (code == "rap") { return "Rapanui"; }
		if (code == "rar") { return "Rarotongan"; }
		if (code == "roa") { return "Romance languages"; }
		if (code == "roh") { return "Romansh"; }
		if (code == "rom") { return "Romany"; }
		if (code == "ron") { return "Romanian"; }
		if (code == "rum") { return "Romanian"; }
		if (code == "run") { return "Rundi"; }
		if (code == "rup") { return "Aromanian"; }
		if (code == "rus") { return "Russian"; }
		if (code == "sad") { return "Sandawe"; }
		if (code == "sag") { return "Sango"; }
		if (code == "sah") { return "Yakut"; }
		if (code == "sai") { return "South American Indian languages"; }
		if (code == "sal") { return "Salishan languages"; }
		if (code == "sam") { return "Samaritan Aramaic"; }
		if (code == "san") { return "Sanskrit"; }
		if (code == "sas") { return "Sasak"; }
		if (code == "sat") { return "Santali"; }
		if (code == "scn") { return "Sicilian"; }
		if (code == "sco") { return "Scots"; }
		if (code == "sel") { return "Selkup"; }
		if (code == "sem") { return "Semitic languages"; }
		if (code == "sga") { return "Irish, Old (to 900)"; }
		if (code == "sgn") { return "Sign Languages"; }
		if (code == "shn") { return "Shan"; }
		if (code == "sid") { return "Sidamo"; }
		if (code == "sin") { return "Sinhala"; }
		if (code == "sio") { return "Siouan languages"; }
		if (code == "sit") { return "Sino-Tibetan languages"; }
		if (code == "sla") { return "Slavic languages"; }
		if (code == "slo") { return "Slovak"; }
		if (code == "slv") { return "Slovenian"; }
		if (code == "sma") { return "Southern Sami"; }
		if (code == "sme") { return "Northern Sami"; }
		if (code == "smi") { return "Sami languages"; }
		if (code == "smj") { return "Lule Sami"; }
		if (code == "smn") { return "Inari Sami"; }
		if (code == "smo") { return "Samoan"; }
		if (code == "sms") { return "Skolt Sami"; }
		if (code == "sna") { return "Shona"; }
		if (code == "snd") { return "Sindhi"; }
		if (code == "snk") { return "Soninke"; }
		if (code == "sog") { return "Sogdian"; }
		if (code == "som") { return "Somali"; }
		if (code == "son") { return "Songhai languages"; }
		if (code == "sot") { return "Sotho, Southern"; }
		if (code == "spa") { return "Spanish"; }
		if (code == "sqi") { return "Albanian"; }
		if (code == "srd") { return "Sardinian"; }
		if (code == "srn") { return "Sranan Tongo"; }
		if (code == "srp") { return "Serbian"; }
		if (code == "srr") { return "Serer"; }
		if (code == "ssa") { return "Nilo-Saharan languages"; }
		if (code == "ssw") { return "Swati"; }
		if (code == "suk") { return "Sukuma"; }
		if (code == "sun") { return "Sundanese"; }
		if (code == "sus") { return "Susu"; }
		if (code == "sux") { return "Sumerian"; }
		if (code == "swa") { return "Swahili"; }
		if (code == "swe") { return "Swedish"; }
		if (code == "syc") { return "Classical Syriac"; }
		if (code == "syr") { return "Syriac"; }
		if (code == "tah") { return "Tahitian"; }
		if (code == "tai") { return "Tai languages"; }
		if (code == "tam") { return "Tamil"; }
		if (code == "tat") { return "Tatar"; }
		if (code == "tel") { return "Telugu"; }
		if (code == "tem") { return "Timne"; }
		if (code == "ter") { return "Tereno"; }
		if (code == "tet") { return "Tetum"; }
		if (code == "tgk") { return "Tajik"; }
		if (code == "tgl") { return "Tagalog"; }
		if (code == "tha") { return "Thai"; }
		if (code == "tib") { return "Tibetian"; }
		if (code == "tig") { return "Tigre"; }
		if (code == "tir") { return "Tigrinya"; }
		if (code == "tiv") { return "Tiv"; }
		if (code == "tkl") { return "Tokelau"; }
		if (code == "tlh") { return "Klingon"; }
		if (code == "tli") { return "Tlingit"; }
		if (code == "tmh") { return "Tamashek"; }
		if (code == "tog") { return "Tonga (Nyasa)"; }
		if (code == "ton") { return "Tonga (Tonga Islands)"; }
		if (code == "tpi") { return "Tok Pisin"; }
		if (code == "tsi") { return "Tsimshian"; }
		if (code == "tsn") { return "Tswana"; }
		if (code == "tso") { return "Tsonga"; }
		if (code == "tuk") { return "Turkmen"; }
		if (code == "tum") { return "Tumbuka"; }
		if (code == "tup") { return "Tupi languages"; }
		if (code == "tur") { return "Turkish"; }
		if (code == "tut") { return "Altaic languages"; }
		if (code == "tvl") { return "Tuvalu"; }
		if (code == "twi") { return "Twi"; }
		if (code == "tyv") { return "Tuvinian"; }
		if (code == "udm") { return "Udmurt"; }
		if (code == "uga") { return "Ugaritic"; }
		if (code == "uig") { return "Uighur"; }
		if (code == "ukr") { return "Ukrainian"; }
		if (code == "umb") { return "Umbundu"; }
		if (code == "und") { return "Undetermined"; }
		if (code == "urd") { return "Urdu"; }
		if (code == "uzb") { return "Uzbek"; }
		if (code == "vai") { return "Vai"; }
		if (code == "ven") { return "Venda"; }
		if (code == "vie") { return "Vietnamese"; }
		if (code == "vol") { return "Volapük"; }
		if (code == "vot") { return "Votic"; }
		if (code == "wak") { return "Wakashan languages"; }
		if (code == "wal") { return "Wolaitta"; }
		if (code == "war") { return "Waray"; }
		if (code == "was") { return "Washo"; }
		if (code == "wel") { return "Welsh"; }
		if (code == "wel") { return "Welsh"; }
		if (code == "wen") { return "Sorbian languages"; }
		if (code == "wln") { return "Walloon"; }
		if (code == "wol") { return "Wolof"; }
		if (code == "xal") { return "Kalmyk"; }
		if (code == "xho") { return "Xhosa"; }
		if (code == "yao") { return "Yao"; }
		if (code == "yap") { return "Yapese"; }
		if (code == "yid") { return "Yiddish"; }
		if (code == "yor") { return "Yoruba"; }
		if (code == "ypk") { return "Yupik languages"; }
		if (code == "zap") { return "Zapotec"; }
		if (code == "zbl") { return "Blissymbols"; }
		if (code == "zen") { return "Zenaga"; }
		if (code == "zgh") { return "Moroccan"; }
		if (code == "zha") { return "Zhuang"; }
		if (code == "zho") { return "Chinese"; }
		if (code == "znd") { return "Zande languages"; }
		if (code == "zul") { return "Zulu"; }
		if (code == "zun") { return "Zuni"; }
		if (code == "zza") { return "Zaza"; }
	}
	return code;
}


// END_MERGE


// END_MERGE

} // end namespace hum



