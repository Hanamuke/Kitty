#include "misc.hpp"
#include "moves.hpp"
#include "position.hpp"
#include "tt.hpp"
#include <chrono>
#include <iostream>

Key zobrist_stm;
Key zobrist_sqp[NB_SQUARE][NB_PIECE];
Key zobrist_ep[1 + NB_SQUARE];
Key zobrist_castle[1 << NB_CASTLING];

std::random_device rd;
Gen gen;

void zobrist_init()
{
  zobrist_stm = gen();
  for (Key &key : zobrist_castle)
    key = gen();
  for (Key &key : zobrist_ep)
    key = gen();
  for (auto &tab : zobrist_sqp)
    for (Key &key : tab)
      key = gen();
}
namespace Perft
{
HashTable<PerftEntry> hash;

uint64_t perft_search(Position &pos, Depth depth)
{
  static Moves moves[10];
  bool hit;
  uint64_t nodes = 0;
  PerftEntry *pe = hash.probe(pos.get_key(), hit);
  if (hit && pe->depth == depth)
    return pe->perft;

  moves[depth].set_pos(&pos);
  if (depth == 1)
    return moves[depth].size();

  for (ValueMove m : moves[depth])
  {
    pos.do_move(m);
    nodes += perft_search(pos, Depth(depth - 1));
    pos.undo_move();
  }
  if (pe != nullptr)
  {
    pe->key = pos.get_key();
    pe->perft = nodes;
    pe->depth = Depth(depth);
  }
  return nodes;
}

void perft()
{
  constexpr size_t NB_POSITIONS = 19;
  static const std::string position[NB_POSITIONS] = {
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
      "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
      "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
      "1k6/1b6/8/8/7R/8/8/4K2R b K - 0 1",
      "3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1",
      "8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1",
      "8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1",
      "5k2/8/8/8/8/8/8/4K2R w K - 0 1",
      "3k4/8/8/8/8/8/8/R3K3 w Q - 0 1",
      "r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1",
      "r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1",
      "2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1",
      "8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1",
      "4k3/1P6/8/8/8/8/K7/8 w - - 0 1",
      "8/P1k5/K7/8/8/8/8/8 w - - 0 1",
      "K1k5/8/P7/8/8/8/8/8 w - - 0 1",
      "8/k1P5/8/1K6/8/8/8/8 w - - 0 1",
      "8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1"};
  constexpr int depth[NB_POSITIONS] = {6, 5, 7, 6, 5, 6, 6, 6, 6, 6,
                                       4, 4, 6, 5, 6, 6, 6, 7, 4};
  constexpr uint64_t solution[NB_POSITIONS] = {
      119060324, 193690690, 178633661, 706045033, 1063513, 1134888, 1015133,
      1440467, 661072, 803711, 1274206, 1720476, 3821001, 1004658,
      217342, 92683, 2217, 567584, 23527};
  Position pos;
  auto t0 = std::chrono::system_clock::now();
  uint64_t total_nodes = 0;
  hash.resize(16);
  for (size_t i = 0; i < NB_POSITIONS; i++)
  {
    std::cout << Sync::lock << "Position " << i + 1 << '/' << NB_POSITIONS
              << '\n'
              << Sync::unlock;
    pos.set_position(position[i]);
    Moves moves(&pos);
    uint64_t nodes = 0;
    for (ValueMove m : moves)
    {
      pos.do_move(m);
      uint64_t temp_nodes = perft_search(pos, Depth(depth[i] - 1));
      nodes += temp_nodes;
      pos.undo_move();
    }
    std::cout << Sync::lock << nodes << " nodes searched --> "
              << (nodes == solution[i] ? "OK" : "FAIL") << "\n\n"
              << Sync::unlock;
    total_nodes += nodes;
  }
  auto t1 = std::chrono::system_clock::now();
  float seconds =
      (float)((t1 - t0).count() * std::chrono::system_clock::period::num) /
      (float)std::chrono::system_clock::period::den;
  std::cerr << "Time : " << seconds << " seconds"
            << "\nNodes : " << total_nodes
            << "\nNodes/second : " << (int)(total_nodes / seconds) << std::endl;
  ;
}
} // namespace Perft