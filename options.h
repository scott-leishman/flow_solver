#ifndef OPTIONS_H
#define OPTIONS_H

#include <filesystem>
#include <string_view>
#include <vector>

class Options
{
public:
  Options();
  ~Options() = default;

  void parse(int argc, char** argv);

  void usage(int exitcode) const;
  static bool existsAndIsReadable(const std::filesystem::path& p);
  static bool checkReadable(std::filesystem::perms permissions);
  int convertToInt(const std::string_view& str);
  double convertToDouble(const std::string_view& str);

  int display_quiet() const { return _display_quiet; }
  int display_diagnostics() const { return _display_diagnostics; }
  int display_animate() const { return _display_animate; }
  int display_color() const { return _display_color; }
  int display_fast() const { return _display_fast; }
  int display_save_svg() const { return _display_save_svg; }

  int node_check_touch() const { return _node_check_touch; }
  int node_check_stranded() const { return _node_check_stranded; }
  int node_check_deadends() const { return _node_check_deadends; }
  int node_bottleneck_limit() const { return _node_bottleneck_limit; }
  int node_penalize_exploration() const { return _node_penalize_exploration; }

  int order_autosort_colors() const { return _order_autosort_colors; }
  int order_most_constrained() const { return _order_most_constrained; }
  int order_forced_first() const { return _order_forced_first; }
  int order_random() const { return _order_random; }

  int search_best_first() const { return _search_best_first; }
  int search_outside_in() const { return _search_outside_in; }
  size_t search_max_nodes() const { return _search_max_nodes; }
  double search_max_mb() const { return _search_max_mb; }
  int search_fast_forward() const { return _search_fast_forward; }

  const std::vector<std::string_view>& input_files() const
  {
    return _input_files;
  }
  const std::vector<std::string_view>& user_orders() const
  {
    return _user_orders;
  }
  const std::vector<std::string_view>& hint_files() const
  {
    return _hint_files;
  }

  void printFlagOptions() const;
  void printOptionState() const;

private:
  // Option handlers to simplify the parse function.
  void handleNodeBottleneckLimitOption(
    std::vector<std::string_view>::const_iterator& it,
    std::vector<std::string_view>::const_iterator end);
  void handleSearchMaxNodesOption(
    std::vector<std::string_view>::const_iterator& it,
    std::vector<std::string_view>::const_iterator end);
  void handleSearchMaxMbOption(
    std::vector<std::string_view>::const_iterator& it,
    std::vector<std::string_view>::const_iterator end);
  void handleHintOption(std::vector<std::string_view>::const_iterator& it,
                        std::vector<std::string_view>::const_iterator end);
  void handleUserOrders(std::vector<std::string_view>::const_iterator& it,
                        std::vector<std::string_view>::const_iterator end);
  void handlePuzzleFileToSolve(const std::string_view& opt);
  void handleUnrecognizedOption(const std::string_view& opt) const;

  int _display_quiet;
  int _display_diagnostics;
  int _display_animate;
  int _display_color;
  int _display_fast;
  int _display_save_svg;

  int _node_check_touch;
  int _node_check_stranded;
  int _node_check_deadends;
  int _node_bottleneck_limit;
  int _node_penalize_exploration;

  int _order_autosort_colors;
  int _order_most_constrained;
  int _order_forced_first;
  int _order_random;

  int _search_best_first;
  int _search_outside_in;
  size_t _search_max_nodes;
  double _search_max_mb;
  int _search_fast_forward;

  std::vector<std::string_view> _input_files;
  std::vector<std::string_view> _user_orders;
  std::vector<std::string_view> _hint_files;

  // Define data table to capture all the arguments so we can
  // iterate over them in a loop.
  struct flag_options_struct
  {
    const std::string_view short_char;
    const std::string_view long_string;
    int* dst_flag;
    int dst_value;
  };

  std::vector<flag_options_struct> _flag_options;
};

#endif // OPTIONS_H