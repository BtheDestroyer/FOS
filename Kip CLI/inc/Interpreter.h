#pragma once
#include <array>
#include <string>
#include <cstdint>

class Interpreter
{
public:
  Interpreter();
  ~Interpreter();

  bool Running();

  void CLIInput();
  void RunFile(std::string filename);

private:
  bool running = true;
  std::array<uint8_t, 0x10000> memory;
};

