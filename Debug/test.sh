#!/bin/bash

for i in `seq 0 23`; do
	./calc $i.in > $i.out.test
	if diff $i.out $i.out.test > ast.log; then
		echo -n "$i passed "
	else
		echo -n "$i FAILED "
		rm $i.out.test
		break
	fi
	rm $i.out.test
done
echo ""
