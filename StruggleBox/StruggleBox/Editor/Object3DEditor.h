#ifndef OBJECT3D_EDITOR_H
#define OBJECT3D_EDITOR_H

#include "EditorScene.h"
#include "AABB3D.h"
#include "EditorCursor3D.h"
#include "Light3D.h"

class LightSystem3D;
class TBGUI;
class Text;
class ObjectWindow;
class MeshWindow;

class InstancedMesh;
class VoxelData;

///  Provides voxel object editing facilities
class Object3DEditor : public EditorScene
{
public:
    Object3DEditor(
		std::shared_ptr<TBGUI> gui,
		std::shared_ptr<Camera> camera,
		std::shared_ptr<Renderer> renderer,
		std::shared_ptr<Options> options,
		std::shared_ptr<Input> input,
		std::shared_ptr<LightSystem3D> lights,
		std::shared_ptr<Text> text);
    virtual ~Object3DEditor();
    
    // Overridden from Scene
    void Initialize();
    void ReInitialize();
    void Release();
    void Pause();
    void Resume();
    void Update(const double delta);
    void Draw();
    
    // Editing stuff
    void DrawEditCursor();
    void Cancel();
    void Create();
    void Erase();

    // Labels and buttons
    void ShowEditor();
    void RemoveEditor();

    // Loading/Saving
    void LoadObject( const std::string fileName );
    void SaveObject( const std::string fileName );

private:
	std::shared_ptr<LightSystem3D> _lighting;
	std::shared_ptr<Text> _text;
	
	std::shared_ptr<InstancedMesh> _mesh;
	std::shared_ptr<VoxelData> _voxels;

	ObjectWindow* _objectWindow;
	MeshWindow* _meshWindow;

    // Cursor coordinates
	glm::ivec3 _cursorObjectCoord;
    AABB3D selectionAABB;

    // Block editing - move to objectwindow
    int columnHeight;                   // Height of column to be created

    // Labels
    std::vector<int> editorLabels;


	unsigned int _instanceID = 0;

    bool OnEvent(const std::string& theEvent,
                 const float& amount);
    bool OnMouse(const glm::ivec2& coord);

	void refreshVoxelMesh();

};

#endif
