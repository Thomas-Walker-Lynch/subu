#!/bin/bash -x
./tranche test2.trc.c >test2stdout.dat
diff test2.c test2.c.expected
diff test2.h test2.h.expected
diff test2stdout.dat test2stdout.dat.expected
rm test2.c test2.h


