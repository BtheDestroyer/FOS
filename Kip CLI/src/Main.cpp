#include "kip.h"
#include "Interpreter.h"

int main(int argc, char* argv[])
{
  Interpreter I;
  if (argc == 1)
    while (I.Running())
      I.CLIInput();
  else
    I.RunFile(argv[1]);
  return 0;
}
