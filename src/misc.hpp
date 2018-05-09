#ifndef MISC_DEFINED
#define MISC_DEFINED
#include "types.hpp"
#include <fstream>
#include <mutex>
#include <random>
#include <x86intrin.h>

#define popcount(x) __builtin_popcountll(x)
#define pext(x, y) _pext_u64(x, y)
#define ctz(x) __builtin_ctzll(x)

constexpr char startfen[] =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// Synchronises outputs
// EX : std::cout << Sync::lock << "message" << Sync::unlock;

enum class Sync { lock, unlock };
inline std::ostream &operator<<(std::ostream &os, Sync s) {
  static std::mutex m;
  if (s == Sync::lock)
    m.lock();
  else
    m.unlock();
  return os;
}

extern std::random_device rd;
struct Gen {
  uint64_t operator()() {
    uint64_t x = state; /* The state must be seeded with a nonzero value. */
    x ^= x >> 12;       // a
    x ^= x << 25;       // b
    x ^= x >> 27;       // c
    state = x;
    return x * 0x2545F4914F6CDD1DULL;
  }
  constexpr static uint64_t min() { return 0ULL; }
  constexpr static uint64_t max() { return 0xffffffffffffffffULL; }
  using result_type = uint64_t;
  uint64_t state = rd();
};

extern Gen gen;
inline uint64_t sparse_rand64() { return gen() & gen() & gen(); }

extern Key zobrist_stm;
extern Key zobrist_sqp[NB_SQUARE][NB_PIECE];
extern Key zobrist_ep[1 + NB_SQUARE];
extern Key zobrist_castle[1 << NB_CASTLING];

void zobrist_init();

namespace Perft {
void perft();
}

#endif
