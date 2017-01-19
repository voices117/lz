# binary file name
TARGET      := lzss

# makefile parameters
SRCDIR      := src
TESTDIR     := tests
BUILDDIR    := int
TARGETDIR   := target
SRCEXT      := c

# compiler parameters
CC          := gcc
CFLAGS      := -g3 -std=c99 -Wall -Wpedantic -Werror
LIB         := m
INC         := /usr/local/include
DEFINES     :=


#---------------------------------------------------------------------------------
# DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------

# sets the src directory in the VPATH
VPATH := $(SRCDIR)

# sets the build directory based on the profile
BUILDDIR := $(BUILDDIR)/$(PROFILE)

# source files
SRCS := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))

# object files
OBJS := $(patsubst %,$(BUILDDIR)/%,$(SRCS:.$(SRCEXT)=.o))

# includes the flag to generate the dependency files when compiling
CFLAGS += -MD


# special definitions used for the unit tests
ifeq ($(MAKECMDGOALS),profile-tests)
    # adds an extra include so the tests can include the sources
	INC += src

	# sets the special define for tests
	DEFINES := __TESTS__ $(DEFINES)

	# includes the tests directory in the VPATH
	VPATH := $(TESTDIR) $(VPATH)

	# test sources
	TEST_SRCS := $(shell find $(TESTDIR) -type f -name *.$(SRCEXT))

	# test objects
	OBJS := $(patsubst %,$(BUILDDIR)/%,$(TEST_SRCS:.$(SRCEXT)=.o)) $(OBJS)
endif

# adds the include prefix to the include directories
INC := $(addprefix -I,$(INC))

# adds the lib prefix to the libraries
LIB := $(addprefix -l,$(LIB))

# adds the define prefix to the defines
DEFINES := $(addprefix -D,$(DEFINES))


# default: compiles the binary
$(TARGET):
	@$(MAKE) profile-$(TARGET) PROFILE=$(TARGET)

# compiles and runs the unit tests
tests:
	@$(MAKE) profile-tests PROFILE=tests

# clean objects and binaries
clean:
	@$(RM) -rf $(BUILDDIR) $(TARGETDIR)

# creates the directories
dirs:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

# INTERNAL: builds the binary
profile-$(TARGET): $(OBJS) | dirs
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) $^ $(LIB) -o $(TARGETDIR)/$(TARGET)
	@echo "LD $@"

# INTERNAL: builds and runs the unit tests
profile-tests: $(OBJS) | dirs
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) $^ $(LIB) -o $(TARGETDIR)/tests
	@echo "LD $@"
	./$(TARGETDIR)/tests

# rule to build object files
$(BUILDDIR)/%.o: %.$(SRCEXT)
	@mkdir -p $(basename $@)
	@echo "CC $<"
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) $(LIB) -c -o $@ $<


.PHONY: clean dirs tests $(TARGET) profile-$(TARGET) profile-tests

# includes generated dependency files
-include $(OBJS:.o=.d)
