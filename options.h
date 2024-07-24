#ifndef OPTIONS_H
#define OPTIONS_H

// Options for this program
typedef struct options_struct {

  int    display_quiet;
  int    display_diagnostics;
  int    display_animate;
  int    display_color;
  int    display_fast;
  int    display_save_svg;
  
  int    node_check_touch;
  int    node_check_stranded;
  int    node_check_deadends;
  int    node_bottleneck_limit;
  int    node_penalize_exploration;
  
  int    order_autosort_colors;
  int    order_most_constrained;
  int    order_forced_first;
  int    order_random;
  
  int    search_best_first;
  int    search_outside_in;
  size_t search_max_nodes;
  double search_max_mb;
  int    search_fast_forward;
  
} options_t;

#endif // OPTIONS_H