#include "help_window.h"

help_window::help_window(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("光学计算公式与说明");
    resize(700, 600);

    QVBoxLayout *layout = new QVBoxLayout(this);

    // 使用滚动区域，防止内容过多显示不全
    QScrollArea *scroll = new QScrollArea;
    QWidget *content = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(content);

    // 1. 公式 (输出结果)
    QLabel *FormulaLabel = new QLabel;
    QPixmap Formula("../Image/gongshi"); // 确保你的图片放在运行目录下
    if (!Formula.isNull())
    {
        FormulaLabel->setPixmap(Formula.scaledToWidth(600, Qt::SmoothTransformation));
    } else {
        FormulaLabel->setText("<p style='color:red;'>[ 未找到 formula.png 图片文件 ]</p>");
    }
    contentLayout->addWidget(FormulaLabel);

    // 2. 图片展示部分 (Zemax 色散公式)
    QLabel *imgLabel = new QLabel;
    QPixmap pix("../Image/Zemax"); // 确保你的图片放在运行目录下
    if (!pix.isNull())
    {
        imgLabel->setPixmap(pix.scaledToWidth(600, Qt::SmoothTransformation));
    } else {
        imgLabel->setText("<p style='color:red;'>[ 未找到 formula.png 图片文件 ]</p>");
    }
    contentLayout->addWidget(imgLabel);

    // 3. 底部关闭按钮
    QPushButton *btnClose = new QPushButton("关闭");
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);

    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    layout->addWidget(scroll);
    layout->addWidget(btnClose);
}
