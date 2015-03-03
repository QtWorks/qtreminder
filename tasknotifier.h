#ifndef TASKNOTIFIER_H
#define TASKNOTIFIER_H

#include <QWidget>

class TaskView;

namespace Ui {
class TaskNotifier;
}

class TaskNotifier : public QWidget
{
    Q_OBJECT

public:
    explicit TaskNotifier(QString txtTask, int h, int task_index,QString taskTime, QWidget *parent = 0);
    int taskIndex;
    ~TaskNotifier();

private slots:
    void on_btnOk_clicked();

public:
    Ui::TaskNotifier *ui;
};

#endif // TASKNOTIFIER_H
