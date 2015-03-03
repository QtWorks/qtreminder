#include "sidewidget.h"
#include "ui_sidewidget.h"

SideWidget::SideWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SideWidjet)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Tool);
}

SideWidget::~SideWidget()
{
    delete ui;
}

void SideWidget::on_btnExit_clicked()
{
//    close();
}

void SideWidget::on_btnDelete_clicked()
{

}

void SideWidget::leaveEvent(QEvent *e)
{
    Q_UNUSED(e);
    emit mouseLeaved();
}
