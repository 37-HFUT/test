#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "help_window.h"

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent)
, ui(new Ui::MainWindow)    //初始化指针ui
{
    ui->setupUi(this);

    this->setWindowTitle("单透镜参数计算器");

    // --- 统一美化开始 ---
    // 使用 R"(...)" 包含所有 QSS 规则，不要在中间插入 ui-> 这样的代码
    this->setStyleSheet(R"(
        /* 1. 窗口整体背景 */
        QMainWindow {
            background-color: #F5F7FA;
        }

        /* 2. 左侧面板渐变背景 */
        /* 确保你在 UI 设计器里给那个 Frame 命名为 leftPanel */
        #leftPanel {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                        stop:0 #FFFFFF, stop:1 #E4E7ED);
            border-right: 1px solid #DCDFE6;
        }

        /* 3. 输入框美化 */
        QLineEdit {
            border: 1px solid #DCDFE6;
            border-radius: 4px;
            padding: 4px;
            background: white;
            font-size: 13px;
        }
        QLineEdit:focus {
            border: 1px solid #409EFF;
        }

        /* 4. 按钮统一美化 */
        QPushButton {
            background-color: #409EFF;
            color: white;
            border-radius: 6px;
            padding: 6px 10px;
            min-height: 28px;
            font-weight: bold;
            font-size: 13px;
            border: none;
        }
        QPushButton:hover {
            background-color: #66B1FF;
        }
        QPushButton:pressed {
            background-color: #3A8EE6;
        }

        /* 5. 特殊处理“查看公式”按钮 */
        /* 确保你在 UI 设计器里给该按钮命名为 btnHelp */
        QPushButton#btnHelp {
            background-color: #72767b;
            color: #FFFFFF;
        }
        QPushButton#btnHelp:hover {
            background-color: #909399;
        }

        /* 6. 结果显示标签 (label_Result) 美化 */
        #label_Result {
            background-color: #ffffff;
            border: 1px solid #dcdfe6;
            border-radius: 8px;
            padding: 10px;
            color: #303133;
            font-family: "Microsoft YaHei";
        }
    )");
    // --- 统一美化结束 ---
}

MainWindow::~MainWindow()
{
    delete ui;
}
//计算槽函数
void MainWindow::on_btnCalc_clicked()
{
   updateAll(); // 按钮点一下，执行全套更新
}
//切换槽函数
void MainWindow::on_btnMode_clicked()
{
    ui->canvas->show_switch = !ui->canvas->show_switch;

    ui->canvas->update();
}
// mainwindow.cpp

void MainWindow::on_btnHelp_clicked() // 假设你的按钮叫 btnHelp
{
    // 创建你新定义的 help_window 类实例，而不是之前的 FormulaDialog
    help_window *hw = new help_window(this);

    // 设置窗口关闭时自动释放内存
    hw->setAttribute(Qt::WA_DeleteOnClose);

    // 模态显示（用户必须关闭它才能操作主窗口）
    hw->exec();
}
// 当滑动条移动时执行
void MainWindow::on_slider_D_valueChanged(int value)
{
    // 1. 将整数转换为浮点数（缩小10倍）
    double d_val = value / 10.0;

    // 2. 同步更新文本框，让用户看到具体的数字
    ui->edit_D->setText(QString::number(d_val, 'f', 1));

    // 3. 直接调用你之前的计算逻辑
    // 为了方便，建议把原本 on_btnCalc_clicked() 里的代码提取成一个独立的函数 updateAll()
    updateAll();
}
// 建议：把计算和重绘逻辑封装一下，方便重复调用
void MainWindow::updateAll()
{
    // 1. 设置开关为 true
      // ui->canvas->is_initialized = true;

       // 2. 传递参数
    //   ui->canvas->R1 = ui->edit_R1->text().toDouble();


    // 1. 获取输入并检查是否为有效数字
    bool okR1, okR2, okD, okNd, okVd, okL;
    double r1 = ui->edit_R1->text().toDouble(&okR1);
    double r2 = ui->edit_R2->text().toDouble(&okR2);
    double d  = ui->edit_D->text().toDouble(&okD);
    double nd = ui->edit_Nd->text().toDouble(&okNd);
    double vd = ui->edit_Vd->text().toDouble(&okVd);
    double L  = ui->edit_L->text().toDouble(&okL);

    // 2. 健壮性检查逻辑
    QString errorMsg = "";
    // 检查是否输入了非数字字符
    if (!okR1 || !okR2 || !okD || !okNd || !okVd || !okL)
    {
        errorMsg = "请输入全部有效的数值！";
    }
    // 检查物理合理性
    else if (nd <= 1.0)
    {
        errorMsg = "错误：折射率 nd 必须大于 1.0";
    }
    else if (d <= 0)
    {
        errorMsg = "错误：中心厚度 d 必须大于 0";
    }
    else if (vd <= 0)
    {
        errorMsg = "错误：阿贝数 vd 必须为正数";
    }
    else if (r1 == 0 || r2 == 0)
    {
        // 在光学中，半径为0没有物理意义（平面应设为极大值如999999）
        errorMsg = "错误：半径不能为0 (平面请用极大值代替)";
    } else if(L > 0 || L == 0)
    {
        errorMsg = "错误：L的值小于0";
    }
    // 3. 如果有错误，显示红色提示并终止计算
    if (!errorMsg.isEmpty())
    {
        ui->label_Result->setStyleSheet("color: red; font-weight: bold; font-size: 16px;");
        ui->label_Result->setText(errorMsg);
        return;   //停止后续计算
    }

    // 4. 数据合法，恢复正常样式并执行计算
    ui->label_Result->setStyleSheet("color: darkgreen; font-weight: bold;");

    // 2. 调用算法类进行计算
    LensResult res = math_algorithm::calculate(r1, r2, d, nd, L);

    // 3. 构建完整的输出字符串
    // 使用 HTML 表格语法构造字符串
       QString resultText = QString(
           "<b>[ 计算结果 ]</b><br>"
           "<table border='0' style='margin-top:5px;'>"
           "<tr><td>焦距 f':</td><td align='right' style='padding-left:15px;'>%1 mm</td></tr>"
           "<tr><td>前主面 lH:</td><td align='right' style='padding-left:15px;'>%2 mm</td></tr>"
           "<tr><td>后主面 l'H':</td><td align='right' style='padding-left:15px;'>%3 mm</td></tr>"
           "<tr><td>像距 L':</td><td align='right' style='padding-left:15px;'>%4 mm</td></tr>"
           "<tr><td>放大率 Beta:</td><td align='right' style='padding-left:15px;'>%5</td></tr>"
           "</table>"
       )
       .arg(res.f_prime, 0, 'f', 4)
       .arg(res.l_H, 0, 'f', 4)
       .arg(res.l_prime_H_prime, 0, 'f', 4)
       .arg(res.L_prime, 0, 'f', 4)
       .arg(res.beta, 0, 'f', 4);

       ui->label_Result->setText(resultText);
    // 5. 更新绘图区（Canvas）
    ui->canvas->R1 = r1;
    ui->canvas->R2 = r2;
    ui->canvas->d = d;
    ui->canvas->nd = nd;
    ui->canvas->L_obj = L;
    ui->canvas->update(); // 触发重绘

    // 简单写一下：
    r1 = ui->edit_R1->text().toDouble();
    r2 = ui->edit_R2->text().toDouble();
    d  = ui->edit_D->text().toDouble(); // 此时这里已经是滑动条同步过来的值了
    // ... 获取其他参数 ...

    // 更新画布参数
    ui->canvas->d = d;
    // ... 更新其他参数 ...

    // 执行计算显示文本并重绘
    res = math_algorithm::calculate(r1, r2, d, ui->canvas->nd, ui->canvas->L_obj);
    ui->label_Result->setText(QString(
        "<b>[ 计算结果 ]</b><br>"
        "<table border='0' style='margin-top:5px;'>"
        "<tr><td>焦距 f':</td><td align='right' style='padding-left:15px;'>%1 mm</td></tr>"
        "<tr><td>前主面 lH:</td><td align='right' style='padding-left:15px;'>%2 mm</td></tr>"
        "<tr><td>后主面 l'H':</td><td align='right' style='padding-left:15px;'>%3 mm</td></tr>"
        "<tr><td>像距 L':</td><td align='right' style='padding-left:15px;'>%4 mm</td></tr>"
        "<tr><td>放大率 Beta:</td><td align='right' style='padding-left:15px;'>%5</td></tr>"
        "</table>")
        .arg(res.f_prime, 0, 'f', 4)           // 填入 %1
        .arg(res.l_H, 0, 'f', 4)               // 填入 %2
        .arg(res.l_prime_H_prime, 0, 'f', 4)   // 填入 %3
        .arg(res.L_prime, 0, 'f', 4)           // 填入 %4
        .arg(res.beta, 0, 'f', 4)              // 填入 %5
    );
    ui->canvas->update();           // 实时刷新透镜形状
}
