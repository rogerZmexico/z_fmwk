/****************************************************************************
** file:        mywidgetfactory.h
** project:  zserver
** author:   giovanniangeli@tbe.it
** created at: Sat, 27 May 2006 21:38:46 +0000; by addAClass.py ver. 0.2
** license: GPL
****************************************************************************/
#ifndef MYWIDGETFACTORY_H
#define MYWIDGETFACTORY_H

#include <uilib/qwidgetfactory.h>

class MyWidgetFactory : public QWidgetFactory
{

public:

    MyWidgetFactory();
    ~MyWidgetFactory();

    QWidget *createWidget( const QString &className, QWidget *parent, const char *name ) const;
    QStringList widgets();
    bool dumpWidgets();

public slots:

};
#endif // MYWIDGETFACTORY_H
