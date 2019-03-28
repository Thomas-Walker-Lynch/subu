#!/usr/bin/python
# see the help option for syntax
# this script must be run from root or sudo
#
# on Fedora 29 os.getresuid returned all zeros for a script run from sudo.
# Hence, I am using the environment variable SUDO_USER

import getpass
import os
import sys
import libuser
from __future__ import print_function

command = os.path.base(argv[0])

#--------------------------------------------------------------------------------
# utilities
#
def prn(str):
  print(str,end='')

#--------------------------------------------------------------------------------
# help
#
def help():
  print( command +
""" [=help] [=version] [shell=<shell>][owner=<owner-username>] [subu=]<subu-username>
Makes a subservient user.
If no arguments are given, or if =help is given, this message is printed.
When this command is invoked through sudo, $SUDO_USER is taken as the owner's username.
Otherwise, when invoked directly from root, the owner= option must be provided.
The subu-username argument is the username for the new subservient user
The the new subu home directory is created in /home/owner/subu/.
Facls are set to give the owner access to the new subu's home directory.
The shell option is not implemented yet.  Probably need a number of other options also.
"""
  )

def version():
  print(" version 0")

#--------------------------------------------------------------------------------
# a manager for handling error messages
#
class class_err:
"""
An error record has the form [flag, message, args] 
  class is fatal, warning, info  [currently not implemented]
  flag is true if an error has occured [need to change this to a count]
  args is an array of strings to be given after the error message is printed.

The dict holds named error records.

register() is used to name and place error records in the dict. register() is
typically called multiple times to initialize and error instance.

tattle() is used by the program at run time in order to signal errors.

has_error() returns true if tattle was ever called

report() prints an error report.  When errors have occured this 

vector() [unimplemented] returns a bit vector with one bit per fatal error
record, in the order they appear in the dictionary. The bit is set if the error
ever occured.

We check for as many errors as is convenient to do so rather than stopping on
the first error.
"""

  # field offsets into the error record
  flag_dex = 0;
  message_dex = 1;
  args_dex = 2;

  def __init__(self):
    self.total_cnt = 0
    self.dict = {}

  def register(name, message):
    self.dict[name] = [False, message, []]

  def tattle(name, *args):
    self.total_cnt += 1
    if name in self.dict:
      self.dict[name][0] = True
      self.dict[name][2].extend(args)

  def report():
    if self.total_cnt:
      for k,v in self.dict.items():
        if v[self.flag_dex]:
          print(v[self.message_dex],end='')
          args = v[self.args_dex]
          if length(args) :
            print(args[0],end='')
            for arg in args[1:]:
              print( " " + arg, end='')
          print()

#--------------------------------------------------------------------------------
# parse the command line
#
err.register(
  'impossible_split',
  "It is not possible, yet this token split into other than one or two pieces: "
  )
err.register(
  'lone_delim',
  "No spaces should appear around the '=' delimiter."
  )

args = sys.argv[1:]
if len(args) == 0 :
  version()
  help()
  exit(1)
 
#create a dictionary based on the command line arguments
arg_dict = {}
subu_cnt = 0
delim = '='
for token in args:
  token_pair = split(token, delim);
  if len(token_pair) == 1 : #means there was no '=' in the token
    arg_dict['subu'] = token_pair
    subu_cnt++
  elif len(token_pair) == 2 :
    if token_pair[0] == '' and token_pair[1] == '' :
      err.tattle('lone_delim')
    elif token_pair[1] == '' : # then has trailing delim, will treat the same as a leading delim
      arg_dict[token_pair[0]] = None
    elif token_pair[0] == '' : # then has leading delim
      arg_dict[token_pair[1]] = None
    else:
      arg_dict[token_pair[0]] = token_pair[1]
  else:
    err.tattle('impossible_split', token)

if not arg_dict or arg_dict.get('help'):
  help()
  err.report()
  exit(1)

if arg_dict.get('version'):
  version()

#--------------------------------------------------------------------------------
# check that the command line arguments are well formed. 
#
err.register(
  'too_many_args',
  command + " takes at most one non-option argument, but we counted: "
  )
err.register(
  'no_subu'
  command + " missing subservient username."
  )
err.register(
  'bad_username'
  "Usernames match ^[a-z_]([a-z0-9_-]{0,31}|[a-z0-9_-]{0,30}\$)$, but found: "
  )
err.register(
  'unknown_option'
  command + " doesn't implement option: "
  )

subu = arg_dict.get('subu')
if subu_cnt > 1:
  err.tattle('too_many_args')
elif not subu
  err.tattle('no_subu')
elif not re.match("^[a-z_]([a-z0-9_-]{0,31}|[a-z0-9_-]{0,30}\$)$", subu)
  err.tattle('bad_username', subu)

for k in arg_dict:
  if k not in ['help', 'version', 'shell', 'owner', 'subu'] :
    err.tattle('unkown_option', k)

if arg_dict.get('shell') :
  print "shell option aint implemented yet"



#--------------------------------------------------------------------------------
# check that we have root privilege
#
err.register(
  'not_root'
  command + "requires root privilege"
  )

uid = os.getuid()
if uid != 0 :
  err.tattle('not root')

username = getpass.getuser()
sudo_caller_username = os.environ.get('SUDO_USER')

if !sudo_caller_username
  if username not == "root":
    err.tattle('not_root')
  elif:
    owner
    

  def has_error(*errs):
    return self.cnt > 0



#-----




#--------------------------------------------------------------------------------
#  pull out the owner_dir and subu_dir
#
admin= libuser.admin()

err_arg_form = class_err()
err_arg_form.register('too_many', "too many semicolon delineated parts in")

owner_parts = args[0].split(":")
subu_parts = args[1].split(":") 

owner_user_name = owner_parts[0]
if not owner_user_name:
  err_arg_form.tattle('owner_user_name_missing', args[0])
else:
  owner_user = admin.lookupUserByName(owner_user_name)
  if owner_user == None:
    err_arg_form.tattle('no_such_user_name', owner_user_name)
  else:
    

subu_user_name = subu_parts[0]


if length(owner_parts) > 2:
  err_arg_form.tattle('too_many', args[0])
elif length(owner_parts) == 2:
  owner_dir = owner_parts[1]
else # get the home directory
    
  

 

#--------------------------------------------------------------------------------
# set the home directory
#
if len(args) > args_dir_index:
  dir = args[args_dir_index]
else:
  dir = os.getcwd()

home = dir + "/" + name
home_flag = not os.path.exists(home)

#--------------------------------------------------------------------------------
# create the user, setfacls
#
err_cnt = 0
name_available_flag = False

if name_flag:
  admin = libuser.admin()
  name_available_flag = name not in admin.enumeratedUsers()

if owner_flag and name_flag and name_available_flag and home_flag :
  user = admin.initUser(name)
  user[libuser.HOMEDIRECTORY] = home
  if opt_shell : user[libuser.SHELL] = opt_shell
  admin.addUser(user)
  #setfacl -m d:u:plato:rwx,u:plato:rwx directory
  setfacl = "setfacl -m d:u:" + name + ":rwx,u:" + name + ":rwx " + home
  exit(0)

#--------------------------------------------------------------------------------
# error return
#  .. need to grab the return code from setfacl above and delete the user if it fails
#
err_flags = 0
if not owner_flag :
  err_flags |= 2**2
  print "missing owning username argument"
if not name_flag :
  err_flags |= 2**3
  print name + "missing subservient username argument"
if not name_available_flag :
  err_flags |= 2**4
  print name + "specified subservient username already exists"
if not home_flag :
  err_flags |= 2**5
  print "user home directory already exists"

exit(err_flags)
