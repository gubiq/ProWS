include ../bench.mk

.PHONY: all clean

TARGETS = sum 

all: $(TARGETS) 

INC += -I../util
CFLAGS += $(APPFLAGS) $(RDFLAGS) $(INC)
CXXFLAGS += $(APPFLAGS) $(RDFLAGS) $(INC) -fcilk-no-inline -std=c++11
LDFLAGS += -flto -fcilk-no-inline $(RTS_LIB) -lnuma

sum: sum.o
	$(CXX) $^ $(LDFLAGS) $(LIB) -o $@

# delete all object files
clean:
	rm -f *.o $(TARGETS) core.*
