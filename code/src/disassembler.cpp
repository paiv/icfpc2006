
namespace paiv {

  typedef struct DisasmOperation {
    Op code;
    u8 a;
    u8 b;
    u8 c;
    u32 val;
    u8 x;
    u32 offset;
  } DisasmOperation;

  class Disassembler {
  public:
    void disassemble(ostream& so, const Image& image, u32 rebase = 0, u32 selected = -1) const;
    vector<DisasmOperation> decode(const Image& image, u32 rebase = 0) const;
    DisasmOperation decode(u32 raw) const;

    void format(ostream& so, const DisasmOperation& op, u8 selected = false) const;

    string charof(u32 arg) const;
    string char_or_hex(u32 x) const;

  };


void
Disassembler::disassemble(ostream& so, const Image& image, u32 rebase, u32 selected) const {
  for (auto& op : decode(image, rebase))
    format(so, op, op.offset == selected);
}


vector<DisasmOperation>
Disassembler::decode(const Image& image, u32 rebase) const {
  vector<DisasmOperation> ops(image.size());
  u32 offset = 0;

  for (auto raw : image) {
    auto op = decode(raw);
    op.offset = offset + rebase;
    ops[offset++] = op;
  }

  return ops;
}

DisasmOperation
Disassembler::decode(u32 raw) const {
  auto op = Operand::unpack(raw);
  return { op.code, op.a, op.b, op.c, op.val, op.x };
}

inline string
Disassembler::charof(u32 arg) const
{
  switch (arg)
  {
    case '\0':  return "\\0";
    case '\a':  return "\\a";
    case '\b':  return "\\b";
    case '\f':  return "\\f";
    case '\v':  return "\\v";
    case '\t':  return "\\t";
    case '\n':  return "\\n";
    case '\r':  return "\\r";
    case '\'':  return "\\'";
    case '\"':  return "\\\"";
    case '\\':  return "\\\\";
    default:
      if (arg < ' ')
      {
        stringstream so;
        so << "\\x" << setfill('0') << setw(2) << hex << arg;
        return so.str();
      }
      return string(1, (char)arg);
  }
}

string
Disassembler::char_or_hex(u32 x) const
{
  stringstream so;

  if (x < 128)
    so << "'" << charof(x) << "'";
  else
    so << showbase << hex << x;

  return so.str();
}

void
Disassembler::format(ostream& so, const DisasmOperation& op, u8 selected) const {
  char charbuf[20];
  snprintf(charbuf, sizeof(charbuf), "%06x:", op.offset);
  so << charbuf;
  so << setw(3) << right << (selected ? '>' : ' ') << ' ';
  so << setw(4) << left << Operand::opname(op.code);

  if (op.code < Op::HALT) {
    so << " r" << u32(op.a) << ", r" << u32(op.b) << ", r" << u32(op.c);
  }
  else {
    switch (op.code) {

    case Op::ALLOC:
      so << " r" << u32(op.b) << ", " << u32(op.c);
      break;

    case Op::FREE:
    case Op::OUT:
    case Op::IN:
      so << " r" << u32(op.c);
      break;

    case Op::PROG:
      so << " r" << u32(op.b) << ", r" << u32(op.c);
      break;

    case Op::LOAD:
      so << " r" << u32(op.x) << ", " << char_or_hex(op.val);
      break;

    default:
      break;
    }
  }

  so << endl;
}


}
