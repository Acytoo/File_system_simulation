#ifndef CONTENT_H
#define CONTENT_H

#include <QMainWindow>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>


namespace Ui {
class content;
}

class content : public QMainWindow {
    Q_OBJECT

public:
    explicit content(QWidget *parent = 0);
    ~content();


private slots:
    void receiveUserName(QString);

    //start file system simulation functions
    void cd_fun();



private:
    Ui::content *ui;

    QLineEdit *lineEdit_path, *lineEdit_search;
    QLabel *user_name;
    QString path, qstr_name;
    QPushButton *btn_userName;
};

#endif // CONTENT_H
