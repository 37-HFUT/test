#include "math_algorithm.h"

math_algorithm::math_algorithm(QWidget *parent) : QWidget(parent)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

// --- 算法部分 ---
/*Zemax 折射率公式计算 */
double math_algorithm::calculateN(double nd, double vd, double lambda)
{
    const double C1 = -0.335562, C2 = 0.41207, C3 = -0.127723;
    const double D1 = -0.0141585, D2 = 0.010522, D3 = 0.115844;
    const double E1 = -0.000113289, E2 = -0.000028339, E3 = 0.734981;
    const double ld = 0.58756; // d光波长参考值 (um)

    double L1 = lambda * lambda - ld * ld;
    double L2 = 1.0 / (lambda * lambda) - 1.0 / (ld * ld);
    double L3 = 1.0 / std::pow(lambda, 4) - 1.0 / std::pow(ld, 4);

    return nd + (C1 + C2 * nd + C3 * nd * nd) * L1
                      + (D1 + D2 * nd + D3 / vd) * L2
                      + (E1 + E2 * nd + E3 / (vd * vd)) * L3;
}

LensResult math_algorithm::calculate(double R1, double R2, double d, double n, double L_obj)
{
    LensResult res;
    double phi = (n - 1.0) * (1.0 / R1 - 1.0 / R2) + (n - 1.0) * (n - 1.0) * d / (n * R1 * R2);
    res.f_prime = 1.0 / phi;
    res.l_H = -(R1 * d) / (n * (R2 - R1) + (n - 1) * d);
    res.l_prime_H_prime = -(R2 * d) / (n * (R2 - R1) + (n - 1) * d);

   // 高斯公式: 物距 s 相对于前主面
    double s = L_obj - res.l_H;
    if (std::abs(1.0 / s + phi) < 1e-9)
    {
        res.L_prime = 1e9;    // 象征无穷远
        res.beta = 0;
    } else
    {
        double s_prime = 1.0 / (1.0 / s + phi);
        res.L_prime = s_prime + res.l_prime_H_prime;   // 像距相对于第二面顶点
        res.beta = s_prime / s;
    }
    return res;
}

// --- 绘图部分 ---
void math_algorithm::paintEvent(QPaintEvent *)
{
    /*
    // 如果没有经过第一次计算，直接返回，右侧将保持空白
        if (!is_initialized) {
            return;
        }

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        if (show_switch) drawDispersionCurve(painter);
        else drawLensShape(painter);
*/
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (show_switch)
    {
        drawDispersionCurve(painter);     //show_switch为1，绘制色差曲线
    } else
    {
        drawLensShape(painter);    //show_switch为0，绘制镜头
    }
}

void math_algorithm::drawLensShape(QPainter &p)
{
    int w = width(), h = height();

    // --- 画背景淡灰色网格 ---
    p.setPen(QPen(QColor(240, 240, 240), 1));
    for(int x = 0; x < w; x += 20) p.drawLine(x, 0, x, h);
    for(int y = 0; y < h; y += 20) p.drawLine(0, y, w, y);

    p.translate(w / 4, h / 2);
    double scale = 2.0;     //缩放比例

    LensResult res = math_algorithm::calculate(R1, R2, d, nd, L_obj);

    // 绘制光轴
    p.setPen(QPen(Qt::lightGray, 1, Qt::DashLine));
    p.drawLine(-100, 0, 400, 0);

    // 绘制透镜示意形状
    p.setPen(QPen(Qt::blue, 2));
    p.setBrush(QColor(100, 150, 255, 60));
    QPainterPath path;
    double lensH = 40; // 透镜半高度
    path.moveTo(0, -lensH);
    // 简单模拟圆弧：这里用二次贝塞尔曲线示意凹凸
    path.quadTo(R1 > 0 ? 10 : -10, 0, 0, lensH);
    path.lineTo(d * scale, lensH);
    path.quadTo(R2 > 0 ? d*scale+10 : d*scale-10, 0, d*scale, -lensH);
    path.closeSubpath();
    p.drawPath(path);

    // 标注主面 H, H' (红色线)
    p.setPen(QPen(Qt::red, 1));
    double h_pos = res.l_H * scale;
    double hp_pos = (d + res.l_prime_H_prime) * scale;
    p.drawLine(h_pos, -50, h_pos, 50);
    p.drawText(h_pos, 65, "H");
    p.drawLine(hp_pos, -50, hp_pos, 50);
    p.drawText(hp_pos, 65, "H'");

    // 标注焦点 F'
    double f_pos = (d + res.l_prime_H_prime + res.f_prime) * scale;
    p.setBrush(Qt::red);
    p.drawEllipse(QPointF(f_pos, 0), 3, 3);
    p.drawText(f_pos, -10, "F'");

    // --- 色差光路绘制 ---

    // 1. 定义三种代表性波长及其对应的画笔颜色
    struct ColorRay
    {
        double lambda;
        QColor color;
        QString name;
    };
    QVector<ColorRay> rays = {
        {0.4861, QColor(0, 100, 255, 200), "F (Blue)"},  // 蓝光：折射强，焦距短
        {0.5876, QColor(0, 255, 0, 200),   "d (Green)"}, // 参考光
        {0.6563, QColor(255, 0, 0, 200),   "C (Red)"}    // 红光：折射弱，焦距长
    };
    double rayH = 20.0; // 光线入射高度
    // 2. 循环绘制每种色光
    for (const auto& ray : rays) {
        // A. 为当前波长计算特定的折射率 n
        double n_lambda = calculateN(nd, vd, ray.lambda);

        // B. 为当前折射率计算光学结果
        LensResult res_lambda = calculate(R1, R2, d, n_lambda, L_obj);

        // C. 设置当前波长的画笔
        p.setPen(QPen(ray.color, 1.2, Qt::SolidLine));

        // D. 绘制光路 (从物点射出)
        double startX = L_obj * scale;
        double midX1 = 0;
        double midX2 = d * scale;
        double endX = (d + res_lambda.L_prime) * scale;

        // 上半部分光线
        p.drawLine(startX, 0, midX1, -rayH);      // 物点 -> 入射面
        p.drawLine(midX1, -rayH, midX2, -rayH);    // 镜内
        p.drawLine(midX2, -rayH, endX, 0);         // 出射面 -> 像点

        // 下半部分光线 (可选，为了美观)
        p.drawLine(startX, 0, midX1, rayH);
        p.drawLine(midX1, rayH, midX2, rayH);
        p.drawLine(midX2, rayH, endX, 0);

        // E. 在光轴上标出不同颜色的像点位置（小圆点）
        p.setBrush(ray.color);
        p.drawEllipse(QPointF(endX, 0), 2, 2);
    }
}

void math_algorithm::drawDispersionCurve(QPainter &p)
{
    int w = width();
    int h = height();

    // 1. 定义图形区域的边距
    int marginLeft = 80;    // 左侧留出空间写波长数值
    int marginBottom = 60;  // 底部留出空间写偏移数值
    int marginTop = 40;
    int marginRight = 40;

    int graphW = w - marginLeft - marginRight;
    int graphH = h - marginTop - marginBottom;

    // 2. 设置坐标范围
    double lamMin = 0.486;    // F线波长
    double lamMax = 0.6563;   // C线波长
    double shiftRange = 1000.0; // 横轴范围设为 +/- 1000 um (即 10e2)

    // 绘制坐标轴线
    p.setPen(Qt::black);
    // 横轴 (底部)
    p.drawLine(marginLeft, h - marginBottom, w - marginRight, h - marginBottom);
    // 纵轴 (左侧)
    p.drawLine(marginLeft, h - marginBottom, marginLeft, marginTop);
    // 零位参考线 (中央垂直黑线)
    int centerX = marginLeft + graphW / 2;
    p.drawLine(centerX, h - marginBottom, centerX, marginTop);

    // --- 3. 绘制 Y 轴刻度与数值 (波长 Wavelength) ---
    p.setFont(QFont("Arial", 8));
    double yTicks[] = {0.486, 0.52, 0.56, 0.60, 0.64, 0.6563};
    for (double lam : yTicks)
    {
      // 计算像素位置：底部是最小波长，顶部是最大波长
      int y = h - marginBottom - (int)((lam - lamMin) / (lamMax - lamMin) * graphH);
      // 画刻度线
      p.drawLine(marginLeft, y, marginLeft - 5, y);
      // 画数值
      p.drawText(marginLeft - 50, y + 5, QString::number(lam, 'f', 4));
    }
    // Y轴标题
    p.save();
    p.translate(marginLeft - 60, h / 2);
    p.rotate(-90);
    p.drawText(0, 0, "Wavelength in um");
    p.restore();
    // --- 4. 绘制 X 轴刻度与数值 (焦距偏移 Focal Shift) ---
    // 从 -1000 到 1000 每 200 一个刻度
    for (int s = -1000; s <= 1000; s += 200)
    {
      int x = centerX + (int)((s / shiftRange) * (graphW / 2.0));
      // 只画区域内的刻度
      if (x >= marginLeft && x <= w - marginRight)
      {
         p.drawLine(x, h - marginBottom, x, h - marginBottom + 5);
         // 格式化文本，模仿图片中的 -10e2, -800.0 等
         QString label;
         if (abs(s) == 1000)
         {
             label = QString::number(s/100) + "e2";
         }else
         {
             label = QString::number((double)s, 'f', 1);
         }
         p.drawText(x - 20, h - marginBottom + 20, label);
       }
     }
     // X轴标题
     p.drawText(centerX - 50, h - 10, "Focal Shift in um");

     // --- 5. 绘制色差曲线 (核心曲线) ---
     p.setPen(QPen(Qt::blue, 1.5));
     double f_ref = math_algorithm::calculate(R1, R2, d, nd, L_obj).f_prime;
     QPointF lastPt;
     bool first = true;
     for (int i = 0; i <= 100; ++i)
     {
        double lam = lamMin + i * (lamMax - lamMin) / 100.0;
        double n_lam = math_algorithm::calculateN(nd, vd, lam);
        double f_lam = math_algorithm::calculate(R1, R2, d, n_lam, L_obj).f_prime;
        // 计算物理偏移量 (转换为微米 um)
        double shift_um = (f_lam - f_ref) * 1000.0;
        // 转换为像素坐标
        int px = centerX + (int)((shift_um / shiftRange) * (graphW / 2.0));
        int py = h - marginBottom - (int)((lam - lamMin) / (lamMax - lamMin) * graphH);
        // 只绘制在图形区域内的点
        if (px >= marginLeft && px <= w - marginRight)
        {
          if (!first) p.drawLine(lastPt, QPointF(px, py));
          lastPt = QPointF(px, py);
          first = false;
        }
   }
}
