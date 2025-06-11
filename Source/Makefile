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

# xmlReader source
XML_SRC = Config/xmlReader.cpp
XML_INC = -IConfig

# --------- Controller ---------
CONTROLLER_SRC = Controller/main.cpp Controller/controller.cpp $(HTTP_SRC) $(XML_SRC)
CONTROLLER_HDR = Controller/controller.hpp
CONTROLLER_OBJ = $(addprefix $(CONTROLLER_OUT)/, main.o controller.o httpFormat.o xmlReader.o)
CONTROLLER_MOC_CPP = $(CONTROLLER_OUT)/moc_controller.cpp
CONTROLLER_MOC_OBJ = $(CONTROLLER_OUT)/moc_controller.o

# --------- GUI ---------
GUI_SRC = GUI/main.cpp GUI/interface.cpp $(HTTP_SRC)
GUI_HDR = GUI/interface.hpp
GUI_OBJ = $(addprefix $(GUI_OUT)/, main.o interface.o httpFormat.o)
GUI_MOC_CPP = $(GUI_OUT)/moc_interface.cpp
GUI_MOC_OBJ = $(GUI_OUT)/moc_interface.o

# --------- Node ---------
NODE_SRC = Node/main.cpp Node/diskNode.cpp $(HTTP_SRC) $(XML_SRC)
NODE_HDR = Node/diskNode.hpp
NODE_OBJ = $(addprefix $(NODE_OUT)/, main.o diskNode.o httpFormat.o xmlReader.o)
NODE_MOC_CPP = $(NODE_OUT)/moc_diskNode.cpp
NODE_MOC_OBJ = $(NODE_OUT)/moc_diskNode.o

# Targets
TARGETS = TECMFS-CTRL TECMFS-GUI TECMFS-NODE

all: $(TARGETS)

# ------------------ CONTROLLER ------------------

TECMFS-CTRL: $(CONTROLLER_OBJ) $(CONTROLLER_MOC_OBJ)
	$(CXX) $^ -o $@ $(QTLIBS)

$(CONTROLLER_OUT)/%.o: Controller/%.cpp | $(CONTROLLER_OUT)
	$(CXX) $(CXXFLAGS) $(QTINCS) $(HTTP_INC) $(XML_INC) -c $< -o $@

$(CONTROLLER_OUT)/httpFormat.o: HTTP/httpFormat.cpp | $(CONTROLLER_OUT)
	$(CXX) $(CXXFLAGS) $(QTINCS) -c $< -o $@

$(CONTROLLER_OUT)/xmlReader.o: Config/xmlReader.cpp | $(CONTROLLER_OUT)
	$(CXX) $(CXXFLAGS) $(QTINCS) $(XML_INC) -c $< -o $@

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

TECMFS-NODE: $(NODE_OBJ) $(NODE_MOC_OBJ)
	$(CXX) $^ -o $@ $(QTLIBS)

$(NODE_OUT)/%.o: Node/%.cpp | $(NODE_OUT)
	$(CXX) $(CXXFLAGS) $(QTINCS) $(HTTP_INC) $(XML_INC) -c $< -o $@

$(NODE_OUT)/httpFormat.o: HTTP/httpFormat.cpp | $(NODE_OUT)
	$(CXX) $(CXXFLAGS) $(QTINCS) -c $< -o $@

$(NODE_OUT)/xmlReader.o: Config/xmlReader.cpp | $(NODE_OUT)
	$(CXX) $(CXXFLAGS) $(QTINCS) $(XML_INC) -c $< -o $@

$(NODE_MOC_CPP): $(NODE_HDR) | $(NODE_OUT)
	$(MOC) $< -o $@

$(NODE_MOC_OBJ): $(NODE_MOC_CPP)
	$(CXX) $(CXXFLAGS) $(QTINCS) -c $< -o $@

# ------------------ DIRECTORIES ------------------

$(CONTROLLER_OUT) $(GUI_OUT) $(NODE_OUT):
	@mkdir -p $@

# ------------------ CLEAN ------------------

clean:
	rm -rf Build
	rm -f TECMFS-CTRL TECMFS-GUI TECMFS-NODE

.PHONY: all clean TECMFS-CTRL TECMFS-GUI TECMFS-NODE
