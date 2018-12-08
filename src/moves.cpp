#include "bitboards.hpp"
#include "misc.hpp"
#include "position.hpp"
#include "types.hpp"

template <PieceType pt, Color c>
ValueMove *Position::generate_moves(ValueMove *moveList,
                                    Bitboard target) const
{
  constexpr Piece p = make_piece(c, pt);
  for (const Square *sq = pieceList[p]; sq != pieceList[p] + nbPiece[p]; ++sq)
  {
    Bitboard b = is_pinned(*sq)
                     ? attacks_from<pt>(*sq) & target & lineBB[our_king()][*sq]
                     : attacks_from<pt>(*sq) & target;
    while (b)
      *moveList++ = make_move<NORMAL>(*sq, pop_lsb(b));
  }
  return moveList;
}

template <Color c>
ValueMove *Position::generate_pawn_moves(ValueMove *moveList,
                                         Bitboard target) const
{
  constexpr Direction up = c ? SOUTH : NORTH;
  constexpr Piece p = make_piece(c, PAWN);
  for (const Square *sq = pieceList[p]; sq != pieceList[p] + nbPiece[p]; ++sq)
  {
    bool pinned = is_pinned(*sq);
    // Generating captures
    Bitboard b = pinned ? attacks_from<PAWN>(*sq, c) & pieces(!c) &
                              lineBB[our_king()][*sq] & target
                        : attacks_from<PAWN>(*sq, c) & pieces(!c) & target;
    if (b & promotionRankBB)
      while (b)
      {
        Square to = pop_lsb(b);
        *moveList++ = make_move<PROMOTION>(*sq, to, QUEEN);
        *moveList++ = make_move<PROMOTION>(*sq, to, KNIGHT);
        *moveList++ = make_move<PROMOTION>(*sq, to, ROOK);
        *moveList++ = make_move<PROMOTION>(*sq, to, BISHOP);
      }
    else
      while (b)
        *moveList++ = make_move<NORMAL>(*sq, pop_lsb(b));

    // Generating pawn pushes and quiet promotions
    Square to = *sq + up;
    if (!piece(to) && (!pinned || file_of(*sq) == file_of(our_king())))
    {
      if (target & to)
      {
        if (rank_of(to) != (c ? RANK_1 : RANK_8))
          *moveList++ = make_move<NORMAL>(*sq, to);
        else
        {
          *moveList++ = make_move<PROMOTION>(*sq, to, QUEEN);
          *moveList++ = make_move<PROMOTION>(*sq, to, KNIGHT);
          *moveList++ = make_move<PROMOTION>(*sq, to, ROOK);
          *moveList++ = make_move<PROMOTION>(*sq, to, BISHOP);
        }
      }
      if (rank_of(*sq) == (c ? RANK_7 : RANK_2) && !piece(to + up) &&
          (target & (to + up)))
        *moveList++ = make_move<NORMAL>(*sq, to + up);
    }
  }
  // Generating en passant capture
  if (state->epSquare != NO_SQUARE &&
      target & (bb_of(state->epSquare) | (state->epSquare + down(c))))
  {
    Square ksq = our_king();
    Bitboard b = pieces(PAWN, c) & pawnPseudoAttack[state->epSquare][!c];
    while (b)
    {
      Square from = pop_lsb(b);
      if ((!is_pinned(from) || lineBB[from][ksq] & state->epSquare) &&
          rank_of(from) != rank_of(ksq))
        *moveList++ = make_move<EN_PASSANT>(from, state->epSquare);
      else if (rank_of(from) == rank_of(ksq))
      {
        Square takenPawn = state->epSquare + down(c);
        Square closerToKing, furtherFromKing;
        if (betweenBB[ksq][from] & takenPawn)
        {
          closerToKing = takenPawn;
          furtherFromKing = from;
        }
        else
        {
          closerToKing = from;
          furtherFromKing = takenPawn;
        }
        if (pieces() & betweenBB[ksq][closerToKing] ||
            !(attacks_from<ROOK>(furtherFromKing) & pieces(!c) &
              pieces(ROOK, QUEEN) & lineBB[ksq][furtherFromKing]))
          *moveList++ = make_move<EN_PASSANT>(from, state->epSquare);
      }
    }
  }
  return moveList;
}

template <Color c>
ValueMove *Position::generate_king_moves(ValueMove *moveList,
                                         Bitboard target) const
{
  Square from = our_king();
  Bitboard b = attacks_from<KING>(from) & target;
  // normal moves
  while (b)
  {
    Square sq = pop_lsb(b);
    if (!(attacks_to(sq) & pieces(!c)))
      *moveList++ = make_move<NORMAL>(from, sq);
  }
  // castling
  int cstlrights = state->castlingRights;
  if (cstlrights && !state->checkers)
  {
    if constexpr (c)
    {
      if ((cstlrights & B_OO) && !(betweenBB[SQ_E8][SQ_H8] & pieces()) &&
          !((attacks_to(SQ_F8) | attacks_to(SQ_G8)) & pieces(WHITE)))
        *moveList++ = cstl_move[2];
      if ((cstlrights & B_OOO) && !(betweenBB[SQ_E8][SQ_A8] & pieces()) &&
          !((attacks_to(SQ_C8) | attacks_to(SQ_D8)) & pieces(WHITE)))
        *moveList++ = cstl_move[3];
    }
    else
    {
      if ((cstlrights & W_OO) && !(betweenBB[SQ_E1][SQ_H1] & pieces()) &&
          !((attacks_to(SQ_F1) | attacks_to(SQ_G1)) & pieces(BLACK)))
        *moveList++ = cstl_move[0];
      if ((cstlrights & W_OOO) && !(betweenBB[SQ_E1][SQ_A1] & pieces()) &&
          !((attacks_to(SQ_C1) | attacks_to(SQ_D1)) & pieces(BLACK)))
        *moveList++ = cstl_move[1];
    }
  }
  return moveList;
}

ValueMove *Position::generate_moves(ValueMove *moveList) const
{
  Bitboard target = ~pieces(stm);
  Bitboard ktarget = target;
  Bitboard checkers = state->checkers;
  while (checkers)
  {
    Square checker = pop_lsb(checkers);
    if (piece_type_of(piece(checker)) != PAWN)
      ktarget &= ~lineBB[our_king()][checker] | checker;
    target &= betweenBB[our_king()][checker] | checker;
  }
  if (stm)
  {
    if (target)
    {
      moveList = generate_pawn_moves<BLACK>(moveList, target);
      moveList = generate_moves<KNIGHT, BLACK>(moveList, target);
      moveList = generate_moves<BISHOP, BLACK>(moveList, target);
      moveList = generate_moves<ROOK, BLACK>(moveList, target);
      moveList = generate_moves<QUEEN, BLACK>(moveList, target);
    }
    return generate_king_moves<BLACK>(moveList, ktarget);
  }
  else
  {
    if (target)
    {
      moveList = generate_pawn_moves<WHITE>(moveList, target);
      moveList = generate_moves<KNIGHT, WHITE>(moveList, target);
      moveList = generate_moves<BISHOP, WHITE>(moveList, target);
      moveList = generate_moves<ROOK, WHITE>(moveList, target);
      moveList = generate_moves<QUEEN, WHITE>(moveList, target);
    }
    return generate_king_moves<WHITE>(moveList, ktarget);
  }
}