#ifndef TIMELINELIST_H
#define TIMELINELIST_H

#include <QWidget>
#include <DListWidget>
#include <QLabel>
#include "timelineitem.h"
#include <QDebug>
#include "controller/signalmanager.h"
#include "application.h"

DWIDGET_USE_NAMESPACE

class TimelineList : public DListWidget
{
    Q_OBJECT
public:
    explicit TimelineList(QWidget *parent = nullptr);

    void addItemForWidget(QListWidgetItem *aitem);
protected:
//    void wheelEvent(QWheelEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *e);
signals:
    void sigNewTime(QString date,QString num,int index);
    void sigDelTime();
    void sigMoveTime(int y);

public slots:
private:
    bool has;
    QList<int> yList;
};

#endif // TIMELINELIST_H