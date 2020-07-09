#pragma once
#include <vector>
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
  std::vector<uint8_t> memory;
};
