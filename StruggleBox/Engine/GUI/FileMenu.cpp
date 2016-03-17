#include "FileMenu.h"
#include "Locator.h"
#include "Text.h"
#include "GUI.h"
#include "List.h"
#include "Button.h"
#include "TextInput.h"
#include "FileUtil.h"
#include "Renderer.h"

FileMenu::FileMenu(Locator& locator) :
Widget(locator),
_label(nullptr),
_fileList(nullptr),
_button(nullptr),
_textInput(nullptr),
_savingFileMode(false),
_contentHeight(100)
{
    _fileList = _locator.Get<GUI>()->CreateWidget<List>();
    _button = _locator.Get<GUI>()->CreateWidget<Button>();
    _button->SetBehavior(new ButtonBehaviorMember<FileMenu>(this, &FileMenu::onButtonPressed));
    _textInput = _locator.Get<GUI>()->CreateWidget<TextInput>();
}

FileMenu::~FileMenu()
{
    if (_behavior)
    {
        delete _behavior;
        _behavior = nullptr;
    }
    _locator.Get<GUI>()->DestroyWidget(_fileList);
    _fileList = nullptr;
    if (_label)
    {
        _locator.Get<Text>()->DestroyLabel(_label);
        _label = nullptr;
    }
    if (_textInput)
    {
        _locator.Get<GUI>()->DestroyWidget(_textInput);
        _textInput = nullptr;
    }
    if (_button)
    {
        _locator.Get<GUI>()->DestroyWidget(_button);
        _button = nullptr;
    }
}

void FileMenu::Draw(Renderer* renderer)
{
    Widget::Draw(renderer);
    
    // Box for content under menu bar
    glm::vec3 drawPos = glm::vec3(_transform.GetPosition().x-(_size.x*0.5),
                                  _transform.GetPosition().y-(_size.y*0.5)-(_contentHeight),
                                  _transform.GetPosition().z);
    
    glm::ivec2 drawSize = glm::ivec2(_size.x, _contentHeight);
    Widget::DrawBase(renderer, drawPos, drawSize);
    

    
    if (_label)
    {
        if (_focus)
        {
            _label->setColor(COLOR_UI_TEXT_HIGHLIGHT);
        } else {
            
            _label->setColor(COLOR_UI_TEXT);
        }
    }
}

void FileMenu::Update(const double deltaTime)
{
    if (_transform.isDirty())
    {
        refresh();
        _transform.unflagDirty();
    }
}

void FileMenu::setFocus(const bool focus)
{
    _focus = focus;
}

void FileMenu::setActive(const bool active)
{
    _active = active;
}

void FileMenu::setVisibility(const bool visible)
{
    _visible = visible;
}

void FileMenu::OnInteract(const bool interact,
                          const glm::ivec2& coord)
{
    
}

void FileMenu::loadFromPath(const std::string path,
                            const std::string fileType)
{
    _savingFileMode = false;
    _path = path;
    _fileType = fileType;
    refresh();
    _fileList->SetBehavior(new ListBehaviorMember<FileMenu>(this,
                                                            &FileMenu::onFileSelected));
}

void FileMenu::saveToPath(const std::string path,
                          const std::string fileType)
{
    _savingFileMode = true;
    _path = path;
    _fileType = fileType;
    refresh();
    _fileList->SetBehavior(new ListBehaviorMember<FileMenu>(this,
                                                            &FileMenu::onFileSelected));
}

void FileMenu::setName(const std::string text)
{
    if (_label) {
        _label->setText(text);
    } else {
        _label = _locator.Get<Text>()->CreateLabel(text);
    }
}

void FileMenu::refresh()
{
    if (_label)
    {
        glm::vec3 labelPos = _transform.GetPosition();
        labelPos.z += 1;
        _label->getTransform().SetPosition(labelPos);
    }
    
    std::vector<std::string> files;
    if (_fileType.length() == 0)
    {
        FileUtil::GetAllFiles(_path, files);
    }
    else
    {
        FileUtil::GetFilesOfType(_path, _fileType, files);
    }
    
    _fileList->clearEntries();
    for (std::string fileName : files)
    {
        _fileList->addEntry(fileName);
    }
    int fileListHeight = _contentHeight - 26;
    glm::vec3 widgetPos = _transform.GetPosition();
    widgetPos.y -= 2+(fileListHeight/2)+_size.y/2;
    widgetPos.z += 1;

    _fileList->GetTransform().SetPosition(widgetPos);

    const glm::ivec2 fileListSize = glm::ivec2(_size.x-4, fileListHeight);
    _fileList->setSize(fileListSize);
    
    const int maxLabelLength = 32;
    std::string filePathLabel = _path + "*" + _fileType;
    if (filePathLabel.length() > maxLabelLength) {
        filePathLabel = "..." + filePathLabel.substr(filePathLabel.length()-maxLabelLength,
                                             maxLabelLength);
    }
    _fileList->setLabel(filePathLabel);
    widgetPos.y = _transform.GetPosition().y - (fileListHeight + 23);

    if (_button) {
        widgetPos.x = _transform.GetPosition().x+(_size.x/2)-32;
        _button->setSize(glm::ivec2(60, 22));
        _button->GetTransform().SetPosition(widgetPos);
    }
    
    if (_textInput) {
        _textInput->setSize(glm::ivec2(_size.x-(4+60), 22));
        widgetPos.x = 2+_transform.GetPosition().x-(_size.x/2)+_textInput->getSize().x/2;

        _textInput->GetTransform().SetPosition(widgetPos);
    }
}

void FileMenu::onFileSelected(const std::string& fileName)
{
    _currentFileName = fileName;
    if (_currentFileName.length() == 0) return;
    const std::string buttonLabel = _savingFileMode ? "Save" : "Load";
    _button->setLabel(buttonLabel);
    _textInput->setDefaultText(_currentFileName);

    refresh();
}

void FileMenu::onButtonPressed()
{
    if (!_behavior) return;
    if (_currentFileName.length() == 0) return;
    if (!_savingFileMode) {
        bool fileExists = FileUtil::DoesFileExist(_path, _currentFileName);
        if (!fileExists) return;
    }
    _behavior->Trigger(_path + "/" + _currentFileName);
}

