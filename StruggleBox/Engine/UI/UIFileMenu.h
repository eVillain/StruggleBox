//
//  UIFileMenu.h
//  Bloxelizer
//
//  Created by The Drudgerist on 7/28/13.
//
//

#ifndef NGN_UIFILEMENU_H
#define NGN_UIFILEMENU_H
#include "UIWidget.h"
#include "UITextInput.h"
#include "UIButton.h"
#include "UIScrollerList.h"

class UIFileMenuBase : public UIWidget {
    std::string label;
    int labelID;

    std::string filePath;
    std::string fileType;
    std::string defaultFile;
    std::string selectedFile;
    std::vector<std::string> fileList;

    UITextInput<UIFileMenuBase>* textWidget;
    ButtonBase* selectButton;
    ButtonBase* closeButton;
    
    UIScrollerList<UIFileMenuBase>* scrollerList;
    
    bool isLoading;
    double lastTimeClicked;
    
    // Refresh items list
    void RefreshItems();
    
    // File clicked callback
    void SelectFileCB( std::string file );
    void SelectCB( void* unused );
    void CloseCB( void* unused );
    void ReceiveFileName( std::string fileName );
    virtual void DoCallBack( std::string chosenFile ) {};
    
    virtual void UpdatePos( int posX, int posY );
    virtual void Draw( Renderer* renderer );
    // Test cursor events on widgets
    virtual bool PointTest(const glm::ivec2 coord);
    bool CheckHover(const glm::ivec2 coord);
    bool CheckPress(const glm::ivec2 coord);
    bool CheckRelease(const glm::ivec2 coord);
    // Override these for different cursor events
    virtual void CursorHover(const glm::ivec2 coord, bool highlight);
    virtual void CursorPress(const glm::ivec2 coord);
    virtual void CursorRelease(const glm::ivec2 coord);
    
public:
    int                             contentHeight;

    UIFileMenuBase( int posX, int posY,
                   int width, int height,
                   std::string path,
                   std::string type = "",
                   std::string title = "Select thy file:",
                   std::string defaultVal = "defaultFile",
                   bool loading = true,
                   std::string texDefault = "",
                   std::string texActive = "",
                   std::string texPressed = "" );
    ~UIFileMenuBase();
};

template <class UnknownClass>
class UIFileMenu : public UIFileMenuBase {
    // File selector callback attributes
    void ( UnknownClass::*function )( std::string );  // Pointer to a member function
    UnknownClass* object;                       // Pointer to an object instance
public:
    UIFileMenu( int posX, int posY,
               int width, int height,
               std::string path,
               std::string type = "",
               std::string title = "Select thy file:",
               std::string defaultVal = "defaultFile",
               bool loading = true,
               UnknownClass* objectPtr = NULL,
               void( UnknownClass::*func )( std::string ) = NULL ) :
    UIFileMenuBase( posX, posY, width, height, path, type, title, defaultVal, loading),
    function(func),
    object(objectPtr)
    { };
    void DoCallBack( std::string chosenFile ) {
        if ( object && function ) {
            (*object.*function)( chosenFile );
        }
    };
};


#endif
