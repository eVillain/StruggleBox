#include "List.h"
#include "Locator.h"
#include "Text.h"
#include "Log.h"

List::List(Locator& locator) :
Widget(locator),
_label(nullptr)
{
    
}

List::~List()
{
    if (_label) {
        _locator.Get<Text>()->DestroyLabel(_label);
        _label = nullptr;
    }
}

void List::addEntry(const std::string text)
{
    if (_entries.find(text) != _entries.end()) {
        Log::Warn("Trying to add same list entry twice!");
    } else {
        auto label = _locator.Get<Text>()->CreateLabel(text);
        label->setFontSize(_size.y-4);
        _entries[text] = label;
    }
}

void List::setLabel(const std::string text)
{
    if (_label) {
        _label->setText(text);
    } else {
        _label = _locator.Get<Text>()->CreateLabel(text);
    }
    printf("[Button] set label %s use count %lu \n",
           _label->getText().c_str(), _label.use_count());
}
