#include "bitboards.hpp"
#include "misc.hpp"
#include "options.hpp"
#include "threads.hpp"
#include "types.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

void engine_init();

namespace UCI {
std::string engine_info();
void position(std::istream &);
void set_option(std::istream &is);
void go();
} // namespace UCI

int main(int argc, char **argv) {
  std::string cmd, token;
  std::istringstream is;
  // initialisation of the engine
  engine_init();
  // UCI Loop
  // Only this thread will use std::cin, so it doesn't need synchonisation, as
  // opposed to std::cout. cin is tied to cout, it will wait for it to unlocked
  // anyway.
  // accepts one commands from argument.
  for (int i = 1; i < argc; ++i)
    cmd += std::string(argv[i]) + ' ';
  is.str(cmd);
  while (token != "quit") {
    is >> token;
    if (is.eof()) {
      getline(std::cin, cmd);
      is.clear();
      is.str(cmd);
      is >> token;
    }
    if (token == "uci")
      std::cout << Sync::lock << UCI::engine_info() << Options << "uciok\n"
                << Sync::unlock;
    else if (token == "isready")
      std::cout << Sync::lock << "readyok\n" << Sync::unlock;
    else if (token == "ucinewgame")
      Threads.reset();
    else if (token == "position")
      UCI::position(is);
    else if (token == "go")
      UCI::go();
    else if (token == "debug") {
      is >> token;
      if (token == "on")
        Options["debug"].setValue(true);
      else
        Options["debug"].setValue(false);
    } else if (token == "setoption")
      UCI::set_option(is);
    else if (token == "stop")
      Threads.stop_search();
    else if (token == "ponderhit")
      ; // TODO
    else if (token == "perft" || token == "bench") {
      Perft::perft();
      Threads.terminate();
      token = "quit";
    } else if (token == "quit") {
      Threads.terminate();
    }
  }
  return 0;
}

namespace UCI {
std::string engine_info() { return "id name Kitty\nid author Loli\n"; }

void position(std::istream &is) {
  std::string cmd, fen, move;
  Position pos;
  is >> cmd;
  if (cmd == "startpos") {
    pos.set_position(startfen);
    if (!is.eof())
      is >> move;
  } else if (cmd == "fen") {
    is >> fen;
    std::string temp;
    while (!is.eof() && temp != "moves") {
      is >> temp;
      fen += ' ' + temp;
    }
    pos.set_position(fen);
  }
  while (!is.eof()) {
    is >> move;
    pos.do_move(move);
  }
  pos.rebase_stack();
  Threads.setPosition(std::move(pos));
}
} // namespace UCI

void UCI::set_option(std::istream &is) {
  std::string token, name, value;
  while (!is.eof() && token != "name")
    is >> token; // consumes "name"
  while (!is.eof()) {
    is >> token;
    if (token == "value")
      break;
    else
      name += token;
  }
  while (!is.eof()) {
    is >> token;
    value += token;
  }
  Options[name.c_str()].setValue(value.c_str());
}
void engine_init() {
  zobrist_init();
  bitboard_init();
  Threads.init();
}

void UCI::go() { Threads.start_searching(); }
