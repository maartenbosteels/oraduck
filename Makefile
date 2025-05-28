# Compiler
CXX = g++

# Check if ORACLE_HOME is set
ifndef ORACLE_HOME
    $(error ORACLE_HOME is not set. Please set ORACLE_HOME to your Oracle Instant Client directory (e.g., /opt/oracle/instantclient_19_27/instantclient_19_27))
endif

# Compile flags
# Adding -I$(ORACLE_HOME)/rdbms/public as per current subtask requirement.
# For Instant Client, sdk/include usually contains oci.h.
CXXFLAGS = -I$(ORACLE_HOME)/rdbms/public -I$(ORACLE_HOME)/sdk/include -std=c++11 -g -Wall

# Linker flags
# Adding -L$(ORACLE_HOME)/lib as per current subtask requirement.
# For Instant Client .zip installations, libraries are usually directly in $ORACLE_HOME.
LDFLAGS = -L$(ORACLE_HOME) -L$(ORACLE_HOME)/lib -lclntsh

# Target executable
TARGET = oracle_connect

# Source file
SRCS = oracle_connect.cpp

# Default target
all: $(TARGET)

# Rule to build the target
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Clean target
clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
