//
//  SpriteBatch.h
//  NeverEngine
//
//  Created by The Drudgerist on 1/19/13.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//

#ifndef NE_SPRITE_BATCH_H
#define NE_SPRITE_BATCH_H

#include <string>
#include <map>
#include <vector>

#include "CoreTypes.h"
#include "GFXDefines.h"
#include "Rect2D.h"

class SpriteBatch {
private:
    std::map<std::string, Rect2D> frames;
    std::vector<std::string> frameIDs;
public:
    SpriteBatch( );
    ~SpriteBatch( );

    void LoadFromFile(  const std::string filePath, const std::string fileName );
    void CleanUp( void );
    void AddChild( Sprite * child );
    void RemoveChild( Sprite * child );

    bool ContainsChild( Sprite* child );
    Sprite* AddSpriteForFrame( const std::string frame );
    Rect2D GetRectForFrame( const std::string frame );
    const Rect2D GetTexRectForFrame( const std::string frame ) const;
    
    void SetFrameForSprite( const std::string frame, Sprite* sprite );
    std::string GetFrameForID( const int frameID );
    const int GetIDForFrame( const std::string frame ) const;
    std::vector<std::string> GetFrameNames( void );
    int GetNumFrames( void );
    
    std::vector<Sprite*> GetSpritesAtPoint( glm::vec2 point );
    std::vector<Sprite*> GetSpritesInRect( Rect2D rect );
    
    void Update ( const double dt );
    void Draw( Renderer* renderer );
    
    Texture* texture;

    std::vector<Sprite*> children;
};


#endif