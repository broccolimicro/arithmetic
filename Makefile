NAME          = arithmetic
DEPEND        = common

SRCDIR        = $(NAME)
TESTDIR       = tests
GTEST        := ../../googletest
GTEST_I      := -I$(GTEST)/googletest/include -I.
GTEST_L      := -L$(GTEST)/build/lib -L.

CXXFLAGS	    = -O2 -g -Wall -fmessage-length=0 $(DEPEND:%=-I../%)
LDFLAGS		    =  

SOURCES	     := $(shell find $(SRCDIR) -name '*.cpp')
OBJECTS	     := $(SOURCES:%.cpp=%.o)
TARGET		    = lib$(NAME).a

TESTS        := $(shell find $(TESTDIR) -name '*.cpp')
TEST_OBJECTS := $(TESTS:%.cpp=%.o) $(TESTDIR)/gtest_main.o
TEST_TARGET   = test

all: lib

lib: $(TARGET)

tests: lib $(TEST_TARGET)

$(TARGET): $(OBJECTS)
	ar rvs $(TARGET) $(OBJECTS)

%.o: $(SRCDIR)/%.cpp 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c -o $@ $<

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) $(GTEST_L) $^ -pthread -l$(NAME) -lgtest -o $(TEST_TARGET)

$(TESTDIR)/%.o: $(TESTDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(GTEST_I) -MM -MF $(patsubst %.o,%.d,$@) -MT $@ -c $<
	$(CXX) $(CXXFLAGS) $(GTEST_I) $< -c -o $@

$(TESTDIR)/gtest_main.o: $(GTEST)/googletest/src/gtest_main.cc
	$(CXX) $(CXXFLAGS) $(GTEST_I) $< -c -o $@
	
clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f $(TEST_OBJECTS) $(TEST_TARGET)
