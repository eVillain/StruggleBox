#ifndef NGN_OPTIONS_H
#define NGN_OPTIONS_H

#include "Dictionary.h"
#include <map>
#include "Attribute.h"

class CmdProcessor;

//
//  Options.h
//  Ingenium
//
//  Created by Ville-Veikko Urrila on 13/01/12.
//  Copyright (c) 2012 The Drudgerist. All rights reserved.
//
//  Holds options values
//  Key:
//  r_ = renderer options
//  e_ = editor options
//  a_ = audio options
//  n_ = network options
//  d_ = debug options


class Options {
private:
    std::map<const std::string, Attribute*> m_Attributes;
public:
    // Constructor
    Options();
    // Destructor
    ~Options();
    
    // Loading and saving
    bool SaveOptionData(const char *filename);
    bool LoadOptionData(const char *filename);
    void SaveOptions();
    void LoadOptions();
    
    void ResetToDefaults();
    void PrintOpts();
    void AddConsoleVars();
    
    std::map<const std::string, Attribute*>& GetOptionMap() { return m_Attributes; };

    // Interface for options variables
    template<typename T> void AddOption(const std::string& attrName, T value) {
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
//            printf("[Options] attrib %s already added \n", attrName.c_str());
        }
    }
    template<typename T> T& GetOptionDataPtr(const std::string& attrName) {
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
};

#endif
