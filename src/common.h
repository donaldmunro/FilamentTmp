#ifndef __NB_H__
#define __NB_H__

#include "filament/Engine.h"
#include "backend/Platform.h"
#include "filament/Renderer.h"
#include "filament/Camera.h"
#include "filament/Scene.h"
#include "filament/View.h"
#include "filament/Material.h"
#include "filament/VertexBuffer.h"
#include "filament/IndexBuffer.h"
#include "filament/RenderableManager.h"
#include "filament/TransformManager.h"
#include <filament/Texture.h>
#include "utils/EntityManager.h"
#include "utils/Entity.h"
#include "math/quat.h"

struct ColorVertex3D
{
   filament::math::float3 position;
   uint32_t color;
};

struct TexVertex3D
{
   filament::math::float3 position;
   filament::math::float2 uv;
};

bool create_scene(filament::Engine::Backend backend, void* nativeWindow, unsigned windowWidth, unsigned windowHeight);
bool render_scene();
void destroy_scene();
#endif