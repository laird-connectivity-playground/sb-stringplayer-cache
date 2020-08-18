/******************************************************************************
** Copyright (C) 2017 Laird Connectivity
**
** Project: sB-StringPlayer-Cache
**
** Module: main.cpp
**
** Notes:
**
** License:
**
*******************************************************************************/

/******************************************************************************/
// Include Files
/******************************************************************************/
#include <QCoreApplication>
#include "applicationcontroller.h"

//=============================================================================
//=============================================================================
int
main(
    int argc,
    char *argv[]
    )
{
    //Run application
    QCoreApplication a(argc, argv);
    ApplicationController AppCntrl;
    AppCntrl.Run(argc, argv);
    return a.exec();
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
