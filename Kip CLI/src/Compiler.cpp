#include <iostream>
#include <fstream>

#include "Compiler.h"
#include "kip.h"

void Compile(const std::string& source)
{
  Compile(source, source.substr(0, source.find_last_of('.')) + ".ckp");
}

void Compile(const std::string& source, const std::string& output)
{
  std::ifstream iF(source);
  if (iF.is_open())
  {
    std::cout << "Compiling..." << std::endl;
    std::cout << "Source: " << source << std::endl;
    std::cout << "Output: " << output << std::endl;
    std::vector<std::string> lines;
    std::cout << "Loading source..." << std::endl;
    kip::InterpretResult result = kip::LoadFile(source, lines);
    if (!result.success)
      std::cerr << "ERR: " << result.str << std::endl;
    iF.close();
    std::ofstream oF(output);

    kip::Instruction::Context context;
    std::cout << "Building context..." << std::endl;
    std::vector<kip::InterpretResult> bcr = BuildContext(context, lines);
    if (bcr.back().success)
    {
      std::cout << "Generating bytecode..." << std::endl;
      std::vector<uint8_t> bytecode = kip::CompileInstructionsToBytecode(BuildInstructions(context, lines), context);
      std::cout << "Writing to file..." << std::endl;
      oF.write((char*)(bytecode.data()), bytecode.size());
      std::cout << "Done! Final size: " << bytecode.size() << std::endl;
    }
    else
      std::cerr << "ERR: " << bcr.back().str << std::endl;
    oF.close();
  }
}
