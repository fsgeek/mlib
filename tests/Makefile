CC=cc -std=c99
WCFLAGS=-Wall -W -pedantic -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -Wswitch-default -Wswitch-enum -Wcast-align -Wpointer-arith -Wbad-function-cast -Wstrict-overflow=5 -Wstrict-prototypes -Winline -Wundef -Wnested-externs -Wcast-qual -Wshadow -Wunreachable-code -Wlogical-op -Wstrict-aliasing=2 -Wredundant-decls -Wold-style-definition -Wno-unused-function -ftrapv
XCFLAGS=
CFLAGS=$(WCFLAGS) $(XCFLAGS)
CPPFLAGS=-I..
LDFLAGS=-pthread
RM=rm -f
LOG_COMPILER=

all:
	@echo "Nothing to be done."

.POSIX:
.PHONY: valgrind scan-build sanitize check clean *.log coverage synthesis
.SUFFIXES:
.SUFFIXES: .c .exe .log .depend .valgrind .gcov
.SECONDARY:

.c.exe:
	$(CC) $(CFLAGS) $(CPPFLAGS) -g $(LDFLAGS) $< -o $@

.c.depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM -MT `echo $< |sed -e 's/\.c/\.exe/g'` $< > $@

.exe.log:
	$(LOG_COMPILER) ./$<

.exe.valgrind:
	valgrind ./$<

.c.gcov:
	awk '/START_COVERAGE/ { printit=1} (printit == 0) { print }' $< > $<.first.c
	awk '  printit { print } /END_COVERAGE/ { printit=1}' $< > $<.end.c
	$(CC) $(CPPFLAGS) -E -DNDEBUG -DWITHIN_COVERAGE $< |grep -v '^#' |awk ' /END_COVERAGE/ { printit=0} printit { print } /START_COVERAGE/ { printit=1 }' |perl -p -e "s/;(?![\"'])/;\n/g ; s/}(?![\"'])/}\n/g ; s/{(?![\"'])/{\n/g" > $<.middle.c
	cat $<.first.c $<.middle.c $<.end.c > $<.c
	$(CC) $(CPPFLAGS) $(LDFLAGS) -ftest-coverage -fprofile-arcs -g $<.c -o $<.exe
	$(LOG_COMPILER) ./$<.exe
	gcov $<.gcno >  `basename $< .c`.synt
	cat `basename $< .c`.synt
	mv $<.c.gcov `basename $< .c`.gcov
	$(RM) $<.first.c $<.end.c $<.middle.c $<.c $<.exe

-include depend


#####################################################################################################
# Test suite
#####################################################################################################

depend:
	$(RM) depend
	echo "" > depend
	$(MAKE) `ls test-*.c|sed -e 's/\.c/\.depend/g'`
	cat *.depend > depend
	$(RM) *.depend

check: depend
	$(MAKE) `ls test-*.c|sed -e 's/\.c/\.log/g'`

clean:
	$(RM) *.exe *.s *~ *.o *.log *.valgrind ./a*.dat clang_output_* *.c.c *.end.c *.first.c *.middle.c
	$(RM) -rf coverage *.gcda *.gcno *.gcov all.info test.tar coverage-dir mlib-report *.synt

distclean: clean
	$(RM) *.depend depend

#####################################################################################################
# Code generation analysis
#####################################################################################################

gene-list:
	$(CC) $(CFLAGS) $(CPPFLAGS) -O3 -march=native -DNDEBUG -S tgen-mlist.c && less tgen-mlist.s

gene-array:
	$(CC) $(CFLAGS) $(CPPFLAGS) -O3 -march=native -DNDEBUG -S tgen-marray.c && less tgen-marray.s

gene-shared:
	$(CC) $(CFLAGS) $(CPPFLAGS) -O3 -march=native -DNDEBUG -S tgen-shared.c && less tgen-shared.s

gene-bitset:
	$(CC) $(CFLAGS) $(CPPFLAGS) -O3 -march=native -DNDEBUG -S tgen-bitset.c && less tgen-bitset.s

gene-dict:
	$(CC) $(CFLAGS) $(CPPFLAGS) -O3 -march=native -DNDEBUG -S tgen-mdict.c && less tgen-mdict.s

gene-queue:
	$(CC) $(CFLAGS) $(CPPFLAGS) -O3 -march=native -DNDEBUG -S tgen-queue.c && less tgen-queue.s

gene-string:
	$(CC) $(CFLAGS) $(CPPFLAGS) -O3 -march=native -DNDEBUG -S tgen-mstring.c && less tgen-mstring.s

#####################################################################################################
# Coverage
#####################################################################################################

coverage:
	$(MAKE) `ls test-*.c|sed -e 's/\.c/\.gcov/g'`
	$(MAKE) synthesis

SYNTHESIS_DATA=	M-ALGO test-malgo.c.c test-malgo.synt			\
		M-ARRAY test-marray.c.c test-marray.synt		\
		M-BITSET ../m-bitset.h test-mbitset.synt		\
		M-BBPTREE test-mbptree.c test-mbptree.synt		\
		M-BUFFER test-mbuffer.c.c test-mbuffer.synt		\
		M-CORE test-mcore.c.c test-mcore.synt			\
		M-DEQUE test-mdeque.c.c test-mdeque.synt		\
		M-DICT test-mdict.c.c test-mdict.synt			\
		M-GENINT ../m-genint.h test-mgenint.synt		\
		M-I-LIST test-milist.c.c test-milist.synt		\
		M-LIST test-mlist.c.c test-mlist.synt			\
		M-MEMPOOL test-mmempool.c.c test-mmempool.synt		\
		M-MUTEX ../m-mutex.h test-mmutex.synt			\
		M-PRIOQUEUE test-mprioqueue.c.c test-mprioqueue.synt	\
		M-RBTREE test-mrbtree.c.c test-mrbtree.synt		\
		M-SHARED test-mshared.c.c test-mshared.synt		\
		M-SNAPSHOT test-msnapshot.c.c test-msnapshot.synt	\
		M-STRING ../m-string.h test-mstring.synt 		\
		M-TUPLE test-mtuple.c.c test-mtuple.synt 		\
		M-VARIANT test-mvariant.c.c test-mvariant.synt 		\
		M-WORKER ../m-worker.h test-mworker.synt		

synthesis:
	@printf "+-------------------------------+-------------------------------+\n"
	@for i in $(SYNTHESIS_DATA) ; do 			\
	if test "$$j" = "" ; then j=$$i ; continue ; fi;	\
	if test "$$k" = "" ; then k=$$i ; continue ; fi;	\
	printf "|\t%16.16s\t|\t%16.16s\t|\n"	"$$j"	"`grep -v Creating $$i|grep $$k -F -A1 |tail -1|cut -f2 -d':'`"; 	\
	j="";							\
	k="";							\
	done
	@printf "+-------------------------------+-------------------------------+\n"

#####################################################################################################
# Code analyzer tools
#####################################################################################################

valgrind:
	$(MAKE) `ls test-*.c|sed -e 's/\.c/\.valgrind/g'`

scan-build: clean
	$(RM) -rf ./mlib-report
	scan-build -o ./mlib-report $(MAKE) check XCFLAGS="-std=c99"

sanitize:
	$(MAKE) clean check XCFLAGS="-fsanitize=address,undefined"
	$(MAKE) clean check XCFLAGS="-fsanitize=thread -pie -fPIE"


#####################################################################################################
# Test All
#####################################################################################################

WCFLAGS_CLANG=-Wall -W -pedantic -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -Wswitch-default -Wswitch-enum -Wcast-align -Wpointer-arith -Wbad-function-cast -Wstrict-overflow=5 -Wstrict-prototypes -Winline -Wundef -Wnested-externs -Wcast-qual -Wshadow -Wunreachable-code -Wstrict-aliasing=2 -Wredundant-decls -Wold-style-definition -Wno-unused-function -ftrapv
WCFLAGS_CPP=-Wall -W -pedantic -Wshadow -Wpointer-arith -Wcast-qual -Wswitch-default -Wswitch-enum -Wcast-align -Wpointer-arith -Wstrict-overflow=5 -Winline -Wundef -Wcast-qual -Wshadow -Wunreachable-code -Wlogical-op -Wstrict-aliasing=2 -Wredundant-decls -Wno-unused-function -ftrapv

checkall:
	@$(MAKE) clean ; $(MAKE) check CC="gcc -std=c99"
	@$(MAKE) clean ; $(MAKE) check CC="gcc -std=c99 -m32"
	@$(MAKE) clean ; $(MAKE) check CC="gcc -std=c11"
	@$(MAKE) clean ; $(MAKE) check CC="gcc -std=c11 -m32"
	@if which clang ; then $(MAKE) clean ; $(MAKE) check CC="clang -std=c99" WCFLAGS="$(WCFLAGS_CLANG)" ; else echo "SYSTEM CLANG not found: SKIP TEST" ; fi
	@if which g++ ; then $(MAKE) clean ; $(MAKE) check CC="g++ -std=c++11" WCFLAGS="$(WCFLAGS_CPP)" ; else echo "SYSTEM G++ not found: SKIP TEST" ; fi
	@if which clang++ ; then $(MAKE) clean ; $(MAKE) check CC="clang++ -std=c++11" WCFLAGS="$(WCFLAGS_CLANG)" ; else echo "SYSTEM CLANG++ not found: SKIP TEST" ; fi
	@if which tcc ; then $(MAKE) clean ; $(MAKE) check CC="tcc -std=c99" ; else echo "SYSTEM TCC not found: SKIP TEST" ; fi
	@if which musl-gcc ; then $(MAKE) clean ; $(MAKE) check CC="musl-gcc -std=c11" ; else echo "SYSTEM MUSL-GCC not found: SKIP TEST" ; fi
	@for f in /opt/gcc* ; do if test -x "$$f/bin/gcc" ; then $(MAKE) clean ; $(MAKE) check CC="$$f/bin/gcc -std=c99" ; fi ; done
	@for f in /opt/clang* ; do if test -x "$$f/bin/clang" ; then $(MAKE) clean ; $(MAKE) check CC="$$f/bin/clang -std=c99" WCFLAGS="$(WCFLAGS_CLANG)" ; fi ; done
	@for f in /opt/tcc* ; do if test -x "$$f/bin/tcc" ; then $(MAKE) clean ; $(MAKE) check CC="$$f/bin/tcc -std=c99" ; fi ; done
