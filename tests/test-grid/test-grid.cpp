// Description: Example of extracting a 2D pitch grid from
// a score for dissonance analysis.

#include "humlib.h"

using namespace std;
using namespace hum;

int debug = 1;

enum OutputStyle {
   None,
   Raw,
   Kern,
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
void    fillDiatonicGrid (vector<vector<int> >& diatonic,
                          vector<vector<HTp> >& grid);
void    doAnalysis       (vector<vector<string> >& results,
                          vector<vector<HTp> >& grid);
void    doAnalysis       (vector<string>& results, vector<int>& data,
                          vector<HTp>& grid);
int     getPreviousAttack(vector<int>& data, int index);
int     getNextAttack    (vector<int>& data, int index);

void    doAnalysisB      (vector<string>& results, vector<int>& data,
                          vector<HTp>& grid);
int     getAttack        (int n, vector<int>& data, int index,
                          vector<int> attacks);


int main(int argc, char** argv) {

   // handle command-line options:
   Options opts;
   opts.define("r|raw=b",        "print raw grid");
   opts.define("d|diatonic=b",   "print diatonic grid");
   opts.define("m|midi-pitch=b", "print midie-pitch grid");
   opts.define("b|base-40=b",    "print base-40 grid");
   opts.define("k|kern=b",       "print kern pitch grid");
   opts.define("a|analysis=b",   "do melodic analysis");
   opts.process(argc, argv);

   OutputStyle style = None;
   if (opts.getBoolean("raw")) {
      style = Raw;
   } else if (opts.getBoolean("diatonic")) {
      style = Diatonic;
   } else if (opts.getBoolean("midi-pitch")) {
      style = Midi;
   } else if (opts.getBoolean("base-40")) {
      style = Base40;
   } else if (opts.getBoolean("kern")) {
      style = Kern;
   } else {
      style = None;
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

   vector<vector<string> > results;
   if (style != None) {
      printGrid(grid, style);
   } else {

      results.resize(grid.size());
      for (int i=0; i<(int)grid.size(); i++) {
         results[i].resize(infile.getLineCount());
      }

      doAnalysis(results, grid);
      for (int i=0; i<(int)results.size(); i++) {
         infile.appendDataSpine(results[i]);
      }
      cout << infile;
   }

   return 0;
}



//////////////////////////////
//
// doAnalysis -- do a basic melodic analysis of all parts.
//

void doAnalysis(vector<vector<string> >& results, vector<vector<HTp> >& grid) {
   vector<vector<int> > diatonic;
   fillDiatonicGrid(diatonic, grid);
   for (int i=0; i<(int)diatonic.size(); i++) {
      doAnalysisB(results[i], diatonic[i], grid[i]);
   }
}



//////////////////////////////
//
// doAnalysis -- since voice analysis.
//

void doAnalysis(vector<string>& results, vector<int>& data,
      vector<HTp>& vgrid) {
   int previous, next, current;
   int interval1, interval2;
   for (int i=1; i<(int)data.size() - 1; i++) {
      current = data[i];
      if (current <= 0) {
         continue;
      }
      previous = getPreviousAttack(data, i);
      if (previous <= 0) {
         continue;
      }
      next = getNextAttack(data, i);
      if (next <= 0) {
         continue;
      }
      interval1 = current - previous;
      interval2 = next - current;
      int lineindex = vgrid[i]->getLineIndex();
      if ((interval1 == 1) && (interval2 == 1)) {
         results[lineindex] = "pu";
      } else if ((interval1 == -1) && (interval2 == -1)) {
         results[lineindex] = "pd";
      } else if ((interval1 == 1) && (interval2 == -1)) {
         results[lineindex] = "nu";
      } else if ((interval1 == -1) && (interval2 == 1)) {
         results[lineindex] = "nd";
      }
   }
}


// 
// doAnalysisB() improves extraction speed of previous/next note attacks.
//

void doAnalysisB(vector<string>& results, vector<int>& data,
      vector<HTp>& vgrid) {
   int previous, next, current;
   int interval1, interval2;

   // prepare lookup tables for next/previous note attack (or rest).
   vector<int> lastattack(data.size(), -1);
   vector<int> nextattack(data.size(), -1);
   for (int i=0; i<(int)data.size(); i++) {
      if (data[i] >= 0) {
         lastattack[i] = i;
         nextattack[i] = i;
      }
   }

   int value = -1;
   int temp = -1;
   for (int i=(int)nextattack.size() - 1; i>= 0; i--) {
      if (nextattack[i] >= 0) {
         temp = nextattack[i];
         nextattack[i] = value;
         value = temp;
      } else {
         nextattack[i] = value;
      }
   }

   value = -1;
   temp = -1;
   for (int i=0; i<(int)nextattack.size(); i++) {
      if (lastattack[i] >= 0) {
         temp = lastattack[i];
         lastattack[i] = value;
         value = temp;
      } else {
         lastattack[i] = value;
      }
   }

   if (debug) {
      cout << "==============================" << endl;
      cout << "i\tnote\tnext\tprev\n";
      for (int i=0; i<(int)data.size(); i++) {
            cout << i << "\t" 
                 << data[i] << "\t" 
                 << nextattack[i] << "\t" 
                 << lastattack[i] << endl;
      }
      cout << endl;
   }


   for (int i=1; i<(int)data.size() - 1; i++) {
      current = data[i];
      if (current <= 0) {
         continue;
      }
      previous = getAttack(1, data, i, lastattack);
      if (previous <= 0) {
         continue;
      }
      next = getAttack(1, data, i, nextattack);
      if (next <= 0) {
         continue;
      }
      interval1 = current - previous;
      interval2 = next - current;
      int lineindex = vgrid[i]->getLineIndex();
      if ((interval1 == 1) && (interval2 == 1)) {
         results[lineindex] = "pu";
      } else if ((interval1 == -1) && (interval2 == -1)) {
         results[lineindex] = "pd";
      } else if ((interval1 == 1) && (interval2 == -1)) {
         results[lineindex] = "nu";
      } else if ((interval1 == -1) && (interval2 == 1)) {
         results[lineindex] = "nd";
      }
   }
}



//////////////////////////////
//
// getAttack --
//

int getAttack(int n, vector<int>& data, int index, vector<int> attacks) {
   int ordinal = 0;
   index = attacks[index];
   while ((index >= 0) && (ordinal < n)) {
      if (index < 0) {
      	 return 0;
      }
      ordinal++;
      if (ordinal == n) {
         return data[index];
      }
      index = attacks[index];
   }
   return 0;
}



//////////////////////////////
//
// getAttacks --
//

vector<int> getAttacks(int n, vector<int>& data, int index,
      vector<int> attacks) {
   vector<int> output(n, 0);
   int ordinal = 0;
   index = attacks[index];
   while ((index >= 0) && (ordinal < n)) {
      if (index < 0) {
      	 break;
      }
      ordinal++;
      if (ordinal <= n) {
         output[ordinal-1] = data[index];
      }
      if (ordinal == n) {
         break;
      }
      index = attacks[index];
   }
   return output;
}




///////////////////////////////
//
// getPreviousAttack --
//

int getPreviousAttack(vector<int>& data, int index) {
   for (int i = index-1; i>=0; i--) {
      if (data[i] >= 0) {
         return data[i];
      }
   }
   return 0;
}




///////////////////////////////
//
// getNextAttack --
//

int getNextAttack(vector<int>& data, int index) {
   for (int i = index+1; i<(int)data.size(); i++) {
      if (data[i] >= 0) {
         return data[i];
      }
   }
   return 0;
}



//////////////////////////////
//
// fillDiatonicGrid --
//

void fillDiatonicGrid(vector<vector<int> >& diatonic,
      vector<vector<HTp> >& grid) {
   diatonic.resize(grid.size());
   for (int i=0; i<(int)diatonic.size(); i++) {
      diatonic[i].resize(grid[i].size(), 0);
   }
   for (int i=0; i<(int)diatonic.size(); i++) {
      for (int j=0; j<(int)diatonic[i].size(); j++) {
         diatonic[i][j] = getDiatonicPitch(grid[i][j]);
      }
   }
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
      case None:
         break;

      case Raw:
         cout << token;
         break;

      case Kern:
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



