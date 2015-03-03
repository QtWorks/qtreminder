#include "taskview.h"
#include "ui_taskview.h"
#include <ui_taskdialog.h>
#include <ui_tasknotifier.h>
#include <QCloseEvent>
#include <QMessageBox>
#include <QApplication>

TaskView::TaskView(QWidget *parent) ://constructor ....
    QWidget(parent/*, Qt::Dialog*/),
    ui(new Ui::TaskView)
{
    ui->setupUi(this); //setting up the user interface.
    dlgTask = 0;
    //resize the widget according to screen's dimentions:
    QDesktopWidget mainScreen;
    setFixedHeight(mainScreen.height()/HEIGHT_RATIO);
    setFixedWidth(mainScreen.width()/WIDTH_RATIO);
    
    //move the widget to the upper right corner of the screen:
    move(mainScreen.width()- width(),0);
    
    //assign all member data:
    //-unset all program flag bits and set SIDE_WIDGET_SHOWN flag bit:
    sideControlsVisible = 1;
    
    sideControls = new SideWidget(0);
//    sideControls->setWindowOpacity(windowOpacity());
    //-setup geometry animations:
    navigationAnimation = new QPropertyAnimation(ui->tasksNavigation, "pos");
    sideControlsAnimation = new QPropertyAnimation(sideControls, "geometry");
    sideControlsAnimation2 = new QPropertyAnimation(sideControls, "windowOpacity");

    taskNavigator = new QTimer(this);
    //-get machine's DateTime format:
    dateTimeFormat=QLocale::system().dateTimeFormat(QLocale::ShortFormat);
    
    //connect slots to signals:
    connect(sideControls->ui->btnExit,SIGNAL(clicked()),
            this,SLOT(close()));
    connect(sideControls->ui->btnNew,SIGNAL(clicked()),
            this,SLOT(on_btnNewClick()));
    connect(sideControls->ui->btnEdit,SIGNAL(clicked()),
            this,SLOT(on_btnEditClick()));
    connect(sideControls->ui->btnDelete,SIGNAL(clicked()),
            this,SLOT(on_btnDeleteClick()));
    connect(sideControls,SIGNAL(mouseLeaved()),
            this,SLOT(leaved()));
    connect(taskNavigator,SIGNAL(timeout()),
            this,SLOT(taskAnimation()));
    
    //configure animations:
    sideControlsAnimation->setDuration(SWA_DURATION);
    sideControlsAnimation2->setDuration(SWA_DURATION+1000);
    sideControlsAnimation2->setStartValue(0);
    sideControlsAnimation2->setEndValue(windowOpacity());
    navigationAnimation->setDuration(BWA_DURATION);


    setWindowFlags(Qt::FramelessWindowHint);
    paused = 1 ;//set the paused flag bit to excute
    //the on_btnPauseContinue_clicked() function with PAUSED flag bit set:
    ui->btnPauseContinue->click();
    
    //show every task for TA_INTERVAL seconds:
    taskNavigator->start(TA_INTERVAL);
    
    //loadTasksFromFile(strFileName);
    loadTasksFromXmlFile(strXmlFileName);
    currentTaskI = 0;
    viewCurrentTask();
    leaveEvent(0);
    setMouseTracking(1);

    //tray icon
    sti = new QSystemTrayIcon(QIcon(":/new/prefix1/Icon2.ico"),this);
    connect(sti,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
    if(QSystemTrayIcon::isSystemTrayAvailable()){sti->show();}
}

//destructor
TaskView::~TaskView()
{
    qDebug()<<"Destructor...";
    delete ui;
}

void TaskView::trayIconActivated(QSystemTrayIcon::ActivationReason reson)
{
    if(reson == QSystemTrayIcon::DoubleClick
            || reson == QSystemTrayIcon::Trigger)
    {
        raise();
        activateWindow();
    }
}
void TaskView::enterEvent(QEvent *e)
{
    Q_UNUSED(e);// unused parameter
    QDesktopWidget mainScreen;

    setFixedHeight(mainScreen.height()/HEIGHT_RATIO);
    setFixedWidth(mainScreen.width()/WIDTH_RATIO);

    //move the widget to the upper right corner of the screen:
    move(mainScreen.width()- width(),0);

    if(!sideControlsVisible)
    {
        //show Edit and Delete push buttons iff the tasks list has
        //one task, or if the program is paused (PAUSED flag is raised):
        sideControls->ui->btnEdit->setVisible(tasks.count()==1 || paused);
        sideControls->ui->btnDelete->setVisible(tasks.count()==1 || paused);
        //-sideControls' end geometry value:
        QRect swcEndRect(x()-sideControls->width(),y(),
                         sideControls->width(),
                         height());

        //-same geometry at end, but shifted END_RECT_SHIFT pixels to the right
        QRect swcStartRect(swcEndRect);
        swcStartRect.moveRight(swcEndRect.x() + sideControls->width() + ER_SHIFT);
        sideControlsAnimation->setStartValue(swcStartRect);
        sideControlsAnimation->setEndValue(swcEndRect);

        sideControls->show();
        sideControls->setMouseTracking(1);
        sideControlsAnimation->start();
        sideControlsAnimation2->start();

        //show & animate the tasksNavigation widget:
        if(tasks.count() > 1)
        {
            ui->tasksNavigation->show();

            //            QPoint bwEndPoint(QPoint(x(),y()+height()-ui->tasksNavigation->height()));
            //            navigationAnimation->setEndValue(bwEndPoint);
            //            bwEndPoint.setY(bwEndPoint.y()-ER_SHIFT);
            //            navigationAnimation->setStartValue(bwEndPoint);
            //            navigationAnimation->start();
        }
        //raise the sideControlsVisible flag after the side widget is shown
        sideControlsVisible =1;

        raise();
    }
}
void TaskView::leaved()
{
    if(sideControlsVisible && !geometry().contains(cursor().pos()))
    {
        sideControlsVisible=0;
        sideControls->hide();
        ui->tasksNavigation->hide();
    }
}
void TaskView::leaveEvent(QEvent *e)
{
    Q_UNUSED(e);// unused parameter
    if(!sideControls->geometry().contains(cursor().pos()))
    {
        showSideControls = 0;
        if(sideControlsVisible)
        {
            sideControlsVisible=0;
            sideControls->hide();
            ui->tasksNavigation->hide();
        }
    }
}

void TaskView::closeEvent(QCloseEvent *e)
{
    qDebug()<<"Exiting...";
    sti->hide();
    //close all other widgets, save changes then close.
    sideControls->close();
    //saveChangesToFile(strFileName);
    saveChangesToXmlFile(strXmlFileName);
    e->accept();
}

//add new task...
void TaskView::on_btnNewClick()
{
    hide();
    sideControls->hide();
    editing = 0;
    
    //configure a new task dialog and show it:
    dlgTask = new TaskDialog(0);
    connect(dlgTask,SIGNAL(taskDialogeFinished(bool)),
            this,SLOT(onTaskDialogFinished(bool)));
    dlgTask->setWindowTitle("New Task");
    dlgTask->ui->plnTask->clear();
    dlgTask->ui->dateTimeEdit->setDisplayFormat(dateTimeFormat);
    dlgTask->ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    dlgTask->ui->chkHasTime->setChecked(0);
    emit dlgTask->ui->chkHasTime->toggled(0);

    dlgTask->show();
    dlgTask->activateWindow();
    dlgTask->ui->plnTask->setFocus();
}

void TaskView::on_btnEditClick()
{
    hide();
    sideControls->hide();
    editing=1;
    //configure the new task dialog and show it:
    dlgTask = new TaskDialog(0);
    connect(dlgTask,SIGNAL(taskDialogeFinished(bool)),
            this,SLOT(onTaskDialogFinished(bool)));
    dlgTask->setWindowTitle("Edit Task");
    dlgTask->ui->plnTask->setPlainText(tasks[currentTaskI].text);
    dlgTask->ui->dateTimeEdit->setDisplayFormat(dateTimeFormat);

    QDateTime currentDateTime(QDateTime::currentDateTime());
    QDateTime toDoDateTime(tasks[currentTaskI].dateTime);

    if(toDoDateTime.isValid() && toDoDateTime>currentDateTime)
    {
        dlgTask->ui->dateTimeEdit->setDateTime(tasks[currentTaskI].dateTime);
        dlgTask->ui->chkHasTime->setChecked(1);
        emit dlgTask->ui->chkHasTime->toggled(1);
    }
    else
    {
        dlgTask->ui->dateTimeEdit->setDateTime(currentDateTime);
        dlgTask->ui->chkHasTime->setChecked(0);
        emit dlgTask->ui->chkHasTime->toggled(0);
    }

    dlgTask->show();
    dlgTask->activateWindow();

    QTextCursor tc(dlgTask->ui->plnTask->textCursor());
    tc.movePosition(QTextCursor::End);
    dlgTask->ui->plnTask->setTextCursor(tc);
    dlgTask->ui->plnTask->setFocus();
}

void TaskView::onTaskDialogFinished(bool btnOkClicked)
{
    if(btnOkClicked && !editing) // new task
    {
        Task nTask;
        //update the task with the data:
        nTask.text = dlgTask->ui->plnTask->toPlainText();
        nTask.hasDateTime = dlgTask->ui->chkHasTime->isChecked();

        if(nTask.hasDateTime)
        {
            nTask.dateTime = dlgTask->ui->dateTimeEdit->dateTime();
            //if task's DateTime is before the current DateTime, then set
            //the dateTimePassed flag (dateTimePassed=1) and vice versa:
            nTask.dateTimePassed = nTask.dateTime < QDateTime::currentDateTime();
            nTask.notificationShown = nTask.dateTimePassed;
        }
        else
        {
            nTask.dateTime = QDateTime::fromString(0);
        }
        tasks.append(nTask);
        currentTaskI = tasks.count()-1;
        viewCurrentTask();
    }
    if(btnOkClicked && editing) // edit the current task.
    {
        //update the task with the data:
        tasks[currentTaskI].text = dlgTask->ui->plnTask->toPlainText();
        tasks[currentTaskI].hasDateTime = dlgTask->ui->chkHasTime->isChecked();

        if(tasks[currentTaskI].hasDateTime)
        {
            tasks[currentTaskI].dateTime = dlgTask->ui->dateTimeEdit->dateTime();
            //if task's DateTime is before the current DateTime, then set
            //the dateTimePassed flag (dateTimePassed=1) and vice versa:
            tasks[currentTaskI].dateTimePassed = tasks[currentTaskI].dateTime < QDateTime::currentDateTime();
            tasks[currentTaskI].notificationShown = tasks[currentTaskI].dateTimePassed;
        }
        else
        {
            tasks[currentTaskI].dateTime = QDateTime::fromString(0);
        }
        viewCurrentTask();
    }
    //saveChangesToFile(strFileName);
    saveChangesToXmlFile(strXmlFileName);
    show();
    dlgTask->close();
    delete dlgTask;
}

void TaskView::taskAnimation()
{
    if(!tasks.isEmpty())
    {
        //update the current task index:
        currentTaskI ++;
        if(currentTaskI > (tasks.size()-1))
        {
            currentTaskI=0;
        }
        viewCurrentTask();
    }
}

//toggles the PAUSED flag bit
void TaskView::on_btnPauseContinue_clicked()
{
    //if PAUSED flag is raised, show edit and delete buttons
    if(paused)
    {
        taskNavigator->start();
        paused=0;
        ui->btnPauseContinue->setText("Pause");
    }
    else
    {
        taskNavigator->stop();
        paused=1;
        ui->btnPauseContinue->setText("Continue");
    }

    ui->btnNext->setVisible(paused);
    ui->btnPrev->setVisible(paused);
    sideControls->ui->btnEdit->setVisible(tasks.count()==1 || paused);
    sideControls->ui->btnDelete->setVisible(tasks.count()==1 || paused);
    //Do not wait for taskAnimator timer to timeout to view the task:
    viewCurrentTask();

}

//views the task indexed by currentTaskI and che
void TaskView::viewCurrentTask()
{
    if(!tasks.isEmpty() && currentTaskI>=0){
        ui->txtTask->setText(tasks[currentTaskI].text);
        //ui->txtTask->setTextInteractionFlags(Qt::TextBrowserInteraction);
        
        QDateTime currentDateTime(QDateTime::currentDateTime());
        QDateTime taskDateTime(tasks[currentTaskI].dateTime);
        
        //        if(taskDateTime.isValid() && taskDateTime > currentDateTime)
        //        {
        //            tasks[currentTaskI].dateTimePassed = 0;
        //        }
        //        else
        //        {
        //            tasks[currentTaskI].dateTimePassed = 1;
        //        }
        tasks[currentTaskI].dateTimePassed = !(taskDateTime.isValid() &&
                                               (taskDateTime > currentDateTime));

        if(tasks[currentTaskI].hasDateTime)
        {
            if (!tasks[currentTaskI].dateTimePassed)
            {
                ui->lblDateTime->setStyleSheet("color: rgb(0, 255, 0);");
                if(paused)
                {
                    ui->lblDateTime->setStyleSheet("color: rgb(255, 255, 255);");
                    ui->lblDateTime->setText(taskDateTime.toString(dateTimeFormat));
                }
                else
                {
                    QString strTimeRemaining("");
                    int days(currentDateTime.date().daysTo(taskDateTime.date()));
                    
                    float mins(((currentDateTime.time().secsTo(taskDateTime.time()))/60)%60);
                    float hours((currentDateTime.time().secsTo(taskDateTime.time()))/3600);

                    if(days>0)
                    {
                        ui->lblDateTime->setStyleSheet("color: rgb(0, 255, 0);");
                        ui->lblDateTime->setText(taskDateTime.toString(dateTimeFormat));
                    }
                    else
                    {
                        if(hours>1){
                            ui->lblDateTime->setStyleSheet("color: rgb(0, 255, 0);");
                            strTimeRemaining = QString("After: %L1 Hours").arg(hours);
                            ui->lblDateTime->setText(strTimeRemaining);
                        }
                        else if(hours == 1)
                        {
                            ui->lblDateTime->setStyleSheet("color: rgb(0, 255, 0);");
                            strTimeRemaining = QString("After an hour");
                        }
                        else{
                            if(mins>1){
                                ui->lblDateTime->setStyleSheet("color: rgb(0, 255, 0);");
                                strTimeRemaining = "After:";
                                ui->lblDateTime->setText(strTimeRemaining +
                                                         QString("%2 Minutes").arg(mins));
                            }
                            else if(mins == 1)
                            {
                                ui->lblDateTime->setStyleSheet("color: rgb(0, 255, 0);");
                                strTimeRemaining = QString("After a minute");
                            }
                            else
                            {
                                ui->lblDateTime->setStyleSheet("color: rgb(255, 0, 0);");
                                ui->lblDateTime->setText("In Less than a minute");
                            }
                        }
                    }
                }
            }
            else
            {
                ui->lblDateTime->setStyleSheet("color: rgb(255, 0, 0);");
                ui->lblDateTime->setText("Time passed");
                if(!tasks[currentTaskI].notificationShown)
                {
                    notification=new TaskNotifier(tasks[currentTaskI].text,
                                                  height(),currentTaskI,
                                                  tasks[currentTaskI].dateTime.toString(dateTimeFormat));
                    tasks[currentTaskI].notificationShown = 1;
                    connect(notification->ui->btnOk,SIGNAL(clicked())
                            ,this,SLOT(notificationFinished()));
                    connect(notification->ui->btnTskDel,SIGNAL(clicked())
                            ,this,SLOT(notificationFinishedDel()));

                    notification->show();
                    notification->raise();
                    notification->activateWindow();
                }
            }
        }
        else
        {
            ui->lblDateTime->setStyleSheet("color: rgb(255, 255, 255);");
            ui->lblDateTime->setText("No time set");
        }
    }
    else
    {
        currentTaskI = -1;
        ui->lblDateTime->setStyleSheet("color: rgb(255, 255, 255);");
        ui->lblDateTime->setText("All Done...");
        ui->txtTask->clear();
    }
}

void TaskView::on_btnNext_clicked()
{
    if(!tasks.isEmpty())
    {
        currentTaskI ++;
        if(currentTaskI >= tasks.count())
            currentTaskI=0;
        
        viewCurrentTask();
    }
}

void TaskView::on_btnPrev_clicked()
{
    if(!tasks.isEmpty())
    {
        currentTaskI --;
        if(currentTaskI < 0)
            currentTaskI = tasks.count()-1;
        
        viewCurrentTask();
    }
}

void TaskView::on_btnDeleteClick()
{
    QMessageBox::StandardButton ans;
    ans = QMessageBox::warning(parentWidget(),"Warning",
                               "Deleted tasks can not be recovered by the program."
                               "\nDo you really want to delete the task?",
                               QMessageBox::StandardButton::Yes|
                               QMessageBox::StandardButton::No);
    if(ans==QMessageBox::StandardButton::Yes)
    {
        if (!tasks.isEmpty())
        {
            tasks.remove(currentTaskI);
            currentTaskI = tasks.count()-1;
            viewCurrentTask();
            if(tasks.isEmpty())
            {
                sideControls->ui->btnEdit->hide();
                sideControls->ui->btnDelete->hide();
            }
        }
        
        if(tasks.isEmpty())
        {
            sideControls->ui->btnEdit->hide();
            sideControls->ui->btnDelete->hide();
        }
        //        saveChangesToFile(strFileName);
        saveChangesToXmlFile(strXmlFileName);
    }
}

void TaskView::notificationFinished()
{
    delete notification;
}
void TaskView::notificationFinishedDel()
{
    if (!tasks.isEmpty())
    {
        tasks.remove(notification->taskIndex);
        currentTaskI = tasks.count()-1;
        viewCurrentTask();
    }

    if(tasks.isEmpty())
    {
        sideControls->ui->btnEdit->hide();
        sideControls->ui->btnDelete->hide();
    }
//saveChangesToFile(strFileName);
    saveChangesToXmlFile(strXmlFileName);
    delete notification;
}

bool TaskView::saveChangesToFile(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this,"Error","Cannot open file...\n"
                                          "Go get the system administrator to solve this :/");
        return 0;
    }
    QTextStream out(&file);
    for(int i=0;i<tasks.count();i++)
    {
        out<<tasks[i].text<<"\n"<<"%dat%\n"
          <<tasks[i].dateTime.toString("d/M/yyyy h:mm:ss AP")<<"\n"
         <<QString::number(tasks[i].hasDateTime)<<"\n"
        <<QString::number(tasks[i].notificationShown)<<"\n";
    }
    file.close();
    return 1;
}

void TaskView::loadTasksFromFile(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(0,"Warning",QString("Cannot open file \n%1").arg(qAppName()+fileName));
    }
    else
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            Task tsk;
            QString strTsk ="";
            QString lineIn;
            
            //read all text before %dat%:
            for(lineIn = in.readLine();
                lineIn  != "%dat%" && !in.atEnd();
                lineIn = in.readLine())
            {
                strTsk.append(lineIn);
                strTsk.append("\n");
            }
            strTsk.remove(strTsk.length()-1,1);
            tsk.text=strTsk;
            lineIn = in.readLine();
            tsk.dateTime=QDateTime::fromString(lineIn,"d/M/yyyy h:mm:ss AP");
            tsk.hasDateTime =  in.readLine().toInt();
            tsk.notificationShown =  in.readLine().toInt();
            
            tasks.append(tsk);
        }
    }
}

void TaskView::loadTasksFromXmlFile(QString fileName)
{
    QFile file(fileName);
    if(!file.exists())
    {
        saveChangesToXmlFile(fileName);
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(0,"Error","File Error: "+file.errorString());
    }
    else
    {
        QXmlStreamReader xml(&file);
        int i =0;//for indexing
        while (!xml.atEnd())
        {
            if(xml.readNext())
            {
                if(xml.hasError()){
                    QMessageBox::warning(0,"Error","Xml File Error: "+xml.errorString());
                    exit(15);
                }
                
                if(xml.name() == "Task"+QString::number(i))
                {
                    i++;
                    Task nTask;
                    bool notiShown = xml.attributes().value("NS").toInt();
                    nTask.notificationShown = notiShown;
                    
                    nTask.hasDateTime = 0;
                    while (xml.readNextStartElement())
                    {
                        if(xml.name() == "Text")//read text.
                        {
                            nTask.text=xml.readElementText();
                        }
                        else if(xml.name()=="Time")//read time.
                        {
                            nTask.dateTime=QDateTime::fromString(
                                        xml.readElementText(),"d/M/yyyy h:mm:ss AP");
                            
                            QDateTime currentDateTime(QDateTime::currentDateTime());
                            QDateTime taskDateTime(nTask.dateTime);
                            
                            if(taskDateTime.isValid() && taskDateTime > currentDateTime)
                            {
                                nTask.dateTimePassed = 0;
                            }
                            else
                            {
                                nTask.dateTimePassed = 1;
                            }
                            nTask.hasDateTime = 1;
                        }
                        else
                        {
                            xml.skipCurrentElement();
                        }
                    }
                    tasks.append(nTask);
                }
            }
            
        }
    }
}

bool TaskView::saveChangesToXmlFile(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this,"Error","Cannot open file...\n");
        return 0;
    }
    //xml stream writer setup:
    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(false);
    
    //writing xml tags
    xml.writeStartDocument();
    xml.writeStartElement("Tasks");
    
    int total = tasks.count();
    for (int i = 0; i < total; ++i)
    {
        //write task tag:
        xml.writeStartElement("Task"+QString::number(i));
        
        //write task's attributes:
        xml.writeAttribute("NS",QString::number(tasks[i].notificationShown));
        //write task's text:
        xml.writeTextElement("Text",tasks[i].text);
        //write task's time iff the task has time set:
        if(tasks[i].hasDateTime)
            xml.writeTextElement("Time",tasks[i].dateTime.toString("d/M/yyyy h:mm:ss AP"));
        
        xml.writeEndElement();
    }
    xml.writeEndDocument();
    file.close();
    return 1;
}
/*
void TaskView::viewCurrentTask2()
{
    if(!tasks.isEmpty() && currentTaskI>=0){
        ui->txtTask->setText(tasks[currentTaskI].text);

        QDateTime currentDateTime(QDateTime::currentDateTime());
        QDateTime taskDateTime(tasks[currentTaskI].dateTime);

        tasks[currentTaskI].dateTimePassed = !(taskDateTime.isValid() &&
                (taskDateTime > currentDateTime));

        if(tasks[currentTaskI].hasDateTime)
        {
            if (!tasks[currentTaskI].dateTimePassed)
            {
                ui->lblDateTime->setStyleSheet("color: rgb(0, 255, 0);");
                if(paused)
                {
                    ui->lblDateTime->setText(taskDateTime.toString(dateTimeFormat));
                }
                else
                {
                    QString strTimeRemaining("");
                    int days(currentDateTime.date().daysTo(taskDateTime.date()));

                    float mins(((currentDateTime.time().(taskDateTime.time()))/60)%60);
                    float hours((currentDateTime.time().secsTo(taskDateTime.time()))/3600);
                    if(days>0)
                    {
                        ui->lblDateTime->setText(taskDateTime.toString(dateTimeFormat));
                    }
                    else
                    {
                        if(hours>0){
                            strTimeRemaining = QString("After: %L1 Hour(s)").arg(hours);
                            ui->lblDateTime->setText(strTimeRemaining);
                        }
                        else
                        {
                            strTimeRemaining = "After:";
                            ui->lblDateTime->setText(strTimeRemaining +
                                                     QString("%2 Min(s)").arg(mins));
                        }
                    }
                }
            }
            else
            {
                ui->lblDateTime->setStyleSheet("color: rgb(255, 0, 0);");
                ui->lblDateTime->setText("Time passed");
                if(!tasks[currentTaskI].notificationShown)
                {
                    notification=new TaskNotifier(tasks[currentTaskI].text,
                                                  height(),currentTaskI,
                                                  tasks[currentTaskI].dateTime.toString(dateTimeFormat));
                    tasks[currentTaskI].notificationShown = 1;
                    connect(notification->ui->btnOk,SIGNAL(clicked())
                            ,this,SLOT(notificationFinished()));
                    connect(notification->ui->btnTskDel,SIGNAL(clicked())
                            ,this,SLOT(notificationFinishedDel()));

                    notification->show();
                    notification->raise();
                    notification->activateWindow();
                }
            }
        }
        else
        {
            ui->lblDateTime->setText("No time set");
        }
    }
    else
    {
        currentTaskI = -1;
        ui->lblDateTime->setText("All Done...");
        ui->txtTask->clear();
    }
}
*/
