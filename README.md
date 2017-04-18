# Laird MultiDeviceLoader

## About

MultiDeviceLoader is a command-line application for Windows, Linux and Mac that can be used to XCompile an application (Windows only) and download it to multiple Laird devices at the same time and provides options for renaming the file, running it after downloading it and other functions.

This is a companion application for [UwTerminalX](https://github.com/LairdCP/UwTerminalX) and is mostly suited to production/end-stage testing purposes.

## Downloading

Pre-compiled builds can be found by clicking the [Releases](https://github.com/LairdCP/MultiDeviceLoader/releases) tab on Github, builds are available for Linux (32 bit build), Windows (32 bit build) and Mac (64 bit build).

## Setup

### Windows

Download the zip and extract the executable file to a folder of your choice and execute the program using command prompt or powershell using the parameters documented on the [command line arguments wiki page](https://github.com/LairdCP/MultiDeviceLoader/wiki/Command-line-arguments).

### Linux

Download the tar and extract the executable file to a folder of your choice and execute the program using a terminal with the parameters documented on the [command line arguments wiki page](https://github.com/LairdCP/MultiDeviceLoader/wiki/Command-line-arguments).

Extracting the archive can either be done in the GUI or by using a terminal:

	tar xf MultiDeviceLoader.tar ~/

Executing MultiDeviceLoader can be achieved using:

	./MultiDeviceLoader <command line arguments here>

### Mac

Download the dmg and mount the image, then simply copy the executable to a location of your choosing. The executable must be ran using terminal and may require you to adjust your security preferences to allow it to run.

Change to the directory where MultiDeviceLoader is located:

	cd Downloads/MultiDeviceLoader

Executing MultiDeviceLoader can be achieved using:

	./MultiDeviceLoader <command line arguments here>

## Compiling

Compiling MultiDeviceLoader is a similar process to compiling UwTerminalX, please refer to the [UwTerminalX compilation wiki page](https://github.com/LairdCP/UwTerminalX/wiki/Compiling), replacing the UwTerminalX source code tree with the MultiDeviceLoader source code tree.

## Usage

For a list of parameters that can be supplied and to see an example of using the application please refer to the [Sample_Usage.txt file](https://github.com/LairdCP/MultiDeviceLoader/blob/master/Sample_Usage.txt)

*Please note:* Checksum verification is not currently supported on the BT900 so using `Verify=2` or `Verify=3` will cause an error that the file verification has failed.

## License

MultiDeviceLoader is released under the [GPLv3 license](https://github.com/LairdCP/MultiDeviceLoader/blob/master/LICENSE).