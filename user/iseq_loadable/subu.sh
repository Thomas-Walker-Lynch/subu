#!/bin/bash
#

# did not have to do this for F37, seems the pactl was already there
# to make audio work will need to do this:
#    > sudo dnf install pulseaudio-utils
#    > pactl load-module module-native-protocol-tcp
# To load a specific module to the PA server, you add it to /etc/pulse/default.pa:
# I created the file because it was not there... 

#set -x

subu="$1"
shell="${@:2}"

export HOME=$(/usr/local/bin/home)
export PATH=/usr/bin

error=false
user=$(/usr/local/bin/user)
if [ ! $? ]; then
  echo "/usr/local/bin/user failed"
  error=true
fi
if [ -z "$subu" ]; then
  echo "no subuser name supplied"
  error=true
fi

machine="$(hostname -s)"
xkey=$(xauth list | grep "$machine" | head -1 | awk '{print $3}')
if [ -z "$xkey" ]; then
  echo "xauth key not found"
  error=true
fi
if $error; then
  exit 1
fi

if [ -z "$shell" ]; then
  shell="gnome-terminal --title="$subu""
fi
if [ "$shell" == "emacs" ]; then
  shell="emacs --title $subu" 
fi

# SUBU_SHARE_DIR has files optionally shared among subu, e.g. bashrc
read -r -d '' script0 <<-EOF
  export NO_AT_BRIDGE=1 \
  ;touch .Xauthority \
  ;xauth add "$DISPLAY" . "$xkey" \
  ;$shell
EOF

subu_username="$user-$subu"
sudo -E su \
     -l \
     -w SUBU_SHARE_DIR,DISPLAY,PULSE_SERVER \
     -c "$script0" \
     "$subu_username"




#just hangs
#sudo -u  "$subu_username" sh -c "$script0"

#set +x
