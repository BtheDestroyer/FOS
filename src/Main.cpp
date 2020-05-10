#include <fstream>
#include <array>

#include "Debug.hpp"
#include "Window.hpp"
#include "Input.hpp"

#include "kip.h"

#define MULTITHREAD_MODE 1

class MainCore
{
public:
  MainCore()
    : window("Fantasy OS")
  {
    Debug::Log("Starting...");

    kip::MapMemory(kipMem.data(), kipMem.size(), 0x0000);
    window.Clear(Color::BLACK);
    drawSurface = SDL_CreateRGBSurfaceWithFormat(0, 128, 128, 24, SDL_PIXELFORMAT_RGB24);
    if (drawSurface == nullptr)
    {
      Debug::LogError("Could not create drawSurface!");
      return;
    }
    kip::MapMemory((uint8_t*)(drawSurface->pixels), window.resX * window.resY * 4, 0x4000);
    Debug::Log("Started!");
    running = true;

    // Multi threading mode
#if MULTITHREAD_MODE
    // Start Drawing
    graphicsThread = std::thread(&MainCore::DrawLoop, this);
#endif
  }

  ~MainCore()
  {
    Debug::Log("Shutting down...");
    running = false;
    // Multi threading mode
#if MULTITHREAD_MODE
    // Start Drawing
    graphicsThread.join();
#endif
    kip::UnmapMemory(kipMem.data());
    if (drawSurface)
    {
      kip::UnmapMemory((uint8_t*)(drawSurface->pixels));
      SDL_FreeSurface(drawSurface);
      drawSurface = nullptr;
    }
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
      do
      {
        now = std::chrono::high_resolution_clock::now();
        elapsed = now - last;
        micro = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
        dt = micro * 0.000001f;
      } while (dt <= 1.0f / 60.0f);
      Draw(dt);
      last = now;
    }
  }

  void Draw(float dt)
  {
    if (drawSurface)
    {
      SDL_Rect drect{ 0, 0, window.resX, window.resY };
      if (window.resX > window.resY)
        drect = { int((window.resX - window.resY) * 0.5f), 0, window.resY, window.resY };
      else if (window.resX < window.resY)
        drect = { 0, int((window.resY - window.resX) * 0.5f), window.resX, window.resX };
      SDL_BlitScaled(drawSurface, nullptr, window.GetSDLSurface(), &drect);
    }
    window.Update();
  }

  bool RunBIOS()
  {
    return RunFile("rec/bios.kip");
  }

  bool RunFile(std::string filename)
  {
    std::vector<std::string> lines;
    kip::InterpretResult result = kip::LoadFile(filename, lines);
    if (!result.success)
    {
      Debug::LogError(result.str);
      return false;
    }
    size_t folderLen = 0;
    if (filename.find_last_of('/') != size_t(-1))
      folderLen = filename.find_last_of('/');
    if (filename.find_last_of('\\') != size_t(-1))
      folderLen = std::max(folderLen, filename.find_last_of('\\'));
    std::string folder = filename.substr(0, folderLen);
    std::vector<kip::InterpretResult> results = kip::InterpretLines(lines, folder);
    if (results.size() > 0 && !results.back().success)
    {
      Debug::LogError("Error in script: " + results.back().str);
      return false;
    }
    return true;
  }

  bool running = false;
  Window window;
  Input input;
  float timeTillRender = 0;

  std::array<uint8_t, 0x4000> kipMem;
  SDL_Surface *drawSurface;

#if MULTITHREAD_MODE
  std::thread graphicsThread;
#endif
};

int main(int argc, char* argv[])
{
  MainCore core;
  if (core.RunBIOS())
    while (core.running)
      core.Update();
  return 0;
}

