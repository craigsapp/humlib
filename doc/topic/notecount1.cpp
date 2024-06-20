#include "humlib.h"
using namespace std;
using namespace hum;

int getNoteCount(HumdrumFile& infile) {
   int count = 0;
   for (int i=0; i<infile.getLineCount(); i++) {
      if (!infile[i].hasSpines() || !infile[i].isData()) {   
         continue;
      }
      for (int j=0; j<infile[i].getFieldCount(); j++) {
         HTp token = infile[i].token(j);
         if (!token->isKern() || token->isNull()) {
            continue;
         }
         vector<string> subtokens = token->getSubtokens();
         HumRegex hre;
         for (int k=0; k<(int)subtokens.size(); k++) {
            if (hre.search(subtokens[k], R"([_r\]])")) {
               continue;
            }
            if (hre.search(subtokens[k], "[a-gA-G]")) {
               count++;
            }
         }
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
