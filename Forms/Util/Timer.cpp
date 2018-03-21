#include "Timer.hpp"
#include "ui_Timer.h"

Timer::Timer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Timer)
{
    ui->setupUi(this);
}

Timer::~Timer()
{
    delete ui;
}
