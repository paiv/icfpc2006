#include "unity.cpp"

int main(int argc, char* argv[]) {

  if (argc < 2) {
    cerr << "usage: um <file>" << endl;
    return 1;
  }

  ImageLoader loader;
  auto program = loader.read(argv[1]);

  UniversalVM vm;
  vm.exec(program);

  return 0;
}
