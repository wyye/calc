#!/bin/bash

for i in `seq 0 4`; do
	./calc ssa$i.in -c > ssa$i.out.test
	if diff ssa$i.out ssa$i.out.test > ast.log; then
		echo -n "$i passed "
	else
		echo -n "$i FAILED "
		rm ssa$i.out.test
		break
	fi
	rm ssa$i.out.test
done
echo ""
