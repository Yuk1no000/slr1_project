#include "lexer.h"
#include <cctype>

// 构造函数初始化
Lexer::Lexer(string s) : input(s), pos(0) {}

/**
 * @brief 将输入字符串分解为 Token 列表
 * 完善后的词法分析器，支持：
 * 1. 关键字 (while, if, else, int, float, return)
 * 2. 标识符 (id)
 * 3. 数字 (num): 支持整数、小数、正负数
 * 4. 运算符: +, -, *, /, =, >, <, >=, <=, ==, !=
 * 5. 界符: (, ), {, }, ;
 */
vector<Token> Lexer::tokenize() 
{
    vector<Token> tokens;
    while (pos < input.length()) 
    {
        // 1. 跳过空白字符 (空格, Tab, 换行)
        while (pos < input.length() && isspace(input[pos])) 
            pos++;
        if (pos >= input.length()) 
            break;
        
        char c = input[pos];
        
        // 2. 处理字母开头的单词 (关键字或标识符)
        if (isalpha(c) || c == '_') 
        {
            string s;
            // 读取完整的单词 (字母、数字、下划线)
            while (pos < input.length() && (isalnum(input[pos]) || input[pos] == '_')) 
                s += input[pos++];
            
            // 区分关键字和普通标识符
            if (s == "while") tokens.push_back({"while", "while"});
            else if (s == "if") tokens.push_back({"if", "if"});
            else if (s == "else") tokens.push_back({"else", "else"});
            else if (s == "int") tokens.push_back({"int", "int"});
            else if (s == "float") tokens.push_back({"float", "float"});
            else if (s == "return") tokens.push_back({"return", "return"});
            else tokens.push_back({"id", s});
        } 
        // 3. 处理数字 (整数、小数、正负数)
        // 判断是否为数字开头，或者是正负号开头且后面跟着数字（且前一个token不是id/num/右括号，表示是前缀符号）
        else if (isdigit(c) || ((c == '+' || c == '-') && pos + 1 < input.length() && isdigit(input[pos+1]))) 
        {
            // 简单的上下文判断：如果前一个token是id、num、)或}，那么+ -应该是运算符而不是符号位
            bool isSign = false;
            if (c == '+' || c == '-') {
                if (!tokens.empty()) {
                    string lastType = tokens.back().type;
                    if (lastType == "id" || lastType == "num" || lastType == ")" || lastType == "}") {
                        isSign = false;
                    } else {
                        isSign = true;
                    }
                } else {
                    isSign = true; // 第一个token
                }
            }

            if (isSign || isdigit(c)) {
                string s;
                if (isSign) s += input[pos++]; // 吃掉符号
                
                bool hasDot = false;
                while (pos < input.length() && (isdigit(input[pos]) || input[pos] == '.')) {
                    if (input[pos] == '.') {
                        if (hasDot) break; // 已经有一个小数点了
                        hasDot = true;
                    }
                    s += input[pos++];
                }
                tokens.push_back({"num", s}); 
            } else {
                // 是运算符 + 或 -
                string s(1, c);
                tokens.push_back({s, s});
                pos++;
            }
        } 
        
        // 4. 处理符号 (运算符和界符)
        else 
        {
            string s(1, c);
            // 预读下一个字符，处理双字符运算符
            if (pos + 1 < input.length()) {
                char next = input[pos+1];
                if (c == '>' && next == '=') { s = ">="; pos++; }
                else if (c == '<' && next == '=') { s = "<="; pos++; }
                else if (c == '=' && next == '=') { s = "=="; pos++; }
                else if (c == '!' && next == '=') { s = "!="; pos++; }
            }
            
            // 将该字符(或字符串)转化为Token
            tokens.push_back({s, s});
            pos++;
        }
    }
    // 添加结束符 Token，表示输入结束
    tokens.push_back({"#", "#"}); 
    return tokens;
}
