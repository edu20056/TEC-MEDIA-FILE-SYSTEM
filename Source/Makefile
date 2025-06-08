CXX = g++
MOC = moc
UIC = uic
RCC = rcc

CXXFLAGS = -std=c++17 -Wall -Wextra -fPIC
QTINCS = $(shell pkg-config --cflags Qt5Widgets Qt5Core Qt5Network)
QTLIBS = $(shell pkg-config --libs Qt5Widgets Qt5Core Qt5Network)

# Output folders
CONTROLLER_OUT = Build/Controller
GUI_OUT = Build/Gui
NODE_OUT = Build/Node

# HTTP format source
HTTP_SRC = HTTP/httpFormat.cpp
HTTP_INC = -IHTTP

# --------- Controller ---------
CONTROLLER_SRC = Controller/main.cpp Controller/controller.cpp $(HTTP_SRC)
CONTROLLER_HDR = Controller/controller.hpp
CONTROLLER_OBJ = $(addprefix $(CONTROLLER_OUT)/, main.o controller.o httpFormat.o)
CONTROLLER_MOC_CPP = $(CONTROLLER_OUT)/moc_controller.cpp
CONTROLLER_MOC_OBJ = $(CONTROLLER_OUT)/moc_controller.o

# --------- GUI ---------
GUI_SRC = GUI/main.cpp GUI/interface.cpp $(HTTP_SRC)
GUI_HDR = GUI/interface.hpp
GUI_OBJ = $(addprefix $(GUI_OUT)/, main.o interface.o httpFormat.o)
GUI_MOC_CPP = $(GUI_OUT)/moc_interface.cpp
GUI_MOC_OBJ = $(GUI_OUT)/moc_interface.o

# --------- Node ---------

# Targets
TARGETS = TECMFS-CTRL TECMFS-GUI

all: $(TARGETS)

# ------------------ CONTROLLER ------------------

TECMFS-CTRL: $(CONTROLLER_OBJ) $(CONTROLLER_MOC_OBJ)
	$(CXX) $^ -o $@ $(QTLIBS)

$(CONTROLLER_OUT)/%.o: Controller/%.cpp | $(CONTROLLER_OUT)
	$(CXX) $(CXXFLAGS) $(QTINCS) $(HTTP_INC) -c $< -o $@

$(CONTROLLER_OUT)/httpFormat.o: HTTP/httpFormat.cpp | $(CONTROLLER_OUT)
	$(CXX) $(CXXFLAGS) $(QTINCS) -c $< -o $@

$(CONTROLLER_MOC_CPP): $(CONTROLLER_HDR) | $(CONTROLLER_OUT)
	$(MOC) $< -o $@

$(CONTROLLER_MOC_OBJ): $(CONTROLLER_MOC_CPP)
	$(CXX) $(CXXFLAGS) $(QTINCS) -c $< -o $@

# ------------------ GUI ------------------

TECMFS-GUI: $(GUI_OBJ) $(GUI_MOC_OBJ)
	$(CXX) $^ -o $@ $(QTLIBS)

$(GUI_OUT)/%.o: GUI/%.cpp | $(GUI_OUT)
	$(CXX) $(CXXFLAGS) $(QTINCS) $(HTTP_INC) -c $< -o $@

$(GUI_OUT)/httpFormat.o: HTTP/httpFormat.cpp | $(GUI_OUT)
	$(CXX) $(CXXFLAGS) $(QTINCS) -c $< -o $@

$(GUI_MOC_CPP): $(GUI_HDR) | $(GUI_OUT)
	$(MOC) $< -o $@

$(GUI_MOC_OBJ): $(GUI_MOC_CPP)
	$(CXX) $(CXXFLAGS) $(QTINCS) -c $< -o $@

# ------------------ NODES ------------------

# ------------------ DIRECTORIES ------------------

$(CONTROLLER_OUT) $(GUI_OUT) $(NODE_OUT):
	@mkdir -p $@

# ------------------ CLEAN ------------------

clean:
	rm -rf Build
	rm -f TECMFS-CTRL TECMFS-GUI

.PHONY: all clean TECMFS-CTRL TECMFS-GUI
