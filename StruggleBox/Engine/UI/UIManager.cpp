#include "UIManager.h"
#include "UIWidget.h"

#include "HyperVisor.h"
#include "FileUtil.h"
#include "ShaderManager.h"
#include "Options.h"
#include "Renderer.h"
#include "Input.h"

#define DEBUG_WIDGETS 0

UIManager::UIManager()
{
    uiBatch = NULL;
    uiShaderColor = NULL;
    uiShaderTex = NULL;
    widget_vao = -1;
    widget_vbo = -1;
}

UIManager::~UIManager()
{
}

void UIManager::Initialize(Locator &locator)
{
    g_textMan = locator.Get<TextManager>();
    g_renderer = locator.Get<Renderer>();
    g_input = locator.Get<Input>();
    g_input->RegisterMouseObserver(this);
    g_input->RegisterEventObserver(this);
    UIWidget::SetUIManager(this);
}

void UIManager::Terminate()
{
    g_input->UnRegisterMouseObserver(this);
    g_input->UnRegisterEventObserver(this);
    CleanUp();
    UIWidget::SetUIManager(NULL);
}

void UIManager::AddWidget(UIWidget *widget)
{
    for ( unsigned int i=0; i<_widgetList.size(); i++ ) {
        UIWidget* w = _widgetList[i];
        if ( widget == w ) {
            if ( DEBUG_WIDGETS ) {
                printf("[WidgetMan] WARNING: widget not added, already in list\n");
            }
            return;
        }
    }
    _widgetList.push_back(widget);
    if ( DEBUG_WIDGETS ) {
        printf("[WidgetMan] widget added, %lu total\n", _widgetList.size());
    }
}

void UIManager::RemoveWidget( UIWidget *widget )
{
    for ( unsigned int i=0; i<_widgetList.size(); i++ ) {
        UIWidget* w = _widgetList[i];
        if ( widget == w ) {
            _widgetList.erase( _widgetList.begin()+i );
            if ( DEBUG_WIDGETS ) {
                printf("[WidgetMan] widget removed, %lu left\n", _widgetList.size());
            }
            return;
        }
    }
    if ( DEBUG_WIDGETS ) {
        printf("[WidgetMan] WARNING: widget not removed, not in list!\n");
    }
}

void UIManager::LoadUIBatch( const std::string uiBatchFile )
{
    if ( uiBatch != NULL ) { UnloadUIBatch(); }
    std::string spritePath = FileUtil::GetPath().append("Data/GFX/");
    uiBatch = new SpriteBatch();
    uiBatch->LoadFromFile( spritePath, uiBatchFile );
    
    if ( DEBUG_WIDGETS ) {
        printf("[WidgetMan] loaded UI SpriteBatch\n");
    }
}
void UIManager::UnloadUIBatch()
{
    if ( uiBatch == NULL ) { return; }
    delete uiBatch;
    uiBatch = NULL;
    if ( DEBUG_WIDGETS ) {
        printf("[WidgetMan] unloaded UI SpriteBatch\n");
    }
}
void UIManager::LoadUIShaders()
{
    if ( uiShaderColor != NULL ) { return; }
    uiShaderColor = ShaderManager::LoadFromFile("f_ui_color.vsh", "f_ui_color.fsh");
    if ( uiShaderTex != NULL ) { return; }
    uiShaderTex = ShaderManager::LoadFromFile("f_ui_tex.vsh", "f_ui_tex.fsh");
    glGenVertexArrays(1, &widget_vao);
    glBindVertexArray(widget_vao);
    glGenBuffers(1, &widget_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, widget_vbo);
    if ( DEBUG_WIDGETS ) {
        printf("[WidgetMan] loaded UI shaders\n");
    }
}
void UIManager::UnloadUIShaders()
{
    if ( uiShaderColor ) {
        ShaderManager::ClearShader(uiShaderColor);
        uiShaderColor = NULL;
    }
    if ( uiShaderTex ) {
        ShaderManager::ClearShader(uiShaderTex);
        uiShaderTex = NULL;
    }
    printf("[UIWidget] unloaded ui shaders\n");
}

bool UIManager::OnCursorHover(const glm::ivec2& coord)
{
    bool overWidget = false;
    for ( unsigned int i=0; i<_widgetList.size(); i++ ) {
        UIWidget* w = _widgetList[i];
        if( w->PointTest(coord) ) {
            w->CursorHover(coord, true);
            overWidget = true;
        } else {
            w->CursorHover(coord, false);
        }
    }
    return overWidget;
}

bool UIManager::OnCursorPress(const glm::ivec2& coord)
{
    for ( unsigned int i=0; i<_widgetList.size(); i++ ) {
        UIWidget* w = _widgetList[i];
        if( w->PointTest(coord) ) {
            w->CursorPress(coord);
            return true;
        }
    }
    return false;
}

bool UIManager::OnCursorRelease(const glm::ivec2& coord)
{
    for ( unsigned int i=0; i<_widgetList.size(); i++ ) {
        UIWidget* w = _widgetList[i];
        if( w->PointTest(coord) ) {
            w->CursorRelease(coord);
            return true;
        }
    }
    return false;
}

void UIManager::RenderWidgets()
{
    if ( g_renderer == NULL ) return;
    
    if ( uiShaderColor == NULL || uiShaderTex == NULL ) { LoadUIShaders(); }
    // Prepare shaders by passing in MVP matrix
    glm::mat4 MVP;
    g_renderer->GetUIMatrix(MVP);
    if ( uiShaderColor ) {
        uiShaderColor->Begin();
        uiShaderColor->setUniformM4fv("MVP", MVP);
        uiShaderColor->End();
    }
    if ( uiShaderTex ) {
        uiShaderTex->Begin();
        uiShaderTex->setUniformM4fv("MVP", MVP);
        uiShaderTex->End();
    }
    glBindVertexArray(widget_vao);
    glBindBuffer(GL_ARRAY_BUFFER, widget_vbo);
    
    for ( unsigned int i=0; i < _widgetList.size(); i++ ) {
        // Render widget at i
        UIWidget* w = _widgetList[i];
        w->Draw( g_renderer );
    }
    glUseProgram(0);
    glBindVertexArray(0);
}

void UIManager::CleanUp()
{
    for ( unsigned int i=0; i < _widgetList.size(); i++ ) {
        UIWidget* w = _widgetList[i];
        delete w;
    }
    _widgetList.clear();
    UnloadUIBatch();
    UnloadUIShaders();
}

void UIManager::Refresh()
{
    UnloadUIShaders();
}


bool UIManager::OnEvent( const std::string& event, const float& amount )
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

bool UIManager::OnMouse( const glm::ivec2& coord )
{
    _currentMouseCoord = ConvertSDLCoordToScreen(coord);
//    if ( _mouseDrag) {
//        return OnCursorDrag(coord);
//    }
    return OnCursorHover(_currentMouseCoord);
}

glm::ivec2 UIManager::ConvertSDLCoordToScreen(const glm::ivec2& coord) const
{
    glm::ivec2 windowSize = glm::ivec2(g_renderer->windowWidth, g_renderer->windowHeight);
    return glm::ivec2(coord.x - windowSize.x*0.5, windowSize.y*0.5 - coord.y);
}
