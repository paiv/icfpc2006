
namespace paiv {


  u32
  MemoryManager::alloc(u32 size) {
    #if 1
    for (auto it = begin(dormant); it != end(dormant); it++) {
      auto ratio = r32(it->second) / size;
      if (ratio >= 0.1 && ratio <= 10) {
        auto id = it->first;
        dormant.erase(it);
        state->mem[id].resize(size);
        return id;
      }
    }
    #endif

    #if 0
    auto state = this->state;
    auto it = find_if(begin(dormant), end(dormant), [&state, size](const pair<u32,u32>& p){
      auto ratio = r32(get<1>(p)) / size;
      return ratio >= 0.1 && ratio <= 10;
    });

    if (it != end(dormant)) {
      auto id = it->first;
      dormant.erase(it);
      state->mem[id].resize(size);
      return id;
    }
    #endif

    state->mem.push_back(Image(size));
    auto id = state->mem.size() - 1;
    return id;
  }

  void
  MemoryManager::free(u32 id) {
    state->mem[id].clear();
    dormant.insert(make_pair(id, state->mem[id].capacity()));
  }

}
