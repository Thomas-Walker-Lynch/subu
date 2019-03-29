#!/bin/bash -x
./tranche-target test1.dat > test3.out
diff test3.out test3.out.expected
rm test3.out


