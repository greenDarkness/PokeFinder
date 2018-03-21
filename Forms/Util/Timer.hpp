#ifndef TIMER_HPP
#define TIMER_HPP

#include <QMainWindow>

namespace Ui {
class Timer;
}

class Timer : public QMainWindow
{
    Q_OBJECT

public:
    explicit Timer(QWidget *parent = 0);
    ~Timer();

private:
    Ui::Timer *ui;
};

#endif // TIMER_HPP
