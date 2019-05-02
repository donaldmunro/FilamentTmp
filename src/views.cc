#include <iostream>
#include <fstream>
#include <vector>

#include "common.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb/stb_image_resize.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

const float TRIANGLE_Z = -2.0f;
const float BACKGROUND_Z = -1.0f;

static ColorVertex3D TRIANGLE_VERTICES[] =
{
      {{1, 0, TRIANGLE_Z}, 0xffff0000u},
      {{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3), TRIANGLE_Z}, 0xff00ff00u},
      {{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3), TRIANGLE_Z}, 0xff0000ffu},
};

static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };

static TexVertex3D QUAD_VERTICES[] =
   {
         {{-1, -1, BACKGROUND_Z}, {0, 0}},
         {{ 1, -1, BACKGROUND_Z}, {1, 0}},
         {{-1,  1, BACKGROUND_Z}, {0, 1}},
         {{ 1,  1, BACKGROUND_Z}, {1, 1}},
   };

static constexpr uint16_t QUAD_INDICES[6] = {0, 1, 2, 3, 2, 1,};


const std::string BACKGROUND_IMAGE("images/Filament_Logo.png");
filament::Engine* engine = nullptr;
filament::SwapChain* swapChain = nullptr;
filament::Scene* scene = nullptr, *backgroundScene = nullptr;
filament::View* view = nullptr, *backgroundView =nullptr;
filament::VertexBuffer* vb = nullptr, *texvb = nullptr;
filament::IndexBuffer* ib = nullptr, *texib = nullptr;
filament::Material* triangleMaterial = nullptr, *backgroundMaterial = nullptr;
filament::Texture* backgroundTexture = nullptr;
filament::TextureSampler backgroundSampler(filament::TextureSampler::MinFilter::LINEAR,
                                           filament::TextureSampler::MagFilter::LINEAR);
filament::MaterialInstance* backgroundMaterialInst = nullptr;
filament::Camera* perspectiveCamera = nullptr, *backgroundCamera = nullptr;
utils::EntityManager& entityManager = utils::EntityManager::get();
utils::Entity triangle = entityManager.create(), background = entityManager.create();;
filament::Renderer* renderer = nullptr;

bool render_scene()
//-----------------
{
   backgroundView->setClearTargets(true, true, false);
   view->setClearTargets(false, true, false);
   view->setRenderTarget(filament::View::TargetBufferFlags::DEPTH_AND_STENCIL);
   view->setClearColor(filament::Color::toLinear({1, 0, 0, 0}));
   backgroundView->setClearColor(filament::Color::toLinear({0, 1, 0, 1}));
   view->setPostProcessingEnabled(true);
   backgroundView->setPostProcessingEnabled(false);
   backgroundView->setDepthPrepass(filament::View::DepthPrepass::DISABLED);

   // view->setClearTargets(true, true, false);
   // backgroundView->setClearTargets(false, false, false);
   // backgroundView->setRenderTarget(filament::View::TargetBufferFlags::DEPTH_AND_STENCIL);
   // backgroundView->setPostProcessingEnabled(false);
   // view->setPostProcessingEnabled(true);
   // backgroundView->setRenderTarget(filament::View::TargetBufferFlags::DEPTH_AND_STENCIL);

   if (renderer->beginFrame(swapChain))
   {
      renderer->render(backgroundView);
      renderer->render(view);

      // renderer->render(view);
      // renderer->render(backgroundView);

      renderer->endFrame();
      return true;
   }
   return false;
}

bool set_background(const char* BACKGROUND_IMAGE, unsigned windowWidth, unsigned windowHeight);

bool create_scene(filament::Engine::Backend backend, void* nativeWindow, unsigned windowWidth, unsigned windowHeight)
//-------------------------------------------------------------------------------------------------
{
   std::ifstream ifs("material/bakedColor");
   if (! ifs.good())
   {
      std::cerr << "Error opening material file\n";
      return false;
   }
   std::vector<char> triangle_material((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
   if (triangle_material.empty())
   {
      std::cerr << "Error opening material file\n";
      return false;
   }
   ifs.close();
   ifs.open("material/bakedTexture");
   std::vector<char> tex_material((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
   if (tex_material.empty())
   {
      std::cerr << "Error opening texture material file\n";
      return false;
   }
   engine = filament::Engine::create(backend);
   swapChain = engine->createSwapChain(nativeWindow);
   view = engine->createView();
   view->setViewport({ 0, 0, windowWidth, windowHeight });
   perspectiveCamera = engine->createCamera();
   perspectiveCamera->lookAt({0, 0, 0}, {0, 0, -1}, {0, 1, 0});
   std::cout << "Camera position " << perspectiveCamera->getPosition() << "Forward " << perspectiveCamera->getForwardVector() << std::endl;

   perspectiveCamera->setProjection(50, double(windowWidth) / double(windowHeight), 0.0625, 10.0); //, filament::Camera::Fov::HORIZONTAL);
   view->setCamera(perspectiveCamera);
   scene = engine->createScene();

   vb = filament::VertexBuffer::Builder().bufferCount(1).vertexCount(3)
         .attribute(filament::VertexAttribute::POSITION, 0, filament::VertexBuffer::AttributeType::FLOAT3, 0, 16)
         .attribute(filament::VertexAttribute::COLOR, 0, filament::VertexBuffer::AttributeType::UBYTE4, 12, 16)
         .normalized(filament::VertexAttribute::COLOR)
         .build(*engine);
   ib = filament::IndexBuffer::Builder().indexCount(3).bufferType(filament::IndexBuffer::IndexType::USHORT).build(*engine);
   vb->setBufferAt(*engine, 0, filament::VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 48, nullptr));
   ib->setBuffer(*engine, filament::IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));
   triangleMaterial = filament::Material::Builder().package(triangle_material.data(), triangle_material.size()).build(*engine);
   filament::RenderableManager::Builder(1).boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
         .material(0, triangleMaterial->getDefaultInstance())
         .geometry(0, filament::RenderableManager::PrimitiveType::TRIANGLES, vb, ib, 0, 3)
         .culling(false)
         .receiveShadows(false)
         .castShadows(false)
         .build(*engine, triangle);
   filament::TransformManager& transformManager = engine->getTransformManager();
   utils::EntityInstance<filament::TransformManager> triangleTransform = transformManager.getInstance(triangle);
   transformManager.setTransform(triangleTransform, filament::math::mat4f::translation(filament::math::float3(-2.0f, 0.06f, 0.0f))*
          filament::math::mat4f::scaling(filament::math::float3(1.5, 1.5, 1)));
   view->setScene(scene);
   scene->addEntity(triangle);

   backgroundCamera = engine->createCamera();
   backgroundCamera->setProjection(filament::Camera::Projection::ORTHO, -1, 1, -1, 1, -1, 1); //0.0625, 2);
   backgroundCamera->lookAt({0, 0, 0}, {0, 0, -1}, {0, 1, 0});
   backgroundView = engine->createView();
   backgroundView->setViewport({ 0, 0, windowWidth, windowHeight });
   backgroundView->setShadowsEnabled(false);
   backgroundView->setCamera(backgroundCamera);
   backgroundScene = engine->createScene();
   backgroundView->setScene(backgroundScene);
   backgroundTexture = filament::Texture::Builder().width(uint32_t(windowWidth)).height(uint32_t(windowHeight))
                                                   .levels(1).sampler(filament::Texture::Sampler::SAMPLER_2D)
                                                   .format(filament::Texture::InternalFormat::RGBA8).build(*engine);
   texvb = filament::VertexBuffer::Builder().vertexCount(4).bufferCount(1)
                                            .attribute(filament::VertexAttribute::POSITION, 0,
                                                       filament::VertexBuffer::AttributeType::FLOAT3, 0, 20)
                                            .attribute(filament::VertexAttribute::UV0, 0,
                                                       filament::VertexBuffer::AttributeType::FLOAT2, 12, 20).build(*engine);
   texib = filament::IndexBuffer::Builder().indexCount(6).bufferType(filament::IndexBuffer::IndexType::USHORT).build(*engine);
   texvb->setBufferAt(*engine, 0, filament::VertexBuffer::BufferDescriptor(QUAD_VERTICES, 80, nullptr));
   texib->setBuffer(*engine, filament::IndexBuffer::BufferDescriptor(QUAD_INDICES, 12, nullptr));
   backgroundMaterial = filament::Material::Builder().package(tex_material.data(), tex_material.size()).build(*engine);
   backgroundMaterialInst = backgroundMaterial->getDefaultInstance();
   backgroundMaterialInst->setParameter("albedo", backgroundTexture, backgroundSampler);
   filament::RenderableManager::Builder(1).boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
         .material(0, backgroundMaterialInst)
         .geometry(0, filament::RenderableManager::PrimitiveType::TRIANGLES, texvb, texib, 0, 6)
         .culling(false)
         .receiveShadows(false)
         .castShadows(false)
         .build(*engine, background);
   // utils::EntityInstance<filament::TransformManager> backgroundTransform = transformManager.getInstance(background);
   // transformManager.setTransform(backgroundTransform, filament::math::mat4f::scaling(filament::math::float3(0.5, 0.5, 1)));
   backgroundScene->addEntity(background);
   set_background(BACKGROUND_IMAGE.c_str(), windowWidth, windowHeight);

   renderer = engine->createRenderer();
   return (renderer != nullptr);
}

bool set_background(const char* imagefile, unsigned windowWidth, unsigned windowHeight)
//--------------------------------------------------------------------------------------
{
   int imageWidth, imageHeight, imageChannels=3, reqChannels;
   filament::Texture::InternalFormat texFormat = backgroundTexture->getFormat();
   if (texFormat == filament::Texture::InternalFormat::RGBA8)
      reqChannels = 4;
   else
      reqChannels = 3;
   size_t size = windowWidth*windowHeight*reqChannels;
   unsigned char *data = stbi_load(imagefile, &imageWidth, &imageHeight, &imageChannels, reqChannels), *pdata = nullptr;
   if (data != nullptr)
   {
      pdata = new unsigned char[size];
      if ( (imageWidth != windowWidth) || (imageHeight != windowHeight) )
      {
         if (! stbir_resize_uint8(data, imageWidth, imageHeight, 0, pdata, windowWidth, windowHeight, 0, reqChannels))
         {
            delete[] pdata;
            pdata = nullptr;
            std::cerr << "Error resizing background image" << std::endl;
         }
      }
      else
      {
         memcpy(pdata, data, size);
         pdata = data;
      }
      stbi_image_free(data);
      if (pdata != nullptr)
      {
         // stbi_write_jpg("resized.jpg", windowWidth, windowHeight, 4, pdata, 9);
         filament::Texture::PixelBufferDescriptor buffer(pdata, size_t(size),
                                                      filament::Texture::Format::RGBA, filament::Texture::Type::UBYTE,
                                                      [](void* p, size_t size, void* user)
                                                      //----------------------------------
                                                      {
                                                         if (p)
                                                         {
                                                            unsigned char *data = static_cast<unsigned char *>(p);
                                                            delete[] data;
                                                         }
                                                      });
         backgroundTexture->setImage(*engine, 0, std::move(buffer));
      }
   }
   return (pdata != nullptr);
}

void destroy_scene()
//--------------------
{
   if (engine)
   {
      filament::Fence::waitAndDestroy(engine->createFence());
      if (perspectiveCamera) engine->destroy(perspectiveCamera); perspectiveCamera = nullptr;
      if (backgroundCamera) engine->destroy(backgroundCamera); backgroundCamera = nullptr;
      if (view) engine->destroy(view); view = nullptr;
      if (backgroundView) engine->destroy(backgroundView); backgroundView = nullptr;
      if (backgroundMaterial) engine->destroy(backgroundMaterial); backgroundMaterial = nullptr;  
      if (backgroundTexture) engine->destroy(backgroundTexture); backgroundTexture = nullptr;  
      engine->destroy(triangle);
      engine->destroy(background);
      if (scene) engine->destroy(scene); scene = nullptr;
      if (backgroundScene) engine->destroy(backgroundScene); backgroundScene = nullptr;
      if (vb) engine->destroy(vb); vb = nullptr;
      if (ib) engine->destroy(ib); ib = nullptr;
      if (texvb) engine->destroy(texvb); vb = nullptr;
      if (texib) engine->destroy(texib); ib = nullptr;

    }
}
