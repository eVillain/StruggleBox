#ifndef TEXT_CONSTANTS_H
#define TEXT_CONSTANTS_H
#include <string>

typedef enum : unsigned int
{
    Align_Center = 0,
    Align_Left = 1,
    Align_Right = 2,
} TextAlignment;

namespace Fonts
{
    // Supported fonts: Hard-wired for now, must make these configurable later
    enum FontID {
        FONT_DEFAULT,
        FONT_MENU,
        FONT_PIXEL,
        FONT_FANCY,
        FONT_JURA,
        FONT_SEGMENT,
        FONT_FELL_NORMAL,
        FONT_FELL_CAPS,
        FONT_BENTHAM,
    };
    
    const std::string GetFileName( const FontID& theFont );
    
}   /* namespace Fonts */

#endif
