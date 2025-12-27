#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include "grammar.h"

/**
 * @brief SLR(1) 语法分析器类
 * 负责执行语法分析过程，并进行语义动作（生成四元式）
 */
class Parser 
{
    GrammarAnalyzer& G; ///< 引用文法分析器，获取分析表和产生式
    int tempCount;      ///< 临时变量计数器 (T1, T2...)
    int labelCount;     ///< 标号计数器 (L1, L2...)
    
    /**
     * @brief 生成新的临时变量
     * @return string 临时变量名，如 "T1"
     */
    string newTemp();

    /**
     * @brief 生成新的标号
     * @return string 标号名，如 "L1"
     */
    string newLabel();
    
public:
    /**
     * @brief 构造函数
     * @param grammar 已经初始化好的文法分析器引用
     */
    Parser(GrammarAnalyzer& grammar);

    /**
     * @brief 执行语法分析
     * @param input 输入的源代码字符串
     * 
     * 过程:
     * 1. 调用 Lexer 将输入转换为 Token 序列
     * 2. 使用状态栈和符号栈进行移进-归约分析
     * 3. 在归约时执行语义动作，生成四元式
     */
    void parse(string input);
};

#endif
