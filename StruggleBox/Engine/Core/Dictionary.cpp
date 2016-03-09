#include "Dictionary.h" 

#include <iostream>
#include <algorithm>
#include "SysCore.h"

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

int ndigits(const char *s)
{
    int i, nd = 0;
    size_t n = strlen(s);
    
    for (i = 0; i < n; i++) {
        if (isdigit(s[i]))
            nd++;
    }
    return nd;
}

int alphaNumericallyLessThan(const char *s, const char *t)
{
    int sd, td;
    int i, j;
    
    sd = ndigits(s);
    td = ndigits(t);
    
    /* presence of digits */
    if (!sd && !td)
        return strcasecmp(s, t);
    else if (!sd)
        return 1;
    else if (!td)
        return -1;
    
    /* value of digits */
    for (i = 0, j = 0; i < sd && j < td; i++, j++) {
        while (! isdigit(*s))
            s++;
        while (! isdigit(*t))
            t++;
        
        if (*s != *t)
            return *s - *t;
        s++;
        t++;
    }
    
    /* number of digits */
    if (i < sd)
        return 1;
    else if (j < td)
        return -1;
    
    /* value of string after last digit */
    return strcasecmp(s, t);
}


//Constructor
Dictionary::Dictionary()
{
    //Initializers
    dictTree.push_back(pugi::xml_node());
    
    //Create default format
    pugi::xml_node plistNode = doc.append_child("plist");
    plistNode.append_attribute("version") = "1.0";
    plistNode.append_child("dict");
    
    //Set root dictTree node
    dictTree.back() = doc.child("plist").child("dict");
}

//Private methods
std::string Dictionary::cleanStringWhiteSpace(const std::string& _str)
{
    std::string cleanStr = _str;
    cleanStr.erase(remove_if(cleanStr.begin(), cleanStr.end(),  static_cast<int(*)(int)>( isspace )), cleanStr.end());
    return cleanStr;
}

void Dictionary::split(const std::string& src, const char* token, std::vector<std::string>& vect)
{
    size_t nend=0;
    size_t nbegin=0;
    while(nend != -1)
    {
        nend = src.find(token, nbegin);
        if(nend == -1)
            vect.push_back(src.substr(nbegin, src.length()-nbegin));
        else
            vect.push_back(src.substr(nbegin, nend-nbegin));
        nbegin = nend + strlen(token);
    }
}

bool Dictionary::splitWithForm(const std::string& content, std::vector<std::string>& strs)
{
    // string is empty
    if(content.length() == 0){ return false; }
    
    size_t nPosLeft  = content.find('{');
    size_t nPosRight = content.find('}');
    
    // don't have '{' and '}'
    if(nPosLeft == (int)std::string::npos || nPosRight == (int)std::string::npos){ return false; }
    
    // '}' is before '{'
    if(nPosLeft > nPosRight){ return false; }
    
    std::string pointStr = content.substr(nPosLeft + 1, nPosRight - nPosLeft - 1);
    
    // nothing between '{' and '}'
    if(pointStr.length() == 0){ return false; }
    
    size_t nPos1 = pointStr.find('{');
    size_t nPos2 = pointStr.find('}');
    
    // contain '{' or '}' 
    if(nPos1 != (int)std::string::npos || nPos2 != (int)std::string::npos){ return false; }
    
    split(pointStr, ",", strs);
    if (strs.size() != 2 || strs[0].length() == 0 || strs[1].length() == 0)
    {
        return false;
    }
    
    return true;
}

bool Dictionary::rectFromString(const std::string& pszContent, Rect2D& rect)
{
    //Clean white spaces
    std::string content = cleanStringWhiteSpace(pszContent);
    
    // find the first '{' and the third '}'
    size_t nPosLeft  = content.find('{');
    size_t nPosRight = content.find('}');
    for (int i = 1; i < 3; ++i)
    {
        if (nPosRight == (int)std::string::npos)
        {
            break;
        }
        nPosRight = content.find('}', nPosRight + 1);
    }
    if(nPosLeft == (int)std::string::npos || nPosRight == (int)std::string::npos){ return false; }
    
    content = content.substr(nPosLeft + 1, nPosRight - nPosLeft - 1);
    size_t nPointEnd = content.find('}');
    if(nPointEnd == (int)std::string::npos){ return false; }
    nPointEnd = content.find(',', nPointEnd);
    if(nPointEnd == (int)std::string::npos){ return false; }
    
    // get the point string and size string
    std::string pointStr = content.substr(0, nPointEnd);
    std::string sizeStr  = content.substr(nPointEnd + 1, content.length() - nPointEnd);
    
    // split the string with ','
    std::vector<std::string> pointInfo;
    if(!splitWithForm(pointStr, pointInfo)){ return false; }
    std::vector<std::string> sizeInfo;
    if(!splitWithForm(sizeStr, sizeInfo)){ return false; }
    
    float x = (float) atof(pointInfo[0].c_str());
    float y = (float) atof(pointInfo[1].c_str());
    float width  = (float) atof(sizeInfo[0].c_str());
    float height = (float) atof(sizeInfo[1].c_str());
    
    rect = Rect2D(x, y, width, height);
    return true;
}

bool Dictionary::vec2FromString(const std::string& pszContent, glm::vec2& vec2)
{
    std::vector<std::string> strs;
    if(!splitWithForm(cleanStringWhiteSpace(pszContent), strs))
    {
        return false;
    }
    
    float x = (float) atof(strs[0].c_str());
    float y = (float) atof(strs[1].c_str());
    
    vec2 = glm::vec2(x, y);
    return true;
}
bool Dictionary::ColorFromString(const std::string& pszContent, Color& col)
{
    std::vector<std::string> strs;
    if(!splitWithForm(cleanStringWhiteSpace(pszContent), strs))
    {
        return false;
    }
    
    float r = (float) atof(strs[0].c_str());
    float g = (float) atof(strs[1].c_str());
    float b = (float) atof(strs[2].c_str());
    float a = (float) atof(strs[3].c_str());
    col = RGBAColor(r, g, b, a);
    return true;
}
//Methods
bool Dictionary::loadRootSubDictFromFile(const char* fileName)
{
    //Clear existing
    dictTree.clear();
    dictTree.push_back(pugi::xml_node());

    // NEW
//    std::string path = SysCore:`:GetPath().append(fileName);
//    unsigned char* pBuffer = NULL;
//    unsigned long bufferSize = 0;
//    pBuffer = CCFileUtils::sharedFileUtils()->getFileData(path, "r", &bufferSize);
//    xml_parse_result result = doc.load_buffer(pBuffer,bufferSize);
    // END NEW

    //Open file
    pugi::xml_parse_result result = doc.load_file( fileName );
    
    if(!result)
    {
        //CCLog("%s \n",string("DS_ENGINE:> :ERROR: FAILED TO PARSE XML FILE: ").append(fileName).append(": WITH ERROR: ").append(result.description()).c_str());
        printf("[Dictionary] Error loading dictionary: %s, error:%s\n", fileName, result.description());
    	return false;
    }

    //Set root dictTree node
    dictTree.back() = doc.child("plist").child("dict");
    return true;
}

bool Dictionary::saveRootSubDictToFile(const char* fileName)
{
//    std::string path = FileUtil::GetPath().append(fileName);

    //Attempt to save file
    bool result = doc.save_file( fileName );
    
    if(!result)
    {
        printf("[Dictionary] Error saving dictionary %s\n", fileName);
        ////if(DS_DEBUG_MODE){ printf("%s \n",string("DS_ENGINE:> :ERROR: FAILED TO SAVE XML FILE: ").append(fileName).c_str()); }
    }
    
    return result;
}

bool Dictionary::stepIntoSubDictWithKey(const char* key)
{
    for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
    {
        if(node.child_value() == std::string(key) && node.next_sibling() == node.next_sibling("dict"))
        {
            dictTree.push_back(node.next_sibling());
            return true;
        }
    }
    
    ////if(DS_DEBUG_MODE){ printf("%s \n",string("DS_ENGINE:> :ERROR: FAILED TO STEP INTO XML DICTIONARY WITH KEY: ").append(key).c_str()); }
    return false;
}

void Dictionary::stepOutOfSubDict()
{
    if(dictTree.size() > 1)
    {
        dictTree.pop_back();
    }
}

void Dictionary::stepBackToRootSubDict()
{
    if(dictTree.size() > 1)
    {
        dictTree.clear();
        dictTree.push_back(pugi::xml_node());
        dictTree.back() = doc.child("plist").child("dict");
    }
}

uint32_t Dictionary::getNumKeys()
{
    uint32_t numKeys = 0;
    
    for(pugi::xml_node node = dictTree.back().first_child(); node; node = node.next_sibling().next_sibling())
    {
        numKeys++;
    }
    
    return numKeys;
}

std::string Dictionary::getKey(uint32_t index)
{
    uint32_t count = 0;
    
    for(pugi::xml_node node = dictTree.back().first_child(); node; node = node.next_sibling().next_sibling())
    {
        if(count == index){ return node.child_value(); }
        count++;
    }
    
    return std::string();
}

std::vector<std::string> Dictionary::getAllKeys()
{
    std::vector<std::string> strings;
    
    for(pugi::xml_node node = dictTree.back().first_child(); node; node = node.next_sibling().next_sibling())
    {
        strings.push_back(node.child_value());
    }
    
    return strings;
}

uint32_t Dictionary::getIndexOfKey(const char* key)
{
    uint32_t count = 0;
    
    for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
    {
        if(node.child_value() == std::string(key))
        {
            return count;
        }
        count++;
    }
    
    return 0;
}

uint32_t Dictionary::getIndexOfKeyWithClosestAlphaNumericalMatch(const char* key)
{
    uint32_t count = 0;
    
    for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
    {
        if(alphaNumericallyLessThan(key, node.child_value()))
        {
            return count;
        }
        count++;
    }
    
    return 0;
}

void Dictionary::removeKey(uint32_t index)
{
    uint32_t count = 0;
    
    for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
    {
        if(count == index)
        { 
            //Remove the node
            dictTree.back().remove_child(node.next_sibling());
            dictTree.back().remove_child(node);
            break;
        }
        count++;
    }
}

void Dictionary::removeKey(const char* key)
{
    //Check to see if the value already exists withing the current sub dict
    for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
    {
        if(node.child_value() == std::string(key))
        {
            //Already exists, remove the node
            dictTree.back().remove_child(node.next_sibling());
            dictTree.back().remove_child(node);
            break;
        }
    }
}

void Dictionary::removeAllKeys()
{
    while(pugi::xml_node node = dictTree.back().child("key"))
    {
        dictTree.back().remove_child(node.next_sibling());
        dictTree.back().remove_child(node);
    }
}

int Dictionary::getIntegerForKey(const char* key)
{
    for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
    {
        if(node.child_value() == std::string(key) && (node.next_sibling() == node.next_sibling("integer") || node.next_sibling() == node.next_sibling("real")))
        {
            return (int)strtol(node.next_sibling().child_value(), NULL, 10);
        }
    }
    
    //if(DS_DEBUG_MODE){ printf("%s \n",string("DS_ENGINE:> :ERROR: FAILED TO READ INTEGER VALUE FOR KEY: ").append(key).c_str()); }
    return 0;
}

bool Dictionary::getBoolForKey(const char* key)
{
    for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
    {
        if(node.child_value() == std::string(key) && ((node.next_sibling() == node.next_sibling("true") || node.next_sibling() == node.next_sibling("1")) || (node.next_sibling() == node.next_sibling("false") || node.next_sibling() == node.next_sibling("0"))))
        {
            if(node.next_sibling() == node.next_sibling("true") || node.next_sibling() == node.next_sibling("1"))
                return true;
            else return false;
        }
    }
    
    //if(DS_DEBUG_MODE){ printf("%s \n",string("DS_ENGINE:> :ERROR: FAILED TO READ BOOL VALUE FOR KEY: ").append(key).c_str()); }
    return false;
}

float Dictionary::getFloatForKey(const char* key)
{
    for( pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key") )
    {
        if( node.child_value() == std::string(key) && (node.next_sibling() == node.next_sibling("integer") ||
			node.next_sibling() == node.next_sibling("real")) ) {
            return (float)strtod(node.next_sibling().child_value(), NULL);
        }
    }
    return 0.0f;
}

std::string Dictionary::getStringForKey(const char* key)
{
    for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
    {
        if(node.child_value() == std::string(key) && node.next_sibling() == node.next_sibling("string"))
        {
            return node.next_sibling().child_value();
        }
    }
    
    //if(DS_DEBUG_MODE){ printf("%s \n",string("DS_ENGINE:> :ERROR: FAILED TO READ STRING VALUE FOR KEY: ").append(key).c_str()); }
    return "";
}

glm::vec2 Dictionary::getVec2ForKey(const char* key)
{
    for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
    {
        if(node.child_value() == std::string(key) && node.next_sibling() == node.next_sibling("string"))
        {
            glm::vec2 vec2;
            if(vec2FromString(node.next_sibling().child_value(), vec2))
            {
                return vec2;
            }
            else
            {
                //if(DS_DEBUG_MODE){ printf("%s \n",string("DS_ENGINE:> :ERROR: MALFORMED VEC2 VALUE FOR KEY: ").append(key).c_str()); }
                return glm::vec2();
            }
        }
    }
    //if(DS_DEBUG_MODE){ printf("%s \n",string("DS_ENGINE:> :ERROR: FAILED TO READ VEC2 VALUE FOR KEY: ").append(key).c_str()); }
    return glm::vec2();
}
Color Dictionary::getColorForKey(const char* key)
{
    for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
    {
        if(node.child_value() == std::string(key) && node.next_sibling() == node.next_sibling("string"))
        {
            Color col;
            if(ColorFromString(node.next_sibling().child_value(), col))
            {
                return col;
            }
            else
            {
                //if(DS_DEBUG_MODE){ printf("%s \n",string("DS_ENGINE:> :ERROR: MALFORMED COLOR VALUE FOR KEY: ").append(key).c_str()); }
                return COLOR_NONE;
            }
        }
    }
    //if(DS_DEBUG_MODE){ printf("%s \n",string("DS_ENGINE:> :ERROR: FAILED TO READ COLOR VALUE FOR KEY: ").append(key).c_str()); }
    return COLOR_NONE;
}
Rect2D Dictionary::getRectForKey(const char* key) {
    for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key")) {
        if(node.child_value() == std::string(key) && node.next_sibling() == node.next_sibling("string")) {
            Rect2D rect;
            if( rectFromString(node.next_sibling().child_value(), rect) ) { return rect; }
            else { return Rect2D(); }
        }
    }
    return Rect2D();
}

std::vector<std::string> Dictionary::getStringArrayForKey(const char* key) {
    for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key")) {
        if(node.child_value() == std::string(key) && node.next_sibling() == node.next_sibling("array")) {
            std::vector<std::string> value;
            for(pugi::xml_node arrayNode = node.next_sibling().first_child(); arrayNode; arrayNode = arrayNode.next_sibling()) {
                value.push_back(arrayNode.child_value());
            }
            return value;
        }
    }
    return std::vector<std::string>();
}

std::vector<glm::vec2> Dictionary::getVec2ArrayForKey(const char* key)
{
    std::vector<std::string> strings = getStringArrayForKey(key);
    std::vector<glm::vec2> vectorVec2;
    
    for( unsigned int i=0; i<strings.size(); i++ )
    {
        vectorVec2.push_back(glm::vec2());
        if(!vec2FromString(strings[i], vectorVec2.back()))
        {
            //if(DS_DEBUG_MODE){ printf("%s \n",string("DS_ENGINE:> :ERROR: MALFORMED VEC2 VALUE FOR ARRAY KEY: ").append(key).c_str()); }
            vectorVec2.clear();
            break;
        }
    }
    
    return vectorVec2;
}

std::vector<Rect2D> Dictionary::getRectArrayForKey(const char* key)
{
    std::vector<std::string> strings = getStringArrayForKey(key);
    std::vector<Rect2D> vectorRect;
    
    for( unsigned int i=0; i<strings.size(); i++ ) {
        vectorRect.push_back(Rect2D());
        if(!rectFromString(strings[i], vectorRect.back())) {
            vectorRect.clear();
            break;
        }
    }
    
    return vectorRect;
}
int Dictionary::getTypeForKey(const char *key) {
    for( pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key") )
    {
        if( node.child_value() == std::string(key) &&
           ( (node.next_sibling() == node.next_sibling("true"))||
             (node.next_sibling() == node.next_sibling("false")) ) ) {
            return DD_Bool;
        } else if( node.child_value() == std::string(key) && (node.next_sibling() == node.next_sibling("integer") ) ) {
            return DD_Int;
        } else if( node.child_value() == std::string(key) && (node.next_sibling() == node.next_sibling("real") ) ) {
            return DD_Float;
        } else if( node.child_value() == std::string(key) && (node.next_sibling() == node.next_sibling("string") ) ) {
            return DD_String;
        }
    }
    return 0;
}
void Dictionary::setSubDictForKey(const char* key) { setSubDictForKey(key, false); }
void Dictionary::setSubDictForKey(const char* key, bool alphaNumericallySorted)
{
    //Check to see if the sub dict already exists withing the current sub dict
    removeKey(key);
    //Create a key value pair as a child of the current sub dict
    pugi::xml_node keyNode;
    bool appendKey = true;
    if(alphaNumericallySorted)
    {
        for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
        {
            if(!alphaNumericallyLessThan(node.child_value(), key))
            {
                keyNode = dictTree.back().insert_child_before("key", node);
                appendKey = false;
                break;
            }
        }
    }
    if(appendKey){ keyNode = dictTree.back().append_child("key"); }
    keyNode.append_child(pugi::node_pcdata).set_value(key);
    dictTree.back().insert_child_after("dict", keyNode);
}

void Dictionary::setIntegerForKey(const char* key, int value){ setIntegerForKey(key, value, false); }
void Dictionary::setIntegerForKey(const char* key, int value, bool alphaNumericallySorted)
{
    //Create a string from the input value
    std::stringstream stream;
    stream << value;
    
    //Check to see if the value already exists withing the current sub dict
    removeKey(key);
    
    //Create a key value pair as a child of the current sub dict
    pugi::xml_node keyNode;
    bool appendKey = true;
    if(alphaNumericallySorted)
    {
        for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
        {
            if(!alphaNumericallyLessThan(node.child_value(), key))
            {
                keyNode = dictTree.back().insert_child_before("key", node);
                appendKey = false;
                break;
            }
        }
    }
    
    if(appendKey){ keyNode = dictTree.back().append_child("key"); }
    keyNode.append_child(pugi::node_pcdata).set_value(key);
    dictTree.back().insert_child_after("integer", keyNode).append_child(pugi::node_pcdata).set_value(stream.str().c_str());
}

void Dictionary::setBoolForKey(const char* key, bool value){ setBoolForKey(key, value, false); }
void Dictionary::setBoolForKey(const char* key, bool value, bool alphaNumericallySorted)
{
    //Check to see if the value already exists withing the current sub dict
    removeKey(key);
    
    //Create a key value pair as a child of the current sub dict
    pugi::xml_node keyNode;
    bool appendKey = true;
    if(alphaNumericallySorted)
    {
        for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
        {
            if(!alphaNumericallyLessThan(node.child_value(), key))
            {
                keyNode = dictTree.back().insert_child_before("key", node);
                appendKey = false;
                break;
            }
        }
    }
    
    if(appendKey){ keyNode = dictTree.back().append_child("key"); }
    keyNode.append_child(pugi::node_pcdata).set_value(key);
    if(value){ dictTree.back().insert_child_after("true", keyNode); }
    else     { dictTree.back().insert_child_after("false", keyNode); }
}

void Dictionary::setFloatForKey(const char* key, float value){ setFloatForKey(key, value, false); }
void Dictionary::setFloatForKey(const char* key, float value, bool alphaNumericallySorted)
{
    //Create a string from the input value
    std::stringstream stream;
    stream << value;
    
    //Check to see if the value already exists withing the current sub dict
    removeKey(key);
    
    //Create a key value pair as a child of the current sub dict
    pugi::xml_node keyNode;
    bool appendKey = true;
    if(alphaNumericallySorted)
    {
        for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
        {
            if(!alphaNumericallyLessThan(node.child_value(), key))
            {
                keyNode = dictTree.back().insert_child_before("key", node);
                appendKey = false;
                break;
            }
        }
    }
    
    if(appendKey){ keyNode = dictTree.back().append_child("key"); }
    keyNode.append_child(pugi::node_pcdata).set_value(key);
    dictTree.back().insert_child_after("real", keyNode).append_child(pugi::node_pcdata).set_value(stream.str().c_str());
}

void Dictionary::setStringForKey(const char* key, const std::string& value){ setStringForKey(key, value, false); }
void Dictionary::setStringForKey(const char* key, const std::string& value, bool alphaNumericallySorted)
{
    //Check to see if the value already exists withing the current sub dict
    removeKey(key);
    
    //Create a key value pair as a child of the current sub dict
    pugi::xml_node keyNode;
    bool appendKey = true;
    if(alphaNumericallySorted)
    {
        for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
        {
            if(!alphaNumericallyLessThan(node.child_value(), key))
            {
                keyNode = dictTree.back().insert_child_before("key", node);
                appendKey = false;
                break;
            }
        }
    }
    
    if(appendKey){ keyNode = dictTree.back().append_child("key"); }
    keyNode.append_child(pugi::node_pcdata).set_value(key);
    dictTree.back().insert_child_after("string", keyNode).append_child(pugi::node_pcdata).set_value(value.c_str());
}

void Dictionary::setVec2ForKey(const char* key, const glm::vec2& value){ setVec2ForKey(key, value, false); }
void Dictionary::setVec2ForKey(const char* key, const glm::vec2& value, bool alphaNumericallySorted)
{
    //Create a string from the input value
    std::stringstream stream;
    stream << "{";
    stream << value.x;
    stream << ",";
    stream << value.y;
    stream << "}";
    
    //Check to see if the value already exists withing the current sub dict
    removeKey(key);
    
    //Create a key value pair as a child of the current sub dict
    pugi::xml_node keyNode;
    bool appendKey = true;
    if(alphaNumericallySorted)
    {
        for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
        {
            if(!alphaNumericallyLessThan(node.child_value(), key))
            {
                keyNode = dictTree.back().insert_child_before("key", node);
                appendKey = false;
                break;
            }
        }
    }
    
    if(appendKey){ keyNode = dictTree.back().append_child("key"); }
    keyNode.append_child(pugi::node_pcdata).set_value(key);
    dictTree.back().insert_child_after("string", keyNode).append_child(pugi::node_pcdata).set_value(stream.str().c_str());
}

void Dictionary::setRectForKey(const char* key, const Rect2D& value){ setRectForKey(key, value, false); }
void Dictionary::setRectForKey(const char* key, const Rect2D& value, bool alphaNumericallySorted)
{
    //Create a string from the input value
    std::stringstream stream;
    stream << "{{";
    stream << value.x;
    stream << ",";
    stream << value.y;
    stream << "},{";
    stream << value.w;
    stream << ",";
    stream << value.h;
    stream << "}}";
    
    //Check to see if the value already exists withing the current sub dict
    removeKey(key);
    
    //Create a key value pair as a child of the current sub dict
    pugi::xml_node keyNode;
    bool appendKey = true;
    if(alphaNumericallySorted)
    {
        for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
        {
            if(!alphaNumericallyLessThan(node.child_value(), key))
            {
                keyNode = dictTree.back().insert_child_before("key", node);
                appendKey = false;
                break;
            }
        }
    }
    
    if(appendKey){ keyNode = dictTree.back().append_child("key"); }
    keyNode.append_child(pugi::node_pcdata).set_value(key);
    dictTree.back().insert_child_after("string", keyNode).append_child(pugi::node_pcdata).set_value(stream.str().c_str());
}

void Dictionary::setStringArrayForKey(const char* key, const std::vector<std::string>& value){ setStringArrayForKey(key, value, false); }
void Dictionary::setStringArrayForKey(const char* key, const std::vector<std::string>& value, bool alphaNumericallySorted)
{
    //Check to see if the value already exists withing the current sub dict
    removeKey(key);
    
    //Create a key value pair as a child of the current sub dict
    pugi::xml_node keyNode;
    pugi::xml_node arrayNode;
    bool appendKey = true;
    if(alphaNumericallySorted)
    {
        for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
        {
            if(!alphaNumericallyLessThan(node.child_value(), key))
            {
                keyNode = dictTree.back().insert_child_before("key", node);
                appendKey = false;
                break;
            }
        }
    }
    
    if(appendKey){ keyNode = dictTree.back().append_child("key"); }
    keyNode.append_child(pugi::node_pcdata).set_value(key);
    arrayNode = dictTree.back().insert_child_after("array", keyNode);
    
    //Fill the array
    for( unsigned int i=0; i<value.size(); i++ ) {
        arrayNode.append_child("string").append_child(pugi::node_pcdata).set_value(value[i].c_str());
    }
}

void Dictionary::setVec2ArrayForKey(const char* key, const std::vector<glm::vec2>& value){ setVec2ArrayForKey(key, value, false); }
void Dictionary::setVec2ArrayForKey(const char* key, const std::vector<glm::vec2>& value, bool alphaNumericallySorted)
{
    //Check to see if the value already exists withing the current sub dict
    removeKey(key);
    
    //Create a key value pair as a child of the current sub dict
    pugi::xml_node keyNode;
    pugi::xml_node arrayNode;
    bool appendKey = true;
    if(alphaNumericallySorted)
    {
        for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
        {
            if(!alphaNumericallyLessThan(node.child_value(), key))
            {
                keyNode = dictTree.back().insert_child_before("key", node);
                appendKey = false;
                break;
            }
        }
    }
    
    if(appendKey){ keyNode = dictTree.back().append_child("key"); }
    keyNode.append_child(pugi::node_pcdata).set_value(key);
    arrayNode = dictTree.back().insert_child_after("array", keyNode);
    
    //Fill the array
    for( unsigned int i=0; i<value.size(); i++ ) {
        //Create a string from the input value
        std::stringstream stream;
        stream << "{";
        stream << value[i].x;
        stream << ",";
        stream << value[i].y;
        stream << "}";
        
        arrayNode.append_child("string").append_child(pugi::node_pcdata).set_value(stream.str().c_str());
    }
}

void Dictionary::setRectArrayForKey(const char* key, const std::vector<Rect2D>& value){ setRectArrayForKey(key, value, false); }
void Dictionary::setRectArrayForKey(const char* key, const std::vector<Rect2D>& value, bool alphaNumericallySorted)
{
    //Check to see if the value already exists withing the current sub dict
    removeKey(key);
    
    //Create a key value pair as a child of the current sub dict
    pugi::xml_node keyNode;
    pugi::xml_node arrayNode;
    bool appendKey = true;
    if(alphaNumericallySorted)
    {
        for(pugi::xml_node node = dictTree.back().child("key"); node; node = node.next_sibling("key"))
        {
            if(!alphaNumericallyLessThan(node.child_value(), key))
            {
                keyNode = dictTree.back().insert_child_before("key", node);
                appendKey = false;
                break;
            }
        }
    }
    
    if(appendKey){ keyNode = dictTree.back().append_child("key"); }
    keyNode.append_child(pugi::node_pcdata).set_value(key);
    arrayNode = dictTree.back().insert_child_after("array", keyNode);
    
    //Fill the array
    for( unsigned int i=0; i<value.size(); i++ ) {
        //Create a string from the input value
        std::stringstream stream;
        stream << "{{";
        stream << value[i].x;
        stream << ",";
        stream << value[i].y;
        stream << "},{";
        stream << value[i].w;
        stream << ",";
        stream << value[i].h;
        stream << "}}";
        arrayNode.append_child("string").append_child(pugi::node_pcdata).set_value(stream.str().c_str());
    }
}

