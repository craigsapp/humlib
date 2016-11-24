// Description: Example of extracting a 2D pitch grid from
// a score for dissonance analysis.

#include "humlib.h"

#define REST -1000

using namespace std;
using namespace hum;

enum OutputStyle {
   Raw,
   Note,
   Diatonic,
   Midi,
   Base40
};

// function declarations:
void    extractNoteGrid  (vector<vector<HTp> >& grid, HumdrumFile& infile);
void    printGrid        (vector<vector<HTp> >& grid, OutputStyle style);
string  getKernPitch     (HTp token);
int     getDiatonicPitch (HTp token);
int     getMidiPitch     (HTp token);
int     getBase40Pitch   (HTp token);

int main(int argc, char** argv) {

   // handle command-line options:
   Options opts;
   opts.define("r|raw=b",        "print raw grid");
   opts.define("d|diatonic=b",   "print diatonic grid");
   opts.define("m|midi-pitch=b", "print midie-pitch grid");
   opts.define("b|base-40=b",    "print base-40 grid");
   opts.process(argc, argv);

   OutputStyle style = Note;
   if (opts.getBoolean("raw")) {
      style = Raw;
   } else if (opts.getBoolean("diatonic")) {
      style = Diatonic;
   } else if (opts.getBoolean("midi-pitch")) {
      style = Midi;
   } else if (opts.getBoolean("base-40")) {
      style = Base40;
   }

   // read an inputfile from the first filename argument, or standard input
   HumdrumFile infile;
   if (opts.getArgCount() > 0) {
      infile.read(opts.getArgument(1));
   } else {
      infile.read(cin);
   }

   // extract diatonic pitch list from desired spine
   vector<vector<HTp> > grid;
   extractNoteGrid(grid, infile);

   printGrid(grid, style);

   return 0;
}



///////////////////////////////
//
// printToken -- print the data token in a variety of styles.
//

void printToken(HTp token, OutputStyle style) {
   if (token == NULL) {
      // shouldn't happen
      return;
   }

   switch (style) {
      case Raw:
         cout << token;
         break;

      case Note:
         cout << getKernPitch(token);
         break;
         
      case Diatonic:
         cout << getDiatonicPitch(token);
         break;

      case Midi:
         cout << getMidiPitch(token);
         break;

      case Base40:
         cout << getBase40Pitch(token);
         break;
   }
}



//////////////////////////////
//
// getKernPitch -- return **kern pitch of a note.
//     returns "R" for rests and puts parenthese around notes
//     that are sustains.
//

string getKernPitch(HTp token) {
   if (token->isRest()) {
      return "R";
   }
   int b40  = Convert::kernToBase40(token->resolveNull());
   string pitch = Convert::base40ToKern(b40);
   string output;
   bool sustain = token->isNull() || token->isSecondaryTiedNote();
   if (sustain) {
      output = "(";
      output += pitch;
      output += ")";
   } else {
      output = pitch;
   }
   return output;
}



//////////////////////////////
//
// getDiatonicPitch -- return turn diatonic value of a pitch.
//     returns 0 for rests and negative values for 
//     sustains.
//

int getDiatonicPitch(HTp token) {
   bool sustain = token->isNull() || token->isSecondaryTiedNote();
   if (token->isRest()) {
      return 0;
   } else {
      int b7 = Convert::kernToBase7(token->resolveNull());
      return (sustain ? -b7 : b7);
   }
}



//////////////////////////////
//
// getMidiPitch -- return turn MIDI key number of a pitch.
//     returns 0 for rests and negative values for 
//     sustains.
//

int getMidiPitch(HTp token) {
   bool sustain = token->isNull() || token->isSecondaryTiedNote();
   if (token->isRest()) {
      return 0;
   } else {
      int b12 = Convert::kernToMidiNoteNumber(token->resolveNull());
      return (sustain ? -b12 : b12);
   }
}



//////////////////////////////
//
// getBase40Pitch -- return turn diatonic value of a pitch.
//     returns 0 for rests and negative values for 
//     sustains.
//

int getBase40Pitch(HTp token) {
   bool sustain = token->isNull() || token->isSecondaryTiedNote();
   if (token->isRest()) {
      return 0;
   } else {
      int b40 = Convert::kernToBase40(token->resolveNull());
      return (sustain ? -b40 : b40);
   }
}



//////////////////////////////
//
// printGrid --
//

void printGrid(vector<vector<HTp> >& grid, OutputStyle style) {

   for (int j=0; j<(int)grid[0].size(); j++) {
      for (int i=0; i<(int)grid.size(); i++) {
         printToken(grid[i][j], style);
         if (i < (int)grid.size() - 1) {
            cout << "\t";
         }
      }
      cout << endl;
   }
}



//////////////////////////////
//
// extractNoteAttacks -- Generate a two-dimensional list of notes
//     in a score.  Each row has at least one note attack.
//

void extractNoteGrid(vector<vector<HTp> >& grid, HumdrumFile& infile) {
   vector<HTp> kernspines = infile.getKernSpineStartList();

   if (kernspines.size() == 0) {
      cerr << "Error: no **kern spines in file" << endl;
      exit(1);
   }

   grid.resize(kernspines.size());
   for (int i=0; i<(int)grid.size(); i++) {
      grid[i].reserve(infile.getLineCount());
   }

   int attack = 0;
   int track, lasttrack;
   vector<HTp> current;
   for (int i=0; i<infile.getLineCount(); i++) {
      if (!infile[i].isData()) {
         continue;
      }
      track = 0;
      attack = 0;
      current.clear();
      for (int j=0; j<infile[i].getFieldCount(); j++) {
         lasttrack = track;
         track = infile.token(i, j)->getTrack();
         if (!infile[i].token(j)->isDataType("**kern")) {
            continue;
         }
         if (track == lasttrack) {
            // secondary voice: ignore
            continue;
         } 
         current.push_back(infile.token(i, j));
         if (!(current.back()->isRest() 
            || current.back()->isSecondaryTiedNote())) {
            attack++;
         }
      }
      if (attack == 0) {
         continue;
      }
      if (current.size() != kernspines.size()) {
         cerr << "Unequal vector sizes " << current.size() 
              << " compared to " << kernspines.size() << endl;
      }
      for (int j=0; j<(int)current.size(); j++) {
         grid[j].push_back(current[j]);
      }
   }
}



