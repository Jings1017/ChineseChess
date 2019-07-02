#include "block.h"

/*block::block(QObject *parent) : QObject(parent), id(-1)
{

}*/

block::block(int id)
 : _id(id), _live(true), _checked(false)
{
    init();
}

const QString
block::text() const
{
    if (_id < 16)
    {
        switch(_type)
        {
        case JU: return "俥";
        case MA: return "傌";
        case XIANG: return "相";
        case SHI: return "仕";
        case JIANG: return "帥";
        case PAO: return "炮";
        default: return "兵";
        }
    }
    else
    {
        switch(_type)
        {
        case JU: return "車";
        case MA: return "馬";
        case XIANG: return "象";
        case SHI: return "士";
        case JIANG: return "將";
        case PAO: return "砲";
        default: return "卒";
        }
    }
}


void block::init()
{
    switch (_id) {
    case 0:
       _row = 0;
       _col = 0;
       _type = block::JU;
       _color = RED;
       break;
    case 1:
       _row = 0;
       _col = 1;
       _type = block::MA;
       _color = RED;
       break;
    case 2:
       _row = 0;
       _col = 2;
       _type = block::XIANG;
       _color = RED;
       break;
    case 3:
       _row = 0;
       _col = 3;
       _type = block::SHI;
       _color = RED;
       break;
    case 4:
       _row = 0;
       _col = 4;
       _type = block::JIANG;
       _color = RED;
       break;
    case 5:
       _row = 0;
       _col = 5;
       _type = block::SHI;
       _color = RED;
       break;
    case 6:
       _row = 0;
       _col = 6;
       _type = block::XIANG;
       _color = RED;
       break;
    case 7:
       _row = 0;
       _col = 7;
       _type = block::MA;
       _color = RED;
       break;
    case 8:
       _row = 0;
       _col = 8;
       _type = block::JU;
       _color = RED;
       break;
    case 9:
       _row = 2;
       _col = 1;
       _type = block::PAO;
       _color = RED;
       break;
    case 10:
       _row = 2;
       _col = 7;
       _type = block::PAO;
       _color = RED;
       break;
    case 11:
       _row = 3;
       _col = 0;
       _type = block::BING;
       _color = RED;
       break;
    case 12:
       _row = 3;
       _col = 2;
       _type = block::BING;
       _color = RED;
       break;
    case 13:
       _row = 3;
       _col = 4;
       _type = block::BING;
       _color = RED;
       break;
    case 14:
       _row = 3;
       _col = 6;
       _type = block::BING;
       _color = RED;
       break;
    case 15:
       _row = 3;
       _col = 8;
       _type = block::BING;
       _color = RED;
       break;
    case 16:
       _row = 9;
       _col = 0;
       _type = block::JU;
       _color = BLACK;
       break;
    case 17:
       _row = 9;
       _col = 1;
       _type = block::MA;
       _color = BLACK;
       break;
    case 18:
       _row = 9;
       _col = 2;
       _type = block::XIANG;
       _color = BLACK;
       break;
    case 19:
       _row = 9;
       _col = 3;
       _type = block::SHI;
       _color = BLACK;
       break;
    case 20:
       _row = 9;
       _col = 4;
       _type = block::JIANG;
       _color = BLACK;
       break;
    case 21:
       _row = 9;
       _col = 5;
       _type = block::SHI;
       _color = BLACK;
       break;
    case 22:
       _row = 9;
       _col = 6;
       _type = block::XIANG;
       _color = BLACK;
       break;
    case 23:
       _row = 9;
       _col = 7;
       _type = block::MA;
       _color = BLACK;
       break;
    case 24:
       _row = 9;
       _col = 8;
       _type = block::JU;
       _color = BLACK;
       break;
    case 25:
       _row = 7;
       _col = 1;
       _type = block::PAO;
       _color = BLACK;
       break;
    case 26:
       _row = 7;
       _col = 7;
       _type = block::PAO;
       _color = BLACK;
       break;
    case 27:
       _row = 6;
       _col = 0;
       _type = block::BING;
       _color = BLACK;
       break;
    case 28:
       _row = 6;
       _col = 2;
       _type = block::BING;
       _color = BLACK;
       break;
    case 29:
       _row = 6;
       _col = 4;
       _type = block::BING;
       _color = BLACK;
       break;
    case 30:
       _row = 6;
       _col = 6;
       _type = block::BING;
       _color = BLACK;
       break;
    case 31:
       _row = 6;
       _col = 8;
       _type = block::BING;
       _color = BLACK;
       break;
    }

}

void block::reinit()
{
    _live = true;
    _checked = false;
    init();
}

void block::edit_init(int y, int x)
{
     _row = y;
     _col = x;
}

void block::register_init()
{
    for(int id =1 ; id<32 ; id++)
    {
      _row = 0;
      _col = 12;

    }
}

void block::set_pos(int row, int col)
{
    if (col > COL || col < 0)
    {
        throw(std::out_of_range("illegal !"));
    }
    if (row > ROW || row < 0)
    {
        throw(std::out_of_range("illegal !"));
    }
    _col = col;
    _row = row;
}
