#include "options.hpp"
#include "threads.hpp"

OptionManager Options;
void thread_resize();

std::ostream &operator<<(std::ostream &os, OptionManager const &om) {
  for (const auto &it : om.optionsMap)
    os << "option name " << it.first << ' ' << it.second;
  return os;
}

std::ostream &operator<<(std::ostream &os, Option const &o) {
  os << "type " << o.type;
  if (strcmp(o.type, "button"))
    os << " default " << o.default_value;
  if (!strcmp(o.type, "spin"))
    os << " min " << o.min << " max " << o.max;
  else if (!strcmp(o.type, "combo"))
    for (const char *s : o.combo_values)
      os << " var " << s;
  os << '\n';
  return os;
}

bool Option::setValue(const char *s) {
  assert(type != "button");
  if (!strcmp(type, "string")) {
    str_value = s;
    onChange();
    return true;
  } else if (!strcmp(type, "combo")) {
    if (combo_values.count(s)) {
      str_value = s;
      onChange();
      return true;
    } else
      return false;
  } else if (!strcmp(type, "check")) {
    if (!strcmp(s, "true")) {
      int_value = true;
      onChange();
      return true;
    } else if (!strcmp(s, "false")) {
      int_value = false;
      onChange();
      return true;
    } else
      return false;
  }
  int v = stoi((std::string)s);
  if (v >= min && v <= max) {
    int_value = v;
    onChange();
    return true;
  }
  return false;
}

bool Option::setValue(int v) {
  assert(type == "spin" || type == "check");
  if (!strcmp(type, "check") || (v >= min && v <= max)) {
    int_value = v;
    onChange();
    return true;
  }
  return false;
}

OptionManager::OptionManager() {
  newOption("Debug", false);
  newOption("Threads", 1, 1, 1, thread_resize);
}

Option::Option(int dflt, int _min, int _max, Callback c)
    : type("spin"),
      default_value(
          (std::to_string(std::min(_max, std::max(dflt, _min)))).c_str()),
      int_value(dflt), min(_min), max(_max), onChange(c) {
  assert(min <= max);
}

Option::Option(Callback c) : type("button"), onChange(c) {}

Option::Option(bool dflt, Callback c)
    : type("check"), default_value(dflt ? "true" : "false"), int_value(dflt),
      onChange(c) {}

Option::Option(std::initializer_list<const char *> list, Callback c)
    : type("combo"), default_value(list.size() > 0 ? *list.begin() : ""),
      onChange(c) {
  str_value = (const char *)default_value;
  assert(list.size() > 0);
  for (const char *s : list) {
    combo_values.emplace(s);
  }
}

Option::Option(const char *dflt, Callback c)
    : type("string"), default_value(dflt), str_value(dflt), onChange(c) {}

void no_callback() {}

void thread_resize() { Threads.init(); }
