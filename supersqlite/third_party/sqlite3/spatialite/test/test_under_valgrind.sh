#! /bin/bash
for i in *; do
if test -x $i -a -f $i -a $i != "test_under_valgrind.sh"; then
    libtool --mode=execute valgrind --track-origins=yes  --tool=memcheck --num-callers=20  --leak-check=full --show-reachable=yes --suppressions=geos-init.supp $i
fi;
done;
