CXX = g++
MOC = moc
UIC = uic
RCC = rcc

CXXFLAGS = -std=c++17 -Wall -Wextra -I$(SRCDIR)
QTFLAGS = -fPIC
QTINCS = $(shell pkg-config --cflags Qt5Widgets)
QTLIBS = $(shell pkg-config --libs Qt5Widgets)

SRCDIR = Source
BUILDDIR = Resource
TARGET = TEC-MFS

SOURCES = $(SRCDIR)/main.cpp $(SRCDIR)/mainwindow.cpp $(SRCDIR)/secondwindow.cpp
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)

HEADERS_NEEDING_MOC = $(SRCDIR)/mainwindow.hpp $(SRCDIR)/secondwindow.hpp
MOC_SRC = $(HEADERS_NEEDING_MOC:$(SRCDIR)/%.hpp=$(BUILDDIR)/moc_%.cpp)
MOC_OBJ = $(MOC_SRC:%.cpp=%.o)

all: $(TARGET) $(MOC_SRC)

$(TARGET): $(OBJECTS) $(MOC_OBJ)
	$(CXX) $^ -o $@ $(QTLIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(QTFLAGS) $(QTINCS) -c $< -o $@

$(BUILDDIR)/moc_%.cpp: $(SRCDIR)/%.hpp | $(BUILDDIR)
	$(MOC) $< -o $@
	@touch $(BUILDDIR)/moc_$*.cpp  # Asegura timestamp actualizado

$(BUILDDIR)/moc_%.o: $(BUILDDIR)/moc_%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(QTFLAGS) $(QTINCS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)
	rm $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild