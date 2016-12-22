# binary file name
TARGET      := lzss

# makefile parameters
SRCDIR      := src
TESTDIR     := tests
BUILDDIR    := int
TARGETDIR   := target
SRCEXT      := c
OBJEXT      := o

# compiler parameters
CC          := gcc
CFLAGS      := -g -std=c99 -Wall -Wpedantic -Werror
LIB         :=
INC         := /usr/local/include
DEFINES     :=


#---------------------------------------------------------------------------------
# DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------

# sets the src directory in the VPATH
VPATH := $(SRCDIR)

# source files
SRCS := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))

# object files
OBJS := $(patsubst %,$(BUILDDIR)/%,$(SRCS:.$(SRCEXT)=.$(OBJEXT)))

# special definitions used for the unit tests
ifeq ($(MAKECMDGOALS),tests)
    # adds an extra include so the tests can include the sources
	INC += src

	# sets the special define for tests
	DEFINES := __TESTS__ $(DEFINES)

	# includes the tests directory in the VPATH
	VPATH := $(TESTDIR) $(VPATH)

	# test sources
	TEST_SRCS := $(shell find $(TESTDIR) -type f -name *.$(SRCEXT))

	# test objects
	OBJS := $(patsubst %,$(BUILDDIR)/%,$(TEST_SRCS:.$(SRCEXT)=.$(OBJEXT))) $(OBJS)
endif

# adds the include prefix to the include directories
INC := $(addprefix -I,$(INC))

# adds the define prefix to the defines
DEFINES := $(addprefix -D,$(DEFINES))


# default: compiles the binary
$(TARGET): $(OBJS) | dirs
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) $^ $(LIB) -o $(TARGETDIR)/$(TARGET)
	@echo "LD $@"

# builds and runs the unit tests
tests: $(OBJS) | dirs
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) $^ $(LIB) -o $(TARGETDIR)/tests
	@echo "LD $@"
	./$(TARGETDIR)/tests

# clean objects and binaries
clean:
	@$(RM) -rf $(BUILDDIR) $(TARGETDIR)

# creates the directories
dirs:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

# rule to build dependencies files
$(BUILDDIR)/%.d:
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) -MM $*.$(SRCEXT) -MT $(BUILDDIR)/$*.$(OBJEXT) > $(BUILDDIR)/$*.d

# rule to build object files
$(BUILDDIR)/%.$(OBJEXT): %.$(SRCEXT) $(BUILDDIR)/%.d
	@echo "CC $<"
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) -c -o $@ $<


.PHONY: clean dirs tests $(TARGET)

# includes generated dependency files
-include $(OBJS:.$(OBJEXT)=.d)

# this avoids the generated files from being deleted
.SECONDARY:
