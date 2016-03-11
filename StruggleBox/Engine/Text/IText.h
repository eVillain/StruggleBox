#ifndef ITEXT_H
#define ITEXT_H

#include "RenderDefines.h"
#include "Color.h"
#include "TextConstants.h"
#include <string>

class FontAtlas;
class Label;

class IText
{
    friend class Label;

public:
    virtual ~IText() {};
    
    virtual void Initialize() = 0;
    virtual void Terminate() = 0;
    
    virtual void Update( double delta ) = 0;
    virtual void Render2D() = 0;
    virtual void Render3D() = 0;

protected:
    virtual void Add(Label*label) = 0;
    virtual void Remove(Label*label) = 0;
    
    virtual FontAtlas* GetAtlas( std::string filename, int size ) = 0;
};

// Null class that does nothing
class NullText : public IText
{
public:
    void Initialize( ) { /* Do nothing */ };
    void Terminate() { /* Do nothing */ };
    
    void Update( double delta ) { /* Do nothing */ };
    void Render2D() { /* Do nothing */ };
    void Render3D() { /* Do nothing */ };

protected:
    void Add(Label*label) { /* Do nothing */ };
    void Remove(Label*label) { /* Do nothing */ };
    
    FontAtlas* GetAtlas( std::string filename, int size ) { return NULL; };
};

#endif
