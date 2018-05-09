#ifndef TYPES_INCLUDED
#define TYPES_INCLUDED
#include <cstdint>
#include <ostream>
#include <string>

#ifdef __BMI2__
constexpr bool use_bmi2 = true;
#else
constexpr bool use_bmi2 = false;
#endif

using Key = uint64_t;

enum Color : int { WHITE, BLACK, NB_COLOR };
constexpr Color operator!(Color c) { return Color((int)c ^ 1); }
constexpr void operator^=(Color &c, int i __attribute__((unused))) {
  c = Color((int)c ^ 1);
}

enum PieceType {
  NO_PIECE_TYPE,
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  KING,
  ALL_PIECE,
  NB_PIECE_TYPE
};
const std::string pieceTypeStr = " pnbrqk.";

enum Piece {
  NO_PIECE,
  W_PAWN,
  W_KNIGHT,
  W_BISHOP,
  W_ROOK,
  W_QUEEN,
  W_KING,
  B_PAWN = 0b1001,
  B_KNIGHT,
  B_BISHOP,
  B_ROOK,
  B_QUEEN,
  B_KING,
  NB_PIECE = 15,
  MAX_PIECE_NB = 10
};
const std::string pieceStr = " PNBRQK  pnbrqk";

constexpr Color color_of(Piece p) { return Color((p & 0b1000) >> 3); }
constexpr PieceType piece_type_of(Piece p) { return PieceType(p & 0b0111); };
constexpr Piece make_piece(Color c, PieceType pt) { return Piece(c << 3 | pt); }

using Bitboard = uint64_t;

enum File {
  FILE_A,
  FILE_B,
  FILE_C,
  FILE_D,
  FILE_E,
  FILE_F,
  FILE_G,
  FILE_H,
  NB_FILE
};
enum Rank {
  RANK_1,
  RANK_2,
  RANK_3,
  RANK_4,
  RANK_5,
  RANK_6,
  RANK_7,
  RANK_8,
  NB_RANK
};

enum Diagonal {
  DIAG_1,
  DIAG_2,
  DIAG_3,
  DIAG_4,
  DIAG_5,
  DIAG_6,
  DIAG_7,
  DIAG_8,
  DIAG_B,
  DIAG_C,
  DIAG_D,
  DIAG_E,
  DIAG_F,
  DIAG_G,
  DIAG_H,
  NB_DIAGONAL = 15
};

enum Square {
  SQ_A1,
  SQ_B1,
  SQ_C1,
  SQ_D1,
  SQ_E1,
  SQ_F1,
  SQ_G1,
  SQ_H1,
  SQ_A2,
  SQ_B2,
  SQ_C2,
  SQ_D2,
  SQ_E2,
  SQ_F2,
  SQ_G2,
  SQ_H2,
  SQ_A3,
  SQ_B3,
  SQ_C3,
  SQ_D3,
  SQ_E3,
  SQ_F3,
  SQ_G3,
  SQ_H3,
  SQ_A4,
  SQ_B4,
  SQ_C4,
  SQ_D4,
  SQ_E4,
  SQ_F4,
  SQ_G4,
  SQ_H4,
  SQ_A5,
  SQ_B5,
  SQ_C5,
  SQ_D5,
  SQ_E5,
  SQ_F5,
  SQ_G5,
  SQ_H5,
  SQ_A6,
  SQ_B6,
  SQ_C6,
  SQ_D6,
  SQ_E6,
  SQ_F6,
  SQ_G6,
  SQ_H6,
  SQ_A7,
  SQ_B7,
  SQ_C7,
  SQ_D7,
  SQ_E7,
  SQ_F7,
  SQ_G7,
  SQ_H7,
  SQ_A8,
  SQ_B8,
  SQ_C8,
  SQ_D8,
  SQ_E8,
  SQ_F8,
  SQ_G8,
  SQ_H8,
  NO_SQUARE,
  NB_SQUARE = 64,
};

enum Direction {
  NORTH = 8,
  EAST = 1,
  WEST = -1,
  SOUTH = -8,
  NORTH_EAST = NORTH + EAST,
  NORTH_WEST = NORTH + WEST,
  SOUTH_EAST = SOUTH + EAST,
  SOUTH_WEST = SOUTH + WEST
};

constexpr Direction up(Color stm) { return Direction((1 - 2 * stm) * NORTH); }
constexpr Direction down(Color stm) { return Direction((2 * stm - 1) * NORTH); }
constexpr File file_of(Square sq) { return File(sq & 7); }
constexpr Rank rank_of(Square sq) { return Rank(sq >> 3); }
constexpr Square make_square(File f, Rank r) { return Square(f + (r << 3)); }
inline Square make_square(std::string const &str) {
  return make_square(File(str[0] - 'a'), Rank(str[1] - '1'));
}

enum Castling : unsigned {
  NO_CASTLING,
  W_OO = 0b0001,
  W_OOO = 0b0010,
  B_OO = 0b0100,
  B_OOO = 0b1000,
  ALL_CASTLING = 0b1111,
  NB_CASTLING = 4
};

constexpr Square castlingRookMove[NB_CASTLING][2] = {
    {SQ_H1, SQ_F1}, {SQ_A1, SQ_D1}, {SQ_H8, SQ_F8}, {SQ_A8, SQ_D8}};

constexpr unsigned castlingBySquare[NB_SQUARE] = {
    W_OOO,        NO_CASTLING, NO_CASTLING, NO_CASTLING, W_OO | W_OOO,
    NO_CASTLING,  NO_CASTLING, W_OO,        NO_CASTLING, NO_CASTLING,
    NO_CASTLING,  NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING,
    NO_CASTLING,  NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING,
    NO_CASTLING,  NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING,
    NO_CASTLING,  NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING,
    NO_CASTLING,  NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING,
    NO_CASTLING,  NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING,
    NO_CASTLING,  NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING,
    NO_CASTLING,  NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING,
    NO_CASTLING,  NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING,
    NO_CASTLING,  B_OOO,       NO_CASTLING, NO_CASTLING, NO_CASTLING,
    B_OO | B_OOO, NO_CASTLING, NO_CASTLING, B_OO};
/*
    0-5 : from sq
    6-11 : to sq
    12-13 : prom/castling
    14-15 : movetype
    */
using Move = uint16_t;
enum SpecialMove : Move { NULL_MOVE, NO_MOVE = NULL_MOVE };

// A structure made up of a value and a precision
using Value = int16_t;
enum Precision : uint16_t {
  MIN_PRECISION,
  MAX_PRECISION = (uint16_t)-1,
};

struct Estimate {
  Value v;
  Precision p;
};

enum Phase : uint16_t { MID_GAME = 18000, END_GAME = 8000 };

struct Score {
  Value mg;
  Value eg;
  constexpr Value value(Value material) {
    float phase = std::min(
        0.f,
        std::max(1.f, (float)(material - MID_GAME) / (END_GAME - MID_GAME)));
    return phase * eg + (1 - phase) * mg;
  }
};

struct ValueMove {
  ValueMove(Move move) : m(move), v(0) {}
  ValueMove() : m(NO_MOVE), v(0) {}
  operator Move &() { return m; }
  Move m;
  uint16_t v;
};

enum MoveType {
  NORMAL,
  EN_PASSANT = 1 << 14,
  PROMOTION = 1 << 15,
  CASTLING = 3 << 14,
};

template <MoveType mt>
constexpr Move make_move(Square from, Square to, PieceType pieceProm = KNIGHT) {
  return from | to << 6 | (pieceProm - KNIGHT) << 12 | mt;
}
constexpr Move cstl_move[NB_CASTLING] = {
    SQ_E1 | SQ_G1 << 6 | CASTLING, SQ_E1 | SQ_C1 << 6 | 1 << 12 | CASTLING,
    SQ_E8 | SQ_G8 << 6 | 2 << 12 | CASTLING,
    SQ_E8 | SQ_C8 << 6 | 3 << 12 | CASTLING};

constexpr Square get_from(Move m) { return Square(m & 0x003f); }
constexpr Square get_to(Move m) { return Square((m & 0x0fc0) >> 6); }
constexpr PieceType get_prom(Move m) {
  return PieceType(((m & 0x3000) >> 12) + KNIGHT);
}
constexpr int get_castling(Move m) { return (m & 0x3000) >> 12; }
constexpr int get_pawn_move(Move m) { return m & 0x3000; }
constexpr MoveType get_move_type(Move m) { return MoveType(m & 0xc000); }

#define OP_ON(T)                                                               \
  inline T operator++(T &t) { return t = T(t + 1); }                           \
  inline T operator--(T &t) { return t = T(t - 1); }                           \
  inline T operator+=(T &t1, T t2) { return t1 = T(t1 + t2); }                 \
  inline T operator+=(T &t1, int t2) { return t1 = T(t1 + t2); }
OP_ON(Square)
OP_ON(File)
OP_ON(Rank)
OP_ON(Piece)
OP_ON(Diagonal)
#undef OP_ON
inline Square operator+=(Square &sq, Direction d) {
  return sq = Square(sq + d);
}
inline Square operator+(Square const &sq, Direction d) {
  return Square(sq + (int)d);
}

inline Score operator*(int i, Score const &a) {
  return {Value(i * a.mg), Value(i * a.eg)};
}

inline void operator+=(Score &s, Score const &a) {
  s.mg += a.mg;
  s.eg += a.eg;
}

template <typename T> constexpr int distance(T t1, T t2) {
  return (t1 >= t2) ? t1 - t2 : t2 - t1;
}
template <> constexpr int distance(Square sq1, Square sq2) {
  return sq1 >= sq2 ? (sq1 - sq2) % NB_RANK + (sq1 - sq2) / NB_FILE
                    : (sq2 - sq1) % NB_RANK + (sq2 - sq1) / NB_FILE;
}

inline std::ostream &operator<<(std::ostream &os, Color c) {
  return os << (c ? 'b' : 'w');
}

inline std::ostream &operator<<(std::ostream &os, File f) {
  return os << static_cast<char>('a' + f);
}
inline std::ostream &operator<<(std::ostream &os, Rank r) {
  return os << static_cast<char>('1' + r);
}
inline std::ostream &operator<<(std::ostream &os, Square sq) {
  return os << file_of(sq) << rank_of(sq);
}
inline std::ostream &operator<<(std::ostream &os, Piece p) {
  return os << pieceStr[p];
}
inline std::ostream &operator<<(std::ostream &os, PieceType pt) {
  return os << pieceTypeStr[pt];
}

enum MovePrint {};
inline std::ostream &operator<<(std::ostream &os, MovePrint m) {
  if (m == MovePrint(NULL_MOVE))
    return os << "0000";
  MoveType mt = get_move_type(m);
  os << get_from(m) << get_to(m);
  return mt == PROMOTION ? os << get_prom(m) : os;
}

enum Depth : unsigned { MAX_PLY = 256 };

// Maximum number of legal moves in any position
constexpr unsigned MAX_MOVES = 200;

enum GenType { NON_EVASION, CAPTURES };

#endif
