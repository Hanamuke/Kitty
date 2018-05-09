#include "misc.hpp"
#include "moves.hpp"
#include "threads.hpp"
#include "types.hpp"
#include <iostream>

void Thread::search() {

  Moves moves(&pos), moves_opponent;
  if (idx != 0)
    return;
  std::shuffle(moves.begin(), moves.end(), gen);
  std::cout << Sync::lock << "bestmove " << (MovePrint)(Move)(*moves.begin())
            << std::endl
            << Sync::unlock;
  std::cout << Sync::lock
            << "info depth 0 nodes 1 score cp 100 nps 0 tbhits 0 pv "
            << (MovePrint)(Move)(*moves.begin()) << std::endl
            << Sync::unlock;
  return;
}
