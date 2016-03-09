#ifndef OPTIONS_H
#define OPTIONS_H

#include "Dictionary.h"
#include <map>
#include "Attribute.h"

class CmdProcessor;

//  Holds options values
//  Key:
//  r_ = renderer options
//  e_ = editor options
//  a_ = audio options
//  n_ = network options
//  d_ = debug options

class Options
{
public:
    Options();
    
    void save();
    void load();
    void setDefaults();

    // Interface for options variables
    template<typename T> void addOption(const std::string& attrName,
                                        T value)
    {
        if ( m_Attributes.find( attrName ) == m_Attributes.end() ) {
            Attribute* newAttrib = new Attribute(value);
            m_Attributes[attrName] = (Attribute*)newAttrib;
        } else {
            Attribute *a = ((Attribute*) m_Attributes[attrName]);
            if ( a->GetMagicNumber() != a->magic_number_for<T>() ) {
                throw "[Options] incorrect data type to add (already added with different type)";
            } else {
                // Set previous attribute value
                a->as<T>() = value;
            }
        }
    }
    
    template<typename T> T& getOption(const std::string& attrName)
    {
        Attribute *a = NULL;
        if ( m_Attributes.find( attrName ) != m_Attributes.end() ) {
            a = ((Attribute*) m_Attributes[attrName]);
            if ( a->GetMagicNumber() != a->magic_number_for<T>() ) {
                throw "[Options] incorrect data type to get";
            }
        } else {
            a = new Attribute(T());
            m_Attributes[attrName] = (Attribute*)a;
        }
        return a->as<T>();
    }
    
    std::map<const std::string, Attribute*>& getAllOptions()
    {
        return m_Attributes;
    }
private:
    std::map<const std::string, Attribute*> m_Attributes;

    bool save(const char *filename);
    bool load(const char *filename);
    void printDebugInfo();
    void addConsoleVars();
};

#endif
