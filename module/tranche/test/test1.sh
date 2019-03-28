#!/bin/bash -x
./tranche test1.dat >test1stdout.dat
diff test11.dat test11.dat.expected
diff test12.dat test12.dat.expected
diff test13.dat test13.dat.expected
diff test14.dat test14.dat.expected
diff test15.dat test15.dat.expected
diff test1stdout.dat test1stdout.dat.expected
rm test11.dat test12.dat test13.dat test14.dat test15.dat test1stdout.dat

