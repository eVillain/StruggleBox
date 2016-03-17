#ifndef FILE_MENU_H
#define FILE_MENU_H

#include "Widget.h"
#include "ListBehavior.h"
#include <memory>

class Label;
class List;
class Button;
class TextInput;

class FileMenu : public Widget
{
    friend class GUI;
public:
    ~FileMenu();

    void Draw(Renderer* renderer);
    void Update(const double deltaTime);
    void setFocus(const bool focus);
    void setActive(const bool active);
    void setVisibility(const bool visible);

    void setBehavior(ListBehavior* behavior) { _behavior = behavior; }

    void loadFromPath(const std::string path,
                      const std::string fileType = "");
    void saveToPath(const std::string path,
                    const std::string fileType = "");
    
    void setName(const std::string text);
    std::shared_ptr<Label> getLabel() { return _label; }
    
    void setContentHeight(const int contentHeight) { _contentHeight = contentHeight; }
protected:
    FileMenu(Locator& locator);
    
    void OnInteract(const bool interact,
                    const glm::ivec2& coord);
    
private:
    void refresh();
    void onFileSelected(const std::string& fileName);
    
    void onButtonPressed();
    
    ListBehavior* _behavior;
    std::shared_ptr<Label> _label;
    std::shared_ptr<List> _fileList;
    std::shared_ptr<Button> _button;
    std::shared_ptr<TextInput> _textInput;
    
    std::string _path;
    std::string _fileType;
    std::string _currentFileName;
    bool _savingFileMode;
    int _contentHeight;
};

#endif /* FILE_MENU_H */
