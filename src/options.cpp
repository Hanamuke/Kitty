#include "options.hpp"
#include "threads.hpp"

OptionManager Options;
void thread_resize();

std::ostream &operator<<(std::ostream &os, OptionManager const &om)
{
  for (const auto &option : om.optionsMap)
    os << "option name " << option.first << ' ' << option.second;
  return os;
}

std::ostream &operator<<(std::ostream &os, Option const &o)
{
  os << "type " << o.type;
  if (o.type != "button")
    os << " default " << o.default_value;
  if (o.type == "spin")
    os << " min " << o.min << " max " << o.max;
  else if (o.type == "combo")
    for (std::string const &s : o.combo_values)
      os << " var " << s;
  os << '\n';
  return os;
}

bool Option::setValue(std::string const &s)
{
  assert(type != "button");
  if (type == "string")
  {
    str_value = s;
    onChange();
    return true;
  }
  else if (type == "combo")
  {
    if (combo_values.count(s))
    {
      str_value = s;
      onChange();
      return true;
    }
    else
      return false;
  }
  else if (type == "check")
  {
    if (s == "true")
    {
      int_value = true;
      onChange();
      return true;
    }
    else if (s == "false")
    {
      int_value = false;
      onChange();
      return true;
    }
    else
      return false;
  }
  int v = stoi((std::string)s);
  if (v >= min && v <= max)
  {
    int_value = v;
    onChange();
    return true;
  }
  return false;
}

bool Option::setValue(int v)
{
  assert(type == "spin" || type == "check");
  if (type == "check" || (v >= min && v <= max))
  {
    int_value = v;
    onChange();
    return true;
  }
  return false;
}

OptionManager::OptionManager()
{
  newOption("Debug", false);
  newOption("Threads", 1, 1, 1, thread_resize);
}

Option::Option(int dflt, int _min, int _max, Callback c)
    : type("spin"),
      default_value(
          (std::to_string(std::min(_max, std::max(dflt, _min)))).c_str()),
      int_value(dflt), min(_min), max(_max), onChange(c)
{
  assert(min <= max);
}

Option::Option(Callback c) : type("button"), onChange(c) {}

Option::Option(bool dflt, Callback c)
    : type("check"), default_value(dflt ? "true" : "false"), int_value(dflt),
      onChange(c) {}

Option::Option(std::initializer_list<const std::string> values_list, Callback c)
    : type("combo"),
      default_value(values_list.size() > 0 ? *values_list.begin() : ""),
      onChange(c)
{
  str_value = default_value;
  assert(values_list.size() > 0);
  for (std::string const &s : values_list)
  {
    combo_values.emplace(s);
  }
}

Option::Option(std::string const &dflt, Callback c)
    : type("string"), default_value(dflt), str_value(dflt), onChange(c) {}

void no_callback() {}

void thread_resize() { Threads.init(); }
