#!/usr/bin/env python
from __future__ import print_function
import pexpect
import re
import sys

UMapp = '../build/release/um'
SourceCodex = '../../notes/codex.um'

DumpFile = '../../notes/dump1.um'
PubsFile = '../../notes/pubs.txt'


def dump_matches(fn, rx, s):
  m = re.findall(rx, s)
  if m:
    with open(fn, 'ab') as fd:
      for s in m:
        print(s, file=fd)

pubrx = re.compile(r'([A-Z.]+=[0-9@]+\|[0-9a-f]+)')
def grab_pubs(s):
  dump_matches(PubsFile, pubrx, s)


p = pexpect.spawn(UMapp, args=[SourceCodex], echo=True, timeout=60)
p.logfile_read = sys.stdout

p.expect_exact('login:')
p.sendline('guest')

p.expect_exact('You have new mail.')
grab_pubs(p.before)

p.sendline('./a.out')
p.expect_exact('core dumped')

p.sendline('cat core')
p.expect(r'INTRO\..*')
grab_pubs(p.after)

p.expect_exact('You have new mail.')
grab_pubs(p.before)

p.sendline('mail')
p.expect_exact('Admin')

p.sendline('cd code')
p.sendline('/bin/umodem mycode done')
p.expect_exact('Terminate with')

p.sendline('V REM hi')
p.sendline('done')

p.expect(r'INTRO\..*')
grab_pubs(p.after)

p.sendline('/bin/qbasic mycode')
p.expect(r'INTRO\..*')
grab_pubs(p.after)


# p.logfile_read = None
# p.interact()
