#include <cassert>
#include <charconv>
#include <sstream>
#include <string>
#include <system_error>

#include <iostream> // for debugging

#include "options.h"
#include "utest.h"

Options::Options()
{
  // set defaults
  _display_quiet = 0;
  _display_diagnostics = 0;
  _display_animate = 1;
  _display_color = 0; // Need to implement terminal_has_color();
  _display_fast = 0;
  _display_save_svg = 0;

  _node_check_touch = 1;
  _node_check_stranded = 1;
  _node_check_deadends = 1;
  _node_bottleneck_limit = 3;
  _node_penalize_exploration = 0;

  _order_autosort_colors = 1;
  _order_most_constrained = 1;
  _order_forced_first = 1;

  _search_best_first = 1;
  _search_outside_in = 1;
  _search_max_nodes = 0;
  _search_max_mb = 128;
  _search_fast_forward = 1;

  // initialise the _flag_options table
  _flag_options.push_back({ "-q", "--quiet", &_display_quiet, 1 });
  _flag_options.push_back({ "-D", "--diagnostics", &_display_diagnostics, 1 });
  _flag_options.push_back({ "-A", "--animation", &_display_animate, 0 });
#ifndef _WIN32
  _flag_options.push_back({ "-C", "--color", &_display_color, 1 });
#endif
  _flag_options.push_back({ "-F", "--fast", &_display_fast, 1 });
  _flag_options.push_back({ "-S", "--svg", &_display_save_svg, 1 });
  _flag_options.push_back({ "-t", "--touch", &_node_check_touch, 0 });
  _flag_options.push_back({ "-s", "--stranded", &_node_check_stranded, 0 });
  _flag_options.push_back({ "-d", "--deadends", &_node_check_deadends, 0 });
  _flag_options.push_back({ "-b", "--bottlenecks", 0, 0 });
  _flag_options.push_back(
    { "-e", "--no-explore", &_node_penalize_exploration, 1 });
  _flag_options.push_back(
    { "-a", "--no-autosort", &_order_autosort_colors, 0 });
  _flag_options.push_back({ "-o", "--order", 0, 0 });
  _flag_options.push_back({ "-r", "--randomize", &_order_random, 1 });
  _flag_options.push_back({ "-f", "--forced", &_order_forced_first, 0 });
  _flag_options.push_back(
    { "-c", "--constrained", &_order_most_constrained, 0 });
  _flag_options.push_back({ "-O", "--no-outside-in", &_search_outside_in, 0 });
  _flag_options.push_back({ "-B", "--breadth-first", &_search_best_first, 0 });
  _flag_options.push_back({ "-Q", "--queue-always", &_search_fast_forward, 0 });
  _flag_options.push_back({ "-n", "--max-nodes", 0, 0 });
  _flag_options.push_back({ "-m", "--max-storage", 0, 0 });
  _flag_options.push_back({ "-H", "--hint", 0, 0 });
  _flag_options.push_back({ "-h", "--help", 0, 0 });
};

void
Options::usage(int exitcode) const
{
  std::ostringstream u;

  u << "usage: flow_solver [ OPTIONS ] [ -H HINT1.txt ] [ -o ORDER1 ] "
       "BOARD1.txt\n"
    << "                   [ [ -H HINT2.txt ] [ -o ORDER2 ] BOARD2.txt [ ... ] "
    << "]\n\n"
    << "Display options:\n\n"
    << "  -q, --quiet             Reduce output\n"
    << "  -D, --diagnostics       Print diagnostics when search unsuccessful\n"
    << "  -A, --no-animation      Disable animating solution\n"
    << " -F, --fast              Speed up animation 4x\n"
#ifndef _WIN32
    << "  -C, --color Force use of ANSI color\n"
#endif
    << "  -S, --svg               Output final state to SVG\n"
    << "\n"
    << "Node evaluation options:\n\n"
    << "  -t, --touch             Disable path self-touch test\n"
    << "  -s, --stranded          Disable stranded checking\n"
    << "  -d, --deadends          Disable dead-end checking\n"
    << "  -b, --bottlenecks N     Set bottleneck limit check (default "
    << _node_bottleneck_limit << ")\n"
    << "  -e, --no-explore        Penalize exploring away from walls\n"
    << "\n"
    << "Color ordering options:\n\n"
    << "  -a, --no-autosort       Disable auto-sort of color order\n"
    << "  -r, --randomize         Shuffle order of colors before solving\n"
    << "  -f, --forced            Disable ordering forced moved first\n"
    << "  -c, --constrained       Disable order by most constrained\n"
    << "\n"
    << "Search options:\n\n"
    << "  -O, --no-outside-in     Disable outside-in searching\n"
    << "  -B, --breadth-first     Breadth-first search instead of best-first\n"
    << "  -n, --max-nodes N       Restrict storage to N nodes\n"
    << "  -m, --max-storage N     Restrict storage to N MB (default "
    << _search_max_mb << ")\n"
    << "  -Q, --queue-always      Disable \"fast-forward\" queue bypassing\n"
    << "\n"
    << "Options affecting the next input file:\n\n"
    << "  -o, --order ORDER       Set color order on command line\n"
    << "  -H, --hint HINTFILE     Provide hint for previous board.\n"
    << "\n"
    << "Help:\n\n"
    << "  -h, --help              See this help text\n\n";

  std::string usage = u.str();
  std::cout << usage << std::endl;

  exit(exitcode);
}

//////////////////////////////////////////////////////////////////////
// Check file exists

UTEST(Options, exists_true)
{
  EXPECT_TRUE(Options::existsAndIsReadable("options.cpp"));
}

UTEST(Options, exists_false)
{
  EXPECT_FALSE(Options::existsAndIsReadable("does-not-exists.cpp"));
}

bool
Options::existsAndIsReadable(const std::filesystem::path& p)
{
  namespace fs = std::filesystem;

  if (fs::exists(p)) {
    std::error_code ec; // For noexcept overload usage.
    auto perms = fs::status(p, ec).permissions();
    if (checkReadable(perms)) {
      return true;
    }
  }
  return false;
}

bool
Options::checkReadable(std::filesystem::perms permissions)
{
  namespace fs = std::filesystem;

  return (permissions & fs::perms::owner_read) != fs::perms::none &&
         (permissions & fs::perms::group_read) != fs::perms::none &&
         (permissions & fs::perms::others_read) != fs::perms::none;
}

int
Options::convertToInt(const std::string_view& str)
{
  int value{ 0 };
  auto result = std::from_chars(str.data(), str.data() + str.size(), value);

  if (result.ec == std::errc()) {
    return value;
  } else if (result.ec == std::errc::invalid_argument) {
    throw std::invalid_argument("Invalid argument: could not convert.");
  } else if (result.ec == std::errc::result_out_of_range) {
    throw std::out_of_range("Result out of range.");
  }

  return value;
}

UTEST(Options, convert_to_int)
{
  Options opt;
  EXPECT_EQ(opt.convertToInt("123"), 123);
  EXPECT_EQ(opt.convertToInt("0"), 0);
  EXPECT_EQ(opt.convertToInt("1"), 1);
  EXPECT_EQ(opt.convertToInt("100"), 100);
  EXPECT_EQ(opt.convertToInt("1000"), 1000);
}

double
Options::convertToDouble(const std::string_view& str)
{
  double value{ 0 };
  auto result = std::from_chars(str.data(), str.data() + str.size(), value);

  if (result.ec == std::errc()) {
    return value;
  } else if (result.ec == std::errc::invalid_argument) {
    throw std::invalid_argument("Invalid argument: could not convert.");
  } else if (result.ec == std::errc::result_out_of_range) {
    throw std::out_of_range("Result out of range.");
  }

  return value;
}

UTEST(Options, convert_to_double)
{
  Options opt;
  EXPECT_EQ(opt.convertToDouble("123.45"), 123.45);
  EXPECT_EQ(opt.convertToDouble("0.0"), 0.0);
  EXPECT_EQ(opt.convertToDouble("1.0"), 1.0);
  EXPECT_EQ(opt.convertToDouble("100.0"), 100.0);
  EXPECT_EQ(opt.convertToDouble("1000.0"), 1000.0);
  EXPECT_EXCEPTION_WITH_MESSAGE(opt.convertToDouble("one-hundred"),
                                std::invalid_argument,
                                "Invalid argument: could not convert.");
}

void
Options::parse(int argc, char** argv)
{
  const std::vector<std::string_view> args(argv + 1, argv + argc);

  for (std::vector<std::string_view>::const_iterator it = args.cbegin();
       it != args.cend();
       ++it) {
    const auto& opt = *it;
    bool foundMatchingOption = false;

    for (const auto& entry : _flag_options) {
      if (entry.short_char == opt || entry.long_string == opt) {
        foundMatchingOption = true;
        if (entry.dst_flag) {
          *entry.dst_flag = entry.dst_value;
        } else if (entry.short_char == "-b") {
          handleNodeBottleneckLimitOption(it, args.cend());
        } else if (entry.short_char == "-n") {
          handleSearchMaxNodesOption(it, args.cend());
        } else if (entry.short_char == "-m") {
          handleSearchMaxMbOption(it, args.cend());
        } else if (entry.short_char == "-H") {
          handleHintOption(it, args.cend());
        } else if (entry.short_char == "-o") {
          handleUserOrders(it, args.cend());
        } else if (entry.short_char == "-h") {
          usage(0);
        } else { // should not happen
          handleUnrecognizedOption(opt);
        }
        break; // we have found a match, no point checking other _flag_options
      } // if arg match
    } // loop over _flag_options

    if (foundMatchingOption) {
      continue;
    } else if (existsAndIsReadable(opt)) {
      handlePuzzleFileToSolve(opt);
    } else {
      handleUnrecognizedOption(opt);
    }
  } // loop over args

  if (_input_files.empty()) {
    std::cerr << "no input files\n\n" << std::endl;
    exit(1);
  }

  /* TODO: Don't understand the following code

  else if (user_orders[num_inputs]) {
    fprintf(stderr, "order specified *after* last input file!\n\n");
    exit(1);
  }
  else if (hint_files[num_inputs]) {
    fprintf(stderr, "hint file specified *after* last input file!\n\n");
    exit(1);
  }
  */
}

UTEST(Options, parse)
{
  Options opt;

  // as we check for existence of files, we need to pass valid files, so we
  // pass in the name of this source file multiple times.
  std::vector<std::string_view> argv = {
    "flow_solver", "-q",    "-D",          "-A",          "-C",          "-F",
    "-S",          "-t",    "-s",          "-d",          "-b",          "3",
    "-e",          "-a",    "-o",          "options.cpp", "-r",          "-f",
    "-c",          "-O",    "-B",          "-Q",          "-n",          "100",
    "-m",          "128.0", "-H",          "options.cpp", "options.cpp", "-H",
    "options.cpp", "-o",    "options.cpp", "options.cpp"
  };

  // convert to c-style strings
  std::vector<char*> cstr_argv;
  for (const auto& sv : argv) {
    cstr_argv.push_back(const_cast<char*>(sv.data()));
  }

  // Pass the array to the function
  opt.parse(cstr_argv.size(), cstr_argv.data());

  EXPECT_EQ(opt.display_quiet(), 1);
  EXPECT_EQ(opt.display_diagnostics(), 1);
  EXPECT_EQ(opt.display_animate(), 0);
  EXPECT_EQ(opt.display_color(), 1);
  EXPECT_EQ(opt.display_fast(), 1);
  EXPECT_EQ(opt.display_save_svg(), 1);
  EXPECT_EQ(opt.node_check_touch(), 0);
  EXPECT_EQ(opt.node_check_stranded(), 0);
  EXPECT_EQ(opt.node_check_deadends(), 0);
  EXPECT_EQ(opt.node_bottleneck_limit(), 3);
  EXPECT_EQ(opt.node_penalize_exploration(), 1);
  EXPECT_EQ(opt.order_autosort_colors(), 0);
  EXPECT_EQ(opt.order_most_constrained(), 0);
  EXPECT_EQ(opt.order_forced_first(), 0);
  EXPECT_EQ(opt.order_random(), 1);
  EXPECT_EQ(opt.search_best_first(), 0);
  EXPECT_EQ(opt.search_outside_in(), 0);
  EXPECT_EQ(opt.search_max_nodes(), 100);
  EXPECT_EQ(opt.search_max_mb(), 128.0);
  EXPECT_EQ(opt.search_fast_forward(), 0);
  EXPECT_EQ(opt.input_files().size(), 2);
  EXPECT_EQ(opt.hint_files().size(), 2);
  EXPECT_EQ(opt.user_orders().size(), 2);
}

void
Options::handleNodeBottleneckLimitOption(
  std::vector<std::string_view>::const_iterator& it,
  std::vector<std::string_view>::const_iterator end)
{
  ++it;
  if (it != end) {
    const auto& next_arg = *it;
    _node_bottleneck_limit = convertToInt(next_arg);
  } else {
    std::cerr << "No next argument for -b." << std::endl;
  }
}

void
Options::handleSearchMaxNodesOption(
  std::vector<std::string_view>::const_iterator& it,
  std::vector<std::string_view>::const_iterator end)
{
  ++it;
  if (it != end) {
    const auto& next_arg = *it;
    _search_max_nodes = convertToInt(next_arg);
  } else {
    std::cerr << "No next argument for -n." << std::endl;
  }
}

void
Options::handleSearchMaxMbOption(
  std::vector<std::string_view>::const_iterator& it,
  std::vector<std::string_view>::const_iterator end)
{
  ++it;
  if (it != end) {
    const auto& next_arg = *it;
    _search_max_mb = convertToDouble(next_arg);
  } else {
    std::cerr << "No next argument for -m." << std::endl;
  }
}

void
Options::handleHintOption(std::vector<std::string_view>::const_iterator& it,
                          std::vector<std::string_view>::const_iterator end)
{
  ++it;
  if (it != end) {
    const auto& next_arg = *it;

    // check if file exists
    if (existsAndIsReadable(next_arg)) {
      _hint_files.push_back(next_arg);
    } else {
      throw std::runtime_error("Error opening file: " + std::string(next_arg));
    }
  } else {
    throw std::invalid_argument("No next argument.");
  }
}

void
Options::handleUserOrders(std::vector<std::string_view>::const_iterator& it,
                          std::vector<std::string_view>::const_iterator end)
{
  ++it;
  if (it != end) {
    const auto& next_arg = *it;
    _user_orders.push_back(next_arg);
  } else {
    throw std::invalid_argument("No next argument.");
  }
}

void
Options::handlePuzzleFileToSolve(const std::string_view& opt)
{
  if (existsAndIsReadable(opt)) {
    _input_files.push_back(opt);
  } else {
    throw std::runtime_error("Error opening file: " + std::string(opt));
  }
}

void
Options::handleUnrecognizedOption(const std::string_view& opt) const
{
  std::cerr << "unrecognized option: " << opt << std::endl;
  usage(1);
}

void
Options::printFlagOptions() const
{
  for (const auto& entry : _flag_options) {
    std::cout << "short_char: " << entry.short_char
              << " long_string: " << entry.long_string
              << " dst_flag: " << entry.dst_flag
              << " dst_value:  " << entry.dst_value << std::endl;
  }
}

void
Options::printOptionState() const
{
  std::cout << "display_quiet: " << _display_quiet << std::endl;
  std::cout << "display_diagnostics: " << _display_diagnostics << std::endl;
  std::cout << "display_animate: " << _display_animate << std::endl;
  std::cout << "display_color: " << _display_color << std::endl;
  std::cout << "display_fast: " << _display_fast << std::endl;
  std::cout << "display_save_svg: " << _display_save_svg << std::endl;
  std::cout << "node_check_touch: " << _node_check_touch << std::endl;
  std::cout << "node_check_stranded: " << _node_check_stranded << std::endl;
  std::cout << "node_check_deadends: " << _node_check_deadends << std::endl;
  std::cout << "node_bottleneck_limit: " << _node_bottleneck_limit << std::endl;
  std::cout << "node_penalize_exploration: " << _node_penalize_exploration
            << std::endl;
  std::cout << "order_autosort_colors: " << _order_autosort_colors << std::endl;
  std::cout << "order_most_constrained: " << _order_most_constrained
            << std::endl;
  std::cout << "order_forced_first: " << _order_forced_first << std::endl;
  std::cout << "order_random: " << _order_random << std::endl;
  std::cout << "search_best_first: " << _search_best_first << std::endl;
  std::cout << "search_outside_in: " << _search_outside_in << std::endl;
  std::cout << "search_max_nodes: " << _search_max_nodes << std::endl;
  std::cout << "search_max_mb: " << _search_max_mb << std::endl;
  std::cout << "search_fast_forward: " << _search_fast_forward << std::endl;

  std::cout << "input_files: ";
  for (const auto& file : _input_files) {
    std::cout << file << " ";
  }
  std::cout << std::endl;

  std::cout << "user_orders: ";
  for (const auto& order : _user_orders) {
    std::cout << order << " ";
  }
  std::cout << std::endl;

  std::cout << "hint_files: ";
  for (const auto& hint : _hint_files) {
    std::cout << hint << " ";
  }
  std::cout << std::endl;
}
