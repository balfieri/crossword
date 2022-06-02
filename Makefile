# This Makefile should work on macOS, cigwin (Win10), and Linux. If not, let me know.
#
LIBS=-lz -lpthread 
FLAGS=-std=c++17 -O3 -Werror -Wextra -Wstrict-aliasing -pedantic -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-include-dirs  -Woverloaded-virtual -Wredundant-decls -Wsign-promo -Wstrict-overflow=5 -Wswitch-default -Wundef -g $(LIBS)

OS=$(shell uname)
ifeq ($(OS), Darwin)
CC=clang
GPP=g++
else
ifeq ($(OS), Linux)
FLAGS+=-Wno-empty-body -Wno-strict-overflow -Wno-switch-default -Wno-shift-negative-value -Wno-maybe-uninitialized -Wno-sign-promo -Wno-overloaded-virtual -DNO_FMT_LL 
GPP=export PATH=/home/utils/gcc-8.1.0/bin:/usr/local/bin:/bin:/usr/bin:/sbin:/usr/sbin:/usr/lib:/etc:/home/nv/bin:/home/utils/p4-2018.1/bin:/usr/local/lsf/bin; g++
else
ifeq ($(findstring CYGWIN,$(OS)),CYGWIN)
FLAGS+=-Wno-missing-include-dirs -Wno-empty-body -Wno-strict-overflow -Wno-switch-default -Wno-shift-negative-value -Wno-maybe-uninitialized -Wno-sign-promo -Wno-overloaded-virtual -Wno-unused-result -DNO_FMT_LL -U__STRICT_ANSI__
GPP=g++
else
GPP=UNKNOWN_OPERATING_SYSTEM
endif
endif
endif

DEPS=Makefile *.h

gen_puz: gen_puz.cpp ${DEPS}
	$(GPP) $(FLAGS) $(EXTRA_CFLAGS) -o gen_puz gen_puz.cpp $(LIBS)

clean:
	rm -fr gen_puz *.o *.dSYM *.out
