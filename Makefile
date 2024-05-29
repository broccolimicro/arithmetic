SRCDIR       =  arithmetic
CXXFLAGS	 =  -O2 -g -Wall -fmessage-length=0 -I../common
SOURCES	    :=  $(shell find $(SRCDIR) -name '*.cpp')
TESTS        := $(wildcard tests/*.cpp)
OBJECTS	    :=  $(SOURCES:%.cpp=%.o)
TEST_OBJECTS := $(TESTS:.cpp=.o)
LDFLAGS		 =  
GTEST        := ../../googletest
GTEST_I      := -I$(GTEST)/googletest/include -I.
GTEST_L      := -L$(GTEST)/build/lib -L.
TARGET		 =  lib$(SRCDIR).a
TEST_TARGET  = test

all: lib

lib: $(TARGET)

tests: lib $(TEST_TARGET)

$(TARGET): $(OBJECTS)
	ar rvs $(TARGET) $(OBJECTS)

%.o: $(SRCDIR)/%.cpp 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c -o $@ $<

$(TEST_TARGET): $(TEST_OBJECTS) tests/gtest_main.o
	$(CXX) $(CXXFLAGS) $(GTEST_L) $^ -pthread -larithmetic -lgtest -o $(TEST_TARGET)

tests/%.o: tests/%.cpp
	$(CXX) $(CXXFLAGS) $(GTEST_I) -MM -MF $(patsubst %.o,%.d,$@) -MT $@ -c $<
	$(CXX) $(CXXFLAGS) $(GTEST_I) $< -c -o $@

tests/gtest_main.o: $(GTEST)/googletest/src/gtest_main.cc
	$(CXX) $(CXXFLAGS) $(GTEST_I) $< -c -o $@
	
clean:
	rm -f $(OBJECTS) $(TARGET) tests/gtest_main.o
	rm -f $(TEST_OBJECTS) $(TEST_TARGET)
