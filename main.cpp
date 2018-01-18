/******************************************************************************
** Copyright (C) 2015-2018 Laird
**
** Project: MultiDeviceLoader
**
** Module: main.cpp
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

/*
 * Command line parameters:
 *
 * RunOnExit=n        1 = run application when downloaded, 0 = do not run application when downloaded
 * EraseFS=n          1 = erase file-system, 0 = do not erase file-system
 * XCompile=n         1 = XCompile source and download application (Windows only), 0 = download application as-is
 * FlowControl=n      1 = Hardware flow control, 2 = Software flow control, 0 = No flow control
 * Baud=n             Baud rate (300-921600, 9600 is default for BL600/BL620 and 115200 is default for BT900/BL652/RM1xx)
 * DownloadFile=n     Path and filename to XCompile/download
 * RenameFile=n       Filename to download the file to the module as
 * PortFile=n         Path and filename to a port configuration file (not needed if Port=n is used)
 * Port=n             Specify a port to use (can be specified multiple times for additional ports), Windows: COM[1-255], Linux: /dev/[device] (not needed if PortFile=n is used)
 * Verbose=n          Output verbosity: 0 = none, 1 = normal, 2 = extra
 * Verify=n           File verification: 0 = none, 1 = checks file exists on module after download, 2 = check using checksum, 3 = checks file exists on module and checks using checksum
 * XCompDir=n         Specifies the directory to XCompilers (Windows only)
 * AllowPortFail=n    1 = continue running if some ports fail to open, 0 = quit program if any ports fail to open (default)
 * FWRHSize=n         Number of bytes to write per line to target devices. If a BL620-US is being used on GNU/Linux then setting this value to 50 might be required (default is 72, must be a multiple of 2).
 * OpenMode=n         0 = open files normally (will fail if file already exists), 1 = delete file before opening
 * DownloadCheck=n    0 = do not check download progress, 1 = check for error responses when downloading files
 */

/******************************************************************************/
// Include Files
/******************************************************************************/
#include "main.h"

/******************************************************************************/
// Local Functions or Private Members
/******************************************************************************/
int
main(
    int argc,
    char *argv[]
    )
{
    //Start QCoreApplication
    QCoreApplication app(argc, argv);

    //Local variables
    QString strDownloadFilename(""); //Filename to XCompile/download
    qint32 intBaudRate = QSerialPort::Baud9600; //Baud rate to use, 9600 by default
    bool bEraseFS = false; //True if filesystem is to be erased.
    QString strRenameFilename(""); //Filename to store on the target device
    bool bRunOnExit = false; //True if application should be executed after download
    bool bXCompileApplication = false; //True if the application is to be XCompiled before downloading
    unsigned char ucExtraVerbose = 0; //1 or 2 to output additional verbose messages
    qint8 intFlowControl = QSerialPort::HardwareControl; //Flow control, 0 = none, 1 = hardware (default), 2 = software
    QString strXCompilerDirectory(""); //Directory containing the XCompilers
    unsigned char ucCPortNum = 1; //Current port number
    unsigned char ucNumPorts = 0; //Total number of ports used
    unsigned char ucVerifyCheck = 1; //File verification: (0 = none, 1 = using at+dir, 2 = checksum, 3 = at+dir and checksum)
    QRegularExpression rxExp1("10\t0\t(.+?)\r"); //ATI 0 match
    QRegularExpression rxExp2("10\t13\t(.+?) (.+?) \r"); //ATI 13 match
    QRegularExpression rxExp3("01\t([0-9A-F]{4})\r"); //Error running match
    QByteArray baHexData; //Hex data of UWC file
    bool bAllowPortFail = false; //True if a port fails to open then continue without quitting the application
    int intFWRHSize = DefaultFWRHSize; //Number of characters to send per line to each module
    QString strChecksum; //Checksum of file (hex)
    bool bOpenmode = false; //When false will open file normally (fails if file exists), when true will delete file before opening
    bool bDownloadCheck = false; //When true will check for errors during file download

    //Output version
    qDebug() << "Laird MultiDeviceLoader" << AppVersion << "built" << __DATE__;

    //Go through and parse the command line parameters
    QStringList slArgs = QCoreApplication::arguments();
    unsigned char chi = 1;
    while (chi < slArgs.length())
    {
        if (slArgs[chi].left(10).toUpper() == "RUNONEXIT=")
        {
            //Run on exit configuration
            bRunOnExit = (slArgs[chi].right(1) == "0" ? false : true);
        }
        else if (slArgs[chi].left(8).toUpper() == "ERASEFS=")
        {
            //Erase filesystem configuration
            bEraseFS = (slArgs[chi].right(1) == "0" ? false : true);
        }
#ifdef _WIN32
        else if (slArgs[chi].left(9).toUpper() == "XCOMPILE=")
        {
            //XCompile application configuration (Only for Windows)
            bXCompileApplication = (slArgs[chi].right(1) == "0" ? false : true);
        }
#endif
        else if (slArgs[chi].left(12).toUpper() == "FLOWCONTROL=")
        {
            //Flow control configuration
            if (slArgs[chi].right(1) == "1")
            {
                //Hardware
                intFlowControl = QSerialPort::HardwareControl;
            }
            else if (slArgs[chi].right(1) == "2")
            {
                //Software
                intFlowControl = QSerialPort::SoftwareControl;
            }
            else
            {
                //Disabled
                intFlowControl = QSerialPort::NoFlowControl;
            }
        }
        else if (slArgs[chi].left(5).toUpper() == "BAUD=")
        {
            //Baud rate configuration
            intBaudRate = slArgs[chi].mid(5, -1).toInt();
        }
        else if (slArgs[chi].left(13).toUpper() == "DOWNLOADFILE=")
        {
            //Download/XCompile filename
            strDownloadFilename = slArgs[chi].mid(13, -1);
        }
        else if (slArgs[chi].left(11).toUpper() == "RENAMEFILE=")
        {
            //Rename filename
            strRenameFilename = slArgs[chi].mid(11, -1);
        }
        else if (slArgs[chi].left(14).toUpper() == "ALLOWPORTFAIL=")
        {
            //Set allowing port opening to fail or not
            bAllowPortFail = (slArgs[chi].mid(14, 1).toInt() == 0 ? false : true);
        }
        else if (slArgs[chi].left(9).toUpper() == "FWRHSIZE=")
        {
            //Set the maximum line length
            intFWRHSize = slArgs[chi].mid(9, -1).toInt();
        }
        else if (slArgs[chi].left(9).toUpper() == "PORTFILE=")
        {
            //Load ports from INI file
            if (QFile::exists(slArgs[chi].mid(9, -1)) == true)
            {
                //File exists - load it
                QString CPort;
                QSettings PortSettings(QString(slArgs[chi].mid(9, -1)), QSettings::IniFormat);
                while (ucCPortNum <= MaxPorts)
                {
                    //Load each port if required
                    CPort = PortSettings.value(QString("Port").append(QString::number(ucCPortNum)), "").toString();
                    if (CPort.length() > 3)
                    {
                        //Open this port
                        SerialHandles[ucNumPorts].setPortName(CPort);
                        SerialHandles[ucNumPorts].setBaudRate(intBaudRate);
                        SerialHandles[ucNumPorts].setDataBits(QSerialPort::Data8);
                        SerialHandles[ucNumPorts].setStopBits(QSerialPort::OneStop);
                        SerialHandles[ucNumPorts].setParity(QSerialPort::NoParity);
                        SerialHandles[ucNumPorts].setFlowControl((QSerialPort::FlowControl)intFlowControl);
                        if (SerialHandles[ucNumPorts].open(QIODevice::ReadWrite))
                        {
                            //Open success
                            ++ucNumPorts;
                        }
                        else
                        {
                            //Failed to open
                            qDebug() << "Failed to open port:" << CPort;
                            if (bAllowPortFail == false)
                            {
                                //Exit application
                                qDebug() << "AllowPortFail set to false, exiting.";
                                ClosePorts(ucNumPorts);
                                return ERROR_CODE_PORT_OPEN_FAILED;
                            }
                        }
                    }
                    ++ucCPortNum;
                }
            }
        }
        else if (slArgs[chi].left(5).toUpper() == "PORT=")
        {
            //Port for BL600/BL620/BL652/BT900 device
            if (ucNumPorts > MaxPorts)
            {
                //Too many ports have been opened.
                qDebug() << "Too many ports have been opened, maximum supported ports: " << MaxPorts;
                ClosePorts(ucNumPorts);
                return ERROR_CODE_TOO_MANY_PORTS;
            }

            //Open new port
            SerialHandles[ucNumPorts].setPortName(slArgs[chi].mid(5, -1));
            SerialHandles[ucNumPorts].setBaudRate(intBaudRate);
            SerialHandles[ucNumPorts].setDataBits(QSerialPort::Data8);
            SerialHandles[ucNumPorts].setStopBits(QSerialPort::OneStop);
            SerialHandles[ucNumPorts].setParity(QSerialPort::NoParity);
            SerialHandles[ucNumPorts].setFlowControl((QSerialPort::FlowControl)intFlowControl);
            if (SerialHandles[ucNumPorts].open(QIODevice::ReadWrite))
            {
                //Open success
                ++ucNumPorts;
            }
            else
            {
                //Failed to open
                qDebug() << "Failed to open port:" << slArgs[chi].mid(5, -1);
                if (bAllowPortFail == false)
                {
                    //Exit application
                    qDebug() << "AllowPortFail set to false, exiting.";
                    ClosePorts(ucNumPorts);
                    return ERROR_CODE_PORT_OPEN_FAILED;
                }
            }
        }
        else if (slArgs[chi].left(8).toUpper() == "VERBOSE=")
        {
            //Enable extra verbose debugging
            if (slArgs[chi].right(1) == "1")
            {
                //Verbose logging
                ucExtraVerbose = 1;
            }
            else if (slArgs[chi].right(1) == "2")
            {
                //Extra verbose logging
                ucExtraVerbose = 2;
            }
        }
#ifdef _WIN32
        else if (slArgs[chi].left(9).toUpper() == "XCOMPDIR=")
        {
            //Update XCompiler directory
            strXCompilerDirectory = slArgs[chi].mid(9, -1);
        }
#endif
        else if (slArgs[chi].left(7).toUpper() == "VERIFY=")
        {
            //Verify mode configuration
            if (slArgs[chi].right(1) == "0")
            {
                //No verification
                ucVerifyCheck = 0;
            }
            else if (slArgs[chi].right(1) == "1")
            {
                //Use at+dir
                ucVerifyCheck = 1;
            }
            else if (slArgs[chi].right(1) == "2")
            {
                //Checksum
                ucVerifyCheck = 2;
            }
            else if (slArgs[chi].right(1) == "3")
            {
                //Use at+dir and checksum
                ucVerifyCheck = 3;
            }
        }
        else if (slArgs[chi].left(9).toUpper() == "OPENMODE=")
        {
            //OpenMode configuration
            bOpenmode = (slArgs[chi].right(1) == "0" ? false : true);
        }
        else if (slArgs[chi].left(14).toUpper() == "DOWNLOADCHECK=")
        {
            //DownloadCheck configuration
            bDownloadCheck = (slArgs[chi].right(1) == "0" ? false : true);
        }
        ++chi;
    }
    if (chi == 1)
    {
        //No parameters passed
        qDebug() << "No parameters passed to executable, exiting.\r\nQuick help:\r\n  RunOnExit=n      1 = run application, 0 = do not run application\r\n  EraseFS=n        1 = erase file-system, 0 = do not erase file-system\r\n  XCompile=n       1 = XCompile source code (Windows only), 0 = download file as-is\r\n  FlowControl=n    Flow control: 1 = Hardware (default), 2 = Software, 0 = NA\r\n  Baud=n           Baud rate (300-921600, Mac only supports up to 230400)\r\n  DownloadFile=n   Path and filename to XCompile/download\r\n  RenameFile=n     Filename to download the file to the module as\r\n  PortFile=n       Path/filename to a port configuration file\r\n  Port=n           Specify a port to use (specify multiple times for additional ports), Windows: COM[1-255], Mac/Linux: /dev/[device]\r\n  Verbose=n        Output verbosity: 0 = none, 1 = normal, 2 = extra\r\n  Verify=n         File verification: 0 = none, 1 = checks file exists on module after download, 2 = check using checksum, 3 = checks file exists on module and checks using checksum\r\n  XCompDir=n       Specifies the directory to XCompilers (Windows only)\r\n  AllowPortFail=n  1 = continue running if some ports fail to open, 0 = quit program if any ports fail to open (default)\r\n  FWRHSize=n       Number of bytes to write per line to target devices (default is 72, must be multiple of 2)\r\n  OpenMode=n       0 = open files normally (will fail if file already exists), 1 = delete file before opening\r\n  DownloadCheck=n  0 = do not check download progress, 1 = check for error responses when downloading files\r\n";
        return ERROR_CODE_NO_PARAMETERS;
    }

    if (strDownloadFilename == "" || QFile::exists(strDownloadFilename) == false)
    {
        //Invalid download filename or file does not exist
        qDebug() << "Invalid or no download filename specified.";
        ClosePorts(ucNumPorts);
        return ERROR_CODE_INVALID_DOWNLOAD_FILENAME;
    }
    else if (!(intBaudRate >= 300 && intBaudRate <= 921600))
    {
        //Invalid baud rate
        qDebug() << "Invalid baud rate specified.";
        ClosePorts(ucNumPorts);
        return ERROR_CODE_INVALID_BAUD_RATE;
    }

    if (strRenameFilename == "")
    {
        //Nothing to rename to so use download filename as base
        QFileInfo Tmp(strDownloadFilename);
        if (Tmp.fileName().length() > MaxFilenameLength)
        {
            //Filename is too long - shortern
            strRenameFilename = Tmp.fileName();
            if (strRenameFilename.indexOf(".") > 0)
            {
                //Remove past the dot
                strRenameFilename = strRenameFilename.left(strRenameFilename.indexOf("."));
            }
        }
        else if (Tmp.fileName().indexOf(".") > 0)
        {
            //With file extension
            strRenameFilename = Tmp.fileName();
            strRenameFilename = strRenameFilename.left(strRenameFilename.indexOf("."));
        }
        else
        {
            //No file extension
            strRenameFilename = Tmp.fileName();
        }
    }
    else if (strRenameFilename.length() > MaxFilenameLength)
    {
        //Supplied rename filename is too long - shortern
        strRenameFilename = strRenameFilename.left(MaxFilenameLength);
    }

    if (!(ucNumPorts > 0))
    {
        //No devices were opened
        qDebug() << "Error: No ports were provided to download files to.";
        return ERROR_CODE_NO_PORTS_PROVIDED;
    }

    //Enable break on all devices
    if (ucExtraVerbose > 0)
    {
        //Debugging information
        qDebug() << "Applying BREAK to devices...";
    }
    ucCPortNum = 0;
    while (ucCPortNum < ucNumPorts)
    {
        SerialHandles[ucCPortNum].setBreakEnabled(true);
        ++ucCPortNum;
    }

    //Short wait
#ifdef _WIN32
    Sleep(500);
#else
    usleep(500000);
#endif

    //Disable break on all devices
    if (ucExtraVerbose > 0)
    {
        //Debugging information
        qDebug() << "Removing BREAK from devices...";
    }
    ucCPortNum = 0;
    while (ucCPortNum < ucNumPorts)
    {
        SerialHandles[ucCPortNum].setBreakEnabled(false);
        ++ucCPortNum;
    }

    //Short wait
#ifdef _WIN32
    Sleep(2000);
#else
    usleep(2000000);
#endif

#ifdef _WIN32
    if (bXCompileApplication == true)
    {
        //XCompile application - get XCompiler details from first device
        if (ucExtraVerbose > 0)
        {
            qDebug() << "Fetching XCompiler details from first device...";
        }
        SerialHandles[0].flush();
        SerialHandles[0].write("at i 0\rat i 13\r");
        SerialHandles[0].waitForBytesWritten(-1);
        while (SerialHandles[0].waitForReadyRead(1000));
        QByteArray RecDat = SerialHandles[0].readAll();

        //Set regex dot to match everything
        rxExp1.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
        rxExp2.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);

        //Match regex
        QRegularExpressionMatch match1 = rxExp1.match(RecDat);
        QRegularExpressionMatch match2 = rxExp2.match(RecDat);

        //Do we have a match?
        if (!match1.hasMatch() || !match2.hasMatch())
        {
            //Failed to get data
            qDebug() << "Failed to get XCompiler filename from first device.";
            ClosePorts(ucNumPorts);
            return ERROR_CODE_XCOMPILE_DETAILS_FAILED;
        }

        //Construct XCompiler filename
        QString XCompFilename = QString(strXCompilerDirectory).append("XComp_").append(AtiToXCompName(match1.captured(1))).append("_").append(match2.captured(1)).append("_").append(match2.captured(2)).append(".exe");

        //Output XCompiler filename
        if (ucExtraVerbose > 0)
        {
            qDebug() << "Using XCompiler" << XCompFilename;
        }

        if (!QFile::exists(XCompFilename))
        {
            //XCompiler doesn't exist
            qDebug() << "XCompiler executable does not exist.";
            return ERROR_CODE_INVALID_XCOMPILER;
        }

        //XCompile file
        signed char ExitCode = QProcess::execute(XCompFilename, QStringList() << strDownloadFilename << QString("/O") << QString(strDownloadFilename).append(".uwc"));
        if (ExitCode != 0)
        {
            //XCompile failed
            if (ExitCode == -1)
            {
                //Process crashed
                qDebug() << "XCompiler crashed!";
            }
            else if (ExitCode == -2)
            {
                //No such program
                qDebug() << "XCompiler does not exist, " << XCompFilename;
            }
            else
            {
                //Failed for another reason
                qDebug() << "XCompiler failed.";
            }
            ClosePorts(ucNumPorts);
            return ERROR_CODE_XCOMPILE_FAILED;
        }

        //Append UWC to download filename
        strDownloadFilename.append(".uwc");
        if (!QFile::exists(strDownloadFilename))
        {
            //File doesn't exist - exit
            qDebug() << "XCompiled application " << strDownloadFilename << " does not exist.";
            ClosePorts(ucNumPorts);
            return ERROR_CODE_XCOMPILE_FILE_INVALID;
        }
    }
#endif

    if (bEraseFS == true)
    {
        //Erase all filesystems if needed
        if (ucExtraVerbose > 0)
        {
            qDebug() << "Erasing filesystems...";
        }
        ucCPortNum = 0;
        while (ucCPortNum < ucNumPorts)
        {
            SerialHandles[ucCPortNum].write("at&f 1\r");
            while (SerialHandles[ucCPortNum].waitForReadyRead(1000));
            ++ucCPortNum;
        }

        //Short wait for devices to recover
#ifdef _WIN32
        Sleep(1000);
#else
        usleep(1000000);
#endif
    }

    //Get hex data from file
    if (ucExtraVerbose > 0)
    {
        qDebug() << "Converting file data into hex...";
    }
    QFile file(strDownloadFilename);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    baHexData.clear();
    while (!in.atEnd())
    {
        //One byte at a time, convert the data to hex
        quint8 ThisByte;
        in >> ThisByte;
        QString ThisHex;
        ThisHex.setNum(ThisByte, 16);
        if (ThisHex.length() == 1)
        {
            //Expand to 2 characters
            baHexData.append("0");
        }

        //Add the hex character to the string
        baHexData.append(ThisHex.toUpper());
    }

    //Calculate checksum if needed
    if ((ucVerifyCheck & 2) == 2)
    {
        //Rewind file
        file.seek(0);

        if (ucExtraVerbose > 0)
        {
            qDebug() << "Calculating file checksum...";
        }

        //Checksum value
        unsigned short usChecksum = 0xFFFF;

        while (!in.atEnd())
        {
            //Calculate checksum byte-by-byte
            quint8 ThisByte;
            in >> ThisByte;
            usChecksum = ByteChecksum(usChecksum, ThisByte);
        }

        //Convert to hex
        strChecksum = QString::number(usChecksum, 16).toUpper();

        while (strChecksum.length() < 4)
        {
            //Expand to 4 characters
            strChecksum.insert(0, "0");
        }

        //Add hex start
        strChecksum.prepend("0x");

        if (ucExtraVerbose > 0)
        {
            qDebug() << "File checksum:" << strChecksum;
        }
    }
    file.close();

    //Open files on devices
    if (ucExtraVerbose > 0)
    {
        qDebug() << "Opening files on devices...";
    }
    ucCPortNum = 0;
    while (ucCPortNum < ucNumPorts)
    {
        if (bOpenmode == true)
        {
            //Delete files
            SerialHandles[ucCPortNum].write(QString("AT+DEL \"").append(strRenameFilename).append("\" +\r").toUtf8());
            while (SerialHandles[ucCPortNum].waitForReadyRead(250));
        }
        SerialHandles[ucCPortNum].write(QString("AT+FOW \"").append(strRenameFilename).append("\"\r").toUtf8());
        while (SerialHandles[ucCPortNum].waitForReadyRead(250));
        ++ucCPortNum;
    }
    ucCPortNum = 0;
    while (ucCPortNum < ucNumPorts)
    {
        if (bDownloadCheck == true)
        {
            //Check response
            QByteArray TmpBA = SerialHandles[ucCPortNum].readAll();
            if (TmpBA.indexOf("\n00\r") == -1)
            {
                //Error
                int iRemove = TmpBA.indexOf("\n01\t");
                if (iRemove == -1)
                {
                    //Response not valid
                    qDebug() << "Error whilst opening file, unknown response:" << TmpBA << "from device #" << (ucCPortNum+1);
                    ClosePorts(ucNumPorts);
                    return ERROR_CODE_DOWNLOAD_FAIL;
                }
                iRemove += 4;
                TmpBA = TmpBA.mid(iRemove, 4);
                TmpBA.prepend("0x");
                qDebug() << "Error response during file opening:" << TmpBA << "from device #" << (ucCPortNum+1);
                ClosePorts(ucNumPorts);
                return ERROR_CODE_DOWNLOAD_ERROR;
            }
        }
        else
        {
            //Do not check response
            SerialHandles[ucCPortNum].readAll();
        }
        ++ucCPortNum;
    }

    //Download!
    if (ucExtraVerbose > 0)
    {
        qDebug() << "Initiating file downloads...";
    }
    unsigned int Size = baHexData.size();
    unsigned int Sent = 0;
    while (Sent < Size)
    {
        //Send this block
        QByteArray TmpBA = QString("AT+FWRH \"").append(baHexData.mid(Sent, intFWRHSize)).append("\"\r").toUtf8();
        ucCPortNum = 0;
        while (ucCPortNum < ucNumPorts)
        {
            SerialHandles[ucCPortNum].write(TmpBA);
            ++ucCPortNum;
        }
        ucCPortNum = 0;
        while (ucCPortNum < ucNumPorts)
        {
            SerialHandles[ucCPortNum].waitForBytesWritten(-1);
            while (SerialHandles[ucCPortNum].waitForReadyRead(PauseRate) == false);
            ++ucCPortNum;
        }

        ucCPortNum = 0;
        while (ucCPortNum < ucNumPorts)
        {
            if (bDownloadCheck == true)
            {
                //Check response
                TmpBA = SerialHandles[ucCPortNum].readAll();
                if (TmpBA.indexOf("\n00\r") == -1)
                {
                    //Error
                    int iRemove = TmpBA.indexOf("\n01\t");
                    if (iRemove == -1)
                    {
                        //Response not valid
                        qDebug() << "Error whilst downloading, unknown response:" << TmpBA << "from device #" << (ucCPortNum+1);
                        ClosePorts(ucNumPorts);
                        return ERROR_CODE_DOWNLOAD_FAIL;
                    }
                    iRemove += 4;
                    TmpBA = TmpBA.mid(iRemove, 4);
                    TmpBA.prepend("0x");
                    qDebug() << "Error response during download:" << TmpBA << "from device #" << (ucCPortNum+1);
                    ClosePorts(ucNumPorts);
                    return ERROR_CODE_DOWNLOAD_ERROR;
                }
            }
            else
            {
                //Do not check response
                SerialHandles[ucCPortNum].readAll();
            }
            ++ucCPortNum;
        }
        Sent += intFWRHSize;
        if (ucExtraVerbose > 1)
        {
            //Additional verbose debugging
            qDebug() << QString(TmpBA);
        }
    }

    //Close files
    if (ucExtraVerbose > 0)
    {
        qDebug() << "Closing files on devices...";
    }
    ucCPortNum = 0;
    while (ucCPortNum < ucNumPorts)
    {
        SerialHandles[ucCPortNum].write("AT+FCL\r");
        while (SerialHandles[ucCPortNum].waitForReadyRead(250));
        ++ucCPortNum;
    }
    ucCPortNum = 0;
    while (ucCPortNum < ucNumPorts)
    {
        if (bDownloadCheck == true)
        {
            //Check response
            QByteArray TmpBA = SerialHandles[ucCPortNum].readAll();
            if (TmpBA.indexOf("\n00\r") == -1)
            {
                //Error
                int iRemove = TmpBA.indexOf("\n01\t");
                if (iRemove == -1)
                {
                    //Response not valid
                    qDebug() << "Error whilst closing file handle, unknown response:" << TmpBA << "from device #" << (ucCPortNum+1);
                    ClosePorts(ucNumPorts);
                    return ERROR_CODE_FILECLOSE_FAIL;
                }
                iRemove += 4;
                TmpBA = TmpBA.mid(iRemove, 4);
                TmpBA.prepend("0x");
                qDebug() << "Error response whilst closing file handle:" << TmpBA << "from device #" << (ucCPortNum+1);
                ClosePorts(ucNumPorts);
                return ERROR_CODE_FILECLOSE_ERROR;
            }
        }
        else
        {
            //Do not check response
            SerialHandles[ucCPortNum].readAll();
        }
        ++ucCPortNum;
    }

    //File verification check
    if (ucVerifyCheck > 0)
    {
        if ((ucVerifyCheck & 2) == 2)
        {
            //File checksum
            if (ucExtraVerbose > 0)
            {
                qDebug() << "Testing module checksums against target:" << strChecksum;
            }

            ucCPortNum = 0;
            while (ucCPortNum < ucNumPorts)
            {
                SerialHandles[ucCPortNum].write(QString("ATI 0xC12C\r").toUtf8());
                while (SerialHandles[ucCPortNum].waitForReadyRead(1000));
                QByteArray RecDat = SerialHandles[ucCPortNum].readAll();
                int iRemove = RecDat.indexOf("\t49452\t");
                if (iRemove == -1)
                {
                    //Checksum not found
                    qDebug() << "Module checksum response not valid:" << RecDat << "from device #" << (ucCPortNum+1);
                    ClosePorts(ucNumPorts);
                    return ERROR_CODE_CHECKSUM_ERROR;
                }
                iRemove += 7;
                RecDat = RecDat.mid(iRemove, 4);
                RecDat.prepend("0x");
                if (RecDat != strChecksum.toUtf8())
                {
                    //File checksum mismatch
                    qDebug() << "Downloaded file checksum does not match:" << RecDat << "from device #" << (ucCPortNum+1);
                    ClosePorts(ucNumPorts);
                    return ERROR_CODE_CHECKSUM_FAIL;
                }
                ++ucCPortNum;
            }
        }

        if ((ucVerifyCheck & 1) == 1)
        {
            //Verify that files appear on at+dir listings
            if (ucExtraVerbose > 0)
            {
                qDebug() << "Checking files are visible in at+dir listing...";
            }
            ucCPortNum = 0;
            while (ucCPortNum < ucNumPorts)
            {
                SerialHandles[ucCPortNum].write(QString("AT+DIR\r").toUtf8());
                while (SerialHandles[ucCPortNum].waitForReadyRead(1000));
                QByteArray RecDat = SerialHandles[ucCPortNum].readAll();
                if (!(RecDat.indexOf(strRenameFilename) > 0))
                {
                    //File didn't download
                    qDebug() << "Downloaded file is missing from device #" << (ucCPortNum+1);
                    ClosePorts(ucNumPorts);
                    return ERROR_CODE_FILE_NOT_DOWNLOADED;
                }
                ++ucCPortNum;
            }
        }
    }

    //Wait for all bytes to be written
    ucCPortNum = 0;
    while (ucCPortNum < ucNumPorts)
    {
        while (SerialHandles[ucCPortNum].waitForBytesWritten(500));
        ++ucCPortNum;
    }

    if (bRunOnExit == true)
    {
        //Run file
        if (ucExtraVerbose > 0)
        {
            qDebug() << "Running applications...";
        }
        ucCPortNum = 0;
        rxExp3.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
        while (ucCPortNum < ucNumPorts)
        {
            SerialHandles[ucCPortNum].write(QString("at+run \"").append(strRenameFilename).append("\"\r").toUtf8());
            while (SerialHandles[ucCPortNum].waitForReadyRead(500));
            QByteArray RecDat = SerialHandles[ucCPortNum].readAll();
            QRegularExpressionMatch match3 = rxExp3.match(RecDat);
            if (match3.hasMatch())
            {
                //File didn't run successfully
                qDebug() << "Downloaded file failed to run on device #" << (ucCPortNum+1) << "- Error code:" << match3.captured(1);
                ClosePorts(ucNumPorts);
                return ERROR_CODE_FILE_NOT_DOWNLOADED;
            }
            ++ucCPortNum;
        }
    }

    //Short delay
#ifdef _WIN32
    Sleep(1000);
#else
    usleep(1000000);
#endif

    //Close ports
    if (ucExtraVerbose > 0)
    {
        qDebug() << "Closing serial ports...";
    }
    ClosePorts(ucNumPorts);

    //Output success
    qDebug() << "Multidownload complete!";

    return SUCCESS_CODE;
}

//=============================================================================
//=============================================================================
void
ClosePorts(
    unsigned char ucNumPorts
    )
{
    //Closes all open serial ports
    if (ucNumPorts == 0)
    {
        //No ports to close
        return;
    }
    ucNumPorts--;

    //Cycle through closing all ports
    while (ucNumPorts >= 0)
    {
        if (SerialHandles[ucNumPorts].isOpen() == true)
        {
            //Port is open - close
            SerialHandles[ucNumPorts].close();
        }

        if (ucNumPorts == 0)
        {
            //Ports closed
            return;
        }
        --ucNumPorts;
    }
}

#ifdef _WIN32
//=============================================================================
//=============================================================================
QString
AtiToXCompName(
    QString strAtiResp
    )
{
    //Function to extract XCompiler name from ATI response
    if (strAtiResp.length() > MaxDevNameSize)
    {
        //Shorten device name
        strAtiResp = strAtiResp.left(MaxDevNameSize);
    }

    //Only allow a-z, A-Z, 0-9 and underscores in device name
    int iI = 0;
    while (iI < strAtiResp.length())
    {
        //Check each character
        char cTmpC = strAtiResp.at(iI).toLatin1();
        if (!(cTmpC > 47 && cTmpC < 58) && !(cTmpC > 64 && cTmpC < 91) && !(cTmpC > 96 && cTmpC < 123) && cTmpC != 95)
        {
            //Replace non-alphanumeric character with an underscore
            strAtiResp[iI] = '_';
        }
        ++iI;
    }
    return strAtiResp;
}
#endif

//=============================================================================
//=============================================================================
unsigned short
ByteChecksum(
    unsigned short usCrcVal,
    unsigned char ucChar
    )
{
    //Calculates checksum of a byte
    unsigned char i;
    unsigned short usData;

    for(i=0, usData=(unsigned int)0xff & ucChar++; i < 8; i++, usData >>= 1)
    {
        if ((usCrcVal & 0x0001) ^ (usData & 0x0001))
        {
            usCrcVal = (usCrcVal >> 1) ^ 0x8408;
        }
        else
        {
            usCrcVal >>= 1;
        }
    }
    usCrcVal = ~usCrcVal;
    usData = usCrcVal;
    usCrcVal = (usCrcVal << 8) | (usData >> 8 & 0xff);

    return usCrcVal;
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
