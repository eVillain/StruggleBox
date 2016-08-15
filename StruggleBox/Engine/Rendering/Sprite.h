#ifndef SPRITE_H
#define SPRITE_H

#include "GFXDefines.h"
#include "Rect2D.h"
#include "Color.h"
#include <string>

class Texture;
class SpriteBatch;

class Sprite
{
public:
    Sprite( Texture * tex, float w, float h, glm::vec2 position = glm::vec2(), Rect2D rect = Rect2D(), Color col = COLOR_WHITE );
    Sprite( SpriteBatch* batch, std::string frame, glm::vec2 position = glm::vec2(), Color col = COLOR_WHITE );
    virtual ~Sprite();
    
    static Sprite* SpriteFromFile( const std::string filePath, const std::string fileName, glm::vec2 position = glm::vec2() );

    virtual void Update( double dt ) {};
    void FitToRect( const Rect2D &r );
    void Draw();
    void Visit();
    
    SpriteBatch* spriteBatch;
    Texture* texture;
    Rect2D rect;            // Position, size
    int z;                    // depth
    glm::vec2 scale;
    glm::vec2 rot;
    Color color;
    
    Rect2D texRect;
    bool batched;               // Is part of spritebatch
    int batchID;                // ID for frame in batch
};

#endif
