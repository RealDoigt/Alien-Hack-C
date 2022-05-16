#ifndef DRAW_H
#define DRAW_H
#include "bits.h"
#include "raylib.h"

typedef struct
{
    Vector2 center;
    float radius;
}
Circle;

typedef struct
{
    Vector2 v1;
    Vector2 v2;
    Vector2 v3;
}
Triangle;

typedef struct
{
    int row;
    char is_clicked;
    operation type;

    union
    {
        Rectangle rectangle;
        Triangle triangle;
        Circle circle;
    };
}
Button;

Button button_constructor(operation type, int row);
char pointer_hovers(const Button* button);
void draw_button(Button* button);
void draw_level(Level level);
void init_button_images();
void free_button_images();
#endif
