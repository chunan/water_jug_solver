#include <vector>
#include <iostream>

#include "water_jug_solver.h"

using std::vector;
using std::pair;
using std::cout;
using std::endl;

struct State {
  State(int d) : depth(d) {}
  Pos pos;
  Pos prev;
  int depth;  // -1 for uninitialized.
};


// Perform BFS on [x, y] where x \in {0, X} or y \in {0, Y}.  It's impossible
// to have both jugs partially filled while knowing precisely the volume of
// water in them. The total number of state will be 2 * (X + Y).
class WaterJugSolverCoprime {
  public:
    WaterJugSolverCoprime(int X, int Y);

    // Perform BFS until target is found. This will cache the depth of the
    // visited states, hence reduce the time to find a new target.
    bool Solve(int target, Path *path);

  private:
    // Convert from Pos (tuple) to linear index.
    int StateIndex(const Pos &pos) const;

    // Try to initialize the state at `dest_pos` reached from `src`, and append
    // its index to `queue_`. Return true if `dest_pos` match `target`.
    bool Explore(const State &src, const Pos &dest_pos, int target,
                 const State **sol);

    // Find the other configuration by transferring water from one jug to
    // another. Since they cannot be partially filled at the same time, there
    // is only one way to transfer.
    Pos OtherEnd(const State &state) const;

    // Find in `queue_` to see if there is any `state_index` that match the
    // required target.
    bool FindCachedSol(int target, const State **sol) const;

    // data
    const int X_;  // X_ <= Y_
    const int Y_;
    // Next index in queue_ to use as source to explore.
    int queue_start_;
    // All possible states in the lattice (there are 2 * (X + Y) of them).
    // State contains its position in the lattice and its previous state, such
    // that when all elements in states_ are populated, the BFS tree is
    // complete. Nnote that we only find one of the possibly many trees.
    vector<State> states_;
    // The BFS queue_ storing the explored states.
    vector<int> queue_;  // Store the index in states_.
};

WaterJugSolverCoprime::WaterJugSolverCoprime(int X, int Y)
    : X_(std::min<int>(X, Y)), Y_(std::max<int>(X, Y)) {
  int N = 2 * (X + Y);
  queue_start_ = 0;
  states_.resize(N, State(-1));
  queue_.reserve(N);
  // Initialize origin.
  const Pos origin = Pos({0, 0});
  auto origin_state_index = StateIndex(origin);
  State &origin_state = states_[origin_state_index];
  origin_state.pos = origin;
  origin_state.prev = origin;  // Not used.
  origin_state.depth = 0;
  // Push origin to queue_ so it's ready to go.
  queue_.push_back(origin_state_index);
}

bool WaterJugSolverCoprime::Solve(int target, Path *path) {
  const State *sol = nullptr;
  bool solved = FindCachedSol(target, &sol);
  while (!solved && queue_start_ < queue_.size()) {
    int src_index = queue_[queue_start_++];
    const State &src = states_[src_index];
    const int x = src.pos.first;
    const int y = src.pos.second;
    // Do not try to empty jug if it's partially filled. It slows down the
    // process.
    if (x == 0) {  // (0, y) -> (X_, y), fill first jug.
      solved |= Explore(src, {X_, y}, target, &sol);
    } else if (x == X_) {  // (X_, y) -> (0, y), empty first jug.
      solved |= Explore(src, {0, y}, target, &sol);
    }
    if (y == 0) {  // (x, 0) -> (x, Y_), fill the second jug.
      solved |= Explore(src, {x, Y_}, target, &sol);
    } else if (y == Y_) {  // (x, Y_) -> (x, 0), empty the second jug.
      solved |= Explore(src, {x, 0}, target, &sol);
    }
    // Check for transferability.
    if (x + y == 0 || x + y == X_ + Y_) continue;
    Pos dest = OtherEnd(src);
    solved |= Explore(src, dest, target, &sol);
  }
  if (!solved) {
    return false;
  }
  // We have found the solution, leave `queue_` as is (can be resumed in later
  // Solve call). Backtrack the path to the origin.
  path->resize(sol->depth + 1);
  path->back() = sol->pos;
  for (int d = sol->depth - 1; d >= 0; --d) {
    int next_state_index = StateIndex((*path)[d + 1]);
    (*path)[d] = states_[next_state_index].prev;
  }
  return true;
}

int WaterJugSolverCoprime::StateIndex(const Pos &pos) const {
  // The states are stored in the following order:
  // (0, 0), (0, 1), ..., (0, Y),      // # = Y + 1
  // (X, 0), (X, 1), ..., (X, Y),      // # = Y + 1
  // (1, 0), (2, 0), ..., (X - 1, 0),  // # = X - 1
  // (1, Y), (2, Y), ..., (X - 1, Y).  // # = X - 1
  //
  int i = pos.first;
  int j = pos.second;
  if (i == 0) {  // Total Y + 1
    return j;
  } else if (i == X_) {  // Total Y + 1
    return Y_ + 1 + j;
  } else if (j == 0) {  // Total X - 1
    return 2 * Y_ + i + 1;  // 2 * (Y + 1) + i - 1
  } else {  // Total X - 1
    return 2 * Y_ + X_ + i;  // 2 * (Y + 1) + (X - 1) + i - 1
  }
}

bool WaterJugSolverCoprime::Explore(const State &src, const Pos &dest_pos,
                                    int target, const State **sol) {
  int dest_index = StateIndex(dest_pos);
  if (states_[dest_index].depth != -1) return false;
  queue_.push_back(dest_index);
  State &dest = states_[dest_index];
  dest.pos = dest_pos;
  dest.prev = src.pos;
  dest.depth = src.depth + 1;
  if (dest_pos.first + dest_pos.second == target) {
    *sol = &dest;
    return true;
  }
  return false;
}

Pos WaterJugSolverCoprime::OtherEnd(const State &state) const {
  const int x = state.pos.first;
  const int y = state.pos.second;
  if (x + y == 0 || x + y == X_ + Y_) {
    return Pos({-1, -1});
  }

  if (x + y <= X_) {
    if (x == 0) {
      return Pos({y, 0});  // J2 -> J1.
    } else {  // y == 0
      return Pos({0, x});  // J1 -> J2.
    }
  } else {  // x + y > X_
    if (x == 0 || y == Y_) {
      return Pos({X_, x + y - X_});  // Fill J1.
    } else if (x + y <= Y_) {  // In fact, x == X_.
      return Pos({0, x + y});  // Empty J1.
    } else {
      return Pos(x + y - Y_, Y_);  // Fill J2.
    }
  }
}

bool WaterJugSolverCoprime::FindCachedSol(int target, const State **sol) const {
  for (const auto index : queue_) {
    const State &state = states_[index];
    if (state.pos.first + state.pos.second != target) continue;
    *sol = &state;
    return true;
  }
  return false;
}

int gcd(int a, int b) {
  if (a <= 0 || b <= 0) return 1;
  while (b > 0) {
    a = a % b;  // => a < b, a can be 0
    std::swap(a, b);  // a > b, b can be 0, i.e., a = gcd.
  }
  return a;
}

void Print(const Path &path, int target) {
  cout << "---------------- " << target
       << " (" << path.size() <<")" << " ----------------\n";
  for (const auto &t : path) {
    cout << "(" << t.first << ", " << t.second << ")\n";
  }
}

WaterJugSolver::WaterJugSolver(int X, int Y)
    : F_(X + Y), r_(gcd(X, Y)),
      algo(new WaterJugSolverCoprime(X / r_, Y / r_)) {}

WaterJugSolver::~WaterJugSolver() {}

bool WaterJugSolver::Solve(int target, Path *path) {
  if (target < 0 || target > F_) return false;
  if (r_ > 1 && target % r_ != 0) return false;
  if (!algo->Solve(target / r_, path)) return false;
  if (r_ != 1) {
    for (Pos &loc : *path) {
      loc.first *= r_;
      loc.second *= r_;
    }
  }
  return true;
}
