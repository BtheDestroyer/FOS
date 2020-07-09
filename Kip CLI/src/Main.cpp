#include "kip.h"
#include "Interpreter.h"
#include "Compiler.h"

int main(int argc, char* argv[])
{
  Interpreter I;
  if (argc == 1)
    while (I.Running())
      I.CLIInput();
  else if (argc == 2)
    I.RunFile(argv[1]);
  else if (argc == 3 && !std::strcmp(argv[2], "compile"))
    Compile(argv[1]);
  return 0;
}
