#ifndef BUTTON_LABELED_H
#define BUTTON_LABELED_H

#include "Button.h"
#include "Label.h"
#include <memory>

class ButtonLabeled : public Button
{
public:
    ButtonLabeled();
    
    virtual void Draw(Renderer* renderer);

    void SetLabel(std::shared_ptr<Label> label) { _label = label; }
private:
    std::shared_ptr<Label> _label;
};

#endif /* BUTTON_LABELED_H */
