#include "grammar.cpp"
#include "parser.cpp"
#include "lexer.cpp"
#include <iostream>
#include <fstream>

/**
 * @brief 主程序
 * 
 * 流程:
 * 1. 初始化文法分析器 GrammarAnalyzer
 * 2. 加载文法文件 LoadGrammer(testfile.txt)
 * 3. 构建 SLR(1) 分析表 (First/Follow -> DFA -> Table)
 * 4. 初始化语法分析器 Parser
 * 5. 读取源代码文件 (source.txt)
 * 6. 执行语法分析并输出四元式
 */
int main() 
{
    GrammarAnalyzer G;
    
    // 1. 读取testfile文件获取并加载文法G
    // 文法文件格式: S -> while ( C ) { S }
    G.loadGrammar("testfile.txt");
    
    //打印该文法
    cout << "文法加载:" << endl;
    for (const auto& p : G.grammar) 
    {
        cout << p.toString() << endl;
    }
    cout << "------------------------" << endl;
    
    // 2. 构建分析表
    // 如果存在冲突 (Shift-Reduce 或 Reduce-Reduce)，则构建失败
    if (!G.build()) 
    {
        cout << "该文法不是SLR(1)文法!" << endl;
        return 1;
    }
    cout << "SLR(1)分析表成功构建!" << endl;
    cout << "DFA状态集数量共有: " << G.states.size() << endl;
    cout << "------------------------" << endl;
    
    // 3. 分析输入
    // 初始化语法分析器，传入构建好的文法分析器 G
    // 接下来使用配置好的SLR(1)分析表来解决语法问题，传入文法G到语法分析器parse中
    Parser parser(G);
    
    // 测试用例备用：while ( a > b ) { x = y }
    // 使用嵌套语句进行测试
    //string srcLine = "while ( a > b ) { while ( c > d ) { x = y } }";
    string srcLine = "while ( a > b ) { x = y }";
   // string srcLine = "while (a > b) { while (c < 10) { x = x + 1 } }";
    cout << "原句型为: " << srcLine << endl;
    cout << "------------------------" << endl;

    // 词法分析展示
    cout << "词法分析展示:" << endl;
    Lexer lexer(srcLine);
    vector<Token> tokens = lexer.tokenize();
    //打印一下获得的词法分析的Token
    for (const auto& t : tokens) 
    {
        cout << "<" << t.type << ", " << t.value << "> "<<endl;
    }
    cout << endl << "------------------------" << endl;

    // 语法分析与语义分析
    parser.parse(srcLine);
    
    system("pause");
    return 0;
}
