//
// Created by Edot on 2023/9/26.
//

#ifndef EXP2_CPP_PARSER_H_
#define EXP2_CPP_PARSER_H_
#include <vector>
#include <map>
#include "TokenValue.h"
#include "ErrorType.h"
#include "Token.h"

class Token;
class Lexer;
class Env;
struct UglyQuaternionItem;
class InterCode;

class Parser{
  public:
    Parser(const char *path, InterCode *inter_code);
    ~Parser();
    void Parse();

    bool has_error = false;
  private:
    void NextMatch(TokenType token_type);

    void HandleError();
    void AddError(int line_num, Error error);
    void PrintErrors();

    void CompUnit();
    void Decls();
    void FuncDefs();
    void Decl();
    void ConstDecl();
    void ConstDeclTail();
    void BType();
    void ConstDef();
    void ConstDefTail(int &dimension, std::vector<int> &axes);
    void ConstInitVal(TokenValue *entry);
    void ConstInitValTail(TokenValue *entry);
    void VarDecl();
    void VarDeclTail();
    void VarDef();
    void VarDefTail(int &dimension, std::vector<int> &axes);
    void InitVal(UglyQuaternionItem var_item, int index);
    void InitValTail(UglyQuaternionItem array_item);
    void FuncDef();
    void MainFuncDef();
    void FuncType(int &dimension);
    void FuncFParams(TokenValue *func_entry);
    void FuncFParamsTail(TokenValue *func_entry);
    void FuncFParam(TokenValue *func_entry);
    void FuncFParamTail(std::vector<int> &axes);
    void Block(bool &has_return_at_last);
    void BlockTail(bool &has_return_at_last);
    void BlockItem(bool &is_return);
    void Stmt(bool &is_return);
    void ForStmt();
    void Exp(int &value, int &dimension, bool solve_value, UglyQuaternionItem &result);
    void Cond(UglyQuaternionItem &result);
    void LVal(int &value, int &dimension, bool &is_const, bool solve_value, bool if_dereference, UglyQuaternionItem &result);
    void PrimaryExp(int &value, int &dimension, bool solve_value, UglyQuaternionItem &result);
    void Number(int &value);
    void UnaryExp(int &value, int &dimension, bool solve_value, UglyQuaternionItem &result);
    void UnaryOp();
    void FuncRParams(int &param_count, std::vector<int> &params_dimensions);
    void MulExp(int &value, int &dimension, bool solve_value, UglyQuaternionItem &result);
    void MulExpTail(int &value, bool solve_value, UglyQuaternionItem &result);
    void AddExp(int &value, int &dimension, bool solve_value, UglyQuaternionItem &result);
    void AddExpTail(int &value, bool solve_value, UglyQuaternionItem &result);
    void RelExp(UglyQuaternionItem &result);
    void RelExpTail(UglyQuaternionItem &result);
    void EqExp(UglyQuaternionItem &result);
    void EqExpTail(UglyQuaternionItem &result);
    void LAndExp(UglyQuaternionItem &result);
    void LAndExpTail(UglyQuaternionItem &result);
    void LOrExp(UglyQuaternionItem &result);
    void LOrExpTail(UglyQuaternionItem &result);
    void ConstExp(int &value);




    Lexer *lexer;
    std::vector<Token> tokens;
    long long current_token_index;
    Env *env;
    bool skip_push; // if false, push env when a block begin; if true, env would have been pushed at a func id
    bool in_void; // if true, means parser is in a void func, return should be followed by semicolon
    int in_cycle = 0; // if greater than 0, means parser is in a for block
    std::map<int, char, std::less<int>> errors;
    InterCode *inter_code;
};

#endif //EXP2_CPP_PARSER_H_
