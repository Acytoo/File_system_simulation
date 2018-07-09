#ifndef CONTENT_H
#define CONTENT_H

#include <QMainWindow>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>

namespace Ui {
class content;
}

class content : public QMainWindow {
    Q_OBJECT

public:
    explicit content(QWidget *parent = 0);
    ~content();

    void refresh();

    void refresh_bulletin();

    void contextMenuEvent(QContextMenuEvent *event);



signals:
    void start();

private slots:
    //void receiveUserName(QString);

    //start file system simulation functions
    void cd_fun();
    void cd_parent();
    void fun();
    void re(QString);
    void add_file();
    void add_folder();
    void del_adjust();
    void pop_detail();
    void on_list_content_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::content *ui;

    QLineEdit *lineEdit_path, *lineEdit_search;
    QLabel *user_name;
    QString path, qstr_name;
    QPushButton *btn_userName, *btn_level_up, *btn_goto;
    QMenu *menu_content ;
    QAction *action_add_folder, *action_add_file, *action_del, *action_detail;

};

#endif // CONTENT_H
