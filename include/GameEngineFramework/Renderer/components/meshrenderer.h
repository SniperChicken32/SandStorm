#ifndef __COMPONENT_ENTITY
#define __COMPONENT_ENTITY

#include <GameEngineFramework/Transform/Transform.h>

#include <GameEngineFramework/Renderer/components/material.h>
#include <GameEngineFramework/Renderer/components/mesh.h>

class ENGINE_API LevelOfDetail {
    
public:
    
    float distance;
    
    Mesh* mesh;
    
};




class ENGINE_API MeshRenderer {
    
public:
    
    /// Should this renderer be included in the scene.
    bool isActive;
    
    /// Pointer to the mesh component.
    Mesh* mesh;
    
    /// Pointer to the material component.
    Material* material;
    
    /// Transformation element.
    Transform transform;
    
    MeshRenderer();
    
private:
    
    friend class RenderSystem;
    
};


#endif
