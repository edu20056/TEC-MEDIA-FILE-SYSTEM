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
Begin by cloning the repository and navigating to the project directory. The compilation process uses a _Makefile_, so simply run the `make` in  the _Source_ folder. Then procid to execute `./TECMFS-CTRL` and `./TECMFS-GUI` in their respective order -and in your terminal of choice- to run the application. 

If changes are done to the code or want to remove generated files and binaries, you can clean the directory by running `make clean`.

## Problems to Fix
### Used Port
As for right now the `TECMFS-CTRL` generates a server that is not finalized automatically, so the process could go ongoing until it's manually ended.

To verify if the process is ongoing you may use:
```bash
lsof -i :<PID>
```

If there is an existing process initialized via the command `TECMFS-CTRL`, it can be killed with:
```bash
kill <PID>
```
