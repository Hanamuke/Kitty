#ifndef POSITION_DEFINED
#define POSITION_DEFINED
#include "bitboards.hpp"
#include "misc.hpp"
#include "types.hpp"
#include <cassert>
#include <ostream>
#include <stack>
#include <string>
#include <vector>

struct StateInfo {
  int castlingRights;
  int rule50;

  Square epSquare;
  Piece capturedPiece;
  Bitboard blockers[NB_COLOR];
  Bitboard pinners[NB_COLOR];
  Bitboard checkers;

  Move move;
};

class Position {
public:
  Position();
  Position(Position &&) = default;
  Position &operator=(Position &&) = default;
  Position(Position const &) = delete;
  void set_position(std::string fen);
  void rebase_stack(); // sets current position as root position

  void do_move(Move m);
  void do_move(std::string str);
  void undo_move();
  void do_null_move();
  void undo_null_move();

  template <PieceType pt> Bitboard attacks_from(Square sq) const;
  template <PieceType pt> Bitboard attacks_from(Square sq, Color c) const;
  Bitboard attacks_to(Square sq) const;

  Bitboard pieces() const;
  Bitboard pieces(Piece) const;
  Bitboard pieces(PieceType) const;
  Bitboard pieces(PieceType, PieceType) const;
  Bitboard pieces(PieceType, Color) const;
  Bitboard pieces(Color) const;
  Piece piece(Square) const;
  Square our_king() const;
  Square their_king() const;

  inline Key get_key() const { return key; }

  bool is_pinned(Square) const;
  bool in_check() const;

  ValueMove *generate_moves(ValueMove *moveList) const;

  bool is_valid() const;
  std::string to_fen() const;
  friend std::ostream &operator<<(std::ostream &, Position const &);

  Estimate estimate();

private:
  void reset();
  void next_state();
  void restore_state();
  void put_piece(Square, Piece);
  void remove_piece(Square);
  void move_piece(Square from, Square to);
  void set_cs_right(char token); // used to read FEN
  void compute_pins();

  // Move generation
  template <PieceType, Color>
  ValueMove *generate_moves(ValueMove *moveList, Bitboard target) const;

  template <Color>
  ValueMove *generate_pawn_moves(ValueMove *moveList, Bitboard target) const;

  template <Color>
  ValueMove *generate_king_moves(ValueMove *moveList, Bitboard target) const;

  Bitboard bbByPieceType[NB_PIECE_TYPE];
  Bitboard bbByColor[NB_COLOR];
  Piece pieceBySquare[NB_SQUARE];
  Square pieceList[NB_PIECE][MAX_PIECE_NB];
  size_t index[NB_SQUARE];
  size_t nbPiece[NB_PIECE];

  Key key;
  Color stm;
  int ply;
  StateInfo *state;
  StateInfo stateStack[MAX_PLY];
};

inline Bitboard Position::pieces() const { return bbByPieceType[ALL_PIECE]; }

inline Bitboard Position::pieces(Piece p) const {
  return bbByPieceType[piece_type_of(p)] & bbByColor[color_of(p)];
}

inline Bitboard Position::pieces(PieceType pt) const {
  return bbByPieceType[pt];
}

inline Bitboard Position::pieces(PieceType pt1, PieceType pt2) const {
  return bbByPieceType[pt1] | bbByPieceType[pt2];
}

inline Bitboard Position::pieces(PieceType pt, Color c) const {
  return bbByPieceType[pt] & bbByColor[c];
}

inline Bitboard Position::pieces(Color c) const { return bbByColor[c]; }

inline Piece Position::piece(Square sq) const { return pieceBySquare[sq]; }

inline Square Position::our_king() const {
  return pieceList[make_piece(stm, KING)][0];
}

inline Square Position::their_king() const {
  return pieceList[make_piece(!stm, KING)][0];
}

inline bool Position::is_pinned(Square sq) const {
  return state->blockers[stm] & sq;
}

template <PieceType pt>
inline Bitboard Position::attacks_from(Square sq) const {
  assert(pt != PAWN);
  if (pt == BISHOP)
    return bishopMagics[sq][magic_index<BISHOP>(pieces(), sq)];
  if (pt == ROOK)
    return rookMagics[sq][magic_index<ROOK>(pieces(), sq)];
  if (pt == QUEEN)
    return bishopMagics[sq][magic_index<BISHOP>(pieces(), sq)] |
           rookMagics[sq][magic_index<ROOK>(pieces(), sq)];
  if (pt == KING)
    return ringBB[sq];
  return knightPseudoAttacks[sq];
}
template <>
inline Bitboard Position::attacks_from<PAWN>(Square sq, Color us) const {
  return (pawnPseudoAttack[sq][us] & pieces());
}

inline Bitboard Position::attacks_to(Square sq) const {
  return (attacks_from<BISHOP>(sq) & pieces(BISHOP, QUEEN)) |
         (attacks_from<ROOK>(sq) & pieces(ROOK, QUEEN)) |
         (attacks_from<KNIGHT>(sq) & pieces(KNIGHT)) |
         (attacks_from<PAWN>(sq, stm) & pieces(PAWN, !stm)) |
         (attacks_from<PAWN>(sq, !stm) & pieces(PAWN, stm)) |
         (attacks_from<KING>(sq) & pieces(KING));
}

inline bool Position::in_check() const { return state->checkers; }

inline void Position::rebase_stack() {
  size_t rebaseSize = state->rule50;
  StateInfo *curr = state - rebaseSize;
  // memcpy would be undefined behaviour since source and destination overlap
  for (size_t i = 0; i <= rebaseSize; ++i)
    stateStack[i] = *(curr++);
  state = stateStack + rebaseSize;
}

#endif