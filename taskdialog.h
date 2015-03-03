#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <QDialog>
#include <QLocale>
#include <QVector>
namespace Ui {
class TaskDialog;
}

class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    TaskDialog(QWidget *parent = 0);
    ~TaskDialog();
    QString dateTimeFormat=QLocale::system().dateTimeFormat(QLocale::ShortFormat);
signals:
    void taskDialogeFinished(bool btnOkClicked);
private slots:

    void on_chkHasTime_toggled(bool checked);
    void on_btnOk_clicked();

    void on_btnCancel_clicked();

public:
    Ui::TaskDialog *ui;
};

#endif // TASKDIALOG_H
