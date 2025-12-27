# SLR(1) Parser & Intermediate Code Generator

这是一个基于 SLR(1) 分析方法的编译器前端实现项目，主要针对 **WHILE 循环语句** 进行语法分析并翻译生成 **四元式** 中间代码。

##  项目功能

- **自定义文法支持**：从 	estfile.txt 加载上下文无关文法。
- **SLR(1) 分析表构建**：
  - 自动计算 First 集和 Follow 集。
  - 构建 LR(0) 项目集规范族 (DFA)。
  - 生成 SLR(1) 分析表 (Action 和 Goto 表)。
- **语法分析**：对源代码进行移进-归约 (Shift-Reduce) 分析。
- **中间代码生成**：在归约过程中执行语义动作，生成四元式序列。

##  文件结构

`	ext
slr_project/
 main.cpp            # 主程序入口，负责流程控制
 grammar.h/cpp       # 文法分析器：负责文法加载、First/Follow集计算、分析表构建
 parser.h/cpp        # 语法分析器：负责执行 SLR(1) 分析过程
 lexer.h/cpp         # 词法分析器：负责将源代码分割为 Token 流
 common.h            # 公共数据结构定义
 testfile.txt        # [输入] 文法定义文件
 source.txt          # [输入] 待分析的源代码文件
 output.txt          # [输出] 分析结果与四元式
``n
##  编译与运行

本项目使用 C++ 编写，建议使用 g++ 进行编译。

### 编译

由于 main.cpp 中直接包含了实现文件（如 #include "grammar.cpp"），你可以直接编译 main.cpp：

`ash
g++ main.cpp -o slr_parser
``n
### 运行

确保目录下存在 	estfile.txt 和 source.txt 文件，然后运行生成的可执行文件：

`ash
# Windows
.\slr_parser.exe

# Linux/Mac
./slr_parser
``n
##  输入示例

### 1. 文法定义 (	estfile.txt)
定义了支持 WHILE 循环和赋值语句的文法：
`	ext
S' -> S
S -> while ( C ) { S }
S -> id = E
C -> E > E
C -> E < E
C -> E == E
E -> id + E
E -> num + E
E -> id
E -> num
``n
### 2. 源代码 (source.txt)
待分析的代码片段：
`c
while ( a > b ) { 
    x = y 
}
``n
##  输出结果

程序运行后将在控制台输出：
1. 加载的文法规则。
2. SLR(1) 分析表构建状态（成功/失败）。
3. DFA 状态集数量。
4. 语法分析过程（移进/归约动作）。
5. 生成的四元式序列。

##  注意事项

- 确保输入文件的编码格式为 UTF-8 或 ANSI，避免读取乱码。
- 文法必须满足 SLR(1) 条件，否则分析表构建会失败并报错。
