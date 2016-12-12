# compiler and Linker
CC          := gcc

# the Target Binary Program
TARGET      := lzss

# the Directories, Source, Includes, Objects, Binary and Resources
SRCDIR      := src
TESTDIR     := test
INCDIR      := inc
BUILDDIR    := int
TEST_BUILDDIR    := int/test
TARGETDIR   := target
RESDIR      := res
SRCEXT      := c
DEPEXT      := d
OBJEXT      := o

# flags, Libraries and Includes
CFLAGS      := -g -std=c99 -Wall -Wpedantic -Werror
LIB         :=
INC         := -I$(INCDIR) -I/usr/local/include
TEST_INC    := -I$(INCDIR) -I/usr/local/include
INCDEP      := -I$(INCDIR)

#---------------------------------------------------------------------------------
# DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------
SOURCES      := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS      := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

TEST_SOURCES := $(shell find $(TESTDIR) -type f -name *.$(SRCEXT))
TEST_OBJECTS := $(patsubst $(TESTDIR)/%,$(TEST_BUILDDIR)/%,$(TEST_SOURCES:.$(SRCEXT)=.$(OBJEXT)))
TEST_OBJECTS += $(OBJECTS)

# default Make
all: $(TARGET)

# remake
remake: cleaner all

# make the Directories
directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)
	@mkdir -p $(TEST_BUILDDIR)

# clean only objects
clean:
	@$(RM) -rf $(BUILDDIR)

# full clean, Objects and Binaries
cleaner: clean
	@$(RM) -rf $(TARGETDIR)

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))
-include $(TEST_OBJECTS:.$(OBJEXT)=.$(DEPEXT))

# link
$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGETDIR)/$(TARGET) $^ $(LIB)

# builds and runs the unit tests
test: $(TEST_OBJECTS)
	$(CC) -o $(TARGETDIR)/utest $^ $(LIB)

# compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<
	@$(CC) $(CFLAGS) $(INCDEP) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
	@cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	@rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

# compile
$(TEST_BUILDDIR)/%.$(OBJEXT): $(TESTDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(TEST_INC) -c -o $@ $<
	$(CC) $(CFLAGS) $(TEST_INC) -MM $(TESTDIR)/$*.$(SRCEXT) > $(TEST_BUILDDIR)/$*.$(DEPEXT)
	@cp -f $(TEST_BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(TEST_BUILDDIR)/$*.$(OBJEXT):|' < $(TEST_BUILDDIR)/$*.$(DEPEXT).tmp > $(TEST_BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(TEST_BUILDDIR)/$*.$(DEPEXT)
	@rm -f $(TEST_BUILDDIR)/$*.$(DEPEXT).tmp

# non-File Targets
.PHONY: all remake clean cleaner resources
