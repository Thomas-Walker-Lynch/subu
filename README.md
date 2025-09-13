* Key terms

masu == master user
subu == sub user

* Description

The scripts here can be used to give a master user the ability to administer sub users. The master user can add sub-users, delete them, and adminnister the sub users files.

Sub users are containers built over the user system.

User processes all have pids, and contained permissions.

User's have virtual memory.

User files have owners, and ownership permissions, and also group permissions (sharing permissions) via unix and ACLs.

IP policy routine rules support network isolation by uid.

X windows has sharing facilities and security policies for the display.

Most I/O devices already give virtual presentations of themselves to users. I.e. give each user full functionality while keeping the users separate.  Those which do not have always been a pain when writing containers, no exception here.


* Key to this approach are the tools:

machinectl
bindfs
wg and iproute2

* Experience

I've been using this approach for some years, though much of it is ad hoc. Occasionally I wrote scripts but many were then deprecated. Slowly the full picture is emerging in in this git repo.

It would be great to get together with another person who would like to experiment with this. Having two or more users would hasten the maturity of the scripts.

* Also included, scripts for putting the master users and their subus on a remotely mounted encrypted drive. This adds the conveience of being able to walk up to linux boxes and provide one's user data, or to remotely mount a user on a box.

To do:

Initializing, adding, and deleting subu.  Adding and removing subu can become so common that there is a data management problem. At one point I had a python module with a constant 'table' of data. I plan to use sqlite for managing this problem and to introduce a new set of scripts. I will probably jump to C to support suid root and begin the integration of subu into Linux.

Static local mounts of masu have not been developed, because I do not use them personally. It would require a simplification of the current remote mount scripts.

