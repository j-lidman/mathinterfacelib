include Makefile.specific

# Path declaration

# Features


# Tools
CXX=g++

INCLUDES=-Iinclude -I$(BOOST_PATH)/include -I$(LIBXML2_INCLUDE) -Ilibs
CFLAGS=$(INCLUDES) -g -O2 -Wall -fPIC --std=c++0x -DBOOST_SYSTEM_NO_DEPRECATED
MEXFLAGS=$(INCLUDES) -g
LDFLAGS=-ldl -lxml2 -L$(BOOST_PATH)/lib -lboost_filesystem -lboost_system -lboost_regex -lrt #-Wl,-R$(MATLAB_PATH)/bin/glnxa64 -L$(MATLAB_PATH)/bin/glnxa64 -lmx -lmat -leng

SRCFILES_C=src/Session.C src/matwlib.C src/Offline_Engine.C src/Octave_Engine.C
SRCFILES_MEX=src/matwlibFunc.cpp
TESTFILES_C=
TESTFILES_CPP=tests/cvx_test.cpp tests/sfo_test.cpp
#tests/test_lib_Eigen.C

################## Third-party support ##################
#Matlab support
ifneq ($(MATLAB_PATH),)
	MEX=$(MATLAB_PATH)/bin/glnxa64/mex
	CFLAGS += -DHAS_MATLAB -I$(MATLAB_PATH)/extern/include
	SRCFILES_C += src/MATLAB_Engine.C
	TESTFILES_C += tests/test_lib_Matlab.C
endif
# Eigen support
ifneq ($(EIGEN_PATH),)
	CFLAGS += -DHAS_EIGEN -I$(EIGEN_PATH)
	SRCFILES_C += src/Eigen_Engine.C
	TESTFILES_C += tests/test_lib_Eigen.C
endif
# MATIO support?
ifneq ($(MATIO_PATH),)
	LDFLAGS += $(MATIO_PATH)/lib/libmatio.a -lz
	CFLAGS += -DHAS_MATIO -I$(MATIO_PATH)/include
endif
# Octave support
ifneq ($(OCTAVE_PATH),)
	CFLAGS += -DHAS_OCTAVE -I$(OCTAVE_PATH)
   LDFLAGS += -loctave
	SRCFILES_C += src/Octave_Engine.C
	TESTFILES_C += tests/test_lib_Octave.C
endif

OBJ = $(subst .C,.o,$(subst src/,output/, $(SRCFILES_C)))
OBJ_TESTS = $(subst .C,.o,$(subst tests/,output/, $(TESTFILES)))
OBJ_MEX = $(subst .cpp,.mexa64,$(subst src/,output/, $(SRCFILES_MEX)))

################## Compilation ##################
all: libmatwlib.a libmatwlib.so $(OBJ_MEX)
clean:
	rm -f output/*.o output/*.mexa64 libmatwlib.a libmatwlib.so

# Common compilation
output/%.o: src/%.C
	$(CXX) $(CFLAGS) -c -o $@ $<
output/%.o: tests/%.C
	$(CXX) $(CFLAGS) -c -o $@ $<
output/%.o: tests/%.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<
output/%.mexa64: src/%.cpp
	$(MEX) $(MEXFLAGS) $(LDFLAGS) -output $@ $<

libmatwlib.so: $(OBJ)
	$(CXX) $(LDFLAGS) -shared -o $@ $^
libmatwlib.a: $(OBJ)
	ar rcs $@ $^
	ranlib $@ 

.PHONY: clean

# Applications
APPLICATION=executeOffline.C
apps: all $(APPLICATION)
	$(CXX) $(CFLAGS) -o executeOffline executeOffline.C -L. -lmatwlib $(LDFLAGS)

#Tests
testsfilesC: $(TESTFILES_C) $(OBJ_TESTS)
	$(CXX) $(CFLAGS) -o $(notdir $(subst .C,,$<)) $< -L. -lmatwlib $(LDFLAGS)
testsfilesCPP: $(TESTFILES_CPP) $(OBJ_TESTS)
	$(CXX) $(CFLAGS) -o $(notdir $(subst .cpp,,$<)) $< -L. -lmatwlib $(LDFLAGS)
tests: all testsfilesC testsfilesCPP

# DO NOT DELETE THIS LINE -- make depend needs it
