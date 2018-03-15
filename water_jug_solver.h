#include <utility>
#include <vector>
#include <memory>

typedef std::pair<int, int> Pos;
typedef std::vector<Pos> Path;

class WaterJugSolverCoprime;

class WaterJugSolver {
 public:
   WaterJugSolver(int X, int Y);
   ~WaterJugSolver();
   bool Solve(int volume, Path *seq);
 private:
   const int F_;  // Maximum volume.
   const int r_;  // gcd of two jugs.
   std::unique_ptr<WaterJugSolverCoprime> algo;
};

void Print(const Path &path, int target);
