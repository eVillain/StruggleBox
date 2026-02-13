#ifndef CONSOLE_DEFS_H
#define CONSOLE_DEFS_H

#include "Color.h"
#include <string>

class Label;

static const int CONSOLE_FONT_SIZE=18;
static const int CONSOLE_BG_DEPTH=1;
static const int CONSOLE_TEXT_DEPTH=10;
static const int CONSOLE_MAX_MESSAGES=10;
static const int CONSOLE_TEXT_HEIGHT=22;

typedef struct {
    std::string text;
    Color color;
    double timeStamp;
} ConsoleLine;

#endif /* CONSOLE_DEFS_H */
