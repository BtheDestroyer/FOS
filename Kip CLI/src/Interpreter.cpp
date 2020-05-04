#include <iostream>
#include <fstream>

#include "Interpreter.h"
#include "kip.h"

Interpreter::Interpreter()
{
  std::cout << "Kip Command Line Interpreter |" << std::endl;
  std::cout << "-----------------------------/" << std::endl;
  std::cout << "(c) Bryce Dixon 2020 |" << std::endl;
  std::cout << "---------------------/" << std::endl;
  std::cout << "RAM: $000 - $800 |" << std::endl;
  std::cout << "-----------------/" << std::endl;
  std::cout << "Kip V" << __KIP_VERSION_MAJOR << "." << __KIP_VERSION_MINOR << std::endl << std::endl;
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
    unsigned i = 0;
    unsigned iw = 0;
    for (size_t c = text.size(); c > 0; c /= 10)
      ++iw;
    for (std::string line : text)
    {
      if (line.empty())
      {
        std::string lp = "";
        unsigned pad = iw;
        for (unsigned c = ++i; c > 0; c /= 10)
          --pad;
        lp.resize(pad, ' ');
        std::cout << lp << std::to_string(i) << ": " << std::endl;
        continue;
      }
      kip::InterpretResult r = kip::InterpretLine(line);
      if (!r.success)
      {
        std::cerr << "ERR: " << r.str << std::endl;
        break;
      }
      else
      {
        std::string lp = "";
        unsigned pad = iw;
        for (unsigned c = ++i; c > 0; c /= 10)
          --pad;
        lp.resize(pad, ' ');
        std::cout << lp << std::to_string(i) << ": " << r.str << std::endl;
      }
    }
  }
  else
    std::cerr << "ERR: Couldn't read file " << filename << std::endl;
}
