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
   for (int i=0; i<infile.getLineCount(); i++) {
      for (int j=0; j<infile[i].getFieldCount(); j++) {
         cout << infile.token(i, j);
         if (j < infile[i].getFieldCount()) {
            cout << '\t';
         }
      }
      cout << '\n';
   }
   return 0;
}
