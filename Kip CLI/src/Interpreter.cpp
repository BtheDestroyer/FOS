#include <iostream>
#include <fstream>

#include "Interpreter.h"
#include "kip.h"

Interpreter::Interpreter()
{
  std::cout << "Kip Command Line Interpreter         |RAM: $0000 - $FFFF |" << std::endl;
  std::cout << "(c) Bryce Dixon 2020                 |-------------------/" << std::endl;
  std::cout << "https://github.com/bthedestroyer/kip |" << std::endl;
  std::cout << "-------------------------------------/" << std::endl;
  std::cout << "Kip V" << uint32_t(kip::versionMajor) << "." << uint32_t(kip::versionMinor) << std::endl << std::endl;
  memory.resize(0x10000);
  kip::MapMemory(memory.data(), uint32_t(memory.size()), 0x0000);
  kip::SetStackPointer(uint32_t(memory.size() - 1));
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
  std::vector<std::string> lines;
  {
    kip::InterpretResult result = kip::LoadFile(filename, lines);
    if (!result.success)
      std::cerr << "ERR: " << result.str << std::endl;
  }
  size_t folderLen = 0;
  if (filename.find_last_of('/') != size_t(-1))
    folderLen = filename.find_last_of('/');
  if (filename.find_last_of('\\') != size_t(-1))
    folderLen = std::max(folderLen, filename.find_last_of('\\'));
  std::string folder = filename.substr(0, folderLen);
  std::vector<kip::InterpretResult> results = kip::InterpretLines(lines, folder);
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
