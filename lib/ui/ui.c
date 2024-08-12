#include "ui/ui.h"

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

#include "array/array.h"

static bool ui_should_draw(const UI_Base* base) {
    return base->texture.id > 0 && base->flags & UI_FLAG_ACTIVE;
}

bool ui_onclick_recursive(UI_Base* this,
                          int posx,
                          int posy,
                          int tarx,
                          int tary) {
    int x = this->x + posx;
    int y = this->y + posy;
    int w = this->texture.width;
    int h = this->texture.height;

    for (int i = 0; i < array_length(this->children); i++) {
        UI_Base* child = array_get(this->children, i);

        if (ui_onclick_recursive(child, x, y, tarx, tary)) {
            return true;
        }
    }

    Rectangle bounds = {x, y, w, h};
    if (CheckCollisionPointRec((Vector2){tarx, tary}, bounds)) {
        UI_Button* button = (void*)this;
        if (button->base.flags & UI_FLAG_CLICKABLE && button->onclick) {
            button->onclick(this, this->userdata);
        }

        return true;
    }

    return false;
}

bool ui_onclick(UI_Base* this, int x, int y) {
    return ui_onclick_recursive(this, 0, 0, x, y);
}

static void ui_draw_component(const UI_Base* this, int x, int y) {
    float posx = this->x + x;
    float posy = this->y + y;
    if (ui_should_draw(this)) {
        int width  = this->texture.width;
        int height = this->texture.height;
        DrawRectangle(posx, posy, width, height, WHITE);
    }

    for (int i = 0; i < array_length(this->children); i++) {
        const UI_Base* child = array_get(this->children, i);
        if (child != NULL) {
            ui_draw_component(child, posx, posy);
        }
    }
}

void ui_draw(const UI_Base* this) {
    ui_draw_component(this, 0, 0);
}

void ui_addchild(UI_Base* parent, UI_Base* child) {
    array_push(parent->children, child);
}

UI_Base* ui_new(UI_Type type) {
    UI_Base* result = NULL;
    switch (type) {
        case UI_TYPE_BASE:
            result        = calloc(1, sizeof(UI_Base));
            result->flags = UI_FLAG_ACTIVE;
            break;
        case UI_TYPE_BUTTON:
            result        = calloc(1, sizeof(UI_Button));
            result->flags = UI_FLAG_CLICKABLE | UI_FLAG_ACTIVE;
            break;
    }

    result->type     = type;
    result->children = array_new();

    return result;
}
