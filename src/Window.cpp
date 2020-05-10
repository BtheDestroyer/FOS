#define __WINDOW_CPP

#include <algorithm>
#include <string>

#include "Debug.hpp"
#include "Window.hpp"

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

Window::Window(std::string title)
  : sdlWindow(nullptr), resX(resX_), resY(resY_), midFrame(false)
{
  if (count == 0)
  {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
      Debug::LogError(std::string("SDL could not initialize! SDL_Error: ") + std::string(SDL_GetError()));
      return;
    }
  }
  SDL_DisplayMode DM;
  SDL_GetCurrentDisplayMode(0, &DM);
  resX_ = DM.w;
  resY_ = DM.h;
  sdlWindow = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, resX, resY, SDL_WINDOW_SHOWN);
  if (!SDL_GL_SetSwapInterval(0))
  {
    Debug::LogError(std::string("Could not disable vsync! SDL_Error: ") + std::string(SDL_GetError()));
  }
  if (sdlWindow == nullptr)
  {
    Debug::LogError(std::string("Window could not be created! SDL_Error: ") + std::string(SDL_GetError()));
    return;
  }
  sdlSurf = SDL_GetWindowSurface(sdlWindow);
  if (sdlSurf == nullptr)
  {
    Debug::LogError(std::string("Renderer could not be created! SDL_Error: ") + std::string(SDL_GetError()));
    return;
  }
  if (++count > windows.size())
  {
    windows.push_back(this);
    id = count - 1;
  }
  else
  {
    for (uint8_t i = 0; id == -1 && i < windows.size(); ++i)
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

  Update();
}

Window::Window(std::string title, unsigned width, unsigned height)
  : sdlWindow(nullptr), resX(resX_), resY(resY_), resX_(width), resY_(height), midFrame(false)
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
  sdlSurf = SDL_GetWindowSurface(sdlWindow);
  if (sdlSurf == nullptr)
  {
    Debug::LogError(std::string("Renderer could not be created! SDL_Error: ") + std::string(SDL_GetError()));
    return;
  }
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

  Update();
}

Window::~Window()
{
  EndFrame();
  if (sdlSurf)
    sdlSurf = nullptr;
  if (sdlWindow != nullptr)
  {
    SDL_DestroyWindow(sdlWindow);
    sdlWindow = nullptr;
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
  SDL_FillRect(sdlSurf, nullptr, color);
}

void Window::DrawRect(SDL_Rect *rect, unsigned char r, unsigned char g, unsigned char b)
{
  if (!rect)
    SDL_FillRect(sdlSurf, nullptr, Pixel(r, g, b, 0xFF));
  else
  {
    SDL_Rect rect2{rect->x * RESOLUTION_SCALE, rect->y * RESOLUTION_SCALE, rect->w * RESOLUTION_SCALE,  rect->h * RESOLUTION_SCALE};
    SDL_FillRect(sdlSurf, &rect2, Pixel(r, g, b, 0xFF));
  }
}

void Window::DrawRect(SDL_Rect rect, unsigned char r, unsigned char g, unsigned char b)
{
  rect.x *= RESOLUTION_SCALE;
  rect.y *= RESOLUTION_SCALE;
  rect.w *= RESOLUTION_SCALE;
  rect.h *= RESOLUTION_SCALE;
  SDL_FillRect(sdlSurf, &rect, Pixel(r, g, b, 0xFF));
}

void Window::DrawRect(SDL_Rect *rect, Pixel color)
{
  if (!rect)
    SDL_FillRect(sdlSurf, nullptr, color);
  else
  {
    SDL_Rect rect2{rect->x * RESOLUTION_SCALE, rect->y * RESOLUTION_SCALE, rect->w * RESOLUTION_SCALE,  rect->h * RESOLUTION_SCALE};
    SDL_FillRect(sdlSurf, &rect2, color);
  }
}

void Window::DrawRect(SDL_Rect rect, Pixel color)
{
  rect.x *= RESOLUTION_SCALE;
  rect.y *= RESOLUTION_SCALE;
  rect.w *= RESOLUTION_SCALE;
  rect.h *= RESOLUTION_SCALE;
  SDL_FillRect(sdlSurf, &rect, color);
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
  SDL_UpdateWindowSurface(sdlWindow);
  //SDL_RenderPresent(sdlRenderer);
}

void Window::EndFrame()
{
  if (midFrame)
  {
    SwapBuffers();
    midFrame = false;
  }
}

void Window::Update()
{
  EndFrame();
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

SDL_Surface* Window::GetSDLSurface()
{
  return sdlSurf;
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
