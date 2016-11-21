// Description: Print a Humdrum file in CSV format.

#include "humlib.h"

using namespace hum;

int main(int argc, char** argv) {
   int dpc = 0;    // diatonic pitch class 0=C, 1=D, ..., 6=B
   int acc = 0;    // accidental alteration -2=double flat, 0=natural, 1=sharp
   int oct = 0;    // 4 = middle-C octave
   int maxacc = 0; // non-zero integer
   int wbh = 0;    // integer encoded pitch
   bool tochromatic;

   if (argc < 2) {
      // too few parameters
      return 1;
   } else if (argc < 4) {
      // convert wbh to chromatic pitch
      wbh    = atoi(argv[1]); // assuming octave will not be negative
      maxacc = atoi(argv[2]);
      tochromatic = true;
   } else if (argc > 4) {
      // convert chromatic pitch to wbh
      dpc    = atoi(argv[1]) % 7; // assuming non-negative value
      acc    = atoi(argv[2]);
      oct    = atoi(argv[3]);
      maxacc = atoi(argv[4]);
      tochromatic = false;
   } else {
      return 1;
   }

   if (tochromatic) {
      Convert::wbhToPitch(dpc, acc, oct, maxacc, wbh);
      cout << "dpc = " << dpc << endl;
      cout << "acc = " << acc << endl;
      cout << "oct = " << oct << endl;
      cout << "wbh = " << wbh << endl;
      cout << "name = " << 'A' + dpc;
      if (acc > 0) {
         for (int i=0; i<acc; i++) {
            cout << "#";
         }
      } else if (acc < 0) {
         for (int i=0; i<-acc; i++) {
            cout << "b";
         }
      }
      cout << oct;
      cout << endl;
   } else {
      wbh  = Convert::pitchToWbh(dpc, acc, oct, maxacc);
      cout << "wbh = " << wbh << endl;
   }

   return 0;
}



