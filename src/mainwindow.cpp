#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "osgwidget.hpp"
#include "dronenode.hpp"
#include <ros/ros.h>
#include <QToolBar>
#include <QProcess>
#include <QMessageBox>

MainWindow::MainWindow(int argc,char** argv,QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_argc{argc},
    m_argv{argv},
    m_drone_node{m_argc,m_argv},
    m_process{new QProcess{this}}
{
    m_ui->setupUi(this);

    OSGWidget *osg_widget{new OSGWidget};
    setCentralWidget(osg_widget);
    this->createToolbar();
    this->setupStatusBar();

    connect(&m_drone_node,&quad::DroneNode::statesChanged,
            osg_widget, &OSGWidget::updateDroneStates);
}

MainWindow::~MainWindow()
{
    delete m_ui;
    if (m_app_started_roscore)
    {
        m_process->close();
        QProcess kill_roscore;
        kill_roscore.start(QString{"killall"}, QStringList() << "-9" << "roscore");
        kill_roscore.waitForFinished();
        kill_roscore.close();

        QProcess kill_rosmaster;
        kill_rosmaster.start(QString{"killall"}, QStringList() << "-9" << "rosmaster");
        kill_rosmaster.waitForFinished();
        kill_rosmaster.close();
    }
}

void MainWindow::updateRosStatus()
{
    if(m_drone_node.rosIsConnected())
        m_ui->connection_label->setPixmap(m_check_icon.pixmap(16,16));
    else
        m_ui->connection_label->setPixmap(m_x_icon.pixmap(16,16));
}

void MainWindow::startRosCore()
{
    if (!m_drone_node.rosIsConnected())
    {
        QString program{"roscore"};
        m_process->start(program);
        while (!m_drone_node.rosIsConnected()) {}
        m_app_started_roscore = true;
        m_ui->statusbar->showMessage(tr("Started ROS Core"),5000);
    }
    else
        m_ui->statusbar->showMessage(tr("ROS Core Is Already Running"),5000);
    this->updateRosStatus();
}

void MainWindow::createToolbar()
{
    QToolBar *tool_bar{addToolBar(tr("Main Actions Toolbar"))};
    QAction *start_action{createStartAction()};
    tool_bar->addAction(start_action);
    QAction *roscore_action{createRoscoreAction()};
    tool_bar->addAction(roscore_action);

    m_main_toolbar = tool_bar;
}

void MainWindow::setupStatusBar()
{
    this->updateRosStatus();
    m_ui->statusbar->addPermanentWidget(m_ui->ros_label);
    m_ui->statusbar->addPermanentWidget(m_ui->connection_label);
    m_ui->connection_label->hide();

}

QAction* MainWindow::createStartAction()
{
    const QIcon start_icon{QIcon(":myicons/start.png")};
    QAction *start_action{new QAction(start_icon, tr("&Run Simulation (Ctrl+R)"), this)};
    start_action->setShortcut(QKeySequence{tr("Ctrl+R")});
    start_action->setStatusTip(tr("Run simulation will either begin or resume simulation"));
    connect(start_action, &QAction::triggered, this, &MainWindow::startSimulation);

    return start_action;
}

void MainWindow::startSimulation()
{
    bool test{m_drone_node.init()};
    if (!test)
        QMessageBox::warning(this,tr("NO ROS MASTER DETECTED!"),
                      tr("Can not start the simulation. Connect to a ros master and try again."));
}

QAction* MainWindow::createRoscoreAction()
{
    const QIcon ros_icon{QIcon(":myicons/ros.png")};
    QAction *start_ros_action{new QAction(ros_icon, tr("&Start ROS core"), this)};
    start_ros_action->setStatusTip(tr("This will start a ROS core on local machine"));
    connect(start_ros_action, &QAction::triggered, this, &MainWindow::on_view_ROS_Settings_Panel_triggered);

    return start_ros_action;
}

void MainWindow::on_start_triggered()
{
    this->startSimulation();
}

void MainWindow::on_close_triggered()
{
    QApplication::quit();
}


void MainWindow::on_roscore_button_clicked()
{
    this->startRosCore();
}

void MainWindow::on_ros_check_box_clicked()
{
    m_drone_node.setUseRos(m_ui->ros_check_box->isChecked());
}

void MainWindow::on_view_ROS_Settings_Panel_triggered()
{
    if (m_ui->ros_dock->isVisible())
        m_ui->ros_dock->hide();
    else
        m_ui->ros_dock->show();
}

void MainWindow::on_view_ROS_Connection_Status_triggered()
{

}
