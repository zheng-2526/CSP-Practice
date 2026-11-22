#include "puzzlemodel.h"

#include <algorithm>
#include <numeric>
#include <random>

PuzzleModel::PuzzleModel() {
    reset();
}

void PuzzleModel::reset() {
    order.resize(tileCount);
    std::iota(order.begin(), order.end(), 0);
}

void PuzzleModel::setDifficulty(int n) {
    if(n < 3 || n > 5) return;
    rowCount = n;
    tileCount = n * n;
    reset();
}

void PuzzleModel::shuffle() {
    static std::mt19937 randomEngine(std::random_device{}());
    std::shuffle(order.begin(), order.end(), randomEngine);

    // 打乱后若碰巧仍是原顺序，交换前两块，保证新游戏确实被打乱。
    if(isSolved()) std::swap(order[0], order[1]);
}

void PuzzleModel::swapTiles(int first, int second) {
    if(first < 0 || first >= tileCount) return;
    if(second < 0 || second >= tileCount) return;
    std::swap(order[first], order[second]);
}

bool PuzzleModel::isSolved() const {
    for(int i = 0; i < tileCount; i++) {
        if(order[i] != i) return false;
    }
    return true;
}

int PuzzleModel::tileAt(int index) const {
    if(index < 0 || index >= tileCount) return -1;
    return order[index];
}
