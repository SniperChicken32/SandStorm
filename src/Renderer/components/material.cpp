#include <GameEngineFramework/Renderer/components/material.h>

#define GLEW_STATIC
#include <gl/glew.h>


Material::Material() : 
    
    width(0),
    height(0),
    
    doDepthTest(true),
    doBlending(false),
    doFaceCulling(true),
    doShadowPass(true),
    
    shadowDistance(30),
    shadowStencilLength(5),
    
    textureFiltration(GL_LINEAR_MIPMAP_LINEAR),
    
    depthFunc(MATERIAL_DEPTH_LESS),
    faceWinding(GL_CCW),
    faceCullSide(GL_BACK),
    blendSource(BLEND_SRC_COLOR),
    blendDestination(BLEND_SRC_ALPHA),
    blendAlphaSource(BLEND_ONE_MINUS_SRC_COLOR),
    blendAlphaDestination(BLEND_ONE_MINUS_SRC_ALPHA),
    blendFunction(BLEND_EQUATION_ADD),
    
    shader(nullptr)
{
    ambient = Color(0, 0, 0, 1);
    diffuse = Color(1, 1, 1, 1);
    specular = Color(0, 0, 0, 1);
    
    glGenTextures(1, &mTextureBuffer);
    
    return;
}

Material::~Material() {
    glDeleteTextures(1, &mTextureBuffer);
    return;
}

void Material::EnableDepthTest(void)  {doDepthTest = true;}
void Material::DisableDepthTest(void) {doDepthTest = false;}
void Material::SetDepthFunction(GLint func) {depthFunc = func;}

GLint Material::GetTextureFiltration(void) {return textureFiltration;}
void Material::SetTextureFiltration(GLint filtration) {textureFiltration = filtration;}

void Material::EnableCulling(void) {doFaceCulling = true;}
void Material::DisableCulling(void) {doFaceCulling = false;}
void Material::SetFaceCullingSide(GLint side) {faceCullSide = side;}
void Material::SetFaceWindingOrder(GLint direction) {faceWinding = direction;}

void Material::EnableBlending(void) {doBlending = true;}
void Material::DisableBlending(void) {doBlending = false;}
void Material::SetBlending(GLint src, GLint dest)  {blendSource = src; blendDestination = dest;}
void Material::SetBlendingAlpha(GLint srca, GLint desta)  {blendAlphaSource = srca; blendAlphaDestination = desta;}
void Material::SetBlendingFunction(GLint func)  {blendFunction = func;}

void Material::EnableShadowPass(void) {doShadowPass = true; return;}
void Material::DisableShadowPass(void) {doShadowPass = false; return;}



void Material::Bind(void) {glBindTexture(GL_TEXTURE_2D, mTextureBuffer); return;}

void Material::BindTextureSlot(unsigned int slot) {glActiveTexture(GL_TEXTURE0 + slot); return;}

void Material::UpdateTextureBuffer(void* textureData) {
    glBindTexture(GL_TEXTURE_2D, mTextureBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureFiltration);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    return;
}

void Material::GenerateMipMaps(void) {
    glBindTexture(GL_TEXTURE_2D, mTextureBuffer);
    glGenerateMipmap(GL_TEXTURE_2D);
    return;
}
    
