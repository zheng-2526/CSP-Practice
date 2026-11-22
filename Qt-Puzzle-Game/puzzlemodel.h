#ifndef PUZZLEMODEL_H
#define PUZZLEMODEL_H

#include <vector>

class PuzzleModel {
public:
    int rowCount = 3;
    int tileCount = 9;

    PuzzleModel();

    void reset();
    void setDifficulty(int n);
    void shuffle();
    void swapTiles(int first, int second);
    bool isSolved() const;
    int tileAt(int index) const;

private:
    // order[i] 表示界面第 i 个位置当前放的是原图中的第几块。
    std::vector<int> order;
};

#endif
