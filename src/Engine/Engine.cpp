#include "engine.h"

__declspec(dllexport) EngineComponents  Components;
__declspec(dllexport) ColorPreset       Colors;
__declspec(dllexport) RandomGen         Random;
__declspec(dllexport) Logger            Log;
__declspec(dllexport) Timer             PhysicsTime;
__declspec(dllexport) Timer             Time;

__declspec(dllexport) Serialization     Serializer;
__declspec(dllexport) ResourceManager   Resources;
__declspec(dllexport) ScriptSystem      Scripting;
__declspec(dllexport) RenderSystem      Renderer;
__declspec(dllexport) PhysicsSystem     Physics;
__declspec(dllexport) AudioSystem       Audio;
__declspec(dllexport) InputSystem       Input;
__declspec(dllexport) MathCore          Math;
__declspec(dllexport) ActorSystem       AI;

__declspec(dllexport) ApplicationLayer      Application;
__declspec(dllexport) EngineSystemManager   Engine;



EngineSystemManager::EngineSystemManager(void) : 
    sceneMain(nullptr),
    
    doUpdateDataStream(true),
    streamSize(0)
{
}

GameObject* EngineSystemManager::CreateGameObject(void) {
    GameObject* newGameObject = mGameObjects.Create();
    mGameObjectActive.push_back(newGameObject);
    doUpdateDataStream = true;
    return newGameObject;
}

bool EngineSystemManager::DestroyGameObject(GameObject* gameObjectPtr) {
    assert(gameObjectPtr != nullptr);
    
    // Remove the game object from the active list
    for (std::vector<GameObject*>::iterator it = mGameObjectActive.begin(); it != mGameObjectActive.end(); ++it) {
        GameObject* thisGameObjectPtr = *it;
        if (gameObjectPtr == thisGameObjectPtr) {
            mGameObjectActive.erase(it);
            break;
        }
    }
    
    // Remove all components
    for (unsigned int i=0; i < gameObjectPtr->GetComponentCount(); i++) {
        
        Component* componentPtr = gameObjectPtr->GetComponentIndex(i);
        DestroyComponent(componentPtr);
        continue;
    }
    
    mGameObjects.Destroy(gameObjectPtr);
    
    doUpdateDataStream = true;
    return true;
}

GameObject* EngineSystemManager::CreateCameraController(glm::vec3 position, glm::vec3 scale) {
    
    GameObject* cameraController = CreateGameObject();
    cameraController->name = "camera";
    cameraController->transform.position = position;
    
    // Add a camera component
    Component* cameraComponent = CreateComponent(Components.Camera);
    Camera* cameraMain = (Camera*)cameraComponent->mObject;
    cameraMain->EnableMouseLook();
    
    SetCursorPos(Renderer.displayCenter.x, Renderer.displayCenter.y);
    
    // Add a rigid body component
    Component* rigidBodyComponent = CreateComponent(Components.RigidBody);
    RigidBody* rigidBody = (RigidBody*)rigidBodyComponent->mObject;
    
    rp3d::Vector3 bodyPosition(position.x, position.y, position.z);
    rp3d::Quaternion quat = rp3d::Quaternion::identity();
    
    rp3d::Transform bodyTransform(bodyPosition, quat);
    rigidBody->setTransform(bodyTransform);
    
    // Add a scripting component
    Component* scriptComponent = CreateComponent(Components.Script);
    Script* script = (Script*)scriptComponent->mObject;
    script->name = "controller";
    script->gameObject = cameraController;
    script->isActive = true;
    
    cameraController->AddComponent(cameraComponent);
    cameraController->AddComponent(rigidBodyComponent);
    cameraController->AddComponent(scriptComponent);
    
    cameraController->SetAngularAxisLockFactor(0, 0, 0);
    cameraController->SetLinearDamping(3);
    cameraController->SetAngularDamping(1);
    cameraController->SetMass(10);
    
    // Collider
    BoxShape* boxShape = Physics.CreateColliderBox(scale.x, scale.y, scale.z);
    cameraController->AddColliderBox(boxShape, 0, 0, 0);
    
    doUpdateDataStream = true;
    return cameraController;
}

GameObject* EngineSystemManager::CreateSky(std::string meshTagName, Color colorLow, Color colorHigh, float biasMul) {
    
    Mesh* skyMesh = Resources.CreateMeshFromTag(meshTagName);
    if (skyMesh == nullptr) return nullptr;
    
    Material* skyMaterial = Renderer.CreateMaterial();
    
    skyMaterial->diffuse = Color(1, 1, 1);
    skyMaterial->DisableDepthTest();
    skyMaterial->ambient = Colors.MakeGrayScale(0.4);
    skyMaterial->diffuse = Colors.MakeGrayScale(0.4);
    skyMaterial->shader = shaders.color;
    
    for (unsigned int i=0; i < skyMesh->GetNumberOfVertices(); i++) {
        Vertex vertex = skyMesh->GetVertex(i);
        
        if (vertex.y > 0) {
            vertex.r = Math.Lerp(colorHigh.r, colorLow.r, vertex.y * biasMul);
            vertex.g = Math.Lerp(colorHigh.g, colorLow.g, vertex.y * biasMul);
            vertex.b = Math.Lerp(colorHigh.b, colorLow.b, vertex.y * biasMul);
        } else {
            vertex.r = Math.Lerp(colorLow.r, colorHigh.r, vertex.y * biasMul);
            vertex.g = Math.Lerp(colorLow.g, colorHigh.g, vertex.y * biasMul);
            vertex.b = Math.Lerp(colorLow.b, colorHigh.b, vertex.y * biasMul);
        }
        
        skyMesh->SetVertex(i, vertex);
    }
    skyMesh->UploadToGPU();
    
    GameObject* skyObject = CreateGameObject();
    skyObject->name = "sky";
    skyObject->AddComponent( CreateComponentMeshRenderer(skyMesh, skyMaterial) );
    
    skyObject->transform.SetScale(10000, 2000, 10000);
    
    doUpdateDataStream = true;
    return skyObject;
}

Component* EngineSystemManager::CreateComponentMeshRenderer(Mesh* meshPtr, Material* materialPtr) {
    Component* rendererComponent = CreateComponent(Components.MeshRenderer);
    MeshRenderer* meshRenderer = (MeshRenderer*)rendererComponent->mObject;
    
    meshRenderer->mesh = meshPtr;
    meshRenderer->material = materialPtr;
    
    if (sceneMain != nullptr) 
        sceneMain->AddMeshRendererToSceneRoot( meshRenderer );
    
    doUpdateDataStream = true;
    return rendererComponent;
}

Component* EngineSystemManager::CreateComponentLight(glm::vec3 position) {
    Component* lightComponent = CreateComponent(Components.Light);
    Light* lightPoint = (Light*)lightComponent->mObject;
    
    lightPoint->position = position;
    
    if (sceneMain != nullptr) 
        sceneMain->AddLightToSceneRoot( lightPoint );
    
    doUpdateDataStream = true;
    return lightComponent;
}

GameObject* EngineSystemManager::CreateAIActor(glm::vec3 position) {
    
    GameObject* newGameObject = CreateGameObject();
    newGameObject->AddComponent( CreateComponent(Components.Actor) );
    newGameObject->AddComponent( CreateComponent(Components.RigidBody) );
    newGameObject->AddComponent( CreateComponent(Components.MeshRenderer) );
    
    // Basic cube mesh
    Mesh* meshPtr = Resources.CreateMeshFromTag("cube");
    Material* materialPtr = Renderer.CreateMaterial();
    
    materialPtr->shader = shaders.color;
    
    // Mesh renderer component
    MeshRenderer* entityRenderer = newGameObject->mMeshRendererCache;
    
    entityRenderer->mesh = meshPtr;
    entityRenderer->material = materialPtr;
    
    if (sceneMain != nullptr) 
        sceneMain->AddMeshRendererToSceneRoot( entityRenderer );
    
    float scale = 1.0;
    
    // Collider
    BoxShape* boxShape = Physics.CreateColliderBox(scale, scale, scale);
    newGameObject->AddColliderBox(boxShape, 0, 0, 0);
    newGameObject->EnableGravity();
    
    // Physics
    newGameObject->SetMass(10);
    newGameObject->SetLinearDamping(3);
    newGameObject->SetAngularDamping(1);
    
    //newGameObject->CalculatePhysics();
    
    newGameObject->SetLinearAxisLockFactor(1, 1, 1);
    newGameObject->SetAngularAxisLockFactor(0, 1, 0);
    
    newGameObject->transform.SetScale(scale, scale, scale);
    newGameObject->SetPosition(position);
    
    doUpdateDataStream = true;
    return newGameObject;
}

GameObject* EngineSystemManager::CreateOverlayRenderer(void) {
    GameObject* overlayObject = Create<GameObject>();
    
    overlayObject->transform.RotateAxis(-180, Vector3(0, 1, 0));
    overlayObject->transform.RotateAxis( -90, Vector3(0, 0, 1));
    
    Mesh*     overlayMesh     = Create<Mesh>();
    Material* overlayMaterial = Create<Material>();
    
    overlayMaterial->shader = shaders.color;
    overlayMaterial->ambient = Colors.black;
    
    overlayMaterial->SetDepthFunction(MATERIAL_DEPTH_ALWAYS);
    overlayMaterial->SetTextureFiltration(MATERIAL_FILTER_NONE);
    overlayMaterial->DisableCulling();
    
    overlayObject->AddComponent( CreateComponent<MeshRenderer>(overlayMesh, overlayMaterial) );
    
    doUpdateDataStream = true;
    return overlayObject;
}

GameObject* EngineSystemManager::CreateOverlayTextRenderer(std::string text, unsigned int textSize, Color color, std::string materialTag) {
    
    GameObject* overlayObject = CreateOverlayRenderer();
    overlayObject->AddComponent( CreateComponent<Text>() );
    Text* textElement = overlayObject->GetComponent<Text>();
    
    textElement->text  = text;
    textElement->color = color;
    textElement->size = textSize;
    
    overlayObject->transform.scale = Vector3(textSize, 1, textSize);
    
    MeshRenderer* overlayRenderer = overlayObject->GetComponent<MeshRenderer>();
    
    // Sprite sheet material
    Destroy<Material>( overlayRenderer->material );
    overlayRenderer->material = Resources.CreateMaterialFromTag( materialTag );
    overlayRenderer->material->ambient  = Colors.black;
    overlayRenderer->material->shader = shaders.UI;
    
    overlayRenderer->material->SetBlending(BLEND_ONE, BLEND_ONE_MINUS_SRC_ALPHA);
    overlayRenderer->material->EnableBlending();
    
    overlayRenderer->material->SetDepthFunction(MATERIAL_DEPTH_ALWAYS);
    overlayRenderer->material->SetTextureFiltration(MATERIAL_FILTER_NONE);
    
    overlayRenderer->material->DisableCulling();
    
    doUpdateDataStream = true;
    return overlayObject;
}

void EngineSystemManager::AddMeshText(GameObject* overlayObject, float xPos, float yPos, float scaleWidth, float scaleHeight, std::string text, Color textColor) {
    
    Mesh* meshPtr = overlayObject->GetComponent<MeshRenderer>()->mesh;
    if (meshPtr == nullptr) 
        return;
    
    for (unsigned int i=0; i < text.size(); i++)
        AddMeshSubSprite(overlayObject, xPos + i, yPos, scaleWidth, scaleHeight, text[i], textColor);
    
    meshPtr->UploadToGPU();
    
    return;
}

void EngineSystemManager::AddMeshSubSprite(GameObject* overlayObject, float xPos, float yPos, float scaleWidth, float scaleHeight, int index, Color meshColor) {
    
    Mesh* meshPtr = overlayObject->GetComponent<MeshRenderer>()->mesh;
    if (meshPtr == nullptr) 
        return;
    
    Text* textPtr = overlayObject->GetComponent<Text>();
    if (textPtr == nullptr) 
        return;
    
    // Sprite atlas layout
    float spriteStartX  = textPtr->sprite.subSpriteX;
    float spriteStartY  = textPtr->sprite.subSpriteY;
    float spriteWidth   = textPtr->sprite.subSpriteWidth;
    float spriteHeight  = textPtr->sprite.subSpriteHeight;
    
    float spacingWidth  = textPtr->width;
    float spacingHeight = textPtr->height;
    
    int mapWidth  = textPtr->sprite.width;
    int mapHeight = textPtr->sprite.height;
    
    // Calculate the sub sprite in the map grid
    int subWidth  = 0;
    int subHeight = 0;
    
    for (int i=0; i < index; i++) {
        
        subWidth++;
        
        if (subWidth > mapWidth) {
            subWidth=0;
            
            subHeight++;
            
            if (subHeight > mapHeight)
                return;
        }
    }
    
    meshPtr->AddPlain(yPos * spacingHeight, 
                      0, 
                      -(xPos * spacingWidth), 
                      scaleWidth, scaleHeight, 
                      meshColor, 
                      spriteWidth, spriteHeight, 
                      spriteStartX, spriteStartY, 
                      subWidth, subHeight);
    
    return;
}

GameObject* EngineSystemManager::GetGameObject(unsigned int index) {
    if (index < mGameObjectActive.size()) 
        return mGameObjectActive[index];
    return nullptr;
}

unsigned int EngineSystemManager::GetGameObjectCount(void) {
    return mGameObjects.Size();
}

unsigned int EngineSystemManager::GetComponentCount(void) {
    return mComponents.Size();
}

void EngineSystemManager::Initiate() {
    
    shaders.texture       = Resources.CreateShaderFromTag("texture");
    shaders.textureUnlit  = Resources.CreateShaderFromTag("textureUnlit");
    shaders.color         = Resources.CreateShaderFromTag("color");
    shaders.colorUnlit    = Resources.CreateShaderFromTag("colorUnlit");
    shaders.UI            = Resources.CreateShaderFromTag("UI");
    
    return;
}

Component* EngineSystemManager::CreateComponent(ComponentType type) {
    void* component_object = nullptr;
    
    switch (type) {
        
        case COMPONENT_TYPE_MESH_RENDERER: {
            MeshRenderer* meshRendererPtr = Renderer.CreateMeshRenderer();
            component_object = (void*)meshRendererPtr;
            break;
        }
        case COMPONENT_TYPE_CAMERA: {
            component_object = (void*)Renderer.CreateCamera();
            break;
        }
        case COMPONENT_TYPE_LIGHT: {
            Light* lightPtr = Renderer.CreateLight();
            component_object = (void*)lightPtr;
            break;
        }
        case COMPONENT_TYPE_SCRIPT: {
            component_object = (void*)Scripting.CreateScript();
            break;
        }
        case COMPONENT_TYPE_RIGID_BODY: {
            component_object = (void*)Physics.CreateRigidBody();
            break;
        }
        case COMPONENT_TYPE_ACTOR: {
            component_object = (void*)AI.CreateActor();
            break;
        }
        case COMPONENT_TYPE_TEXT: {
            component_object = (void*)mTextObjects.Create();
            Text* textComponent = (Text*)component_object;
            break;
        }
        
        default:
            return nullptr;
    }
    
    Component* newComponent = mComponents.Create();
    newComponent->SetComponent(type, component_object);
    
    doUpdateDataStream = true;
    return newComponent;
}

bool EngineSystemManager::DestroyComponent(Component* componentPtr) {
    assert(componentPtr != nullptr);
    
    ComponentType componentType = componentPtr->GetType();
    
    switch (componentType) {
        
        case COMPONENT_TYPE_MESH_RENDERER: {
            MeshRenderer* componentEntityRenderer = (MeshRenderer*)componentPtr->GetComponent();
            Renderer.DestroyMeshRenderer(componentEntityRenderer);
            break;
        }
        case COMPONENT_TYPE_CAMERA: {
            Camera* componentCamera = (Camera*)componentPtr->GetComponent();
            Renderer.DestroyCamera(componentCamera);
            break;
        }
        case COMPONENT_TYPE_LIGHT: {
            Light* componentLight = (Light*)componentPtr->GetComponent();
            Renderer.DestroyLight(componentLight);
            break;
        }
        case COMPONENT_TYPE_SCRIPT: {
            Script* componentScript = (Script*)componentPtr->GetComponent();
            Scripting.DestroyScript(componentScript);
            break;
        }
        case COMPONENT_TYPE_RIGID_BODY: {
            RigidBody* componentRigidBody = (RigidBody*)componentPtr->GetComponent();
            Physics.DestroyRigidBody(componentRigidBody);
            break;
        }
        case COMPONENT_TYPE_ACTOR: {
            Actor* actorObject = (Actor*)componentPtr->GetComponent();
            AI.DestroyActor(actorObject);
            break;
        }
        case COMPONENT_TYPE_TEXT: {
            Text* textObject = (Text*)componentPtr->GetComponent();
            mTextObjects.Destroy(textObject);
            break;
        }
        
        default:
            return false;
    }
    mComponents.Destroy(componentPtr);
    
    doUpdateDataStream = true;
    return true;
}

void EngineSystemManager::Update(void) {
    
    // Update player position in the AI simulation
    if (sceneMain != nullptr) {
        Camera* activeCamera = sceneMain->camera;
        if (activeCamera != nullptr) {
            AI.SetPlayerWorldPosition( activeCamera->transform.position );
        }
    }
    
    // Check to update the data stream
    if (doUpdateDataStream) {
        
        doUpdateDataStream = false;
        
        streamSize = mGameObjects.Size();
        
        for (int i=0; i < streamSize; i++ ) {
            
            streamBuffer[i].gameObject    = mGameObjects[i];
            
            streamBuffer[i].text          = mGameObjects[i]->mTextCache;
            streamBuffer[i].light         = mGameObjects[i]->mLightCache;
            streamBuffer[i].actor         = mGameObjects[i]->mActorCache;
            streamBuffer[i].camera        = mGameObjects[i]->mCameraCache;
            streamBuffer[i].rigidBody     = mGameObjects[i]->mRigidBodyCache;
            streamBuffer[i].meshRenderer  = mGameObjects[i]->mMeshRendererCache;
            
            continue;
        }
        
    }
    
    
    //
    // Run the game object list
    //
    
    for (int i=0; i < mGameObjects.Size(); i++ ) {
        
        if (!streamBuffer[i].gameObject->isActive) 
            continue;
        
        // Current transform
        Transform currentTransform;
        currentTransform.position    = streamBuffer[i].gameObject->transform.position;
        currentTransform.orientation = streamBuffer[i].gameObject->transform.orientation;
        currentTransform.scale       = streamBuffer[i].gameObject->transform.scale;
        
        // Calculate parent transforms
        GameObject* parent = streamBuffer[i].gameObject->parent;
        
        // Roll over the parent matrix transform chain
        while (parent != nullptr) {
            
            currentTransform.position    += parent->transform.position;
            currentTransform.scale       *= parent->transform.scale;
            currentTransform.orientation *= parent->transform.orientation;
            
            parent = parent->parent;
        }
        
        glm::mat4 translation = glm::translate(glm::mat4(1), currentTransform.position);
        glm::mat4 rotation    = glm::toMat4(currentTransform.orientation);
        glm::mat4 scale       = glm::scale(glm::mat4(1), currentTransform.scale);
        
        currentTransform.matrix = translation * rotation * scale;
        
        
        //
        // Rigid body
        //
        if (streamBuffer[i].rigidBody != nullptr) {
            
            // Use the rigid body as the source transform
            rp3d::Transform bodyTransform = streamBuffer[i].rigidBody->getTransform();
            rp3d::Vector3 bodyPosition = bodyTransform.getPosition();
            rp3d::Quaternion quaterion = bodyTransform.getOrientation();
            
            currentTransform.position.x = bodyPosition.x;
            currentTransform.position.y = bodyPosition.y;
            currentTransform.position.z = bodyPosition.z;
            
            currentTransform.orientation.w = quaterion.w;
            currentTransform.orientation.x = quaterion.x;
            currentTransform.orientation.y = quaterion.y;
            currentTransform.orientation.z = quaterion.z;
            
            // Source matrix
            bodyTransform.getOpenGLMatrix(&currentTransform.matrix[0][0]);
            
            // Update the game object transform
            streamBuffer[i].gameObject->transform.position    = currentTransform.position;
            streamBuffer[i].gameObject->transform.orientation = currentTransform.orientation;
            
            currentTransform.matrix = glm::scale(currentTransform.matrix, streamBuffer[i].gameObject->transform.scale);
            
        }
        
        
        //
        // Mesh renderer
        //
        if (streamBuffer[i].meshRenderer != nullptr) {
            
            streamBuffer[i].meshRenderer->transform.matrix = currentTransform.matrix;
            
            if (streamBuffer[i].rigidBody == nullptr) {
                streamBuffer[i].meshRenderer->transform.position     = currentTransform.position;
                streamBuffer[i].meshRenderer->transform.orientation  = currentTransform.orientation;
                streamBuffer[i].meshRenderer->transform.scale        = currentTransform.scale;
            }
            
        }
        
        
        
        //
        // Actor
        //
        if (streamBuffer[i].actor != nullptr) {
            
            if (streamBuffer[i].actor->GetActive()) {
                
                if (streamBuffer[i].rigidBody != nullptr) {
                    
                    glm::vec3 actorVelocity = streamBuffer[i].actor->GetVelocity();
                    
                    // Set AI inputs
                    streamBuffer[i].actor->SetPosition( currentTransform.position );
                    
                    
                    // Get AI outputs
                    
                    // Apply force velocity
                    streamBuffer[i].rigidBody->applyLocalForceAtCenterOfMass(rp3d::Vector3(actorVelocity.x, actorVelocity.y, actorVelocity.z));
                    
                }
            }
            
        }
        
        
        
        //
        // Text canvas
        //
        if (streamBuffer[i].text != nullptr) {
            
            if (streamBuffer[i].meshRenderer != nullptr) {
                
                //
                // Anchor RIGHT
                //
                
                if (streamBuffer[i].text->canvas.anchorRight) {
                    streamBuffer[i].gameObject->transform.position.z = Renderer.viewport.w + 
                                                                       streamBuffer[i].text->size * 
                                                                       streamBuffer[i].text->canvas.x;
                    
                    // Keep text on screen when anchored right
                    streamBuffer[i].gameObject->transform.position.z -= streamBuffer[i].text->text.size() * // length of string
                                                                        streamBuffer[i].text->size;         // Size of font text
                    
                } else {
                    
                    //
                    // Anchor LEFT by default
                    //
                    
                    streamBuffer[i].gameObject->transform.position.z  = (streamBuffer[i].text->canvas.x * streamBuffer[i].text->size);
                    streamBuffer[i].gameObject->transform.position.z += streamBuffer[i].text->size;
                    
                    //
                    // Anchor CENTER horizontally
                    //
                    
                    if (streamBuffer[i].text->canvas.anchorCenterHorz) {
                        
                        streamBuffer[i].gameObject->transform.position.z = (Renderer.viewport.w / 2) + (streamBuffer[i].text->canvas.x * streamBuffer[i].text->size);
                        
                    }
                    
                }
                
                //
                // Anchor TOP
                //
                
                if (streamBuffer[i].text->canvas.anchorTop) {
                    int topAnchorTotal = Renderer.displaySize.y - Renderer.viewport.h;
                    
                    topAnchorTotal += (streamBuffer[i].text->size * streamBuffer[i].text->size) / 2;
                    topAnchorTotal += streamBuffer[i].text->size * streamBuffer[i].text->canvas.y;
                    
                    streamBuffer[i].gameObject->transform.position.y = topAnchorTotal;
                } else {
                    
                    //
                    // Anchor BOTTOM by default
                    //
                    
                    streamBuffer[i].gameObject->transform.position.y  = Renderer.displaySize.y - streamBuffer[i].text->size;
                    streamBuffer[i].gameObject->transform.position.y -= streamBuffer[i].text->size * -(streamBuffer[i].text->canvas.y);
                    
                    //
                    // Anchor CENTER vertically
                    //
                    
                    if (streamBuffer[i].text->canvas.anchorCenterVert) {
                        int topAnchorTotal = Renderer.displaySize.y - Renderer.viewport.h / 2;
                        
                        topAnchorTotal += (streamBuffer[i].text->size * streamBuffer[i].text->size) / 2;
                        topAnchorTotal += (streamBuffer[i].text->size * streamBuffer[i].text->canvas.y) - (streamBuffer[i].text->size * 2);
                        
                        streamBuffer[i].gameObject->transform.position.y = topAnchorTotal;
                    }
                    
                }
                
                float textGlyphWidth  = streamBuffer[i].text->glyphWidth;
                float textGlyphHeight = streamBuffer[i].text->glyphHeight;
                
                streamBuffer[i].meshRenderer->mesh->ClearSubMeshes();
                Engine.AddMeshText(streamBuffer[i].gameObject, 0, 0, textGlyphWidth, textGlyphHeight, streamBuffer[i].text->text, streamBuffer[i].text->color);
            }
        }
        
        
        //
        // Camera
        //
        if (streamBuffer[i].camera != nullptr) {
            
            // Update mouse looking
            if (streamBuffer[i].camera->useMouseLook) {
                
                POINT cursorPos;
                GetCursorPos(&cursorPos);
                SetCursorPos(Renderer.displayCenter.x, Renderer.displayCenter.y);
                
                float MouseDiffX = (cursorPos.x - Renderer.displayCenter.x) * streamBuffer[i].camera->MouseSensitivityYaw;
                float MouseDiffY = (cursorPos.y - Renderer.displayCenter.y) * streamBuffer[i].camera->MouseSensitivityPitch;
                
                streamBuffer[i].camera->lookAngle.x += MouseDiffX * 0.01;
                streamBuffer[i].camera->lookAngle.y -= MouseDiffY * 0.01;
                
                // Yaw limit
                if (streamBuffer[i].camera->lookAngle.x >= 0.109655) {streamBuffer[i].camera->lookAngle.x -= 0.109655;}
                if (streamBuffer[i].camera->lookAngle.x <= 0.109655) {streamBuffer[i].camera->lookAngle.x += 0.109655;}
                
                // Pitch limit
                if (streamBuffer[i].camera->lookAngle.y >  0.0274f) streamBuffer[i].camera->lookAngle.y =  0.0274f;
                if (streamBuffer[i].camera->lookAngle.y < -0.0274f) streamBuffer[i].camera->lookAngle.y = -0.0274f;
                
            }
            
            // Restore looking angle
            streamBuffer[i].camera->transform.orientation.x = streamBuffer[i].camera->lookAngle.x;
            streamBuffer[i].camera->transform.orientation.y = streamBuffer[i].camera->lookAngle.y;
            
            streamBuffer[i].camera->transform.position = currentTransform.position;
            
        }
        
        
        //
        // Lights
        //
        if (streamBuffer[i].light != nullptr) {
            streamBuffer[i].light->position    = currentTransform.position;
            streamBuffer[i].light->direction   = currentTransform.EulerAngles();
        }
        
        continue;
    }
    
    return;
}

void EngineSystemManager::Shutdown(void) {
    
    while (GetGameObjectCount() > 0) {
        DestroyGameObject( GetGameObject(0) );
    }
    
    Renderer.DestroyShader(shaders.texture);
    Renderer.DestroyShader(shaders.textureUnlit);
    Renderer.DestroyShader(shaders.color);
    Renderer.DestroyShader(shaders.UI);
    
    assert(GetGameObjectCount() == 0);
    assert(mComponents.Size() == 0);
    
    return;
}

