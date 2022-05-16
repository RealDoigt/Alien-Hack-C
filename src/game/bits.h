#ifndef BITS_H
#define BITS_H
#define LEVEL_MAX 8
typedef enum
{
    SHIFT_LEFT,
    SHIFT_RIGHT,
    NOT,
    OR,
    XOR,
    AND
}
operation;

typedef enum
{
    ERROR,
    NORMAL,
    EXPLOSIVE, // if it's sandwiched between two active bits on the vertical axis, those bits and itself become dead except if already dead
    VIRAL, // all adjacent bits become alive on change except if dead
    ZOMBIE, // switch between a dead and alive state on change
    WISE // unnaffected by other bits.
}
bit_type;

typedef struct
{
    bit_type type;
    char is_alive; // can only be 0 or 1
}
Bit;

typedef struct
{
    char height; // number of rows, min 1, max is level max.
    char solution; // display solution in binary form
    char* text_file; // where the level's introduction text is written
    char shifts, ors, xors, ands, nots; // number of ops allowed
    Bit bits[LEVEL_MAX][LEVEL_MAX]; // level
}
Level;

char is_solved(Level level);
void to_binary_string(char solution, char* result);
int do_operation(Level* level, operation op, int row);
void dissolve_row(Level* level, int row);
void apply_effects(Level* level, int row);
Level level_constructor(char* data_file, char* text_file);
#endif
