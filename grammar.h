#ifndef GRAMMAR_H
#define GRAMMAR_H

#include "common.h"

/**
 * @brief DFA 状态结构体
 * 代表 LR 分析中的一个状态，包含项目集和转移关系
 */
struct State 
{
    int id;                         // 状态编号
    set<Item> items;                // 该状态包含的 LR(0) 项目集合
    map<string, int> transitions;   // 状态转移表: 输入符号 -> 目标状态ID
};

/**
 * @brief 文法分析器类
 * 负责加载文法、计算 First/Follow 集、构造 DFA 和生成 SLR(1) 分析表
 */
class GrammarAnalyzer 
{
public:
    // 文法相关数据
    vector<Production> grammar;     // 产生式列表
    set<string> terminals;          // 终结符集合
    set<string> nonTerminals;       // 非终结符集合
    string startSymbol;             // 开始符号
    
    // 集合计算结果
    map<string, set<string>> firstSets;  // First 集
    map<string, set<string>> followSets; // Follow 集
    
    // 分析表相关数据
    vector<State> states;                        // DFA 状态集
    map<int, map<string, Action>> actionTable;   // Action 表: [状态][终结符] -> 动作
    map<int, map<string, int>> gotoTable;        // Goto 表: [状态][非终结符] -> 目标状态

    /**
     * 从文件加载文法
     */
    void loadGrammar(const string& filename);

    /**
     * @brief 执行完整的构建流程
     * 包括计算 First/Follow 集，构造 DFA，生成分析表
     * @return 如果成功生成 SLR(1) 表返回 true，否则返回 false
     */
    bool build(); 
    
private:
    
    void computeFirst();  // 计算 First 集
    void computeFollow(); // 计算 Follow 集
    
    
    set<Item> closure(set<Item> I); // 计算项目集闭包
    set<Item> gotoState(const set<Item>& I, const string& X); // 计算状态转移
    void buildDFA();      // 构造 LR(0) 项目集规范族 (DFA)
    bool buildSLRTable(); // 根据 DFA 和 Follow 集构造 SLR(1) 分析表
    
    bool isTerminal(const string& s); // 判断是否为终结符
};

#endif
