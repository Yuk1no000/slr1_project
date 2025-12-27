#include "grammar.h"
#include <fstream>
#include <sstream>
#include <algorithm>

// 判断符号是否为终结符
bool GrammarAnalyzer::isTerminal(const string& s) {
    return terminals.count(s);
}

/**
 * @brief 加载文法文件
 * 文件格式: LHS -> RHS (符号间用空格分隔)
 * 例如: S -> while ( C ) { S }
 */
void GrammarAnalyzer::loadGrammar(const string& filename) {
    ifstream file(filename);
    string line;
    int id = 0;
    while (getline(file, line)) 
    {
        if (line.empty()) continue;
        stringstream ss(line);
        string lhs, arrow, sym;
        //lhs -- 产生式左部  arrow -- 箭头  sym -- 产生式右部
        ss >> lhs >> arrow; // 读取左部和箭头
        
        if (id == 0) 
            startSymbol = lhs; // 第一条产生式的左部作为开始符号
        nonTerminals.insert(lhs); //产生式左部为非终结符，加入到非终结符集合中去
        
        //获取产生式右部rhs
        vector<string> rhs;
        while (ss >> sym) 
        {
            rhs.push_back(sym);
        }
        //将id 产生式左部 右部存入语法中
        grammar.push_back({id++, lhs, rhs});
    }
    
    // 遍历所有产生式右部，找出所有未作为左部出现的符号，即为终结符
    for (const auto& prod : grammar) 
    {
        for (const auto& sym : prod.rhs) 
        {
            //find(sym)若返回末尾迭代器(end())，则说明未在set中找到该元素，将其insert进终结符集中
            if (nonTerminals.find(sym) == nonTerminals.end()) 
            {
                terminals.insert(sym);
            }
        }
    }
    terminals.insert("#"); // 显式添加结束符
}

/**
 * @brief 计算 First 集
 * 算法:
 * 1. 若 X 是终结符，First(X) = {X}
 * 2. 若 X -> Y1 Y2 ... Yk，则将 First(Y1) 加入 First(X)
 *    (简化版：本实验文法不涉及 epsilon 推导，逻辑较简单)
 * 3. 重复直到集合不再变化
 * 
 * 针对该文法G
 * S' -> S
    S -> while ( C ) { S }
    S -> id = E
    C -> E > E
    E -> id
 * 整个算法重复三次
 */
void GrammarAnalyzer::computeFirst() 
{
    bool changed = true;
    while (changed) 
    {
        changed = false;
        for (const auto& prod : grammar) 
        {
            string X = prod.lhs;
            size_t oldSize = firstSets[X].size();
            
            if (prod.rhs.empty()) continue;
            
            //获取产生式的右部的第一个字符
            string Y = prod.rhs[0];
            if (isTerminal(Y)) 
            {//如果是产生式右部第一个符号为终结符，那么X的first集就为该终结符
                firstSets[X].insert(Y);
            } 
            else 
            {//如果产生式右部第一个符号是非终结符，那么就将该非终结符的first集加入到X的first集中
                for (const auto& f : firstSets[Y]) 
                {
                    firstSets[X].insert(f);
                }
            }
            
            //当有非终结符的first集更新时，必须要再多循环整个遍历算法一次
            if (firstSets[X].size() > oldSize) 
                changed = true;
        }
    }
}

/**
 * @brief 计算 Follow 集
 * 算法:
 * 1. 将 # 加入 Follow(StartSymbol)
 * 2. 对于产生式 A -> alpha B beta:
 *    将 First(beta) - {epsilon} 加入 Follow(B)
 * 3. 对于产生式 A -> alpha B 或 A -> alpha B beta (且 beta 推导 epsilon):
 *    将 Follow(A) 加入 Follow(B)
 */
void GrammarAnalyzer::computeFollow() 
{
    followSets[startSymbol].insert("#");
    bool changed = true;
    while (changed) 
    {
        changed = false;
        //遍历产生式
        for (const auto& prod : grammar) 
        {
            string A = prod.lhs;
            for (size_t i = 0; i < prod.rhs.size(); ++i) 
            {
                string B = prod.rhs[i];
                //终结符直接跳过
                if (isTerminal(B)) 
                    continue;

                //遍历到一个非终结符B
                size_t oldSize = followSets[B].size();
                //情况1：该终结符不是最后一位
                if (i + 1 < prod.rhs.size()) 
                {
                    // 情况: A -> ... B beta
                    string beta = prod.rhs[i+1];
                    if (isTerminal(beta)) 
                    {
                        //如果B后一位是终结符，那么将其加入到follow集中
                        followSets[B].insert(beta);
                    } 
                    else 
                    {
                    // 情况 A -> ... B B' ...
                    // follow(B) += follow(B')
                    // 如果后一位是非终结符，那么将其follow集添加到B的follow集中来
                        for (const auto& f : firstSets[beta]) 
                        {
                            followSets[B].insert(f);
                        }
                    }
                } 
                else 
                {
                    // 情况: A -> ... B (B 在末尾)
                    // 将 Follow(A) 加入到 Follow(B) || Follow(B) += Follow(A)
                    for (const auto& f : followSets[A]) 
                    {
                        followSets[B].insert(f);
                    }
                }
                
                if (followSets[B].size() > oldSize) 
                    changed = true;
            }
        }
    }
}

/**
 * @brief 计算项目集闭包 Closure(I)
 * 规则:
 * 若 A -> alpha . B beta 在 I 中，则将所有 B -> . gamma 加入 I
 */
set<Item> GrammarAnalyzer::closure(set<Item> I) 
{
    bool changed = true;
    while (changed)
    {
        changed = false;
        set<Item> newItems = I;
        for (const auto& item : I) 
        {
            // 检查圆点后面是否有符号
            if (item.dotPos < grammar[item.prodIndex].rhs.size()) 
            {
                //获取圆点后符号（终结符或者非终结符）
                string B = grammar[item.prodIndex].rhs[item.dotPos];
                // 如果圆点后是非终结符，展开它
                if (nonTerminals.count(B)) 
                {
                    for (const auto& prod : grammar) 
                    {
                        //找到该非终结符的产生式，展开它并加入到newItems中
                        if (prod.lhs == B) 
                        {
                            Item newItem = {prod.id, 0}; // 圆点在开头
                            //去重
                            if (newItems.find(newItem) == newItems.end()) 
                            {
                                newItems.insert(newItem);
                                //添加了新项目集，需要再重新遍历一次
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
        I = newItems;
    }
    return I;
}

/**
 * @brief 计算状态转移 Goto(I, X)
 * 规则:
 * 将 I 中所有圆点后是 X 的项目，圆点后移一位，构成新集合 J，返回 Closure(J)
 */
set<Item> GrammarAnalyzer::gotoState(const set<Item>& I, const string& X) 
{
    set<Item> J;
    for (const auto& item : I) 
    {
        if (item.dotPos < grammar[item.prodIndex].rhs.size()) 
        {
            if (grammar[item.prodIndex].rhs[item.dotPos] == X) 
            {
                J.insert({item.prodIndex, item.dotPos + 1});
            }
        }
    }
    return closure(J);
}

/**
 * @brief 构造 DFA (LR(0) 项目集规范族)
 * 从初始状态开始，不断计算 Goto 生成新状态，直到不产生新状态为止
 */
void GrammarAnalyzer::buildDFA() 
{
    // 1. 初始状态: S' -> . S 的闭包
    Item startItem = {0, 0};//第一个0表示该状态在文法中属于第0个产生式，第二个0表示当前圆点的位置位于最开始处
    set<Item> startSet;
    startSet.insert(startItem);
    startSet = closure(startSet);
    
    states.push_back({0, startSet});//存入第一个状态I0，第一个0表示状态集的编号，第二个表示存入状态集的项目集合
    
    int processed = 0;
    //循环遍历直至无新状态集，判断条件即为Processed == states.size
    while (processed < states.size()) 
    {
        // 收集当前状态所有可能的下一个输入符号
        set<string> nextSymbols;
        for (const auto& item : states[processed].items) 
        {
            //当前项目集中圆点不在最后，将所有圆点后的符号添加到nextSymbols集中
            if (item.dotPos < grammar[item.prodIndex].rhs.size()) 
            {
                nextSymbols.insert(grammar[item.prodIndex].rhs[item.dotPos]);
            }
        }
        
        // 对每个符号计算 Goto
        for (const auto& X : nextSymbols) 
        {
            //对于下一个输入符号，通过GotoState函数获取经过该输入符号的下一个状态集
            set<Item> nextItemSet = gotoState(states[processed].items, X);
            if (nextItemSet.empty()) continue;
            
            // 检查该状态集是否已存在
            int existingStateId = -1;
            for (int i = 0; i < states.size(); ++i) 
            {
                //若存在
                if (states[i].items == nextItemSet) 
                {
                    existingStateId = i;
                    break;
                }
            }
            
            // 若不存在，添加新状态（设置该状态集合的ID和项目集合）
            if (existingStateId == -1) 
            {
                existingStateId = states.size();
                states.push_back({existingStateId, nextItemSet});//第一个id表示状态集的编号In,第二个表示存入状态集的项目集合
            }
            
            // 记录状态集的第三个属性：transitions,记录转移关系 I0--X-->I1
            
            states[processed].transitions[X] = existingStateId;
        }
        processed++;
    }
}

/**
 * @brief 构造 SLR(1) 分析表
 * 结合 DFA 和 Follow 集生成 Action 和 Goto 表
 * 判断该文法是否为SLR(1)分析法，有无冲突(一般无)，返回true
 */
bool GrammarAnalyzer::buildSLRTable() 
{
    for (int i = 0; i < states.size(); ++i) //遍历状态表
    {
        const State& S = states[i];
        
        // 1. 处理 Shift 动作 (移进) 和 GOTO表
        // 根据map<string int> transition状态转移表: 输入符号 -> 目标状态ID
        // 若有转移 state[i] --a--> state[j] 且 a 是终结符，则 Action[i][a] = sj
        for (const auto& trans : S.transitions) 
        {
            string symbol = trans.first;//trans的第一个属性表示终结符
            int target = trans.second;//第二个属性表示下一个状态ID
            // 如果是终结符 填入ACTION表中
            if (isTerminal(symbol)) 
            {
                if (actionTable[i].count(symbol)) 
                {
                    //如果ACTION表中在同一行存在关于终结符symbol的动作(移进/规约)，说明存在移进规约冲突，直接返回false
                    cout << "错误：存在移进规约冲突，位于状态 " << i << " 符号 " << symbol << endl;
                    return false;
                }
                //移进{s,target}.s表示移进，target是下一个状态
                actionTable[i][symbol] = {'s', target};
            } 
            else 
            {
                // 若 a 是非终结符E或者T等等填入GOTO表，则 Goto[i][a] = j 表示状态转移
                // GOTO target下一个状态
                gotoTable[i][symbol] = target;
            }
        }
        
        // 2. 处理 Reduce 动作 (归约)
        // 只有圆点在最后时候才能规约
        // 若项目 A -> alpha . 属于 state[i]，则对 Follow(A) 中的每个符号 a 都进行规约对应产生式的ID，Action[i][a] = r(prod_id)

        // 遍历当前状态下的所有项目集，因为可能不止一个项目
        for (const auto& item : S.items) 
        {
            if (item.dotPos == grammar[item.prodIndex].rhs.size()) 
            { // 圆点在最后
                // 如果项目对应的产生式是第一个产生式的话呢，就直接ACC接受，填入{'a',0}
                if (grammar[item.prodIndex].lhs == startSymbol) //接受
                {
                    // 接受状态: S' -> S .
                    actionTable[i]["#"] = {'a', 0};
                } 
                else //规约
                {
                    //获取该项目在文法中对应的产生式的左部，利用follow集来判断是否存在冲突(rr和sr)
                    string A = grammar[item.prodIndex].lhs;
                    for (const auto& a : followSets[A]) 
                    {
                        if (actionTable[i].count(a)) 
                        {
                            // 冲突检测 ·
                            
                            if (actionTable[i][a].type == 's') 
                                return false; // 移进-归约冲突
                             // 如果表中已经存在动作，且是归约('r')，并且产生式编号不同
                            if (actionTable[i][a].type == 'r' && actionTable[i][a].val != item.prodIndex) 
                                return false; // 归约-归约冲突
                        }
                        //填写action表的规约动作，r表示动作，item.proIndex表示该项目对应的产生式的下标
                        actionTable[i][a] = {'r', item.prodIndex};
                    }
                }
            }
        }
    }

    // 打印 SLR(1) 分析表
    cout << "SLR(1) 分析表:" << endl;
    
    // 收集所有列头
    vector<string> headers;
    for (const auto& t : terminals) headers.push_back(t);
    
    // 收集 Goto 表中出现的非终结符
    set<string> gotoCols;
    for(const auto& stateRow : gotoTable) 
    {
        for(const auto& entry : stateRow.second) 
        {
            gotoCols.insert(entry.first);
        }
    }
    for (const auto& nt : nonTerminals) {
        if (gotoCols.count(nt)) headers.push_back(nt);
    }
    
    // 打印表头
    cout << "State\t";
    for (const auto& h : headers) cout << h << "\t";
    cout << endl;

    // 打印每一行
    for (const auto& state : states) 
    {
        int s = state.id;
        cout << s << "\t";
        for (const auto& h : headers) 
        {
            if (terminals.count(h)) 
            {
                // Action 表
                if (actionTable[s].count(h))
                 {
                    Action act = actionTable[s][h];
                    if (act.type == 's') cout << "s" << act.val;
                    else if (act.type == 'r') cout << "r" << act.val;
                    else if (act.type == 'a') cout << "acc";
                }
            } else {
                // Goto 表
                if (gotoTable[s].count(h)) {
                    cout << gotoTable[s][h];
                }
            }
            cout << "\t";
        }
        cout << endl;
    }

    return true;
}

bool GrammarAnalyzer::build() 
{
    computeFirst();//构建First集
    computeFollow();//构建Follow集
    buildDFA();
    return buildSLRTable();
}
