#!/usr/bin/python

import getpass
import os

username = getpass.getuser()
print "my username is: " + username

sudo_caller_username = os.environ.get('SUDO_USER')
if sudo_caller_username:
  print "the sudo caller's username is: " + sudo_caller_username
else:
  print "there is no sudo caller"

print "uid: " + str(os.getuid())
print "euid: " + str(os.geteuid())
print "resuid: " + str(os.getresuid())

null_lookup = os.environ.get('X3841232341')
if null_lookup:
  print "very surprising, found X3841232341 in the environment"
elif null_lookup == None:
  print "null lookup result: None"
else:
  print "null lookup result evaluates to false, but it is not None"

for k,v in os.environ.items():
  print (k, v)
  

