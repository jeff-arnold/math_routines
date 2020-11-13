# -*- mode: makefile-gmake;  fill-column: 132 -*-
#  Time-stamp: "Modified on 13-November-2020 at 09:03:31 by jeff on Jeffs-MacBook-Pro.local"

## ****************************************************************
## * Copyright (C) 2020  J.M. Arnold  jearnold <at> cern <dot> ch *
## ****************************************************************

## *****************************************************************************
## This program is free software: you can redistribute it and/or modify it     *
## under the terms of the GNU Lesser General Public License as published by    *
## the Free Software Foundation, either version 2 of the License, or (at your  *
## option) any later version.                                                  *
##                                                                             *
## This program is distributed in the hope that it will be useful, but WITHOUT *
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public        *
## License for more details.                                                   *
##                                                                             *
## You should have received a copy of the GNU Lesser General Public License    *
## along with this program.  If not, see <https://www.gnu.org/licenses/>.      *
## *****************************************************************************

# OS=$(shell uname -s)

CC = gcc
CXX = g++

AVX ?= -mavx
FMA ?= -mfma
OPT ?= -O2

INCLUDES = -I /opt/local/include
LDFLAGS = -L /opt/local/lib
LDLIBS = -lm

MPFR_LIB = -lmpfr

TEST_EXES = test_expxsqr_accuracy test_libm_expxsqr_accuracy test_expmxsqr_accuracy test_libm_expmxsqr_accuracy
TEST_OBJS = $(patsubst %, %.o, $(TEST_EXES))
FUNC_NAMES = expxsqr expmxsqr
FUNC_OBJS = $(patsubst %, %.o, $(FUNC_NAMES)) $(patsubst %, libm_%.o, $(FUNC_NAMES)) $(patsubst %, mpfr_%.o, $(FUNC_NAMES)) $(patsubst %, mpfr_libm_%.o, $(FUNC_NAMES))
FUNC_MISC = $(patsubst %, %.i, $(FUNC_NAMES)) $(patsubst %, %.s, $(FUNC_NAMES))
MISC_EXES = make_bins
MISC_OBJS = make_bins.o utils.o

.PHONY : all accuracy_tests libm_accuracy_tests

all: accuracy_tests libm_accuracy_tests

accuracy_tests: test_expxsqr_accuracy test_expmxsqr_accuracy

libm_accuracy_tests: test_libm_expxsqr_accuracy test_libm_expmxsqr_accuracy

test_expxsqr_accuracy : test_expxsqr_accuracy.o expxsqr.o mpfr_expxsqr.o utils.o 
	$(CC) $(OPT) $(OUTPUT_OPTION) $(LDFLAGS) $^ $(MPFR_LIB) $(LDLIBS)

test_expmxsqr_accuracy : test_expmxsqr_accuracy.o expmxsqr.o mpfr_expmxsqr.o utils.o 
	$(CC) $(OPT) $(OUTPUT_OPTION) $(LDFLAGS) $^ $(MPFR_LIB) $(LDLIBS)

test_libm_expxsqr_accuracy : test_libm_expxsqr_accuracy.o libm_expxsqr.o mpfr_expxsqr.o utils.o 
	$(CC) $(OPT) $(OUTPUT_OPTION) $(LDFLAGS) $^ $(MPFR_LIB) $(LDLIBS)

test_libm_expmxsqr_accuracy : test_libm_expmxsqr_accuracy.o libm_expmxsqr.o mpfr_expmxsqr.o utils.o 
	$(CC) $(OPT) $(OUTPUT_OPTION) $(LDFLAGS) $^ $(MPFR_LIB) $(LDLIBS)

test_expxsqr_accuracy.o : test_accuracy.c test_accuracy.h DD_arithmetic.h utils.h
	$(CC) -c -std=c17 -pedantic -Wall -DFUNC_NAME=expxsqr $(CFLAGS) $(OPT) $(AVX) $(FMA) $(INCLUDES) $(OUTPUT_OPTION) $<

test_expmxsqr_accuracy.o : test_accuracy.c test_accuracy.h DD_arithmetic.h utils.h
	$(CC) -c -std=c17 -pedantic -Wall -DFUNC_NAME=expmxsqr $(CFLAGS) $(OPT) $(AVX) $(FMA) $(INCLUDES) $(OUTPUT_OPTION) $<

test_libm_expxsqr_accuracy.o : test_accuracy.c test_accuracy.h DD_arithmetic.h utils.h
	$(CC) -c -std=c17 -pedantic -Wall -DFUNC_NAME=libm_expxsqr -DMPFR_FUNC_NAME=mpfr_expxsqr $(CFLAGS) $(OPT) $(AVX) $(FMA) $(INCLUDES) $(OUTPUT_OPTION) $<

test_libm_expmxsqr_accuracy.o : test_accuracy.c test_accuracy.h DD_arithmetic.h utils.h
	$(CC) -c -std=c17 -pedantic -Wall -DFUNC_NAME=libm_expmxsqr -DMPFR_FUNC_NAME=mpfr_expmxsqr $(CFLAGS) $(OPT) $(AVX) $(FMA) $(INCLUDES) $(OUTPUT_OPTION) $<

expxsqr.o expmxsqr.o : %.o : %.c DD_arithmetic.h
	$(CC) -c -std=c17 -pedantic -Wall $(CFLAGS) $(OPT) $(AVX) $(FMA) $(OUTPUT_OPTION) $<

expxsqr.i expmxsqr.i : %.i : %.c DD_arithmetic.h
	$(CC) -c -std=c17 -pedantic -Wall $(CFLAGS) $(OPT) $(AVX) $(FMA) -E $< > $@

expxsqr.s expmxsqr.s : %.s : %.c DD_arithmetic.h
	$(CC) -c -std=c17 -pedantic -Wall $(CFLAGS) $(OPT) $(AVX) $(FMA) -S -fverbose-asm -Wa,-adhln $< > $@

ifdef USE_SPLIT
override USE_SPLIT = -DUSE_SPLIT
endif

libm_expxsqr.o libm_expmxsqr.o : %.o : %.c
	$(CC) -c -std=c17 -pedantic -Wall $(CFLAGS) $(USE_SPLIT) $(OPT) $(AVX) $(FMA) $(OUTPUT_OPTION) $<

mpfr_expxsqr.o mpfr_expmxsqr.o : %.o : %.c DD_arithmetic.h
	$(CC) -c -std=c17 -pedantic -Wall $(CFLAGS) $(OPT) $(AVX) $(FMA) $(INCLUDES) $(OUTPUT_OPTION) $<

utils.o : utils.c DD_arithmetic.h
	$(CC) -c -std=c17 -pedantic -Wall $(OPT) $(AVX) $(FMA) $(INCLUDES) $(OUTPUT_OPTION) $<

make_bins : make_bins.o
	$(CC) $(OPT) $(OUTPUT_OPTION) $(LDFLAGS) $^ $(MPFR_LIB) $(LDLIBS)

make_bins.o : make_bins.c
	$(CC) -c -std=c17 -pedantic -Wall $(CFLAGS) $(OPT) $(AVX) $(FMA) $(INCLUDES) $(OUTPUT_OPTION) $<

.PHONY : clean realclean
clean :
	rm -rf $(TEST_OBJS) $(FUNC_OBJS) $(FUNC_MISC) $(MISC_OBJS)

realclean : clean
	rm -rf $(TEST_EXES) $(MISC_OBJS) $(MISC_EXES)
