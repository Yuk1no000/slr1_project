#ifndef LEXER_H
#define LEXER_H

#include "common.h"

/**
 * @brief 词法分析器类
 * 负责将源代码字符串转换为 Token 序列
 */
class Lexer 
{
    string input; // 输入的源代码字符串
    int pos;      // 当前扫描到的字符位置

public:
    /**
     * @brief 构造函数
     * @param s 源代码字符串
     */
    Lexer(string s);

    /**
     * @brief 执行词法分析
     * @return 解析出的 Token 向量
     */
    vector<Token> tokenize();
};

#endif
