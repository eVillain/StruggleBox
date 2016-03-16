#ifndef LIST_H
#define LIST_H

#include "Widget.h"
#include <map>
#include <string>
#include <memory>

class Label;

class List : public Widget
{
public:
    ~List();
    
    void addEntry(const std::string text);

    void setLabel(const std::string text);
    std::shared_ptr<Label> getLabel() { return _label; }
    
protected:
    List(Locator& locator);
    
private:
    std::shared_ptr<Label> _label;
    std::map<const std::string, std::shared_ptr<Label>> _entries;
    std::vector<std::shared_ptr<Widget>> _buttons;
};

#endif /* LIST_H */
