#include "GUI.h"
#include "Widget.h"
#include "Log.h"
#include "Locator.h"
#include "Renderer.h"

GUI::GUI()
{
    _spriteBatch = nullptr;
    _shaderColor = nullptr;
    _shaderTex = nullptr;
    _vao = -1;
    _vbo = -1;
}

bool GUI::Initialize(Locator& locator)
{
    Log::Info("GUI initializing...");
    _input = locator.Get<Input>();
    _input->RegisterEventObserver(this);
    _input->RegisterMouseObserver(this);
    _mouseDrag = false;
    
    _renderer = locator.Get<Renderer>();
    return true;
}

bool GUI::Terminate()
{
    Log::Info("GUI terminating...");
    _input->UnRegisterEventObserver(this);
    _input->UnRegisterMouseObserver(this);
    return true;
}

void GUI::AddChild(std::shared_ptr<Widget> widget)
{
    _widgets.push_back(widget);
}

void GUI::RemoveChild(std::shared_ptr<Widget> widget)
{
    std::vector<std::shared_ptr<Widget>>::iterator it;
    it = std::find(_widgets.begin(), _widgets.end(), widget);
    if ( it != _widgets.end() )
    {
        _widgets.erase(it);
    } else {
        Log::Warn("GUI tried to remove a non-existent widget!");
    }
}

void GUI::Update(const double deltaTime)
{
    for (std::shared_ptr<Widget> widget : _widgets)
    {
        widget->Update();
    }
}

void GUI::Draw(Renderer* renderer)
{
    for (std::shared_ptr<Widget> widget : _widgets)
    {
        widget->Draw(renderer);
    }
}

bool GUI::OnCursorPress(const glm::ivec2& coord)
{
    for (std::shared_ptr<Widget> widget : _widgets)
    {
        if( widget->Contains(coord) )
        {
            widget->OnInteract(true, coord);
            return true;
        }
    }
    return false;
}

bool GUI::OnCursorDrag(const glm::ivec2& coord)
{
    for (std::shared_ptr<Widget> widget : _widgets)
    {
        if( widget->Contains(coord) )
        {
            widget->OnDrag(coord);
            return true;
        }
    }
    return false;
}

bool GUI::OnCursorRelease(const glm::ivec2& coord)
{
    for (std::shared_ptr<Widget> widget : _widgets)
    {
        if( widget->Contains(coord) )
        {
            widget->OnInteract(false, coord);
            return true;
        }
    }
    return false;
}

bool GUI::OnCursorHover(const glm::ivec2& coord)
{
    bool overWidget = false;
    for (std::shared_ptr<Widget> widget : _widgets)
    {
        if( widget->Contains(coord) )
        {
            widget->SetFocus(true);
            overWidget = true;
        } else {
            widget->SetFocus(false);
        }
    }
    if ( !overWidget ) { _mouseDrag = false; }
    return overWidget;
}

bool GUI::OnEvent(const std::string& event,
                  const float& amount)
{
    if ( event == "shoot")
    {
        if ( amount == 1 )
        {
            bool clickedWidget = OnCursorPress(_currentMouseCoord);
            if ( clickedWidget ) { _mouseDrag = true; }
            return clickedWidget;
        }
        else if ( amount == -1 )
        {
            _mouseDrag = false;
            return OnCursorRelease(_currentMouseCoord);
        }
    }
    return false;
}

bool GUI::OnMouse(const glm::ivec2& coord)
{
    _currentMouseCoord.x = coord.x;
    _currentMouseCoord.y = coord.y;
    if ( _mouseDrag) {
        return OnCursorDrag(coord);
    }
    return OnCursorHover(_currentMouseCoord);
}
