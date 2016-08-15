#ifndef DICTIONARY_H
#define DICTIONARY_H

// This is a .plist file parser using PugiXML
// Mainly used for reading spritesheet definitions
// Based on code by Alan Ide, edited by The Drudgerist
// From: http://www.cocos2d-x.org/boards/6/topics/6125

#include "pugixml.hpp" 
#include <vector>
#include "DictionaryHelpers.h"
#include "Rect2D.h"
#include "GFXDefines.h"
#include "Color.h"

enum DictDataType {
    DD_Bool = 1,
    DD_Int = 2,
    DD_Float = 3,
    DD_String = 4,
};

class Dictionary
{
private:
    //Data
    pugi::xml_document doc;
    std::vector<pugi::xml_node> dictTree;
    
    //Constructor blockers
    Dictionary(const Dictionary& dictionary);                //Copy constructor - blocker
    Dictionary& operator= (const Dictionary& dictionary);    //Assignment - blocker
    
    //Methods
    std::string cleanStringWhiteSpace(const std::string& _str);
    void split(const std::string& src, const char* token, std::vector<std::string>& vect);
    bool splitWithForm(const std::string& content, std::vector<std::string>& strs);
    bool rectFromString(const std::string& pszContent, Rect2D& rect);
    bool vec2FromString(const std::string& pszContent, glm::vec2& vec2);
    bool ColorFromString(const std::string& pszContent, Color& col);
public:
    //Constructor
    Dictionary();
    
    //Methods
    bool loadRootSubDictFromFile(const char* fileName);
    bool saveRootSubDictToFile(const char* fileName);
    bool stepIntoSubDictWithKey(const char* key);
    void stepOutOfSubDict();
    void stepBackToRootSubDict();
    
    uint32_t getNumKeys();
    std::string getKey(uint32_t index);
    std::vector<std::string> getAllKeys();
    uint32_t getIndexOfKey(const char* key);
    uint32_t getIndexOfKeyWithClosestAlphaNumericalMatch(const char* key);
    
    void removeKey(uint32_t index);
    void removeKey(const char* key);
    void removeAllKeys();
    
    int getIntegerForKey(const char* key);
    bool getBoolForKey(const char* key);
    float getFloatForKey(const char* key);
    std::string getStringForKey(const char* key);
    glm::vec2 getVec2ForKey(const char* key);
    Color getColorForKey(const char* key);
    Rect2D getRectForKey(const char* key);
    std::vector<std::string> getStringArrayForKey(const char* key);
    std::vector<glm::vec2> getVec2ArrayForKey(const char* key);
    std::vector<Rect2D> getRectArrayForKey(const char* key);
    //void getValueForKey(const char* key);
    int getTypeForKey(const char* key);
    
    void setSubDictForKey(const char* key);
    void setSubDictForKey(const char* key, bool alphaNumericallySorted);
    void setIntegerForKey(const char* key, int value);
    void setIntegerForKey(const char* key, int value, bool alphaNumericallySorted);
    void setBoolForKey(const char* key, bool value);
    void setBoolForKey(const char* key, bool value, bool alphaNumericallySorted);
    void setFloatForKey(const char* key, float value);
    void setFloatForKey(const char* key, float value, bool alphaNumericallySorted);
    void setStringForKey(const char* key, const std::string& value);
    void setStringForKey(const char* key, const std::string& value, bool alphaNumericallySorted);
    void setVec2ForKey(const char* key, const glm::vec2& value);
    void setVec2ForKey(const char* key, const glm::vec2& value, bool alphaNumericallySorted);
    void setRectForKey(const char* key, const Rect2D& value);
    void setRectForKey(const char* key, const Rect2D& value, bool alphaNumericallySorted);
    void setStringArrayForKey(const char* key, const std::vector<std::string>& value);
    void setStringArrayForKey(const char* key, const std::vector<std::string>& value, bool alphaNumericallySorted);
    void setVec2ArrayForKey(const char* key, const std::vector<glm::vec2>& value);
    void setVec2ArrayForKey(const char* key, const std::vector<glm::vec2>& value, bool alphaNumericallySorted);
    void setRectArrayForKey(const char* key, const std::vector<Rect2D>& value);
    void setRectArrayForKey(const char* key, const std::vector<Rect2D>& value, bool alphaNumericallySorted);
};

#endif /* DICTIONARY_H */
