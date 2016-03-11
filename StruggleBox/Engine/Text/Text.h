#ifndef TEXT_H
#define TEXT_H

#include "Label.h"
#include "Shader.h"
#include "FontAtlasPool.h"
#include "TextVertBuffer.h"
#include <map>
#include <string>
#include <memory>

class Locator;
class Renderer;

class Text
{
public:
    Text();
    ~Text();
    
    bool Initialize(Locator& locator);
    bool Terminate();
    
    void Draw();

    std::shared_ptr<Label> CreateLabel(const std::string text);
    void DestroyLabel(std::shared_ptr<Label> label);
    
protected:

    
private:
    Renderer* _renderer;
    bool _initialized;
    std::unique_ptr<FontAtlasPool> _atlasFactory;
    std::unique_ptr<Shader> _textShader;
    std::unique_ptr<Shader> _textShaderDeferred;
    std::map<std::shared_ptr<Label>, TextVertBuffer*> _labels2D;
    std::map<std::shared_ptr<Label>, TextVertBuffer*> _labels3D;
};


#endif
