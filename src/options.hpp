#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <map>
#include <ostream>
#include <set>
#include <string>

using Callback = void (*)();
void no_callback();
class Option {
public:
  Option() = delete;
  Option(Option &&) = delete;

  // Constructors for the different option types
  Option(int dflt, int min, int max, Callback = no_callback);
  Option(Callback);
  Option(bool dflt, Callback = no_callback);
  Option(std::string const &, Callback = no_callback);
  Option(std::initializer_list<const std::string>, Callback = no_callback);

  friend std::ostream &operator<<(std::ostream &, Option const &);

  // return true if the change was done (i.e. Valid value);
  bool setValue(std::string const &);
  bool setValue(int);

  // getters
  operator int() const {
    assert(type == "spin" || type == "check");
    if (type == "check")
      return !!int_value;
    return int_value;
  }
  operator std::string() const {
    assert(type != "button");
    if (type == "spin" || type == "check")
      return std::to_string(int_value);
    return str_value;
  }

private:
  std::string type, default_value, str_value;
  int int_value, min, max;
  Callback onChange;
  // list of possible string values for combo type options.
  std::set<std::string> combo_values;
};

class OptionManager {
public:
  OptionManager();
  OptionManager(OptionManager &&) = delete;
  friend std::ostream &operator<<(std::ostream &, OptionManager const &);
  template <typename... Args>
  void newOption(std::string const &name, Args &&... args) noexcept {
    optionsMap.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                       std::forward_as_tuple(args...));
  }
  inline Option &operator[](std::string const &s) {
    assert(optionsMap.count(s));
    return optionsMap.at(s);
  }
  inline bool contains(std::string const &s) { return optionsMap.count(s); }

private:
  // UCI specifies that option names should be case insensitive
  struct CompareCaseInsensitive {
    bool operator()(std::string const &a, std::string const &b) const {
      return std::lexicographical_compare(
          a.cbegin(), a.cend(), b.cbegin(), b.cend(),
          [](char c1, char c2) { return tolower(c1) < tolower(c2); });
    }
  };
  std::map<const std::string, Option, CompareCaseInsensitive> optionsMap;
};
extern OptionManager Options;
#endif
