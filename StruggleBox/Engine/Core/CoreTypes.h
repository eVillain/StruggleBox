#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <string>

// Forward declarations for engine classes and types
class HyperVisor;
class CommandProcessor;
class Renderer;
class Scene;
class SceneManager;
class UIManager;
class Input;
class LightSystem2D;
class LightSystem3D;
class Particles;
class StatTracker;

class Options;
class FileSelectorBase;

// Sound manager, not yet implemented
class SoundMan;
// Network layer, not yet implemented
class NetLayer;

// Camera
class Camera;
// UI Widgets
class UIWidget;
class ButtonBase;
class UIFileMenuBase;
class UIWorldMenuBase;
class UIInventory;

// Rendering
class TextManager;
class Shader;
class Texture;
class TextureManager;
class Sprite;
class SpriteBatch;

// Entities
class Entity;
class EntityComponent;

#endif