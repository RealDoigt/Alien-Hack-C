#include "bits.h"
#include "raylib.h"

static char to_real_bits(Level level)
{
    char result = 0;

    // Note: the first bit array is the one used for getting the solution

    /*
        The way this works is that you can imagine this as if I was inserting the bit value in a byte
        by treating it as an array.
    */
    for (int i = 0; i < LEVEL_MAX; ++i) result |= level.bits[0][i].is_alive << i;
    return result;
}

char is_solved(Level level)
{
    return level.solution == to_real_bits(level);
}

void to_binary_string(char solution, char* result)
{
    // expected code on the caller's side: char result[LEVEL_MAX + 1];

    // this takes each bit separately and evaluates its truthiness to determine the value of result[i]
    for (int i = 0; i < LEVEL_MAX; ++i)
        result[LEVEL_MAX - 1 - i] = (char)(solution << (LEVEL_MAX - 1 - i)) >> (LEVEL_MAX - 1) ? '1' : '0';

    result[LEVEL_MAX] = '\0';
}

/*
    returns a negative number if an error was encountered:
    -1      invalid row
    -2      invalid operation
    -11     not enough shifts
    -12     not enough ors
    -13     not enough xors
    -14     not enough ands
    -15     not enough nots

    returns a positive number if the operation is succesfull:
    1       only the row itself was changed
    2       the row was dissolved into the one below
*/
int do_operation(Level* level, operation op, int row)
{
    if (row >= level->height) return -1;

    /*
        Note: The way it works is 'or', 'xor' and 'and' dissolves a row into the one below.
        However, that's not the case for 'shift' and 'not', which only apply to the row itself.
        Also the last true bit evaluated is the one that is kept, if no true bit is found or if not appropriate,
        the last false bit (so the bit on the bottom row).
    */
    switch (op)
    {
        case SHIFT_LEFT:

            if (level->shifts > 0) --level->shifts;
            else return -11;

            for (int i = LEVEL_MAX - 1; i >= 1; --i) level->bits[row][i] = level->bits[row][i - 1];
            level->bits[row][0] = (Bit){NORMAL, 0};
            break;

        case SHIFT_RIGHT:

            if (level->shifts > 0) --level->shifts;
            else return -11;

            for (int i = 0; i < LEVEL_MAX - 1; ++i) level->bits[row][i] = level->bits[row][i + 1];
            level->bits[row][LEVEL_MAX - 1] = (Bit){NORMAL, 0};
            break;

        case OR:

            if (level->ors > 0) --level->ors;
            else return -12;

            for (int i = 0; i < LEVEL_MAX; ++i)
                level->bits[row - 1][i] = level->bits[row][i].is_alive ? level->bits[row][i] : level->bits[row - 1][i];
            break;

        case XOR:

            if (level->xors > 0) --level->xors;
            else return -13;

            // this could've been a very messy ternary...
            for (int i = 0; i < LEVEL_MAX; ++i)
                if (level->bits[row][i].is_alive ^ level->bits[row - 1][i].is_alive)
                    level->bits[row - 1][i] = level->bits[row][i].is_alive ? level->bits[row][i] : level->bits[row - 1][i];
            break;

        case AND:

            if (level->ands > 0) --level->ands;
            else return -14;

            for (int i = 0; i < LEVEL_MAX; ++i)
            {
                if (!(level->bits[row][i].is_alive && level->bits[row - 1][i].is_alive))
                    level->bits[row - 1][i] = !level->bits[row - 1][i].is_alive ? level->bits[row - 1][i] : level->bits[row][i];
            }
            break;

        case NOT:

            if (level->nots > 0) --level->nots;
            else return -15;

            for (int i = 0; i < LEVEL_MAX; ++i)
                level->bits[row][i].is_alive = !level->bits[row][i].is_alive;
            break;

        default: return -2;
    }

    return op < OR ? 1 : 2;
}

void dissolve_row(Level* level, int row)
{
    // I made the decision to add the +1 here so that the code can immediately verify the conditions once
    for (int i = row + 1; i < level->height; ++i)
        for (int j = 0; j < LEVEL_MAX; ++j)
            level->bits[i - 1][j] = level->bits[i][j];

    --level->height;
}

void apply_effects(Level* level, int row)
{
    for (int i = 0; i < LEVEL_MAX; ++i)
        switch (level->bits[row][i].type)
        {
            case EXPLOSIVE:

                if (level->bits[row][i].is_alive && i - 1 >= 0 && level->bits[row][i - 1].is_alive && i + 1 < LEVEL_MAX && level->bits[row][i + 1].is_alive)
                {
                    level->bits[row][i + 1].is_alive = 0;
                    level->bits[row][i - 1].is_alive = 0;
                    level->bits[row][i].is_alive = 0;
                }
                break;

            case VIRAL:

                if (level->bits[row][i].is_alive)
                {
                    if (row + 1 < LEVEL_MAX) level->bits[row + 1][i].is_alive = 1;
                    if (row - 1 >= 0) level->bits[row - 1][i].is_alive = 1;

                    if (i + 1 < LEVEL_MAX) level->bits[row][i + 1].is_alive = 1;
                    if (i - 1 >= 0) level->bits[row][i - 1].is_alive = 1;
                }
                break;

            case ZOMBIE:

                level->bits[row][i].is_alive = !level->bits[row][i].is_alive;
                break;

            case WISE:

                level->bits[row][i].is_alive = (level->solution << (LEVEL_MAX - i)) >> (LEVEL_MAX - 1);
                break;

            default: break;
        }
}

static Bit bit_constructor(bit_type type, char is_alive)
{
    Bit result;

    if (type >= NORMAL && type <= WISE) result.type = type;
    else return (Bit){ERROR, -1};

    result.is_alive = is_alive;
    return result;
}

Level level_constructor(char* data_file, char* text_file)
{
    Level result;
    result.text_file = text_file;

    if (FileExists(data_file))
    {
        unsigned int bytes;
        unsigned char* data = LoadFileData(data_file, &bytes);

        if (bytes >= 15) // smallest number of bytes possible for a level
        {
            result.height = data[0];

            if (result.height > 0 && result.height < LEVEL_MAX)
            {
                result.solution = data[1];
                result.shifts   = data[2];
                result.ors      = data[3];
                result.xors     = data[4];
                result.ands     = data[5];
                result.nots     = data[6];

                /*
                    Note regarding the bit struct: one's max value is 5 or 101 and the other is 1 or... 1
                    So I decided to cram both bytes into one where the first 3 bits are the bit type and
                    the last one is the is_alive bit.
                */
                for (int y = 0; y < result.height; ++y)
                {
                    for (int x = 0; x < LEVEL_MAX; ++x)
                    {
                        // live bit is last bit ->  0000 000X
                        // type tribit is in the first four bits -> 0000 XXX0
                        Bit bit = bit_constructor(data[y * LEVEL_MAX + x + 7] >> 1, data[y * LEVEL_MAX + x + 7] & 1);

                        if (bit.type == ERROR)
                        {
                            // ERROR: Invalid bit
                            result.height = -4;
                            break;
                        }

                        result.bits[y][x] = bit;
                    }

                    if (result.height == -4) break;
                }
            }

            // ERROR: Invalid height
            else result.height = -3;
        }

        // ERROR: File too small
        else result.height = -1;

        UnloadFileData(data);
    }

    // ERROR: File not found
    else result.height = -1;

    return result;
}
