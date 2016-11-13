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

users = load_users(PwdFile)


def play_adventure(login, pwd):
  with pexpect.spawn(UMapp, args=[SourceCodex], echo=True, timeout=10) as p:
    p.logfile_read = sys.stdout
    p.expect_exact('login:')
    p.sendline(login)
    p.expect_exact('password:')
    p.sendline(pwd)
    p.expect_exact('logged in as')

    p.sendline('run adventure')
    p.expect_exact('You are in a room')

    # p.sendline('take pamphlet')
    # p.expect_exact('You are now carrying')

    # p.sendline('take manifesto')
    # p.expect_exact('You are now carrying')

    p.sendline('n')
    p.expect_exact('there is some trash.')

    p.sendline('take bolt')
    p.expect_exact('You are now carrying')

    p.sendline('take spring')
    p.expect_exact('You are now carrying')
    p.sendline('inc spring')
    p.expect_exact('has been destroyed.')

    p.sendline('take button')
    p.expect_exact('You are now carrying')

    p.sendline('take processor')
    p.expect_exact('You are now carrying')

    p.sendline('take pill')
    p.expect_exact('You are now carrying')
    p.sendline('inc pill')
    p.expect_exact('has been destroyed.')

    p.sendline('take radio')
    p.expect_exact('You are now carrying')

    p.sendline('take cache')
    p.expect_exact('You are now carrying')

    p.sendline('comb processor with cache')
    p.expect_exact('successfully combined')

    p.sendline('take blue transistor')
    p.expect_exact('You are now carrying')
    p.sendline('inc blue transistor')
    p.expect_exact('has been destroyed.')
    # p.sendline('comb radio with blue transistor')
    # p.expect_exact('successfully combined')

    p.sendline('take antenna')
    p.expect_exact('You are now carrying')
    p.sendline('comb radio with antenna')
    p.expect_exact('successfully combined')

    p.sendline('take screw')
    p.expect_exact('You are now carrying')

    p.sendline('take motherboard')
    p.expect_exact('You are now carrying')
    p.sendline('comb motherboard with screw')
    p.expect_exact('successfully combined')
    #
    p.sendline('take A-1920-IXB')
    p.expect_exact('You are now carrying')
    p.sendline('comb A-1920-IXB with processor')
    p.expect_exact('successfully combined')
    # p.sendline('comb A-1920-IXB with bolt')
    # p.expect_exact('successfully combined')

    p.sendline('take red transistor')
    p.expect_exact('You are now carrying')
    # p.sendline('comb radio with red transistor')
    # p.expect_exact('successfully combined')
    #
    # p.sendline('take keypad')
    # p.expect_exact('You are now carrying')
    # p.sendline('comb keypad with button')
    # p.expect_exact('successfully combined')

    # p.sendline('comb A-1920-IXB with red transistor')
    # p.expect_exact('successfully combined')

    p.logfile_read = None
    p.interact()


play_adventure('howie', users['howie'])
