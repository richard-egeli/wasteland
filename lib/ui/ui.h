#ifndef LIB_UI_UI_H_
#define LIB_UI_UI_H_

#include <array/array.h>
#include <raylib.h>

typedef enum UI_Type {
    UI_TYPE_BASE,
    UI_TYPE_BUTTON,
} UI_Type;

typedef enum UI_Flag {
    UI_FLAG_NONE      = 0x00,
    UI_FLAG_ACTIVE    = 0x01,
    UI_FLAG_CLICKABLE = 0x02,
} UI_Flag;

typedef enum UI_Anchor {
    UI_ANCHOR_TOPLEFT,
    UI_ANCHOR_TOPRIGHT,
    UI_ANCHOR_BOTTOMRIGHT,
    UI_ANCHOR_BOTTOMLEFT,
    UI_ANCHOR_TOP,
    UI_ANCHOR_RIGHT,
    UI_ANCHOR_BOTTOM,
    UI_ANCHOR_LEFT,
    UI_ANCHOR_CENTER,
} UI_Anchor;

typedef struct UI_Base {
    UI_Type type;
    UI_Flag flags;
    UI_Anchor self_anchor;
    UI_Anchor child_anchor;
    float x;
    float y;

    Texture texture;
    Array* children;
    void* userdata;

    struct UI_Base* up;
    struct UI_Base* right;
    struct UI_Base* down;
    struct UI_Base* left;
} UI_Base;

typedef struct UI_Button {
    UI_Base base;
    void (*onclick)(UI_Base* base, void* userdata);
} UI_Button;

bool ui_onclick(UI_Base* base, int x, int y);

void ui_draw(const UI_Base* base);

void ui_addchild(UI_Base* parent, UI_Base* child);

UI_Base* ui_new(UI_Type type);

#endif  // LIB_UI_UI_H_
