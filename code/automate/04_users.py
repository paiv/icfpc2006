#!/usr/bin/env python
from __future__ import print_function
import pexpect
import re
import sys

UMapp = '../build/release/um'
SourceCodex = '../../notes/codex.um'

PubsFile = '../../notes/pubs.txt'
PwdFile = '../../notes/passwd.txt'


def load_list(fn):
  with open(fn, 'rb') as fd:
    s = fd.read()
  return list(set(s.split()))

def load_users(fn):
  res = {}
  users = load_list(fn)
  for u in users:
    (login,pwd) = u.split(':')
    res[login] = pwd
  return res

def dump_matches(fn, rx, s):
  m = re.findall(rx, s)
  if m:
    with open(fn, 'ab') as fd:
      for s in m:
        print(s, file=fd)

pubrx = re.compile(r'([A-Z.]+=[0-9@]+\|[0-9a-f]+)')
def grab_pubs(s):
  dump_matches(PubsFile, pubrx, s)


users = load_users(PwdFile)


def user_ohmega(login, pwd):
  with pexpect.spawn(UMapp, args=[SourceCodex], echo=True, timeout=10) as p:
    p.logfile_read = sys.stdout
    p.expect_exact('login:')
    p.sendline(login)
    p.expect_exact('password:')
    p.sendline(pwd)
    p.expect_exact('logged in as')

    p.sendline('mail')
    p.expect_exact('to quit.')

    p.sendline('0')
    p.expect_exact('Press Enter')
    grab_pubs(p.before)


user_ohmega('ohmega', users['ohmega'])
