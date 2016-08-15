#ifndef TEXT_VERT_BUFFER_H
#define TEXT_VERT_BUFFER_H

#include <GL/glew.h>
#include <string>
#include <memory>

class Renderer;
class VertBuffer;
class FontAtlas;

class TextVertBuffer
{
public:
    TextVertBuffer(std::shared_ptr<Renderer> renderer);
    
	void bind();

    void buffer(const std::string& text,
                const FontAtlas& atlas,
                const int fontSize);

    void draw();
    
private:
    std::shared_ptr<VertBuffer> _vertBuffer;
    unsigned int _count;
    unsigned int _size;
};

#endif /* TEXT_VERT_BUFFER_H */
