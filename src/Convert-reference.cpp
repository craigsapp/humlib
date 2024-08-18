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
		code.push_back(toupper(abbreviation[i]));
	}

	if (code.size() == 2) {
		// ISO 639-1 language codes

		if (code == "AA") { return "Afar"; }
		if (code == "AB") { return "Abkhazian"; }
		if (code == "AE") { return "Avestan"; }
		if (code == "AF") { return "Afrikaans"; }
		if (code == "AK") { return "Akan"; }
		if (code == "AM") { return "Amharic"; }
		if (code == "AN") { return "Aragonese"; }
		if (code == "AR") { return "Arabic"; }
		if (code == "AS") { return "Assamese"; }
		if (code == "AV") { return "Avaric"; }
		if (code == "AY") { return "Aymara"; }
		if (code == "AZ") { return "Azerbaijani"; }
		if (code == "BA") { return "Bashkir"; }
		if (code == "BE") { return "Belarusian"; }
		if (code == "BG") { return "Bulgarian"; }
		if (code == "BH") { return "Bihari languages"; }
		if (code == "BI") { return "Bislama"; }
		if (code == "BM") { return "Bambara"; }
		if (code == "BN") { return "Bengali"; }
		if (code == "BO") { return "Tibetan"; }
		if (code == "BR") { return "Breton"; }
		if (code == "BS") { return "Bosnian"; }
		if (code == "CA") { return "Catalan"; }
		if (code == "CE") { return "Chechen"; }
		if (code == "CH") { return "Chamorro"; }
		if (code == "CO") { return "Corsican"; }
		if (code == "CR") { return "Cree"; }
		if (code == "CS") { return "Czech"; }
		if (code == "CS") { return "Czech"; }
		if (code == "CU") { return "Church Slavic"; }
		if (code == "CV") { return "Chuvash"; }
		if (code == "CY") { return "Welsh"; }
		if (code == "CY") { return "Welsh"; }
		if (code == "DA") { return "Danish"; }
		if (code == "DE") { return "German"; }
		if (code == "DV") { return "Divehi"; }
		if (code == "DZ") { return "Dzongkha"; }
		if (code == "EE") { return "Ewe"; }
		if (code == "EL") { return "Greek, Modern (1453-)"; }
		if (code == "EN") { return "English"; }
		if (code == "EO") { return "Esperanto"; }
		if (code == "ES") { return "Spanish"; }
		if (code == "ET") { return "Estonian"; }
		if (code == "EU") { return "Basque"; }
		if (code == "EU") { return "Basque"; }
		if (code == "FA") { return "Persian"; }
		if (code == "FF") { return "Fulah"; }
		if (code == "FI") { return "Finnish"; }
		if (code == "FJ") { return "Fijian"; }
		if (code == "FO") { return "Faroese"; }
		if (code == "FR") { return "French"; }
		if (code == "FY") { return "Western Frisian"; }
		if (code == "GA") { return "Irish"; }
		if (code == "GD") { return "Gaelic"; }
		if (code == "GL") { return "Galician"; }
		if (code == "GN") { return "Guarani"; }
		if (code == "GU") { return "Gujarati"; }
		if (code == "GV") { return "Manx"; }
		if (code == "HA") { return "Hausa"; }
		if (code == "HE") { return "Hebrew"; }
		if (code == "HI") { return "Hindi"; }
		if (code == "HO") { return "Hiri Motu"; }
		if (code == "HR") { return "Croatian"; }
		if (code == "HT") { return "Haitian"; }
		if (code == "HU") { return "Hungarian"; }
		if (code == "HY") { return "Armenian"; }
		if (code == "HZ") { return "Herero"; }
		if (code == "IA") { return "Interlingua"; }
		if (code == "ID") { return "Indonesian"; }
		if (code == "IE") { return "Interlingue"; }
		if (code == "IG") { return "Igbo"; }
		if (code == "II") { return "Sichuan Yi"; }
		if (code == "IK") { return "Inupiaq"; }
		if (code == "IO") { return "Ido"; }
		if (code == "IS") { return "Icelandic"; }
		if (code == "IT") { return "Italian"; }
		if (code == "IU") { return "Inuktitut"; }
		if (code == "JA") { return "Japanese"; }
		if (code == "JV") { return "Javanese"; }
		if (code == "KA") { return "Georgian"; }
		if (code == "KG") { return "Kongo"; }
		if (code == "KI") { return "Kikuyu"; }
		if (code == "KJ") { return "Kuanyama"; }
		if (code == "KK") { return "Kazakh"; }
		if (code == "KL") { return "Greenlandic"; }
		if (code == "KM") { return "Central Khmer"; }
		if (code == "KN") { return "Kannada"; }
		if (code == "KO") { return "Korean"; }
		if (code == "KR") { return "Kanuri"; }
		if (code == "KS") { return "Kashmiri"; }
		if (code == "KU") { return "Kurdish"; }
		if (code == "KV") { return "Komi"; }
		if (code == "KW") { return "Cornish"; }
		if (code == "KY") { return "Kirghiz"; }
		if (code == "LA") { return "Latin"; }
		if (code == "LB") { return "Luxembourgish"; }
		if (code == "LG") { return "Ganda"; }
		if (code == "LI") { return "Limburgan"; }
		if (code == "LN") { return "Lingala"; }
		if (code == "LO") { return "Lao"; }
		if (code == "LT") { return "Lithuanian"; }
		if (code == "LU") { return "Luba-Katanga"; }
		if (code == "LV") { return "Latvian"; }
		if (code == "MG") { return "Malagasy"; }
		if (code == "MH") { return "Marshallese"; }
		if (code == "MI") { return "Maori"; }
		if (code == "MK") { return "Macedonian"; }
		if (code == "MK") { return "Macedonian"; }
		if (code == "ML") { return "Malayalam"; }
		if (code == "MN") { return "Mongolian"; }
		if (code == "MR") { return "Marathi"; }
		if (code == "MS") { return "Malay"; }
		if (code == "MT") { return "Maltese"; }
		if (code == "MY") { return "Burmese"; }
		if (code == "MY") { return "Burmese"; }
		if (code == "NA") { return "Nauru"; }
		if (code == "NB") { return "Bokmål, Norwegian"; }
		if (code == "ND") { return "Ndebele, North"; }
		if (code == "NE") { return "Nepali"; }
		if (code == "NG") { return "Ndonga"; }
		if (code == "NL") { return "Dutch"; }
		if (code == "NL") { return "Dutch"; }
		if (code == "NN") { return "Norwegian Nynorsk"; }
		if (code == "NO") { return "Norwegian"; }
		if (code == "NR") { return "Ndebele, South"; }
		if (code == "NV") { return "Navajo"; }
		if (code == "NY") { return "Chichewa"; }
		if (code == "OC") { return "Occitan (post 1500)"; }
		if (code == "OJ") { return "Ojibwa"; }
		if (code == "OM") { return "Oromo"; }
		if (code == "OR") { return "Oriya"; }
		if (code == "OS") { return "Ossetian"; }
		if (code == "PA") { return "Panjabi"; }
		if (code == "PI") { return "Pali"; }
		if (code == "PL") { return "Polish"; }
		if (code == "PS") { return "Pushto"; }
		if (code == "PT") { return "Portuguese"; }
		if (code == "QU") { return "Quechua"; }
		if (code == "RM") { return "Romansh"; }
		if (code == "RN") { return "Rundi"; }
		if (code == "RO") { return "Romanian"; }
		if (code == "RU") { return "Russian"; }
		if (code == "RW") { return "Kinyarwanda"; }
		if (code == "SA") { return "Sanskrit"; }
		if (code == "SC") { return "Sardinian"; }
		if (code == "SD") { return "Sindhi"; }
		if (code == "SE") { return "Northern Sami"; }
		if (code == "SG") { return "Sango"; }
		if (code == "SI") { return "Sinhala"; }
		if (code == "SL") { return "Slovenian"; }
		if (code == "SM") { return "Samoan"; }
		if (code == "SN") { return "Shona"; }
		if (code == "SO") { return "Somali"; }
		if (code == "SQ") { return "Albanian"; }
		if (code == "SR") { return "Serbian"; }
		if (code == "SS") { return "Swati"; }
		if (code == "ST") { return "Sotho, Southern"; }
		if (code == "SU") { return "Sundanese"; }
		if (code == "SV") { return "Swedish"; }
		if (code == "SW") { return "Swahili"; }
		if (code == "TA") { return "Tamil"; }
		if (code == "TE") { return "Telugu"; }
		if (code == "TG") { return "Tajik"; }
		if (code == "TH") { return "Thai"; }
		if (code == "TI") { return "Tigrinya"; }
		if (code == "TK") { return "Turkmen"; }
		if (code == "TL") { return "Tagalog"; }
		if (code == "TN") { return "Tswana"; }
		if (code == "TO") { return "Tonga (Tonga Islands)"; }
		if (code == "TR") { return "Turkish"; }
		if (code == "TS") { return "Tsonga"; }
		if (code == "TT") { return "Tatar"; }
		if (code == "TW") { return "Twi"; }
		if (code == "TY") { return "Tahitian"; }
		if (code == "UG") { return "Uighur"; }
		if (code == "UK") { return "Ukrainian"; }
		if (code == "UR") { return "Urdu"; }
		if (code == "UZ") { return "Uzbek"; }
		if (code == "VE") { return "Venda"; }
		if (code == "VI") { return "Vietnamese"; }
		if (code == "VO") { return "Volapük"; }
		if (code == "WA") { return "Walloon"; }
		if (code == "WO") { return "Wolof"; }
		if (code == "XH") { return "Xhosa"; }
		if (code == "YI") { return "Yiddish"; }
		if (code == "YO") { return "Yoruba"; }
		if (code == "ZA") { return "Zhuang"; }
		if (code == "ZH") { return "Chinese"; }
		if (code == "ZU") { return "Zulu"; }

	} else if (code.size() == 3) {
		// ISO 639-2 language codes

		if (code == "AAR") { return "Afar"; }
		if (code == "ABK") { return "Abkhazian"; }
		if (code == "ACE") { return "Achinese"; }
		if (code == "ACH") { return "Acoli"; }
		if (code == "ADA") { return "Adangme"; }
		if (code == "ADY") { return "Adyghe"; }
		if (code == "AFA") { return "Afro-Asiatic languages"; }
		if (code == "AFH") { return "Afrihili"; }
		if (code == "AFR") { return "Afrikaans"; }
		if (code == "AIN") { return "Ainu"; }
		if (code == "AKA") { return "Akan"; }
		if (code == "AKK") { return "Akkadian"; }
		if (code == "ALB") { return "Albanian"; }
		if (code == "ALE") { return "Aleut"; }
		if (code == "ALG") { return "Algonquian languages"; }
		if (code == "ALT") { return "Southern Altai"; }
		if (code == "AMH") { return "Amharic"; }
		if (code == "ANG") { return "English, Old (ca.450-1100)"; }
		if (code == "ANP") { return "Angika"; }
		if (code == "APA") { return "Apache languages"; }
		if (code == "ARA") { return "Arabic"; }
		if (code == "ARC") { return "Aramaic (700-300 BCE)"; }
		if (code == "ARG") { return "Aragonese"; }
		if (code == "ARM") { return "Armenian"; }
		if (code == "ARN") { return "Mapudungun"; }
		if (code == "ARP") { return "Arapaho"; }
		if (code == "ART") { return "Artificial languages"; }
		if (code == "ARW") { return "Arawak"; }
		if (code == "ASM") { return "Assamese"; }
		if (code == "AST") { return "Asturian"; }
		if (code == "ATH") { return "Athapascan languages"; }
		if (code == "AUS") { return "Australian languages"; }
		if (code == "AVA") { return "Avaric"; }
		if (code == "AVE") { return "Avestan"; }
		if (code == "AWA") { return "Awadhi"; }
		if (code == "AYM") { return "Aymara"; }
		if (code == "AZE") { return "Azerbaijani"; }
		if (code == "BAD") { return "Banda languages"; }
		if (code == "BAI") { return "Bamileke languages"; }
		if (code == "BAK") { return "Bashkir"; }
		if (code == "BAL") { return "Baluchi"; }
		if (code == "BAM") { return "Bambara"; }
		if (code == "BAN") { return "Balinese"; }
		if (code == "BAQ") { return "Basque"; }
		if (code == "BAQ") { return "Basque"; }
		if (code == "BAS") { return "Basa"; }
		if (code == "BAT") { return "Baltic languages"; }
		if (code == "BEJ") { return "Beja"; }
		if (code == "BEL") { return "Belarusian"; }
		if (code == "BEM") { return "Bemba"; }
		if (code == "BEN") { return "Bengali"; }
		if (code == "BER") { return "Berber languages"; }
		if (code == "BHO") { return "Bhojpuri"; }
		if (code == "BIH") { return "Bihari languages"; }
		if (code == "BIK") { return "Bikol"; }
		if (code == "BIN") { return "Bini"; }
		if (code == "BIS") { return "Bislama"; }
		if (code == "BLA") { return "Siksika"; }
		if (code == "BNT") { return "Bantu languages"; }
		if (code == "BOD") { return "Tibetan"; }
		if (code == "BOS") { return "Bosnian"; }
		if (code == "BRA") { return "Braj"; }
		if (code == "BRE") { return "Breton"; }
		if (code == "BTK") { return "Batak languages"; }
		if (code == "BUA") { return "Buriat"; }
		if (code == "BUG") { return "Buginese"; }
		if (code == "BUL") { return "Bulgarian"; }
		if (code == "BUR") { return "Burmese"; }
		if (code == "BUR") { return "Burmese"; }
		if (code == "BYN") { return "Blin"; }
		if (code == "CAD") { return "Caddo"; }
		if (code == "CAI") { return "Central American Indian languages"; }
		if (code == "CAR") { return "Galibi Carib"; }
		if (code == "CAT") { return "Catalan"; }
		if (code == "CAU") { return "Caucasian languages"; }
		if (code == "CEB") { return "Cebuano"; }
		if (code == "CEL") { return "Celtic languages"; }
		if (code == "CES") { return "Czech"; }
		if (code == "CES") { return "Czech"; }
		if (code == "CHA") { return "Chamorro"; }
		if (code == "CHB") { return "Chibcha"; }
		if (code == "CHE") { return "Chechen"; }
		if (code == "CHG") { return "Chagatai"; }
		if (code == "CHI") { return "Chinese"; }
		if (code == "CHK") { return "Chuukese"; }
		if (code == "CHM") { return "Mari"; }
		if (code == "CHN") { return "Chinook jargon"; }
		if (code == "CHO") { return "Choctaw"; }
		if (code == "CHP") { return "Chipewyan"; }
		if (code == "CHR") { return "Cherokee"; }
		if (code == "CHU") { return "Church Slavic"; }
		if (code == "CHV") { return "Chuvash"; }
		if (code == "CHY") { return "Cheyenne"; }
		if (code == "CMC") { return "Chamic languages"; }
		if (code == "CNR") { return "Montenegrin"; }
		if (code == "COP") { return "Coptic"; }
		if (code == "COR") { return "Cornish"; }
		if (code == "COS") { return "Corsican"; }
		if (code == "CPE") { return "Creoles and pidgins, English based"; }
		if (code == "CPF") { return "Creoles and pidgins, French-based"; }
		if (code == "CPP") { return "Creoles and pidgins, Portuguese-based"; }
		if (code == "CRE") { return "Cree"; }
		if (code == "CRH") { return "Crimean Tatar"; }
		if (code == "CRP") { return "Creoles and pidgins"; }
		if (code == "CSB") { return "Kashubian"; }
		if (code == "CUS") { return "Cushitic languages"; }
		if (code == "CYM") { return "Welsh"; }
		if (code == "CYM") { return "Welsh"; }
		if (code == "CZE") { return "Czech"; }
		if (code == "CZE") { return "Czech"; }
		if (code == "DAK") { return "Dakota"; }
		if (code == "DAN") { return "Danish"; }
		if (code == "DAR") { return "Dargwa"; }
		if (code == "DAY") { return "Land Dayak languages"; }
		if (code == "DEL") { return "Delaware"; }
		if (code == "DEN") { return "Slave (Athapascan)"; }
		if (code == "DEU") { return "German"; }
		if (code == "DGR") { return "Dogrib"; }
		if (code == "DIN") { return "Dinka"; }
		if (code == "DIV") { return "Divehi"; }
		if (code == "DOI") { return "Dogri"; }
		if (code == "DRA") { return "Dravidian languages"; }
		if (code == "DSB") { return "Lower Sorbian"; }
		if (code == "DUA") { return "Duala"; }
		if (code == "DUM") { return "Dutch, Middle (ca.1050-1350)"; }
		if (code == "DUT") { return "Dutch"; }
		if (code == "DUT") { return "Dutch"; }
		if (code == "DYU") { return "Dyula"; }
		if (code == "DZO") { return "Dzongkha"; }
		if (code == "EFI") { return "Efik"; }
		if (code == "EGY") { return "Egyptian (Ancient)"; }
		if (code == "EKA") { return "Ekajuk"; }
		if (code == "ELL") { return "Greek, Modern (1453-)"; }
		if (code == "ELX") { return "Elamite"; }
		if (code == "ENG") { return "English"; }
		if (code == "ENM") { return "English, Middle (1100-1500)"; }
		if (code == "EPO") { return "Esperanto"; }
		if (code == "EST") { return "Estonian"; }
		if (code == "EUS") { return "Basque"; }
		if (code == "EUS") { return "Basque"; }
		if (code == "EWE") { return "Ewe"; }
		if (code == "EWO") { return "Ewondo"; }
		if (code == "FAN") { return "Fang"; }
		if (code == "FAO") { return "Faroese"; }
		if (code == "FAS") { return "Persian"; }
		if (code == "FAT") { return "Fanti"; }
		if (code == "FIJ") { return "Fijian"; }
		if (code == "FIL") { return "Filipino"; }
		if (code == "FIN") { return "Finnish"; }
		if (code == "FIU") { return "Finno-Ugrian languages"; }
		if (code == "FON") { return "Fon"; }
		if (code == "FRA") { return "French"; }
		if (code == "FRE") { return "French"; }
		if (code == "FRM") { return "French, Middle (ca.1400-1600)"; }
		if (code == "FRO") { return "French, Old (842-ca.1400)"; }
		if (code == "FRR") { return "Northern Frisian"; }
		if (code == "FRS") { return "Eastern Frisian"; }
		if (code == "FRY") { return "Western Frisian"; }
		if (code == "FUL") { return "Fulah"; }
		if (code == "FUR") { return "Friulian"; }
		if (code == "GAA") { return "Ga"; }
		if (code == "GAY") { return "Gayo"; }
		if (code == "GBA") { return "Gbaya"; }
		if (code == "GEM") { return "Germanic languages"; }
		if (code == "GEO") { return "Georgin"; }
		if (code == "GER") { return "German"; }
		if (code == "GEZ") { return "Geez"; }
		if (code == "GIL") { return "Gilbertese"; }
		if (code == "GLA") { return "Gaelic"; }
		if (code == "GLE") { return "Irish"; }
		if (code == "GLG") { return "Galician"; }
		if (code == "GLV") { return "Manx"; }
		if (code == "GMH") { return "German, Middle High (ca.1050-1500)"; }
		if (code == "GOH") { return "German, Old High (ca.750-1050)"; }
		if (code == "GON") { return "Gondi"; }
		if (code == "GOR") { return "Gorontalo"; }
		if (code == "GOT") { return "Gothic"; }
		if (code == "GRB") { return "Grebo"; }
		if (code == "GRC") { return "Greek, Ancient (to 1453)"; }
		if (code == "GRE") { return "Greek"; }
		if (code == "GRN") { return "Guarani"; }
		if (code == "GSW") { return "Swiss German"; }
		if (code == "GUJ") { return "Gujarati"; }
		if (code == "GWI") { return "Gwich'in"; }
		if (code == "HAI") { return "Haida"; }
		if (code == "HAT") { return "Haitian"; }
		if (code == "HAU") { return "Hausa"; }
		if (code == "HAW") { return "Hawaiian"; }
		if (code == "HEB") { return "Hebrew"; }
		if (code == "HER") { return "Herero"; }
		if (code == "HIL") { return "Hiligaynon"; }
		if (code == "HIM") { return "Himachali languages"; }
		if (code == "HIN") { return "Hindi"; }
		if (code == "HIT") { return "Hittite"; }
		if (code == "HMN") { return "Hmong"; }
		if (code == "HMO") { return "Hiri Motu"; }
		if (code == "HRV") { return "Croatian"; }
		if (code == "HSB") { return "Upper Sorbian"; }
		if (code == "HUN") { return "Hungarian"; }
		if (code == "HUP") { return "Hupa"; }
		if (code == "HYE") { return "Armenian"; }
		if (code == "IBA") { return "Iban"; }
		if (code == "IBO") { return "Igbo"; }
		if (code == "ICE") { return "Icelandic"; }
		if (code == "IDO") { return "Ido"; }
		if (code == "III") { return "Sichuan Yi"; }
		if (code == "IJO") { return "Ijo languages"; }
		if (code == "IKU") { return "Inuktitut"; }
		if (code == "ILE") { return "Interlingue"; }
		if (code == "ILO") { return "Iloko"; }
		if (code == "INA") { return "Interlingua)"; }
		if (code == "INC") { return "Indic languages"; }
		if (code == "IND") { return "Indonesian"; }
		if (code == "INE") { return "Indo-European languages"; }
		if (code == "INH") { return "Ingush"; }
		if (code == "IPK") { return "Inupiaq"; }
		if (code == "IRA") { return "Iranian languages"; }
		if (code == "IRO") { return "Iroquoian languages"; }
		if (code == "ISL") { return "Icelandic"; }
		if (code == "ITA") { return "Italian"; }
		if (code == "JAV") { return "Javanese"; }
		if (code == "JBO") { return "Lojban"; }
		if (code == "JPN") { return "Japanese"; }
		if (code == "JPR") { return "Judeo-Persian"; }
		if (code == "JRB") { return "Judeo-Arabic"; }
		if (code == "KAA") { return "Kara-Kalpak"; }
		if (code == "KAB") { return "Kabyle"; }
		if (code == "KAC") { return "Kachin"; }
		if (code == "KAL") { return "Greenlandic"; }
		if (code == "KAM") { return "Kamba"; }
		if (code == "KAN") { return "Kannada"; }
		if (code == "KAR") { return "Karen languages"; }
		if (code == "KAS") { return "Kashmiri"; }
		if (code == "KAT") { return "Georgian"; }
		if (code == "KAU") { return "Kanuri"; }
		if (code == "KAW") { return "Kawi"; }
		if (code == "KAZ") { return "Kazakh"; }
		if (code == "KBD") { return "Kabardian"; }
		if (code == "KHA") { return "Khasi"; }
		if (code == "KHI") { return "Khoisan languages"; }
		if (code == "KHM") { return "Central Khmer"; }
		if (code == "KHO") { return "Khotanese"; }
		if (code == "KIK") { return "Kikuyu"; }
		if (code == "KIN") { return "Kinyarwanda"; }
		if (code == "KIR") { return "Kirghiz"; }
		if (code == "KMB") { return "Kimbundu"; }
		if (code == "KOK") { return "Konkani"; }
		if (code == "KOM") { return "Komi"; }
		if (code == "KON") { return "Kongo"; }
		if (code == "KOR") { return "Korean"; }
		if (code == "KOS") { return "Kosraean"; }
		if (code == "KPE") { return "Kpelle"; }
		if (code == "KRC") { return "Karachay-Balkar"; }
		if (code == "KRL") { return "Karelian"; }
		if (code == "KRO") { return "Kru languages"; }
		if (code == "KRU") { return "Kurukh"; }
		if (code == "KUA") { return "Kuanyama"; }
		if (code == "KUM") { return "Kumyk"; }
		if (code == "KUR") { return "Kurdish"; }
		if (code == "KUT") { return "Kutenai"; }
		if (code == "LAD") { return "Ladino"; }
		if (code == "LAH") { return "Lahnda"; }
		if (code == "LAM") { return "Lamba"; }
		if (code == "LAO") { return "Lao"; }
		if (code == "LAT") { return "Latin"; }
		if (code == "LAV") { return "Latvian"; }
		if (code == "LEZ") { return "Lezghian"; }
		if (code == "LIM") { return "Limburgan"; }
		if (code == "LIN") { return "Lingala"; }
		if (code == "LIT") { return "Lithuanian"; }
		if (code == "LOL") { return "Mongo"; }
		if (code == "LOZ") { return "Lozi"; }
		if (code == "LTZ") { return "Luxembourgish"; }
		if (code == "LUA") { return "Luba-Lulua"; }
		if (code == "LUB") { return "Luba-Katanga"; }
		if (code == "LUG") { return "Ganda"; }
		if (code == "LUI") { return "Luiseno"; }
		if (code == "LUN") { return "Lunda"; }
		if (code == "LUO") { return "Luo (Kenya and Tanzania)"; }
		if (code == "LUS") { return "Lushai"; }
		if (code == "MAC") { return "Macedonian"; }
		if (code == "MAC") { return "Macedonian"; }
		if (code == "MAD") { return "Madurese"; }
		if (code == "MAG") { return "Magahi"; }
		if (code == "MAH") { return "Marshallese"; }
		if (code == "MAI") { return "Maithili"; }
		if (code == "MAK") { return "Makasar"; }
		if (code == "MAL") { return "Malayalam"; }
		if (code == "MAN") { return "Mandingo"; }
		if (code == "MAO") { return "Maori"; }
		if (code == "MAP") { return "Austronesian languages"; }
		if (code == "MAR") { return "Marathi"; }
		if (code == "MAS") { return "Masai"; }
		if (code == "MAY") { return "Malay"; }
		if (code == "MDF") { return "Moksha"; }
		if (code == "MDR") { return "Mandar"; }
		if (code == "MEN") { return "Mende"; }
		if (code == "MGA") { return "Irish, Middle (900-1200)"; }
		if (code == "MIC") { return "Mi'kmaq"; }
		if (code == "MIN") { return "Minangkabau"; }
		if (code == "MIS") { return "Uncoded languages"; }
		if (code == "MKD") { return "Macedonian"; }
		if (code == "MKD") { return "Macedonian"; }
		if (code == "MKH") { return "Mon-Khmer languages"; }
		if (code == "MLG") { return "Malagasy"; }
		if (code == "MLT") { return "Maltese"; }
		if (code == "MNC") { return "Manchu"; }
		if (code == "MNI") { return "Manipuri"; }
		if (code == "MNO") { return "Manobo languages"; }
		if (code == "MOH") { return "Mohawk"; }
		if (code == "MON") { return "Mongolian"; }
		if (code == "MOS") { return "Mossi"; }
		if (code == "MRI") { return "Maori"; }
		if (code == "MSA") { return "Malay"; }
		if (code == "MUL") { return "Multiple languages"; }
		if (code == "MUN") { return "Munda languages"; }
		if (code == "MUS") { return "Creek"; }
		if (code == "MWL") { return "Mirandese"; }
		if (code == "MWR") { return "Marwari"; }
		if (code == "MYA") { return "Burmese"; }
		if (code == "MYA") { return "Burmese"; }
		if (code == "MYN") { return "Mayan languages"; }
		if (code == "MYV") { return "Erzya"; }
		if (code == "NAH") { return "Nahuatl languages"; }
		if (code == "NAI") { return "North American Indian languages"; }
		if (code == "NAP") { return "Neapolitan"; }
		if (code == "NAU") { return "Nauru"; }
		if (code == "NAV") { return "Navajo"; }
		if (code == "NBL") { return "Ndebele, South"; }
		if (code == "NDE") { return "Ndebele, North"; }
		if (code == "NDO") { return "Ndonga"; }
		if (code == "NDS") { return "Low German"; }
		if (code == "NEP") { return "Nepali"; }
		if (code == "NEW") { return "Nepal Bhasa"; }
		if (code == "NIA") { return "Nias"; }
		if (code == "NIC") { return "Niger-Kordofanian languages"; }
		if (code == "NIU") { return "Niuean"; }
		if (code == "NLD") { return "Dutch"; }
		if (code == "NLD") { return "Dutch"; }
		if (code == "NNO") { return "Norwegian Nynorsk"; }
		if (code == "NOB") { return "Bokmål, Norwegian"; }
		if (code == "NOG") { return "Nogai"; }
		if (code == "NON") { return "Norse, Old"; }
		if (code == "NOR") { return "Norwegian"; }
		if (code == "NQO") { return "N'Ko"; }
		if (code == "NSO") { return "Pedi"; }
		if (code == "NUB") { return "Nubian languages"; }
		if (code == "NWC") { return "Classical Newari"; }
		if (code == "NYA") { return "Chichewa"; }
		if (code == "NYM") { return "Nyamwezi"; }
		if (code == "NYN") { return "Nyankole"; }
		if (code == "NYO") { return "Nyoro"; }
		if (code == "NZI") { return "Nzima"; }
		if (code == "OCI") { return "Occitan (post 1500)"; }
		if (code == "OJI") { return "Ojibwa"; }
		if (code == "ORI") { return "Oriya"; }
		if (code == "ORM") { return "Oromo"; }
		if (code == "OSA") { return "Osage"; }
		if (code == "OSS") { return "Ossetian"; }
		if (code == "OTA") { return "Turkish, Ottoman (1500-1928)"; }
		if (code == "OTO") { return "Otomian languages"; }
		if (code == "PAA") { return "Papuan languages"; }
		if (code == "PAG") { return "Pangasinan"; }
		if (code == "PAL") { return "Pahlavi"; }
		if (code == "PAM") { return "Pampanga"; }
		if (code == "PAN") { return "Panjabi"; }
		if (code == "PAP") { return "Papiamento"; }
		if (code == "PAU") { return "Palauan"; }
		if (code == "PEO") { return "Persian, Old (ca.600-400 B.C.)"; }
		if (code == "PER") { return "Persian"; }
		if (code == "PHI") { return "Philippine languages"; }
		if (code == "PHN") { return "Phoenician"; }
		if (code == "PLI") { return "Pali"; }
		if (code == "POL") { return "Polish"; }
		if (code == "PON") { return "Pohnpeian"; }
		if (code == "POR") { return "Portuguese"; }
		if (code == "PRA") { return "Prakrit languages"; }
		if (code == "PRO") { return "Provençal, Old (to 1500)"; }
		if (code == "PUS") { return "Pushto"; }
		if (code == "QUE") { return "Quechua"; }
		if (code == "RAJ") { return "Rajasthani"; }
		if (code == "RAP") { return "Rapanui"; }
		if (code == "RAR") { return "Rarotongan"; }
		if (code == "ROA") { return "Romance languages"; }
		if (code == "ROH") { return "Romansh"; }
		if (code == "ROM") { return "Romany"; }
		if (code == "RON") { return "Romanian"; }
		if (code == "RUM") { return "Romanian"; }
		if (code == "RUN") { return "Rundi"; }
		if (code == "RUP") { return "Aromanian"; }
		if (code == "RUS") { return "Russian"; }
		if (code == "SAD") { return "Sandawe"; }
		if (code == "SAG") { return "Sango"; }
		if (code == "SAH") { return "Yakut"; }
		if (code == "SAI") { return "South American Indian languages"; }
		if (code == "SAL") { return "Salishan languages"; }
		if (code == "SAM") { return "Samaritan Aramaic"; }
		if (code == "SAN") { return "Sanskrit"; }
		if (code == "SAS") { return "Sasak"; }
		if (code == "SAT") { return "Santali"; }
		if (code == "SCN") { return "Sicilian"; }
		if (code == "SCO") { return "Scots"; }
		if (code == "SEL") { return "Selkup"; }
		if (code == "SEM") { return "Semitic languages"; }
		if (code == "SGA") { return "Irish, Old (to 900)"; }
		if (code == "SGN") { return "Sign Languages"; }
		if (code == "SHN") { return "Shan"; }
		if (code == "SID") { return "Sidamo"; }
		if (code == "SIN") { return "Sinhala"; }
		if (code == "SIO") { return "Siouan languages"; }
		if (code == "SIT") { return "Sino-Tibetan languages"; }
		if (code == "SLA") { return "Slavic languages"; }
		if (code == "SLO") { return "Slovak"; }
		if (code == "SLV") { return "Slovenian"; }
		if (code == "SMA") { return "Southern Sami"; }
		if (code == "SME") { return "Northern Sami"; }
		if (code == "SMI") { return "Sami languages"; }
		if (code == "SMJ") { return "Lule Sami"; }
		if (code == "SMN") { return "Inari Sami"; }
		if (code == "SMO") { return "Samoan"; }
		if (code == "SMS") { return "Skolt Sami"; }
		if (code == "SNA") { return "Shona"; }
		if (code == "SND") { return "Sindhi"; }
		if (code == "SNK") { return "Soninke"; }
		if (code == "SOG") { return "Sogdian"; }
		if (code == "SOM") { return "Somali"; }
		if (code == "SON") { return "Songhai languages"; }
		if (code == "SOT") { return "Sotho, Southern"; }
		if (code == "SPA") { return "Spanish"; }
		if (code == "SQI") { return "Albanian"; }
		if (code == "SRD") { return "Sardinian"; }
		if (code == "SRN") { return "Sranan Tongo"; }
		if (code == "SRP") { return "Serbian"; }
		if (code == "SRR") { return "Serer"; }
		if (code == "SSA") { return "Nilo-Saharan languages"; }
		if (code == "SSW") { return "Swati"; }
		if (code == "SUK") { return "Sukuma"; }
		if (code == "SUN") { return "Sundanese"; }
		if (code == "SUS") { return "Susu"; }
		if (code == "SUX") { return "Sumerian"; }
		if (code == "SWA") { return "Swahili"; }
		if (code == "SWE") { return "Swedish"; }
		if (code == "SYC") { return "Classical Syriac"; }
		if (code == "SYR") { return "Syriac"; }
		if (code == "TAH") { return "Tahitian"; }
		if (code == "TAI") { return "Tai languages"; }
		if (code == "TAM") { return "Tamil"; }
		if (code == "TAT") { return "Tatar"; }
		if (code == "TEL") { return "Telugu"; }
		if (code == "TEM") { return "Timne"; }
		if (code == "TER") { return "Tereno"; }
		if (code == "TET") { return "Tetum"; }
		if (code == "TGK") { return "Tajik"; }
		if (code == "TGL") { return "Tagalog"; }
		if (code == "THA") { return "Thai"; }
		if (code == "TIB") { return "Tibetian"; }
		if (code == "TIG") { return "Tigre"; }
		if (code == "TIR") { return "Tigrinya"; }
		if (code == "TIV") { return "Tiv"; }
		if (code == "TKL") { return "Tokelau"; }
		if (code == "TLH") { return "Klingon"; }
		if (code == "TLI") { return "Tlingit"; }
		if (code == "TMH") { return "Tamashek"; }
		if (code == "TOG") { return "Tonga (Nyasa)"; }
		if (code == "TON") { return "Tonga (Tonga Islands)"; }
		if (code == "TPI") { return "Tok Pisin"; }
		if (code == "TSI") { return "Tsimshian"; }
		if (code == "TSN") { return "Tswana"; }
		if (code == "TSO") { return "Tsonga"; }
		if (code == "TUK") { return "Turkmen"; }
		if (code == "TUM") { return "Tumbuka"; }
		if (code == "TUP") { return "Tupi languages"; }
		if (code == "TUR") { return "Turkish"; }
		if (code == "TUT") { return "Altaic languages"; }
		if (code == "TVL") { return "Tuvalu"; }
		if (code == "TWI") { return "Twi"; }
		if (code == "TYV") { return "Tuvinian"; }
		if (code == "UDM") { return "Udmurt"; }
		if (code == "UGA") { return "Ugaritic"; }
		if (code == "UIG") { return "Uighur"; }
		if (code == "UKR") { return "Ukrainian"; }
		if (code == "UMB") { return "Umbundu"; }
		if (code == "UND") { return "Undetermined"; }
		if (code == "URD") { return "Urdu"; }
		if (code == "UZB") { return "Uzbek"; }
		if (code == "VAI") { return "Vai"; }
		if (code == "VEN") { return "Venda"; }
		if (code == "VIE") { return "Vietnamese"; }
		if (code == "VOL") { return "Volapük"; }
		if (code == "VOT") { return "Votic"; }
		if (code == "WAK") { return "Wakashan languages"; }
		if (code == "WAL") { return "Wolaitta"; }
		if (code == "WAR") { return "Waray"; }
		if (code == "WAS") { return "Washo"; }
		if (code == "WEL") { return "Welsh"; }
		if (code == "WEL") { return "Welsh"; }
		if (code == "WEN") { return "Sorbian languages"; }
		if (code == "WLN") { return "Walloon"; }
		if (code == "WOL") { return "Wolof"; }
		if (code == "XAL") { return "Kalmyk"; }
		if (code == "XHO") { return "Xhosa"; }
		if (code == "YAO") { return "Yao"; }
		if (code == "YAP") { return "Yapese"; }
		if (code == "YID") { return "Yiddish"; }
		if (code == "YOR") { return "Yoruba"; }
		if (code == "YPK") { return "Yupik languages"; }
		if (code == "ZAP") { return "Zapotec"; }
		if (code == "ZBL") { return "Blissymbols"; }
		if (code == "ZEN") { return "Zenaga"; }
		if (code == "ZGH") { return "Moroccan"; }
		if (code == "ZHA") { return "Zhuang"; }
		if (code == "ZHO") { return "Chinese"; }
		if (code == "ZND") { return "Zande languages"; }
		if (code == "ZUL") { return "Zulu"; }
		if (code == "ZUN") { return "Zuni"; }
		if (code == "ZZA") { return "Zaza"; }
	}
	return code;
}



// END_MERGE

} // end namespace hum



