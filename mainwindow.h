#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/* 使MSVC支持中文
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif*/

#include <QMainWindow>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>
#include <QLCDNumber>
#include <QPushButton>
#include <stack>
#include <QLabel>
#include <utility>
#include "block.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void on_pushButton_start_clicked();
    void on_pushButton_back_clicked();
    void on_pushButton_edit_clicked();
    void on_pushButton_register_clicked();
    void showTime();
private:
    Ui::MainWindow *ui;

    block* blocks[32];
    int color;
    int chess_id;      // unchosen = -1
    bool start_game;
    QPainter *painter;
    QTimer *myTimer;
    QTime *TimeRecord;
    QTimer *timer;
    QLCDNumber *myLCDNumber;
    struct block_tmp
    {
        block_tmp()
            : id(-1), row(-1), col(-1) {}
        block_tmp(int i, int r, int c)
            : id(i), row(r), col(c) {}
        int id;
        int row;
        int col;
    };
    std::stack<std::pair<block_tmp, block_tmp>> block_tmps;

    inline void change_color();

    void paintChessboard() const;
    void paintBlock(int id) const;
    void paintCross(int row, int col) const;
    void paintWinner() const;

    void move_block(int row, int col);
    inline void save_move();
    inline void save_eaten (int id);

    bool moveable(int id, int row, int col) const;
    bool moveable_bing(int id, int row, int col) const;
    bool moveable_jiang(int id, int row, int col) const;
    bool moveable_ju(int id, int row, int col) const;
    bool moveable_pao(int id, int row, int col) const;
    bool moveable_shi(int id, int row, int col) const;
    bool moveable_xiang(int id, int row, int col) const;
    bool moveable_ma(int id, int row, int col) const;


};

#endif // MAINWINDOW_H
