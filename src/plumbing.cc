#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <regex>
#include <chrono>
#include <thread>

#include "common.h"

#include "SDL.h"
#include <SDL_syswm.h>

unsigned windowWidth =0, windowHeight =0;
SDL_Window* window = nullptr;
void* nativeWindow = nullptr;

void event_loop();

int main(int argc, char** argv)
//------------------------------
   {
   filament::Engine::Backend backend = filament::Engine::Backend::OPENGL;
   if (argc > 1)
   {
      std::string arg(argv[1]);
      std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
      if ( (arg == "vulkan") || (arg == "vulcan") )
         backend = filament::Engine::Backend::VULKAN;
   }
   if (SDL_Init(SDL_INIT_EVENTS /*| SDL_INIT_VIDEO*/) < 0)
   {
      std::cerr << "Error initializing SDL\n";
      return 1;
   }
   SDL_DisplayMode dm;
   dm.w = dm.h = -1;
   if (SDL_GetCurrentDisplayMode(0, &dm) != 0)
   {
      SDL_Window* tmpwin = SDL_CreateWindow("Dummy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1, 1,
                                            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
      if (SDL_GetCurrentDisplayMode(0, &dm) != 0)
         dm.w = dm.h = -1;
      if (tmpwin != nullptr)
         SDL_DestroyWindow(tmpwin);
   }
   if ( (dm.w > 0) && (dm.h > 0) )
   {
      windowWidth = std::max(dm.w - 20, static_cast<int>(windowWidth));
      windowHeight = std::max(dm.h - 20, static_cast<int>(windowHeight));
   }
   window = SDL_CreateWindow("Background Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight,
                             SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
   if (window == nullptr)
   {
      std::cerr << "Error creating SDL window " << SDL_GetError() << std::endl;
      return 1;
   }
   SDL_SysWMinfo wmi;
   SDL_VERSION(&wmi.version);
   if (SDL_GetWindowWMInfo(window, &wmi) == SDL_TRUE)
   {
#ifdef WIN32
      nativeWindow = reinterpret_cast<void*>(wmi.info.win.window);
#else
      nativeWindow = reinterpret_cast<void*>(wmi.info.x11.window);
#endif
   }
   else
   {
      std::cerr << "Error getting native window handle from SDL\n";
      return 1;
   }
   if (! create_scene(backend, nativeWindow, windowWidth, windowHeight))
   {
      std::cerr << "Error creating scene.\n";
      std::exit(1);
   }
   event_loop();
}

bool process_sdl_events(int maxEvents =16)
//----------------------------------------
{
   SDL_Event ev;
   int count = 0;
   while (SDL_PollEvent(&ev))
   {
      if (count++ > maxEvents) return true;
      switch (ev.type)
      {
         case SDL_QUIT: return false;
         case SDL_KEYUP:
         {
            int key = ev.key.keysym.scancode;
            bool isCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
            // bool isShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
            if ( (key == SDL_SCANCODE_ESCAPE) || ( (key == SDL_SCANCODE_Q) && (isCtrl) ) )
               return false;
//            switch (key)
//            {
//               case SDL_SCANCODE_LEFT:
//                  break;
//
//               case SDL_SCANCODE_RIGHT:
//                  break;
//            }
         }
      }
   }
   return true;
}

void event_loop()
//---------------
{
   double fps = 40;
   long spf = 1000000000L / fps;
   auto frame_start = std::chrono::high_resolution_clock::now();
   bool is_quit = false;
   do
   {
      if (window == nullptr) continue;
      if (! process_sdl_events())
         break;
      auto frame_end = std::chrono::high_resolution_clock::now();
      long elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(frame_end - frame_start).count(), last_duration;
      // std::cout << elapsed << " " << spf << std::endl;
      if (elapsed >= spf)
      {
         frame_start = frame_end;
         render_scene();
         last_duration = elapsed;
//         renderer->resetUserTime();
      }
      else //if (elapsed < spf)
         std::this_thread::sleep_for(std::chrono::milliseconds(5));
   } while (! is_quit);
   destroy_scene();
}