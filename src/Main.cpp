#include <fstream>

#include "Debug.hpp"
#include "Window.hpp"
#include "Input.hpp"

#include "kip.h"

#define MULTITHREAD_MODE 1

class MainCore
{
public:
  MainCore()
    : window("Fantasy OS", 256, 256)
  {
    Debug::Log("Starting...");
    //SDL_SetWindowFullscreen(window.GetSDLWindow(), SDL_WINDOW_FULLSCREEN);

    // Multi threading mode
#if MULTITHREAD_MODE
    // Start Drawing
    graphicsThread = std::thread(&MainCore::DrawLoop, this);
#endif

    window.Clear(Color::BLACK);
    drawSurface = SDL_GetWindowSurface(window.GetSDLWindow());
    kip::MapMemory((uint8_t*)(drawSurface->pixels), window.resX * window.resY * 4, 0x4000);
    Debug::Log("Started!");
    running = true;
  }

  ~MainCore()
  {
    Debug::Log("Shutting down...");
    kip::UnmapMemory(0x4000);
    drawSurface = nullptr;
    // Multi threading mode
#if MULTITHREAD_MODE
    // Start Drawing
    graphicsThread.join();
#endif
  }

  void Update()
  {
    static auto last = std::chrono::high_resolution_clock::now();
    static auto now = std::chrono::high_resolution_clock::now();
    static auto elapsed = now - last;
    static long long micro = 0;
    static float dt = 0;
    now = std::chrono::high_resolution_clock::now();
    elapsed = now - last;
    micro = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    dt = micro * 0.000001f;
    last = now;
    Update(dt);
  }

  void Update(float dt)
  {
    if (!running)
      return;
    timeTillRender += dt;
    input.Update();
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
      if (e.type == SDL_QUIT)
        running = false;
      else
      {
        window.HandleEvent(&e);
        input.HandleEvent(&e);
      }
    }

    // Single threading mode
#if !MULTITHREAD_MODE
    if (timeTillRender >= 1.0f / 60.0f)
    {
      Draw(dt);
      timeTillRender -= 1.0f / 60.0f;
    }
#endif
  }
  
  void DrawLoop()
  {
    auto last = std::chrono::high_resolution_clock::now();
    auto now = last;
    auto elapsed = now - last;
    long long micro;
    float dt = 0;
    while (running)
    {
      while (dt <= 1.0f / 60.0f)
      {
        now = std::chrono::high_resolution_clock::now();
        elapsed = now - last;
        micro = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        dt = micro * 0.000001f;
      }
      Draw(dt);
      last = now;
    }
  }

  void Draw(float dt)
  {
    window.Update();
  }

  }
  
  void UpdateImGUI(float dt)
  {
  }

  bool running = false;
  Window window;
  Input input;
  float timeTillRender = 0;

  SDL_Surface *drawSurface;

#if MULTITHREAD_MODE
  std::thread graphicsThread;
#endif
};

int main(int argc, char* argv[])
{
  MainCore core;
  while (core.running)
    core.Update();
  return 0;
}

