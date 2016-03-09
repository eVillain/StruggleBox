#ifndef UI_TEXT_INPUT_H
#define UI_TEXT_INPUT_H

#include "UIWidget.h"
#include "Input.h"
#include "CoreTypes.h"

class UITextInputBase :
public UIWidget, public InputEventListener, public TextInputEventListener
{
public:
    UITextInputBase(int posX, int posY,
                    int width,int height,
                    int depth,
                    std::string description,
                    std::string defaultVal="<text input>" );
    virtual ~UITextInputBase( void );
    
    // Overriden from UIWidget
    virtual void UpdatePos( int posX, int posY );
    void CursorRelease(const glm::ivec2 coord);
    void CursorPress(const glm::ivec2 coord);
    void CursorHover(const glm::ivec2 coord, bool highlight);
    void Draw( Renderer* renderer );
    
    // Keyboard key and character callbacks
    void ReceiveChar(int key, int action);
    void ReceiveKey(int key, int action);
    // Text input operations
    void StartTextInput( void );
    void StopTextInput( void );
    void CancelTextInput( void );
    void UpdateTextInput( void );
    void Update( void ) {};
    void Clear( void );
    
    std::string GetText( void );
    void SetText( std::string newText );

protected:
    /// Override this in subclasses to actually perform callback
    virtual void DoCallBack() {};
    
private:
    // Text input attributes
    std::string inputValue;
    std::string label;                          // Text label on box
    int labelID;                                // Box text label ID number
    std::string startValue;                     // Starting input value
    
    bool grabKeyboardInput;
    int cursorPosition;
    int inputLabelID;
    
    double lastCursorBlink;
    bool cursorBlink;
    double lastBackSpace;

    bool OnEvent(const std::string& event,
                         const float& amount);
    void OnTextInput(const std::string& text);
};

// Text input widget with member callback function
template <class UnknownClass>
class UITextInput : public UITextInputBase {
private:
    // Text input callback attributes
    void ( UnknownClass::*function )( std::string );    // Pointer to a member function
    UnknownClass* object;                               // Pointer to an object instance
public:
    UITextInput( int posX, int posY,
                int width,int height,
                int depth,
                std::string description,
                std::string defaultVal="<enter text>",
                UnknownClass* objectPtr = NULL,
                void( UnknownClass::*func )( std::string ) = NULL ) :
    UITextInputBase( posX, posY, width, height, depth, description, defaultVal ),
    function(func),
    object(objectPtr)
    { };
    
    // Execute member callback function, sending text
    void DoCallBack() {
        if ( object && function ) {
            (*object.*function)( GetText() );
        }
    };
};

// Text input widget with static callback
class UITextInputSCB : public UITextInputBase {
private:
    // Pointer to text input callback function
    void ( *function )( std::string );
public:
    UITextInputSCB( int posX, int posY,
                int width,int height,
                   int depth,
                std::string description,
                void( *func )( std::string ) = NULL ) :
    UITextInputBase( posX, posY, width, height, depth, description ),
    function(func)
    { };
    
    // Execute static callback function, sending text
    void DoCallBack() {
        if ( function ) {
            (*function)( GetText() );
        }
    };
};

#endif /* UI_TEXT_INPUT_H */
