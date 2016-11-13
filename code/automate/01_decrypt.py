#!/usr/bin/env python
import pexpect
import re
import sys
import tty

UMapp = '../build/release/um'
SourceCodex = '../../spec/codex.umz'

DumpFile = '../../notes/codex.um'

p = pexpect.spawn(UMapp, args=[SourceCodex], echo=False, timeout=60)
p.logfile_read = sys.stdout
p.expect_exact('enter decryption key:')
p.sendline(r'(\b.bb)(\v.vv)06FHPVboundvarHRAk')
p.expect_exact('ok')
p.expect_exact('Choose a command:')
p.logfile_read = None

tty.setraw(p.child_fd)

p.sendline('p')
# p.expect_exact('follows colon:')

rx = re.compile(r'.*?follows colon:')

with open(DumpFile, 'wb') as fd:
  s = p.read_nonblocking(4000)
  s = rx.sub('', s)
  fd.write(s)

  while True:
    try:
      s = p.read_nonblocking(4000)
      fd.write(s)
    except pexpect.EOF:
      break
