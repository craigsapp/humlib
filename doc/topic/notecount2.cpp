#include "humlib.h"
using namespace std;
using namespace hum;

int getNoteCount(HumdrumFile& infile) {
   int count = 0;
   for (int i=0; i<infile.getStrandCount(); i++) {
      HTp current = infile.getStrandStart(i);
      if (!current->isKern()) {
         continue;
      }
      HTp send = infile.getStrandEnd(i);
      while (current && (current != send)) {
         if (!current->isData() || current->isNull()) {
            current = current->getNextToken();
            continue;
         }
         vector<string> subtokens = current->getSubtokens();
         HumRegex hre;
         for (int k=0; k<(int)subtokens.size(); k++) {
            if (hre.search(subtokens[k], R"([_r\]])")) {
               continue;
            }
            if (hre.search(subtokens[k], "[a-gA-G]")) {
               count++;
            }
         }
         current = current->getNextToken();
      }
   }
   return count;
}

int main(int argc, char** argv) {
   Options options;
   options.process(argc, argv);
   HumdrumFileStream instream(options);
   HumdrumFile infile;
   int noteCount = 0;
   while (instream.read(infile)) {
      noteCount += getNoteCount(infile);
   }
   cerr << "NOTES: " << noteCount << endl;
   return 0;
}
