#ifndef FILEWINDOW_H
#define FILEWINDOW_H

#include <QDialog>

namespace Ui {
class filewindow;
}

class filewindow : public QDialog
{
    Q_OBJECT

public:
    explicit filewindow(QWidget *parent = 0);
    ~filewindow();



private slots:
    void on_btn_logout_clicked();
    void receiveUserName(QString);

private:
    Ui::filewindow *ui;
    QString path;   //模仿Windows文件管理器，在正面的上方标出地址，以line edit表示，同时可以cd到输入的目录
};

#endif // FILEWINDOW_H
