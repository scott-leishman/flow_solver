#ifndef BOARD_H
#define BOARD_H

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "game_state.h"

class Board
{
public:
  Board();
  ~Board();

  int game_read(const char* filename, game_info_t* info, game_state_t* state);
  int game_read_hint(const game_info_t* info,
                     const game_state_t* state,
                     const char* filename);

private:
};

#endif // BOARD_H