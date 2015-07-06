#Laird MultiDeviceLoader

##About

MultiDeviceLoader is a command-line application for Windows and Linux that can be used to XCompile an application (Windows only) and download it to multiple Laird devices at the same time and provides options for renaming the file, running it after downloading it and other functions.

This is a companion application for [UwTerminalX](https://github.com/LairdCP/UwTerminalX).

##Downloading

Pre-compiled builds can be found by clicking the [Releases](https://github.com/LairdCP/MultiDeviceLoader/releases) tab on Github, builds are available for Linux (32 bit build) and Windows (32 bit build).

##Setup

###Windows

Download the zip and extract the executable file to a folder of your choice and execute the program using command prompt or powershell using the parameters documented on the [command line arguments wiki page](https://github.com/LairdCP/MultiDeviceLoader/wiki/Command-line-arguments).

###Linux

Download the tar and extract the executable file to a folder of your choice and execute the program using a terminal with the parameters documented on the [command line arguments wiki page](https://github.com/LairdCP/MultiDeviceLoader/wiki/Command-line-arguments).

Extracting the archive can either be done in the GUI or by using a terminal:

	tar xf MultiDeviceLoader.tar ~/

Executing MultiDeviceLoader can be achieved using

	./MultiDeviceLoader <command line arguments here>

##Compiling

Compiling MultiDeviceLoader is a similar process to compiling UwTerminalX, please refer to the [UwTerminalX compilation wiki page](https://github.com/LairdCP/UwTerminalX/wiki/Compiling), replacing the UwTerminalX source code tree with the MultiDeviceLoader source code tree.

##License

MultiDeviceLoader is released under the [GPLv3 license](https://github.com/LairdCP/MultiDeviceLoader/blob/master/LICENSE).