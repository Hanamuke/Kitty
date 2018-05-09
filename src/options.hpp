#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstring>
#include <iostream>
#include <map>
#include <mutex>
#include <ostream>
#include <set>
#include <string>

using Callback = void (*)();
void no_callback();
class Option {
public:
  // never called but the compiler don't know it
  Option() = delete;
  Option(Option &&) = delete;

  // Constructors for the different option types
  Option(int dflt, int min, int max, Callback = no_callback);
  Option(Callback);
  Option(bool dflt, Callback = no_callback);
  Option(const char *, Callback = no_callback);
  Option(std::initializer_list<const char *>, Callback = no_callback);

  friend std::ostream &operator<<(std::ostream &, Option const &);

  // return true if the change was done (i.e. Valid value);
  bool setValue(const char *);
  bool setValue(int);

  // getters
  operator int() const {
    assert(type == "spin" || type == "check");
    if (!strcmp(type, "check"))
      return !!int_value;
    return int_value;
  }
  operator std::string() const {
    assert(type != "button");
    if (!strcmp(type, "spin") || !strcmp(type, "check"))
      return std::to_string(int_value);
    return (const char *)str_value;
  }

private:
  std::atomic<const char *> type, default_value, str_value;
  std::atomic_int int_value, min, max;
  Callback onChange;
  // list of possible string values for combo type options.
  std::set<std::atomic<const char *>> combo_values;
};

class OptionManager {
public:
  OptionManager();
  OptionManager(OptionManager &&) = delete;
  friend std::ostream &operator<<(std::ostream &, OptionManager const &);
  template <typename... Args>
  void newOption(const char *name, Args &&... args) noexcept {
    optionsMap.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                       std::forward_as_tuple(args...));
  }
  inline Option &operator[](const char *s) {
    assert(optionsMap.count(s));
    return optionsMap.at(s);
  }
  inline bool contains(const char *s) { return optionsMap.count(s); }

private:
  // UCI specifies that option names should be case insensitive
  struct CompareCaseInsensitive {
    bool operator()(std::string const &a, std::string const &b) const {
      return std::lexicographical_compare(
          a.cbegin(), a.cend(), b.cbegin(), b.cend(),
          [](char c1, char c2) { return tolower(c1) < tolower(c2); });
    }
  };
  std::map<const char *, Option, CompareCaseInsensitive> optionsMap;
};
extern OptionManager Options;
#endif
