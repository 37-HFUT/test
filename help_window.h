#ifndef HELP_WINDOW_H
#define HELP_WINDOW_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include <QPixmap>


class help_window : public QDialog
{
    Q_OBJECT
public:
    explicit help_window(QWidget *parent = nullptr);
};

#endif // HELP_WINDOW_H
