#ifndef LIST_H
#define LIST_H

#include "Widget.h"
#include "ListBehavior.h"
#include <vector>
#include <string>
#include <memory>

class Label;
class Renderer;

class List : public Widget
{
    friend class GUI;
public:
    ~List();
    
    void Draw(Renderer* renderer);

    void Update(const double deltaTime);
    
    void SetBehavior(ListBehavior* behavior) { _behavior = behavior; }

    void addEntry(const std::string text);
    void clearEntries();
    
    void setLabel(const std::string text);
    std::shared_ptr<Label> getLabel() { return _label; }
    
    void setFontSize(const int fontSize);
    
protected:
    List(Locator& locator);
    
    void OnInteract(const bool interact,
                    const glm::ivec2& coord);

    void refresh();
    
private:
    ListBehavior* _behavior;
    std::shared_ptr<Label> _label;
    std::vector<const std::string> _entries;
    std::vector<std::shared_ptr<Label>> _entryLabels;
    std::vector<std::shared_ptr<Widget>> _buttons;
    
    int _entryFontSize;
    int _maxLabels;
    int _currentSelection;
};

#endif /* LIST_H */
