CFLAGS 	:= -std=c17
CFLAGS 	+= -no-pie
CFLAGS 	+= -g3
CFLAGS 	+= -ggdb
CFLAGS 	+= -Wall
CFLAGS 	+= -Wextra
CFLAGS 	+= -Warray-bounds
CFLAGS 	+= -Wconversion
CFLAGS 	+= -Wmissing-braces
CFLAGS 	+= -Wno-parentheses
CFLAGS 	+= -Wno-format-truncation
CFLAGS	+= -Wno-type-limits
CFLAGS 	+= -Wpedantic
CFLAGS 	+= -Wstrict-prototypes
CFLAGS 	+= -Wwrite-strings
CFLAGS 	+= -Winline
CFLAGS 	+= -s
CFLAGS 	+= -O2
CFLAGS 	+= -D_FORTIFY_SOURCE=2
CFLAGS	+= -MD

BIN 	     := filter
SRCS 		 := $(wildcard src/*.c)
INSTALL_PATH := /usr/local/bin

LDLIBS 	:= -lm

all: $(BIN)

$(BIN): $(SRCS:.c=.o) 
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -o $@

install: $(INSTALL_PATH)/$(BIN)

$(INSTALL_PATH)/$(BIN): $(BIN)
	install $< $@

clean:
	$(RM) src/*.o src/*.d 

fclean:
	$(RM) $(BIN) 

-include $(wildcard src/*.d)

.PHONY: clean all fclean install
.DELETE_ON_ERROR:
