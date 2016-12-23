//
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Apr  5 13:07:18 PDT 1998
// Last Modified: Sat Mar  1 09:31:01 PST 2014 Implemented with STL.
// Last Modified: Thu Dec 15 09:06:52 PST 2016 Adjusted internal storage.
// Filename:      Options.cpp
// Web Address:   https://github.com/craigsapp/humlib/blob/master/include/Options.h
// vim:           syntax=cpp ts=3 noexpandtab nowrap
// Syntax:        C++11
//
// Description:   Interface to command-line options.
//

#include "Options.h"

#include <stdlib.h>
#include <string.h>
#include <cctype>
#include <iostream>
#include <algorithm>

using namespace std;

namespace hum {

// START_MERGE


///////////////////////////////////////////////////////////////////////////
//
// Option_register class function definitions.
//


//////////////////////////////
//
// Option_register::Option_register -- Constructor.
//

Option_register::Option_register(void) {
	m_modifiedQ = 0;
	setType('s');
}


Option_register::Option_register(const string& aDefinition, char aType,
		const string& aDefaultOption) {
	m_modifiedQ = 0;
	setType(aType);
	setDefinition(aDefinition);
	setDefault(aDefaultOption);
}


Option_register::Option_register(const string& aDefinition, char aType,
		const string& aDefaultOption, const string& aModifiedOption) {
	m_modifiedQ = 0;
	setType(aType);
	setDefinition(aDefinition);
	setDefault(aDefaultOption);
	setModified(aModifiedOption);
}


Option_register::Option_register(const Option_register& reg) {
	m_definition = reg.m_definition;
	m_description = reg.m_description;
	m_defaultOption = reg.m_defaultOption;
	m_modifiedOption = reg.m_modifiedOption;
	m_modifiedQ = reg.m_modifiedQ;
	m_type = reg.m_type;
}



//////////////////////////////
//
// Option_register::operator= --
//

Option_register& Option_register::operator=(const Option_register& reg) {
	if (this == &reg) {
		return *this;
	}
	m_definition = reg.m_definition;
	m_description = reg.m_description;
	m_defaultOption = reg.m_defaultOption;
	m_modifiedOption = reg.m_modifiedOption;
	m_modifiedQ = reg.m_modifiedQ;
	m_type = reg.m_type;
	return *this;
}



//////////////////////////////
//
// Option_register::~Option_register -- Destructor.
//

Option_register::~Option_register() {
	// do nothing
}



//////////////////////////////
//
// Option_register::clearModified -- Clear any changes in the option value.
//

void Option_register::clearModified(void) {
	m_modifiedOption.clear();
	m_modifiedQ = 0;
}



//////////////////////////////
//
// Option_register::getDefinition -- Returns the initial definition.
//	string used to define this entry.
//

string Option_register::getDefinition(void) {
	return m_definition;
}



//////////////////////////////
//
// Option_register::getDescription -- Return the textual description
//      of the entry.
//

string Option_register::getDescription(void) {
	return m_description;
}



//////////////////////////////
//
// Option_register::getDefault --  Return the default value string.
//

string Option_register::getDefault(void) {
	return m_defaultOption;
}



//////////////////////////////
//
// Option_register::getModified -- Return the modified option string.
//

string Option_register::getModified(void) {
	return m_modifiedOption;
}



//////////////////////////////
//
// Option_register::isModified -- Return true if option has been
//    set on the command-line.
//

int Option_register::isModified(void) {
	return m_modifiedQ;
}



//////////////////////////////
//
// Option_register::getType -- Return the data type of the option.
//

char Option_register::getType(void) {
	return m_type;
}



//////////////////////////////
//
// Option_register::getOption -- return the modified option
//  	or the default option if no modified option.
//

string Option_register::getOption(void) {
	if (isModified()) {
		return getModified();
	} else {
		return getDefault();
	}
}



//////////////////////////////
//
// Option_register::reset -- clear contents of register entry.
//

void Option_register::reset(void) {
	m_definition.clear();
	m_description.clear();
	m_defaultOption.clear();
	m_modifiedOption.clear();
	m_modifiedQ = false;
	m_type = 's';
}



//////////////////////////////
//
// Option_register::setDefault -- Set the default value.
//

void Option_register::setDefault(const string& aString) {
	m_defaultOption = aString;
}



//////////////////////////////
//
// Option_register::setDefinition -- Set the option definition.
//

void Option_register::setDefinition(const string& aString) {
	m_definition = aString;
}



//////////////////////////////
//
// Option_register::setDescription -- Set the textual description.
//

void Option_register::setDescription(const string& aString) {
	m_description = aString;
}



//////////////////////////////
//
// Option_register::setModified -- Set the modified value.
//

void Option_register::setModified(const string& aString) {
	m_modifiedOption = aString;
	m_modifiedQ = 1;
}



//////////////////////////////
//
// Option_register::setType -- Set the option type.
//

void Option_register::setType(char aType) {
	m_type = aType;
}



//////////////////////////////
//
// Option_register::print -- Print the state of the option registery entry.
//     Useul for debugging.
//

ostream& Option_register::print(ostream& out) {
	out << "definition:\t"     << m_definition     << endl;
	out << "description:\t"    << m_description    << endl;
	out << "defaultOption:\t"  << m_defaultOption  << endl;
	out << "modifiedOption:\t" << m_modifiedOption << endl;
	out << "modifiedQ:\t\t"    << m_modifiedQ      << endl;
	out << "type:\t\t"         << m_type           << endl;
	return out;
};




///////////////////////////////////////////////////////////////////////////
//
// Options class function definitions.
//

//////////////////////////////
//
// Options::Options -- Constructor.
//

Options::Options(void) {
	// do nothing
}


Options::Options(int argc, char** argv) {
	setOptions(argc, argv);
}


Options::Options(const Options& options) {
	m_argv = options.m_argv;
	m_arguments = options.m_arguments;
	m_optionFlag = options.m_optionFlag;
	m_optionList = options.m_optionList;
	m_options_error_checkQ = options.m_options_error_checkQ;
	m_processedQ = options.m_processedQ;
	m_suppressQ = options.m_suppressQ;
	m_optionsArgQ = options.m_optionsArgQ;
	for (int i=0; i<(int)options.m_optionRegister.size(); i++) {
		Option_register* orr = new Option_register(*options.m_optionRegister[i]);
		m_optionRegister.push_back(orr);
	}

}



//////////////////////////////
//
// Options::~Options -- Destructor.
//

Options::~Options() {
	reset();
}



//////////////////////////////
//
// Options::operator= --
//

Options& Options::operator=(const Options& options) {
	if (this == &options) {
		return *this;
	}
	m_argv = options.m_argv;
	m_arguments = options.m_arguments;
	m_optionFlag = options.m_optionFlag;
	m_optionList = options.m_optionList;
	m_options_error_checkQ = options.m_options_error_checkQ;
	m_processedQ = options.m_processedQ;
	m_suppressQ = options.m_suppressQ;
	m_optionsArgQ = options.m_optionsArgQ;

	for (int i=0; i<(int)m_optionRegister.size(); i++) {
		delete m_optionRegister[i];
		m_optionRegister[i] = NULL;
	}
	m_optionRegister.clear();
	
	for (int i=0; i<(int)options.m_optionRegister.size(); i++) {
		Option_register* orr = new Option_register(*options.m_optionRegister[i]);
		m_optionRegister.push_back(orr);
	}

	m_error.str("");
	return *this;
}



//////////////////////////////
//
// Options::argc -- returns the argument count as input from main().
//

int Options::argc(void) const {
	return (int)m_argv.size();
}



//////////////////////////////
//
// Options::argv -- returns the arguments strings as input from main().
//

const vector<string>& Options::argv(void) const {
	return m_argv;
}



//////////////////////////////
//
// Options::define -- store an option definition in the registry.  Option
//     definitions have this sructure:
//        option-name|alias-name1|alias-name2=option-type:option-default
// option-name :: name of the option (one or more character, not including
//      spaces or equal signs.
// alias-name  :: equivalent name(s) of the option.
// option-type :: single charater indicating the option data type.
// option-default :: default value for option if no given on the command-line.
//

int Options::define(const string& aDefinition) {
	Option_register* definitionEntry = NULL;

	// Error if definition string doesn't contain an equals sign
	auto location = aDefinition.find("=");
	if (location == string::npos) {
		m_error << "Error: no \"=\" in option definition: " << aDefinition << endl;
		return -1;
	}

	string aliases = aDefinition.substr(0, location);
	string rest    = aDefinition.substr(location+1);
	string otype   = rest;
	string ovalue  = "";

	location = rest.find(":");
	if (location != string::npos) {
		otype  = rest.substr(0, location);
		ovalue = rest.substr(location+1);
	}

	// Remove anyspaces in the option type field
	otype.erase(remove_if(otype.begin(), otype.end(), ::isspace), otype.end());

	// Option types are only a single charater (b, i, d, c or s)
	if (otype.size() != 1) {
		m_error << "Error: option type is invalid: " << otype
			  << " in option definition: " << aDefinition << endl;
		return -1;
	}

	// Check to make sure that the type is known
	if (otype[0] != OPTION_STRING_TYPE  &&
		 otype[0] != OPTION_INT_TYPE     &&
		 otype[0] != OPTION_FLOAT_TYPE   &&
		 otype[0] != OPTION_DOUBLE_TYPE  &&
		 otype[0] != OPTION_BOOLEAN_TYPE &&
		 otype[0] != OPTION_CHAR_TYPE ) {
		m_error << "Error: unknown option type \'" << otype[0]
			  << "\' in defintion: " << aDefinition << endl;
		return -1;
	}

	// Set up space for a option entry in the registry
	definitionEntry = new Option_register(aDefinition, otype[0], ovalue);

	auto definitionIndex = m_optionRegister.size();

	// Store option aliases
	string optionName;
	unsigned int i;
	aliases += '|';
	for (i=0; i<aliases.size(); i++) {
		if (::isspace(aliases[i])) {
			continue;
		} else if (aliases[i] == '|') {
			if (isDefined(optionName)) {
				m_error << "Option \"" << optionName << "\" from definition:" << endl;
				m_error << "\t" << aDefinition << endl;
				m_error << "is already defined in: " << endl;
				m_error << "\t" << getDefinition(optionName) << endl;
				return -1;
			}
			if (optionName.size() > 0) {
				m_optionList[optionName] = definitionIndex;
			}
			optionName.clear();
		} else {
			optionName += aliases[i];
		}
	}

	// Store definition in registry and return its indexed location.
	// This location will be used to link option aliases to the main
	// command name.
	m_optionRegister.push_back(definitionEntry);
	return definitionIndex;
}


int Options::define(const string& aDefinition, const string& aDescription) {
	int index = define(aDefinition);
	m_optionRegister[index]->setDescription(aDescription);
	return index;
}



//////////////////////////////
//
// Options::isDefined -- Return true if option is present in registry.
//

int Options::isDefined(const string& name) {
	if (m_optionList.find(name) == m_optionList.end()) {
		return 0;
	} else {
		return 1;
	}
}



//////////////////////////////
//
// Options::getArg -- returns the specified argument.
//	argurment 0 is the command name.
//

string Options::getArg(int index) {
	if (index == 0) {
		if (m_argv.empty()) {
			return "";
		} else {
			return m_argv[0];
		}
	}
	if (index < 1 || index > (int)m_arguments.size()) {
		m_error << "Error: argument " << index << " does not exist." << endl;
		return "";
	}
	return m_arguments[index - 1];
}

// Alias:

string Options::getArgument(int index) {
	return getArg(index);
}



//////////////////////////////
//
// Options::getArgCount --  number of arguments on command line.
//	does not count the options or the command name.
//

int Options::getArgCount(void) {
	return m_arguments.size();
}

// Alias:

int Options::getArgumentCount(void) {
	return getArgCount();
}



//////////////////////////////
//
// Options::getArgList -- return a string vector of the arguments
//     after the options have been parsed out of it.  This list
//     excludes the command name (uses Options::getCommand() for that).
//

vector<string>& Options::getArgList(vector<string>& output) {
	output = m_arguments;
	return output;
}

// Alias:

vector<string>& Options::getArgumentList(vector<string>& output) {
	return getArgList(output);
}



//////////////////////////////
//
// Options::getBoolean --  returns true if the option was
//	used on the command line.
//

int Options::getBoolean(const string& optionName) {
	int index = getRegIndex(optionName);
	if (index < 0) {
		return 0;
	}
	return m_optionRegister[index]->isModified();
}



//////////////////////////////
//
// Options::getCommand -- returns argv[0] (the first string
//     in the original argv list.
//

string Options::getCommand(void) {
	if (m_argv.empty()) {
		return "";
	} else {
		return m_argv[0];
	}
}



//////////////////////////////
//
// Options::getCommandLine -- returns a string which contains the
//     command-line call to the program, including any appended
//     options.  This command only works after .process() is run.
//

string Options::getCommandLine(void) {
	string output;
	for (int i=0; i<(int)m_argv.size(); i++) {
		// check for how " and ' are dealt with in m_arguments...
		output += m_argv[i];
		if ((int)output.size() < (int)m_argv.size() - 1) {
			output += ' ';
		}
	}
	return output;
}



//////////////////////////////
//
// Options::getDefinition -- returns the definition for the specified
//      option name.  Returns empty string if there is no entry for
//      the option name.  spaces count in the input option name.
//

string Options::getDefinition(const string& optionName) {
	auto it = m_optionList.find(optionName);
	if (it == m_optionList.end()) {
		return "";
	} else {
		return m_optionRegister[it->second]->getDefinition();
	}
}



//////////////////////////////
//
// Options::getDouble -- returns the double float associated
//	with the given option.  Returns 0 if there is no
//	number associated with the option.
//

double Options::getDouble(const string& optionName) {
	return strtod(getString(optionName).c_str(), (char**)NULL);
}



//////////////////////////////
//
// Options::getChar -- Return the first character in the option string;
//      If the length is zero, then return '\0'.
//

char Options::getChar(const string& optionName) {
	return getString(optionName).c_str()[0];
}



//////////////////////////////
//
// Options::getFloat -- Return the floating point number
//	associated with the given option.
//

float Options::getFloat(const string& optionName) {
	return (float)getDouble(optionName);
}



//////////////////////////////
//
// Options::getInt -- Return the integer argument.  Can handle
//	hexadecimal, decimal, and octal written in standard
//	C syntax.
//

int Options::getInt(const string& optionName) {
	return (int)strtol(getString(optionName).c_str(), (char**)NULL, 0);
}

int Options::getInteger(const string& optionName) {
	return getInt(optionName);
}



//////////////////////////////
//
// Options::getString -- Return the option argument string.
//

string Options::getString(const string& optionName) {
	int index = getRegIndex(optionName);
	if (index < 0) {
		return "UNKNOWN OPTION";
	} else {
		return m_optionRegister[index]->getOption();
	}
}



//////////////////////////////
//
// Options::optionsArg -- Return true if --options is present
//    on the command line, otherwise returns false.
//

int Options::optionsArg(void) {
	return m_optionsArgQ;
}



//////////////////////////////
//
// Options::print -- Print a list of the defined options.
//

ostream& Options::print(ostream& out) {
	for (unsigned int i=0; i<m_optionRegister.size(); i++) {
		out << m_optionRegister[i]->getDefinition() << "\t"
			  << m_optionRegister[i]->getDescription() << endl;
	}
	return out;
}



//////////////////////////////
//
// Options::reset -- Clear all defined options.
//

void Options::reset(void) {
	m_argv.clear();
	m_arguments.clear();

	for (int i=0; i<(int)m_optionRegister.size(); i++) {
		delete m_optionRegister[i];
		m_optionRegister[i] = NULL;
	}
	m_optionRegister.clear();
}



//////////////////////////////
//
// Options::getFlag -- Set the character which is usually set to a dash.
//

char Options::getFlag(void) {
	return m_optionFlag;
}



//////////////////////////////
//
// Options::setFlag -- Set the character used to indicate an
//	option.  For unix this is usually '-', in MS-DOS,
//	this is usually '/';  But the syntax of the Options
//	class is for Unix-style options.
//

void Options::setFlag(char aFlag) {
	m_optionFlag = aFlag;
}



//////////////////////////////
//
// Options::setModified --
//

void Options::setModified(const string& optionName, const string& aString) {
	int index = getRegIndex(optionName);
	if (index < 0) {
		return;
	}

	m_optionRegister[getRegIndex(optionName)]->setModified(aString);
}



//////////////////////////////
//
// Options::setOptions --  Store the input list of options.
//

void Options::setOptions(int argc, char** argv) {
	m_processedQ = 0;
	m_argv.resize(argc);
	for (int i=0; i<argc; i++) {
		m_argv[i] = argv[i];
	}
}


void Options::setOptions(vector<string>& argv) {
	m_processedQ = 0;
	m_argv = argv;
}


void Options::setOptions(string& args) {
	m_processedQ = 0;
   m_argv = tokenizeCommandLine(args);
}



//////////////////////////////
//
// Options::appendOptions -- Add argc and argv data to the current
//      list residing inside the Options class variable.
//

void Options::appendOptions(int argc, char** argv) {
	m_processedQ = 0;
	for (int i=0; i<argc; i++) {
		m_argv.push_back(argv[i]);
	}
}


void Options::appendOptions(vector<string>& argv) {
	m_processedQ = 0;
	for (int i=0; i<(int)argv.size(); i++) {
		m_argv.push_back(argv[i]);
	}
}



//////////////////////////////
//
// Options::appendOptions -- parse the string like command-line arguments.
//   Either double or single quotes can be used to encapsulate
//   a command-line token.  If double quotes are used to encapsulate,
//   then you will not have to back quote single quotes inside the
//   token string, but you will have to backslash double quotes:
//      "-T \"\"" but "-T ''"
//   Likewise for single quotes in reverse with double quotes:
//      '-T \'\'' is equal to: '-T ""'
//

void Options::appendOptions(string& args) {
	vector<string> arglist = tokenizeCommandLine(args);
	appendOptions(arglist);
}



//////////////////////////////
//
// Options::tokenizeCommandLine -- Parse a string for individual
//    command-line strings.
//

vector<string> Options::tokenizeCommandLine(string& arguments) {
	char ch;
	int doublequote = 0;
	int singlequote = 0;

	vector<string> tokens;
	vector<string> tempargv;
	string tempvalue;

	tokens.reserve(100);
	tempargv.reserve(100);
	tempvalue.reserve(1000);

	for (int i=0; i<(int)arguments.size(); i++) {
		if (!singlequote && (arguments[i] == '"')) {
			if ((i>0) && (arguments[i-1] != '\\')) {
				doublequote = !doublequote;
				if (doublequote == 0) {
					// finished a doublequoted section of data, so store
					// even if it is the empty string
					ch = '\0';
					tempvalue += (ch);
					tokens.push_back(tempvalue);
					tempvalue.clear();
					continue;
				} else {
					// don't store the leading ":
					continue;
				}
			}
		} else if (!doublequote && (arguments[i] == '\'')) {
			if ((i>0) && (arguments[i-1] != '\\')) {
				singlequote = !singlequote;
				if (singlequote == 0) {
					// finished a singlequote section of data, so store
					// even if it is the empty string
					ch = '\0';
					tempvalue += ch;
					tokens.push_back(tempvalue);
					tempvalue.clear();
					continue;
				} else {
					// don't store the leading ":
					continue;
				}
			}
		}

		if ((!doublequote && !singlequote) && std::isspace(arguments[i])) {
			if (tempvalue.size() > 0) {
				// tempvalue += ch;
				tokens.push_back(tempvalue);
				tempvalue.clear();
			}
		} else {
			ch = arguments[i];
			tempvalue += ch;
		}
	}
	if (tempvalue.size() > 0) {
		tokens.push_back(tempvalue);
		tempvalue.clear();
	}

	return tokens;

}



//////////////////////////////
//
// Options:getType -- Return the type of the option.
//

char Options::getType(const string& optionName) {
	int index = getRegIndex(optionName);
	if (index < 0) {
		return -1;
	} else {
		return m_optionRegister[getRegIndex(optionName)]->getType();
	}
}



//////////////////////////////
//
// Options::process -- Same as xverify.
//   	default values: error_check = 1, suppress = 0;
//

bool Options::process(int argc, char** argv, int error_check, int suppress) {
	setOptions(argc, argv);
	xverify(error_check, suppress);
	return !hasParseError();
}


bool Options::process(vector<string>& argv, int error_check, int suppress) {
	setOptions(argv);
	xverify(error_check, suppress);
	return !hasParseError();
}


bool Options::process(string& argv, int error_check, int suppress) {
	setOptions(argv);
	xverify(error_check, suppress);
	return !hasParseError();
}


bool Options::process(int error_check, int suppress) {
	xverify(error_check, suppress);
	return !hasParseError();
}



//////////////////////////////
//
// Options::xverify --
//	default value: error_check = 1, suppress = 0;
//

void Options::xverify(int argc, char** argv, int error_check, int suppress) {
	setOptions(argc, argv);
	xverify(error_check, suppress);
}


void Options::xverify(int error_check, int suppress) {
	m_options_error_checkQ = error_check;
	m_suppressQ = suppress ? true : false;


	// if calling xverify again, must remove previous argument list.
	if (m_arguments.size() != 0) {
		m_arguments.clear();
	}

	int position   = 0;
	int running    = 0;
	bool optionend = false;
	int i          = 1;
	int oldi;
	int terminate = 1000; // for malformed options (missing arguments)
	int tcount = 0;

	while ((i < (int)m_argv.size()) && !optionend) {
		tcount++;
		if (tcount > terminate) {
			m_error << "Error: missing option argument" << endl;
			break;
		}
		if (isOption(m_argv[i], i)) {
			oldi = i;
			i = storeOption(i, position, running);
			if (i != oldi) {
				running = 0;
				position = 0;
			}
		} else {
			if (m_argv[i].size() == 2 && m_argv[i][0] == getFlag() &&
				m_argv[i][2] == getFlag() ) {
					optionend = 1;
				i++;
				break;
			} else {                          // this is an argument
				m_arguments.push_back(m_argv[i]);
				i++;
			}
		}
		if (hasParseError()) {
			break;
		}
	}
}





///////////////////////////////////////////////////////////////////////////
//
// private functions
//


//////////////////////////////
//
// Options::getRegIndex -- returns the index of the option associated
//	with this name.
//

int Options::getRegIndex(const string& optionName) {
	if (m_suppressQ && (optionName == "options")) {
			return -1;
	}

	if (optionName == "options") {
		print(cout);
		return -1;
	}

	auto it = m_optionList.find(optionName);
	if (it == m_optionList.end()) {
		if (m_options_error_checkQ) {
			m_error << "Error: unknown option \"" << optionName << "\"." << endl;
			print(cout);
			return -1;
		} else {
			return -1;
		}
	} else {
		return it->second;
	}
}



//////////////////////////////
//
// Options::isOption --  returns true if the string is an option.
//	"--" is not an option, also '-' is not an option.
//	aString is assumed to not be NULL.
//

bool Options::isOption(const string& aString, int& argp) {
	if (aString[0] == getFlag()) {
		if (aString[1] == '\0') {
			argp++;
			return false;
		} else if (aString[1] == getFlag()) {
			if (aString[2] == '\0') {
				argp++;
				return false;
			} else {
				return true;
			}
		} else {
			return true;
		}
	} else {
		return false;
	}
}



//////////////////////////////
//
// Options::storeOption --
//

#define OPTION_FORM_SHORT     0
#define OPTION_FORM_LONG      1
#define OPTION_FORM_CONTINUE  2

int Options::storeOption(int index, int& position, int& running) {
	int optionForm;
	char tempname[1024];
	char optionType = '\0';

	if (running) {
		optionForm = OPTION_FORM_CONTINUE;
	} else if (m_argv[index][1] == getFlag()) {
		optionForm = OPTION_FORM_LONG;
	} else {
		optionForm = OPTION_FORM_SHORT;
	}

	switch (optionForm) {
		case OPTION_FORM_CONTINUE:
			position++;
			tempname[0] = m_argv[index][position];
			tempname[1] = '\0';
			optionType = getType(tempname);
			if (optionType != OPTION_BOOLEAN_TYPE) {
				running = 0;
				position++;
			}
			break;
		case OPTION_FORM_SHORT:
			position = 1;
			tempname[0] = m_argv[index][position];
			tempname[1] = '\0';
			optionType = getType(tempname);
			if (optionType != OPTION_BOOLEAN_TYPE) {
				position++;
			}
			break;
		case OPTION_FORM_LONG:
			position = 2;
			while (m_argv[index][position] != '=' &&
					m_argv[index][position] != '\0') {
				tempname[position-2] = m_argv[index][position];
				position++;
			}
			tempname[position-2] = '\0';
			optionType = getType(tempname);
			if (optionType == -1) {         // suppressed --options option
				m_optionsArgQ = 1;
				break;
			}
			if (m_argv[index][position] == '=') {
				if (optionType == OPTION_BOOLEAN_TYPE) {
					m_error << "Error: boolean variable cannot have any options: "
						  << tempname << endl;
					return -1;
				}
				position++;
			}
			break;
	}

	if (optionType == -1) {              // suppressed --options option
		m_optionsArgQ = 1;
		index++;
		position = 0;
		return index;
	}

	if (m_argv[index][position] == '\0' &&
			optionType != OPTION_BOOLEAN_TYPE) {
		index++;
		position = 0;
	}

	if (optionForm != OPTION_FORM_LONG && optionType == OPTION_BOOLEAN_TYPE &&
			m_argv[index][position+1] != '\0') {
		running = 1;
	} else if (optionType == OPTION_BOOLEAN_TYPE &&
			m_argv[index][position+1] == '\0') {
		running = 0;
	}

	if (index >= (int)m_argv.size()) {
		m_error << "Error: last option requires a parameter" << endl;
		return -1;
	}
	setModified(tempname, &m_argv[index][position]);

	if (!running) {
		index++;
	}
	return index;
}



//////////////////////////////
//
// Options::printOptionList --
//

ostream& Options::printOptionList(ostream& out) {
	for (auto it = m_optionList.begin(); it != m_optionList.end(); it++) {
		out << it->first << "\t" << it->second << endl;
	}
	return out;
}



//////////////////////////////
//
// Options::printOptionBooleanState --
//

ostream& Options::printOptionListBooleanState(ostream& out) {
	for (auto it = m_optionList.begin(); it != m_optionList.end(); it++) {
		out << it->first << "\t"
			 << m_optionRegister[it->second]->isModified() << endl;
	}
	return out;
}



//////////////////////////////
//
// Options::printRegister --
//

ostream& Options::printRegister(ostream& out) {
	for (auto it = m_optionRegister.begin(); it != m_optionRegister.end(); it++) {
		(*it)->print(out);
	}
	return out;
}



/////////////////////////////
//
// Options::hasParseError -- Returns true if there was an error parsing
//     the arguments.
//

bool Options::hasParseError(void) {
	return !m_error.str().empty();
}



//////////////////////////////
//
// Options::getParseError --
//

string Options::getParseError(void) {
	return m_error.str();
}


ostream& Options::getParseError(ostream& out) {
	out << m_error.str();
	return m_error;
}


// END_MERGE

} // end namespace Humdrum



