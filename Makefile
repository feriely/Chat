CXX=g++
CXXFLAGS=-Wall -g
RM=rm -f

SRCS=$(wildcard *.cc)
OBJS=$(SRCS:.cc=.o)

all: chatsvr

chatsvr: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $<

clean:
	$(RM) chatsvr $(OBJS)
