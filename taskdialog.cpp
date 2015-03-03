#include "taskdialog.h"
#include "ui_taskdialog.h"
#include <QMessageBox>

TaskDialog::TaskDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TaskDialog)
{
    //setting up user interface:
    ui->setupUi(this);
    //setWindowFlags(Qt::CustomizeWindowHint);
    ui->dateTimeEdit->setDisplayFormat(dateTimeFormat);
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
}

TaskDialog::~TaskDialog()
{
    delete ui;
}

void TaskDialog::on_chkHasTime_toggled(bool checked)
{
    ui->dateTimeEdit->setVisible(checked);
}

void TaskDialog::on_btnOk_clicked()
{
    //do not accept empty text:
    if(ui->plnTask->toPlainText().isEmpty())
    {
        QMessageBox::warning(this,"What? o_O","Empty task ?!!"
                                              "\nAre you freaking kidding me!! -_-");
    }
    else
    {
        emit taskDialogeFinished(1);
    }
}

//Cancel button is clicked, then Do nothing to the task:
void TaskDialog::on_btnCancel_clicked()
{
    emit taskDialogeFinished(0);
}


