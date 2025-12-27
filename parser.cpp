#include "parser.h"
#include "lexer.h"
#include <stack>
#include <fstream>

Parser::Parser(GrammarAnalyzer& grammar) : G(grammar), tempCount(0), labelCount(0) {}

string Parser::newTemp() 
{ 
    return "T" + to_string(++tempCount); 
}
string Parser::newLabel() 
{ 
    return "L" + to_string(++labelCount); 
}

/**
 * @brief 核心分析函数
 * 
 * 算法流程:
 * 1. 初始化状态栈(压入0)和符号栈
 * 2. 循环读取输入符号 a
 * 3. 查 Action 表 Action[栈顶状态][a]
 *    - Shift(s): 状态压栈，符号压栈，读头前移
 *    - Reduce(r): 弹出 |RHS| 个状态和符号，查 Goto 表压入新状态，执行语义动作
 *    - Accept(a): 分析成功，输出四元式
 *    - Error: 报错
 */
void Parser::parse(string input) 
{
    //初始化词法分析器，获得tokens
    Lexer lexer(input);
    vector<Token> tokens = lexer.tokenize();
    
    stack<int> stateStack;       // 状态栈
    stack<Attribute> symbolStack;   // 符号栈 (存储语义属性)，
                                    // 包含两个属性：place为变量名、标号名；code为四元式数组，用于输出
    
    stateStack.push(0); // 初始状态
    int ip = 0; // tokens数组指针，用来逐行分析每个token种别码
    
    cout << "正在分析: " << input << endl;
    cout << "步骤\t状态栈\t\t符号\t动作" << endl;
    int step = 0;

    while (true) 
    {
        int s = stateStack.top();//获取当前状态
        string a = tokens[ip].type;  //token的第一个属性id,while,{,},(,)
        string val = tokens[ip].value;//该token具体数值:数字，字母,while,{,},(,)
        
        // 打印分析过程
        cout << ++step << "\t" << s << "\t\t" << val << "\t";

        // 查表失败，报错
        if (G.actionTable[s].find(a) == G.actionTable[s].end()) 
        {
            cout << "错误" << endl;
            cout << "语法错误，在符号 " << val << " 处" << endl;
            return;
        }
        
        //下一步动作--act
        // 根据当前状态栈栈顶s和符号栈栈顶a，查表决定下一步动作
        Action act = G.actionTable[s][a];
        
        if (act.type == 's') 
        { // 移进动作
            cout << "移进 " << act.val << endl;
            stateStack.push(act.val);//val 对于 Shift 是目标状态ID，对于 Reduce 是产生式ID
            Attribute attr;
            attr.place = val; // 终结符的 place 属性就是其词法值
            symbolStack.push(attr); //将当前符号的值压入符号栈中
            ip++;//token数组++，读取下一个token

        } 
        else if (act.type == 'r') 
        { // 归约动作
            int prodId = act.val;//val 对于 Shift 是目标状态ID，对于 Reduce 是产生式ID
            Production prod = G.grammar[prodId];//获取规约的那条产生式prod
            cout << "归约 " << prod.toString() << endl;
            int len = prod.rhs.size();//获取规约的数量
            
            // 弹出右部符号对应的属性，准备进行语义计算
            // 弹出对应产生式右部数量的状态栈和符号栈，符号栈的弹出用Attribute数组rhsAttrs记录下来
            vector<Attribute> rhsAttrs(len);
            for (int i = len - 1; i >= 0; --i) 
            {
                stateStack.pop();
                rhsAttrs[i] = symbolStack.top();
                symbolStack.pop();
            }
            
            // 规约完后根据状态栈顶ID和产生式左部非终结符符号跳转（GOTO）到对应状态中
            // 状态转移: Goto[当前栈顶][LHS]
            int t = stateStack.top();
            stateStack.push(G.gotoTable[t][prod.lhs]);//将跳转后的ID压入状态栈中
            
            Attribute lhsAttr; // 产生式左部的属性
            
            // --- 语义动作 (Semantic Actions) ---
            // 根据不同的产生式，生成对应的四元式代码
            
            // 产生式: S -> while ( C ) { S }
            // 逻辑:
            // 1. 生成两个标号 startLabel, exitLabel
            // 2. 代码结构:
            //    startLabel:
            //    (C 的代码)
            //    if C is false goto exitLabel
            //    (S 的代码)
            //    goto startLabel
            //    exitLabel:

            // 填该Push进符号栈的符号lhsAttr也就是规约产生式左部符号
            if (prod.rhs.size() == 7 && prod.rhs[0] == "while") 
            {
                Attribute C = rhsAttrs[2];
                Attribute S1 = rhsAttrs[5];
                
                string startLabel = newLabel();
                string exitLabel = newLabel();
                
                lhsAttr.code.push_back({"label", "-", "-", startLabel});//放置循环开始的标签L1，方便跳转
                lhsAttr.code.insert(lhsAttr.code.end(), C.code.begin(), C.code.end());//插入条件C的代码
                lhsAttr.code.push_back({"jfalse", C.place, "-", exitLabel});//如果C为假，跳转到出口L2
                lhsAttr.code.insert(lhsAttr.code.end(), S1.code.begin(), S1.code.end());//C为真，执行S1代码
                lhsAttr.code.push_back({"jump", "-", "-", startLabel});//生成无条件跳转指令，回到开头L1中
                lhsAttr.code.push_back({"label", "-", "-", exitLabel});//循环退出的标签L2
            }
            // 产生式: S -> id = E
            // 逻辑: 生成赋值四元式 (=, E.place, -, id.place)
            else if (prod.rhs.size() == 3 && prod.rhs[1] == "=") 
            {
                Attribute id = rhsAttrs[0];
                Attribute E = rhsAttrs[2];
                
                lhsAttr.code = E.code; // 继承 E 的代码（如果E = a+b这种复杂形式表达式时）
                lhsAttr.code.push_back({"=", E.place, "-", id.place});
            }
            // 产生式: C -> E > E
            // 逻辑: 生成比较四元式 (>, E1.place, E2.place, newTemp)
            else if (prod.rhs.size() == 3 && prod.rhs[1] == ">") 
            {
                Attribute E1 = rhsAttrs[0];
                Attribute E2 = rhsAttrs[2];
                
                lhsAttr.place = newTemp();//将计算结果临时存放在临时变量Tn中
                //当E1和E2均是复杂表达式时
                lhsAttr.code = E1.code;
                lhsAttr.code.insert(lhsAttr.code.end(), E2.code.begin(), E2.code.end());
                //添加四元式，结果存放在临时变量newTemp()中
                lhsAttr.code.push_back({">", E1.place, E2.place, lhsAttr.place});
            }
            // 产生式: C -> E < E
            else if (prod.rhs.size() == 3 && prod.rhs[1] == "<") 
            {
                Attribute E1 = rhsAttrs[0];
                Attribute E2 = rhsAttrs[2];
                
                lhsAttr.place = newTemp();
                lhsAttr.code = E1.code;
                lhsAttr.code.insert(lhsAttr.code.end(), E2.code.begin(), E2.code.end());
                lhsAttr.code.push_back({"<", E1.place, E2.place, lhsAttr.place});
            }
            // 产生式: C -> E == E
            else if (prod.rhs.size() == 3 && prod.rhs[1] == "==") 
            {
                Attribute E1 = rhsAttrs[0];
                Attribute E2 = rhsAttrs[2];
                
                lhsAttr.place = newTemp();
                lhsAttr.code = E1.code;
                lhsAttr.code.insert(lhsAttr.code.end(), E2.code.begin(), E2.code.end());
                lhsAttr.code.push_back({"==", E1.place, E2.place, lhsAttr.place});
            }
            // 产生式: E -> id + E 或 E -> num + E
            else if (prod.rhs.size() == 3 && prod.rhs[1] == "+") 
            {
                Attribute op1 = rhsAttrs[0]; // id 或 num
                Attribute E2 = rhsAttrs[2];
                
                lhsAttr.place = newTemp();
                lhsAttr.code = E2.code;
                lhsAttr.code.push_back({"+", op1.place, E2.place, lhsAttr.place});
            }
            // 产生式: E -> id
            // 逻辑: 传递属性
            // 无需产生四元式
            else if (prod.rhs.size() == 1 && prod.rhs[0] == "id") 
            {
                lhsAttr.place = rhsAttrs[0].place;//E的变量 = id的变量
            }
            // 产生式: E -> num
            else if (prod.rhs.size() == 1 && prod.rhs[0] == "num") 
            {
                lhsAttr.place = rhsAttrs[0].place;
            }
            
            symbolStack.push(lhsAttr);//将规约完的表达式存入符号栈中
            
        } 
        else if (act.type == 'a') 
        { // 接受动作
            cout << "接受" << endl;
            cout << "分析成功！" << endl;
            //接受，弹出符号栈栈顶最后的符号
            Attribute res = symbolStack.top();
            cout << "生成的四元式：" << endl;
            
            ofstream outFile("output.txt");
            int line = 1;
            for (const auto& q : res.code) 
            {
                string qStr = q.toString();
                //按行打印到txt和屏幕上
                cout << line << ": " << qStr << endl;
                outFile << line++ << ": " << qStr << endl;
            }
            outFile.close();
            cout << "四元式已保存到 output.txt" << endl;
            break;
        }
    }
}
