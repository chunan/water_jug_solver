#include <cstdlib>
#include <iostream>

#include "water_jug_solver.h"

using std::cout;
using std::endl;

int main(int argc, char **argv) {
  if (argc < 4) {
    cout << argv[0] << " <Volume1> <Volume2> "
         << "<required volume1> <required volume2> ..." << endl;
    return -1;
  } else {
    const int X = atoi(argv[1]);
    const int Y = atoi(argv[2]);
    WaterJugSolver solver(X, Y);
    Path path;
    for (int i = 3; i < argc; ++i) {
      int target = atoi(argv[i]);
      if (solver.Solve(target, &path)) {
        Print(path, target);
      } else {
        cout << "Cannot get volume " << target
             << " from jug of volume " << X
             << " and " << Y << "." << endl;
      }
    }
  }
  return 0;
}
