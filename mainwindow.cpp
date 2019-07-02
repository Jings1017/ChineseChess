#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPalette>
#include <QPainter>
#include <QMessageBox>
#include <QDebug>
#include <QString>
#include <QTime>
#include <QInputDialog>

#define DEBUG
#define block_size  70
#define WIDTH   (LEFT * 2 + block_size * block::COL+500)
#define HEIGHT  (UP * 2 + block_size * block::ROW)
#define LEFT    70
#define UP      70
#define RIGHT   (LEFT + block::COL * block_size  )
#define DOWN    (UP + block::ROW * block_size)
#define COLOR_BACKGROUND        QColor(120, 120, 120)
#define COLOR_CHESS_UNCHECKED   QColor(230, 230, 180)
#define COLOR_CHESS_CHECKED     QColor(255, 209, 105)
#define COLOR_CHESS_BLACK       QColor(0, 0, 0)
#define COLOR_CHESS_RED         QColor(220, 0, 0)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    //painter(0),
    color(block::RED),
    chess_id(-1),
    start_game(false)
{
    ui->setupUi(this);

    // new  chess piece
    for (int i = 0; i < block::NUM; ++i)
    {
        blocks[i] = new block(i);
    }

    // set window
    setWindowTitle("Chinese Chess");

    //size can't change
    resize(WIDTH, HEIGHT);
    setMinimumSize(WIDTH, HEIGHT); // set min size
    setMaximumSize(WIDTH, HEIGHT); // set max size

    // set background color
    QPalette pal;
    pal.setBrush(QPalette::Background, COLOR_BACKGROUND);
    setPalette(pal);

    // button size
    ui->pushButton_start->setGeometry    (RIGHT + 40, UP +  3 * block_size +  30, block_size +40, block_size / 2 +50);
    ui->pushButton_back->setGeometry     (RIGHT + 40, UP +  4 * block_size +  60, block_size +40, block_size / 2 +50);
    ui->pushButton_edit->setGeometry     (RIGHT + 40, UP +  5 * block_size +  90, block_size +40, block_size / 2 +50);
    ui->pushButton_register->setGeometry (RIGHT + 40, UP +  6 * block_size + 120, block_size +40, block_size / 2 +50);

    // timer
    myTimer = new QTimer(this);
    myLCDNumber = new QLCDNumber(this);
    myLCDNumber->setGeometry(RIGHT+block_size/2,UP+block_size,block_size*7/4,2*block_size);
    myLCDNumber->setDigitCount(8);
    myLCDNumber->setPalette(Qt::black);
    myTimer->start(1000);
    showTime();
    connect(myTimer,SIGNAL(timeout()),this,SLOT(showTime()));

    // mouse tracking
    setMouseTracking(true);
}
// destructor for chess piece
MainWindow::~MainWindow()
{
    for (int i = 0; i < block::NUM; ++i)
    {
        delete blocks[i];
    }
    delete ui;
}

void MainWindow::showTime()
{
    QTime time = QTime::currentTime();
    QString text = time.toString("hh:mm:ss");
    myLCDNumber->display(text);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    // game over
    if (!blocks[4]->is_live() || !blocks[20]->is_live()) // king is killed
    {
        return;
    }

    int x = event->x();
    int y = event->y();

    // chess is out of range // illegal
    if (x < LEFT -  block_size / 2 || x > RIGHT + block_size / 2 ||
          y < UP -  block_size / 2 || y > DOWN +  block_size / 2)
    {
        QMessageBox::warning(this,tr("Warning !"),tr("What do you wanna do ? "));
    }

    int col = (x - LEFT + block_size / 2) / block_size;
    int row = (y - UP + block_size / 2) / block_size;
    int c   = LEFT + col * block_size;
    int r   =   UP + row * block_size;

    // click illegal
    if ((x - c) * (x - c) + (y - r) * (y - r) >= block_size * block_size / 4)
    {
       return;
    }

    for (int i = 0; i < 32; ++i)
    {
        // not check killed chess
        if (!blocks[i]->is_live())
        {
            continue;
        }

        // chosen chess
        if (blocks[i]->col() == col && blocks[i]->row() == row)
        {
            // if choose the error color
            if (chess_id == -1 && blocks[i]->color() != color)
            {
                QMessageBox::warning(this,tr("Warning !"),tr("Dude , it's not your turn !"));
                return;
            }

            // if the chess is killed
            if (chess_id != -1 && blocks[chess_id]->is_checked() && blocks[i]->color() != blocks[chess_id]->color())
            {
                // if can't move
                if (!moveable(chess_id, row, col))
                {
                    QMessageBox::warning(this,tr("Warning !"),tr("illegal !"));
                    return;
                }

                save_eaten(i); // store the step
                blocks[i]->dead(); // the chess is killed
                move_block(row, col); // move chess

                goto end;
            }

            // cancel the choosen chess
            if (chess_id == i)
            {
                blocks[chess_id]->clicked();
                chess_id = -1;

                goto end;
            }

            // decide to choose another chess
            blocks[chess_id]->clicked();
            chess_id = i;
            blocks[chess_id]->clicked();

            goto end;
        }
    }

    // if out of range
    if ((x - c) *(x - c) + (y - r) * (y - r) >= block_size * block_size / 8)
    {
        return;
    }

    if (chess_id != -1 && blocks[chess_id]->is_checked())
    {
        // can't move
        if (!moveable(chess_id, row, col))
        {
            return;
        }

        save_move(); // store the step
        move_block(row, col); // move
    }

end:
    update();
    return;
}

// move the chess
void MainWindow::move_block(int row, int col)
{
    // modify the chess coordinate
    try
    {
        blocks[chess_id]->set_pos(row, col);
    }
    catch(std::out_of_range &e)
    {
        qDebug() << e.what();
    }

    // if choose none chess
    blocks[chess_id]->clicked();
    chess_id = -1;

    change_color(); // change to other player
}

// save move // using stack
inline void MainWindow::save_move()
{
    block_tmps.push(std::make_pair<block_tmp,block_tmp> ( block_tmp(),
                block_tmp(chess_id, blocks[chess_id]->row(), blocks[chess_id]->col()) ));
}

// save eaten  // using stack
inline void MainWindow::save_eaten(int id)
{
    block_tmps.push(std::make_pair<block_tmp, block_tmp>(
                block_tmp(id, blocks[id]->row(), blocks[id]->col()),
                block_tmp(chess_id, blocks[chess_id]->row(), blocks[chess_id]->col()) ));
}

void MainWindow::paintChessboard() const
{
    QPen pen;
    pen.setWidth(3);
    painter->setPen(pen);

    for (int i = 1; i < block::COL; ++i) // |
    {
        // for upper
        painter->drawLine(LEFT + block_size * i,   UP, LEFT + block_size * i,   UP + 4 * block_size);
        // for lower
        painter->drawLine(LEFT + block_size * i, DOWN, LEFT + block_size * i, DOWN - 4 * block_size);
    }
    //  |   |
    painter->drawLine (LEFT, UP, LEFT, DOWN);
    painter->drawLine(RIGHT,UP, RIGHT, DOWN);
    for (int i = 0; i <= block::ROW; ++i) // ---
    {
        painter->drawLine(LEFT,  UP + block_size * i, RIGHT, UP + block_size * i);
    }

    // x-axis
    painter->drawText(LEFT              , DOWN+block_size*3/4 , block_size , block_size/2 , Qt::AlignLeft,tr("0"));
    painter->drawText(LEFT+block_size*1 , DOWN+block_size*3/4 , block_size , block_size/2 , Qt::AlignLeft,tr("1"));
    painter->drawText(LEFT+block_size*2 , DOWN+block_size*3/4 , block_size , block_size/2 , Qt::AlignLeft,tr("2"));
    painter->drawText(LEFT+block_size*3 , DOWN+block_size*3/4 , block_size , block_size/2 , Qt::AlignLeft,tr("3"));
    painter->drawText(LEFT+block_size*4 , DOWN+block_size*3/4 , block_size , block_size/2 , Qt::AlignLeft,tr("4"));
    painter->drawText(LEFT+block_size*5 , DOWN+block_size*3/4 , block_size , block_size/2 , Qt::AlignLeft,tr("5"));
    painter->drawText(LEFT+block_size*6 , DOWN+block_size*3/4 , block_size , block_size/2 , Qt::AlignLeft,tr("6"));
    painter->drawText(LEFT+block_size*7 , DOWN+block_size*3/4 , block_size , block_size/2 , Qt::AlignLeft,tr("7"));
    painter->drawText(LEFT+block_size*8 , DOWN+block_size*3/4 , block_size , block_size/2 , Qt::AlignLeft,tr("8"));
    painter->drawText(LEFT+block_size*9 , DOWN+block_size*3/4 , block_size , block_size/2 , Qt::AlignLeft,tr("x"));
    // y-axis
    painter->drawText(LEFT-block_size*3/4 ,UP-block_size/2, block_size , block_size/2 , Qt::AlignLeft,tr("y"));
    painter->drawText(LEFT-block_size*3/4, UP             , block_size , block_size/2 , Qt::AlignLeft,tr("0"));
    painter->drawText(LEFT-block_size*3/4, UP+block_size*1, block_size , block_size/2 , Qt::AlignLeft,tr("1"));
    painter->drawText(LEFT-block_size*3/4, UP+block_size*2, block_size , block_size/2 , Qt::AlignLeft,tr("2"));
    painter->drawText(LEFT-block_size*3/4, UP+block_size*3, block_size , block_size/2 , Qt::AlignLeft,tr("3"));
    painter->drawText(LEFT-block_size*3/4, UP+block_size*4, block_size , block_size/2 , Qt::AlignLeft,tr("4"));
    painter->drawText(LEFT-block_size*3/4, UP+block_size*5, block_size , block_size/2 , Qt::AlignLeft,tr("5"));
    painter->drawText(LEFT-block_size*3/4, UP+block_size*6, block_size , block_size/2 , Qt::AlignLeft,tr("6"));
    painter->drawText(LEFT-block_size*3/4, UP+block_size*7, block_size , block_size/2 , Qt::AlignLeft,tr("7"));
    painter->drawText(LEFT-block_size*3/4, UP+block_size*8, block_size , block_size/2 , Qt::AlignLeft,tr("8"));
    painter->drawText(LEFT-block_size*3/4, UP+block_size*9, block_size , block_size/2 , Qt::AlignLeft,tr("9"));

    // X
    painter->drawLine(LEFT + 3 * block_size, UP                   , RIGHT - 3 * block_size, UP   + 2 * block_size );
    painter->drawLine(LEFT + 3 * block_size, UP   + 2 * block_size, RIGHT - 3 * block_size, UP                    );
    painter->drawLine(LEFT + 3 * block_size, DOWN - 2 * block_size, RIGHT - 3 * block_size, DOWN                  );
    painter->drawLine(LEFT + 3 * block_size, DOWN                 , RIGHT - 3 * block_size, DOWN - 2 * block_size );

    //  _| |_
    //  -| |-
    for(int i=2;i<=block::COL;i+=5)
    {
        paintCross(i,1);
        paintCross(i,7);
    }
    for (int i=0;i<=block::COL;i+=2)
    {
        paintCross(3, i);
        paintCross(6, i);
    }

    // edge line
    pen.setWidth(7);
    painter->setPen(pen);
    // l
    painter->drawLine(LEFT - block_size / 10,  UP - block_size / 10,  LEFT - block_size / 10,  DOWN + block_size / 10);
    // u
    painter->drawLine(LEFT - block_size / 10,  UP - block_size / 10, RIGHT + block_size / 10,    UP - block_size / 10);
    // r
    painter->drawLine(RIGHT + block_size / 10, UP - block_size / 10, RIGHT + block_size / 10,  DOWN + block_size / 10);
    // d
    painter->drawLine(LEFT - block_size / 10, DOWN + block_size / 10, RIGHT + block_size / 10, DOWN + block_size / 10);

    //id table
    // l
    painter->drawLine(RIGHT + 3*block_size,  UP - block_size / 10, RIGHT + 3*block_size,  DOWN + block_size / 10);
    // r
    painter->drawLine(RIGHT + 8*block_size,  UP - block_size / 10, RIGHT + 8*block_size,  DOWN + block_size / 10);
    // u
    painter->drawLine(RIGHT + 3*block_size , UP - block_size / 10, RIGHT + 8*block_size,    UP - block_size / 10);
    // d
    painter->drawLine(RIGHT + 3*block_size ,DOWN + block_size / 10,RIGHT + 8*block_size,  DOWN + block_size / 10);

    //id num
    painter->drawText(RIGHT + 11/2*block_size, UP-30 , block_size,   block_size , Qt::AlignLeft,tr("id table"));
    painter->drawText(RIGHT + 4*block_size, UP+2 , block_size,   block_size , Qt::AlignLeft,tr("0"));
    painter->drawText(RIGHT + 5*block_size, UP+2 , block_size,   block_size , Qt::AlignLeft,tr("1"));
    painter->drawText(RIGHT + 6*block_size, UP+2 , block_size,   block_size , Qt::AlignLeft,tr("2"));
    painter->drawText(RIGHT + 7*block_size, UP+2 , block_size,   block_size , Qt::AlignLeft,tr("3"));
    painter->drawText(RIGHT + 3*block_size + 8, UP+ 1*block_size-2 , block_size,   block_size , Qt::AlignLeft,tr(" 0"));
    painter->drawText(RIGHT + 3*block_size + 8, UP+ 2*block_size-2 , block_size,   block_size , Qt::AlignLeft,tr(" 4"));
    painter->drawText(RIGHT + 3*block_size + 8, UP+ 3*block_size-2 , block_size,   block_size , Qt::AlignLeft,tr(" 8"));
    painter->drawText(RIGHT + 3*block_size + 8, UP+ 4*block_size-2 , block_size,   block_size , Qt::AlignLeft,tr("12"));
    painter->drawText(RIGHT + 3*block_size + 8, UP+ 5*block_size-2 , block_size,   block_size , Qt::AlignLeft,tr("16"));
    painter->drawText(RIGHT + 3*block_size + 8, UP+ 6*block_size-2 , block_size,   block_size , Qt::AlignLeft,tr("20"));
    painter->drawText(RIGHT + 3*block_size + 8, UP+ 7*block_size-2 , block_size,   block_size , Qt::AlignLeft,tr("24"));
    painter->drawText(RIGHT + 3*block_size + 8, UP+ 8*block_size-2 , block_size,   block_size , Qt::AlignLeft,tr("28"));


    // river and  word  // if game over , then don't print
    if (!blocks[4]->is_live() || !blocks[20]->is_live())
    {
        return;
    }

    painter->setFont(QFont("consolas", block_size * 1/2, 50));
    painter->drawText(QRect(LEFT + block_size, UP + 4 * block_size, 8 * block_size, block_size), " 楚河      漢界 ");
    painter->resetMatrix();
}


void MainWindow::paintWinner() const
{
    if (!blocks[4]->is_live()) // red king  die
    {
        painter->setFont(QFont("consolas", block_size * 45/ 100, 50));
        painter->drawText(QRect(LEFT +   block_size, UP + 4 * block_size + 10 ,
                               10 * block_size, block_size), "Black        WON");
        return;
    }

    if (!blocks[20]->is_live()) // balck king die
    {
        painter->setFont(QFont("consolas", block_size * 45 / 100, 50));
        painter->drawText(QRect(LEFT +   block_size,  UP + 4 * block_size + 10 ,
                               10 * block_size, block_size), "Red          WON");
        return;
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    (void*)event;

    painter = new QPainter(this);

    // Antialiasing
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHints(QPainter::SmoothPixmapTransform);

    paintChessboard();

    if (!start_game)
    {
        goto end;
    }

    // paint chess
    for (int i = 0; i < block::NUM; ++i)
    {
        paintBlock(i);
    }

    paintWinner();

end:
    delete painter;
    update();
}

void MainWindow::paintCross(int row, int col) const
{
    int r = UP   + row * block_size;
    int c = LEFT + col * block_size;
    int w = block_size / 3;
    int l = block_size / 10;

    if (col != 0) // left part
    {
        painter->drawLine(c - l, r - w, c - l, r - l); //  |
        painter->drawLine(c - w, r - l, c - l, r - l); // --
        painter->drawLine(c - w, r + l, c - l, r + l); // --
        painter->drawLine(c - l, r + w, c - l, r + l); //  |
    }
    if (col != 8) // right part
    {
        painter->drawLine(c + l, r - w, c + l, r - l); //   |
        painter->drawLine(c + w, r - l, c + l, r - l); //   --
        painter->drawLine(c + l, r + w, c + l, r + l); //   --
        painter->drawLine(c + w, r + l, c + l, r + l); //   |
    }
}

void MainWindow::paintBlock(int id) const
{
    if (!blocks[id]->is_live())
    {
        return;
    }

    QPen pen;

    // if chosen
    if (blocks[id]->is_checked())
    {
        painter->setBrush(COLOR_CHESS_CHECKED);
    }
    else
    {
        painter->setBrush(COLOR_CHESS_UNCHECKED);
    }

    // red or black
    if (id < 16) // red
    {
        pen.setColor(COLOR_CHESS_RED);
    }
    else // black
    {
        pen.setColor(COLOR_CHESS_BLACK);
    }

    //circle
    pen.setWidth(4);
    painter->setPen(pen);


    // outline circle
    painter->drawEllipse(QPoint(LEFT + block_size * blocks[id]->col(), UP + block_size * blocks[id]->row()),
                                block_size / 2 - 2, block_size / 2 - 2);
    pen.setWidth(2);
    painter->setPen(pen);

    // inner circle
    painter->drawEllipse(QPoint(LEFT + block_size * blocks[id]->col(), UP + block_size * blocks[id]->row()),
                                block_size / 2 - 7, block_size / 2 - 7);
    // text part
    painter->setFont(QFont("consolas", block_size * 4 / 10));
    painter->drawText(QRect(LEFT + block_size * blocks[id]->col() - block_size / 2 + 15,
                            UP   + block_size * blocks[id]->row() - block_size / 2 +  5,
                            block_size, block_size), blocks[id]->text());
}

inline void MainWindow::change_color()
{
    if(color == block::RED)
    {
        color = block::BLACK;
    }
    else {
        color = block::RED;
    }
}

void MainWindow::on_pushButton_start_clicked()
{
    if (start_game == false)
    {
        start_game = true;
        ui->pushButton_start->setText("restart");
    }
    else if (ui->pushButton_start->text() == QString("restart"))
    {
        if (QMessageBox::question(this, "Warning ! ", "Restart a new game?", "No", "Yes") != 1)
        {
            return;
        }
        start_game = false;
        ui->pushButton_start->setText("start");
    }

    // reset the chess
    for (int i = 0; i < block::NUM; ++i)
    {
        blocks[i]->reinit();
    }
    color = block::RED;
    chess_id = -1;

    update();
}

void MainWindow::on_pushButton_back_clicked() // retract
{
    if (block_tmps.empty())
    {
        QMessageBox::warning(this,tr("Warning !"),tr("bro , you did not move anything !"));
        return;
    }

    //  last step
    blocks[ block_tmps.top().second.id ]->set_pos(  block_tmps.top().second.row , block_tmps.top().second.col );
    // relife
    if (block_tmps.top().first.id != -1)
    {
        blocks[block_tmps.top().first.id]->live();
        blocks[block_tmps.top().first.id]->set_pos( block_tmps.top().first.row, block_tmps.top().first.col );
    }

    block_tmps.pop();

    change_color();

    update();
}

void MainWindow::on_pushButton_edit_clicked()
{
   int id  = QInputDialog::getInt(this,"","id:");
   int  y  = QInputDialog::getInt(this,"","y:" );
   int  x  = QInputDialog::getInt(this,"","x:" );
   blocks[id]->edit_init(y,x);
}

void MainWindow::on_pushButton_register_clicked()
{
  for(int id=0;id<32;id++)
  {
      blocks[id]->edit_init(id/4+1,id%4+12);
  }
}


bool MainWindow::moveable(int id, int row, int col) const
{

    switch(blocks[id]->type())
    {
    case block::BING:
        return moveable_bing(id, row, col);
    case block::JU:
        return moveable_ju(id, row, col);
    case block::JIANG:
        return moveable_jiang(id, row, col);
    case block::PAO:
        return moveable_pao(id, row, col);
    case block::SHI:
        return moveable_shi(id, row, col);
    case block::XIANG:
        return moveable_xiang(id, row, col);
    case block::MA:
        return moveable_ma(id, row, col);
    default:
        break;
    }
    return false;
}

// jiang
bool MainWindow::moveable_jiang(int id, int row, int col) const
{
    if (blocks[4]->col() == blocks[20]->col()) // in the same col
    {

        if ((blocks[20]->row() == row && blocks[20]->col() == col) ||
            (blocks[4]->row() == row && blocks[4]->col() == col))
        {
            for (int i = 0; i < 32; ++i)
            {
                if ((blocks[i]->col() == col) &&
                    ((blocks[i]->row() > row && blocks[i]->row() < blocks[id]->row()) ||
                     (blocks[i]->row() < row && blocks[i]->row() > blocks[id]->row())))
                {
                    return false;
                }
            }
            return true;
        }
    }

    // only move in X+
    if (id == 4) // red
    {
        if (row < 0 || row > 2 || col < 3 || col > 5)
        {
            return false;
        }
    }
    else // black
    {
        if (row < 7 || row > 9 || col < 3 || col > 5)
        {
            return false;
        }
    }

    int flag = abs(row - blocks[id]->row()) * 10 + abs(col - blocks[id]->col());

    // only move one block
    if (flag != 1 && flag != 10)
    {
        return false;
    }

    return true;
}

// bing
bool MainWindow::moveable_bing(int id, int row, int col) const
{
    int flag = abs(row - blocks[id]->row()) * 10 +  abs(col - blocks[id]->col());

    // jump 1
    if (flag != 1 && flag != 10)
    {
        return false;
    }
    // if red
    if (blocks[id]->color() == block::RED)
    {
        // can't  move back
        if (row < blocks[id]->row())
        {
            return false;
        }
        // only forward before cross river
        if (blocks[id]->row() <= 4 && flag != 10)
        {
            return false;
        }

    }
    else // if black
    {
        // can't move back
        if (row > blocks[id]->row())
        {
            return false;
        }
        // only forward before cross river
        if (blocks[id]->row() >= 5 && flag != 10)
        {
            return false;
        }
    }

    return true;
}

// ju
bool MainWindow::moveable_ju(int id, int row, int col) const
{
    // only can move in a line
    if (row != blocks[id]->row() && col != blocks[id]->col()) // if move u-d and l-r at the same time
    {
        return false;
    }

    if (row == blocks[id]->row()) // left-right
    {
        for (int i = 0; i < 32; ++i)
        {
            if ((blocks[i]->row() == row) &&
                ((blocks[i]->col() > col && blocks[i]->col() < blocks[id]->col()) ||
                 (blocks[i]->col() < col && blocks[i]->col() > blocks[id]->col())    ) )
            {
                return false;
            }
        }
    }
    else if (col == blocks[id]->col()) // up-down
    {
        for (int i = 0; i < 32; ++i)
        {
            if ((blocks[i]->col() == col) &&
                ((blocks[i]->row() > row && blocks[i]->row() < blocks[id]->row()) ||
                 (blocks[i]->row() < row && blocks[i]->row() > blocks[id]->row())   ) )
            {
                return false;
            }
        }
    }
    return true;
}

// pao  // bug
bool MainWindow::moveable_pao(int id, int row, int col) const
{
    int cnt = 0;
    if (row == blocks[id]->row()) // l-r
    {
        for (int i = 0; i < 32; ++i)
        {
            if ((blocks[i]->row() == row) &&
                (blocks[i]->col() > col && blocks[i]->col() < blocks[id]->col()))  //  goal - x - pao
            {
                // if have 2 or more piece between them
                cnt++;
                if(blocks[i]->is_live()==false)
                {
                    cnt--;
                }
                if (cnt > 1)
                {
                    return false;
                }
            }
            else if ((blocks[i]->row() == row) &&
                   (blocks[i]->col() < col && blocks[i]->col() > blocks[id]->col())) //  pao  - x - goal
            {
                cnt++;
                if(blocks[i]->is_live()==false)
                {
                    cnt--;
                }
                if (cnt > 1)
                {
                    return false;
                }
            }
        }
    }
    else if (col == blocks[id]->col()) // u-d
    {
        for (int i = 0; i < 32; ++i)
        {
            if ((blocks[i]->col() == col) &&
                (blocks[i]->row() > row && blocks[i]->row() < blocks[id]->row() ))
            {
                // if have 2 or more piece between them
                cnt++;
                if(blocks[i]->is_live()==false)
                {
                    cnt--;
                }
                if (cnt > 1)
                {
                    return false;
                }
            }
            else if ((blocks[i]->col() == col) &&
                 (blocks[i]->row() < row && blocks[i]->row() > blocks[id]->row() ))
            {
                // if have 2 or more piece between them
                cnt++;
                if(blocks[i]->is_live()==false)
                {
                    cnt--;
                }
                if (cnt > 1)
                {
                    return false;
                }
            }
        }
    }

    // if only one piece between them
    if (cnt == 1)
    {
        for (int i = 0; i < 32; ++i)
        {
            if (blocks[i]->col() == col && blocks[i]->row() == row)
            {
                return true;
            }
        }
        return false;
    }

    // if no chess between them
    for (int i = 0; i < 32; ++i)
    {
        if (blocks[i]->col() == col && blocks[i]->row() == row)
        {
            return false;
        }
    }
    return moveable_ju(id, row, col);
}

// shi
bool MainWindow::moveable_shi(int id, int row, int col) const
{
    //  only move in X+
    if (blocks[id]->color() == block::RED) // red
    {
        if (row < 0 || row > 2 || col < 3 || col > 5)
        {
            return false;
        }
    }
    else // black
    {
        if (row < 7 || row > 9 || col < 3 || col > 5)
        {
            return false;
        }
    }

    int flag = abs(row - blocks[id]->row()) * 10 + abs(col - blocks[id]->col());

    // only move X
    if (flag != 11)
    {
        return false;
    }

    return true;
}

// ma
bool MainWindow::moveable_ma(int id, int row, int col) const
{
    int flag = abs(row - blocks[id]->row()) * 10 + abs(col - blocks[id]->col());

    // O------
    // |  |  |
    // ------O

    if (flag != 12 && flag != 21)
    {
        return false;
    }

    // have piece beside ma
    // O--X---
    // |  |  |
    // X-----O

    for (int i = 0; i < 32; ++i)
    {
        if (abs(row - blocks[id]->row()) == 1)
        {
            if (blocks[i]->row() == blocks[id]->row() &&
                blocks[i]->col() == (col + blocks[id]->col()) / 2)
            {
                return false;
            }
        }
        else
        {
            if (blocks[i]->col() == blocks[id]->col() &&
                blocks[i]->row() == (row + blocks[id]->row()) / 2)
            {
                return false;
            }
        }
    }

    return true;
}

// xiang
bool MainWindow::moveable_xiang(int id, int row, int col) const
{
    // can't cross river
    if (blocks[id]->color() == block::RED) // red
    {
        if (row > 4)
        {
            return false;
        }
    }
    else // black
    {
        if (row < 5)
        {
            return false;
        }
    }

    int flag = abs(row - blocks[id]->row()) * 10 + abs(col - blocks[id]->col());

    //  O_________
    //  |    |    |
    //  ___________
    //  |    |    |
    //   _________O

    if (flag != 22)
    {
        return false;
    }

    //  O_________
    //  |    |    |
    //  _____X_____
    //  |    |    |
    //   _________O

    for (int i = 0; i < 32; ++i)
    {
        if (blocks[i]->col() == (col + blocks[id]->col()) / 2 &&
            blocks[i]->row() == (row + blocks[id]->row()) / 2)
        {
            return false;
        }
    }

    return true;
}
