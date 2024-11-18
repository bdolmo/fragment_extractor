CXX = g++
CC = gcc
CXXFLAGS = -Wall -g
CFLAGS = -Wall -g
HTSLIB_DIR = htslib

INCLUDES = -I$(HTSLIB_DIR)

LIBS = -L$(HTSLIB_DIR) -L/usr/local/lib -L/usr/lib/x86_64-linux-gnu -lhts -lz -lbz2 -llzma -lpthread
ifeq ($(STATIC),1)
    LIBS += -static -ldeflate
endif

# Only include main.cpp, BamReader.cpp, and BamRecord.cpp
CPP_SRCS = cfDNAcounter.cpp
CPP_OBJS = $(CPP_SRCS:.cpp=.o)
DEPS = $(CPP_OBJS:.o=.d)
TARGET = fragment_extractor

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(CPP_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

-include $(DEPS)

.PHONY: clean run

clean:
	rm -f $(CPP_OBJS) $(TARGET) $(DEPS)

run: $(TARGET)
	./$(TARGET)
