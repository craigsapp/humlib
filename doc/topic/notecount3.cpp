#include "humlib.h"
using namespace std;
using namespace hum;

int getNoteCount(HumdrumFile& infile) {
   int count = 0;
   vector<HTp> kernStarts = infile.getKernSpineStartList();
   for (int i=0; i<(int)kernStarts.size(); i++) {
      int spine = kernStarts[i]->getSpineIndex();
      for (int j=0; j<infile.getStrandCount(spine); j++) {
         HTp current = infile.getStrandStart(spine, j);
         HTp send = infile.getStrandEnd(spine, j);
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
