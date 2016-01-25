// Description: test HumHash class.

#include <iostream>
#include "HumHash.h"
#include "HumNum.h"

using namespace Humdrum;

int main(int argc, char** argv) {
   HumHash myhash;
   myhash.setValue("LO", "N", "vis", "3");
   cout << "VALUE for vis = " << myhash.getValue("LO", "N", "vis") << endl;
   cout << "VALUE for vis = " << myhash.getValue("LO:N:vis") << endl;
   // myhash.deleteValue("LO:N:vis");
   cout << "VALUE for vis = " << myhash.getValue("LO:N:vis") << endl;
   cout << "INTVALUE for vis = " << myhash.getValueInt("LO:N:vis") << endl;
   cout << "DOES vis exist? " << myhash.isDefined("LO:N:vis") << endl;
   cout << "DOES x exist? " << myhash.isDefined("LO:N:x") << endl;
   cout << "VALUE for x = \"" << myhash.getValue("LO", "N", "x") 
        << "\"" << endl;
   cout << "DOES x exist? " << myhash.isDefined("LO:N:x") << endl;
   vector<string> keys = myhash.getKeys("LO", "N");
   for (int i=0; i<keys.size(); i++) {
      cout << "KEY " << i << ": " << keys[i] << endl;
   }
   // HumNum n("4/5");
   HumNum n(5,4);
   cout << "NEW NUMBER IS " << n << endl;
   myhash.setValue("global", n);
   myhash.setValue("", "global", n);
   myhash.setValue("", "", "global", n);
   cout << "global = " << myhash.getValueFraction("global") << endl;
   cout << ":global = " << myhash.getValueFraction("", "global") << endl;
   cout << "::global = " << myhash.getValueFraction("", "", "global") << endl;
   cout << "int value = " << myhash.getValueInt("", "", "global") << endl;
   cout << "int float = " << myhash.getValueFloat("", "", "global") << endl;

   return 0;
}



