#include "raylib.h"
#include "game/bits.h"
#include "game/draw.h"

int main(void)
{
    const int screenWidth = 720;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "The Alien Hack");

    SetTargetFPS(25);

    Level level = level_constructor("level_1.dat", "nothing.txt");
    init_button_images();

    Button or_button = button_constructor(OR, 0);
    Button xor_button = button_constructor(XOR, 0);
    Button and_button = button_constructor(AND, 0);
    Button shiftl_button = button_constructor(SHIFT_LEFT, 0);
    Button shiftr_button = button_constructor(SHIFT_RIGHT, 0);
    Button not_button = button_constructor(NOT, 0);

    char solution_text[LEVEL_MAX + 1];
    to_binary_string(level.solution, solution_text);

    if (level.height > 0)
        while (!WindowShouldClose())
        {
            BeginDrawing();
            ClearBackground(BLACK);

            if (IsKeyDown(KEY_LEFT_ALT) && IsKeyReleased(KEY_ENTER))
                ToggleFullscreen();

            draw_button(&or_button);
            draw_button(&xor_button);
            draw_button(&and_button);
            draw_button(&shiftl_button);
            draw_button(&shiftr_button);

            draw_button(&not_button);

            draw_level(level);

            DrawText(solution_text, 145, level.height * 115, 50, WHITE);

            EndDrawing();
        }

    free_button_images();
    CloseWindow();
    return 0;
}
