#include "bits.h"
#include "raylib.h"
#include "draw.h"

#define START_X 100
#define START_Y 50
#define BIT_HEIGHT 60
#define BIT_WIDTH 20

#define SCALE_FACTOR .60

static Texture2D button_images[4];

void init_button_images()
{
    button_images[0] = LoadTexture("xor_is_clicked.png");
    button_images[1] = LoadTexture("xor.png");

    button_images[2] = LoadTexture("and_is_clicked.png");
    button_images[3] = LoadTexture("and.png");
}

void free_button_images()
{
    UnloadTexture(button_images[0]);
    UnloadTexture(button_images[1]);
    UnloadTexture(button_images[2]);
    UnloadTexture(button_images[3]);
}

void draw_level(Level level)
{
    for (int y = level.height - 1; y >= 0; --y)
    {
        for (int x = 0; x < LEVEL_MAX; ++x)
        {
            Color color;

            switch (level.bits[y][x].type)
            {
                case NORMAL:
                    color = GREEN;
                    break;

                case EXPLOSIVE:
                    color = RED;
                    break;

                case VIRAL:
                    color = PURPLE;
                    break;

                case ZOMBIE:
                    color = YELLOW;
                    break;

                case WISE:
                    color = WHITE;
                    break;

                default: return;
            }

            if (!level.bits[y][x].is_alive)
            {
                color.r >>= 3;
                color.g >>= 3;
                color.b >>= 3;
            }

            DrawRectangle(START_X + (x << 5), START_Y + y * 100, BIT_WIDTH, BIT_HEIGHT, color);
        }
    }
}

static void draw_shift_button(Button* button)
{
    int x_extra_padding = button->type == SHIFT_LEFT ? START_X + LEVEL_MAX * 58 : 20;

    Vector2 v1 = (Vector2){(START_X - BIT_WIDTH + x_extra_padding) * SCALE_FACTOR, START_Y + button->row * 100};

    Vector2 v2 = (Vector2){(START_X - BIT_WIDTH + x_extra_padding) * SCALE_FACTOR, START_Y + button->row * 100 + BIT_HEIGHT};

    Vector2 v3 = (Vector2){(START_X + BIT_WIDTH + x_extra_padding - 10) * SCALE_FACTOR, START_Y + button->row * 100 + BIT_HEIGHT / 2};

    if (button->type == SHIFT_LEFT) v3.x -= BIT_WIDTH << 1;

    if (button->is_clicked) DrawTriangle(v1, v2, v3, WHITE);
    else DrawTriangleLines(v1, v2, v3, WHITE);

    button->triangle.v1 = v1;
    button->triangle.v2 = v2;
    button->triangle.v3 = v3;
}

static void draw_not_button(Button* button)
{
    button->rectangle.x = (float)((START_X << 1) + BIT_WIDTH * LEVEL_MAX + 40);
    button->rectangle.y = (float)(START_Y + button->row);
    button->rectangle.width = (float)BIT_WIDTH / 3;
    button->rectangle.height = (float)BIT_HEIGHT + 5;

    if (button->is_clicked)
    {
        DrawRectangleRec(button->rectangle, WHITE);
        DrawCircle(button->rectangle.x + 7, START_Y + 53 + button->row, 7, WHITE);
    }

    else
    {
        DrawRectangleLines(button->rectangle.x, button->rectangle.y, button->rectangle.width * 2, button->rectangle.height - 25, WHITE);

        // radius 10 -> 20/2
        DrawCircleLines(button->rectangle.x + 7, START_Y + 53, 7, WHITE);
    }
}

static void draw_or_button(Button* button)
{
    button->circle.center = (Vector2){START_X + 125, START_Y + button->row * 100 + 80};
    button->circle.radius = 10;

    if (button->is_clicked)
    {
        DrawCircleV(button->circle.center, button->circle.radius, WHITE);
        DrawCircleV(button->circle.center, button->circle.radius * .16, BLACK);
    }

    else
    {
        DrawCircleLines(button->circle.center.x, button->circle.center.y, button->circle.radius, WHITE);
        DrawCircleLines(button->circle.center.x, button->circle.center.y, button->circle.radius * .16, WHITE);
    }
}

static void draw_image_button(Button* button, int index_start)
{
    button->rectangle.y = START_Y + button->row * 100 + 70;

    if (button->is_clicked)
        DrawTexture(button_images[index_start], button->rectangle.x, button->rectangle.y, WHITE);

    else
        DrawTexture(button_images[index_start + 1], button->rectangle.x, button->rectangle.y, WHITE);
}

static void draw_xor_button(Button* button)
{
    button->rectangle.x = START_X + 90;

    button->rectangle.width = button_images[0].width;
    button->rectangle.height = button_images[0].height;

    draw_image_button(button, 0);
}

static void draw_and_button(Button* button)
{
    button->rectangle.x = START_X + 140;

    button->rectangle.width = button_images[2].width;
    button->rectangle.height = button_images[2].height;

    draw_image_button(button, 2);
}

void draw_button(Button* button)
{
    switch (button->type)
    {
        case NOT:
            draw_not_button(button);
            break;

        case OR:
            draw_or_button(button);
            break;

        case XOR:
            draw_xor_button(button);
            break;

        case AND:
            draw_and_button(button);
            break;

        default:
            draw_shift_button(button);
            break;
    }
}

Button button_constructor(operation type, int row)
{
    Button result;

    switch (type)
    {
        case SHIFT_LEFT:
        case SHIFT_RIGHT:
            result.triangle = (Triangle){(Vector2){0, 0}, (Vector2){0, 0}, (Vector2){0, 0}};
            break;

        case OR:
            result.circle = (Circle){(Vector2){0, 0}, 0};
            break;

        default:

            result.rectangle.x = 0;
            result.rectangle.y = 0;
            result.rectangle.width = 0;
            result.rectangle.height = 0;
            break;
    }

    result.is_clicked = 0;
    result.type = type;
    result.row = row;
    return result;
}

char pointer_hovers(const Button* button)
{
    Vector2 pointer = GetMousePosition();

    switch (button->type)
    {
        case SHIFT_LEFT:
        case SHIFT_RIGHT:
            return CheckCollisionPointTriangle(pointer, button->triangle.v1, button->triangle.v2, button->triangle.v3);

        case OR:
            return CheckCollisionPointCircle(pointer, button->circle.center, button->circle.radius);

        default:
            return CheckCollisionPointRec(pointer, button->rectangle);
    }
}
