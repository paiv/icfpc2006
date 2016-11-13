
namespace std {
  template<>
  class hash<pair<u32,u32>> {
  public:
    size_t operator () (const pair<u32,u32>& p) const {
      hash<u32> h;
      size_t seed = 0;
      seed ^= h(p.first) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      seed ^= h(p.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      return seed;
    }
  };
}


namespace paiv {

  typedef vector<u32> Image;


  class ImageLoader {
  public:
    Image read(const string& fileName) const;
  };


  typedef struct vm_state {
    array<u32, 8> reg;
    vector<vector<u32>> mem;
    u32 ip;
    vector<u32> stack;

    vm_state() : ip(0) {
      reg.fill(0);
      stack.reserve(1024);
    }

    void stack_add(u32 x) {
      if (stack.size() + 4 >= stack.capacity())
        stack.erase(begin(stack), begin(stack) + stack.capacity() / 2);
      stack.push_back(x);
    }

  } vm_state;


  class MemoryManager {
  private:
    vm_state* state;
    unordered_set<pair<u32,u32>> dormant;
  public:
    MemoryManager(vm_state* state) : state(state) {}

    u32 alloc(u32 size);
    void free(u32 id);
  };


  typedef union {
    char chars[8];
    u64 word;
  } ImageSign;


  class UniversalVM;

  class Snapshot {
  public:
    void take(const UniversalVM* vm);
    void restore(UniversalVM* vm) const;

    u8 save(const string& fn) const;
    u8 load(const string& fn);
    u8 load_program(const Image& image);

    string fn;
    vm_state state;
  };


  typedef struct Operand {
    Op code;
    u8 a;
    u8 b;
    u8 c;
    u32 val;
    u8 x;

    static Operand unpack(u32 raw);
    u32 pack() const;

    static Op unpack_code(u32 raw) { return Op(raw >> 28); }
    static Operand unpackABC(u32 raw);
    static Operand unpackBC(u32 raw);
    static Operand unpackC(u32 raw);
    static Operand unpackVal(u32 raw);

    static const char*
    opname(Op op) {
      switch (op) {
        case CMOVE: return "cmov";
        case READ: return "read";
        case WRITE: return "write";
        case ADD: return "add";
        case MUL: return "mul";
        case DIV: return "div";
        case NAND: return "nand";
        case HALT: return "halt";
        case ALLOC: return "alloc";
        case FREE: return "free";
        case OUT: return "out";
        case IN: return "in";
        case PROG: return "prog";
        case LOAD: return "load";
      }
      return "(?)";
    }

    friend ostream& operator << (ostream& so, const Operand& op) {
      if (op.code == Op::LOAD)
        so << "{" << opname(op.code) << " a:" << int(op.x) << ", val:" << op.val << "}";
      else
        so << "{" << opname(op.code) << " a:" << int(op.a) << ", b:" << int(op.b) << ", c:" << int(op.c) << "}";
      return so;
    }

  } Operand;


  typedef struct {
    string name;
    string arg;
  } VmEvent;

  typedef struct {
    string name;
    u32 arg;
  } DebuggerCommand;


  class UniversalVM : public enable_shared_from_this<UniversalVM> {
  private:
    vm_state state;
    MemoryManager memmgr;

  protected:
    void setState(const vm_state& newState) {
      state = newState;
      memmgr = MemoryManager(&state);
    }

    friend class Snapshot;
    friend class Debugger;

  public:
    UniversalVM() : id(0), halted(false), stopped(false), trace(false), memmgr(&state) {
      static u8 vmid_sequence = 0;
      id = vmid_sequence++;
      receiveEndpoint = string("inproc://debug") + to_string(id);
      reportEndpoint = string("inproc://vm") + to_string(id);
    }

    void exec(const Image& image);
    void run(zmq::socket_t* controller = nullptr, zmq::socket_t* publisher = nullptr);

    string messagingEndpoint() const { return receiveEndpoint; }
    string reportingEndpoint() const { return reportEndpoint; }

    void load(const Image& image);
    void step();
    void halt() { stopped = halted = true; }

    Snapshot save();
    void load(const Snapshot& snapshot);

  private:
    u8 dispatch(Op code, u32 raw);
    void report(zmq::socket_t* publisher, const string& name, const string& arg = "") const;

    u32 reg(u8 index) const { return state.reg[index]; }
    u32& reg(u8 index) { return state.reg[index]; }

    const Image& mem(u32 index) const { return state.mem[index]; }
    Image& mem(u32 index) { return state.mem[index]; }

    u32 prog(u32 index) const { return mem(0)[index]; }

  private:
    u8 id;
    string receiveEndpoint;
    string reportEndpoint;
    u8 halted;
    u8 stopped;
    u8 trace;
    vector<u32> executionBreakpoints;
  };

}
