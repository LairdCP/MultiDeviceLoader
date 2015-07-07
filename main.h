/******************************************************************************
** Copyright (C) 2015 Laird
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
#define FWRHSize 72 //Number of hex characters to send per FWRH line
#define PauseRate 100 //Lower = faster but will break slower baud rates. 50 for 9600, 20 for 115200
#define MaxPorts 10 //Number of maximum ports to allow
#define MaxFilenameLength 20 //Maximum allowable length of a filename (for the target BL600/BL620/BT900 device)
#define AppVersion "v0.79b" //Version string

/******************************************************************************/
// Global/Static Variable Declarations
/******************************************************************************/
QSerialPort SerialHandles[MaxPorts]; //Handle of Serial handle objects

/******************************************************************************/
// Defines
/******************************************************************************/
#define SUCCESS_CODE                            0
#define ERROR_CODE_TOO_MANY_PORTS               1
#define ERROR_CODE_INVALID_DOWNLOAD_FILENAME    2
#define ERROR_CODE_INVALID_BAUD_RATE            3
#define ERROR_CODE_NO_PORTS_PROVIDED            4
#define ERROR_CODE_XCOMPILE_DETAILS_FAILED      5
#define ERROR_CODE_INVALID_XCOMPILER            6
#define ERROR_CODE_XCOMPILE_FAILED              7
#define ERROR_CODE_XCOMPILE_FILE_INVALID        8
#define ERROR_CODE_FILE_NOT_DOWNLOADED          9

/******************************************************************************/
// Class definitions
/******************************************************************************/
int
main
    (
    int argc,
    char *argv[]
    );

void
ClosePorts
    (
    unsigned char ucNumPorts
    );

#endif // MAIN_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/
