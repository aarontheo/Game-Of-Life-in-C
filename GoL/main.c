#include <stdio.h>
#include <time.h>

#define BOARD_WIDTH 64
#define BOARD_HEIGHT 32
#define TIME_STEP 0.1

#define ALIVE 1
#define DEAD 0
#define P(x, y)                                                                \
  ((Pos){x, y}) // This is a shortcut to make a Pos, I decided on this macro
                // instead of removing the struct altogether.
#define len(arr)                                                               \
  (sizeof(arr) / sizeof(0 [arr])) // Doing 0[arr] vs. arr[0] is apparently
                                  // better for safety reasons

typedef int Board[BOARD_HEIGHT][BOARD_WIDTH];
typedef struct {
  int x;
  int y;
} Pos;

void sleep(float seconds) {
  clock_t endwait;
  endwait = clock() + seconds * CLOCKS_PER_SEC;
  while (clock() < endwait) {
  }
}

// Turns out that C has a janky modulus operator that behaves oddly with
// negative numbers.
int positive_mod(int a, int b) { return ((a % b) + b) % b; }

// Wraps a Pos to fit within the bounds of BOARD_WIDTH and BOARD_HEIGHT.
Pos wrap_pos(Pos pos) {
  return (Pos){positive_mod(pos.x, BOARD_WIDTH),
               positive_mod(pos.y, BOARD_HEIGHT)};
}

// This returns a cell for any coordinate.
int get_cell(Board board, Pos pos) {
  pos = wrap_pos(pos);
  return board[pos.y][pos.x];
}

// This sets the state of a cell for any coordinate.
void set_cell(Board board, Pos pos, int state) {
  pos = wrap_pos(pos);
  board[pos.y][pos.x] = state;
}

// Returns the sum of the states of cells in the 8-neighborhood.
// If working with only booleans, (1 or 0), this is the same as returning the
// number of living neighbors.
int neighbors_sum(Board board, Pos pos) {
  int sum = 0;
  for (int x = -1; x <= 1; x++) {
    for (int y = -1; y <= 1; y++) {
      if (x == 0 && y == 0) {
        continue;
      }
      sum += get_cell(board, P(pos.x + x, pos.y + y));
    }
  }
  return sum;
}

// Returns cell behavior for Conway's Game of Life rule.
//
// The rules for Conway's Game of Life are:
// Birth: A dead cell becomes alive if it has exactly three live neighbors
// Death by isolation: A live cell dies if it has one or fewer live neighbors
// Death by overcrowding: A live cell dies if it has four or more live neighbors
// Survival: A live cell survives if it has two or three live neighbors
int rule_GOL(int state, int living_neighbors) {
  // printf("Living neighbors: %d\n", living_neighbors);
  switch (state) {
  case ALIVE:
    if (living_neighbors <= 1 || living_neighbors >= 4) {
      return DEAD;
    }
    return ALIVE;
  case DEAD:
    if (living_neighbors == 3) {
      return ALIVE;
    }
  }
  return state; // If we don't know how to evaluate the state, just leave it.
}

// Returns whether the next state of a cell is alive or dead.
// Does this by counting the living cells around it
int get_next_state(Board board, Pos pos) {
  return rule_GOL(get_cell(board, pos), neighbors_sum(board, pos));
}

void empty_board(Board board) {
  for (int row = 0; row < BOARD_HEIGHT; row++) {
    for (int col = 0; col < BOARD_WIDTH; col++) {
      set_cell(board, P(col, row), DEAD);
    }
  }
}

void copy_board(Board from, Board to) {
  for (int row = 0; row < BOARD_HEIGHT; row++) {
    for (int col = 0; col < BOARD_WIDTH; col++) {
      to[row][col] = from[row][col];
    }
  }
}

// Cannot return array types; they are second-class citizens in C.
// Therefore, this function needs to modify a buffer Board.
// Once calculations are run, the board is overwritten with the buffer.
void advance_board(Board board, Board buffer) {
  for (int x = 0; x < BOARD_WIDTH; x++) {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      int next_state = get_next_state(board, P(x, y));
      set_cell(buffer, P(x, y), next_state);
    }
  }
  copy_board(buffer, board);
}

int min(int a, int b) { return (a < b) ? a : b; }

int max(int a, int b) { return (a > b) ? a : b; }

#define SYMBOL_ALIVE 'O'
#define SYMBOL_DEAD ' '
#define SYMBOL_BORDER '#'
// Prints a board to the terminal.
// Uses the defined symbols for alive and dead.
// I considered adding a char array parameter to define a char for each state,
// but this is easier and I don't need it at the moment. May add later.
void display_board(Board board) {
  // TODO: maybe add a border?
  for (int i = 0; i < BOARD_WIDTH + 2; i++) {
    printf("%c", SYMBOL_BORDER);
  }
  printf("\n");
  for (int row = 0; row < BOARD_HEIGHT; row++) {
    printf("%c", SYMBOL_BORDER);
    for (int col = 0; col < BOARD_WIDTH; col++) {
      printf("%c", (get_cell(board, P(col, row)) == ALIVE) ? SYMBOL_ALIVE
                                                           : SYMBOL_DEAD);
    }
    printf("%c", SYMBOL_BORDER);
    printf("\n");
  }
  for (int i = 0; i < BOARD_WIDTH + 2; i++) {
    printf("%c", SYMBOL_BORDER);
  }
  printf("\n");
}

void place_cell(Board board, Pos pos) { set_cell(board, pos, ALIVE); }

void place_r_pentomino(Board board, Pos offset) {
  place_cell(board, P(1 + offset.x, 0 + offset.y));
  place_cell(board, P(2 + offset.x, 0 + offset.y));
  place_cell(board, P(0 + offset.x, 1 + offset.y));
  place_cell(board, P(1 + offset.x, 1 + offset.y));
  place_cell(board, P(1 + offset.x, 2 + offset.y));
}

void place_square(Board board, Pos offset) {
  place_cell(board, P(0 + offset.x, 0 + offset.y));
  place_cell(board, P(1 + offset.x, 0 + offset.y));
  place_cell(board, P(0 + offset.x, 1 + offset.y));
  place_cell(board, P(1 + offset.x, 1 + offset.y));
}

void place_blinker(Board board, Pos offset) {
  place_cell(board, P(0 + offset.x, 0 + offset.y));
  place_cell(board, P(0 + offset.x, 1 + offset.y));
  place_cell(board, P(0 + offset.x, 2 + offset.y));
}

void place_glider(Board board, Pos offset) {
	place_cell(board, P(1 + offset.x, 0 + offset.y));
	place_cell(board, P(2 + offset.x, 1 + offset.y));
	place_cell(board, P(0 + offset.x, 2 + offset.y));
	place_cell(board, P(1 + offset.x, 2 + offset.y));
	place_cell(board, P(2 + offset.x, 2 + offset.y));
}

int main(void) {
  // Initialize the board
  Board board;
  Board buffer;
  empty_board(board);
  empty_board(buffer);
  place_r_pentomino(board, P(BOARD_WIDTH / 2, BOARD_HEIGHT / 2));
  // place_blinker(board, P(4, 4));
  // place_glider(board, P(4, 4));


  // simulation loop
  while (1) { // Until we care to stop the simulation
    display_board(board);
    advance_board(board, buffer);
    sleep(TIME_STEP);
  }

  return 0;
}
