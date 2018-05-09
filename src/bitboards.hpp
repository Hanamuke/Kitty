#ifndef BITBOARDS_DEFINED
#define BITBOARDS_DEFINED
#include "misc.hpp"
#include "types.hpp"
#include <array>

constexpr Bitboard FILE_A_BB = 0x0101010101010101ULL;
constexpr Bitboard FILE_B_BB = FILE_A_BB << 1;
constexpr Bitboard FILE_C_BB = FILE_A_BB << 2;
constexpr Bitboard FILE_D_BB = FILE_A_BB << 3;
constexpr Bitboard FILE_E_BB = FILE_A_BB << 4;
constexpr Bitboard FILE_F_BB = FILE_A_BB << 5;
constexpr Bitboard FILE_G_BB = FILE_A_BB << 6;
constexpr Bitboard FILE_H_BB = FILE_A_BB << 7;

constexpr Bitboard RANK_1_BB = 0x0000000000000ffULL;
constexpr Bitboard RANK_2_BB = RANK_1_BB << 1 * 8;
constexpr Bitboard RANK_3_BB = RANK_1_BB << 2 * 8;
constexpr Bitboard RANK_4_BB = RANK_1_BB << 3 * 8;
constexpr Bitboard RANK_5_BB = RANK_1_BB << 4 * 8;
constexpr Bitboard RANK_6_BB = RANK_1_BB << 5 * 8;
constexpr Bitboard RANK_7_BB = RANK_1_BB << 6 * 8;
constexpr Bitboard RANK_8_BB = RANK_1_BB << 7 * 8;

constexpr Bitboard promotionRankBB = RANK_1_BB | RANK_8_BB;
constexpr Bitboard borderBB = FILE_A_BB | FILE_H_BB | RANK_1_BB | RANK_8_BB;

extern Bitboard squareBB[NB_SQUARE];
extern Bitboard fileBB[NB_FILE];
extern Bitboard rankBB[NB_RANK];
extern Bitboard diagonalBB[NB_DIAGONAL];
extern Bitboard antiDiagonalBB[NB_DIAGONAL];
extern Bitboard adjacentFilesBB[NB_FILE];
extern Bitboard ringBB[NB_SQUARE];
extern Bitboard lineBB[NB_SQUARE][NB_SQUARE];
extern Bitboard betweenBB[NB_SQUARE][NB_SQUARE];
extern Bitboard bishopPseudoAttacks[NB_SQUARE];
extern Bitboard rookPseudoAttacks[NB_SQUARE];
extern Bitboard knightPseudoAttacks[NB_SQUARE];
extern Bitboard pawnPseudoAttack[NB_SQUARE][NB_COLOR];
extern Bitboard bishopMagicMask[NB_SQUARE];
extern Bitboard rookMagicMask[NB_SQUARE];
extern Bitboard forwardBB[NB_SQUARE][NB_COLOR];
extern Bitboard pawnPushBB[NB_SQUARE][NB_COLOR];

extern Bitboard bishopMagics[NB_SQUARE][1 << 9];
extern Bitboard rookMagics[NB_SQUARE][1 << 12];

extern uint64_t bishopMagicNumber[NB_SQUARE];
extern uint64_t rookMagicNumber[NB_SQUARE];

template <PieceType pt> inline uint64_t magic_index(Bitboard input, Square sq) {
  static_assert(pt == BISHOP || pt == ROOK,
                " Attempt to use Magics on wrong piece");
  Bitboard mask = pt == BISHOP ? bishopMagicMask[sq] : rookMagicMask[sq];
  if (use_bmi2)
    return pext(input, mask);
  else
    return ((input & mask) *
            (pt == BISHOP ? bishopMagicNumber[sq] : rookMagicNumber[sq])) >>
           (64 - (pt == BISHOP ? 9 : 12));
}

Bitboard shift(Bitboard bb, Direction d);

template <Direction d> inline Bitboard shift(Bitboard bb) {
  switch (d) {
  case NORTH:
    return bb << NORTH;
  case SOUTH:
    return bb >> NORTH;
  case EAST:
    return (bb & ~FILE_H_BB) << EAST;
  case WEST:
    return (bb & ~FILE_A_BB) >> EAST;
  case SOUTH_EAST:
    return (bb & ~FILE_H_BB) >> NORTH_WEST;
  case NORTH_EAST:
    return (bb & ~FILE_H_BB) << NORTH_EAST;
  case SOUTH_WEST:
    return (bb & ~FILE_A_BB) >> NORTH_EAST;
  case NORTH_WEST:
    return (bb & ~FILE_A_BB) << NORTH_WEST;
  default:;
  }
}

inline Square pop_lsb(Bitboard &b) {
  Square sq = Square(ctz(b));
  b &= b - 1;
  return sq;
}

inline bool more_than_one(Bitboard bb) { return bb & (bb - 1); }

std::string print_bb(Bitboard);
void bitboard_init();

inline Bitboard bb_of(Square sq) { return squareBB[sq]; }
inline Bitboard bb_of(File f) { return fileBB[f]; }
inline Bitboard bb_of(Rank r) { return rankBB[r]; }

inline Bitboard operator~(Square sq) { return ~squareBB[sq]; }
inline Bitboard operator~(File f) { return ~fileBB[f]; }
inline Bitboard operator~(Rank r) { return ~rankBB[r]; }

inline Bitboard operator&(Bitboard b, Square sq) { return b & squareBB[sq]; }
inline Bitboard operator|(Bitboard b, Square sq) { return b | squareBB[sq]; }
inline Bitboard operator^(Bitboard b, Square sq) { return b ^ squareBB[sq]; }
inline void operator&=(Bitboard &b, Square sq) { b &= squareBB[sq]; }
inline void operator|=(Bitboard &b, Square sq) { b |= squareBB[sq]; }
inline void operator^=(Bitboard &b, Square sq) { b ^= squareBB[sq]; }

inline Bitboard operator&(Bitboard b, File f) { return b & fileBB[f]; }
inline Bitboard operator|(Bitboard b, File f) { return b | fileBB[f]; }
inline Bitboard operator^(Bitboard b, File f) { return b ^ fileBB[f]; }
inline void operator&=(Bitboard &b, File f) { b &= fileBB[f]; }
inline void operator|=(Bitboard &b, File f) { b |= fileBB[f]; }
inline void operator^=(Bitboard &b, File f) { b ^= fileBB[f]; }

inline Bitboard operator&(Bitboard b, Rank r) { return b & rankBB[r]; }
inline Bitboard operator|(Bitboard b, Rank r) { return b | rankBB[r]; }
inline Bitboard operator^(Bitboard b, Rank r) { return b ^ rankBB[r]; }
inline void operator&=(Bitboard &b, Rank r) { b &= rankBB[r]; }
inline void operator|=(Bitboard &b, Rank r) { b |= rankBB[r]; }
inline void operator^=(Bitboard &b, Rank r) { b ^= rankBB[r]; }

#endif