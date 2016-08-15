#ifndef DICTIONARY_HELPERS_H
#define DICTIONARY_HELPERS_H

#include <stdio.h>
#include <sstream>
#include <stdint.h>

inline static std::string itos(int number)
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}

inline static std::string ftos(float number)
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}

static bool alphaNumericallyLessThan(const std::string& s1, const std::string& s2)
{ 
    //AlphaNumeric test...
    size_t tSize = s1.size() < s2.size() ? s1.size() : s2.size();
    
    for( unsigned int i=0; i<tSize; i++ )
    {
        //English ASCII sort helper to make the sort case insensitive. Other languages using UTF8 will be SOL.
        uint32_t c1 = (s1[i] >= 97 && s1[i] <= 122) ? s1[i] - 32 : s1[i];
        uint32_t c2 = (s2[i] >= 97 && s2[i] <= 122) ? s2[i] - 32 : s2[i];
        
        if(c2 > c1){ return true; }
        if(c2 < c1){ return false; }
    }
    
    //Default return
    return true;
}

inline static void sortAlphaNumerically(std::vector<std::string>& strings)
{
    //Return if empty
    if(strings.size() == 0){ return; }
    
    //Sort
    std::vector<std::string>sortedStrings;
    sortedStrings.push_back(strings[0]);
    
    for(unsigned int i=1; i<strings.size(); i++)
    {
        bool append = true;
        for(unsigned int j=0; j<sortedStrings.size(); j++)
        {
            if(!alphaNumericallyLessThan(sortedStrings[j], strings[i]))
            {
                sortedStrings.insert(sortedStrings.begin() + j, strings[i]);
                append = false;
                break;
            }
        }
        
        if(append)
        {
            sortedStrings.push_back(strings[i]);
        }
    }
    
    // Assign from sorted
    strings = sortedStrings;
}

inline static void splitByWhiteSpace(const std::string& str, std::vector<std::string>& splitStr)
{
    //Clear incomming
    splitStr.clear();
    
    //Split
    const char *start = str.c_str();
    const char *end   = start + str.size();
    const char *token = start;
    
    while(start!=end)
    {
        if(isspace(*start))
        {
            if(token < start)
            { 
                splitStr.push_back(std::string(token, start));
                if(splitStr.back() == " "){ splitStr.pop_back(); } //Dont allow empty
            }
            
            start++;
            token = start;
        }
        
        else{ start++; }
    } 
    
    if(token < start)
    { 
        splitStr.push_back(std::string(token, start));
        if(splitStr.back() == " "){ splitStr.pop_back(); } //Dont allow empty
    }
}

#endif
