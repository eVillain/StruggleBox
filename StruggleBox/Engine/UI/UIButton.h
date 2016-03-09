#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "UIWidget.h"
#include <functional>

enum UIButtonState {
    BUTTON_OFF = 0,
    BUTTON_ON = 1,
    BUTTON_TOGGLING_ON = 2,
    BUTTON_TOGGLING_OFF = 3,
};

enum UIButtonType {
    BUTTON_TYPE_DEFAULT,        // Default boxy button
    BUTTON_TYPE_NEON,           // Neony polygonal button
    BUTTON_TYPE_SIMPLE,         // No background, just text
    BUTTON_TYPE_TEXTURED,       // Texture background
};


/// Base widget class for buttons
class ButtonBase : public UIWidget
{
protected:
    // Button attributes
    std::string label;                          // Text label on button
    bool toggleable;                            // Whether button is toggleable
    int type;                                   // Type of button (default, neon, simple, textured)
    // Button parameters
    UIButtonState state;                        // State of button (off/on, toggling off/on)
    int labelID;                                // Button text label ID number
    
    static std::vector<ButtonBase*> _buttonList;

public:
    // Default constructor
    ButtonBase( std::string buttonLabel,
               int posX, int posY,
               int width,int height,
               int buttonType,
               bool canToggle,
               std::string texDefault = "",
               std::string texActive = "",
               std::string texPressed = "" );
    // Default destructor
    virtual ~ButtonBase( void );
    
    static ButtonBase* CreateButton( std::string buttonLabel,
                                    int posX, int posY,
                                    int width, int height,
                                    int buttonType = BUTTON_TYPE_DEFAULT,
                                    bool canToggle = false );
    static ButtonBase* CreateButton( std::string buttonLabel,
                                    int posX, int posY,
                                    int width, int height,
                                    bool canToggle = false,
                                    std::string texDefault = "",
                                    std::string texActive = "",
                                    std::string texPressed = "" );
    
    static void DeleteButton( ButtonBase* button );
    static void DetachButton( ButtonBase* button );

    static void DeleteButtonByLabel( std::string label );
    static void DeleteAllButtons( void );
    static void ToggleButton( ButtonBase* button );
    static void ToggleButtonByLabel( std::string label );
    
    void CursorRelease(const glm::ivec2 coord);
    void CursorPress(const glm::ivec2 coord);
    void CursorHover(const glm::ivec2 coord, bool highlight);
    void Draw( Renderer* renderer );
    
    virtual void Update( void );
    virtual void UpdatePos( int posX, int posY );
    virtual void UpdateLabel( const std::string newLabel );

    void DrawDefaultButton( Renderer* renderer );
    void DrawNeonButton( Renderer* renderer );
    void DrawSimpleButton( Renderer* renderer );
    void DrawTexturedButton( Renderer* renderer );
    virtual void DoCallBack() {};   // Override this in subclasses to actually perform callback
};


// This is a button class with an instance member C++ style callback mechanism
// and rendered using OpenGL

// Buttons should be created with the CreateButton method.
// This is an example of a button being created from an object of class MyClass,
// where the 'this' parameter can also be a pointer to any object of class MyClass
// and ButtonCB can be any instance method of MyClass with the correct signature (see below)
//UIButton<MyClass>::CreateButton(("Almighty Button"), 0,0,200,30, this, &MyClass::ButtonCB, NULL);

// This is an example callback function. We can assign a pointer to this function
// and it's owner which we can store and later call.
// void* clientdata is an optional user defined pointer to pass with callback.
// All callbacks should follow this signature (returning void, taking in a void*)
//void MyClass::TheButtonCallback(void *clientData)
//{
//	printf("Behold, for I am your button and I have thus been summoned\n");
//}
template <class UnknownClass>
class UIButton : private ButtonBase
{
private:
    // Button callback attributes
    void ( UnknownClass::*function )( void* );  // Pointer to a member function
    UnknownClass* object;                       // Pointer to an object instance
    void* callBackData;                         // Data pointer to pass with callback
public:
    UIButton( std::string buttonLabel,
             int posX, int posY,
             int width,int height,
             UnknownClass* objectPtr,
             void( UnknownClass::*func )( void* ),
             void* clientData,
             int buttonType,
             bool canToggle,
             std::string texDefault,
             std::string texActive,
             std::string texPressed ) :
    ButtonBase( buttonLabel, posX, posY, width, height,
               buttonType, canToggle,
               texDefault, texActive, texPressed ),
    function(func),
    object(objectPtr),
    callBackData(clientData)
    { };

    static UIButton* CreateButton( std::string buttonLabel,
                                  int posX, int posY,
                                  int width, int height,
                                  UnknownClass* objectPtr = NULL,
                                  void( UnknownClass::*func )( void* ) = NULL,
                                  void* clientData = NULL,
                                  int buttonType = BUTTON_TYPE_DEFAULT,
                                  bool canToggle = false,
                                  std::string texDefault = "",
                                  std::string texActive = "",
                                  std::string texPressed = "" ) {
        UIButton * newButton = new UIButton( buttonLabel, posX,posY,width,height,
                                 objectPtr, func, clientData, buttonType, canToggle,
                                 texDefault, texActive, texPressed);
        return newButton;
    }
    
    // Execute member callback function
    void DoCallBack() {
        if ( object && function ) {
            (*object.*function)( callBackData );
        }
    };
};

// Same as above but can be used to pass ints into callback instead of a void pointer
template <class UnknownClass>
class UIButtonSelector : private ButtonBase {
private:
    // Button callback attributes
    void ( UnknownClass::*function )( int );    // Pointer to a member function
    UnknownClass* object;                       // Pointer to an object instance
    int callBackData;                           // Selection data to pass with callback
public:
    UIButtonSelector( std::string buttonLabel,
                     int posX, int posY,
                     int width,int height,
                     UnknownClass* objectPtr,
                     void( UnknownClass::*func )( int ),
                     int clientData,
                     int buttonType,
                     bool canToggle,
                     std::string texDefault,
                     std::string texActive,
                     std::string texPressed ) :
    ButtonBase( buttonLabel, posX, posY, width, height,
               buttonType, canToggle,
               texDefault, texActive, texPressed ),
    function(func),
    object(objectPtr),
    callBackData(clientData)
    { };
    
    static UIButtonSelector* CreateButton( std::string buttonLabel,
                                          int posX, int posY,
                                          int width, int height,
                                          UnknownClass* objectPtr = NULL,
                                          void( UnknownClass::*func )( int ) = NULL,
                                          int clientData = 0,
                                          int buttonType = BUTTON_TYPE_DEFAULT,
                                          bool canToggle = false,
                                          std::string texDefault = "",
                                          std::string texActive = "",
                                          std::string texPressed = "" ) {
        UIButtonSelector * newButton;
        newButton = new UIButtonSelector( buttonLabel, posX,posY,width,height,
                                         objectPtr, func, clientData, buttonType, canToggle,
                                         texDefault, texActive, texPressed );
        return newButton;
    }
    
    // Execute member callback function
    void DoCallBack() {
        if ( object && function ) {
            (*object.*function)( callBackData );
        }
    };
};


// Button which sets variable of unknown type
template <typename UnknownType>
class UIButtonVari : private ButtonBase {
private:
    // Button callback attributes
    UnknownType* object;                    // Pointer to the object owner
    UnknownType callBackData;
public:
    UIButtonVari( std::string buttonLabel,
                 int posX, int posY,
                 int width,int height,
                 UnknownType* objectP,
                 UnknownType clientData,
                 int buttonType,
                 bool canToggle,
                 std::string texDefault,
                 std::string texActive,
                 std::string texPressed ) :
    ButtonBase( buttonLabel, posX, posY, width, height,
               buttonType, canToggle,
               texDefault, texActive, texPressed ),
    object(objectP),
    callBackData(clientData)
    { };
    
    static UIButtonVari* CreateButton( std::string buttonLabel,
                                      int posX, int posY,
                                      int width, int height,
                                      UnknownType* objectP = NULL,
                                      UnknownType clientData = NULL,
                                      int buttonType = BUTTON_TYPE_DEFAULT,
                                      bool canToggle = false,
                                      std::string texDefault = "",
                                      std::string texActive = "",
                                      std::string texPressed = "" ) {
        UIButtonVari * newButton;
        newButton = new UIButtonVari(buttonLabel, posX,posY,width,height,
                                     objectP, clientData, buttonType, canToggle,
                                     texDefault, texActive, texPressed);
        return newButton;
    }
    
    // Execute member callback function
    void DoCallBack() {
        if ( object ) {
            UnknownType* obj = object;
            *obj = callBackData;
        }
    };
};



// This is a button class with a static member or C style callback mechanism

// We define a function pointer type. ButtonCallback is a pointer to a function that
// is called when the button is pressed or toggled
typedef void (*ButtonCallback)(void* clientData);

// This is an example callback function. Notice that it's type is the same
// an the ButtonCallback type. We can assign a pointer to this function which
// we can store and later call.
// void* clientdata is an optional user defined pointer to pass with callback
//static void TheButtonCallback(void *clientData)
//{
//	printf("Behold, for I am your button and I have thus been summoned\n");
//}
class UIButtonSCB : private ButtonBase {
private:
    ButtonCallback callbackFunction;            // Pointer to a static callback function
    void* callBackData;                         // Data pointer to pass with callback
public:
    UIButtonSCB( std::string buttonLabel,
                int posX, int posY,
                int width,int height,
                ButtonCallback callBack,
                void* clientData,
                int buttonType,
                bool canToggle,
                std::string texDefault,
                std::string texActive,
                std::string texPressed) :
    ButtonBase( buttonLabel, posX, posY, width, height,
               buttonType, canToggle,
               texDefault, texActive, texPressed ),
    callbackFunction(callBack),
    callBackData(clientData)
    { };
    
    static UIButtonSCB* CreateButton( std::string buttonLabel,
                             int posX, int posY,
                             int width, int height,
                             ButtonCallback callBack,
                             void* clientData = NULL,
                             int buttonType = BUTTON_TYPE_DEFAULT,
                             bool canToggle = false,
                             std::string texDefault = "",
                             std::string texActive = "",
                             std::string texPressed = "" ) {
        UIButtonSCB * newButton;
        newButton = new UIButtonSCB(buttonLabel, posX,posY,width,height,
                                    callBack, clientData, buttonType, canToggle,
                                    texDefault, texActive, texPressed);
        return newButton;
    }
    // Execute static callback function
    void DoCallBack() {
        if ( callbackFunction )
        { callbackFunction( callBackData ); }
    };
};

class UIButtonToggle : private ButtonBase {
private:
    bool* callBackData;
public:
    UIButtonToggle( std::string buttonLabel,
                int posX, int posY,
                int width,int height,
                bool* clientData,
                int buttonType,
                bool canToggle,
                std::string texDefault,
                std::string texActive,
                std::string texPressed) :
    ButtonBase( buttonLabel, posX, posY, width, height,
               buttonType, canToggle,
               texDefault, texActive, texPressed ),
    callBackData(clientData)
    { };
    
    static UIButtonToggle* CreateButton( std::string buttonLabel,
                                     int posX, int posY,
                                     int width, int height,
                                     bool* clientData = NULL,
                                     int buttonType = BUTTON_TYPE_DEFAULT,
                                     bool canToggle = false,
                                     std::string texDefault = "",
                                     std::string texActive = "",
                                     std::string texPressed = "" ) {
        UIButtonToggle * newButton;
        newButton = new UIButtonToggle(buttonLabel, posX,posY,width,height,
                                    clientData, buttonType, canToggle,
                                    texDefault, texActive, texPressed);
        return newButton;
    }
    // Execute static callback function
    void DoCallBack() {
        if ( callBackData ) {
            *callBackData = !(*callBackData );
            if ( *callBackData ) {
                state = BUTTON_ON;
            } else {
                state = BUTTON_OFF;
            }
        }

    };
};

// Last addition, the lambda button
// As the name implies this button stores a lambda code block
// And of course executes it when pressed
class UIButtonLambda : private ButtonBase {
    std::function<void()> function;
    
    
public:
    UIButtonLambda( std::string buttonLabel,
                   int posX, int posY,
                   int width,int height,
                   std::function<void()> func,
                   int buttonType,
                   bool canToggle,
                   std::string texDefault,
                   std::string texActive,
                   std::string texPressed ) :
    ButtonBase( buttonLabel, posX, posY, width, height,
               buttonType, canToggle,
               texDefault, texActive, texPressed ),
    function(func)
    { };
    
    static UIButtonLambda* CreateButton( std::string buttonLabel,
                                        int posX, int posY,
                                        int width, int height,
                                        std::function<void()> func,
                                        int buttonType = BUTTON_TYPE_DEFAULT,
                                        bool canToggle = false,
                                        std::string texDefault = "",
                                        std::string texActive = "",
                                        std::string texPressed = "" ) {
        UIButtonLambda * newButton = new UIButtonLambda( buttonLabel, posX,posY,width,height,
                                            func, buttonType, canToggle,
                                            texDefault, texActive, texPressed);
        return newButton;
    }
    
    // Execute member callback function
    void DoCallBack() {
        if ( function ) {
            function();
        }
    };
};

#endif */ UI_BUTTON_H */
