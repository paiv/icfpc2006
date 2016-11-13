#include <algorithm>
#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
#include <zmq.hpp>
#include <unistd.h>


using namespace std;

#include "types_primitive.hpp"

using namespace paiv;

#include "opcodes.hpp"
#include "types.hpp"
#include "loader.cpp"
#include "memory_manager.cpp"
#include "snapshot.cpp"
#include "operation.cpp"
#include "vm.cpp"
