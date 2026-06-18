#
#		███╗   ███╗ █████╗ ██╗  ██╗███████╗███████╗██╗██╗     ███████╗
#		████╗ ████║██╔══██╗██║ ██╔╝██╔════╝██╔════╝██║██║     ██╔════╝
#		██╔████╔██║███████║█████╔╝ █████╗  █████╗  ██║██║     █████╗  
#		██║╚██╔╝██║██╔══██║██╔═██╗ ██╔══╝  ██╔══╝  ██║██║     ██╔══╝  
#		██║ ╚═╝ ██║██║  ██║██║  ██╗███████╗██║     ██║███████╗███████╗
#		╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝     ╚═╝╚══════╝╚══════╝
#
# mogrify -format png *.ppm

# OUTPUT OF `$(cc) -v`:
# 	Using built-in specs.
# 	COLLECT_GCC=gcc
# 	COLLECT_LTO_WRAPPER=/usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/lto-wrapper
# 	Target: x86_64-pc-linux-gnu
# 	Configured with: /build/gcc/src/gcc/configure \ 
#		--enable-languages=ada,c,c++,d,fortran,go,lto,m2,objc,obj-c++,rust,cobol \
#		--enable-bootstrap --prefix=/usr --libdir=/usr/lib --libexecdir=/usr/lib \
#		--mandir=/usr/share/man --infodir=/usr/share/info \
#		--with-bugurl=https://gitlab.archlinux.org/archlinux/packaging/packages/gcc/-/issues \
#		--with-build-config=bootstrap-lto --with-linker-hash-style=gnu --with-system-zlib \
#		--enable-__cxa_atexit --enable-cet=auto --enable-checking=release --enable-clocale=gnu \
#		--enable-default-pie --enable-default-ssp --enable-gnu-indirect-function \
#		--enable-gnu-unique-object --enable-libstdcxx-backtrace --enable-link-serialization=1 \
#		--enable-linker-build-id --enable-lto --enableultilib --enable-plugin --enable-shared \
#		--enable-threads=posix --disable-libssp --disable-libstdcxx-pch --disable-werror
# 	Thread model: posix
# 	Supported LTO compression algorithms: zlib zstd
#	gcc version 15.2.1 20251112 (GCC) 


TARGET = tes 
LIBS_INCLUDED = 1
# OPTIMIZING = 1



#		███████╗███████╗████████╗████████╗██╗███╗   ██╗ ██████╗ ███████╗
#		██╔════╝██╔════╝╚══██╔══╝╚══██╔══╝██║████╗  ██║██╔════╝ ██╔════╝
#		███████╗█████╗     ██║      ██║   ██║██╔██╗ ██║██║  ███╗███████╗
#		╚════██║██╔══╝     ██║      ██║   ██║██║╚██╗██║██║   ██║╚════██║
#		███████║███████╗   ██║      ██║   ██║██║ ╚████║╚██████╔╝███████║
#		╚══════╝╚══════╝   ╚═╝      ╚═╝   ╚═╝╚═╝  ╚═══╝ ╚═════╝ ╚══════╝                                                                

# GLOBAL SETTINGS: compiler & compilerflags & compilerwarnings
.PHONY: clean all clean_libs clean_all

CC      	:= gcc
CWARNINGS	:= -Wall -Wextra -Wpedantic # -Wconversion -Wsign-conversion # -Weverything
CFLAGS  	:= $(CWARNINGS)
LDLIBS		= -pthread -lm # libraries to link against & link time optimization

ifdef OPTIMIZING
CFLAGS += -O3 -flto -fno-plt -march=native
endif

ifdef DEBUG # if debugging
CFLAGS += -g -fno-omit-frame-pointer -march=native # NOTE: OPTIMIZING IS STILL ON!
ifdef OPTIMIZING
$(info debug on: optimizing is still on default! Add -O0 to turn off optimisation)
endif
endif

# PROJECT STRUCTURE SETTINGS
SRC_DIR	= src
INC_DIR	= include
OBJ_DIR	= build
ifdef LIBS_INCLUDED # if we have a project with internal libraries
LIB_DIR = libs
endif
IFLAGS := -I$(INC_DIR) # include flags

# source files
SRCS 	:= $(wildcard $(SRC_DIR)/*.c)
OBJS	:= $(addprefix $(OBJ_DIR)/,$(notdir $(SRCS:.c=.o)))

# compiling static libraries in project
ifdef LIBS_INCLUDED
LIB_SRCS		:= $(wildcard libs/*/src/*.c) # the .c files in libs/...
LIB_SRCS_NAMES	:= $(notdir $(LIB_SRCS)) # library source file base names
LIB_OBJS		:= $(addprefix $(OBJ_DIR)/,$(LIB_SRCS_NAMES:.c=.o)) # the corresponding .o files in build/
STATIC_LIBS		:= $(patsubst %.c,%.a,$(subst /src/,/,$(LIB_SRCS))) # DANGEROUS!!!: target .a files
LIB_INCDS		:= $(wildcard $(LIB_DIR)/*/include) # list of all include dirs in libs folder
IFLAGS			+= $(addprefix -I,$(LIB_INCDS)) # adds -I[includes] to IFLAGS
endif

#####################################################################
## custom additions
HAVE_OPENCL := $(shell ldconfig -p | grep -q OpenCL && echo 1)

ifeq ($(USE_OPENCL),1)
  ifeq ($(HAVE_OPENCL),1)
    CFLAGS  += -DUSE_OPENCL
    LDLIBS  += -lOpenCL
  else
    $(error OpenCL requested but not found)
  endif
endif
#####################################################################

#				██████╗ ██╗   ██╗██╗     ███████╗███████╗
#				██╔══██╗██║   ██║██║     ██╔════╝██╔════╝
#				██████╔╝██║   ██║██║     █████╗  ███████╗
#				██╔══██╗██║   ██║██║     ██╔══╝  ╚════██║
#				██║  ██║╚██████╔╝███████╗███████╗███████║
#				╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚══════╝╚══════╝

ifdef LIBS_INCLUDED
all: $(STATIC_LIBS) $(OBJ_DIR)/$(TARGET)

$(STATIC_LIBS):
	$(MAKE) -C $(dir $@) CFLAGS="$(CFLAGS)"
endif

ifndef LIBS_INCLUDED
all: $(TARGET)
endif

$(OBJ_DIR)/$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(STATIC_LIBS) $(LDLIBS)

build/%.o: src/%.c $(INC_DIR)/defs.h
	@mkdir -p build/
	$(CC) $(CFLAGS) $(IFLAGS) -c $< -o $@

clean:
	rm -rf build/
clean_libs: clean
	rm libs/*/*.a
clean_all: clean clean_libs