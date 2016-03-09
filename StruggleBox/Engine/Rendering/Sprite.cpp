
#include <algorithm>
#include "Sprite.h"
#include "SpriteBatch.h"
#include "Texture.h"
#include "TextureManager.h"


Sprite::Sprite( Texture * tex, float w, float h,
               glm::vec2 position, Rect2D rect, Color col ) :
texture(tex),
rect(position.x,position.y,w,h),
z(0),
scale(1.0, 1.0),
rot(0.0, 0.0),
color(col),
texRect(rect),
batched(false),
batchID(-1)
{
    texture->refCount++;
}
Sprite::Sprite( SpriteBatch* batch, std::string frame, glm::vec2 position, Color col )
{
    if ( batch == NULL ) {
        printf( "Failing to init sprite, no batch for frame: %s\n", frame.c_str() );
        return;
    }
    spriteBatch = batch;
    texture = spriteBatch->texture;
    rect = spriteBatch->GetRectForFrame( frame );
    texRect = spriteBatch->GetTexRectForFrame( frame );
    z=0;
    scale = glm::vec2(1.0f,1.0f);
    rot = glm::vec2(0.0f,0.0f);
    color = col;
    spriteBatch->AddChild( this );
    batched = true;
    batchID = batch->GetIDForFrame( frame );    
}
Sprite* Sprite::SpriteFromFile( const std::string filePath, const std::string fileName, glm::vec2 position )
{
    Texture* tex = TextureManager::Inst()->LoadTexture( filePath, fileName );
    if (!tex) {
        return NULL;
    }    
    Sprite* spr = new Sprite( tex, (float)(tex->GetWidth()), (float)(tex->GetHeight()), position, Rect2D(0.0f,0.0f,1.0f,1.0f) );
    return spr;
}
Sprite::~Sprite() {
    if (!batched) {
        texture->refCount--;
        TextureManager::Inst()->UnloadTexture( texture );
    } else {
        spriteBatch->RemoveChild( this );
    }
}
void Sprite::FitToRect( const Rect2D &r ) {
    // resize sprite to fit
    if ( rect.w > r.w ||
        rect.h > r.h ) {
        float scaleX = r.w / rect.w;
        float scaleY = r.h / rect.h;
        float minScale = (std::min)(scaleX, scaleY);
        rect.w = rect.w * minScale;
        rect.h = rect.h * minScale;
    }
    rect.x = r.x;
    rect.y = r.y;
}
void Sprite::Draw( ) {
    if (batched) return;
    if (!texture) return;
    
    // Bind the texture, enable the proper arrays
    texture->Bind();
    // Enable plain 2D texturing
    glEnable( GL_TEXTURE_2D );
    
    // Bind the texture, enable the proper arrays
    glEnableClientState( GL_VERTEX_ARRAY );
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    
    Visit();
    
    glDisable( GL_TEXTURE_2D );
}
void Sprite::Visit() {
    glColor4f(color.r, color.g, color.b, color.a);
    glPushMatrix(); {
        float hW = rect.w*0.5f;
        float hH = rect.h*0.5f;
        glTranslatef(rect.x+hW*scale.x, rect.y+hH*scale.y, (float)z);
        glScalef(scale.x, scale.y, 1.0f);
        glRotatef( (GLfloat)(rot.x*(180.0f/M_PI)), 0.0f, 0.0f, 1.0f);
        const float verts[] = {
            -hW, -hH,
             hW, -hH,
             hW,  hH,
            -hW,  hH
        };
        const float texVerts[] = {
            texRect.x               , texRect.y - texRect.h,
            texRect.x + texRect.w   , texRect.y - texRect.h,
            texRect.x + texRect.w   , texRect.y,
            texRect.x               , texRect.y
        };
        glVertexPointer(2, GL_FLOAT, 0, verts);
        glTexCoordPointer(2, GL_FLOAT, 0, texVerts);
        glDrawArrays(GL_QUADS, 0, 4);
    } glPopMatrix();
}
