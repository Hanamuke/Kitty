#include "bitboards.hpp"
#include "misc.hpp"
#include <array>
#include <cassert>
#include <cstring>

Bitboard squareBB[NB_SQUARE];
Bitboard fileBB[NB_FILE];
Bitboard rankBB[NB_RANK];
Bitboard diagonalBB[NB_DIAGONAL];
Bitboard antiDiagonalBB[NB_DIAGONAL];
Bitboard adjacentFilesBB[NB_FILE];
Bitboard adjacentRanksBB[NB_FILE];
Bitboard ringBB[NB_SQUARE];
Bitboard lineBB[NB_SQUARE][NB_SQUARE];
Bitboard betweenBB[NB_SQUARE][NB_SQUARE];

Bitboard knightPseudoAttacks[NB_SQUARE];
Bitboard bishopPseudoAttacks[NB_SQUARE];
Bitboard rookPseudoAttacks[NB_SQUARE];
Bitboard queenPseudoAttacks[NB_SQUARE];
Bitboard bishopMagicMask[NB_SQUARE];
Bitboard rookMagicMask[NB_SQUARE];
Bitboard bishopMagics[NB_SQUARE][1 << 9];
Bitboard rookMagics[NB_SQUARE][1 << 12];
Bitboard pawnPseudoAttack[NB_SQUARE][NB_COLOR];
Bitboard forwardBB[NB_SQUARE][NB_COLOR];

// only used when the target is not bmi2 compatible (older than 2013)
uint64_t bishopMagicNumber[NB_SQUARE];
uint64_t rookMagicNumber[NB_SQUARE];

void magic_init();

void bitboard_init()
{
  for (File f = FILE_A; f <= FILE_H; ++f)
  {
    fileBB[f] = FILE_A_BB << (f * EAST);
    adjacentFilesBB[f] = shift<EAST>(fileBB[f]) | shift<WEST>(fileBB[f]);
  }
  for (Rank r = RANK_1; r <= RANK_8; ++r)
  {
    rankBB[r] = 0x00000000000000ffULL << (r * NORTH);
    adjacentRanksBB[r] = shift<NORTH>(rankBB[r]) | shift<SOUTH>(rankBB[r]);
  }
  for (Square sq = SQ_A1; sq <= SQ_H8; ++sq)
  {
    squareBB[sq] = 0x0000000000000001ULL << sq;
    ringBB[sq] = (adjacentFilesBB[file_of(sq)] | file_of(sq)) &
                 (adjacentRanksBB[rank_of(sq)] | rank_of(sq)) & ~sq;
    rookPseudoAttacks[sq] = (bb_of(file_of(sq)) | bb_of(rank_of(sq))) & ~sq;
    forwardBB[sq][WHITE] = shift<NORTH>(bb_of(sq));
    forwardBB[sq][BLACK] = shift<SOUTH>(bb_of(sq));
    pawnPseudoAttack[sq][WHITE] =
        shift<NORTH_EAST>(bb_of(sq)) | shift<NORTH_WEST>(bb_of(sq));
    pawnPseudoAttack[sq][BLACK] =
        shift<SOUTH_EAST>(bb_of(sq)) | shift<SOUTH_WEST>(bb_of(sq));
  }
  for (Diagonal d = DIAG_1; d < NB_DIAGONAL; ++d)
    for (Square sq = SQ_A1; sq <= SQ_H8; ++sq)
    {
      if (file_of(sq) + rank_of(sq) == d)
        diagonalBB[d] |= squareBB[sq];
      if (7 + rank_of(sq) - file_of(sq) == d)
        antiDiagonalBB[d] |= sq;
    }
  for (Square sq = SQ_A1; sq <= SQ_H8; ++sq)
  {
    for (Square sq2 = sq + EAST; sq2 <= SQ_H8; ++sq2)
    {
      if (file_of(sq) + rank_of(sq) == file_of(sq2) + rank_of(sq2) ||
          file_of(sq) + rank_of(sq2) == file_of(sq2) + rank_of(sq))
      {
        bishopPseudoAttacks[sq] |= sq2;
        bishopPseudoAttacks[sq2] |= sq;
      }
      if ((file_of(sq) - file_of(sq2)) * (file_of(sq) - file_of(sq2)) +
              (rank_of(sq) - rank_of(sq2)) * (rank_of(sq) - rank_of(sq2)) ==
          5)
      {
        knightPseudoAttacks[sq] |= sq2;
        knightPseudoAttacks[sq2] |= sq;
      }
      if (file_of(sq) + rank_of(sq) == file_of(sq2) + rank_of(sq2))
        lineBB[sq][sq2] = lineBB[sq2][sq] =
            diagonalBB[file_of(sq) + rank_of(sq)];
      else if (file_of(sq) + rank_of(sq2) == file_of(sq2) + rank_of(sq))
        lineBB[sq][sq2] = lineBB[sq2][sq] =
            antiDiagonalBB[7 + rank_of(sq) - file_of(sq)];
      else if (file_of(sq) == file_of(sq2))
        lineBB[sq][sq2] = lineBB[sq2][sq] = fileBB[file_of(sq)];
      else if (rank_of(sq) == rank_of(sq2))
        lineBB[sq][sq2] = lineBB[sq2][sq] = rankBB[rank_of(sq)];
      for (Square sq3 = sq + EAST; sq3 < sq2; ++sq3)
        if (lineBB[sq][sq2] & sq3)
        {
          betweenBB[sq][sq2] |= sq3;
          betweenBB[sq2][sq] |= sq3;
        }
    }
    queenPseudoAttacks[sq] = rookPseudoAttacks[sq] | bishopPseudoAttacks[sq];
  }
  magic_init();
}

Bitboard bishop_attack(Square sq, Bitboard occupied)
{
  Bitboard ret{0};
  for (Direction d : {NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST})
  {
    Bitboard tmp = bb_of(sq);
    do
    {
      tmp = shift(tmp, d);
      ret |= tmp;
    } while (tmp && !(tmp & occupied));
  }
  return ret;
}

Bitboard rook_attack(Square sq, Bitboard occupied)
{
  Bitboard ret{0};
  for (Direction d : {NORTH, EAST, SOUTH, WEST})
  {
    Bitboard tmp = bb_of(sq);
    do
    {
      tmp = shift(tmp, d);
      ret |= tmp;
    } while (tmp && !(tmp & occupied));
  }
  return ret;
}

void magic_init()
{
  for (Square sq = SQ_A1; sq <= SQ_H8; ++sq)
  {
    bishopMagicMask[sq] = bishopPseudoAttacks[sq] & ~borderBB;
    Bitboard input = bishopMagicMask[sq];
    if (!use_bmi2)
    {
      bishopMagicNumber[sq] = sparse_rand64();
      memset(bishopMagics[sq], 0, (1 << 9) * sizeof(**bishopMagics));
    }
    while (true)
    {
      uint64_t index = magic_index<BISHOP>(input, sq);
      if (use_bmi2 || !bishopMagics[sq][index] ||
          bishop_attack(sq, input) == bishopMagics[sq][index])
        bishopMagics[sq][index] = bishop_attack(sq, input);
      else
      {
        // Effectively loops and tries another
        // magic number until there is no collision
        --sq;
        break;
      }
      if (!input)
        break;
      input = (input - 1) & bishopMagicMask[sq];
    }
  }
  for (Square sq = SQ_A1; sq <= SQ_H8; ++sq)
  {
    Bitboard mask = borderBB;
    mask &= file_of(sq) == FILE_A ? ~FILE_A_BB
                                  : file_of(sq) == FILE_H ? ~FILE_H_BB : ~0ULL;
    mask &= rank_of(sq) == RANK_1 ? ~RANK_1_BB
                                  : rank_of(sq) == RANK_8 ? ~RANK_8_BB : ~0ULL;
    mask |= bb_of(SQ_A1) | SQ_A8 | SQ_H1 | SQ_H8;
    mask = rookPseudoAttacks[sq] & ~mask;
    rookMagicMask[sq] = mask;
    Bitboard input = rookMagicMask[sq];
    ;
    if (!use_bmi2)
    {
      rookMagicNumber[sq] = sparse_rand64();
      memset(rookMagics[sq], 0, (1 << 12) * sizeof(**rookMagics));
    }
    while (true)
    {
      uint64_t index = magic_index<ROOK>(input, sq);
      if (use_bmi2 || !rookMagics[sq][index] ||
          rook_attack(sq, input) == rookMagics[sq][index])
        rookMagics[sq][index] = rook_attack(sq, input);
      else
      {
        --sq;
        break;
      }
      if (!input)
        break;
      input = (input - 1) & rookMagicMask[sq];
    }
  }
}

std::string print_bb(Bitboard bb)
{
  std::string ret = "";
  for (Rank r = RANK_8; r >= RANK_1; --r)
  {
    for (File f = FILE_A; f <= FILE_H; ++f)
    {
      ret += ' ';
      ret += std::to_string(!!(bb & f & r));
    }
    ret += '\n';
  }
  return ret;
}

Bitboard shift(Bitboard bb, Direction d)
{
  switch (d)
  {
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
  default:
    assert(false && "Incorrect direction parameter");
    return 0;
  }
}
