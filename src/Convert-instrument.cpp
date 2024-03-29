//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sat Feb 29 20:10:49 PST 2020
// Last Modified: Sat Feb 29 20:10:52 PST 2020
// Filename:      Convert-instrument.cpp
// URL:           https://github.com/craigsapp/humlib/blob/master/src/Convert-instrument.cpp
// Syntax:        C++11; humlib
// vim:           syntax=cpp ts=3 noexpandtab nowrap
//
// Description:   Conversions related to instrument records.
//

#include "Convert.h"

#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace hum {

// START_MERGE


//////////////////////////////
//
// Convert::getInstrumentList -- List from https://bit.ly/humdrum-instrument-codes
//

vector<pair<string, string> > Convert::getInstrumentList(void) {
	return vector<pair<string, string> > {
		{"accor",	"klav"},
		{"alto",	"vox"},
		{"anvil",	"idio"},
		{"archl",	"str"},
		{"armon",	"ww"},
		{"arpa",	"str"},
		{"bagpI",	"ww"},
		{"bagpS",	"ww"},
		{"banjo",	"str"},
		{"bansu",	"ww"},
		{"barit",	"vox"},
		{"baset",	"ww"},
		{"bass",	"vox"},
		{"bdrum",	"idio"},
		{"bguit",	"str"},
		{"biwa",	"str"},
		{"brush",	"idio"},
		{"bscan",	"vox"},
		{"bspro",	"vox"},
		{"bugle",	"bras"},
		{"calam",	"ww"},
		{"calpe",	"ww"},
		{"calto",	"vox"},
		{"campn",	"idio"},
		{"cangl",	"ww"},
		{"canto",	"vox"},
		{"caril",	"idio"},
		{"castr",	"vox"},
		{"casts",	"idio"},
		{"cbass",	"str"},
		{"cello",	"str"},
		{"cemba",	"klav"},
		{"cetra",	"str"},
		{"chain",	"idio"},
		{"chcym",	"idio"},
		{"chime",	"idio"},
		{"chlma",	"ww"},
		{"chlms",	"ww"},
		{"chlmt",	"ww"},
		{"clap",	"idio"},
		{"clara",	"ww"},
		{"clarb",	"ww"},
		{"claro",	"ww"},
		{"clarp",	"ww"},
		{"clars",	"ww"},
		{"clave",	"idio"},
		{"clavi",	"klav"},
		{"clest",	"klav"},
		{"clrno",	"bras"},
		{"colsp",	"vox"},
		{"conga",	"idio"},
		{"cor",	"bras"},
		{"cornm",	"ww"},
		{"corno",	"ww"},
		{"cornt",	"bras"},
		{"coro",	"vox"},
		{"crshc",	"idio"},
		{"ctenor",	"vox"},
		{"ctina",	"klav"},
		{"drmsp",	"vox"},
		{"drum",	"idio"},
		{"drumP",	"idio"},
		{"dulc",	"str"},
		{"eguit",	"str"},
		{"fag_c",	"ww"},
		{"fagot",	"ww"},
		{"false",	"vox"},
		{"fdrum",	"idio"},
		{"feme",	"vox"},
		{"fife",	"ww"},
		{"fingc",	"idio"},
		{"flex",	"idio"},
		{"flt",	"ww"},
		{"flt_a",	"ww"},
		{"flt_b",	"ww"},
		{"fltda",	"ww"},
		{"fltdb",	"ww"},
		{"fltdn",	"ww"},
		{"fltds",	"ww"},
		{"fltdt",	"ww"},
		{"flugh",	"bras"},
		{"forte",	"klav"},
		{"gen",	"gen"},
		{"genB",	"gen"},
		{"genT",	"gen"},
		{"glock",	"idio"},
		{"gong",	"idio"},
		{"guitr",	"str"},
		{"hammd",	"klav"},
		{"hbell",	"idio"},
		{"heck",	"ww"},
		{"heltn",	"vox"},
		{"hichi",	"ww"},
		{"hurdy",	"str"},
		{"kitv",	"str"},
		{"klav",	"klav"},
		{"kokyu",	"str"},
		{"komun",	"str"},
		{"koto",	"str"},
		{"kruma",	"ww"},
		{"krumb",	"ww"},
		{"krums",	"ww"},
		{"krumt",	"ww"},
		{"lion",	"idio"},
		{"liuto",	"str"},
		{"lyrsp",	"vox"},
		{"lyrtn",	"vox"},
		{"male",	"vox"},
		{"mando",	"str"},
		{"marac",	"idio"},
		{"marim",	"idio"},
		{"mbari",	"vox"},
		{"mezzo",	"vox"},
		{"nfant",	"vox"},
		{"nokan",	"ww"},
		{"oboe",	"ww"},
		{"oboeD",	"ww"},
		{"ocari",	"ww"},
		{"ondes",	"klav"},
		{"ophic",	"bras"},
		{"organ",	"klav"},
		{"oud",	"str"},
		{"paila",	"idio"},
		{"panpi",	"ww"},
		{"pbell",	"idio"},
		{"pguit",	"str"},
		{"physh",	"klav"},
		{"piano",	"klav"},
		{"piatt",	"idio"},
		{"picco",	"ww"},
		{"pipa",	"str"},
		{"piri",	"ww"},
		{"porta",	"klav"},
		{"psalt",	"str"},
		{"qin",	"str"},
		{"quinto",	"vox"},
		{"quitr",	"str"},
		{"rackt",	"ww"},
		{"ratch",	"idio"},
		{"ratl",	"idio"},
		{"rebec",	"str"},
		{"recit",	"vox"},
		{"reedo",	"klav"},
		{"rhode",	"klav"},
		{"ridec",	"idio"},
		{"sarod",	"str"},
		{"sarus",	"ww"},
		{"saxA",	"ww"},
		{"saxB",	"ww"},
		{"saxC",	"ww"},
		{"saxN",	"ww"},
		{"saxR",	"ww"},
		{"saxS",	"ww"},
		{"saxT",	"ww"},
		{"sbell",	"idio"},
		{"sdrum",	"idio"},
		{"serp",	"bras"},
		{"sesto",	"vox"},
		{"shaku",	"ww"},
		{"shami",	"str"},
		{"sheng",	"ww"},
		{"sho",	"ww"},
		{"siren",	"idio"},
		{"sitar",	"str"},
		{"slap",	"idio"},
		{"soprn",	"vox"},
		{"spok",	"vox"},
		{"spokF",	"vox"},
		{"spokM",	"vox"},
		{"spshc",	"idio"},
		{"steel",	"idio"},
		{"stim",	"vox"},
		{"stimA",	"vox"},
		{"stimB",	"vox"},
		{"stimC",	"vox"},
		{"stimR",	"vox"},
		{"stimS",	"vox"},
		{"strdr",	"idio"},
		{"sxhA",	"bras"},
		{"sxhB",	"bras"},
		{"sxhC",	"bras"},
		{"sxhR",	"bras"},
		{"sxhS",	"bras"},
		{"sxhT",	"bras"},
		{"synth",	"klav"},
		{"tabla",	"idio"},
		{"tambn",	"idio"},
		{"tambu",	"str"},
		{"tanbr",	"str"},
		{"tblok",	"idio"},
		{"tdrum",	"idio"},
		{"tenor",	"vox"},
		{"timpa",	"idio"},
		{"tiorb",	"str"},
		{"tom",	"idio"},
		{"trngl",	"idio"},
		{"tromP",	"bras"},
		{"troma",	"bras"},
		{"tromb",	"bras"},
		{"tromp",	"bras"},
		{"tromt",	"bras"},
		{"trumB",	"bras"},
		{"tuba",	"bras"},
		{"tubaB",	"bras"},
		{"tubaC",	"bras"},
		{"tubaT",	"bras"},
		{"tubaU",	"bras"},
		{"ukule",	"str"},
		{"vibra",	"idio"},
		{"vina",	"str"},
		{"viola",	"str"},
		{"violb",	"str"},
		{"viold",	"str"},
		{"viole",	"str"},
		{"violn",	"str"},
		{"violp",	"str"},
		{"viols",	"str"},
		{"violt",	"str"},
		{"vox",	"vox"},
		{"wblok",	"idio"},
		{"xylo",	"idio"},
		{"zithr",	"str"},
		{"zurna",	"ww"}
	};
}



// END_MERGE

} // end namespace hum



