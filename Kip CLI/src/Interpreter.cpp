#include <iostream>
#include <fstream>

#include "Interpreter.h"
#include "kip.h"

Interpreter::Interpreter()
{
  std::cout << "Kip Command Line Interpreter         |RAM: $000 - $800 |" << std::endl;
  std::cout << "(c) Bryce Dixon 2020                 |-----------------/" << std::endl;
  std::cout << "https://github.com/bthedestroyer/kip |" << std::endl;
  std::cout << "-------------------------------------/" << std::endl;
  std::cout << "Kip V" << kip::versionMajor << "." << kip::versionMinor << std::endl << std::endl;
  kip::MapMemory(memory.data(), uint32_t(memory.size()), 0x0000);
}

Interpreter::~Interpreter()
{
  kip::UnmapMemory(memory.data());
}

bool Interpreter::Running()
{
  return running;
}

void Interpreter::CLIInput()
{
  char input[1024];
  std::cout << "-> ";
  std::cin.getline(input, 1024);
  kip::InterpretResult r = kip::InterpretLine(input);
  if (!r.success)
    std::cerr << "ERR: " << r.str << std::endl;
  else
    std::cout << r.str << std::endl;
}

void Interpreter::RunFile(std::string filename)
{
  std::ifstream file(filename);
  if (file.is_open())
  {
    std::vector<std::string> text;
    char line[2048];
    while (!file.eof())
    {
      file.getline(line, 2048);
      text.push_back(line);
    }
    file.close();
    std::vector<kip::InterpretResult> results = kip::InterpretLines(text);
    for (kip::InterpretResult result : results)
    {
      if (!result.success)
      {
        std::cerr << "ERR: " << result.str << std::endl;
        break;
      }
      else
      {
        std::cout << result.str << std::endl;
      }
    }
  }
  else
    std::cerr << "ERR: Couldn't read file " << filename << std::endl;
}
