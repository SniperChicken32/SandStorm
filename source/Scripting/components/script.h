#ifndef __COMPONENT_SCRIPT
#define __COMPONENT_SCRIPT

void DefaultFunctionPtr(void* gameObject);

#define  nullfunc  DefaultFunctionPtr



struct Script {
    
    /// State whether the script is active and should be updated.
    bool isActive;
    
    /// State whether OnCreate() has already been called.
    bool hasBeenInitiated;
    
    /// Pointer to the game object who owns this script.
    void* gameObject;
    
    
    /// This function will be called on object creation.
    void(*OnCreate)(void* gameObjectPtr);
    
    /// This function will be called once per frame.
    void(*OnUpdate)(void* gameObjectPtr);
    
    
    Script();
    
};


#endif
