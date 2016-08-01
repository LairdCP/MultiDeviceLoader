/******************************************************************************
** Copyright (C) 2015-2016 Laird
**
** Project: MultiDeviceLoader
**
** Module: main.h
**
** Notes:
**
** License: This program is free software: you can redistribute it and/or
**          modify it under the terms of the GNU General Public License as
**          published by the Free Software Foundation, version 3.
**
**          This program is distributed in the hope that it will be useful,
**          but WITHOUT ANY WARRANTY; without even the implied warranty of
**          MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**          GNU General Public License for more details.
**
**          You should have received a copy of the GNU General Public License
**          along with this program.  If not, see http://www.gnu.org/licenses/
**
*******************************************************************************/
#ifndef MAIN_H
#define MAIN_H

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QCoreApplication>
#include <QtSerialPort/QSerialPort>
#include <QSettings>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QRegularExpression>
#include <QProcess>
#include <QDataStream>
#ifdef _WIN32
//Windows include for Sleep()
#include <windows.h>
#else
//Linux include for usleep()
#include <unistd.h>
#endif

/******************************************************************************/
// Defines
/******************************************************************************/
#define DefaultFWRHSize   72       //Number of hex characters to send per FWRH line
#define PauseRate         100      //Lower = faster but will break slower baud rates. 50 for 9600, 20 for 115200
#define MaxPorts          10       //Number of maximum ports to allow
#define MaxFilenameLength 20       //Maximum allowable length of a filename (for the target BL600/BL620/BT900 device)
#define AppVersion        "v1.03"  //Version string
#define MaxDevNameSize    8        //Size (in characters) to allow for a module device name (characters past this point will be chopped off)

/******************************************************************************/
// Global/Static Variable Declarations
/******************************************************************************/
QSerialPort SerialHandles[MaxPorts]; //Handle of Serial handle objects

/******************************************************************************/
// Defines for error/success codes
/******************************************************************************/
#define SUCCESS_CODE                            0  //Exit code if everything was successful
#define ERROR_CODE_TOO_MANY_PORTS               1  //Exit code if too many ports have been specified on the command line
#define ERROR_CODE_INVALID_DOWNLOAD_FILENAME    2  //Exit code if the specified download filename is not valid
#define ERROR_CODE_INVALID_BAUD_RATE            3  //Exit code if the specified baud rate is not valid
#define ERROR_CODE_NO_PORTS_PROVIDED            4  //Exit code if no ports have been provided
#define ERROR_CODE_XCOMPILE_DETAILS_FAILED      5  //Exit code if an error occured getting the XCompiler details from the first module (Windows only)
#define ERROR_CODE_INVALID_XCOMPILER            6  //Exit code if required XCompiler cannot be found (Windows only)
#define ERROR_CODE_XCOMPILE_FAILED              7  //Exit code if XCompilation failed (Windows only)
#define ERROR_CODE_XCOMPILE_FILE_INVALID        8  //Exit code if the file created by the XCompiler does not exist (Windows only)
#define ERROR_CODE_FILE_NOT_DOWNLOADED          9  //Exit code if the file is missing from one or more of the modules
#define ERROR_CODE_PORT_OPEN_FAILED             10 //Exit code if a port failed to opened and AllowPortFail is set to 0 or not set
#define ERROR_CODE_NO_PARAMETERS                11 //Exit code if no parameters was supplied to the program

/******************************************************************************/
// Class definitions
/******************************************************************************/
int
main(
    int argc,
    char *argv[]
    );

void
ClosePorts(
    unsigned char ucNumPorts
    );

QString
AtiToXCompName(
    QString strAtiResp
    );

#endif // MAIN_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/
