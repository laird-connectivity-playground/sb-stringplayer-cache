/******************************************************************************
** Copyright (C) 2017 Laird Connectivity
**
** Project: sB-StringPlayer-Cache
**
** Module: applicationcontroller.h
**
** Notes:
**
** License:
**
*******************************************************************************/
#ifndef APPLICATIONCONTROLLER_H
#define APPLICATIONCONTROLLER_H

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QObject>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QProcess>
#include <QCoreApplication>
#include <QTimer>

/******************************************************************************/
// Defines
/******************************************************************************/
#define VERBOSE         0   //Controls output: 0 = normal, 1 = list files + deletions, 2 = list executed files, 3 = show executed file exit status/code
#define APPVERSION "v0.93"  //Version

/******************************************************************************/
// Constants
/******************************************************************************/
const QString strMode = "mode";            //Command line argument for setting mode
const QString strCompiler = "xcompiler";   //Command line argument for XCompiler path and filename
const QString strPlayerise = "playerise";  //Command line argument for playerise path and filename
const QString strInputFolder = "input";    //Command line argument for input filter path
const QString strOutputFolder = "output";  //Command line argument for output folder path
const QString strSuffix = "suffix";        //Command line argument for output suffix
const QString strThreads = "threads";      //Command line argument for number of parallel threads

/******************************************************************************/
// Class definitions
/******************************************************************************/
class ApplicationController : public QObject
{
    Q_OBJECT
public:
    explicit ApplicationController(QObject *parent = nullptr);
    ~ApplicationController();
    void
    Run(
        int argc,
        char *argv[]
        );

signals:

public slots:
    void
    process_finished(
        int intCode,
#if VERBOSE > 2
        QProcess::ExitStatus esStatus
#else
        QProcess::ExitStatus
#endif
        );
    void
    DoQuit(
        );

private:
    quint8 RuntimeConfigPresent;
    quint8 RunMode;
    QString RunXCompiler;
    QString RunPlayerise;
    QString RunInputFolder;
    QString RunTempFolder;
    QString RunOutputFolder;
    QString RunSuffix;
    qint8 Threads;
    qint8 Running;
    QProcess *ProcessThreads;
    QTimer tmrEndQuit;
    QStringList SbFiles;
    QStringList UwcFiles;
};

#endif // APPLICATIONCONTROLLER_H

/******************************************************************************/
// END OF FILE
/******************************************************************************/
