//
// Copyright 1998-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Apr  5 13:07:18 PDT 1998
// Last Modified: Sat Mar  1 09:27:49 PST 2014 Implemented with STL.
// Filename:      Options.h
// Web Address:   https://github.com/craigsapp/humlib/blob/master/include/Options.h
// Documentation: http://sig.sapp.org/doc/classes/Options
// vim:           syntax=cpp ts=3 noexpandtab nowrap
// Syntax:        C++11
//
// Description:   Interface to command-line options.
//

#ifndef _OPTIONS_H_INCLUDED
#define _OPTIONS_H_INCLUDED

#include <vector>
#include <map>
#include <string>
#include <sstream>

using namespace std;

namespace hum {

// START_MERGE

class Option_register {
	public:
		         Option_register     (void);
		         Option_register     (const string& aDefinition, char aType,
		                                  const string& aDefaultOption);
		         Option_register     (const string& aDefinition, char aType,
		                                  const string& aDefaultOption,
		                                  const string& aModifiedOption);
		         Option_register     (const Option_register& reg);
		        ~Option_register     ();

		Option_register& operator=(const Option_register& reg);
		void     clearModified      (void);
		string   getDefinition      (void);
		string   getDefault         (void);
		string   getOption          (void);
		string   getModified        (void);
		string   getDescription     (void);
		int      isModified         (void);
		char     getType            (void);
		void     reset              (void);
		void     setDefault         (const string& aString);
		void     setDefinition      (const string& aString);
		void     setDescription     (const string& aString);
		void     setModified        (const string& aString);
		void     setType            (char aType);
		ostream& print              (ostream& out);

	protected:
		string       m_definition;
		string       m_description;
		string       m_defaultOption;
		string       m_modifiedOption;
		int          m_modifiedQ;
		char         m_type;
};


class Options {
	public:
		                Options           (void);
		                Options           (int argc, char** argv);
		                Options           (const Options& options);
		               ~Options           ();

		Options&        operator=         (const Options& options);
		int             argc              (void) const;
		const vector<string>& argv        (void) const;
		int             define            (const string& aDefinition);
		int             define            (const string& aDefinition,
		                                   const string& description);
		string          getArg            (int index);
		string          getArgument       (int index);
		int             getArgCount       (void);
		int             getArgumentCount  (void);
		vector<string>& getArgList        (vector<string>& output);
		vector<string>& getArgumentList   (vector<string>& output);
		int             getBoolean        (const string& optionName);
		string          getCommand        (void);
		string          getCommandLine    (void);
		string          getDefinition     (const string& optionName);
		double          getDouble         (const string& optionName);
		char            getFlag           (void);
		char            getChar           (const string& optionName);
		float           getFloat          (const string& optionName);
		int             getInt            (const string& optionName);
		int             getInteger        (const string& optionName);
		string          getString         (const string& optionName);
		char            getType           (const string& optionName);
		int             optionsArg        (void);
		ostream&        print             (ostream& out);
		ostream&        printOptionList   (ostream& out);
		ostream&        printOptionListBooleanState(ostream& out);
		bool            process           (int error_check = 1, int suppress = 0);
		bool            process           (int argc, char** argv,
		                                      int error_check = 1,
		                                      int suppress = 0);
		bool            process           (const vector<string>& argv,
		                                      int error_check = 1,
		                                      int suppress = 0);
		bool            process           (const string& argv, int error_check = 1,
		                                      int suppress = 0);
		void            reset             (void);
		void            xverify           (int argc, char** argv,
		                                      int error_check = 1,
		                                      int suppress = 0);
		void            xverify           (int error_check = 1,
		                                      int suppress = 0);
		void            setFlag           (char aFlag);
		void            setModified       (const string& optionName,
		                                   const string& optionValue);
		void            setOptions        (int argc, char** argv);
		void            setOptions        (const vector<string>& argv);
		void            setOptions        (const string& args);
		void            appendOptions     (int argc, char** argv);
		void            appendOptions     (string& args);
		void            appendOptions     (vector<string>& argv);
		ostream&        printRegister     (ostream& out);
		int             isDefined         (const string& name);
		static vector<string> tokenizeCommandLine(const string& args);
		bool            hasParseError     (void);
		string          getParseError     (void);
		ostream&        getParseError     (ostream& out);

	protected:
		// m_argv: the list of raw command line strings including
		// a mix of options and non-option argument.
		vector<string> m_argv;

		// m_arguments: list of parsed command-line arguments which
		// are not options, or the command (argv[0]);
		vector<string> m_arguments;

		// m_optionRegister: store for the states/values of each option.
		vector<Option_register*> m_optionRegister;

		// m_optionFlag: the character which indicates an option.
		// Generally a dash, but could be made a slash for Windows environments.
		char m_optionFlag = '-';

		// m_optionList:
		map<string, int> m_optionList;

		//
		// boolern options for object:
		//

		// m_options_error_check: for .verify() function.
		bool m_options_error_checkQ = true;

		// m_processedQ: true if process() was run.  This will parse
		// the command-line arguments into a list of options, and also
		// enable boolean versions of the options.
		bool m_processedQ = false;

		// m_suppressQ: true means to suppress automatic --options option
		// listing.
		bool m_suppressQ = false;

		// m_optionsArgument: indicate that --options was used.
		bool m_optionsArgQ = false;

		// m_error: used to store errors in parsing command-line options.
		stringstream m_error;

	private:
		int     getRegIndex    (const string& optionName);
		bool    isOption       (const string& aString, int& argp);
		int     storeOption    (int gargp, int& position, int& running);
};

#define OPTION_BOOLEAN_TYPE   'b'
#define OPTION_CHAR_TYPE      'c'
#define OPTION_DOUBLE_TYPE    'd'
#define OPTION_FLOAT_TYPE     'f'
#define OPTION_INT_TYPE       'i'
#define OPTION_STRING_TYPE    's'
#define OPTION_UNKNOWN_TYPE   'x'


// END_MERGE

} // end namespace hum

#endif  /* _OPTIONS_H_INCLUDED */



