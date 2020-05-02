#define __WINDOW_CPP

#include <algorithm>
#include <string>

#include "Debug.hpp"
#include "Window.hpp"

#pragma warning(push, 0)
#include "imgui.h"
#include "imgui_sdl.h"
#pragma warning(pop)

#undef __WINDOW_CPP

std::string hex(uint64_t n, uint8_t d)
{
  std::string s(d, '0');
  for (int i = d - 1; i >= 0; i--, n >>= 4)
    s[i] = "0123456789ABCDEF"[n & 0xF];
  return s;
};

Pixel::Pixel() : r(0), g(0), b(0), a(255) {}
Pixel::Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) : r(red), g(green), b(blue), a(alpha) {}
Pixel::Pixel(uint32_t p) : n(p) {}

bool Pixel::operator==(const Pixel& p) const
{
  return n == p.n;
}
bool Pixel::operator!=(const Pixel& p) const
{
  return n != p.n;
}

/////////////////////////////////////////////////////

Sprite::Sprite() : width(0), height(0), pColData(nullptr) {}

Sprite::Sprite(int32_t w, int32_t h)
{
  Resize(w, h);
}

Sprite::~Sprite()
{
  if (pColData) delete pColData;
}

void Sprite::Resize(int32_t w, int32_t h)
{
  width = w;
  height = h;
  pColData = new Pixel[width * height];
  for (int32_t i = 0; i < width * height; i++)
    pColData[i] = Pixel();
}

Pixel Sprite::GetPixel(int32_t x, int32_t y)
{
  if (modeSample == Sprite::Mode::NORMAL)
  {
    if (x >= 0 && x < width && y >= 0 && y < height)
      return pColData[y * width + x];
    else
      return Pixel(0, 0, 0, 0);
  }
  else
  {
    return pColData[std::abs(y % height) * width + std::abs(x % width)];
  }
}

bool  Sprite::SetPixel(int32_t x, int32_t y, Pixel p)
{
  if (x >= 0 && x < width && y >= 0 && y < height)
  {
    pColData[y*width + x] = p;
    return true;
  }
  else
    return false;
}

Pixel Sprite::Sample(float x, float y)
{
  return GetPixel(std::min((int32_t)((x * (float)width)), width - 1), std::min((int32_t)((y * (float)height)), height - 1));
}

/////////////////////////////////////////////////////

Window* Window::mainWindow = nullptr;
uint8_t Window::count = 0;
std::vector<Window*> Window::windows;

Window::Window(std::string title, unsigned width, unsigned height)
  : sdlWindow(nullptr), resX(width), resY(height), midFrame(false)
{
  if(count == 0)
  {
    if (SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0)
    {
      Debug::LogError(std::string("SDL could not initialize! SDL_Error: ") + std::string(SDL_GetError()));
      return;
    }
  }
  sdlWindow = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, resX, resY, SDL_WINDOW_SHOWN);
  if (!SDL_GL_SetSwapInterval(0))
  {
    Debug::LogError(std::string("Could not disable vsync! SDL_Error: ") + std::string(SDL_GetError()));
  }
  if(sdlWindow == nullptr)
  {
    Debug::LogError(std::string("Window could not be created! SDL_Error: ") + std::string(SDL_GetError()));
    return;
  }
  int drivers = SDL_GetNumRenderDrivers();
  Debug::Log("Render driver count: " + std::to_string(drivers));
  sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
  if(sdlRenderer == nullptr)
  {
    Debug::LogError(std::string("Renderer could not be created! SDL_Error: ") + std::string(SDL_GetError()));
    return;
  }
  /*
  sdlTextureRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
  if (sdlTextureRenderer == nullptr)
  {
    Debug::LogError(std::string("Texture Renderer could not be created! SDL_Error: ") + std::string(SDL_GetError()));
    return;
  }
  */
  ImGui::CreateContext();
  ImGuiSDL::Initialize(sdlRenderer, resX, resY);
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
  io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
  io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
  io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
  io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
  io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
  io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
  io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
  io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
  io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
  io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
  io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
  io.KeyMap[ImGuiKey_KeyPadEnter] = SDL_SCANCODE_KP_ENTER;
  io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
  io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
  io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
  io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
  io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
  io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;
  SetSDLRenderTarget(nullptr);
  if (++count > windows.size())
  {
    windows.push_back(this);
    id = count - 1;
  }
  else
  {
    for (uint8_t i = 0 ; id == -1 && i < windows.size(); ++i)
    {
      if (windows[i] == nullptr)
      {
        windows[i] = this;
        id = i;
      }
    }
  }
  if (id == -1)
    Debug::LogError("Window couldn't be added to static list for some reason!");

  Debug::Log("Created window " + std::to_string(id));

  Clear();
  ReleaseSDLRenderer();
  Update();
}

Window::~Window()
{
  EndFrame();
  ImGuiSDL::Deinitialize();
  if (sdlTextureRenderer)
  {
    SDL_DestroyRenderer(sdlTextureRenderer);
    sdlTextureRenderer = nullptr;
  }
  if (sdlRenderer)
  {
    SDL_DestroyRenderer(sdlRenderer);
    sdlRenderer = nullptr;
  }
  if (sdlWindow != nullptr)
  {
    ImGui::DestroyContext();
    SDL_DestroyWindow(sdlWindow);
    sdlWindow = nullptr;
  }
  if (fontTex)
  {
    SDL_DestroyTexture(fontTex);
    fontTex = nullptr;
  }
  if (--count == 0)
  {
    SDL_Quit();
  }
  if (id != -1)
  {
    if (windows[id] == mainWindow)
      mainWindow = nullptr;
    windows[id] = nullptr;
    id = -1;
  }
}

void Window::SetPixelMode(Pixel::Mode m)
{
  nPixelMode = m;
}

void Window::Clear(Pixel color)
{
  SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, 255);
  DrawRect({ 0, 0, resX / RESOLUTION_SCALE, resY / RESOLUTION_SCALE }, color);
  SDL_RenderClear(sdlRenderer);
  DrawRect({0, 0, resX / RESOLUTION_SCALE, resY / RESOLUTION_SCALE}, color);
}

void Window::DrawRect(SDL_Rect *rect, unsigned char r, unsigned char g, unsigned char b)
{
  SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
  if (!rect)
    SDL_RenderDrawRect(sdlRenderer, nullptr);
  else
  {
    SDL_Rect rect2{rect->x * RESOLUTION_SCALE, rect->y * RESOLUTION_SCALE, rect->w * RESOLUTION_SCALE,  rect->h * RESOLUTION_SCALE};
    SDL_RenderDrawRect(sdlRenderer, &rect2);
  }
}

void Window::DrawRect(SDL_Rect rect, unsigned char r, unsigned char g, unsigned char b)
{
  rect.x *= RESOLUTION_SCALE;
  rect.y *= RESOLUTION_SCALE;
  rect.w *= RESOLUTION_SCALE;
  rect.h *= RESOLUTION_SCALE;
  SDL_SetRenderDrawColor(sdlRenderer, r, g, b, 255);
  SDL_RenderDrawRect(sdlRenderer, &rect);
}

void Window::DrawRect(SDL_Rect *rect, Pixel color)
{
  SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, 255);
  if (!rect)
    SDL_RenderDrawRect(sdlRenderer, nullptr);
  else
  {
    SDL_Rect rect2{rect->x * RESOLUTION_SCALE, rect->y * RESOLUTION_SCALE, rect->w * RESOLUTION_SCALE,  rect->h * RESOLUTION_SCALE};
    SDL_RenderDrawRect(sdlRenderer, &rect2);
  }
}

void Window::DrawRect(SDL_Rect rect, Pixel color)
{
  rect.x *= RESOLUTION_SCALE;
  rect.y *= RESOLUTION_SCALE;
  rect.w *= RESOLUTION_SCALE;
  rect.h *= RESOLUTION_SCALE;
  SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, 255);
  SDL_RenderDrawRect(sdlRenderer, &rect);
}

void Window::DrawPixel(int32_t x, int32_t y, unsigned char r, unsigned char g, unsigned char b)
{
  DrawRect({x, y, 1, 1}, r, g, b);
}

void Window::DrawPixel(int32_t x, int32_t y, Pixel color)
{
  DrawRect({x, y, 1, 1}, color);
}

void Window::SwapBuffers()
{
  //SDL_UpdateWindowSurface(sdlWindow);
  SDL_RenderPresent(sdlRenderer);
}

void Window::EndFrame()
{
  if (midFrame)
  {
    ImGui::Render();
    SetSDLRenderTarget(nullptr);
    ImGuiSDL::Render(ImGui::GetDrawData());
    SwapBuffers();
    ReleaseSDLRenderer();
    midFrame = false;
  }
}

void Window::Update()
{
  EndFrame();
  ImGui::NewFrame();
  midFrame = true;
}

bool Window::HandleEvent(SDL_Event *event)
{
  return false;
}

SDL_Window *Window::GetSDLWindow()
{
  return sdlWindow;
}

SDL_Renderer *Window::GetSDLRenderer()
{
  return sdlRenderer;
}

SDL_Renderer* Window::GetSDLTextureRenderer()
{
  if (sdlTextureRenderer)
    return sdlTextureRenderer;
  return sdlRenderer;
}

void Window::SetSDLRenderTarget(SDL_Texture *target)
{
  rendererLocked.lock();
  if (sdlRTarget != target)
    if (SDL_SetRenderTarget(sdlRenderer, sdlRTarget = target))
      Debug::LogError(std::string("Could not set render target! SDL_Error: ") + std::string(SDL_GetError()));
}

void Window::ReleaseSDLRenderer()
{
  rendererLocked.unlock();
}

void Window::SetSDLTextureRenderTarget(SDL_Texture* target)
{
  if (sdlTextureRenderer)
  {
    rendererTextureLocked.lock();
    if (sdlRTextureTarget != target)
      if (SDL_SetRenderTarget(sdlTextureRenderer, sdlRTextureTarget = target))
        Debug::LogError(std::string("Could not set render target! SDL_Error: ") + std::string(SDL_GetError()));
  }
  else
    SetSDLRenderTarget(target);
}

void Window::ReleaseSDLTextureRenderer()
{
  if (sdlTextureRenderer)
  {
    rendererTextureLocked.unlock();
  }
  else
    ReleaseSDLRenderer();
}

int Window::GetId()
{
  return id;
}

Window *Window::GetWindow()
{
  if (mainWindow)
    return mainWindow;
  for (uint8_t i = 0; i < windows.size(); ++i)
    if (windows[i] != nullptr)
      return mainWindow = windows[i];
  return nullptr;
}

Window *Window::GetWindow(uint8_t id)
{
  if (id < 0 || id >= windows.size())
    return nullptr;
  return windows[id];
}
