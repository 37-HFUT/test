#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <math_algorithm.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:

    void on_btnCalc_clicked();  // 计算按钮槽函数
    void on_btnMode_clicked();  // 切换模式按钮的槽函数
    void on_btnHelp_clicked(); // “查看公式”按钮的槽函数
    void on_slider_D_valueChanged(int value); // 滑动条数值改变的槽函数
    void updateAll();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
