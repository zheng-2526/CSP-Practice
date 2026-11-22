# CSP Practice · Qt Widgets 练习

三个独立的 C++17 / Qt Widgets 桌面程序，用于练习界面设计、事件处理和应用逻辑。

Three standalone Qt Widgets projects for practising desktop development in C++17.

| 项目 | 功能 |
| --- | --- |
| [Qt-Calculator](Qt-Calculator/README.md) | 支持键盘输入、嵌套括号和运算优先级，以及取模、平方根和倒数。 |
| [Qt-Puzzle-Game](Qt-Puzzle-Game/README.md) | 拖动交换拼图块，支持 3×3、4×4、5×5 和本地图片。点击“开始挑战”后进入 15 秒倒计时。 |
| [Qt-Multi-Document-Editor](Qt-Multi-Document-Editor/README.md) | 在同一窗口中编辑多个文档，支持文字格式、查找替换和窗口平铺。Windows 支持 RTF 读写，也可使用 TXT 或 HTML。 |

## 用 Qt Creator 打开

在 Windows 上使用 Qt 6.9.1、MinGW 64 位和 CMake 3.21 或更新版本。可从 Qt 安装器中选择对应的 Qt、Qt Creator、MinGW 和 CMake 组件。

1. 在 Qt Creator 选择“文件 → 打开文件或项目”，打开目标项目目录里的 `CMakeLists.txt`。每个目录都可以单独打开。
2. 选择与 Qt 安装版本和编译器匹配的套件，例如 **Qt 6.9.1 MinGW 64-bit**，配置后按 `Ctrl+R` 构建运行。
3. 要可视化修改控件，在项目树中双击该项目的 `mainwindow.ui`。计算器和拼图游戏可在 Designer 的对象查看器中找到 `pages`，再切换到相应页面。

## 在 PowerShell 中构建和测试

打开目标项目目录。随附脚本适用于 Qt 6.9.1 MinGW 安装在 `C:\Qt` 的环境。

```powershell
.\build.ps1
.\run.ps1
```

`build.ps1` 会配置、构建并执行 CTest。如安装路径不同，可以在 Qt Creator 里选用对应套件。也可在已配置好编译器和 CMake 的终端中执行以下命令，按实际安装位置替换示例路径。

```powershell
cmake -S . -B build "-DCMAKE_PREFIX_PATH=C:/Qt/6.9.1/mingw_64"
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

运行程序及界面测试时，确保对应 Qt 套件的 `bin` 目录和编译器运行库在 `PATH` 中。

各项目 README 提供操作方法、文件结构和功能范围。
