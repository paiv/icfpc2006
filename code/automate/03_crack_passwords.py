#!/usr/bin/env python
from __future__ import print_function
import pexpect
import re
import sys

UMapp = '../build/release/um'
SourceCodex = '../../notes/codex.um'

UserFile = '../../notes/users.txt'
DictFile = '../../notes/dictionary.txt'
CrackedFile = '../../notes/cracked.txt'


def dump_matches(fn, rx, s):
  m = re.findall(rx, s)
  if m:
    with open(fn, 'wb') as fd:
      for s in m:
        print(s, file=fd)

pwdrx = re.compile(r'\)\s*=\s*"(\w+)"')
def grab_passwords(s):
  dump_matches(DictFile, pwdrx, s)

userx = re.compile(r'^(\w+)/', re.M)
def grab_users(s):
  dump_matches(UserFile, userx, s)


def load_list(fn):
  with open(fn, 'rb') as fd:
    s = fd.read()
  return list(set(s.split()))


def search_users():
  p = pexpect.spawn(UMapp, args=[SourceCodex], echo=True, timeout=60)
  p.logfile_read = sys.stdout

  p.expect_exact('login:')
  p.sendline('guest')

  p.sendline('cd code')
  p.sendline('cat hack.bas')
  p.expect_exact('CARRIER DROPPED')
  grab_passwords(p.before)

  p.expect_exact('You have new mail.')

  p.sendline('cd /home')
  p.expect_exact('You have new mail.')

  p.sendline('ls')
  p.expect_exact('You have new mail.')
  grab_users(p.before)

  p.sendline('logout')
  p.close()


# search_users()

users = load_list(UserFile)
passwords = load_list(DictFile)

def force_password(login, passwords, extended=False):
  pwdlist = ['%s%02d' % (s,x) for s in passwords for x in range(100)] if extended else passwords

  for pwd in pwdlist:
    with pexpect.spawn(UMapp, args=[SourceCodex], echo=True, timeout=10) as p:
      p.logfile_read = sys.stdout
      p.expect_exact('login:')
      p.sendline(login)
      p.expect_exact(['password:', 'logged in as'])

      if 'logged in as' in p.after:
        return ''

      p.sendline(pwd)
      try:
        p.expect_exact(['ACCESS DENIED', 'logged in as'], timeout=0.2)
        if 'logged in as' in p.after:
          return pwd
      except pexpect.TIMEOUT:
        pass

  return '?'


for usr in users:
  pwd = force_password(usr, passwords, extended=True)
  with open(CrackedFile, 'ab') as fd:
    print('%s:%s' % (usr,pwd), file=fd)
