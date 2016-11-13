
namespace paiv {

  static const ImageSign SIGNv1 = { "UNIVM32" };


  void
  Snapshot::take(const UniversalVM* vm) {
    state = vm->state;
  }

  void
  Snapshot::restore(UniversalVM* vm) const {
    vm->setState(state);
  }

  u8
  Snapshot::save(const string& fn) const {
    ofstream ofs(fn, ios::binary | ios::trunc);
    if (!ofs.good())
      return false;

    u32 memsize = state.mem.size();

    ofs.write(SIGNv1.chars, sizeof(SIGNv1));
    ofs.write((char*)&state.reg[0], state.reg.size() * sizeof(state.reg[0]));
    ofs.write((char*)&state.ip, sizeof(state.ip));
    ofs.write((char*)&memsize, sizeof(memsize));

    for (auto& arr : state.mem) {
      u32 n = arr.size();
      ofs.write((char*)&n, sizeof(n));
      ofs.write((char*)&arr[0], arr.size() * sizeof(arr[0]));
    }

    return true;
  }

  u8
  Snapshot::load(const string& fn) {
    vm_state state;

    ifstream ifs(fn, ios::binary | ios::ate);
    if (!ifs.good())
      return false;

    auto size = ifs.tellg();
    ifs.seekg(0, ios::beg);

    size_t runlen = sizeof(ImageSign);
    if (size > runlen) {
      ImageSign sign;
      ifs.read(&sign.chars[0], sizeof(ImageSign));
      if (sign.word != SIGNv1.word) return false;
    }

    auto nbytes = state.reg.size() * sizeof(state.reg[0]);
    runlen += nbytes;
    if (size > runlen ) {
      ifs.read((char*)&state.reg[0], nbytes);
    }

    runlen += sizeof(state.ip);
    if (size > runlen) {
      ifs.read((char*)&state.ip, sizeof(state.ip));
    }

    u32 memUsed = 0;
    runlen += sizeof(memUsed);
    if (size > runlen) {
      ifs.read((char*)&memUsed, sizeof(memUsed));
    }

    if (memUsed == 0) return false;

    state.mem.resize(memUsed);

    u32 id = 0;
    for (auto& arr : state.mem) {
      u32 nsize = 0;
      runlen += sizeof(nsize);
      if (size > runlen) {
        ifs.read((char*)&nsize, sizeof(nsize));

        runlen += nsize * sizeof(arr[0]);
        if (size >= runlen) {
          arr.resize(nsize);
          ifs.read((char*)&arr[0], nsize * sizeof(arr[0]));

          continue;
        }
      }
      return false;
    }

    this->state = state;
    return true;
  }

  u8
  Snapshot::load_program(const Image& image) {
    state.mem.push_back(image);
    return true;
  }

}
