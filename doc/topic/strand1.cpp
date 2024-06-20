#include "humlib.h"
using namespace std;
using namespace hum;
int main(int argc, char** argv) {
   HumdrumFile infile;
   Options options;
   options.process(argc, argv);
   if (options.getArgCount() > 0) {
      infile.read(options.getArg(1));
   } else {
      infile.read(cin);
   }
   cout << "token\trow/col" << endl;
   for (int i=0; i<infile.getStrandCount(); i++) {
      cout << "=== Strand index " << i << endl;
      HTp current   = infile.getStrandStart(i);
      HTp strandEnd = infile.getStrandEnd(i);
      while (current) {
         cout << current << "\t"
              << current->getLineNumber() << ","
              << current->getFieldNumber() << endl;
         if (current == strandEnd) {
            break;
         }
         current = current->getNextToken();
      }
   }
   return 0;
}
