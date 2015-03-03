#ifndef SIDEWIDJET_H
#define SIDEWIDJET_H

#include <QWidget>

namespace Ui {
class SideWidjet;
}

class SideWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SideWidget(QWidget *parent = 0);
    void leaveEvent(QEvent *e);
    ~SideWidget();

    Ui::SideWidjet *ui;
signals:
    void mouseLeaved();
private slots:
    void on_btnExit_clicked();
    void on_btnDelete_clicked();
};

#endif // SIDEWIDJET_H
