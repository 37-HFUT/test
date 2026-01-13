#ifndef MATH_ALGORITHM_H
#define MATH_ALGORITHM_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <cmath>

// 计算结果的结构体
struct LensResult
{
    double f_prime;          // 焦距
    double l_H;              // 前主面位置 (相对第一面顶点)
    double l_prime_H_prime;  // 后主面位置 (相对第二面顶点)
    double L_prime;          // 像距 (相对第二面顶点)
    double beta;             // 横向放大率
};

class math_algorithm : public QWidget
{
    Q_OBJECT
public:
    explicit math_algorithm(QWidget *parent = nullptr);

    // 外部可调用的输入参数
    double R1 = 50, R2 = -50, d = 10, nd = 1.5168, vd = 64.17, L_obj = -100;

    bool show_switch = false; // 切换：false画形状，true画曲线

    //初始化状态，是否进行了第一次计算
    //bool is_initialized = false; //初始设为 false，表示还没开始第一次计算

    // 核心算法函数
    static double calculateN(double nd, double vd, double lambda);
    static LensResult calculate(double R1, double R2, double d, double n, double L_obj);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawLensShape(QPainter &p);
    void drawDispersionCurve(QPainter &p);
};

#endif // MATH_ALGORITHM_H
