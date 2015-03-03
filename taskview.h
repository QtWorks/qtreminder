#ifndef TASKVIEW_H
#define TASKVIEW_H

#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QLocale>
#include <QDateTime>
#include <QSystemTrayIcon>
#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QMouseEvent>
#include <sidewidget.h>
#include <taskdialog.h>
#include <ui_sidewidget.h>
#include <tasknotifier.h>
#include <QDebug>

//program constants
#define HEIGHT_RATIO 4
#define WIDTH_RATIO 5
#define SWA_DURATION 200 //sideWidgetAnimation duration.
#define BWA_DURATION 200 //browsingWidgetAnimation duration.
#define ER_SHIFT 50 //shifting of the end point of the animation.
#define TA_INTERVAL 3500 //timeout interval of taskAnimator timer.


namespace Ui {
class TaskView;
}

class TaskView : public QWidget
{
    Q_OBJECT
private:

    //program status flags
    //-this flag is raised when it's time to show sideControls widget:
    bool showSideControls;
    //-this flag is raised when the sideControls widget is already shown:
    bool sideControlsVisible;
    bool paused;
    bool editing;
    
public:
    explicit TaskView(QWidget *parent = 0);
    ~TaskView();
    
    struct Task //task details
    {
        QString text; //task's text.
        bool hasDateTime; //does the task have time assigned to it?.
        QDateTime dateTime; //task's date and time.
        bool dateTimePassed; //true iff the date\time assigned to the task has passed.
        bool notificationShown; //true iff notification is shown for the task.
    };
    
    SideWidget *sideControls;
    TaskDialog *dlgTask;
    TaskNotifier *notification;
    QSystemTrayIcon *sti;
    QPropertyAnimation *sideControlsAnimation;//position animation
    QPropertyAnimation *sideControlsAnimation2;//opacity
    QPropertyAnimation *navigationAnimation;
    QString dateTimeFormat;//main program DateTime format...
    int currentTaskI;//current (currently viewed) task's index


    QVector<Task> tasks;//tasks list.
    //monitors the mouse position and show/hide widgets according to it:

    QTimer *taskNavigator;//animation and navigation timer.
    const QString strFileName = "tasx.tsx";
    const QString strXmlFileName = "tasx.xml";
    bool saveChangesToFile(QString fileName);
    bool saveChangesToXmlFile(QString fileName);
    void loadTasksFromFile(QString fileName);
    void loadTasksFromXmlFile(QString fileName);
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);

private slots:
    void on_btnNewClick();
    void on_btnEditClick();
    void on_btnDeleteClick();

    void closeEvent(QCloseEvent *e);
    void taskAnimation();
    void viewCurrentTask();
    void on_btnPauseContinue_clicked();
    void on_btnNext_clicked();
    void on_btnPrev_clicked();
    void onTaskDialogFinished(bool btnOkClicked);
    void notificationFinished();
    void notificationFinishedDel();
    void leaved();
    void trayIconActivated(QSystemTrayIcon::ActivationReason reson);

private:
    Ui::TaskView *ui;
};

#endif // TASKVIEW_H
