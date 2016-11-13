

namespace paiv {


void
UniversalVM::exec(const Image& image) {
  load(image);
  stopped = false;
  run();
}

void
UniversalVM::load(const Image& image) {
  Snapshot snapshot;
  snapshot.load_program(image);
  load(snapshot);
}

Snapshot
UniversalVM::save() {
  u8 wasStopped = stopped;
  stopped = true;
  usleep(100);

  Snapshot snapshot;
  snapshot.take(this);

  stopped = wasStopped;
  return snapshot;
}

void
UniversalVM::load(const Snapshot& snapshot) {
  stopped = true;
  snapshot.restore(this);
}

void
UniversalVM::run(zmq::socket_t* controller, zmq::socket_t* publisher) {
  zmq::message_t message;
  u8 breakpointNext = false;
  u8 breakpointRet = false;
  u32 controlTimer = 0;

  while (!halted)
  {
    if (controller && (controlTimer++ % 100 == 0) && controller->recv(&message, ZMQ_DONTWAIT))
    {
      DebuggerCommand command = *message.data<DebuggerCommand>();

      if (command.name == "step")
      {
        breakpointNext = true;
        stopped = false;
        step();
      }
      else if (command.name == "step-out")
      {
        breakpointRet = true;
        stopped = false;
        step();
      }
      else if (command.name == "stop")
      {
        stopped = true;
      }
      else if (command.name == "resume")
      {
        stopped = false;
        step();
      }
      else if (command.name == "info breakpoints")
      {
        string bps = accumulate(begin(executionBreakpoints), end(executionBreakpoints), string(),
          [](const string& a, u32 b) {
            stringstream so;
            so << a << ' ' << setfill('0') << setw(6) << hex << b;
            return so.str();
          });

        report(publisher, "breakpoints", bps);
      }
      else if (command.name == "set breakpoint")
      {
        executionBreakpoints.push_back(command.arg);
      }
      else if (command.name == "clear breakpoint")
      {
        auto it = find(begin(executionBreakpoints), end(executionBreakpoints), command.arg);
        if (it != end(executionBreakpoints))
          executionBreakpoints.erase(it);
      }
    }

    if (!stopped)
    {
      if (breakpointNext
        || (executionBreakpoints.size() > 0 && find(begin(executionBreakpoints), end(executionBreakpoints), state.ip) != end(executionBreakpoints)))
      {
        breakpointNext = false;
        breakpointRet = false;
        stopped = true;
        report(publisher, "stopped");
      }
      else
      {
        step();
      }
    }
    else
    {
      usleep(1000);
    }
  }
}

void
UniversalVM::step() {
  // if (trace)
  //   clog << state.ip << endl;

  auto raw = prog(state.ip);
  auto code = Operand::unpack_code(raw);

  if (!dispatch(code, raw))
    halted = true;
}

u8
UniversalVM::dispatch(Op code, u32 raw) {
  // if (trace)
  //   clog << op << endl;

  switch (code) {

    case Op::CMOVE:
      {
        auto op = Operand::unpackABC(raw);
        if (reg(op.c) != 0) {
          reg(op.a) = reg(op.b);
        }
      }
      break;

    case Op::READ:
      {
        auto op = Operand::unpackABC(raw);
        reg(op.a) = mem(reg(op.b))[reg(op.c)];
      }
      break;

    case Op::WRITE:
      {
        auto op = Operand::unpackABC(raw);
        mem(reg(op.a))[reg(op.b)] = reg(op.c);
      }
      break;

    case Op::ADD:
      {
        auto op = Operand::unpackABC(raw);
        reg(op.a) = reg(op.b) + reg(op.c);
      }
      break;

    case Op::MUL:
      {
        auto op = Operand::unpackABC(raw);
        reg(op.a) = reg(op.b) * reg(op.c);
      }
      break;

    case Op::DIV:
      {
        auto op = Operand::unpackABC(raw);
        reg(op.a) = reg(op.b) / reg(op.c);
      }
      break;

    case Op::NAND:
      {
        auto op = Operand::unpackABC(raw);
        reg(op.a) = ~(reg(op.b) & reg(op.c));
      }
      break;

    case Op::HALT:
      return false;

    case Op::ALLOC:
      {
        auto op = Operand::unpackBC(raw);
        reg(op.b) = memmgr.alloc(reg(op.c));
      }
      break;

    case Op::FREE:
      {
        auto op = Operand::unpackC(raw);
        memmgr.free(reg(op.c));
      }
      break;

    case Op::OUT:
      {
        auto op = Operand::unpackC(raw);
        printf("%c", reg(op.c));
      }
      break;

    case Op::IN:
      {
        auto op = Operand::unpackC(raw);
        s32 x = getchar();
        reg(op.c) = (x >= 0 && x <= 255) ? x : -1;
      }
      break;

    case Op::PROG:
      {
        auto op = Operand::unpackBC(raw);
        if (reg(op.b) != 0)
          mem(0) = mem(reg(op.b));
        state.stack_add(state.ip);
        state.ip = reg(op.c);
        return true;
      }
      break;

    case Op::LOAD:
      {
        auto op = Operand::unpackVal(raw);
        reg(op.x) = op.val;
      }
      break;

    default:
      cerr << "unhandled opcode " << code << " " << Operand::unpack(raw) << endl;
      __builtin_trap();
      break;
  }

  state.ip += 1;
  return true;
}

void
UniversalVM::report(zmq::socket_t* publisher, const string& name, const string& arg) const {
  if (publisher)
  {
    VmEvent event = { name, arg };
    zmq::message_t message((void*)&event, sizeof(event));
    publisher->send(message);
  }
}


}
