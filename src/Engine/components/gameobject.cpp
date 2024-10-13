#include <GameEngineFramework/Engine/components/gameobject.h>


GameObject::GameObject() : 
    
    name(""),
    isActive(true),
    renderDistance(-1),
    
    mTransformCache(nullptr),
    mCameraCache(nullptr),
    mRigidBodyCache(nullptr),
    mMeshRendererCache(nullptr),
    mLightCache(nullptr),
    mActorCache(nullptr),
    mTextCache(nullptr),
    mPanelCache(nullptr)
{
}


//
// Component functions
//

void GameObject::AddComponent(Component* component) {
    
    mComponentList.push_back(component);
    
    switch (component->GetType()) {
        
        case Components.Transform: {
            assert(mTransformCache == nullptr);
            mTransformCache = (Transform*)component->GetComponent();
            break;
        }
        case Components.RigidBody: {
            assert(mRigidBodyCache == nullptr);
            mRigidBodyCache = (rp3d::RigidBody*)component->GetComponent();
            mRigidBodyCache->setUserData( (void*)this );
            break;
        }
        case Components.MeshRenderer: {
            assert(mMeshRendererCache == nullptr);
            mMeshRendererCache = (MeshRenderer*)component->GetComponent();
            break;
        }
        case Components.Camera: {
            assert(mCameraCache == nullptr);
            mCameraCache = (Camera*)component->GetComponent();
            break;
        }
        case Components.Light: {
            assert(mLightCache == nullptr);
            mLightCache = (Light*)component->GetComponent();
            break;
        }
        case Components.Actor: {
            assert(mActorCache == nullptr);
            mActorCache = (Actor*)component->GetComponent();
            break;
        }
        case Components.Text: {
            assert(mTextCache == nullptr);
            mTextCache = (Text*)component->GetComponent();
            break;
        }
        case Components.Panel: {
            assert(mPanelCache == nullptr);
            mPanelCache = (Panel*)component->GetComponent();
            break;
        }
        
        default: break;
    }
    
    return;
}

bool GameObject::RemoveComponent(Component* component) {
    
    for (std::vector<Component*>::iterator it = mComponentList.begin(); it != mComponentList.end(); ++it) {
        
        Component* thisComponentPtr = *it;
        
        if (component != thisComponentPtr) 
            continue;
        
        mComponentList.erase(it);
        
        switch ( component->GetType() ) {
            
            case Components.Transform: {
                assert(mTransformCache != nullptr);
                mTransformCache = nullptr;
                break;
            }
            case Components.MeshRenderer: {
                assert(mMeshRendererCache != nullptr);
                mMeshRendererCache = nullptr;
                break;
            }
            case Components.RigidBody: {
                assert(mRigidBodyCache != nullptr);
                mRigidBodyCache->setUserData( nullptr );
                mRigidBodyCache = nullptr;
                break;
            }
            case Components.Camera: {
                assert(mCameraCache != nullptr);
                mCameraCache = nullptr;
                break;
            }
            case Components.Light: {
                assert(mLightCache != nullptr);
                mLightCache = nullptr;
                break;
            }
            case Components.Actor: {
                assert(mActorCache != nullptr);
                mActorCache = nullptr;
                break;
            }
            case Components.Text: {
                assert(mTextCache != nullptr);
                mTextCache = nullptr;
                break;
            }
            case Components.Panel: {
                assert(mPanelCache != nullptr);
                mPanelCache = nullptr;
                break;
            }
            
            default: break;
        }
        
        return true;
    }
    
    return false;
}

Component* GameObject::GetComponentIndex(unsigned int index) {
    return mComponentList[index];
}

unsigned int GameObject::GetComponentCount(void) {
    return mComponentList.size();
}



//
// Physics functions
//
glm::vec3 GameObject::GetPosition(void) {
    return mTransformCache->position;
}

void GameObject::SetPosition(float x, float y, float z) {
    mTransformCache->position = glm::vec3(x, y, z);
    if (mRigidBodyCache != nullptr) {
        rp3d::Vector3 position(x, y, z);
        rp3d::Transform bodyTransform = mRigidBodyCache->getTransform();
        bodyTransform.setPosition(position);
        mRigidBodyCache->setTransform(bodyTransform);
    }
    if (mMeshRendererCache != nullptr) {
        mMeshRendererCache->transform.position = glm::vec3(x, y, z);
    }
    if (mCameraCache != nullptr) {
        mCameraCache->transform.position = glm::vec3(x, y, z);
    }
    if (mActorCache!= nullptr) {
        mActorCache->SetPosition( glm::vec3(x, y, z) );
    }
    if (mLightCache != nullptr) {
        mLightCache->position = glm::vec3(x, y, z);
    }
    return;
}

void GameObject::SetPosition(glm::vec3 newPosition) {
    SetPosition(newPosition.x, newPosition.y, newPosition.z);
    return;
}

void GameObject::Activate(void) {
    
    renderDistance = -1;
    isActive = true;
    
    for (unsigned int i=0; i < mComponentList.size(); i++) {
        
        Component* componentPtr = mComponentList[i];
        
        ComponentType type = componentPtr->GetType();
        
        if (type == Components.Script) {
            Script* script = (Script*)componentPtr->GetComponent();
            script->isActive = true;
        }
        if (type == Components.RigidBody) {
            RigidBody* rigidBody = (RigidBody*)componentPtr->GetComponent();
            rigidBody->setIsActive(true);
        }
        if (type == Components.MeshRenderer) {
            MeshRenderer* meshRenderer = (MeshRenderer*)componentPtr->GetComponent();
            meshRenderer->isActive = true;
        }
        if (type == Components.Light) {
            Light* light = (Light*)componentPtr->GetComponent();
            light->isActive = true;
        }
        if (type == Components.Actor) {
            Actor* actor = (Actor*)componentPtr->GetComponent();
            actor->SetActive(true);
        }
        
        continue;
    }
    
    return;
}

void GameObject::Deactivate(void) {
    
    renderDistance = -1;
    isActive = false;
    
    for (unsigned int i=0; i < mComponentList.size(); i++) {
        
        Component* componentPtr = mComponentList[i];
        
        ComponentType type = componentPtr->GetType();
        
        if (type == Components.Script) {
            Script* script = (Script*)componentPtr->GetComponent();
            script->isActive = false;
        }
        if (type == Components.RigidBody) {
            RigidBody* rigidBody = (RigidBody*)componentPtr->GetComponent();
            rigidBody->setIsActive(false);
        }
        if (type == Components.MeshRenderer) {
            MeshRenderer* meshRenderer = (MeshRenderer*)componentPtr->GetComponent();
            meshRenderer->isActive = false;
        }
        if (type == Components.Light) {
            Light* light = (Light*)componentPtr->GetComponent();
            light->isActive = false;
        }
        if (type == Components.Actor) {
            Actor* actor = (Actor*)componentPtr->GetComponent();
            actor->SetActive(false);
        }
        
        continue;
    }
    
    return;
}

void GameObject::AddForce(float x, float y, float z) {
    if (mRigidBodyCache == nullptr) return;
    rp3d::Vector3 nudge(x, y, z);
    mRigidBodyCache->applyLocalForceAtCenterOfMass(nudge);
    return;
}

void GameObject::AddForce(glm::vec3 force) {
    if (mRigidBodyCache == nullptr) return;
    rp3d::Vector3 nudge(force.x, force.y, force.z);
    mRigidBodyCache->applyLocalForceAtCenterOfMass(nudge);
    return;
}

void GameObject::AddTorque(float x, float y, float z) {
    if (mRigidBodyCache == nullptr) return;
    rp3d::Vector3 twist(x, y, z);
    mRigidBodyCache->applyLocalTorque(twist);
    return;
}

void GameObject::AddTorque(glm::vec3 force) {
    if (mRigidBodyCache == nullptr) return;
    rp3d::Vector3 twist(force.x, force.y, force.z);
    mRigidBodyCache->applyLocalTorque(twist);
    return;
}

void GameObject::SetMass(float mass) {
    if (mRigidBodyCache == nullptr) return;
    mRigidBodyCache->setMass(mass);
}

void GameObject::SetLinearDamping(float damping) {
    if (mRigidBodyCache == nullptr) return;
    mRigidBodyCache->setLinearDamping(damping);
}

void GameObject::SetAngularDamping(float damping) {
    if (mRigidBodyCache == nullptr) return;
    mRigidBodyCache->setAngularDamping(damping);
}

void GameObject::EnableGravity(void) {
    if (mRigidBodyCache == nullptr) return;
    mRigidBodyCache->enableGravity(true);
}

void GameObject::DisableGravity(void) {
    if (mRigidBodyCache == nullptr) return;
    mRigidBodyCache->enableGravity(false);
}

void GameObject::CalculatePhysics(void) {
    if (mRigidBodyCache == nullptr) return;
    mRigidBodyCache->updateMassFromColliders();
    mRigidBodyCache->updateLocalCenterOfMassFromColliders();
    mRigidBodyCache->updateLocalInertiaTensorFromColliders();
    return;
}

void GameObject::SetLinearAxisLockFactor(float x, float y, float z) {
    if (mRigidBodyCache == nullptr) return;
    rp3d::Vector3 lockFactor(x, y, z);
    mRigidBodyCache->setLinearLockAxisFactor(lockFactor);
    return;
}

void GameObject::SetAngularAxisLockFactor(float x, float y, float z) {
    if (mRigidBodyCache == nullptr) return;
    rp3d::Vector3 lockFactor(x, y, z);
    mRigidBodyCache->setAngularLockAxisFactor(lockFactor);
    return;
}

void GameObject::AddColliderBox(rp3d::BoxShape* boxShape, float x, float y, float z, LayerMask layer) {
    if (mRigidBodyCache == nullptr) return;
    
    rp3d::Transform offsetTransform;
    offsetTransform.setPosition(rp3d::Vector3(x, y, z));
    
    rp3d::Collider* collider = mRigidBodyCache->addCollider(boxShape, offsetTransform);
    collider->setCollideWithMaskBits( (unsigned short)layer );
    
    return;
}

void GameObject::AddColliderCapsule(rp3d::CapsuleShape* capsuleShape, float x, float y, float z, LayerMask layer) {
    if (mRigidBodyCache == nullptr) return;
    
    rp3d::Transform offsetTransform;
    offsetTransform.setPosition(rp3d::Vector3(x, y, z));
    
    rp3d::Collider* collider = mRigidBodyCache->addCollider(capsuleShape, offsetTransform);
    collider->setCollideWithMaskBits( (unsigned short)layer );
    
    return;
}

void GameObject::AddColliderSphere(rp3d::SphereShape* sphereShape, float x, float y, float z, LayerMask layer) {
    if (mRigidBodyCache == nullptr) return;
    
    rp3d::Transform offsetTransform;
    offsetTransform.setPosition(rp3d::Vector3(x, y, z));
    
    rp3d::Collider* collider = mRigidBodyCache->addCollider(sphereShape, offsetTransform);
    collider->setCollideWithMaskBits( (unsigned short)layer );
    
    return;
}

void GameObject::AddCollider(ColliderTag* colliderTag, float x, float y, float z, LayerMask layer) {
    if (mRigidBodyCache == nullptr) return;
    assert(colliderTag != nullptr);
    if (colliderTag->isStatic) {
        mRigidBodyCache->setType(rp3d::BodyType::STATIC);
    } else {
        mRigidBodyCache->setType(rp3d::BodyType::DYNAMIC);
    }
    rp3d::Transform offsetTransform;
    offsetTransform.setPosition(rp3d::Vector3(x, y, z));
    
    rp3d::Collider* collider = mRigidBodyCache->addCollider(colliderTag->colliderShape, offsetTransform);
    collider->setCollideWithMaskBits( (unsigned short)layer );
    
    return;
}

void GameObject::AddCollider(MeshCollider* meshCollider, float x, float y, float z, LayerMask layer) {
    
    if (mRigidBodyCache == nullptr) return;
    
    rp3d::Transform offsetTransform;
    offsetTransform.setPosition(rp3d::Vector3(x, y, z));
    
    rp3d::Collider* collider = mRigidBodyCache->addCollider( meshCollider->heightFieldShape, offsetTransform );
    
    collider->setCollideWithMaskBits( (unsigned short)layer );
    
    return;
}

void GameObject::SetStatic(void) {
    if (mRigidBodyCache == nullptr) return;
    mRigidBodyCache->setType(rp3d::BodyType::STATIC);
    return;
}

void GameObject::SetDynamic(void) {
    if (mRigidBodyCache == nullptr) return;
    mRigidBodyCache->setType(rp3d::BodyType::DYNAMIC);
    return;
}

void GameObject::SetKinematic(void) {
    if (mRigidBodyCache == nullptr) return;
    mRigidBodyCache->setType(rp3d::BodyType::KINEMATIC);
    return;
}

void GameObject::SetUserData(void* ptr){
    mUserData = ptr;
    return;
}

void* GameObject::GetUserData(void) {
    return mUserData;
}
