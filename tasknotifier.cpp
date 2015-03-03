#include "tasknotifier.h"
#include "ui_tasknotifier.h"
#include "taskview.h"

TaskNotifier::TaskNotifier(QString txtTask, int h, int task_index,
                           QString taskTime, QWidget *parent) :
    QWidget(parent,Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint|Qt::Dialog),
    ui(new Ui::TaskNotifier)
{
    taskIndex=task_index;
    ui->setupUi(this);

    QPropertyAnimation *anim=new QPropertyAnimation(this,"pos");
    QDesktopWidget dw;
    setFixedSize(dw.width()/3.5,h);

    QPoint sV(dw.width(),dw.height()-height());
    anim->setStartValue(sV);
    sV.setX(dw.width()-width());
    anim->setEndValue(sV);
    anim->setDuration(350);
    ui->txtTask->setText(txtTask);
    ui->lblNotify->setText(QString("Timed on: %1").arg(taskTime));
    anim->start();
}

TaskNotifier::~TaskNotifier()
{
    delete ui;
}

void TaskNotifier::on_btnOk_clicked()
{
    close();
}
