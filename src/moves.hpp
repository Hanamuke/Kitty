#ifndef MOVEMAN_DEFINED
#define MOVEMAN_DEFINED
#include "misc.hpp"
#include "position.hpp"
#include "types.hpp"

class Moves
{
public:
  Moves() { pos = nullptr; };
  Moves(const Position *const p) { set_pos(p); }
  Moves(Moves const &) = delete;
  Moves(Moves &&) = default;
  Moves &operator=(Moves &&) = default;

  void set_pos(const Position *const p)
  {
    pos = p;
    currentMove = moves;
    lastMove = pos->generate_moves(moves);
  }

  Move next_move()
  {
    return (currentMove != lastMove) ? *(currentMove++) : NO_MOVE;
  }
  int size() const { return lastMove - moves; }
  ValueMove *begin() const { return (ValueMove *)moves; }
  ValueMove *end() const { return lastMove; }

private:
  const Position *pos;
  ValueMove moves[MAX_MOVES];
  ValueMove *currentMove, *lastMove;
};

#endif