#include "unity.cpp"
#include "disassembler.cpp"


int main(int argc, char* argv[]) {

  if (argc < 2) {
    cerr << "usage: dum <file.um>" << endl;
    return 1;
  }

  ImageLoader loader;
  auto program = loader.read(argv[1]);

  Disassembler dum;
  dum.disassemble(cout, program);

  return 0;
}
