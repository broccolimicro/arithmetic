SRCDIR       =  arithmetic
CXXFLAGS	 =  -O2 -g -Wall -fmessage-length=0 -I../common
SOURCES	    :=  $(shell find $(SRCDIR) -name '*.cpp')
TESTS        := $(wildcard test/*.cpp)
OBJECTS	    :=  $(SOURCES:%.cpp=%.o)
TEST_OBJECTS := $(TESTS:.cpp=.o)
LDFLAGS		 =  
GTEST        := ../../googletest/googletest
GTEST_I      := -I$(GTEST)/include -I.
GTEST_L      := -L$(GTEST)/build/lib -L.
TARGET		 =  lib$(SRCDIR).a
TEST_TARGET  = test_$(SRCDIR)

all: lib

lib: $(TARGET)

test: lib $(TEST_TARGET)

check: test
	./$(TEST_TARGET)

$(TARGET): $(OBJECTS)
	ar rvs $(TARGET) $(OBJECTS)

%.o: $(SRCDIR)/%.cpp 
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c -o $@ $<

$(TEST_TARGET): $(TEST_OBJECTS) test/gtest_main.o
	$(CXX) $(CXXFLAGS) $(GTEST_L) $^ -pthread -larithmetic -lgtest -o $(TEST_TARGET)

test/%.o: test/%.cpp
	$(CXX) $(CXXFLAGS) $(GTEST_I) -MM -MF $(patsubst %.o,%.d,$@) -MT $@ -c $<
	$(CXX) $(CXXFLAGS) $(GTEST_I) $< -c -o $@

test/gtest_main.o: $(GTEST)/src/gtest_main.cc
	$(CXX) $(CXXFLAGS) $(GTEST_I) $< -c -o $@
	
clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f $(TEST_OBJECTS) $(TEST_TARGET)
