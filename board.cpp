#include <cassert>
#include <filesystem>

#include "board.h"

#include "utest.h"

//////////////////////////////////////////////////////////////////////
// Helper function for below.

int
detect_format(FILE* fp)
{

  int max_letter = 'A';
  int c;

  while ((c = fgetc(fp)) != EOF) {
    if (isalpha(c) && c > max_letter) {
      max_letter = c;
    }
  }

  rewind(fp);

  return (max_letter - 'A') < MAX_COLORS;
}

UTEST(Board, detect_format)
{
  namespace fs = std::filesystem;
  std::string path = "./puzzles"; // Path to the "puzzles" sub-directory

  // Iterate over all .txt files in the "puzzles" directory
  for (const auto& entry : fs::directory_iterator(path)) {
    if (entry.is_regular_file() && entry.path().extension() == ".txt") {
      FILE* file = std::fopen(entry.path().c_str(), "r");
      if (file) {
        int result = detect_format(file);
        std::cout << "File: " << entry.path().filename()
                  << " - detect_format() returned: " << result << std::endl;
        std::fclose(file);
      } else {
        std::cerr << "Error opening file: " << entry.path().filename()
                  << std::endl;
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////
// Read game board from text file

int
game_read(const char* filename, game_info_t* info, game_state_t* state)
{
  /*
    FILE* fp = fopen(filename, "r");

    if (!fp) {
      fprintf(stderr, "error opening %s\n", filename);
      return 0;
    }

    int is_alternate_format = detect_format(fp);

    memset(info, 0, sizeof(game_info_t));
    memset(state, 0, sizeof(game_state_t));

    memset(state->pos, 0xff, sizeof(state->pos));

    state->last_color = MAX_COLORS;

    size_t y = 0;

    char buf[MAX_SIZE + 2];

    memset(info->color_tbl, 0xff, sizeof(info->color_tbl));
    memset(info->init_pos, 0xff, sizeof(info->init_pos));
    memset(info->goal_pos, 0xff, sizeof(info->goal_pos));

    while (info->size == 0 || y < info->size) {

      char* s = fgets(buf, MAX_SIZE + 1, fp);
      size_t l = s ? strlen(s) : 0;

      if (!s) {
        fprintf(stderr, "%s:%zu: unexpected EOF\n", filename, y + 1);
        fclose(fp);
        return 0;
      } else if (s[l - 1] != '\n') {
        fprintf(stderr, "%s:%zu line too long\n", filename, y + 1);
        fclose(fp);
        return 0;
      }

      if (l >= 2 && s[l - 2] == '\r') { // DOS line endings
        --l;
      }

      if (info->size == 0) {
        if (l < 3) {
          fprintf(stderr,
                  "%s:1: expected at least 3 characters before newline\n",
                  filename);
          fclose(fp);
          return 0;
        } else if (l - 1 > MAX_SIZE) {
          fprintf(stderr, "%s:1: size too big!\n", filename);
          fclose(fp);
          return 0;
        }
        info->size = l - 1;
      } else if (l != info->size + 1) {
        fprintf(stderr,
                "%s:%zu: wrong number of characters before newline "
                "(expected %zu, but got %zu)\n",
                filename,
                y + 1,
                info->size,
                l - 1);
        fclose(fp);
        return 0;
      }

      for (size_t x = 0; x < info->size; ++x) {

        uint8_t c = s[x];

        if (isalpha(c)) {

          pos_t pos = pos_from_coords(x, y);
          assert(pos < MAX_CELLS);

          int color = info->color_tbl[c];

          if (color >= info->num_colors) {

            color = info->num_colors;

            if (info->num_colors == MAX_COLORS) {
              fprintf(stderr,
                      "%s:%zu: can't use color %c"
                      "- too many colors!\n",
                      filename,
                      y + 1,
                      c);
              fclose(fp);
              return 0;
            }

            int id = is_alternate_format ? (c - 'A') : get_color_id(c);
            if (id < 0 || id >= MAX_COLORS) {
              fprintf(
                stderr, "%s:%zu: unrecognized color %c\n", filename, y + 1, c);
              fclose(fp);
              return 0;
            }

            info->color_ids[color] = id;
            info->color_order[color] = color;

            ++info->num_colors;
            info->color_tbl[c] = color;
            info->init_pos[color] = state->pos[color] = pos;
            state->cells[pos] = cell_create(TYPE_INIT, color, 0);

          } else {

            if (info->goal_pos[color] != INVALID_POS) {
              fprintf(
                stderr, "%s:%zu too many %c already!\n", filename, y + 1, c);
              fclose(fp);
              return 0;
            }
            info->goal_pos[color] = pos;
            state->cells[pos] = cell_create(TYPE_GOAL, color, 0);
          }

        } else {

          ++state->num_free;
        }
      }

      ++y;
    }

    fclose(fp);

    if (!info->num_colors) {
      fprintf(stderr, "empty map!\n");
      return 0;
    }

    for (size_t color = 0; color < info->num_colors; ++color) {

      if (info->goal_pos[color] == INVALID_POS) {
        game_print(info, state);
        fprintf(stderr,
                "\n\n%s: color %s has start but no end\n",
                filename,
                color_name_str(info, color));
        return 0;
      }

      if (g_options.search_outside_in) {

        int init_dist = pos_get_wall_dist(info, info->init_pos[color]);
        int goal_dist = pos_get_wall_dist(info, info->goal_pos[color]);

        if (goal_dist < init_dist) {
          pos_t tmp_pos = info->init_pos[color];
          info->init_pos[color] = info->goal_pos[color];
          info->goal_pos[color] = tmp_pos;
          state->cells[info->init_pos[color]] = cell_create(TYPE_INIT, color,
    0); state->cells[info->goal_pos[color]] = cell_create(TYPE_GOAL, color, 0);
          state->pos[color] = info->init_pos[color];
        }
      }
    }
  */
  return 1;
}

//////////////////////////////////////////////////////////////////////
// Read hint file

int
game_read_hint(const game_info_t* info,
               const game_state_t* state,
               const char* filename,
               uint8_t hint[MAX_CELLS])
{
  /*
    memset(hint, 0xff, MAX_CELLS);

    FILE* fp = fopen(filename, "r");

    if (!fp) {
      fprintf(stderr, "error opening %s\n", filename);
      return 0;
    }

    char buf[MAX_SIZE + 2];

    for (size_t y = 0; y < info->size; ++y) {
      char* s = fgets(buf, info->size + 2, fp);
      if (!s) {
        break;
      }
      size_t l = strlen(s);
      if (l > info->size + 1 || s[l - 1] != '\n') {
        fprintf(stderr, "%s:%zu: line too long!\n", filename, y + 1);
        fclose(fp);
        return 0;
      }
      for (size_t x = 0; x < l - 1; ++x) {
        uint8_t c = buf[x];
        if (isalpha(c)) {
          int color = c > 127 ? 0xff : info->color_tbl[c];
          if (color >= info->num_colors) {
            fprintf(stderr, "%s:%zu: color %c not found!\n", filename, y + 1,
    c); fclose(fp); return 0;
          }
          int pos = pos_from_coords(x, y);
          hint[pos] = color;
        }
      }
    }

    fclose(fp);
    */
  return 1;
}
