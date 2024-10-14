#ifndef GAME_STATE_H
#define GAME_STATE_H

// Positions are 8-bit integers with 4 bits each for y, x.
enum
{

  // Number to represent "not found"
  INVALID_POS = 0xff,

  // Maximum # of colors in a puzzle
  MAX_COLORS = 16,

  // Maximum valid size of a puzzle
  MAX_SIZE = 15,

  // Maximum # cells in a valid puzzle -- since we just use bit
  // shifting to do x/y, need to allocate space for 1 unused column.
  MAX_CELLS = (MAX_SIZE + 1) * MAX_SIZE - 1,

  // One million(ish) bytes
  MEGABYTE = 1024 * 1024,

};

class GameState
{
public:
  // Number to represent "not found"
  static constexpr uint8_t INVALID_POS = 0xff;

  // Maximum # of colors in a puzzle
  static constexpr uint8_t MAX_COLORS = 16;

  // Maximum valid size of a puzzle
  static constexpr uint8_t MAX_SIZE = 15;

  // Maximum # cells in a valid puzzle -- since we just use bit
  // shifting to do x/y, need to allocate space for 1 unused column.
  static constexpr uint8_t MAX_CELLS = (MAX_SIZE + 1) * MAX_SIZE - 1;

  // One million(ish) bytes
  static constexpr int MEGABYTE = 1024 * 1024;
};

// Represent the contents of a cell on the game board
typedef uint8_t cell_t;

// Represent a position within the game board
typedef uint8_t pos_t;

// Static information about a puzzle layout -- anything that does not
// change as the puzzle is solved is stored here.
typedef struct game_info_struct
{

  // Index in color_dict table of codes
  int color_ids[MAX_COLORS];

  // Color order
  int color_order[MAX_COLORS];

  // Initial and goal positions
  pos_t init_pos[MAX_COLORS];
  pos_t goal_pos[MAX_COLORS];

  // Length/width of game board
  size_t size;

  // Number of colors present
  size_t num_colors;

  // Color table for looking up color ID
  uint8_t color_tbl[128];

  // Was user order specified?
  int user_order;

} game_info_t;

// Incremental game state structure for solving -- this is what gets
// written as the search progresses, one state per search node
typedef struct game_state_struct
{

  // State of each cell in the world; a little wasteful to duplicate,
  // since only one changes on each move, but necessary for BFS or A*
  // (would not be needed for depth-first search).
  cell_t cells[MAX_CELLS];

  // Head position
  pos_t pos[MAX_COLORS];

  // How many free cells?
  uint8_t num_free;

  // Which was the last color / endpoint
  uint8_t last_color;

  // Bitflag indicating whether each color has been completed or not
  // (cur_pos is adjacent to goal_pos).
  uint16_t completed;

} game_state_t;

// Search node for A* / BFS.
typedef struct tree_node_struct
{
  game_state_t state;              // Current game state
  uint64_t cost_to_come;           // Cost to come (ignored for BFS)
  uint64_t cost_to_go;             // Heuristic cost (ignored for BFS)
  struct tree_node_struct* parent; // Parent of this node (may be NULL)
} tree_node_t;

// Strategy is to pre-allocate a big block of nodes in advance, and
// hand them out in order.
typedef struct node_storage_struct
{
  tree_node_t* start; // Allocated block
  size_t capacity;    // How many nodes did we allocate?
  size_t count;       // How many did we give out?
} node_storage_t;

#endif // GAME_STATE_H
