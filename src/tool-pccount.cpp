//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Sep  7 20:22:22 PDT 2019
// Last Modified: Sat Sep  7 20:22:25 PDT 2019
// Filename:      tool-pccount.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/tool-pccount.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Analyze pccounttic activity in vocal music.
//
//

#include "tool-pccount.h"
#include "HumRegex.h"
#include "Convert.h"

using namespace std;

namespace hum {

// START_MERGE


/////////////////////////////////
//
// Tool_gridtest::Tool_pccount -- Set the recognized options for the tool.
//

Tool_pccount::Tool_pccount(void) {
	define("a|attacks=b", "count attacks instead of durations");
	define("d|data|vega-data=b", "display the vega-lite template.");
	define("f|full=b", "full count attacks all single sharps and flats.");
	define("ff|double-full=b", "full count attacks all double sharps and flats.");
	define("h|html=b", "generate vega-lite HTML content");
	define("i|id=s:id", "ID for use as variable and in plot title");
	define("K|no-key|no-final=b", "Do not label key tonic or final");
	define("m|maximum=b", "normalize by maximum count");
	define("n|normalize=b", "normalize counts");
	define("p|page=b", "generate vega-lite stand-alone HTML page");
	define("r|ratio|aspect-ratio=d:0.67", "width*ratio=height of vega-lite plot");
	define("s|script|vega-script=b", "generate vega-lite javascript content");
	define("title=s", "Title for plot");
	define("t|template|vega-template=b", "display the vega-lite template.");
	define("w|width=i:400", "width of vega-lite plot");
}



///////////////////////////////
//
// Tool_pccount::run -- Primary interfaces to the tool.
//

bool Tool_pccount::run(HumdrumFileSet& infiles) {
	bool status = true;
	for (int i=0; i<infiles.getCount(); i++) {
		status &= run(infiles[i]);
	}
	return status;
}


bool Tool_pccount::run(const string& indata, ostream& out) {
	HumdrumFile infile(indata);
	return run(infile, out);
}


bool Tool_pccount::run(HumdrumFile& infile, ostream& out) {
	bool status = run(infile);
	return status;
}


bool Tool_pccount::run(HumdrumFile& infile) {
   initialize(infile);
	processFile(infile);
	return true;
}



//////////////////////////////
//
// Tool_pccount::initialize --
//

void Tool_pccount::initialize(HumdrumFile& infile) {
	m_attack     = getBoolean("attacks");
	m_full       = getBoolean("full");
	m_doublefull = getBoolean("double-full");
	m_normalize  = getBoolean("normalize");
	m_maximum    = getBoolean("maximum");
	m_template   = getBoolean("vega-template");
	m_data       = getBoolean("vega-data");
	m_script     = getBoolean("vega-script");
	m_width      = getInteger("width");
	m_ratio      = getDouble("aspect-ratio");
	m_key        = !getBoolean("no-key");
	if (getBoolean("title")) {
		m_title = getString("title");
	}
	m_html       = getBoolean("html");
	m_page       = getBoolean("page");
	if (getBoolean("id")) {
		m_id = getString("id");
	} else {
		string filename = infile.getFilename();
	 	auto pos = filename.rfind("/");
		if (pos != string::npos) {
			filename = filename.substr(pos+1);
		}
		pos = filename.find("-");
		if (pos != string::npos) {
			m_id = filename.substr(0, pos);
		}
	}
	m_parttracks.clear();
	m_names.clear();
	m_abbreviations.clear();
	initializePartInfo(infile);


	// https://encycolorpedia.com/36cd27
	m_vcolor.clear();

	m_vcolor["Canto"]		=	"#e49689";
	m_vcolor["Canto (Canto I)"]	=	"#e49689";
	m_vcolor["Canto I"]		=	"#e49689";
	m_vcolor["Canto Primo"]		=	"#e49689";
	m_vcolor["[Canto 1]"]		=	"#e49689";
	m_vcolor["[Canto]"]		=	"#e49689";
	m_vcolor["[Soprano o Tenore]"]	=	"#e49689";

	m_vcolor["Canto 2."]		=	"#d67365";
	m_vcolor["Canto II"]		=	"#d67365";
	m_vcolor["Canto II [Sesto]"]	=	"#d67365";
	m_vcolor["Canto Sec."]		=	"#d67365";
	m_vcolor["Canto Secondo"]	=	"#d67365";
	m_vcolor["Canto secondo"]	=	"#d67365";
	m_vcolor["[Canto 2]"]		=	"#d67365";

	m_vcolor["Canto III [Settimo]"]	=	"#c54f43";

	m_vcolor["Alto"]		=	"#f4c6a1";
	m_vcolor["Alti"]		=	"#f4c6a1";
	m_vcolor["Alto (Canto III)"]	=	"#f4c6a1";

	m_vcolor["Alto II"]		=	"#edb383";

	m_vcolor["Tenor"]		=	"#ecdf7a";
	m_vcolor["Tenore"]		=	"#ecdf7a";
	m_vcolor["Tenore over Canto"]	=	"#ecdf7a";
	m_vcolor["[Tenore]"]		=	"#ecdf7a";

	m_vcolor["Sesto"]		=	"#c8f0bb";
	m_vcolor["Sesto (Canto II)"]	=	"#c8f0bb";
	m_vcolor["Sesto Canto II"]	=	"#c8f0bb";

	m_vcolor["Quinto"]		=	"#e3f5f8";
	m_vcolor["Qvinto"]		=	"#e3f5f8";

	m_vcolor["Ottava parte [Ottavo]"]	=	"#e0e4f7";

	m_vcolor["Nona parte [Nono]"]	=	"#a39ce5";

	m_vcolor["Basso"]		=	"#d2aef7";

	m_vcolor["Basso II"]		=	"#c69af5";
	m_vcolor["Basso II [Decimo]"]	=	"#c69af5";

	m_vcolor["Basso Continuo"]	=	"#a071ec";
	m_vcolor["Basso continuo"]	=	"#a071ec";
	m_vcolor["[B. C.]"]		=	"#a071ec";
	m_vcolor["[Basso Continuo]"]	=	"#a071ec";
	m_vcolor["[Basso continuo]"]	=	"#a071ec";
	m_vcolor["B.C."]		=	"#a071ec";
	m_vcolor["B.c."]		=	"#a071ec";
}



//////////////////////////////
//
// Tool_pccount::getFinal -- Extract the last unparenthesed letter from a ref record like this:
// 
// !!!final: (A)D
//

string Tool_pccount::getFinal(HumdrumFile& infile) {
	string finalref = infile.getReferenceRecord("final");
	HumRegex hre;
	hre.replaceDestructive(finalref, "", "\\(.*?\\)", "g");
	hre.replaceDestructive(finalref, "", "\\s+", "g");
	if (hre.search(finalref, "^[A-G]$", "i")) {
		return finalref;
	} else {
		return "";
	}
}



//////////////////////////////
//
// Tool_pccount::processFile --
//

void Tool_pccount::processFile(HumdrumFile& infile) {
	countPitches(infile);

	string datavar = "data_" + m_id;
	string target = "id_" + m_id;
	string jsonvar = "vega_" + m_id;
	if (m_template) {
		printVegaLiteJsonTemplate(datavar, infile);
	} else if (m_data) {
		printVegaLiteJsonData();
	} else if (m_script) {
		printVegaLiteScript(jsonvar, target, datavar, infile);
	} else if (m_html) {
		printVegaLiteHtml(jsonvar, target, datavar, infile);
	} else if (m_page) {
		printVegaLitePage(jsonvar, target, datavar, infile);
	} else {
		printHumdrumTable();
	}
}



//////////////////////////////
//
// Tool_pccount::printVegaLitePage --
// 

void Tool_pccount::printVegaLitePage(const string& jsonvar,
		const string& target, const string& datavar, HumdrumFile& infile) {
	stringstream& out = m_free_text;

	out << "<!DOCTYPE html>\n";
	out << "<html>\n";
	out << "  <head>\n";
	out << "    <title>Vega-Lite Bar Chart</title>\n";
	out << "    <meta charset=\"utf-8\" />\n";
	out << "\n";
	out << "    <script src=\"https://cdn.jsdelivr.net/npm/vega@5.4.0\"></script>\n";
	out << "    <script src=\"https://cdn.jsdelivr.net/npm/vega-lite@4.0.0-beta.1\"></script>\n";
	out << "    <script src=\"https://cdn.jsdelivr.net/npm/vega-embed@5\"></script>\n";
	out << "\n";
	out << "    <style media=\"screen\">\n";
	out << "      /* Add space between Vega-Embed links  */\n";
	out << "      .vega-actions a {\n";
	out << "        margin-right: 5px;\n";
	out << "      }\n";
	out << "    </style>\n";
	out << "  </head>\n";
	out << "  <body>\n";
	out << "    <h1>Pitch-class histogram</h1>\n";
	printVegaLiteHtml(jsonvar, target, datavar, infile);
	out << "</body>\n";
	out << "</html>\n";
}



//////////////////////////////
//
// Tool_pccount::printVegaLiteHtml --
// 

void Tool_pccount::printVegaLiteHtml(const string& jsonvar,
		const string& target, const string& datavar, HumdrumFile& infile) {
	stringstream& out = m_free_text;

	out << "<div id=\"" << target << "\"></div>\n";
	out << "\n";
	out << "<script>\n";
	printVegaLiteScript(jsonvar, target, datavar, infile);
	out << "</script>\n";
}



//////////////////////////////
//
// Tool_pccount::printVegaLiteScript --
// 

void Tool_pccount::printVegaLiteScript(const string& jsonvar,
		const string& target, const string& datavar, HumdrumFile& infile) {
	stringstream& out = m_free_text;

	out << "var " << datavar << " =\n";
	printVegaLiteJsonData();
	out << ";\n";
	out << "\n";
	out << "var " << jsonvar << " =\n";
	printVegaLiteJsonTemplate(datavar, infile);
	out << ";\n";
	out << "vegaEmbed('#" << target << "', " << jsonvar << ");\n";
}



//////////////////////////////
//
// Tool_pccount::printVegaLiteJsonData --
//

void Tool_pccount::printVegaLiteJsonData(void) {
	stringstream& out = m_free_text;

	double maxpc = 0.0;
	for (int i=0; i<m_counts[0].size(); i++) {
		if (m_counts[0][i] > maxpc) {
			maxpc = m_counts[0][i];
		}
	}
	out << "[\n";
	int commacounter = 0;
	double percent = 100.0;
	for (int i=1; i<(int)m_counts.size(); i++) {
		for (int j=0; j<(int)m_counts[i].size(); j++) {
			if (m_counts[i][j] == 0.0) {
				continue;
			}
			if (commacounter > 0) {
				out << ",\n\t";
			} else {
				out << "\t";
			}
			commacounter++;
			out << "{\"percent\":" << m_counts[i][j]/maxpc*percent << ", ";
			out << "\"pitch class\":\"" << getPitchClassString(j) << "\", ";
			out << "\"voice\":\"" << m_names[i] << "\"";
			out << "}";
		}
	}
	out << "\n]\n";
}



//////////////////////////////
//
// Tool_pccount::setFactorMaximum -- normalize by the maximum pitch-class value.
//

void Tool_pccount::setFactorMaximum(void) {
	m_factor = 0.0;
	for (int i=0; i<m_counts[0].size(); i++) {
		if (m_counts[0][i] > m_factor) {
			m_factor = m_counts[0][i];
		}
	}
}



//////////////////////////////
//
// Tool_pccount::setFactorNormalize -- normalize by the sum of all pitch class values.
//

void Tool_pccount::setFactorNormalize(void) {
	m_factor = 0.0;
	for (int i=0; i<m_counts[0].size(); i++) {
		m_factor += m_counts[0][i];
	}
}



//////////////////////////////
//
// Tool_pccount::printHumdrumTable --
//

void Tool_pccount::printHumdrumTable(void) {

	double factor = 0.0;

	if (m_maximum) {
		setFactorMaximum();
		m_free_text << "!!!MAX: " << m_factor << endl;
	} else if (m_normalize) {
		setFactorNormalize();
		m_free_text << "!!!TOTAL: " << factor << endl;
	}

	// exclusive interpretation
	m_free_text << "**kern";
	m_free_text << "\t**all";
	for (int i=0; i<(int)m_counts.size() - 1; i++) {
		m_free_text << "\t**part";
	}
	m_free_text << endl;

	// part names
	m_free_text << "*";
	for (int i=0; i<(int)m_counts.size(); i++) {
		if (!m_names[i].empty()) {
			m_free_text << "\t*I\"" << m_names[i];
		}
	}
	m_free_text << endl;

	// part abbreviation
	m_free_text << "*";
	for (int i=0; i<(int)m_counts.size(); i++) {
		if (!m_names[i].empty()) {
			m_free_text << "\t*I\'" << m_abbreviations[i];
		}
	}
	m_free_text << endl;

	for (int i=0; i<(int)m_counts[0].size(); i++) {
		if (m_counts[0][i] == 0) {
			continue;
		}
		if ((i == 5) || (i == 11) || (i == 22) || (i == 28) || (i == 34)) {
			continue;
		}
		string pitch = Convert::base40ToKern(i + 4*40);
		m_free_text << pitch;
		for (int j=0; j<(int)m_counts.size(); j++) {
			if (m_normalize) {
				m_free_text << "\t" << m_counts[j][i] / m_factor;
			} else if (m_maximum) {
				m_free_text << "\t" << m_counts[j][i] / m_factor;
			} else {
				m_free_text << "\t" << m_counts[j][i];
			}
		}
		m_free_text << endl;
	}

	int columns = (int)m_counts.size() + 1;
	for (int i=0; i<columns; i++) {
		m_free_text << "*-";
		if (i < columns - 1) {
			m_free_text << "\t";
		}
	}
	m_free_text << endl;
}



//////////////////////////////
//
// Tool_pccount::countPitches --
//

void Tool_pccount::countPitches(HumdrumFile& infile) {
	if (m_parttracks.size() == 0) {
		return;
	}
	m_counts.clear();
	m_counts.resize(m_parttracks.size());
	for (int i=0; i<(int)m_parttracks.size(); i++) {
		m_counts[i].resize(40);
		fill(m_counts[i].begin(), m_counts[i].end(), 0.0);
	}
	for (int i=0; i<infile.getStrandCount(); i++) {
		HTp sstart = infile.getStrandStart(i);
		HTp send = infile.getStrandEnd(i);
		addCounts(sstart, send);
	}

	// fill in sum for all parts
	for (int i=0; i<(int)m_counts[0].size(); i++) {
		for (int j=1; j<(int)m_counts.size(); j++) {
			m_counts[0][i] += m_counts[j][i];
		}
	}

}


//////////////////////////////
//
// Tool_pccount::addCounts --
//

void Tool_pccount::addCounts(HTp sstart, HTp send) {
	if (!sstart) {
		return;
	}
	if (!sstart->isKern()) {
		return;
	}
	int track = sstart->getTrack();
	int kindex = m_rkern[track];
	HTp current = sstart;
	while (current && (current != send)) {
		if (!current->isData()) {
			current = current->getNextToken();
			continue;
		}
		if (current->isNull() || current->isRest()) {
			current = current->getNextToken();
			continue;
		}
		vector<string> subtokens = current->getSubtokens();
		for (int i=0; i<(int)subtokens.size(); i++) {
			if (m_attack) {
				// ignore sustained parts of notes when counting attacks
				if (subtokens[i].find("_") != string::npos) {
					continue;
				}
				if (subtokens[i].find("]") != string::npos) {
					continue;
				}
			}
			int b40 = Convert::kernToBase40(subtokens[i]);
			if (m_attack) {
				m_counts[kindex][b40%40]++;
			} else {
				double duration = Convert::recipToDuration(subtokens[i]).getFloat();
				m_counts[kindex][b40%40] += duration;
			}
		}
		current = current->getNextToken();
	}
}




//////////////////////////////
//
// Tool_pccount::initializePartInfo --
//

void Tool_pccount::initializePartInfo(HumdrumFile& infile) {
	m_names.clear();
	m_abbreviations.clear();
	m_parttracks.clear();
	m_rkern.clear();

	m_rkern.resize(infile.getTrackCount() + 1);
	fill(m_rkern.begin(), m_rkern.end(), -1);

	m_parttracks.push_back(-1);
	m_names.push_back("all");
	m_abbreviations.push_back("all");

	vector<HTp> starts = infile.getKernSpineStartList();

	int track = 0;
	for (int i=0; i<(int)starts.size(); i++) {
		track = starts[i]->getTrack();
		m_rkern[track] = i+1;
		m_parttracks.push_back(track);
		HTp current = starts[i];
		while (current) {
			if (current->isData()) {
				break;
			}
			if (current->compare(0, 3, "*I\"") == 0) {
				m_names.emplace_back(current->substr(3));
			} else if (current->compare(0, 3, "*I\'") == 0) {
				m_abbreviations.emplace_back(current->substr(3));
			}
			current = current->getNextToken();
		}
	}

}


//////////////////////////////
//
// printVegaLiteJsonTemplate --
//

void Tool_pccount::printVegaLiteJsonTemplate(const string& datavariable, HumdrumFile& infile) {
	stringstream& out = m_free_text;

	out << "{\n";
	out << "	\"$schema\": \"https://vega.github.io/schema/vega-lite/v4.0.0-beta.1.json\",\n";
	out << "	\"data\": {\"values\": " << datavariable << "},\n";
	if (getBoolean("title")) {
		out << "	\"title\": \"" << m_title << "\",\n";
	} else {
		if (m_attack) {
			out << "	\"title\": \"Attack-weighted pitch-class distribution for " << m_id <<" \",\n";
		} else {
			out << "	\"title\": \"Duration-weighted pitch-class distribution for " << m_id <<" \",\n";
		}
	}
	out << "	\"width\": " << m_width << ",\n";
	out << "	\"height\": " << int(m_width * m_ratio) << ",\n";
	out << "	\"encoding\": {\n";
	out << "		\"y\": {\n";
	out << "			\"field\": \"percent\",\n";
	out << "			\"title\": \"Percent of maximum pitch class\",\n";
	out << "			\"type\": \"quantitative\",\n";
	out << "			\"scale\": {\"domain\": [0, 100]},\n";
	out << "			\"aggregate\": \"sum\"\n";
	out << "		},\n";
	out << "		\"x\": {\n";
	out << "			\"field\": \"pitch class\",\n";
	out << "			\"type\": \"nominal\",\n";
	out << "			\"scale\": {\n";
	out << "				\"domain\": [";
		printPitchClassList();
		out << "]\n";
	out << "			},\n";
	out << "			\"axis\": {\n";
	out << "				\"labelAngle\": 0\n";
	out << "			}\n";
	out << "		},\n";
	out << "		\"order\": {\"type\": \"quantitative\"},\n";
	out << "		\"color\": {\n";
	out << "			\"field\": \"voice\",\n";
	out << "			\"type\": \"nominal\",\n";
	if (m_counts.size() == 2) {
		out << "			\"legend\": {\"title\": \"Voice\"},\n";
	} else {
		out << "			\"legend\": {\"title\": \"Voices\"},\n";
	}
	out << "			\"scale\": {\n";
  	out << "				\"domain\": [";
		printVoiceList();
		out << "],\n";
 	out << "				\"range\": [";
		printColorList();
		out << "],\n";
 	out << "				}\n";
	out << "		}\n";
	out << "	},\n";

	out << "	\"layer\": [\n";
	out << "		{\"mark\": \"bar\"}";

	string final = getFinal(infile);
	double percent = getPercent(final);
	if (m_key && !final.empty()) {
		out << ",\n";
		out << "		{\n";
		out << "			\"mark\": {\"type\":\"text\", \"align\":\"center\", \"fill\":\"black\", \"baseline\":\"bottom\"},\n";
		out << "			\"data\": {\"values\": [ {\"pitch class\":\"" << final << "\", \"percent\":" << percent << "}]},\n";
		out << "			\"encoding\": {\"text\": {\"value\":\"final\"}}\n";
		out << "		}\n";
	}

	out << "	]\n";
	out << "}\n";

}



//////////////////////////////
//
// Tool_pccount::getPercent --
//

double Tool_pccount::getPercent(const string& pitchclass) {
	setFactorMaximum();
	int b40 = Convert::kernToBase40(pitchclass);
	int index = b40 % 40;
	double output = m_counts[0][index] / m_factor * 100.0;
	return output;
}



//////////////////////////////
//
// Tool_pccount::printColorList --
//

void Tool_pccount::printColorList(void) {
	stringstream& out = m_free_text;
	for (int i=(int)m_names.size() - 1; i>0; i--) {
		string color = m_vcolor[m_names[i]];
		out << "\"";
		if (color.empty()) {
			out << "black";
		} else {
			out << color;
		}
		out << "\"";
		if (i > 1) {
			out << ", ";
		}
	}
}



//////////////////////////////
//
// Tool_pccount::printVoiceList --
//

void Tool_pccount::printVoiceList(void) {
	stringstream& out = m_free_text;
	for (int i=(int)m_names.size() - 1; i>0; i--) {
		out << "\"";
		out << m_names[i];
		out << "\"";
		if (i > 1) {
			out << ", ";
		}
	}
}



//////////////////////////////
//
// Tool_pccount::printReverseVoiceList --
//

void Tool_pccount::printReverseVoiceList(void) {
	stringstream& out = m_free_text;
	for (int i=1; i<(int)m_names.size(); i++) {
		out << "\"";
		out << m_names[i];
		out << "\"";
		if (i < (int)m_names.size() - 1) {
			out << ", ";
		}
	}
}



//////////////////////////////
//
// Tool_pccount::printPitchClassList --
//

void Tool_pccount::printPitchClassList(void) {
	stringstream& out = m_free_text;

	if (m_counts[0][0] > 0.0)  { out << "\"C♭♭\", "; }
	if (m_counts[0][1] > 0.0)  { out << "\"C♭\", "; }
	out << "\"C\"";
	if (m_counts[0][3] > 0.0)  { out << ", \"C♯\""; }
	if (m_counts[0][4] > 0.0)  { out << ", \"C♯♯\""; }
	// 5 is empty

	if (m_counts[0][6] > 0.0)  { out << ", \"D♭♭\""; }
	if (m_counts[0][7] > 0.0)  { out << ", \"D♭\""; }
	out << ", \"D\"";
	if (m_counts[0][9] > 0.0)  { out << ", \"D♯\""; }
	if (m_counts[0][10] > 0.0) { out << ", \"D♯♯\""; }
	// 11 is empty

	if (m_counts[0][12] > 0.0) { out << ", \"E♭♭\""; }
	if (m_counts[0][13] > 0.0) { out << ", \"E♭\""; }
	out << ", \"E\"";
	if (m_counts[0][15] > 0.0) { out << ", \"E♯\""; }
	if (m_counts[0][16] > 0.0) { out << ", \"E♯♯\""; }

	if (m_counts[0][17] > 0.0) { out << ", \"F♭♭\""; }
	if (m_counts[0][18] > 0.0) { out << ", \"F♭\""; }
	out << ", \"F\"";
	if (m_counts[0][20] > 0.0) { out << ", \"F♯\""; }
	if (m_counts[0][21] > 0.0) { out << ", \"F♯♯\""; }
	// 22 is empty

	if (m_counts[0][23] > 0.0) { out << ", \"G♭♭\""; }
	if (m_counts[0][24] > 0.0) { out << ", \"G♭\""; }
	out << ", \"G\"";
	if (m_counts[0][26] > 0.0) { out << ", \"G♯\""; }
	if (m_counts[0][27] > 0.0) { out << ", \"G♯♯\""; }
	// 28 is empty

	if (m_counts[0][29] > 0.0) { out << ", \"A♭♭\""; }
	if (m_counts[0][30] > 0.0) { out << ", \"A♭\""; }
	out << ", \"A\"";
	if (m_counts[0][32] > 0.0) { out << ", \"A♯\""; }
	if (m_counts[0][33] > 0.0) { out << ", \"A♯♯\""; }
	// 34 is empty

	if (m_counts[0][35] > 0.0) { out << ", \"B♭♭\""; }
	if (m_counts[0][36] > 0.0) { out << ", \"B♭\""; }
	out << ", \"B\"";
	if (m_counts[0][38] > 0.0) { out << ", \"B♯\""; }
	if (m_counts[0][39] > 0.0) { out << ", \"B♯♯\""; }

}


//////////////////////////////
//
// Tool_pccount::getPitchClassString --
//

string Tool_pccount::getPitchClassString(int b40) {
	switch (b40%40) {
		case 0: return "C♭♭";
		case 1: return "C♭";
		case 2: return "C";
		case 3: return "C♯";
		case 4: return "C♯♯";
		// 5 is empty
		case 6: return "D♭♭";
		case 7: return "D♭";
		case 8: return "D";
		case 9: return "D♯";
		case 10: return "D♯♯";
		// 11 is empty
		case 12: return "E♭♭";
		case 13: return "E♭";
		case 14: return "E";
		case 15: return "E♯";
		case 16: return "E♯♯";
		case 17: return "F♭♭";
		case 18: return "F♭";
		case 19: return "F";
		case 20: return "F♯";
		case 21: return "F♯♯";
		// 22 is empty
		case 23: return "G♭♭";
		case 24: return "G♭";
		case 25: return "G";
		case 26: return "G♯";
		case 27: return "G♯♯";
		// 28 is empty
		case 29: return "A♭♭";
		case 30: return "A♭";
		case 31: return "A";
		case 32: return "A♯";
		case 33: return "A♯♯";
		// 34 is empty
		case 35: return "B♭♭";
		case 36: return "B♭";
		case 37: return "B";
		case 38: return "B♯";
		case 39: return "B♯♯";
	}

	return "?";
}



// END_MERGE

} // end namespace hum



