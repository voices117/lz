# compiler
CC          := gcc

# binary file name
TARGET      := lzss

# the Directories, Source, Includes, Objects, Binary and Resources
SRCDIR      := src
TESTDIR     := tests
INCDIR      := inc
BUILDDIR    := int
TARGETDIR   := target
SRCEXT      := c
OBJEXT      := o
DEFINES     :=

# flags, libraries and includes
CFLAGS      := -g -std=c99 -Wall -Wpedantic -Werror
LIB         :=
INC         := -I$(INCDIR) -I/usr/local/include
DEFINES     :=

#---------------------------------------------------------------------------------
# DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------

SOURCES     := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))


# default: compiles the binary
$(TARGET): $(OBJECTS) | dirs
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) -o $(TARGETDIR)/$(TARGET) $^ $(LIB)
	@echo "LD $@"

# builds and runs the unit tests
tests: $(TEST_OBJECTS) $(OBJECTS) | dirs
	$(CC) $(CFLAGS) $(INC) $(DEFINES) -o $(TARGETDIR)/tests $^ $(LIB)
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
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) -MM $(SRCDIR)/$*.$(SRCEXT) -MT $(BUILDDIR)/$*.$(OBJEXT) > $(BUILDDIR)/$*.d

# rule to build object files
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT) $(BUILDDIR)/%.d
	@echo "CC $<"
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) -c -o $@ $<


.PHONY: clean dirs

# includes generated dependency files
-include $(OBJECTS:.$(OBJEXT)=.d)
