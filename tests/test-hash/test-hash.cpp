// Description: test HumHash class.

#include <iostream>
#include "HumHash.h"

using namespace minHumdrum;

int main(int argc, char** argv) {
   HumHash myhash;
   myhash.setParameter("LO", "N", "vis", "1");
   cout << "VALUE for vis = " << myhash.getParameter("LO", "N", "vis") << endl;
   cout << "VALUE for vis = " << myhash.getParameter("LO:N:vis") << endl;
   myhash.deleteParameter("LO:N:vis");
   cout << "VALUE for vis = " << myhash.getParameter("LO:N:vis") << endl;
   cout << "VALUE for x = \"" << myhash.getParameter("LO", "N", "x") 
        << "\"" << endl;

   return 0;
}



