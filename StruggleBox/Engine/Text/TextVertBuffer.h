#ifndef TEXT_VERT_BUFFER_H
#define TEXT_VERT_BUFFER_H

#include <GL/glew.h>
#include <string>

class FontAtlas;

class TextVertBuffer
{
public:
    TextVertBuffer();
    
    void Buffer(const std::string& text,
                const FontAtlas& atlas,
                const int fontSize);
    void Draw();
    
private:
    GLuint _vao, _vbo;
    unsigned int _count;
    unsigned int _size;
};

#endif /* TEXT_VERT_BUFFER_H */
