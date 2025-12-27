#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>

using namespace std;

/**
 * @brief 四元式结构体
 * 用于表示中间代码，格式为 (操作符, 操作数1, 操作数2, 结果)
 * 例如: a = b + c 对应的四元式为 (+, b, c, a)
 */
struct Quad 
{
    string op;      // 操作符 (如 "+", ">", "jfalse", "label")
    string arg1;    // 第一个操作数
    string arg2;    // 第二个操作数 (如果是一元运算则为 "-")
    string result;  // 结果变量或跳转目标标号

    // 将四元式转换为字符串形式，方便输出调试
    string toString() const 
    {
        return "(" + op + ", " + arg1 + ", " + arg2 + ", " + result + ")";
    }
};

/**
 * @brief 产生式结构体
 * 表示文法中的一条规则，如 S -> while ( C ) { S }
 */
struct Production 
{
    int id;             // 产生式的唯一编号
    string lhs;         // 产生式左部 (Left Hand Side)，即非终结符
    vector<string> rhs; // 产生式右部 (Right Hand Side)，符号序列
    
    string toString() const 
    {
        string s = lhs + " ->";
        for (const auto& sym : rhs) s += " " + sym;
        return s;
    }
};

/**
 * @brief LR(0) 项目结构体
 * 表示一个带有“圆点”的产生式，用于跟踪分析进度
 * 例如: A -> alpha . beta 表示已经匹配了 alpha，期望接下来匹配 beta
 */
struct Item 
{
    int prodIndex; // 该项目对应的产生式在 grammar 数组中的下标
    int dotPos;    // 圆点在产生式右部的位置 (0 表示在最左边)
    
    // 重载 < 运算符，以便 Item 可以作为 std::set 的元素
    bool operator<(const Item& other) const 
    {
        if (prodIndex != other.prodIndex) return prodIndex < other.prodIndex;
        return dotPos < other.dotPos;
    }
    
    // 重载 == 运算符
    bool operator==(const Item& other) const 
    {
        return prodIndex == other.prodIndex && dotPos == other.dotPos;
    }
};

/**
 * @brief 语义属性结构体
 * 存储在语法分析栈中，用于在归约时传递信息
 */
struct Attribute 
{
    string place;       // 变量名、临时变量名或标号名AQ Qz
    vector<Quad> code;  // 该语法成分生成的中间代码序列
};

/**
 * @brief 分析表动作结构体
 * 定义了分析表 (Action Table) 中的条目
 */
struct Action 
{
    char type; // 动作类型: 's'(Shift 移进), 'r'(Reduce 归约), 'a'(Accept 接受), 'e'(Error 错误)
    int val;   // 关联值: 对于 Shift 是目标状态ID，对于 Reduce 是产生式ID
};

/**
 * @brief Token 结构体
 * 词法分析器的输出单元
 */
struct Token 
{
    string type;  // Token 类型 (如 "id", "while", "operator")
    string value; // Token 的实际文本值 (如 "count", "while", "+")
};

#endif
