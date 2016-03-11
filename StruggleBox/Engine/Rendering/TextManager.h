#ifndef TEXT_MANAGER_H
#define TEXT_MANAGER_H

#include "Color.h"
#include "FontAtlas.h"
#include <map>
#include <vector>
#include <string>

#define TEXT_FADE_TIME 0.5

class Locator;

enum FontName {
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

typedef struct {
    glm::vec3 pos;
    glm::vec3 rot;
    Color color;
    int size;
    FontName font;
    int blobID;
    double timer;
    bool isUIBlob;
    std::string text;
} TextBlob;

class Renderer;
class Shader;

class TextManager
{
public:
    TextManager();
    
    void Initialize(Locator& locator);
    void Terminate();
    
    int AddText(std::string text, glm::vec3 position,
                bool UISpace = false, int size = 16,
                FontName font = FONT_DEFAULT, double timer = 0.0,
                Color color = COLOR_WHITE,
                glm::vec3 rotation = glm::vec3(0,0,0));
    
    void RemoveText(int blobID);
    
    void UpdateText( unsigned int blobID, std::string newText );
    
    void UpdateTextColor( unsigned int blobID, Color newColor );
    
    void UpdateTextPos( unsigned int blobID, glm::vec3 newPos );
    
    void UpdateTextRot( unsigned int blobID, glm::vec3 newRot );

    void UpdateTextTimer( unsigned int blobID, double newTimer );

    void GetTextSize( unsigned int blobID, float &width, float &height );
    std::string GetFontFileName( FontName theFont );
    
    void Update( double delta );
    void RenderLabels( void );
    void Render( TextBlob& b, FontAtlas * a );
    void RenderFixed( TextBlob& b, FontAtlas * a );
    
private:
    // Font atlases by font filename and iterator
    typedef std::map<std::string, FontAtlas*> AtlasList;
    typedef AtlasList::const_iterator AtlasIter;
    
    // The next blob ID to be generated
    unsigned int _nextId = 0;
    
    // Container for fonts
    AtlasList atlases;
    
    // Counter for each frame
    int blobsRendered;
    
    bool initialized = false;
    FT_Library ft;
    
    // Shader mode vars
    Shader* textShader = NULL;
    GLuint vertexArrayID = 0;
    GLuint vertexBufferID = 0;
    
    
    FontAtlas* GetAtlas( std::string filename, int size );
    void RenderBlob( TextBlob b );
    int AddBlob( TextBlob& newBlob );
};

#endif
