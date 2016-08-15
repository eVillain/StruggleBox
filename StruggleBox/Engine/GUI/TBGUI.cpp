#include "TBGUI.h"
#include "Log.h"
#include "PathUtil.h"
#include "animation/tb_widget_animation.h"
#include "tb_language.h"
#include "tb_font_renderer.h"
#include "renderers/tb_renderer_gl.h"
#include "tb_msg.h"
#include "tb_system.h"
#include "tb_layout.h"

TBGUI::TBGUI()
{
	Log::Debug("[TBGUI] Constructor, instance at %p", this);
	_renderer = std::make_unique<tb::TBRendererGL>();
	tb_core_init(_renderer.get());
	const std::string guiPath = PathUtil::GUIPath();

	// Load language file
	std::string langPath = guiPath + "language/lng_en.tb.txt";
	if (!tb::g_tb_lng->Load(langPath.c_str()))
	{
		Log::Error("[TBGUI] Failed to load language from %s", langPath.c_str());
	}
	else
	{
		Log::Debug("[TBGUI] Loaded language from %s", langPath.c_str());
	}

	// Load the default skin, and override skin that contains the graphics specific to the demo.
	const std::string skinPath = guiPath + "default_skin/skin.tb.txt";
	const std::string skinOverridePath = guiPath + "skin/skin.tb.txt";
	// Load the default skin, and override skin that contains the graphics specific to the demo.
	if (!tb::g_tb_skin->Load(skinPath.c_str(), skinOverridePath.c_str()))
	{
		Log::Error("[TBGUI] Failed to load skin from %s", skinPath.c_str());
	}
	else
	{
		Log::Debug("[TBGUI] Loaded skin from %s", skinPath.c_str());
	}

	// Register font renderers.
#ifdef TB_FONT_RENDERER_TBBF
	void register_tbbf_font_renderer();
	register_tbbf_font_renderer();
#endif
#ifdef TB_FONT_RENDERER_STB
	void register_stb_font_renderer();
	register_stb_font_renderer();
#endif
#ifdef TB_FONT_RENDERER_FREETYPE
	void register_freetype_font_renderer();
	register_freetype_font_renderer();
#endif

	// Add fonts we can use to the font manager.
	const std::string fontPath = PathUtil::FontsPath() + "Muli-Regular.ttf";
	tb::g_font_manager->AddFontInfo(fontPath.c_str(), "Muli");


	// Set the default font description for widgets to one of the fonts we just added
	tb::TBFontDescription fd;
	fd.SetID(TBIDC("Muli"));

	fd.SetSize(tb::g_tb_skin->GetDimensionConverter()->DpToPx(14));
	tb::g_font_manager->SetDefaultFontDescription(fd);

	// Create the font now.
	tb::TBFontFace *font = tb::g_font_manager->CreateFontFace(tb::g_font_manager->GetDefaultFontDescription());

	// Render some glyphs in one go now since we know we are going to use them. It would work fine
	// without this since glyphs are rendered when needed, but with some extra updating of the glyph bitmap.
	if (font)
		font->RenderGlyphs(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~•·åäöÅÄÖ");

	// Give the root widget a background skin
	_root.SetSkinBg(TBIDC("background"));
}

TBGUI::~TBGUI()
{

}

bool TBGUI::initialize()
{
	tb::TBWidgetsAnimationManager::Init();
	return true;
}

void TBGUI::terminate()
{
	tb::TBWidgetsAnimationManager::Shutdown();
}

void TBGUI::update()
{
	tb::TBAnimationManager::Update();
	_root.InvokeProcessStates();
	_root.InvokeProcess();
	tb::TBMessageHandler::ProcessMessages();
}

void TBGUI::draw()
{
	tb::g_renderer->BeginPaint(_root.GetRect().w, _root.GetRect().h);
	_root.InvokePaint(tb::TBWidget::PaintProps());
	tb::g_renderer->EndPaint();

	// If animations are running, reinvalidate immediately
	if (tb::TBAnimationManager::HasAnimationsRunning())
		getRoot()->Invalidate();
}


tb::MODIFIER_KEYS GetModifierKeys()
{
	tb::MODIFIER_KEYS code = tb::TB_MODIFIER_NONE;
	SDL_Keymod mods = SDL_GetModState();
	if (mods & KMOD_ALT)	code |= tb::TB_ALT;
	if (mods & KMOD_CTRL)	code |= tb::TB_CTRL;
	if (mods & KMOD_SHIFT)	code |= tb::TB_SHIFT;
	if (mods & KMOD_GUI)	code |= tb::TB_SUPER; // no idea what SUPER means, but doesn't seem to be used
	return code;
}

tb::MODIFIER_KEYS GetModifierKeys(SDL_Keymod mods)
{
	tb::MODIFIER_KEYS code = tb::TB_MODIFIER_NONE;
	if (mods & KMOD_ALT)	code |= tb::TB_ALT;
	if (mods & KMOD_CTRL)	code |= tb::TB_CTRL;
	if (mods & KMOD_SHIFT)	code |= tb::TB_SHIFT;
	if (mods & KMOD_GUI)	code |= tb::TB_SUPER; // no idea what SUPER means, but doesn't seem to be used
	return code;
}

static bool ShouldEmulateTouchEvent()
{
	// Used to emulate that mouse events are touch events when alt, ctrl and shift are pressed.
	// This makes testing a lot easier when there is no touch screen around :)
	return (GetModifierKeys() & (tb::TB_ALT | tb::TB_CTRL | tb::TB_SHIFT)) ? true : false;
}
// @return Return the upper case of a ascii charcter. Only for shortcut handling.
static int toupr_ascii(int ascii)
{
	if (ascii >= 'a' && ascii <= 'z')
		return ascii + 'A' - 'a';
	return ascii;
}

static bool InvokeShortcut(int key, tb::SPECIAL_KEY special_key, tb::MODIFIER_KEYS modifierkeys, bool down)
{
#ifdef TB_TARGET_MACOSX
	bool shortcut_key = (modifierkeys & tb::TB_SUPER) ? true : false;
#else
	bool shortcut_key = (modifierkeys & tb::TB_CTRL) ? true : false;
#endif
	if (!tb::TBWidget::focused_widget || !down || !shortcut_key)
		return false;
	bool reverse_key = (modifierkeys & tb::TB_SHIFT) ? true : false;
	int upper_key = toupr_ascii(key);
	tb::TBID id;
	if (upper_key == 'X')
		id = TBIDC("cut");
	else if (upper_key == 'C' || special_key == tb::TB_KEY_INSERT)
		id = TBIDC("copy");
	else if (upper_key == 'V' || (special_key == tb::TB_KEY_INSERT && reverse_key))
		id = TBIDC("paste");
	else if (upper_key == 'A')
		id = TBIDC("selectall");
	else if (upper_key == 'Z' || upper_key == 'Y')
	{
		bool undo = upper_key == 'Z';
		if (reverse_key)
			undo = !undo;
		id = undo ? TBIDC("undo") : TBIDC("redo");
	}
	else if (upper_key == 'N')
		id = TBIDC("new");
	else if (upper_key == 'O')
		id = TBIDC("open");
	else if (upper_key == 'S')
		id = TBIDC("save");
	else if (upper_key == 'W')
		id = TBIDC("close");
	else if (special_key == tb::TB_KEY_PAGE_UP)
		id = TBIDC("prev_doc");
	else if (special_key == tb::TB_KEY_PAGE_DOWN)
		id = TBIDC("next_doc");
	else
		return false;

	tb::TBWidgetEvent ev(tb::EVENT_TYPE_SHORTCUT);
	ev.modifierkeys = modifierkeys;
	ev.ref_id = id;
	return tb::TBWidget::focused_widget->InvokeEvent(ev);
}

bool TBGUI::InvokeKey(unsigned int key, tb::SPECIAL_KEY special_key, tb::MODIFIER_KEYS modifierkeys, bool down)
{
	if (InvokeShortcut(key, special_key, modifierkeys, down))
		return true;
	return _root.InvokeKey(key, special_key, modifierkeys, down);
}

// Attempt to convert an sdl event to a TB event, return true if handled
bool TBGUI::HandleSDLEvent(SDL_Event & event)
{
	bool handled = true;
	switch (event.type) {
	case SDL_KEYUP:
	case SDL_KEYDOWN: {
		// SDL_KeyboardEvent
		// Handle any key presses here.
		bool down = event.type == SDL_KEYDOWN;
		tb::MODIFIER_KEYS modifier = GetModifierKeys((SDL_Keymod)event.key.keysym.mod);
		if (event.key.keysym.sym >= SDLK_SPACE && event.key.keysym.sym <= SDLK_z)
		{
			unsigned int character = event.key.keysym.sym;
			InvokeKey(character, tb::TB_KEY_UNDEFINED, modifier, down);
		}
		else
		{
			// handle special keys
			switch (event.key.keysym.sym)
			{
			case SDLK_F1:			InvokeKey(0, tb::TB_KEY_F1, modifier, down); break;
			case SDLK_F2:			InvokeKey(0, tb::TB_KEY_F2, modifier, down); break;
			case SDLK_F3:			InvokeKey(0, tb::TB_KEY_F3, modifier, down); break;
			case SDLK_F4:			InvokeKey(0, tb::TB_KEY_F4, modifier, down); break;
			case SDLK_F5:			InvokeKey(0, tb::TB_KEY_F5, modifier, down); break;
			case SDLK_F6:			InvokeKey(0, tb::TB_KEY_F6, modifier, down); break;
			case SDLK_F7:			InvokeKey(0, tb::TB_KEY_F7, modifier, down); break;
			case SDLK_F8:			InvokeKey(0, tb::TB_KEY_F8, modifier, down); break;
			case SDLK_F9:			InvokeKey(0, tb::TB_KEY_F9, modifier, down); break;
			case SDLK_F10:			InvokeKey(0, tb::TB_KEY_F10, modifier, down); break;
			case SDLK_F11:			InvokeKey(0, tb::TB_KEY_F11, modifier, down); break;
			case SDLK_F12:			InvokeKey(0, tb::TB_KEY_F12, modifier, down); break;
			case SDLK_LEFT:			InvokeKey(0, tb::TB_KEY_LEFT, modifier, down); break;
			case SDLK_UP:			InvokeKey(0, tb::TB_KEY_UP, modifier, down); break;
			case SDLK_RIGHT:		InvokeKey(0, tb::TB_KEY_RIGHT, modifier, down); break;
			case SDLK_DOWN:			InvokeKey(0, tb::TB_KEY_DOWN, modifier, down); break;
			case SDLK_PAGEUP:		InvokeKey(0, tb::TB_KEY_PAGE_UP, modifier, down); break;
			case SDLK_PAGEDOWN:		InvokeKey(0, tb::TB_KEY_PAGE_DOWN, modifier, down); break;
			case SDLK_HOME:			InvokeKey(0, tb::TB_KEY_HOME, modifier, down); break;
			case SDLK_END:			InvokeKey(0, tb::TB_KEY_END, modifier, down); break;
			case SDLK_INSERT:		InvokeKey(0, tb::TB_KEY_INSERT, modifier, down); break;
			case SDLK_TAB:			InvokeKey(0, tb::TB_KEY_TAB, modifier, down); break;
			case SDLK_DELETE:		InvokeKey(0, tb::TB_KEY_DELETE, modifier, down); break;
			case SDLK_BACKSPACE:	InvokeKey(0, tb::TB_KEY_BACKSPACE, modifier, down); break;
			case SDLK_RETURN:
			case SDLK_KP_ENTER:		InvokeKey(0, tb::TB_KEY_ENTER, modifier, down); break;
			case SDLK_ESCAPE:		InvokeKey(0, tb::TB_KEY_ESC, modifier, down); break;
			case SDLK_MENU:
				if (tb::TBWidget::focused_widget && !down)
				{
					tb::TBWidgetEvent ev(tb::EVENT_TYPE_CONTEXT_MENU);
					ev.modifierkeys = modifier;
					tb::TBWidget::focused_widget->InvokeEvent(ev);
				}
				break;
				/* just ignore lone modifier key presses */
			case SDLK_LCTRL:
			case SDLK_RCTRL:
			case SDLK_LALT:
			case SDLK_RALT:
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
			case SDLK_LGUI:
			case SDLK_RGUI:
				break;
			default:
				handled = false;
				break;
			}
		}

		break;
	}
	case SDL_FINGERMOTION:
	case SDL_FINGERDOWN:
	case SDL_FINGERUP:
		//event.tfinger;
		break;

	case SDL_MOUSEMOTION: {
		if (!(ShouldEmulateTouchEvent() && !tb::TBWidget::captured_widget))
			_root.InvokePointerMove(event.motion.x, event.motion.y,
				GetModifierKeys(),
				ShouldEmulateTouchEvent());

		break;
	}
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEBUTTONDOWN: {
		// Handle mouse clicks here.
		tb::MODIFIER_KEYS modifier = GetModifierKeys();
		int x = event.button.x;
		int y = event.button.y;
		if (event.button.button == SDL_BUTTON_LEFT)
		{
			if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				// This is a quick fix with n-click support :)
				static double last_time = 0;
				static int last_x = 0;
				static int last_y = 0;
				static int counter = 1;

				double time = tb::TBSystem::GetTimeMS();
				if (time < last_time + 600 && last_x == x && last_y == y)
					counter++;
				else
					counter = 1;
				last_x = x;
				last_y = y;
				last_time = time;

				_root.InvokePointerDown(x, y, counter, modifier, ShouldEmulateTouchEvent());
			}
			else
				_root.InvokePointerUp(x, y, modifier, ShouldEmulateTouchEvent());
		}
		else if (event.button.button == SDL_BUTTON_RIGHT && event.type == SDL_MOUSEBUTTONUP)
		{
			_root.InvokePointerMove(x, y, modifier, ShouldEmulateTouchEvent());
			if (tb::TBWidget::hovered_widget)
			{
				tb::TBWidget::hovered_widget->ConvertFromRoot(x, y);
				tb::TBWidgetEvent ev(tb::EVENT_TYPE_CONTEXT_MENU, x, y, false, modifier);
				tb::TBWidget::hovered_widget->InvokeEvent(ev);
			}
		}
	}
							  break;
	case SDL_MOUSEWHEEL: {
		int mouse_x, mouse_y;
		SDL_GetMouseState(&mouse_x, &mouse_y);
			_root.InvokeWheel(mouse_x, mouse_y,
			(int)event.wheel.x, -(int)event.wheel.y,
				GetModifierKeys());
		break;
	}
	case SDL_MULTIGESTURE:
		//event.mgesture;
		break;
	case SDL_SYSWMEVENT:
		//event.syswm;
		break;
	case SDL_TEXTEDITING:
		//event.edit;
		break;
	case SDL_TEXTINPUT:
		//event.text;
		break;
	case SDL_USEREVENT:
		// event.user;
		// draw event
		//if (m_has_pending_update)
		//{
		//	m_app->Process();
		//	m_has_pending_update = false;
		//	// Bail out if we get here with invalid dimensions.
		//	// This may happen when minimizing windows (GLFW 3.0.4, Windows 8.1).
		//	if (GetWidth() == 0 || GetHeight() == 0)
		//		; // ignore
		//	else
		//	{
		//		m_app->RenderFrame();
		//		SDL_GL_SwapWindow(mainWindow);
		//	}
		//}
		break;
	default:
		handled = false;
	}
	return handled;
}

void TBGUI::onResized(int width, int height)
{
	Log::Debug("[TBGUI] Resising root to %i x %i", width, height);
	_root.SetRect(tb::TBRect(0, 0, width, height));
}
