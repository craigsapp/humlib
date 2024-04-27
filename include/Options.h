//
// Copyright 1998-2000 by Craig Stuart Sapp, All Rights Reserved.
// Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
// Creation Date: Sun Apr  5 13:07:18 PDT 1998
// Last Modified: Sat Mar  1 09:27:49 PST 2014 Implemented with STL.
// Filename:      Options.h
// Web Address:   https://github.com/craigsapp/humlib/blob/master/include/Options.h
// Documentation: http://sig.sapp.org/doc/classes/Options
// vim:           syntax=cpp ts=3 noexpandtab nowrap
// Syntax:        C++11; humlib
//
// Description:   Interface to command-line options.
//

#ifndef _OPTIONS_H_INCLUDED
#define _OPTIONS_H_INCLUDED

#include <vector>
#include <map>
#include <string>
#include <sstream>

namespace hum {

// START_MERGE

class Option_register {
	public:
		                 Option_register (void);
		                 Option_register (const std::string& aDefinition, char aType,
		                                  const std::string& aDefaultOption);
		                 Option_register (const std::string& aDefinition, char aType,
		                                  const std::string& aDefaultOption,
		                                  const std::string& aModifiedOption);
		                 Option_register (const Option_register& reg);
		                ~Option_register ();

		Option_register& operator=       (const Option_register& reg);
		void             clearModified   (void);
		std::string      getDefinition   (void);
		std::string      getDefault      (void);
		std::string      getOption       (void);
		std::string      getModified     (void);
		std::string      getDescription  (void);
		bool             isModified      (void);
		char             getType         (void);
		void             reset           (void);
		void             setDefault      (const std::string& aString);
		void             setDefinition   (const std::string& aString);
		void             setDescription  (const std::string& aString);
		void             setModified     (const std::string& aString);
		void             setType         (char aType);
		std::ostream&    print           (std::ostream& out);

	protected:
		std::string      m_definition;
		std::string      m_description;
		std::string      m_defaultOption;
		std::string      m_modifiedOption;
		bool             m_modifiedQ;
		char             m_type;
};


class Options {
	public:
		                Options           (void);
		                Options           (int argc, char** argv);
		                Options           (const Options& options);
		               ~Options           ();

		Options&        operator=         (const Options& options);
		int             argc              (void) const;
		const std::vector<std::string>& argv (void) const;
		int             define            (const std::string& aDefinition);
		int             define            (const std::string& aDefinition,
		                                   const std::string& description);
		std::string     getArg            (int index);
		std::string     getArgument       (int index);
		int             getArgCount       (void);
		int             getArgumentCount  (void);
		std::vector<std::string>& getArgList        (std::vector<std::string>& output);
		std::vector<std::string>& getArgumentList   (std::vector<std::string>& output);
		bool            getBoolean        (const std::string& optionName);
		std::string     getCommand        (void);
		std::string     getCommandLine    (void);
		std::string     getDefinition     (const std::string& optionName);
		double          getDouble         (const std::string& optionName);
		char            getFlag           (void);
		char            getChar           (const std::string& optionName);
		float           getFloat          (const std::string& optionName);
		int             getInt            (const std::string& optionName);
		int             getInteger        (const std::string& optionName);
		std::string     getString         (const std::string& optionName);
		char            getType           (const std::string& optionName);
		int             optionsArg        (void);
		std::ostream&   print             (std::ostream& out);
		std::ostream&   printEmscripten   (std::ostream& out);
		std::ostream&   printOptionList   (std::ostream& out);
		std::ostream&   printOptionListBooleanState(std::ostream& out);
		bool            process           (int error_check = 1, int suppress = 0);
		bool            process           (int argc, char** argv,
		                                   int error_check = 1,
		                                   int suppress = 0);
		bool            process           (const std::vector<std::string>& argv,
		                                   int error_check = 1,
		                                   int suppress = 0);
		bool            process           (const std::string& argv, int error_check = 1,
		                                   int suppress = 0);
		void            reset             (void);
		void            xverify           (int argc, char** argv,
		                                   int error_check = 1,
		                                   int suppress = 0);
		void            xverify           (int error_check = 1,
		                                   int suppress = 0);
		void            setFlag           (char aFlag);
		void            setModified       (const std::string& optionName,
		                                   const std::string& optionValue);
		void            setOptions        (int argc, char** argv);
		void            setOptions        (const std::vector<std::string>& argv);
		void            setOptions        (const std::string& args);
		void            appendOptions     (int argc, char** argv);
		void            appendOptions     (std::string& args);
		void            appendOptions     (std::vector<std::string>& argv);
		std::ostream&   printRegister     (std::ostream& out);
		int             isDefined         (const std::string& name);
		static std::vector<std::string> tokenizeCommandLine(const std::string& args);
		bool            hasParseError     (void);
		std::string     getParseError     (void);
		std::ostream&   getParseError     (std::ostream& out);

	protected:
		// m_argv: the list of raw command line strings including
		// a mix of options and non-option argument.
		std::vector<std::string> m_argv;

		// m_arguments: list of parsed command-line arguments which
		// are not options, or the command (argv[0]);
		std::vector<std::string> m_arguments;

		// m_optionRegister: store for the states/values of each option.
		std::vector<Option_register*> m_optionRegister;

		// m_optionFlag: the character which indicates an option.
		// Generally a dash, but could be made a slash for Windows environments.
		char m_optionFlag = '-';

		// m_optionList:
		std::map<std::string, int> m_optionList;

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
		std::stringstream m_error;

	protected:
		int     getRegIndex    (const std::string& optionName);
		bool    isOption       (const std::string& aString, int& argp);
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



