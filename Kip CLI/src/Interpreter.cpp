#include "Interpreter.h"
#include "kip.h"
#include <iostream>

Interpreter::Interpreter()
{
  std::cout << "Kip Command Line Interpreter |" << std::endl;
  std::cout << "-----------------------------/" << std::endl;
  std::cout << "(c) Bryce Dixon 2020 |" << std::endl;
  std::cout << "---------------------/" << std::endl;
  std::cout << "RAM: $000 - $800 |" << std::endl;
  std::cout << "-----------------/" << std::endl << std::endl;
  kip::MapMemory(memory.data(), memory.size(), 0x0000);
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
    std::cerr << r.str << std::endl;
}

void Interpreter::RunFile(std::string filename)
{

}
