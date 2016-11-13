
namespace paiv
{

  class Debugger
  {
  public:
    Debugger(zmq::context_t* context, UniversalVM* vm)
      : context(context), vm(vm)
    {
    }

    void disassemble(ostream& so, u32 size = 10);
    void disassemble(ostream& so, u32 bucket, u32 address, u32 size, u32 selected = 0);

    void showRegisters(ostream& so);
    void dumpMemory(ostream& so, u32 bucket, u32 address, u32 size = 16);
    void showStack(ostream& so, u32 size = 8);

    void step();
    void stepOut();
    void resume();
    void breakOn(u32 address);
    void listBreakpoints();
    void clearBreakpoint(u32 address);

    void writeMemory(u32 bucket, u32 address, u32 value);
    void setRegister(u8 r, u32 value);
    void setNext(u32 ip);

  private:
    zmq::context_t* context;
    UniversalVM* vm;

  private:
    void sendCommand(const string& name, u32 arg = 0) const;
  };


  void
  Debugger::disassemble(ostream& so, u32 size)
  {
    auto ip = vm->state.ip;
    disassemble(so, 0, max(u32(0), ip - 3), size, ip);
  }

  void
  Debugger::disassemble(ostream& so, u32 bucket, u32 address, u32 size, u32 selected)
  {
    auto snapshot = vm->save();

    if (bucket >= snapshot.state.mem.size()) {
      so << "bucket " << bucket << " is outside memory bounds" << endl;
      return;
    }

    auto& mem = snapshot.state.mem[bucket];

    if (address + size >= mem.size()) {
      so << "memory " << hex << address << " (" << dec << size << ") is outside bucket bounds" << endl;
      return;
    }

    auto chunk = Image(begin(mem) + address, begin(mem) + address + size);

    Disassembler disasm;
    disasm.disassemble(so, chunk, address, selected);
  }

  void
  Debugger::showRegisters(ostream& ss)
  {
    stringstream so;
    auto snapshot = vm->save();

    so << setfill(' ');

    so << setw(6) << right << "ip";
    for (size_t i = 0; i < snapshot.state.reg.size(); i++)
      so << setw(6) << right << 'r' << i;
    so << endl;

    so << setfill('0');

    so << hex << setw(6) << snapshot.state.ip << ' ';
    for (auto& r : snapshot.state.reg)
      so << hex << setw(6) << r << ' ';
    so << endl;

    ss << so.str();
  }

  void
  Debugger::dumpMemory(ostream& ss, u32 bucket, u32 address, u32 size)
  {
    stringstream so;
    auto snapshot = vm->save();

    if (bucket >= snapshot.state.mem.size()) {
      ss << "bucket " << bucket << " is outside memory bounds" << endl;
      return;
    }

    auto& mem = snapshot.state.mem[bucket];

    so << setfill('0') << right << hex;

    stringstream ascii;
    u32 n = 0;

    for (u32 p = address; n < size && p < mem.size(); n++, p++)
    {
      if (n % 8 == 0)
      {
        if (n > 1)
        {
          so << ' ' << ascii.str() << endl;
          ascii.str("");
        }
        so << setw(6) << p << ": ";
      }

      u32 x = mem[p];
      so << setw(8) << x << ' ';
      ascii << (x > 31 && x < 128 ? (char)x : '.');
    }

    if (n % 8)
      so << setfill(' ') << setw((8 - (n % 8)) * 5) << ' ';
    so << ' ' << ascii.str() << endl;

    ss << so.str();
  }

  void
  Debugger::showStack(ostream& ss, u32 size)
  {
    stringstream so;
    so << setfill('0');
    so << "call stack:" << endl;

    auto snapshot = vm->save();

    auto& stack = snapshot.state.stack;
    size = min(size_t(size), stack.size());

    for (auto it = begin(stack) + stack.size() - size; it != end(stack); it++) {
      so << setw(6) << hex << *it << ':' << endl;
    }

    ss << so.str();
  }

  void
  Debugger::sendCommand(const string& name, u32 arg) const
  {
    zmq::socket_t socket(*context, ZMQ_PAIR);
    socket.connect(vm->messagingEndpoint());

    DebuggerCommand command = { name, arg };
    zmq::message_t message((void*)&command, sizeof(command));
    socket.send(message);
  }

  void
  Debugger::step()
  {
    sendCommand("step");
  }

  void
  Debugger::stepOut()
  {
    sendCommand("step-out");
  }

  void
  Debugger::resume()
  {
    sendCommand("resume");
  }

  void
  Debugger::breakOn(u32 address)
  {
    sendCommand("set breakpoint", address);
  }

  void
  Debugger::listBreakpoints()
  {
    sendCommand("info breakpoints");
  }

  void
  Debugger::clearBreakpoint(u32 address)
  {
    sendCommand("clear breakpoint", address);
  }

  void
  Debugger::writeMemory(u32 bucket, u32 address, u32 value)
  {
    auto& mem = vm->state.mem;
    if (bucket < mem.size() && address < mem[bucket].size()) {
      mem[bucket][address] = value;
    }
  }

  void
  Debugger::setRegister(u8 r, u32 value)
  {
    vm->state.reg[r] = value;
  }

  void
  Debugger::setNext(u32 ip)
  {
    vm->state.ip = ip;
  }
}
