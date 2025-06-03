# TEC-MEDIA-FILE-SYSTEM
TEC MEDIA FILE SYSTEM is an application designed for managing PDF files through a RAID 5 storage architecture. It employs a Shared Disk Architecture in which multiple disk nodes operate under the coordination of a central controller.

The application includes a graphical user interface built using the Qt framework.

## Dependencies
To build and run the application, a few development tools and libraries must be installed. On Ubuntu-based operating systems, you can install them via the terminal.

First, install the necessary development tools required for compiling the project:
```bash
# Development Tools
sudo apt install build-essential cmake
```
Next, install the Qt libraries, which are needed for the graphical user interface:
```bash
#  Qt Libraries
sudo apt  qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
```

## How to Run
Begin by cloning the repository and navigating to the project directory. The compilation process uses a _Makefile_, so simply run the `make` command in the respective directories to build the project different parts of the program. First compile the `Controller` and the `GUI` -both located in the _Source_ folder-, then procid by executing both programs in their respective order. 

If changes are done to the code or want to remove generated files and binaries, you can clean the directory by running `make clean`.
