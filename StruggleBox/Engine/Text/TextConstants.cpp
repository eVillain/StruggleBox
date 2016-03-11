#include "TextConstants.h"

namespace Fonts
{
    const std::string fontFiles[]
    {
        "ClearSans-Regular.ttf",
        "HiLoDeco.ttf",
        "ORANGEKI.ttf",
        "CRETINO.ttf",
        "Jura-Book.ttf",
        "Segment14.otf",
        "IMFeENrm29P.ttf",
        "IMFeNsc29P.ttf",
        "Bentham.ttf"
    };
    
    const std::string GetFileName(const FontID& theFont)
    {
        return fontFiles[theFont];
    }
}
