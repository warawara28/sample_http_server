PROGRAM = http_server
OBJS = main.cpp
CXX = g++
CXXFLAGS = -std=c++11
LDLIBS = -levent

$(PROGRAM):$(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(PROGRAM) $(LDLIBS)

