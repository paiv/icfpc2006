
namespace paiv {

  Operand
  Operand::unpack(u32 raw) {
    Operand op;
    op.code = Op(raw >> 28);
    op.a = (raw >> 6) & 0x7;
    op.b = (raw >> 3) & 0x7;
    op.c = raw & 0x7;
    op.val = raw & 0x01FFFFFF;
    op.x = (raw >> 25) & 0x7;
    return op;
  }

  Operand
  Operand::unpackABC(u32 raw) {
    Operand op;
    op.a = (raw >> 6) & 0x7;
    op.b = (raw >> 3) & 0x7;
    op.c = raw & 0x7;
    return op;
  }

  Operand
  Operand::unpackBC(u32 raw) {
    Operand op;
    op.b = (raw >> 3) & 0x7;
    op.c = raw & 0x7;
    return op;
  }

  Operand
  Operand::unpackC(u32 raw) {
    Operand op;
    op.c = raw & 0x7;
    return op;
  }

  Operand
  Operand::unpackVal(u32 raw) {
    Operand op;
    op.val = raw & 0x01FFFFFF;
    op.x = (raw >> 25) & 0x7;
    return op;
  }

  u32
  Operand::pack() const {
    return -1;
  }

}
