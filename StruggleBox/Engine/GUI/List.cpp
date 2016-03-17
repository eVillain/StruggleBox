#include "List.h"
#include "Locator.h"
#include "Text.h"
#include "Log.h"
#include "GUI.h"

List::List(Locator& locator) :
Widget(locator),
_behavior(nullptr),
_label(nullptr),
_entryFontSize(14),
_maxLabels(1),
_currentSelection(-1)
{
    
}

List::~List()
{
    if (_behavior)
    {
        delete _behavior;
        _behavior = nullptr;
    }
    
    if (_label)
    {
        _locator.Get<Text>()->DestroyLabel(_label);
    }
    
    Text* text = _locator.Get<Text>();
    for (auto label : _entryLabels) {
        text->DestroyLabel(label);
    }
}

void List::Draw(Renderer *renderer)
{
    Widget::Draw(renderer);
    
}

void List::Update(const double deltaTime)
{
    if (_transform.isDirty())
    {
        refresh();
        _transform.unflagDirty();
    }
}

void List::OnInteract(const bool interact,
                      const glm::ivec2& coord)
{
    if (_focus &&
        !interact)
    {
        int top = (_transform.GetPosition().y + _size.y/2);
        if (_label) top -= _label->getFontSize() + 2;
        
        float distToTop = top - coord.y;
        float ratio = distToTop / _size.y;
        int label = ratio * _maxLabels;
        
        if (_entryLabels.size() > label)
        {
            // Clicked on a label
            if (_currentSelection != -1) {
                _entryLabels[_currentSelection]->setColor(COLOR_UI_TEXT);
            }
            _currentSelection = label;
            _entryLabels[_currentSelection]->setColor(COLOR_UI_TEXT_HIGHLIGHT);
            
            if (_behavior) {
                _behavior->Trigger(_entries[_currentSelection]);
            }
        }
    }
}

void List::addEntry(const std::string text)
{
    _entries.push_back(text);
}

void List::clearEntries()
{
    _entries.clear();
}

void List::setLabel(const std::string text)
{
    if (_label) {
        _label->setText(text);
    } else {
        _label = _locator.Get<Text>()->CreateLabel(text);
    }
}

void List::setFontSize(const int fontSize)
{
    _entryFontSize = fontSize;

    refresh();
}

void List::refresh()
{
    if (_label)
    {
        glm::vec3 labelPos = _transform.GetPosition();
        labelPos.y += (_size.y/2.0f) - (_label->getFontSize()/2 + 2);
        _label->getTransform().SetPosition(labelPos);
    }
    int contentSize = _size.y;
    if (_label) contentSize -= _label->getFontSize();
    _maxLabels = (float)contentSize / _entryFontSize - 0.5f;
    
    printf("max labels: %i, size: %i, font: %i\n", _maxLabels, _size.y, _entryFontSize);
    if (_maxLabels <= 0) _maxLabels = 1;
    
    size_t wantedLabelNum = _entries.size();
    if (wantedLabelNum > _maxLabels) wantedLabelNum = _maxLabels;
    
    while (_entryLabels.size() != wantedLabelNum)
    {
        if (_entryLabels.size() < wantedLabelNum) {
            auto label = _locator.Get<Text>()->CreateLabel("");
            _entryLabels.push_back(label);
        } else if (_entryLabels.size() > wantedLabelNum) {
            _entryLabels.pop_back();
        }
    }
    glm::vec3 widgetPos = _transform.GetPosition();
    widgetPos.y += (_size.y/2) - (2 + _entryFontSize);
    widgetPos.z += 1;
    
    for (size_t i=0; i < _entryLabels.size(); i++)
    {
        auto label = _entryLabels[i];
        const std::string entry = _entries[i];
        label->setText(entry);
        label->setFontSize(_entryFontSize);
        widgetPos.y -= _entryFontSize;
        label->getTransform().SetPosition(widgetPos);
        if (_currentSelection != -1 &&
            i == _currentSelection) {
            _entryLabels[i]->setColor(COLOR_UI_TEXT_HIGHLIGHT);
        } else {
            _entryLabels[i]->setColor(COLOR_UI_TEXT);
        }
    }
}
