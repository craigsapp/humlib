// Description: Example of identifying intermediate step-wise notes
// (note is marked if the note before and after are either rising
//  by a test or falling by a step).

#include "humlib.h"

using namespace std;
using namespace hum;

// function declarations:
vector<HTp> extractNoteAttacks(HTp start);

int main(int argc, char** argv) {

   // handle command-line options (-a, -p, and -k #)
   Options opts;
   opts.define("a|append=b", "append analysis to end of input");
   opts.define("p|prepend=b", "prepend analysis to start of input");
   opts.define("k|kern-spine=i:1", "kern spine to process");
   opts.process(argc, argv);

   // read an inputfile from the first filename argument, or standard input
   HumdrumFile infile;
   if (opts.getArgCount() > 0) {
      infile.read(opts.getArgument(1));
   } else {
      infile.read(cin);
   }

   // get a list of the **kern spines in the file
   vector<HTp> kernspines = infile.getKernSpineStartList();
   int spine = opts.getInteger("kern-spine") - 1;
   if (spine < 0 || spine > (int)kernspines.size()) {
      cerr << "Error: kern spine " << spine+1 << " is out of range." << endl;
      cerr << "Maximum kern spine number is: " << kernspines.size() << endl;
      exit(1);
   }

   // extract diatonic pitch list from desired spine
   #define REST -1000
   vector<HTp> notes = extractNoteAttacks(kernspines[spine]);
   vector<int> diatonic(notes.size(), REST);
   for (int i=0; i<(int)notes.size(); i++) {
      if (!notes[i]->isRest()) {
         diatonic[i] = Convert::kernToBase7(notes[i]);
      }
   }

   // search for adjacent step-wise motion
   vector<string> analysis(infile.getLineCount());
   int line;  // line in original file data of note.
   int interval1, interval2;
   int count = 0;
   for (int i=1; i<(int)diatonic.size()-1; i++) {
      interval1 = diatonic[i] - diatonic[i-1];
      interval2 = diatonic[i+1] - diatonic[i];
      if ((interval1 == 1) && (interval2 == 1)) {
         line = notes[i]->getLineIndex();
         analysis[line] = "u";
         count++;
      }
      if ((interval1 == -1) && (interval2 == -1)) {
         line = notes[i]->getLineIndex();
         analysis[line] = "d";
         count++;
      }
   }

   // print the results:
   if (opts.getBoolean("append")) {
      infile.appendDataSpine(analysis);
      cout << infile;
   } else if (opts.getBoolean("prepend")) {
      infile.prependDataSpine(analysis);
      cout << infile;
   } else {
      cout << count << " passing notes" << endl;
   }

   return 0;
}



//////////////////////////////
//
// extractNoteAttacks -- Generate a list of melodic note attacks in
//     a **kern spine, skipping over tied note sustains, but keeping
//     rests (but not secondary rests).
//

vector<HTp> extractNoteAttacks(HTp start) {
   vector<HTp> output;
   HTp token = start->getNextNonNullDataToken();
   HTp lasttoken = token;
   while (token) {
      if (lasttoken && lasttoken->isRest() && token->isRest()) {
         lasttoken = token;
         token = token->getNextNonNullDataToken();
      }
      if (!token->isSecondaryTiedNote()) {
         output.push_back(token);
      }
      lasttoken = token;
      token = token->getNextNonNullDataToken();
   }
   return output;
}




