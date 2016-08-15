StruggleBox - Voxel-based networked game prototype.
-----------------------------------------------------------------------------------

The goal is a game with networked multiplayer combat in a single room.
Supported platforms are Windows and OSX for now with possible mobile ports later.

Features
-----------------------------------------------------------------------------------

Core Engine:

  * Memory allocators
  * Command processor
  * Debug console
  * Scene management
  * Component-based entity system
  * Keyboard & mouse (Xbox + PS controllers will be added in future)
  * PBR renderer based on Disney Plausible Model
  * FreeType based text renderer
  * Custom logging
  * Various utility classes for timers, file I/O, Base64 encoding, etc.


Setup
-----------------------------------------------------------------------------------

Clone the repository with:

  git clone https://github.com/eVillain/StruggleBox.git

Grab all the required dependencies:

  git submodule init
	git submodule update

This will clone all the required dependencies into the thirdparty/ folder.

There are project files included for VS2015 and Xcode.

Dependencies
-----------------------------------------------------------------------------------
  * SDL 2
  * Bullet Physics
  * FreeType 2 (https://www.freetype.org)
  * GLEW (http://glew.sourceforge.net/)
  * LibPNG
  * Zlib
  * PugiXML
