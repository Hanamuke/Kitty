#include "position.hpp"
#include "misc.hpp"
#include <cassert>
#include <cstring>
#include <sstream>

Position::Position()
{
  state = stateStack;
  set_position(startfen);
}

std::string Position::to_fen() const
{
  std::string ret;
  int empty_cnt;
  for (Rank r = RANK_8; r >= RANK_1; --r)
  {
    empty_cnt = 0;
    for (File f = FILE_A; f <= FILE_H; ++f)
    {
      Square sq = make_square(f, r);
      Piece p = piece(sq);
      if (p)
      {
        if (empty_cnt)
        {
          ret += std::to_string(empty_cnt);
          empty_cnt = 0;
        }
        ret += pieceStr[p];
      }
      else
        ++empty_cnt;
    }
    if (empty_cnt)
      ret += std::to_string(empty_cnt);
    if (r != RANK_1)
      ret += '/';
  }
  ret += ' ';
  ret += stm ? 'b' : 'w';
  ret += ' ';
  int cstl = state->castlingRights;
  if (!cstl)
    ret += '-';
  if (cstl & W_OO)
    ret += 'K';
  if (cstl & W_OOO)
    ret += 'Q';
  if (cstl & B_OO)
    ret += 'k';
  if (cstl & B_OOO)
    ret += 'q';
  ret += ' ';
  if (state->epSquare != NO_SQUARE)
  {
    ret += (char)(file_of(state->epSquare) + 'a');
    ret += (char)(rank_of(state->epSquare) + '1');
  }
  else
    ret += '-';
  ret += ' ';
  ret += std::to_string(state->rule50);
  ret += ' ';
  ret += std::to_string(1 + ply / 2);
  return ret;
}
// assumes that the fen is somewhat correct
void Position::set_position(std::string fen)
{
  reset();
  fen += " 0 1"; // default move numbers in case it is missing
  std::istringstream is(fen);
  char token;
  Square sq = SQ_A8;
  while (sq >= SQ_A1)
  {
    is >> std::noskipws >> token;
    if (token == '/' || token == ' ')
      sq += 2 * SOUTH;
    else
    {
      size_t pos(pieceStr.find(token));
      if (pos != std::string::npos)
        put_piece(sq, Piece(pos));
      else
        sq += token - '1';
      sq += EAST;
    }
  }
  is >> token;
  stm = token == 'w' ? WHITE : BLACK;
  if (stm)
    key ^= zobrist_stm;
  std::string str;
  is >> std::skipws >> str;
  key ^= zobrist_castle[0];
  if (str != "-")
    for (char token : str)
      set_cs_right(token);
  is >> str;
  if (str == "-")
  {
    state->epSquare = NO_SQUARE;
    key ^= zobrist_ep[NO_SQUARE];
  }
  else
  {
    Square sq = make_square(str);
    if (pawnPseudoAttack[sq][!stm] & pieces(PAWN, stm))
    {
      state->epSquare = sq;
      key ^= zobrist_ep[sq];
    }
  }
  is >> state->rule50;
  is >> ply;
  ply = 2 * (ply - 1) + (stm == WHITE ? 0 : 1);
  state->checkers = attacks_to(our_king()) & pieces(!stm);
  compute_pins();
  assert(is_valid());
}

void Position::do_move(std::string input)
{
  if (state - stateStack >= MAX_PLY - 1)
    rebase_stack();
  Move m;
  Square from = make_square(input);
  input.erase(0, 2);
  Square to = make_square(input);
  input.erase(0, 2);
  if (!input.empty())
    m = make_move<PROMOTION>(from, to, PieceType(pieceTypeStr.find(input[0])));
  else if (piece_type_of(piece(from)) == PAWN && to == state->epSquare)
    m = make_move<EN_PASSANT>(from, to);
  else if (piece_type_of(piece(from)) == KING)
  {
    if (distance(file_of(from), file_of(to)) > 1)
      m = cstl_move[(stm << 1) | (file_of(from) > file_of(to))];
    else
      m = make_move<NORMAL>(from, to);
  }
  else
  {
    m = make_move<NORMAL>(from, to);
  }
  do_move(m);
}

void Position::do_move(Move m)
{
  Square from = get_from(m), to = get_to(m);
  MoveType mt = get_move_type(m);
  do_null_move();
  // after that, stm is the side of the opponent relative to the move being made
  state->capturedPiece = piece(to);
  state->move = m;
  key ^= zobrist_castle[state->castlingRights];
  state->castlingRights &= ~castlingBySquare[from];
  if (piece(to))
  {
    state->rule50 = 0;
    state->castlingRights &= ~castlingBySquare[to];
    remove_piece(to);
  }
  key ^= zobrist_castle[state->castlingRights];
  if (mt == EN_PASSANT)
    remove_piece(to + up(stm));
  else if (mt == CASTLING)
    move_piece(castlingRookMove[get_castling(m)][0],
               castlingRookMove[get_castling(m)][1]);
  else if (mt == PROMOTION)
  {
    remove_piece(from);
    put_piece(to, make_piece(!stm, get_prom(m)));
    state->checkers = attacks_to(our_king()) & pieces(!stm);
    compute_pins();
    return;
  }
  move_piece(from, to);
  if (piece_type_of(piece(to)) == PAWN)
  {
    state->rule50 = 0;
    if (distance(rank_of(from), rank_of(to)) >> 1 &&
        pawnPseudoAttack[from + down(stm)][!stm] & pieces(PAWN, stm))
    {
      key ^= zobrist_ep[state->epSquare];
      state->epSquare = from + down(stm);
      key ^= zobrist_ep[from + down(stm)];
    }
  }
  state->checkers = attacks_to(our_king()) & pieces(!stm);
  compute_pins();
  assert(is_valid());
}

void Position::undo_move()
{
  Move m = state->move;
  Square from = get_from(m), to = get_to(m);
  assert(pieces() & to);
  MoveType mt = get_move_type(m);
  if (mt == PROMOTION)
  {
    remove_piece(to);
    put_piece(from, make_piece(!stm, PAWN));
  }
  else
    move_piece(to, from);
  if (state->capturedPiece)
    put_piece(to, state->capturedPiece);
  else if (mt == EN_PASSANT)
    put_piece(to + up(stm), make_piece(stm, PAWN));
  else if (mt == CASTLING)
    move_piece(castlingRookMove[get_castling(m)][1],
               castlingRookMove[get_castling(m)][0]);
  undo_null_move();
  assert(is_valid());
}

inline void Position::do_null_move()
{
  key ^= zobrist_ep[state->epSquare];
  next_state();
  ++ply;
  stm ^= 1;
  key ^= zobrist_stm;
  key ^= zobrist_ep[state->epSquare];
}

inline void Position::undo_null_move()
{
  key ^= zobrist_ep[state->epSquare];
  restore_state();
  --ply;
  stm ^= 1;
  key ^= zobrist_stm;
  key ^= zobrist_ep[state->epSquare];
}

std::ostream &operator<<(std::ostream &os, Position const &pos)
{
  os << pos.to_fen() << '\n';
  os << "+---+---+---+---+---+---+---+---+\n";
  for (Rank r = RANK_8; r >= RANK_1; --r)
  {
    for (File f = FILE_A; f <= FILE_H; ++f)
      os << "| " << pos.piece(make_square(f, r)) << " ";
    os << "|\n+---+---+---+---+---+---+---+---+\n";
  }
  os << (pos.stm == WHITE ? "White" : "Black") << " to move.\n";
  os << pos.ply << " plies into the game.\n";
  os << 100 - pos.state->rule50
     << " plies before the 50 moves rule triggers.\n";
  if (pos.state->epSquare != NO_SQUARE)
    os << "En passant square : " << pos.state->epSquare << '\n';
  return os << "Castling right : " << pos.state->castlingRights;
}

void Position::reset()
{
  std::memset(bbByPieceType, 0, NB_PIECE_TYPE * sizeof(Bitboard));
  std::memset(bbByColor, 0, NB_COLOR * sizeof(Bitboard));
  std::memset(pieceBySquare, NO_PIECE, NB_SQUARE * sizeof(Piece));
  std::memset(nbPiece, 0, NB_PIECE * sizeof(size_t));
  key = 0;
  state = stateStack;
  state->castlingRights = 0;
  state->epSquare = NO_SQUARE;
  state->capturedPiece = NO_PIECE;
  state->move = NO_MOVE;
}

void Position::next_state()
{
  assert(state + 1 < (StateInfo *)stateStack + MAX_PLY);
  memcpy(state + 1, state, offsetof(StateInfo, epSquare));
  (++state)->epSquare = NO_SQUARE;
  ++(state->rule50);
}

inline void Position::restore_state()
{
  assert(state > stateStack);
  key ^= zobrist_castle[state->castlingRights];
  --state;
  key ^= zobrist_castle[state->castlingRights];
}

void Position::put_piece(Square sq, Piece p)
{
  assert(!piece(sq));
  Bitboard bbSq = bb_of(sq);
  bbByPieceType[piece_type_of(p)] ^= bbSq;
  bbByPieceType[ALL_PIECE] ^= bbSq;
  bbByColor[color_of(p)] ^= bbSq;
  key ^= zobrist_sqp[sq][p];
  pieceBySquare[sq] = p;
  index[sq] = nbPiece[p];
  pieceList[p][nbPiece[p]++] = sq;
}

void Position::remove_piece(Square sq)
{
  assert(piece(sq));
  assert(piece_type_of(piece(sq)) != KING);
  Piece p = pieceBySquare[sq];
  Bitboard bbSq = bb_of(sq);
  bbByPieceType[piece_type_of(p)] ^= bbSq;
  bbByPieceType[ALL_PIECE] ^= bbSq;
  bbByColor[color_of(p)] ^= bbSq;
  key ^= zobrist_sqp[sq][p];
  pieceBySquare[sq] = NO_PIECE;
  // otherSquare might be equal to sq, in particular if nbPiece[p]==1 , in which
  // case those three instructions are useless. If statement to filter it out
  // might be faster ?
  // TODO
  Square otherSquare = pieceList[p][--nbPiece[p]];
  index[otherSquare] = index[sq];
  pieceList[p][index[sq]] = otherSquare;
}
void Position::move_piece(Square from, Square to)
{
  Piece p = pieceBySquare[from];
  Bitboard move_bb = bb_of(from) | bb_of(to);
  bbByPieceType[piece_type_of(p)] ^= move_bb;
  bbByPieceType[ALL_PIECE] ^= move_bb;
  bbByColor[color_of(p)] ^= move_bb;
  key ^= zobrist_sqp[from][p];
  key ^= zobrist_sqp[to][p];
  pieceBySquare[from] = NO_PIECE;
  pieceBySquare[to] = p;
  index[to] = index[from];
  pieceList[p][index[from]] = to;
}

void Position::set_cs_right(char token)
{
  key ^= zobrist_castle[state->castlingRights];
  switch (token)
  {
  case 'K':
    state->castlingRights |= W_OO;
    break;
  case 'Q':
    state->castlingRights |= W_OOO;
    break;
  case 'k':
    state->castlingRights |= B_OO;
    break;
  case 'q':
    state->castlingRights |= B_OOO;
    break;
  default:
    assert(false);
  }
  key ^= zobrist_castle[state->castlingRights];
}

void Position::compute_pins()
{
  for (Color c : {WHITE, BLACK})
  {
    state->pinners[c] = 0;
    state->blockers[c] = 0;
    Square ksq = pieceList[make_piece(c, KING)][0];
    Bitboard snipers = ((bishopPseudoAttacks[ksq] & pieces(BISHOP, QUEEN)) |
                        (rookPseudoAttacks[ksq] & pieces(ROOK, QUEEN))) &
                       pieces(!c) & ~state->checkers;
    while (snipers)
    {
      Square ssq = pop_lsb(snipers);
      Bitboard blockers = pieces() & betweenBB[ksq][ssq];
      if (!more_than_one(blockers))
      {
        state->pinners[c] |= ssq;
        state->blockers[c] |= pop_lsb(blockers);
      }
    }
  }
}

bool Position::is_valid() const
{
  assert(nbPiece[make_piece(WHITE, KING)] == 1);
  assert(nbPiece[make_piece(BLACK, KING)] == 1);
  assert(state->rule50 <= 100);
  assert(!(pieces(WHITE) & pieces(BLACK)));
  assert(pieces() == (pieces(WHITE) | pieces(BLACK)));
  for (Square sq = SQ_A1; sq <= SQ_H8; ++sq)
  {
    assert((~pieces() & sq) || piece(sq));
    if (piece(sq) != NO_PIECE)
      assert(sq == pieceList[pieceBySquare[sq]][index[sq]]);
  }
  for (Piece p = W_PAWN; p < NB_PIECE; ++p)
    if (piece_type_of(p) != ALL_PIECE)
      assert(nbPiece[p] ==
             (size_t)popcount(pieces(piece_type_of(p)) & pieces(color_of(p))));
  assert(!(state->castlingRights & W_OO) ||
         (piece(SQ_H1) == W_ROOK && piece(SQ_E1) == W_KING));
  assert(!(state->castlingRights & W_OOO) ||
         (piece(SQ_A1) == W_ROOK && piece(SQ_E1) == W_KING));
  assert(!(state->castlingRights & B_OO) ||
         (piece(SQ_H8) == B_ROOK && piece(SQ_E8) == B_KING));
  assert(!(state->castlingRights & B_OOO) ||
         (piece(SQ_A8) == B_ROOK && piece(SQ_E8) == B_KING));
  [[maybe_unused]] Square ep = state->epSquare;
  assert((rank_of(ep) == RANK_3 && stm == BLACK) ||
         (rank_of(ep) == RANK_6 && stm == WHITE) || ep == NO_SQUARE);
  assert(rank_of(ep) != RANK_3 ||
         (piece(ep + NORTH) == W_PAWN &&
          pieces(PAWN, BLACK) & pawnPseudoAttack[ep][WHITE]));
  assert(rank_of(ep) != RANK_6 ||
         (piece(ep + SOUTH) == B_PAWN &&
          pieces(PAWN, WHITE) & pawnPseudoAttack[ep][BLACK]));
  return true;
}
