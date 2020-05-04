#include "Font.hpp"

Font::Font(std::string path, SDL_Renderer *r)
{
  surf = SDL_LoadBMP(path.c_str());
  tex = SDL_CreateTextureFromSurface(r, surf);
  w = surf->w;
  h = surf->h;
}

Font::~Font()
{
  if (tex)
    SDL_DestroyTexture(tex);
}

SDL_Rect Font::GetLetter(char c)
{
  return SDL_Rect{};
}

std::vector<SDL_Rect> Font::GetString(std::string s)
{
  return std::vector<SDL_Rect>();
}

void Font::SetPixel(unsigned i, Pixel p)
{
  SetPixel(i % w, i / w, p);
}

void Font::SetPixel(unsigned x, unsigned y, Pixel p)
{
  x %= w;
  y %= h;
}

Pixel Font::GetPixel(unsigned i)
{
  return GetPixel(i % w, i / w);
}

Pixel Font::GetPixel(unsigned x, unsigned y)
{
  x %= w;
  y %= h;
  Pixel p;
  SDL_GetRGBA(*((uint32_t*)surf->pixels + y * w + x), surf->format, &p.r, &p.g, &p.b, &p.a);
  return p;
}
