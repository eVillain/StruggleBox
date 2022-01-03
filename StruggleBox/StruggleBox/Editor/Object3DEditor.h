#ifndef OBJECT3D_EDITOR_H
#define OBJECT3D_EDITOR_H

#include "EditorScene.h"
#include "AABB3D.h"
#include "EditorCursor3D.h"
#include "Light3D.h"

class Allocator;
class ObjectWindow;
class MeshWindow;

class InstancedMesh;
class VoxelData;

///  Provides voxel object editing facilities
class Object3DEditor : public EditorScene
{
public:
    Object3DEditor(
		Camera& camera,
        Allocator& allocator,
		Renderer& renderer,
		Options& options,
		Input& input,
        StatTracker& statTracker);
    virtual ~Object3DEditor();
    
    // Overridden from Scene
    void Initialize() override;
    void ReInitialize() override;
    void Pause() override;
    void Resume() override;
    void Update(const double delta) override;
    void Draw() override;
    
    const std::string getFileType() const override { return "*.bwo"; }
    const std::string getFilePath() const override; 
    void onFileLoad(const std::string& file) override { LoadObject(file); }
    void onFileSave(const std::string& file) override { SaveObject(file); }

    // Editing stuff
    void DrawEditCursor();
    void Cancel();
    void Create();
    void Erase();

    // Labels and buttons
    void ShowEditor();
    void RemoveEditor();

private:	
	InstancedMesh* _mesh;
	VoxelData* _voxels;

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

    void resizeVoxelData(const glm::ivec3& resize);
    void rescaleVoxelData(const float scale);

    // Loading/Saving
    void LoadObject(const std::string fileName);
    void SaveObject(const std::string fileName);

    bool OnEvent(const InputEvent event, const float amount);
    bool OnMouse(const glm::ivec2& coord);

	void refreshVoxelMesh();

};

#endif
