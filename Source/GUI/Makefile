CXX = g++
MOC = moc
UIC = uic
RCC = rcc

CXXFLAGS = -std=c++17 -Wall -Wextra -I$(SRCDIR) -I$(HTTPDIR)
QTFLAGS = -fPIC
QTINCS = $(shell pkg-config --cflags Qt5Widgets Qt5Core Qt5Network)
QTLIBS = $(shell pkg-config --libs Qt5Widgets Qt5Core Qt5Network)

SRCDIR = .
HTTPDIR = ../HTTP
BUILDDIR = Resource
TARGET = TECMFS-GUI

SOURCES = $(SRCDIR)/main.cpp $(SRCDIR)/interface.cpp $(HTTPDIR)/httpFormat.cpp
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(filter $(SRCDIR)/%,$(SOURCES))) \
          $(patsubst $(HTTPDIR)/%.cpp,$(BUILDDIR)/%.o,$(filter $(HTTPDIR)/%,$(SOURCES)))

HEADERS_NEEDING_MOC = $(SRCDIR)/interface.hpp
MOC_SRC = $(patsubst $(SRCDIR)/%.hpp,$(BUILDDIR)/moc_%.cpp,$(HEADERS_NEEDING_MOC))
MOC_OBJ = $(MOC_SRC:%.cpp=%.o)

all: $(TARGET)

$(TARGET): $(OBJECTS) $(MOC_OBJ)
	$(CXX) $^ -o $@ $(QTLIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(QTFLAGS) $(QTINCS) -c $< -o $@

$(BUILDDIR)/%.o: $(HTTPDIR)/%.cpp | $(BUILDDIR)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(QTFLAGS) $(QTINCS) -c $< -o $@

$(BUILDDIR)/moc_%.cpp: $(SRCDIR)/%.hpp | $(BUILDDIR)
	@mkdir -p $(@D)
	$(MOC) $< -o $@

$(BUILDDIR)/moc_%.o: $(BUILDDIR)/moc_%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(QTFLAGS) $(QTINCS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)
	rm -f $(TARGET)

.PHONY: all clean rebuild
