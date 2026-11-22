#include <cassert>

#include "puzzlemodel.h"

int main() {
    PuzzleModel model;
    assert(model.isSolved());

    model.swapTiles(0, 1);
    assert(!model.isSolved());
    assert(model.tileAt(0) == 1);
    assert(model.tileAt(1) == 0);

    model.swapTiles(0, 1);
    assert(model.isSolved());

    model.shuffle();
    assert(!model.isSolved());

    for(int n = 3; n <= 5; n++) {
        model.setDifficulty(n);
        assert(model.rowCount == n);
        assert(model.tileCount == n * n);
        assert(model.isSolved());
        model.shuffle();
        assert(!model.isSolved());
        // 逐块复原：同时检查打乱后没有丢失或重复拼图块。
        for(int i = 0; i < model.tileCount; i++) {
            int j = i;
            while(j < model.tileCount && model.tileAt(j) != i) j++;
            assert(j < model.tileCount);
            model.swapTiles(i, j);
        }
        assert(model.isSolved());
    }

    return 0;
}
