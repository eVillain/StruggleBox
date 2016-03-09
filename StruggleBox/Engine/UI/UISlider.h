#ifndef UI_SLIDER_H
#define UI_SLIDER_H

#include "UIWidget.h"
#include "GFXHelpers.h"
#include "TextManager.h"

#define SLIDER_TEXT_SIZE 11

class UISliderBase : public UIWidget {
public:
    int sliderPos;
    std::string m_label;
    int labelID, varLabelID;
    bool dragging;
    
    UISliderBase( int posX, int posY,
                 int width, int height,
                 int depth,
                 std::string label,
                 std::string texDefault = "",
                 std::string texActive = "",
                 std::string texPressed = "" );
    virtual ~UISliderBase( void );
    bool SliderTest(const glm::ivec2 coord);

    void CursorRelease(const glm::ivec2 coord);
    void CursorPress(const glm::ivec2 coord);
    void CursorHover(const glm::ivec2 coord, bool highlight);
    void Draw( Renderer* renderer );
    virtual void DoCallBack() {};   // Override this in subclasses to update param value
    
    virtual void Update( void );
    virtual void UpdatePos( int posX, int posY );

};

template <typename T>
class UISlider : public UISliderBase {
    T* dataPointer;
    T m_min, m_max;
public:
    
    UISlider( int posX, int posY,
            int width, int height,
             int depth,
             std::string label,
             T* data,
             T minVal, T maxVal,
             std::string texDefault = "",
             std::string texActive = "",
             std::string texPressed = "" ) :
    UISliderBase( posX, posY, width, height, depth, label, texDefault, texActive, texPressed ) {
        dataPointer = data;
        m_min = minVal; m_max = maxVal;
    };
    virtual ~UISlider() {
        dataPointer = NULL;
    };
    virtual void Update( void ) {
        UISliderBase::Update();
    };
    virtual void UpdatePos( int posX, int posY ) {
        UISliderBase::UpdatePos(posX, posY);
    };
};
// Specialized classes
template<> class UISlider<float> : public UISliderBase {
    float* dataPointer;
    float m_min, m_max;
public:
    UISlider( int posX, int posY,
             int width, int height,
             int depth,
             std::string label,
             float* data,
             float minVal, float maxVal,
             std::string texDefault = "",
             std::string texActive = "",
             std::string texPressed = "" ) :
    UISliderBase( posX, posY, width, height, depth, label ) {
        dataPointer = data;
        m_min = minVal; m_max = maxVal;
    };
    void DoCallBack( void ) {
        if ( dataPointer != NULL ) {
            // Calculate new data value
            float sliderVal = float(sliderPos-x-7.0f)/float(w-12.5f);
            sliderVal *= (m_max-m_min);
            sliderVal += m_min;

            if ( sliderVal > m_max ) sliderVal = m_max;
            else if ( sliderVal < m_min ) sliderVal = m_min;
            *dataPointer = sliderVal;
            
            std::string varLabel = floatToString( *dataPointer );
            if ( varLabelID == -1 ) {
                varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH),
                                                  true, SLIDER_TEXT_SIZE, FONT_JURA );
            } else {
                g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
                g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
            }
        }
    };
    virtual void Update( void ) {
        UISliderBase::Update();
        if ( dataPointer != NULL ) {
            // Get new slider position for value
            float val = *dataPointer;
            if ( val > m_max ) val = m_max;
            else if ( val < m_min ) val = m_min;
            float sliderX = ( ( ((val-m_min)/float(m_max-m_min))*(w-12) ) + x + 6 );
            sliderPos = (int)sliderX;
            // Update labels
            if ( visible ) { 
                std::string varLabel = floatToString( *dataPointer );
                if ( varLabelID == -1 ) {
                varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH),
                                                  true, SLIDER_TEXT_SIZE, FONT_JURA );
                } else {
                    g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
                    g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
                }
            } else if ( varLabelID != -1 ) {
                g_uiMan->GetTextManager()->RemoveText(varLabelID);
                varLabelID = -1;
            }
        }
    };
    virtual void UpdatePos( int posX, int posY ) {
        UISliderBase::UpdatePos(posX, posY);
        if ( dataPointer != NULL && varLabelID != -1 ) {
            std::string varLabel = floatToString( *dataPointer );
            g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
        }
    };
};
template<> class UISlider<int> : public UISliderBase {
    int* dataPointer;
    int m_min, m_max;
public:
    UISlider( int posX, int posY,
             int width, int height,
             int depth,
             std::string label,
             int* data,
             int minVal, int maxVal,
             std::string texDefault = "",
             std::string texActive = "",
             std::string texPressed = "" ) :
    UISliderBase( posX, posY, width, height, depth, label ) {
        dataPointer = data;
        m_min = minVal; m_max = maxVal;
    };
    void DoCallBack( void ) {
        if ( dataPointer != NULL ) {
            // Calculate new data value
            float sliderVal = float(sliderPos-x-6)/(w-12);
            
            sliderVal *= (m_max-m_min);
            sliderVal += m_min;
            if ( sliderVal > m_max ) sliderVal = m_max;
            else if ( sliderVal < m_min ) sliderVal = m_min;
            *dataPointer = (int)sliderVal;
            std::string varLabel = intToString( *dataPointer );
            if ( varLabelID == -1 ) {
                varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH),
                                                  true, SLIDER_TEXT_SIZE, FONT_JURA );
            } else {
                g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
                g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
            }
        }
    };
    virtual void Update( void ) {
        UISliderBase::Update();
        if ( dataPointer != NULL ) {
            // Get new slider position for value
            int val = *dataPointer;
            if ( val > m_max ) val = m_max;
            else if ( val < m_min ) val = m_min;
            float sliderX = ( ( ((val-m_min)/float(m_max-m_min))*(w-12) ) + x + 6 );
            sliderPos = (int)sliderX;
            // Update labels
            if ( visible ) {
                std::string varLabel = intToString( *dataPointer );
                if ( varLabelID == -1 ) {
                varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH),
                                                  true, SLIDER_TEXT_SIZE, FONT_JURA );
                } else {
                    g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
                    g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
                }
            } else if ( varLabelID != -1 ) {
                g_uiMan->GetTextManager()->RemoveText(varLabelID);
                varLabelID = -1;
            }
        }
    };
    virtual void UpdatePos( int posX, int posY ) {
        UISliderBase::UpdatePos(posX, posY);
        if ( dataPointer != NULL && varLabelID != -1 ) {
            std::string varLabel = intToString( *dataPointer );
            g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
        }
    };
};
// Slider with callback mechanism
template <typename T, typename UnknownClass>
class UISliderCB : public UISliderBase {
    T* dataPointer;
    T m_min, m_max;

    void ( UnknownClass::*function )( void* );  // Pointer to a member function
    UnknownClass* object;                       // Pointer to an object instance
public:
    
    UISliderCB( int posX, int posY,
               int width, int height,
               int depth,
               std::string label,
               T* data,
               T minVal, T maxVal,
               UnknownClass* objectPtr,
               void( UnknownClass::*func )( void* ),
               std::string texDefault = "",
               std::string texActive = "",
               std::string texPressed = "" ) :
    UISliderBase( posX, posY, width, height, depth, label, texDefault, texActive, texPressed ) {
        dataPointer = data;
        m_min = minVal; m_max = maxVal;
    };
    virtual ~UISliderCB() {
        dataPointer = NULL;
    };
    void DoCallBack( void ) {
        if ( dataPointer != NULL ) {
            // Calculate new data value
            float sliderVal = float(sliderPos-x-7.0f)/float(w-12.5f);
            sliderVal *= (m_max-m_min);
            sliderVal += m_min;
            
            if ( sliderVal > m_max ) sliderVal = m_max;
            else if ( sliderVal < m_min ) sliderVal = m_min;
            *dataPointer = sliderVal;
            
            std::string varLabel = floatToString( *dataPointer );
            if ( varLabelID == -1 ) {
                varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH),
                                                                true, SLIDER_TEXT_SIZE, FONT_JURA );
            } else {
                g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
                g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
            }
        }
    };
    virtual void Update( void ) {
        UISliderBase::Update();
        if ( dataPointer != NULL ) {
            // Get new slider position for value
            float val = *dataPointer;
            if ( val > m_max ) val = m_max;
            else if ( val < m_min ) val = m_min;
            float sliderX = ( ( ((val-m_min)/float(m_max-m_min))*(w-12) ) + x + 6 );
            sliderPos = (int)sliderX;
            // Update labels
            if ( visible ) {
                std::string varLabel = floatToString( *dataPointer );
                if ( varLabelID == -1 ) {
                    varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH),
                                                                    true, SLIDER_TEXT_SIZE, FONT_JURA );
                } else {
                    g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
                    g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
                }
            } else if ( varLabelID != -1 ) {
                g_uiMan->GetTextManager()->RemoveText(varLabelID);
                varLabelID = -1;
            }
        }
    };
    virtual void UpdatePos( int posX, int posY ) {
        UISliderBase::UpdatePos(posX, posY);
        if ( dataPointer != NULL && varLabelID != -1 ) {
            std::string varLabel = floatToString( *dataPointer );
            g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
        }
    };
};
// Slider with callback mechanism
template <typename UnknownClass>
class UISliderCBFloat : public UISliderBase {
    float data;
    float m_min, m_max;
    
    void ( UnknownClass::*setFunction )( float );  // Pointer to a member function that takes a float
    float ( UnknownClass::*getFunction )();         // Pointer to a member function that returns a float
    UnknownClass* object;                           // Pointer to an object instance
public:
    
    UISliderCBFloat( int posX, int posY,
                    int width, int height,
                    int depth,
                    std::string label,
                    float minVal, float maxVal,
                    UnknownClass* objectPtr,
                    void( UnknownClass::*setFunc )( float ),
                    float( UnknownClass::*getFunc )(),
                    std::string texDefault = "",
                    std::string texActive = "",
                    std::string texPressed = "" ) :
    UISliderBase( posX, posY, width, height, depth, label, texDefault, texActive, texPressed ) {
        m_min = minVal; m_max = maxVal;
        object = objectPtr;
        setFunction = setFunc;
        getFunction = getFunc;
        data = objectPtr->getFunction();
        
    };
    virtual ~UISliderCBFloat() {
        object = NULL;
    };
    void DoCallBack( void ) {
        if ( object && setFunction ) {
            // Calculate new data value
            float sliderVal = float(sliderPos-x-7.0f)/float(w-12.5f);
            sliderVal *= (m_max-m_min);
            sliderVal += m_min;
            
            if ( sliderVal > m_max ) sliderVal = m_max;
            else if ( sliderVal < m_min ) sliderVal = m_min;
            data = sliderVal;
            object->setFunction(data);
            
            std::string varLabel = floatToString( data );
            if ( varLabelID == -1 ) {
                varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH),
                                                                true, SLIDER_TEXT_SIZE, FONT_JURA );
            } else {
                g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
                g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
            }
        }
    };
    virtual void Update( void ) {
        UISliderBase::Update();
        if ( object && getFunction ) {
            // Get new slider position for value
            float val = object->getFunction();
            if ( val != data ) {    // value changed
                data = val;
                if ( val > m_max ) val = m_max;
                else if ( val < m_min ) val = m_min;
                float sliderX = ( ( ((val-m_min)/float(m_max-m_min))*(w-12) ) + x + 6 );
                sliderPos = (int)sliderX;
                // Update labels
                if ( visible ) {
                    std::string varLabel = floatToString( val );
                    if ( varLabelID == -1 ) {
                        varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH),
                                                                        true, SLIDER_TEXT_SIZE, FONT_JURA );
                    } else {
                        g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
                        g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
                    }
                } else if ( varLabelID != -1 ) {
                    g_uiMan->GetTextManager()->RemoveText(varLabelID);
                    varLabelID = -1;
                }
            }
        }
    };
    virtual void UpdatePos( int posX, int posY ) {
        UISliderBase::UpdatePos(posX, posY);
        if ( object && varLabelID != -1 ) {
            std::string varLabel = floatToString( data );
            g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
        }
    };
};
// Sliders for options menu items
template <typename UnknownClass>
class UISliderOptionFloat : public UISliderBase {
    std::string name;   // Name of option var in dictionary
    float data;
    float m_min, m_max;
    
    void ( UnknownClass::*setFunction )( const std::string&, const float );
    float ( UnknownClass::*getFunction )( const std::string& );
    UnknownClass* object;
public:
    
    UISliderOptionFloat(int posX, int posY,
                        int width, int height,
                        int depth,
                        std::string optName,
                        std::string label,
                        float minVal, float maxVal,
                        UnknownClass* objectPtr,
                        void( UnknownClass::*setFunc )( const std::string&, const float ),
                        float( UnknownClass::*getFunc )( const std::string& ),
                        std::string texDefault = "",
                        std::string texActive = "",
                        std::string texPressed = "" ) :
    UISliderBase( posX, posY, width, height, depth, label, texDefault, texActive, texPressed ) {
        name = optName;
        m_min = minVal; m_max = maxVal;
        object = objectPtr;
        setFunction = setFunc;
        getFunction = getFunc;
        data = 0;
    };
    virtual ~UISliderOptionFloat() {
        object = NULL;
    };
    void DoCallBack( void ) {
        if ( object && setFunction ) {
            // Calculate new data value
            float sliderVal = float(sliderPos-x-7.0f)/float(w-12.5f);
            sliderVal *= (m_max-m_min);
            sliderVal += m_min;
            
            if ( sliderVal > m_max ) sliderVal = m_max;
            else if ( sliderVal < m_min ) sliderVal = m_min;
            data = sliderVal;
            ((*object).*setFunction)(name, data);
            
            std::string varLabel = floatToString( data );
            if ( varLabelID == -1 ) {
                varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH),
                                                                true, SLIDER_TEXT_SIZE, FONT_JURA );
            } else {
                g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
                g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
            }
        }
    };
    virtual void Update( void ) {
        UISliderBase::Update();
        if ( object && getFunction ) {
            // Get new slider position for value
            float val = ((*object).*getFunction)(name);
            if ( val != data ) {    // value changed
                data = val;
                if ( val > m_max ) val = m_max;
                else if ( val < m_min ) val = m_min;
                float sliderX = ( ( ((val-m_min)/float(m_max-m_min))*(w-12) ) + x + 6 );
                sliderPos = (int)sliderX;
                // Update labels
                if ( visible ) {
                    std::string varLabel = floatToString( val );
                    if ( varLabelID == -1 ) {
                        varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH),
                                                                        true, SLIDER_TEXT_SIZE, FONT_JURA );
                    } else {
                        g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
                        g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
                    }
                } else if ( varLabelID != -1 ) {
                    g_uiMan->GetTextManager()->RemoveText(varLabelID);
                    varLabelID = -1;
                }
            }
        }
    };
    virtual void UpdatePos( int posX, int posY ) {
        UISliderBase::UpdatePos(posX, posY);
        if ( object && varLabelID != -1 ) {
            std::string varLabel = floatToString( data );
            g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
        }
    };
};
template <typename UnknownClass>
class UISliderOptionInt : public UISliderBase {
    std::string name;   // Name of option var in dictionary
    int data;
    int m_min, m_max;
    
    void ( UnknownClass::*setFunction )( const std::string&, const int );
    int ( UnknownClass::*getFunction )( const std::string& );
    UnknownClass* object;
public:
    
    UISliderOptionInt(int posX, int posY,
                      int width, int height,
                      int depth,
                      std::string optName,
                      std::string label,
                      int minVal, int maxVal,
                      UnknownClass* objectPtr,
                      void( UnknownClass::*setFunc )( const std::string&, const int ),
                      int( UnknownClass::*getFunc )( const std::string& ),
                      std::string texDefault = "",
                      std::string texActive = "",
                      std::string texPressed = "" ) :
    UISliderBase( posX, posY, width, height, depth, label, texDefault, texActive, texPressed ) {
        name = optName;
        m_min = minVal; m_max = maxVal;
        object = objectPtr;
        setFunction = setFunc;
        getFunction = getFunc;
        data = 0;
    };
    virtual ~UISliderOptionInt() {
        object = NULL;
        setFunction = NULL;
        getFunction = NULL;
    };
    void DoCallBack( void ) {
        if ( object && setFunction ) {
            // Calculate new data value
            float sliderVal = float(sliderPos-x-7.0f)/float(w-12.5f);
            sliderVal *= (m_max-m_min);
            sliderVal += m_min;
            
            if ( sliderVal > m_max ) sliderVal = m_max;
            else if ( sliderVal < m_min ) sliderVal = m_min;
            data = sliderVal;
            ((*object).*setFunction)(name, data);
            
            std::string varLabel = floatToString( data );
            if ( varLabelID == -1 ) {
                varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH),
                                                                true, SLIDER_TEXT_SIZE, FONT_JURA );
            } else {
                g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
                g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
            }
        }
    };
    virtual void Update( void ) {
        UISliderBase::Update();
        if ( object && getFunction ) {
            // Get new slider position for value
            float val = ((*object).*getFunction)(name);
            if ( val != data ) {    // value changed
                data = val;
                if ( val > m_max ) val = m_max;
                else if ( val < m_min ) val = m_min;
                float sliderX = ( ( ((val-m_min)/float(m_max-m_min))*(w-12) ) + x + 6 );
                sliderPos = (int)sliderX;
                // Update labels
                if ( visible ) {
                    std::string varLabel = floatToString( val );
                    if ( varLabelID == -1 ) {
                        varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH),
                                                                        true, SLIDER_TEXT_SIZE, FONT_JURA );
                    } else {
                        g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
                        g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
                    }
                } else if ( varLabelID != -1 ) {
                    g_uiMan->GetTextManager()->RemoveText(varLabelID);
                    varLabelID = -1;
                }
            }
        }
    };
    virtual void UpdatePos( int posX, int posY ) {
        UISliderBase::UpdatePos(posX, posY);
        if ( object && varLabelID != -1 ) {
            std::string varLabel = floatToString( data );
            g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
        }
    };
};


// Specialized classes
//template<> class UISliderCB<float, UnknownClass> : public UISliderBase {
//    float* dataPointer;
//    float m_min, m_max;
//    void ( UnknownClass::*function )( void* );  // Pointer to a member function
//    UnknownClass* object;                       // Pointer to an object instance
//public:
//    UISlider( int posX, int posY,
//             int width, int height,
//             int depth,
//             std::string label,
//             float* data,
//             float minVal, float maxVal,
//             std::string texDefault = "",
//             std::string texActive = "",
//             std::string texPressed = "" ) :
//    UISliderBase( posX, posY, width, height, depth, label ) {
//        dataPointer = data;
//        m_min = minVal; m_max = maxVal;
//    };
//    void DoCallBack( void ) {
//        if ( dataPointer != NULL ) {
//            // Calculate new data value
//            float sliderVal = float(sliderPos-x-7.0f)/float(w-12.5f);
//            sliderVal *= (m_max-m_min);
//            sliderVal += m_min;
//            
//            if ( sliderVal > m_max ) sliderVal = m_max;
//            else if ( sliderVal < m_min ) sliderVal = m_min;
//            *dataPointer = sliderVal;
//            
//            std::string varLabel = floatToString( *dataPointer );
//            if ( varLabelID == -1 ) {
//                varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH),
//                                                                true, SLIDER_TEXT_SIZE, FONT_JURA );
//            } else {
//                g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
//                g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
//            }
//        }
//    };
//    virtual void Update( void ) {
//        UISliderBase::Update();
//        if ( dataPointer != NULL ) {
//            // Get new slider position for value
//            float val = *dataPointer;
//            if ( val > m_max ) val = m_max;
//            else if ( val < m_min ) val = m_min;
//            float sliderX = ( ( ((val-m_min)/float(m_max-m_min))*(w-12) ) + x + 6 );
//            sliderPos = (int)sliderX;
//            // Update labels
//            if ( visible ) {
//                std::string varLabel = floatToString( *dataPointer );
//                if ( varLabelID == -1 ) {
//                    varLabelID = g_uiMan->GetTextManager()->AddText(varLabel, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH),
//                                                                    true, SLIDER_TEXT_SIZE, FONT_JURA );
//                } else {
//                    g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
//                    g_uiMan->GetTextManager()->UpdateText(varLabelID, varLabel);
//                }
//            } else if ( varLabelID != -1 ) {
//                g_uiMan->GetTextManager()->RemoveText(varLabelID);
//                varLabelID = -1;
//            }
//        }
//    };
//    virtual void UpdatePos( int posX, int posY ) {
//        UISliderBase::UpdatePos(posX, posY);
//        if ( dataPointer != NULL && varLabelID != -1 ) {
//            std::string varLabel = floatToString( *dataPointer );
//            g_uiMan->GetTextManager()->UpdateTextPos(varLabelID, glm::vec3(x+w-(varLabel.length()*6.0f), y+(SLIDER_TEXT_SIZE/2),WIDGET_TEXT_DEPTH) );
//        }
//    };
//};

#endif /* UI_SLIDER_H */
