BIN_DIR		= ../bin
CXXFLAGS 	+= -Wall -g -MMD -std=c++1y
LDFLAGS		+=

SRCALL 		= $(wildcard *.cc)
DEPENDS		= $(SRCALL:.cc=.d)

SRC		= $(filter-out %main.cc, $(SRCALL))
OBJ		= $(SRC:.cc=.o)

TARGET 		= femcitiroc_control

all: $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/femcitiroc_control: femcitiroc_control_main.o $(OBJ)
	mkdir -p $(BIN_DIR)
	$(CXX) -o $@ $< $(OBJ) $(LDFLAGS)

clean:
	$(RM) $(BIN_DIR)/* *.o *.d *~

echo 		= /bin/echo -e
show:
	@ $(echo) "CXXFLAGS = $(CXXFLAGS)"
	@ $(echo) "LDFLAGS  = $(LDFLAGS)"
	@ $(echo) "SRCALL   = $(SRCALL)"
	@ $(echo) "DEPENDS  = $(DEPENDS)"
	@ $(echo) "SRC      = $(SRC)"
	@ $(echo) "OBJ      = $(OBJ)"

-include $(DEPENDS)
