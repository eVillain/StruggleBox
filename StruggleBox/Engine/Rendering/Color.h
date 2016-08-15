#ifndef COLOR_H
#define COLOR_H

// TODO: Clean up gfx includes, this shouldnt include glew directly
#include <GL/glew.h>

// This structure is used for OpenGL colors with alpha channel
typedef struct Color {
	GLfloat r, g, b, a;
    inline bool operator==(const Color& rhs)
    { return (r==rhs.r&&g==rhs.g&&b==rhs.b&&a==rhs.a); }
    inline bool operator!=(const Color& rhs)
    { return (r!=rhs.r||g!=rhs.g||b!=rhs.b||a!=rhs.a); }
    inline Color& operator*=(const float& rhs)
    {
        r *= rhs;
        g *= rhs;
        b *= rhs;
        return *this;
    }
} Color;

static inline Color RGBAByteColor(GLuint r, GLuint g, GLuint b, GLuint a){
	Color color = {r/255.0f, g/255.0f, b/255.0f, a/255.0f};
	return color;
}
static inline Color RGBAColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a){
	Color color = {r, g, b, a};
	return color;
}

static inline Color LAColor(GLfloat l, GLfloat a){
	Color color = {l, l, l, a};
	return color;
}

/* --- PRIMARY COLORS --- */
const Color COLOR_NONE = {0.0f, 0.0f, 0.0f, 0.0f};
const Color COLOR_BLACK = {0.0f, 0.0f, 0.0f, 1.0f};
const Color COLOR_WHITE = {1.0f, 1.0f, 1.0f, 1.0f};
const Color COLOR_GREY = {0.5f, 0.5f, 0.5f, 1.0f};

const Color COLOR_RED = {1.0f, 0.0f, 0.0f, 1.0f};
const Color COLOR_GREEN = {0.0f, 1.0f, 0.0f, 1.0f};
const Color COLOR_BLUE = {0.0f, 0.0f, 1.0f, 1.0f};
const Color COLOR_YELLOW = {1.0f, 1.0f, 0.0f, 1.0f};
const Color COLOR_PURPLE = {1.0f, 0.0f, 1.0f, 1.0f};
const Color COLOR_CYAN = {0.0f, 1.0f, 1.0f, 1.0f};

const Color COLOR_ORANGE = {1.0f,0.5f,0.0f,1.0f};
const Color COLOR_BROWN = {0.59f, 0.29f, 0.0f, 1.0f};

/* --- GUI COLORS --- */
const Color COLOR_UI_TEXT = {0.5f,0.5f,0.5f,1.0f};
const Color COLOR_UI_TEXT_ACTIVE = {0.5f,0.5f,0.8f,1.0f};
const Color COLOR_UI_TEXT_HIGHLIGHT = {0.8f,0.8f,1.0f,1.0f};
const Color COLOR_UI_TEXT_INACTIVE = {0.4f,0.4f,0.6f,1.0f};

const Color COLOR_UI_BORDER_OUTER = {0.0f,0.0f,0.0f,1.0f};
const Color COLOR_UI_BORDER_INNER = {1.0f,1.0f,1.0f,0.25f};
const Color COLOR_UI_GRADIENT_TOP = {0.3125f,0.3125f,0.3125f,1.0f};
const Color COLOR_UI_GRADIENT_BOTTOM = {0.25f,0.25f,0.25f,1.0f};

/* --- OTHER RANDOM COLORS --- */
const Color COLOR_FOG_DEFAULT = {0.5f, 0.6f, 0.8f, 1.0f};

const Color COLOR_BLOCK_LEAVES = {0.05f, 0.6f, 0.1f, 0.8f};
const Color COLOR_BLOCK_LEAVES_JUNGLE = {0.25f, 0.7f, 0.2f, 0.8f};

const Color COLOR_LINE = {0.133f, 0.133f, 0.133f, 1.0f};

const Color COLOR_STATIC_LINE = {0.086f, 0.043f, 0.043f, 1.0f};
const Color COLOR_STATIC_DARK = {0.26f, 0.22f, 0.17f, 1.0f};
const Color COLOR_STATIC_MEDIUM = {0.35f, 0.30f, 0.26f, 1.0f};
const Color COLOR_STATIC_LIGHT = {0.39f, 0.35f, 0.30f, 1.0f};
const Color COLOR_STATIC_BRIGHT = {0.67f, 0.73f, 0.8f, 1.0f};

const Color COLOR_STATIC_LINE_VIBRANT = {0.13f, 0.043f, 0.0f, 1.0f};
const Color COLOR_STATIC_DARK_VIBRANT = {0.39f, 0.26f, 0.13f, 1.0f};
const Color COLOR_STATIC_MEDIUM_VIBRANT = {0.73f, 0.35f, 0.26f, 1.0f};
const Color COLOR_STATIC_LIGHT_VIBRANT = {0.67f, 0.8f, 0.35f, 1.0f};
const Color COLOR_STATIC_BRIGHT_VIBRANT = {0.93f, 0.93f, 0.93f, 1.0f};

const Color COLOR_GREY_DARK = {0.086f, 0.086f, 0.086f, 1.0f};
const Color COLOR_GREY_MEDIUM = {0.22f, 0.22f, 0.22f, 1.0f};
const Color COLOR_GREY_LIGHT = {0.26f, 0.26f, 0.26f, 1.0f};
const Color COLOR_GREY_BRIGHT = {0.67f, 0.73f, 0.8f, 1.0f};

const Color COLOR_GREY_DARK_VIBRANT = {0.13f, 0.086f, 0.13f, 1.0f};
const Color COLOR_GREY_MEDIUM_VIBRANT = {0.30f, 0.30f, 0.30f, 1.0f};
const Color COLOR_GREY_LIGHT_VIBRANT = {0.35f, 0.35f, 0.35f, 1.0f};
const Color COLOR_GREY_BRIGHT_VIBRANT = {0.9f, 0.9f, 0.9f, 1.0f};

const Color COLOR_DANGER = {0.73f, 0.266f, 0.333f, 1.0f};
const Color COLOR_DANGER_VIBRANT = {0.93f, 0.22f, 0.26f, 1.0f};

const Color COLOR_BG_BLUE = {0.20f, 0.24f, 0.28f, 1.0f};
const Color COLOR_BG_GREY = {0.4f, 0.4f, 0.4f, 1.0f};
const Color COLOR_GRID = {0.47f, 0.5f, 0.55f, 1.0f};
const Color COLOR_GOOD = {0.27f, 0.73f, 0.33f, 1.0f};
const Color COLOR_CONSTRAINT = {0.5f, 0.75f, 0.0f, 1.0f};
const Color COLOR_PLAYER = {0.5f, 1.0f, 0.5f, 1.0f};
const Color COLOR_AGENT = {1.0f, 1.0f, 0.5f, 1.0f};

// editor colors
const Color COLOR_CREATING_SHAPE = {0.8f, 0.7f, 0.65f, 0.5f};
const Color COLOR_SELECTED_BODY = {0.5f, 1.0f, 1.0f, 1.0f};
const Color COLOR_SELECTED_SHAPE = {0.0f, 0.4f, 1.0f, 0.5f};
const Color COLOR_SELECTED_CONSTRAINT = {0.5f, 1.0f, 0.7f, 1.0f};
const Color COLOR_SELECT  = {0.4f,0.6f,1.0f,1.0f};

const Color COLOR_BB = { 0.3f, 0.5f, 0.3f, 1.0f };
const Color COLOR_DEFAULT_LIGHT = {1.0f, 1.0f, 0.9f, 1.0f};

#endif /* COLOR_H */
