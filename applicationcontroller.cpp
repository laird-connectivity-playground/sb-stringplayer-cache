/******************************************************************************
** Copyright (C) 2017 Laird Connectivity
**
** Project: sB-StringPlayer-Cache
**
** Module: applicationcontroller.cpp
**
** Notes:
**
** License:
**
*******************************************************************************/

/******************************************************************************/
// Include Files
/******************************************************************************/
#include "applicationcontroller.h"

ApplicationController::ApplicationController(QObject *parent) : QObject(parent)
{
    //Constructor
    RuntimeConfigPresent = 0;
    RunMode = 0; //0 = normal (XCompile if missing/updated), 1 = clear all files then XCompile all
    RunXCompiler = "";
    RunSuffix = "";
    RunPlayerise = "";
    RunInputFolder = "";
    RunOutputFolder = "";
    Threads = 4; //Default number of threads (processes to run in parallel)
    ProcessThreads = 0;
    SbFiles.clear();
    UwcFiles.clear();
}

ApplicationController::~ApplicationController()
{
    //Destructor
}

void ApplicationController::Run(int argc, char *argv[])
{
    //Output application name, version and verbosity level
    qDebug() << "\r\n|---------------------|";
    qDebug() << "|sB-StringPlayer-Cache|";
    qDebug() << "|" << APPVERSION << "              |";
    qDebug() << "| Verbosity:" << VERBOSE << "       |";
    qDebug() << "|---------------------|\r\n";

    //Check command line and parse arguments
    int CArg = 1; //First argument is executed filename, skip it
    while (CArg < argc)
    {
        //Check argument
        if (QString(argv[CArg]).indexOf("=") >= 0)
        {
            //Parse the command line
            QString ThisArg = QString(argv[CArg]).left(QString(argv[CArg]).indexOf("=")).toLower();
            QString ThisVal = QString(argv[CArg]).right(QString(argv[CArg]).length() - QString(argv[CArg]).indexOf("=") - 1);

            if (ThisArg == strMode)
            {
                //Operational mode
                RuntimeConfigPresent |= 0b1;
                RunMode = ThisVal.toUInt();
            }
            else if (ThisArg == strCompiler)
            {
                //XCompiler name and path
                RuntimeConfigPresent |= 0b10;
                RunXCompiler = ThisVal;
            }
            else if (ThisArg == strPlayerise)
            {
                //Playerise name and path
                RuntimeConfigPresent |= 0b100;
                RunPlayerise = ThisVal;
            }
            else if (ThisArg == strInputFolder)
            {
                //Input folder
                RuntimeConfigPresent |= 0b1000;
                RunInputFolder = ThisVal;
                if (RunInputFolder.length() > 0 && RunInputFolder.right(1) != "\\")
                {
                    //Append trailing slash
                    RunInputFolder.append("\\");
                }
            }
            else if (ThisArg == strOutputFolder)
            {
                //Output folder
                RuntimeConfigPresent |= 0b10000;
                RunOutputFolder = ThisVal;
                if (RunOutputFolder.length() > 0 && RunOutputFolder.right(1) != "\\")
                {
                    //Append trailing slash
                    RunOutputFolder.append("\\");
                }
            }
            else if (ThisArg == strSuffix)
            {
                //Compiled application suffix
                RuntimeConfigPresent |= 0b100000;
                RunSuffix = QString("-").append(ThisVal);
            }
            else if (ThisArg == strThreads)
            {
                //Number of parallel threads
                RuntimeConfigPresent |= 0b1000000;
                Threads = ThisVal.toUInt();
            }
        }

        //Next argument
        ++CArg;
    }

    //Setup quit timer
    tmrEndQuit.setSingleShot(true);
    connect(&tmrEndQuit, SIGNAL(timeout()), this, SLOT(DoQuit()));
    tmrEndQuit.setInterval(50);

    //Check if sufficient data is present to run
    if (RuntimeConfigPresent == 0)
    {
        //Insufficient data provided to run
        qDebug() << "No command line arguments found, exiting...";
        tmrEndQuit.start();
    }
    else if (RunMode != 0 && RunMode != 1)
    {
        //Invalid run mode
        qDebug() << "Specified run mode is not valid, must be 0 (normal operation) or 1 (clear output folders before processing). Exiting...";
        tmrEndQuit.start();
    }
    else if ((RuntimeConfigPresent & 0b10) == 0)
    {
        //XCompiler not provided
        qDebug() << "XCompiler not provided, exiting...";
        tmrEndQuit.start();
    }
    else if (!QFile::exists(RunXCompiler))
    {
        //Invalid XCompiler provided
        qDebug() << "Specified XCompiler does not exist, exiting...";
        tmrEndQuit.start();
    }
    else if ((RuntimeConfigPresent & 0b100) == 0)
    {
        //Playerise not provided
        qDebug() << "Playerise not provided, exiting...";
        tmrEndQuit.start();
    }
    else if (!QFile::exists(RunPlayerise))
    {
        //Invalid playerise provided
        qDebug() << "Specified playerise does not exist, exiting...";
        tmrEndQuit.start();
    }
    else if (!QFile::exists(RunInputFolder))
    {
        //Input folder does not exist
        qDebug() << "Provided input folder does not exist, exiting...";
        tmrEndQuit.start();
    }
    else
    {
        //Everything seems good
        qDebug() << "Parameters OK, processing...";

        //Check if output directory exists and create it if not
        QDir dirFolders(RunOutputFolder);
        dirFolders.setSorting(QDir::Unsorted);
        dirFolders.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);
        if (!dirFolders.exists())
        {
            //Create output directory
            dirFolders.mkdir(".");
        }

        //XCompiler modification date
        QDateTime dtXCompileModified = QFileInfo(RunXCompiler).lastModified();

        //List smartBASIC files in the input folder
        dirFolders.setPath(RunInputFolder);
        dirFolders.setNameFilters(QStringList() << "*.sb");
        QFileInfoList sBInfoList  = dirFolders.entryInfoList();
        int i = 0;

        //Process each file
        while (i < sBInfoList.length())
        {
            //Check file info
            QFileInfo UwcFile(QString(RunInputFolder).append(sBInfoList.at(i).fileName().left(sBInfoList.at(i).fileName().lastIndexOf(".")).append(RunSuffix).append(".uwc")));
            //qDebug() << sBInfoList.at(i).absoluteFilePath() << ", " << sBInfoList.at(i).lastModified() << ", " << sBInfoList.at(i).fileName().left(sBInfoList.at(i).fileName().indexOf(".")).append(".uwc") << ", " << UwcFile.absoluteFilePath() << ", " << UwcFile.lastModified();

            if (RunMode == 1 || !UwcFile.exists() || sBInfoList.at(i).lastModified() >= UwcFile.lastModified() || dtXCompileModified >= UwcFile.lastModified())
            {
                //sB file needs to be (re)XCompiled
                SbFiles << sBInfoList.at(i).fileName();
                UwcFiles << sBInfoList.at(i).fileName().left(sBInfoList.at(i).fileName().lastIndexOf(".")).append(RunSuffix).append(".uwc");

                if (RunMode == 1 && UwcFile.exists())
                {
                    QFile::remove(QString(RunInputFolder).append(sBInfoList.at(i).fileName().left(sBInfoList.at(i).fileName().lastIndexOf(".")).append(RunSuffix).append(".uwc")));
                }
#if VERBOSE > 0
                qDebug() << "Compile:" << sBInfoList.at(i).fileName();
#endif
            }
            ++i;
        }

        if (SbFiles.count() == 0)
        {
            //No files to process, quit
            tmrEndQuit.start();
        }
        else
        {

            //Setup processes
            ProcessThreads = new QProcess[Threads];
            qint8 CThread = 0;
            while (CThread < Threads)
            {
                connect(&ProcessThreads[CThread], SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(process_finished(int, QProcess::ExitStatus)));
                ++CThread;
            }

            //Run first set of processes for XCompilation in parallel
            Running = 0;
            CThread = 0;
            while (CThread < Threads && SbFiles.count() > 0)
            {
                QString tmpUwc = SbFiles.last();
                tmpUwc = tmpUwc.left(tmpUwc.length()-3).append(RunSuffix).append(".uwc");
                ProcessThreads[CThread].start(QString(RunXCompiler), QStringList(QString(RunInputFolder).append(SbFiles.last())) << "/O" << tmpUwc);
                ++Running;
#if VERBOSE > 1
                qDebug() << "XRun: " << QString(RunXCompiler) << " - " << QString(RunInputFolder).append(SbFiles.last()) << " /O" << tmpUwc;
#endif
                SbFiles.removeLast();
                ++CThread;
            }

            qDebug() << "Started Parallel XCompilation - sB files:" << (SbFiles.count()+Running) << "Max threads:" << Threads << "Running:" << Running;
        }
    }
}

void ApplicationController::process_finished(
    int intCode,
#if VERBOSE > 2
    QProcess::ExitStatus esStatus
#else
    QProcess::ExitStatus
#endif
    )
{
    //
#if VERBOSE > 2
    qDebug() << "Exit Status: " << esStatus << "Exit Code: " << intCode;
#endif
    QObject *Sender = QObject::sender();
    QProcess *Proc = (QProcess*)Sender;

    if (Proc->program() == RunXCompiler && intCode != 0)
    {
        //Application failed to compile, if there is a .sub file then remove it as it's no longer valid
        QString tmpSub(Proc->arguments().at(2));
        tmpSub = tmpSub.left(tmpSub.length()-3).append("sub");
        tmpSub.prepend(RunOutputFolder);
        if (QFile::exists(tmpSub))
        {
#if VERBOSE > 0
            qDebug() << "Rem old sub: " << tmpSub;
#endif
            QFile::remove(tmpSub);
        }
    }

    if (SbFiles.count() == 0)
    {
        if (UwcFiles.count() == 0)
        {
            //Nothing left
            --Running;
            if (Running == 0)
            {
                tmrEndQuit.start();
            }
        }
        else
        {
            //Check if file exists
            while (UwcFiles.count() > 0 && !QFile::exists(QString(RunInputFolder).append(UwcFiles.last())))
            {
                //Skip as file has failed to XCompile
                UwcFiles.removeLast();
            }

            if (UwcFiles.count() == 0)
            {
                //Nothing left
                --Running;
                if (Running == 0)
                {
                    tmrEndQuit.start();
                }
            }
            else
            {
                //Process
                QString tmpSub(UwcFiles.last());
                tmpSub = tmpSub.left(tmpSub.length()-3).append("sub");
                tmpSub.prepend(RunOutputFolder);
                QString tmpFName(UwcFiles.last());
                tmpFName = tmpFName.left(tmpFName.indexOf("."));
#if VERBOSE > 1
                qDebug() << "PRun: " << QString(RunPlayerise) << QString(RunInputFolder).append(UwcFiles.last()) << tmpSub << "datafile" << "/H" << "/F" << tmpFName;
#endif
                Proc->start(QString(RunPlayerise), QStringList(QString(RunInputFolder).append(UwcFiles.last())) << tmpSub << "datafile" << "/H" << "/F" << tmpFName);
                UwcFiles.removeLast();
            }
        }
    }
    else
    {
        //Compile next sB file
        QString tmpUwc = SbFiles.last();
        tmpUwc = tmpUwc.left(tmpUwc.length()-3).append(RunSuffix).append(".uwc");
#if VERBOSE > 1
        qDebug() << "XRun: " << QString(RunXCompiler) << " - " << QString(RunInputFolder).append(SbFiles.last()) << " /O" << tmpUwc;
#endif
        Proc->start(QString(RunXCompiler), QStringList(QString(RunInputFolder).append(SbFiles.last())) << "/O" << tmpUwc);
        SbFiles.removeLast();
    }
}

void ApplicationController::DoQuit(
    )
{
    //Clear up and quit the application
    if (ProcessThreads)
    {
        //Delete threads
        while (Threads > 0)
        {
            --Threads;
            disconnect(&ProcessThreads[Threads], SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(process_finished(int, QProcess::ExitStatus)));
        }
        delete[] ProcessThreads;
        ProcessThreads = 0;
    }

    //Quit application
    QCoreApplication::quit();
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
