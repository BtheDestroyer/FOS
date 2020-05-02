#include <string>
#include <vector>
#include "Window.hpp"
#include "SDL.h"

class Font
{
public:
  Font(std::string path, SDL_Renderer* r);
  ~Font();

  SDL_Rect GetLetter(char c);
  std::vector<SDL_Rect> GetString(std::string s);
  void SetPixel(unsigned i, Pixel p);
  void SetPixel(unsigned x, unsigned y, Pixel p);
  Pixel GetPixel(unsigned i);
  Pixel GetPixel(unsigned x, unsigned y);

  SDL_Texture* tex = nullptr;

private:
  unsigned w, h;
  SDL_Surface* surf = nullptr;
};