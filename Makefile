CXX      = g++
CXXFLAGS = -Wall -O2 -MMD -MP -std=c++23
INCDIR   = -I./include
LIBDIR   = -L./lib
LIBS     =

TARGET   = yu6ikey
SRCDIR   = src
BUILDDIR = build
OBJDIR   = $(BUILDDIR)/obj

SRCS     = $(shell find $(SRCDIR) -name "*.cpp" -type f)
OBJS     = $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
DEPS     = $(OBJS:.o=.d)

all: $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) -o $@ $(OBJS) $(LIBDIR) $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCDIR) -c $< -o $@

run: all
	sudo ./$(BUILDDIR)/$(TARGET)

clean:
	rm -rf $(BUILDDIR)

.PHONY: all run clean

-include $(DEPS)
