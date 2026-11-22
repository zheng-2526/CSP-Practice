# Qt 拼图游戏

这是一个使用 Qt 6、C++17 和 Qt Designer `.ui` 文件编写的限时拼图游戏。

简单为 3×3，中等为 4×4，困难为 5×5。进入游戏、重新打乱、切换图片或难度后，图片处于锁定的准备状态，倒计时停在 15 秒。点击右侧“开始挑战”才开始计时并允许拖动，挑战期间开始按钮禁用。查看原图仍然计时。限时内复原才成功，超时会判定失败并锁定拼图区；重排后可以再次点击“开始挑战”。

## 功能

- 欢迎页面和作者署名“郑亚泽”
- 鼠标拖动两个拼图块进行交换
- 查看原图
- 试玩新图和重新打乱
- 选择本地图片并自动适应拼图区
- 自动判断拼图是否完成

游戏自带一张原创示例图，位于 `images/puzzle_sample.png`。通过“切换图片”可以选择自己的 PNG、JPEG 或 BMP 图片。

## 在 Qt Creator 中打开

1. 打开 Qt Creator。
2. 选择“文件 -> 打开文件或项目”。
3. 打开本目录中的 `CMakeLists.txt`。
4. 双击 `mainwindow.ui` 进入 Designer 设计模式。

`puzzleBoard` 是提升为 `PuzzleWidget` 的自定义控件。可以在 Designer 中调整它的位置和大小，游戏绘制与拖动逻辑位于 `puzzlewidget.cpp`。

## 构建

安装环境见[仓库的构建说明](../README.md#在-powershell-中构建和测试)。

在 PowerShell 中运行：

```powershell
.\build.ps1
```

运行程序：

```powershell
.\run.ps1
```
