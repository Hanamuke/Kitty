#include "misc.hpp"
#include "moves.hpp"
#include "threads.hpp"
#include "types.hpp"
#include <algorithm>
#include <iostream>

void Thread::search() {

  Moves moves(&pos), moves_opponent;
  if (idx != 0)
    return;
  ValueMove m;
  while ((m = moves.next_move())) {
    pos.do_move(m);
    moves_opponent.set_pos(&pos);
    if (pos.in_check() && moves_opponent.size() == 0) {
      std::cout << Sync::lock << "bestmove "
                << (MovePrint)(Move)(moves.size() == 0 ? NULL_MOVE
                                                       : m ? m : *moves.begin())
                << std::endl
                << Sync::unlock;
      return;
    }
    pos.undo_move();
  }

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
