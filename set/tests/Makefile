.POSIX:

.SUFFIXES: .in .out .ok
	
TESTS = 1.ok 2.ok 3.ok 4.ok 5.ok 6.ok 7.ok 8.ok 9.ok 10.ok 11.ok 12.ok 13.ok 14.ok 15.ok 16.ok 17.ok

test: $(TESTS)

clean:
	rm -f $(TESTS)

.in.ok:
	../$(OUT) < $< | diff -bu `basename $< .in`.out -
	touch $@
