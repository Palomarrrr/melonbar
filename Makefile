# Paths
PATH_LOCAL ?= /usr/local
PATH_X11 ?= /usr/X11R6
PATH_MAN ?= ${PATH_LOCAL}/share/man 
PATH_CONFIG ?= ${HOME}/.config
PATH_LOCAL_INC ?= ${PATH_LOCAL}/include
PATH_LOCAL_LIB ?= ${PATH_LOCAL}/lib
PATH_X11_INC ?= ${PATH_X11}/include
PATH_X11_LIB ?= ${PATH_X11}/lib
PATH_FREETYPE_INC ?= /usr/include/freetype2

# Includes/Libs/Flags
INCLUDE = -I${PATH_LOCAL_INC} -I${PATH_X11_INC} -I${PATH_FREETYPE_INC} -I${PATH_X11_INC}/freetype2
LIB = -L${PATH_LOCAL_LIB} -L${PATH_X11_LIB} -lfontconfig -lXft -lX11 -lXrender
CFLAGS += -O2
#CFLAGS += -Wall -pedantic -Wextra -Wno-unused-parameter -g

# Targets
TARGET = melonbar
CFILES = $(wildcard include/*.c)
OBJS = $(patsubst include%,lib%,$(CFILES:.c=.o))

lib/%.o : include/%.c
	@echo ''
	@echo '=====| Building $@ |====='
	${CC} $< $(INCLUDE) $(LIB) ${CFLAGS} -c -o $@

all: $(OBJS)
	@echo ''
	@echo '=====| Building $(TARGET) |====='
	${CC} $(TARGET).c $(INCLUDE) $(LIB) ${CFLAGS} -o $(TARGET) $(OBJS)

install: all
	@echo ''
	@echo '=====| Installing $(TARGET) |====='
	install -m 755 $(TARGET) /usr/local/bin/$(TARGET)
	@echo ''
	@echo 'Make sure to create a `melonbar` directory in .config'
	@echo 'and place the config file in there.'

clean:
	@echo ''
	@echo '=====| Cleaning |====='
	rm $(OBJS)
	rm $(TARGET)
