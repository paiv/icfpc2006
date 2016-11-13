
namespace paiv {

  Image
  ImageLoader::read(const string& fileName) const {
    ifstream ifs(fileName, ios::binary | ios::ate);

    if (!ifs) {
      cerr << "! could not open " << fileName << endl;
    }

    auto size = ifs.tellg();
    ifs.seekg(0, ios::beg);

    Image res(size / 4);
    ifs.read((char*)&res[0], size/4*4);

  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    transform(begin(res), end(res), begin(res), [](u32 x){ return ntohl(x); });
  #endif

    return res;
  }

}
