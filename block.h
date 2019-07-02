#ifndef BLOCK_H
#define BLOCK_H

/* 使MSVC支持中文 */
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QString>
#include <stdexcept>
class block
{
public:
    enum {COL = 8, ROW = 9, NUM = 32};
    enum TYPE{ JU, MA, XIANG, SHI, JIANG, PAO, BING };
    enum COLOR{ RED, BLACK };
    block(int id);

    void init();
    void reinit();
    void register_init();
    void edit_init(int ,int);
    int col() const { return _col; }
    int row() const { return _row; }
    void set_pos(int row, int col);
    COLOR color() const { return _color; }
    TYPE type() const { return _type; }
    void clicked() { _checked = !_checked; }
    bool is_checked() const { return _checked; }
    bool is_live() const { return _live; }
    void dead() { _live = false; }
    void live() { _live = true; }
    const QString text() const;
private:
    const int _id;
    int _col;
    int _row;
    COLOR _color;
    TYPE _type;
    bool _live;
    bool _checked;
};

#endif // BLOCK_H
