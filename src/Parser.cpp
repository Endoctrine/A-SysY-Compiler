//
// Created by Edot on 2023/9/26.
//

#include <iostream>
#include "Token.h"
#include "Lexer.h"
#include "Parser.h"
#include "Env.h"
#include "InterCode.h"

//#define PRINT_TOKEN
#define PRINT_ERROR

Parser::Parser(const char *path, InterCode *inter_code) : current_token_index(0), inter_code(inter_code){
    lexer = new Lexer(path);
    env = new Env(nullptr);
    skip_push = false;
    in_void = false;
    in_cycle = 0;
}

Parser::~Parser(){
    delete lexer;
}

void Parser::NextMatch(TokenType token_type){
    if(tokens[current_token_index].GetTokenType() == token_type){
        if(token_type == RETURNTK && in_void){
            if(tokens[current_token_index + 1].GetTokenType() != SEMICN){
                AddError(tokens[current_token_index].GetLineNumber(), RETURN_MISMATCH);
            }
        }
        if((token_type == BREAKTK || token_type == CONTINUETK) && !in_cycle){
            AddError(tokens[current_token_index].GetLineNumber(), BREAK_CONTINUE_ERROR);
        }
        if(token_type == STRCON){
            if(!tokens[current_token_index].IsFormatLegal()){
                AddError(tokens[current_token_index].GetLineNumber(), ILLEGAL_FORMAT);
            }
        }
#ifdef PRINT_TOKEN
        std::cout << tokens[current_token_index].ToString() << std::endl;
#endif
        current_token_index++;
    } else{
        if(token_type == SEMICN){
            AddError(tokens[current_token_index - 1].GetLineNumber(), SEMICOLON_MISSING);
        }
        if(token_type == RPARENT){
            AddError(tokens[current_token_index - 1].GetLineNumber(), PARENT_MISSING);
        }
        if(token_type == RBRACK){
            AddError(tokens[current_token_index - 1].GetLineNumber(), BRACKET_MISSING);
        }

        HandleError();
    }
}

void Parser::Parse(){
    // step.1 prepare for parsing
    Token current_token = lexer->NextToken();
    while(current_token.GetTokenType() != ENDTK){
        // don't be worried, push_back() will call copy constructor
        // you have nothing to be deep cloned
        tokens.push_back(current_token);
        current_token = lexer->NextToken();
    }
    // step.2 parse
    CompUnit();
    // step.3 report errors
    PrintErrors();
}

void Parser::CompUnit(){
    Decls();
    FuncDefs();
    MainFuncDef();
#ifdef PRINT_TOKEN
    std::cout << "<CompUnit>" << std::endl;
#endif
}

void Parser::Decls(){
    while(true){
        if(tokens[current_token_index].GetTokenType() == INTTK){
            if(tokens[current_token_index + 1].GetTokenType() == MAINTK){
                return;
            }
            if(tokens[current_token_index + 1].GetTokenType() == ENDTK){
                HandleError();
                return;
            }
            if(tokens[current_token_index + 2].GetTokenType() == LPARENT){
                return;
            }
            Decl();
        } else if(tokens[current_token_index].GetTokenType() == CONSTTK){
            Decl();
        } else{
            return;
        }
    }
}

void Parser::FuncDefs(){
    while(true){
        if(tokens[current_token_index].GetTokenType() == INTTK){
            if(tokens[current_token_index + 1].GetTokenType() == MAINTK){
                return;
            }
            FuncDef();
        } else if(tokens[current_token_index].GetTokenType() == VOIDTK){
            FuncDef();
        } else{
            return;
        }
    }
}

void Parser::Decl(){
    if(tokens[current_token_index].GetTokenType() == CONSTTK){
        ConstDecl();
#ifdef PRINT_TOKEN
        //        std::cout << "<Decl>" << std::endl;
#endif
    } else{
        VarDecl();
#ifdef PRINT_TOKEN
        //        std::cout << "<Decl>" << std::endl;
#endif
    }
}

void Parser::ConstDecl(){
    NextMatch(CONSTTK);
    BType();
    ConstDef();
    ConstDeclTail();
    NextMatch(SEMICN);
#ifdef PRINT_TOKEN
    std::cout << "<ConstDecl>" << std::endl;
#endif
}

void Parser::ConstDeclTail(){
    while(true){
        if(tokens[current_token_index].GetTokenType() != COMMA){
            return;
        }
        NextMatch(COMMA);
        ConstDef();
    }
}

void Parser::BType(){
    NextMatch(INTTK);
#ifdef PRINT_TOKEN
    //    std::cout << "<BType>" << std::endl;
#endif
}

void Parser::ConstDef(){
    Token current_token = tokens[current_token_index];
    NextMatch(IDENFR);
    int dimension = 0;
    std::vector<int> axes;
    ConstDefTail(dimension, axes);
    TokenValue *entry;
    entry = new TokenValue(current_token.GetLexeme(), CONST_INT, dimension, false);
    inter_code->AddCode(InterCodeNS::CONDEF, InterCode::MakeName(entry), {}, {});
    for(int i = 0; i < dimension; ++i){
        entry->AddAxis(axes[i]);
    }
    if(!env->TryPut(current_token.GetLexeme(), entry)){
        AddError(current_token.GetLineNumber(), REDEFINED);
    }
    NextMatch(ASSIGN);
    ConstInitVal(entry);

    while(entry->ValuesSize() < entry->Size()){
        entry->AddValue(0);
    }
#ifdef PRINT_TOKEN
    std::cout << "<ConstDef>" << std::endl;
#endif
}

// ConstDefTail -> { '[' ConstExp ']' }
void Parser::ConstDefTail(int &dimension, std::vector<int> &axes){
    while(true){
        if(tokens[current_token_index].GetTokenType() != LBRACK){
            return;
        }
        dimension++;
        NextMatch(LBRACK);
        int temp_size;
        ConstExp(temp_size);
        axes.push_back(temp_size);
        NextMatch(RBRACK);
    }
}

// ConstInitVal → ConstExp @<ConstInitVal>@ | '{' [ ConstInitVal ConstInitValTail ] '}' @<ConstInitVal>@
void Parser::ConstInitVal(TokenValue *entry){
    if(tokens[current_token_index].GetTokenType() == LBRACE){
        if(tokens[current_token_index + 1].GetTokenType() == RBRACE){
            return;
        }
        NextMatch(LBRACE);
        ConstInitVal(entry);
        ConstInitValTail(entry);
        NextMatch(RBRACE);
    } else{
        int temp_value;
        ConstExp(temp_value);
        entry->AddValue(temp_value);
    }
#ifdef PRINT_TOKEN
    std::cout << "<ConstInitVal>" << std::endl;
#endif
}

// ConstInitValTail -> { ',' ConstInitVal }
void Parser::ConstInitValTail(TokenValue *entry){
    while(true){
        if(tokens[current_token_index].GetTokenType() != COMMA){
            return;
        }
        NextMatch(COMMA);
        ConstInitVal(entry);
    }
}

// VarDecl → BType VarDef VarDeclTail ';' @<VarDecl>@
void Parser::VarDecl(){
    BType();
    VarDef();
    VarDeclTail();
    NextMatch(SEMICN);
#ifdef PRINT_TOKEN
    std::cout << "<VarDecl>" << std::endl;
#endif
}

// VarDeclTail -> { ',' VarDef }
void Parser::VarDeclTail(){
    while(true){
        if(tokens[current_token_index].GetTokenType() != COMMA){
            return;
        }
        NextMatch(COMMA);
        VarDef();
    }
}

// VarDef → Ident VarDefTail @<VarDef>@ | Ident VarDefTail '=' InitVal @<VarDef>@
void Parser::VarDef(){
    Token current_token = tokens[current_token_index];
    NextMatch(IDENFR);
    int dimension = 0;
    std::vector<int> axes;
    VarDefTail(dimension, axes);
    TokenValue *entry;
    entry = new TokenValue(current_token.GetLexeme(), VAR_INT, dimension, false);

    for(int i = 0; i < dimension; ++i){
        entry->AddAxis(axes[i]);
    }
    if(!env->TryPut(current_token.GetLexeme(), entry)){
        AddError(current_token.GetLineNumber(), REDEFINED);
    }
    if(tokens[current_token_index].GetTokenType() == ASSIGN){
        NextMatch(ASSIGN);
        UglyQuaternionItem temp_var_item = InterCode::MakeName(entry);
        InitVal(temp_var_item, -1);
    }
#ifdef PRINT_TOKEN
    std::cout << "<VarDef>" << std::endl;
#endif
}

// VarDefTail -> { '[' ConstExp ']' }
void Parser::VarDefTail(int &dimension, std::vector<int> &axes){
    while(true){
        if(tokens[current_token_index].GetTokenType() != LBRACK){
            return;
        }
        dimension++;
        NextMatch(LBRACK);
        int temp_size;
        ConstExp(temp_size);
        axes.push_back(temp_size);
        NextMatch(RBRACK);
    }
}

// InitVal → Exp @<InitVal>@ | '{' [ InitVal InitValTail ] '}' @<InitVal>@
void Parser::InitVal(UglyQuaternionItem var_item, int index){
    UglyQuaternionItem temp_index;
    temp_index.type = InterCodeNS::IMM;
    temp_index.value.numeric_value = index;
    if(tokens[current_token_index].GetTokenType() == LBRACE){
        NextMatch(LBRACE);
        if(tokens[current_token_index].GetTokenType() != RBRACE){
            if(index >= 0){
                auto temp_array_entry =
                    new TokenValue(inter_code->GetTempName(),
                                   TEMP_INT,
                                   var_item.value.name_value->GetIdDimension() - 1,
                                   true);
                for(int i = 0; i < var_item.value.name_value->GetIdDimension() - 1; ++i){
                    temp_array_entry->AddAxis(var_item.value.name_value->shape[i + 1]);
                }
                env->TryPut(temp_array_entry->GetName(), temp_array_entry);
                UglyQuaternionItem temp_array_item = InterCode::MakeName(temp_array_entry);
                inter_code->AddCode(InterCodeNS::REF, var_item, temp_index, temp_array_item);
                InitVal(temp_array_item, 0);
                InitValTail(temp_array_item);
            } else{
                InitVal(var_item, 0);
                InitValTail(var_item);
            }
        }
        NextMatch(RBRACE);
    } else{
        int useless_value;
        int useless_dimension;
        UglyQuaternionItem temp_result;
        Exp(useless_value, useless_dimension, false, temp_result);
        if(!var_item.value.name_value->is_pointer && var_item.value.name_value->GetIdDimension() == 0){
            inter_code->AddCode(InterCodeNS::ASS, temp_result, {}, var_item);
        } else{
            auto temp_array_entry = new TokenValue(inter_code->GetTempName(), TEMP_INT, 0, true);
            env->TryPut(temp_array_entry->GetName(), temp_array_entry);
            UglyQuaternionItem temp_array_item = InterCode::MakeName(temp_array_entry);
            inter_code->AddCode(InterCodeNS::REF, var_item, temp_index, temp_array_item);
            inter_code->AddCode(InterCodeNS::STORE, temp_result, {}, temp_array_item);
        }
    }
#ifdef PRINT_TOKEN
    std::cout << "<InitVal>" << std::endl;
#endif
}

// InitValTail -> { ',' InitVal }
void Parser::InitValTail(UglyQuaternionItem array_item){
    int current_index = 1;
    while(true){
        if(tokens[current_token_index].GetTokenType() != COMMA){
            return;
        }
        NextMatch(COMMA);
        InitVal(array_item, current_index);
        current_index++;
    }
}

// FuncDef → FuncType Ident '(' [FuncFParams] ')' Block @<FuncDef>@
void Parser::FuncDef(){
    int dimension;
    FuncType(dimension);
    if(dimension == -1){
        in_void = true;
    } else{
        in_void = false;
    }
    Token current_token = tokens[current_token_index];
    NextMatch(IDENFR);
    auto entry = new TokenValue(current_token.GetLexeme(), FUNC, dimension, false);
    if(!env->TryPut(current_token.GetLexeme(), entry)){
        AddError(current_token.GetLineNumber(), REDEFINED);
    }
    env = new Env(env);
    skip_push = true;
    NextMatch(LPARENT);
    if(tokens[current_token_index].GetTokenType() != RPARENT){
        FuncFParams(entry);
    }
    UglyQuaternionItem func_name = InterCode::MakeName(entry);
    inter_code->AddCode(InterCodeNS::FUNC, func_name, {}, {});
    NextMatch(RPARENT);
    bool has_return_at_last = false;
    Block(has_return_at_last);
    if(in_void && !has_return_at_last){
        inter_code->AddCode(InterCodeNS::RET, {}, {}, {});
    }
    if(!in_void && !has_return_at_last){
        AddError(tokens[current_token_index - 1].GetLineNumber(), RETURN_MISSING);
    }
#ifdef PRINT_TOKEN
    std::cout << "<FuncDef>" << std::endl;
#endif
}

// MainFuncDef → 'int' 'main' '(' ')' Block @<MainFuncDef>@
void Parser::MainFuncDef(){
    in_void = false;
    inter_code->AddMainFunc();
    NextMatch(INTTK);
    NextMatch(MAINTK);
    NextMatch(LPARENT);
    NextMatch(RPARENT);
    bool has_return_at_last = false;
    Block(has_return_at_last);
    if(!has_return_at_last){
        AddError(tokens[current_token_index - 1].GetLineNumber(), RETURN_MISSING);
    }
#ifdef PRINT_TOKEN
    std::cout << "<MainFuncDef>" << std::endl;
#endif
}

// FuncType → 'void' @<FuncType>@ | 'int' @<FuncType>@
void Parser::FuncType(int &dimension){
    if(tokens[current_token_index].GetTokenType() == VOIDTK){
        NextMatch(VOIDTK);
        dimension = -1;
    } else{
        NextMatch(INTTK);
        dimension = 0;
    }
#ifdef PRINT_TOKEN
    std::cout << "<FuncType>" << std::endl;
#endif
}

// FuncFParams → FuncFParam FuncFParamsTail @<FuncFParams>@
void Parser::FuncFParams(TokenValue *func_entry){
    FuncFParam(func_entry);
    FuncFParamsTail(func_entry);
#ifdef PRINT_TOKEN
    std::cout << "<FuncFParams>" << std::endl;
#endif
}

// FuncFParamsTail -> { ',' FuncFParam }
void Parser::FuncFParamsTail(TokenValue *func_entry){
    while(true){
        if(tokens[current_token_index].GetTokenType() != COMMA){
            return;
        }
        NextMatch(COMMA);
        FuncFParam(func_entry);
    }
}

// FuncFParam → BType Ident ['[' ']' FuncFParamTail] @<FuncFParam>@
void Parser::FuncFParam(TokenValue *func_entry){
    BType();
    Token current_token = tokens[current_token_index];
    NextMatch(IDENFR);
    std::vector<int> axes;
    axes.push_back(0);
    bool is_pointer = false;
    if(tokens[current_token_index].GetTokenType() == LBRACK){
        NextMatch(LBRACK);
        NextMatch(RBRACK);
        FuncFParamTail(axes);
        is_pointer = true;
    }
    TokenValue *entry;
    if(!is_pointer){
        entry = new TokenValue(current_token.GetLexeme(), VAR_INT, 0, false);
    } else{
        entry = new TokenValue(current_token.GetLexeme(), VAR_INT, static_cast<int>(axes.size()), true);
    }
    for(int axis : axes){
        entry->AddAxis(axis);
    }
    entry->is_param = true;
    if(!env->TryPut(current_token.GetLexeme(), entry)){
        AddError(current_token.GetLineNumber(), REDEFINED);
    }
    func_entry->AddParameter(entry);
#ifdef PRINT_TOKEN
    std::cout << "<FuncFParam>" << std::endl;
#endif
}

// FuncFParamTail -> { '[' ConstExp ']' }
void Parser::FuncFParamTail(std::vector<int> &axes){
    while(true){
        if(tokens[current_token_index].GetTokenType() != LBRACK){
            return;
        }
        NextMatch(LBRACK);
        int temp_size;
        ConstExp(temp_size);
        axes.push_back(temp_size);
        NextMatch(RBRACK);
    }
}

// Block → '{' BlockTail '}' @<Block>@
void Parser::Block(bool &has_return_at_last){
    if(!skip_push){
        env = new Env(env);
    } else{
        skip_push = false;
    }
    NextMatch(LBRACE);
    BlockTail(has_return_at_last);
    NextMatch(RBRACE);
    env = env->GetPrev();
#ifdef PRINT_TOKEN
    std::cout << "<Block>" << std::endl;
#endif
}

// BlockTail -> { BlockItem }
void Parser::BlockTail(bool &has_return_at_last){
    while(true){
        if(tokens[current_token_index].GetTokenType() == RBRACE){
            break;
        }
        BlockItem(has_return_at_last);
    }
}

// BlockItem → Decl @<BlockItem>@ | Stmt @<BlockItem>@
void Parser::BlockItem(bool &is_return){
    if(tokens[current_token_index].GetTokenType() == CONSTTK || tokens[current_token_index].GetTokenType() == INTTK){
        Decl();
        is_return = false;
    } else{
        Stmt(is_return);
    }
#ifdef PRINT_TOKEN
    //    std::cout << "<BlockItem>" << std::endl;
#endif
}

/*
Stmt →
LVal '=' Exp ';'  @<Stmt>@
| LVal '=' 'getint''('')'';' @<Stmt>@
| [Exp] ';' @<Stmt>@
 
| Block @<Stmt>@
| 'if' '(' Cond ')' Stmt [ 'else' Stmt ] @<Stmt>@
| 'for' '(' [ForStmt] ';' [Cond] ';' [forStmt] ')' Stmt @<Stmt>@
| 'break' ';' @<Stmt>@
| 'continue' ';' @<Stmt>@
| 'return' [Exp] ';' @<Stmt>@
| 'printf''('FormatString{','Exp}')'';' @<Stmt>@ 尚未完成
 */
void Parser::Stmt(bool &is_return){
    int useless_value, useless_dimension;
    bool useless_is_return;
    is_return = false;
    switch(tokens[current_token_index].GetTokenType()){
        case LBRACE:{ // | Block @<Stmt>@
            Block(useless_is_return);
            break;
        }
        case IFTK:{ // | 'if' '(' Cond ')' Stmt [ 'else' Stmt ] @<Stmt>@
            NextMatch(IFTK);
            NextMatch(LPARENT);
            UglyQuaternionItem condition_result;
            Cond(condition_result);
            int brf_id = inter_code->AddCode(InterCodeNS::BRF, condition_result, {}, {});
            NextMatch(RPARENT);
            Stmt(useless_is_return);
            int br_id = inter_code->AddCode(InterCodeNS::BR, {}, {}, {});
            inter_code->GetQuaternion(brf_id)->result = InterCode::MakeLabel(br_id + 1);
            if(tokens[current_token_index].GetTokenType() == ELSETK){
                NextMatch(ELSETK);
                Stmt(useless_is_return);
            }
            inter_code->GetQuaternion(br_id)->result = InterCode::MakeLabel(inter_code->GetCurrentId());
            break;
        }
        case FORTK:{ // | 'for' '(' [ForStmt] ';' [Cond] ';' [forStmt] ')' Stmt @<Stmt>@
            in_cycle++;
            NextMatch(FORTK);
            NextMatch(LPARENT);
            if(tokens[current_token_index].GetTokenType() != SEMICN){
                ForStmt();
            }
            NextMatch(SEMICN);
            int condition_start = inter_code->GetCurrentId();
            UglyQuaternionItem condition_result;
            int brt_id = 0;
            int brf_id = 0;
            int br_id = 0;
            if(tokens[current_token_index].GetTokenType() != SEMICN){
                Cond(condition_result);
                brt_id = inter_code->AddCode(InterCodeNS::BRT, condition_result, {}, {});
                brf_id = inter_code->AddCode(InterCodeNS::BRF, condition_result, {}, {});
            } else{
                br_id = inter_code->AddCode(InterCodeNS::BR, {}, {}, {});
            }
            inter_code->ForItBegin();
            int it_start = inter_code->GetCurrentId();
            NextMatch(SEMICN);
            if(tokens[current_token_index].GetTokenType() != RPARENT){
                ForStmt();
            }
            inter_code->AddCode(InterCodeNS::BR, {}, {}, InterCode::MakeLabel(condition_start));
            int stmt_start = inter_code->GetCurrentId();
            NextMatch(RPARENT);
            Stmt(useless_is_return);
            inter_code->AddCode(InterCodeNS::BR, {}, {}, InterCode::MakeLabel(it_start));
            int stmt_end = inter_code->GetCurrentId();
            if(br_id){
                inter_code->GetQuaternion(br_id)->result = InterCode::MakeLabel(stmt_start);
            } else{
                inter_code->GetQuaternion(brt_id)->result = InterCode::MakeLabel(stmt_start);
                inter_code->GetQuaternion(brf_id)->result = InterCode::MakeLabel(stmt_end);
            }
            in_cycle--;
            inter_code->ForEnd(it_start);
            break;
        }
        case BREAKTK:{ // | 'break' ';' @<Stmt>@
            NextMatch(BREAKTK);
            NextMatch(SEMICN);
            inter_code->AddForBreak();
            break;
        }
        case CONTINUETK:{ // | 'continue' ';' @<Stmt>@
            NextMatch(CONTINUETK);
            NextMatch(SEMICN);
            inter_code->AddForContinue();
            break;
        }
        case RETURNTK:{ // | 'return' [Exp] ';' @<Stmt>@
            is_return = true;
            NextMatch(RETURNTK);
            if(tokens[current_token_index].GetTokenType() != SEMICN){
                UglyQuaternionItem temp_result;
                Exp(useless_value, useless_dimension, false, temp_result);
                inter_code->AddCode(InterCodeNS::RETVAL, temp_result, {}, {});
            } else{
                inter_code->AddCode(InterCodeNS::RET, {}, {}, {});
            }
            NextMatch(SEMICN);
            break;
        }
        case PRINTFTK:{ // | 'printf''(' FormatString {','Exp} ')' ';' @<Stmt>@
            int formatter_count, exp_count = 0;
            int printf_line_number = tokens[current_token_index].GetLineNumber();
            NextMatch(PRINTFTK);
            NextMatch(LPARENT);
            NextMatch(STRCON);
            auto strcon_entry = new TokenValue(
                inter_code->GetTempName(), TEMP_STR, 0, false
            );
            strcon_entry->strcon_value = tokens[current_token_index - 1].GetLexeme();
            env->TryPut(strcon_entry->GetName(), strcon_entry);
            UglyQuaternionItem strcon_item = InterCode::MakeName(strcon_entry);
            inter_code->AddCode(InterCodeNS::PRINTF, strcon_item, {}, {});
            formatter_count = tokens[current_token_index - 1].FormatterCount();
            while(tokens[current_token_index].GetTokenType() == COMMA){
                NextMatch(COMMA);
                UglyQuaternionItem temp_result;
                Exp(useless_value, useless_dimension, false, temp_result);
                inter_code->AddCode(InterCodeNS::PRINT, temp_result, {}, {});
                exp_count++;
            }
            if(formatter_count != exp_count){
                AddError(printf_line_number, FORMAT_MISMATCH);
            }
            NextMatch(RPARENT);
            NextMatch(SEMICN);
            break;
        }
        default:{
            long long current = current_token_index + 1;
            bool is_assignment = false;
            while(tokens[current].GetTokenType() == LBRACK){
                int bracket_count = 1;
                current++;
                while(bracket_count && tokens[current].GetTokenType() != ENDTK
                    && tokens[current].GetTokenType() != SEMICN){
                    if(tokens[current].GetTokenType() == LBRACK){
                        bracket_count++;
                    }
                    if(tokens[current].GetTokenType() == RBRACK){
                        bracket_count--;
                    }
                    // 以下是在代码优化阶段添加的一部分代码
                    // 这个跟代码优化没啥关系，加它是因为错误混合样例过不去
                    // 考虑这样一种错误：a[1 = b;
                    // 这个错误会使得识别赋值语句的过程出现问题，因为会一读到结尾
                    // 加上下面的段代码，这个错误就能处理了
                    // 但是假如出现 a[1 * b  c = d; 这种错误
                    // 就不好办了。只能希望这种错误是“影响语法分析”的错误，不会考察
                    // 如果真的要考察，那得求赋值语句的follow集，有没有冲突还不好说
                    // 如果真出问题，先试试把下面这段代码删了
                    if(tokens[current].GetTokenType() == ASSIGN){
                        is_assignment = true;
                    }
                    // 以上。
                    current++;
                }
            }
            if(tokens[current].GetTokenType() == ASSIGN){
                is_assignment = true;
            }
            if(is_assignment){
                bool is_const;
                int temp_line_number = tokens[current_token_index].GetLineNumber();
                UglyQuaternionItem l_val_result;
                LVal(useless_value, useless_dimension, is_const, false, false, l_val_result);
                if(is_const){
                    AddError(temp_line_number, ASSIGN_TO_CONST);
                }
                NextMatch(ASSIGN);
                if(tokens[current_token_index].GetTokenType() == GETINTTK){
                    NextMatch(GETINTTK);
                    NextMatch(LPARENT);
                    NextMatch(RPARENT);
                    NextMatch(SEMICN);
                    auto temp_entry = new TokenValue(inter_code->GetTempName(), TEMP_INT, 0, false);
                    env->TryPut(temp_entry->GetName(), temp_entry);
                    UglyQuaternionItem temp = InterCode::MakeName(temp_entry);
                    inter_code->AddCode(InterCodeNS::GETINT, {}, {}, temp);
                    if(!l_val_result.value.name_value->is_pointer){
                        inter_code->AddCode(InterCodeNS::ASS, temp, {}, l_val_result);
                    } else{
                        inter_code->AddCode(InterCodeNS::STORE, temp, {}, l_val_result);
                    }
                } else{
                    UglyQuaternionItem exp_result;
                    Exp(useless_value, useless_dimension, false, exp_result);
                    NextMatch(SEMICN);
                    if(!l_val_result.value.name_value->is_pointer){
                        inter_code->AddCode(InterCodeNS::ASS, exp_result, {}, l_val_result);
                    } else{
                        inter_code->AddCode(InterCodeNS::STORE, exp_result, {}, l_val_result);
                    }
                }
            } else{
                if(tokens[current_token_index].GetTokenType() != SEMICN){
                    UglyQuaternionItem useless_result;
                    Exp(useless_value, useless_dimension, false, useless_result);
                }
                NextMatch(SEMICN);
            }
            break;
        }
    }
#ifdef PRINT_TOKEN
    std::cout << "<Stmt>" << std::endl;
#endif
}

// ForStmt → LVal '=' Exp @<ForStmt>@
void Parser::ForStmt(){
    int useless_value, useless_dimension;
    bool useless_is_const;
    UglyQuaternionItem l_val_result;
    LVal(useless_value, useless_dimension, useless_is_const, false, false, l_val_result);
    NextMatch(ASSIGN);
    UglyQuaternionItem temp_result;
    Exp(useless_value, useless_dimension, false, temp_result);
    if(!l_val_result.value.name_value->is_pointer){
        inter_code->AddCode(InterCodeNS::ASS, temp_result, {}, l_val_result);
    } else{
        inter_code->AddCode(InterCodeNS::STORE, temp_result, {}, l_val_result);
    }
#ifdef PRINT_TOKEN
    std::cout << "<ForStmt>" << std::endl;
#endif
}

// Exp → AddExp @<Exp>@
void Parser::Exp(int &value, int &dimension, bool solve_value, UglyQuaternionItem &result){
    AddExp(value, dimension, solve_value, result);
#ifdef PRINT_TOKEN
    std::cout << "<Exp>" << std::endl;
#endif
}

// Cond → LOrExp @<Cond>@
void Parser::Cond(UglyQuaternionItem &result){
    LOrExp(result);
#ifdef PRINT_TOKEN
    std::cout << "<Cond>" << std::endl;
#endif
}

// LVal → Ident {'[' Exp ']'} @<LVal>@
void Parser::LVal(int &value,
                  int &dimension,
                  bool &is_const,
                  bool solve_value,
                  bool if_dereference,
                  UglyQuaternionItem &result){
    Token current_token = tokens[current_token_index];
    TokenValue *entry = env->Get(current_token.GetLexeme());
    if(!entry){ // 一个简单的错误恢复，不知道对不对
        AddError(current_token.GetLineNumber(), UNDEFINED);
        entry = new TokenValue(current_token.GetLexeme(), VAR_INT, 0, false);
        env->TryPut(entry->GetName(), entry);
    }
    is_const = entry->NotVar();
    NextMatch(IDENFR);
    std::vector<int> indexes;
    dimension = entry->GetIdDimension();
    result = InterCode::MakeName(entry);
    UglyQuaternionItem to_be_ref = result;
    while(tokens[current_token_index].GetTokenType() == LBRACK){
        NextMatch(LBRACK);
        int temp_index, useless_dimension;
        UglyQuaternionItem exp_result;
        Exp(temp_index, useless_dimension, false, exp_result);
        if(!solve_value){
            auto result_entry =
                new TokenValue(inter_code->GetTempName(),
                               TEMP_INT,
                               to_be_ref.value.name_value->GetIdDimension() - 1,
                               true);
            if(result_entry->GetIdDimension() > 0){
                result_entry->AddAxis(0);
            }
            for(int i = 1; i < result_entry->GetIdDimension(); i++){
                result_entry->AddAxis(to_be_ref.value.name_value->shape[i + 1]);
            }
            env->TryPut(result_entry->GetName(), result_entry);
            result = InterCode::MakeName(result_entry);
            inter_code->AddCode(InterCodeNS::REF, to_be_ref, exp_result, result);
        }
        to_be_ref = result;
        indexes.push_back(temp_index);
        NextMatch(RBRACK);
        dimension--;
    }
    if(is_const && dimension){ // const array should not be passed to func
        dimension = -1; // this will trigger "func param type error"
    }
    if(solve_value){
//        switch(entry->GetIdDimension()){
//            case 0:{
//                value = entry->GetValue();
//                break;
//            }
//            case 1:{
//                value = entry->GetValue(indexes[0]);
//                break;
//            }
//            case 2:{
//                value = entry->GetValue(indexes[0], indexes[1]);
//                break;
//            }
//        }
        value = entry->GetValue(indexes);
    }
    if(if_dereference &&
        result.value.name_value->is_pointer &&
        result.value.name_value->GetIdDimension() == 0){
        /*
         * 注意：result.value.name_value->is_pointer &&
         *      result.value.name_value->GetIdDimension() == 0 保证了
         * 这个玩意是需要解引用的。
         * 如果 result.value.name_value->GetIdDimension() > 0，但满足 if_dereference ，说明现在正在进行函数传参，
         * 那么就不需要对 result 做额外的处理。
         * 真的是太糟糕了。。。
         * 更详细地，一般的变量求值也满足 if_dereference && result.value.name_value->GetIdDimension() == 0，
         * 所以还得拉上 result.value.name_value->is_pointer 一起判断。
         *
         * 以下写完代码生成之后更新的注释：
         * 对于常量表达式求值，需要抑制中间代码的生成。
         * 判断是否是常量表达式求值，需要用到solve_value变量，
         * 然而，对于这个分支，if_dereference==true时solve_value一定等于false，
         * 所以不用抑制中间代码的生成，不需要更多的判断了。
         */
        UglyQuaternionItem temp_result;
        auto temp_result_entry = new TokenValue(inter_code->GetTempName(), TEMP_INT, 0, false);
        env->TryPut(temp_result_entry->GetName(), temp_result_entry);
        temp_result = InterCode::MakeName(temp_result_entry);
        inter_code->AddCode(InterCodeNS::LOAD, result, {}, temp_result);
        result = temp_result;
    }
#ifdef PRINT_TOKEN
    std::cout << "<LVal>" << std::endl;
#endif
}

/*
 * PrimaryExp → '(' Exp ')' @<PrimaryExp>@
 * | LVal @<PrimaryExp>@
 * | Number @<PrimaryExp>@
*/
void Parser::PrimaryExp(int &value, int &dimension, bool solve_value, UglyQuaternionItem &result){
    if(tokens[current_token_index].GetTokenType() == LPARENT){
        dimension = 0;
        NextMatch(LPARENT);
        int useless_dimension;
        Exp(value, useless_dimension, solve_value, result);
        NextMatch(RPARENT);
    } else if(tokens[current_token_index].GetTokenType() == INTCON){
        dimension = 0;
        Number(value);
        result.type = InterCodeNS::IMM;
        result.value.numeric_value = value;
    } else{
        bool useless_is_const;
        LVal(value, dimension, useless_is_const, solve_value, true, result);
    }
#ifdef PRINT_TOKEN
    std::cout << "<PrimaryExp>" << std::endl;
#endif
}

// Number → IntConst @<Number>@
void Parser::Number(int &value){
    value = stoi(tokens[current_token_index].GetLexeme());
    NextMatch(INTCON);
#ifdef PRINT_TOKEN
    std::cout << "<Number>" << std::endl;
#endif
}

/*
 * UnaryExp → PrimaryExp @<UnaryExp>@
 * | Ident '(' [FuncRParams] ')' @<UnaryExp>@
 * | UnaryOp UnaryExp @<UnaryExp>@
 */
void Parser::UnaryExp(int &value, int &dimension, bool solve_value, UglyQuaternionItem &result){
    if(tokens[current_token_index].GetTokenType() == IDENFR
        && tokens[current_token_index + 1].GetTokenType() == LPARENT){
        Token current_token = tokens[current_token_index];
        TokenValue *entry = env->Get(current_token.GetLexeme());
        if(!entry){ // 又是一个简单的错误恢复，不知道对不对
            AddError(current_token.GetLineNumber(), UNDEFINED);
            entry = new TokenValue(current_token.GetLexeme(), FUNC, -1, false);
        }
        dimension = entry->GetIdDimension();
        NextMatch(IDENFR);
        NextMatch(LPARENT);
        int param_count = 0;
        std::vector<int> params_dimensions;
        if(tokens[current_token_index].GetTokenType() != RPARENT){
            FuncRParams(param_count, params_dimensions);
        }
        UglyQuaternionItem name = InterCode::MakeName(entry);
        UglyQuaternionItem imm;
        imm.type = InterCodeNS::IMM;
        imm.value.numeric_value = param_count;
        auto result_entry = new TokenValue(inter_code->GetTempName(), TEMP_INT, 0, false);
        env->TryPut(result_entry->GetName(), result_entry);
        result = InterCode::MakeName(result_entry);
        inter_code->AddCode(InterCodeNS::CALL, name, imm, result);
        if(param_count != entry->ParametersSize()){
            AddError(current_token.GetLineNumber(), PARAM_COUNT_MISMATCH);
        }
        if(!entry->IsParamTypeMatched(params_dimensions)){
            AddError(current_token.GetLineNumber(), PARAM_TYPE_MISMATCH);
        }
        NextMatch(RPARENT);
    } else if(tokens[current_token_index].GetTokenType() == PLUS
        || tokens[current_token_index].GetTokenType() == MINU
        || tokens[current_token_index].GetTokenType() == NOT){
        TokenType op_type = tokens[current_token_index].GetTokenType();
        UnaryOp();
        UnaryExp(value, dimension, solve_value, result);
        if(!solve_value){
            auto temp_result_entry = new TokenValue(inter_code->GetTempName(), TEMP_INT, 0, false);
            env->TryPut(temp_result_entry->GetName(), temp_result_entry);
            UglyQuaternionItem temp_result = InterCode::MakeName(temp_result_entry);
            if(op_type == PLUS){
                inter_code->AddCode(InterCodeNS::POS, result, {}, temp_result);
            } else if(op_type == MINU){
                inter_code->AddCode(InterCodeNS::NEG, result, {}, temp_result);
            } else{
                inter_code->AddCode(InterCodeNS::NOT, result, {}, temp_result);
            }
            result = temp_result;
        }
        if(op_type == MINU){
            value = -value;
        }
    } else{
        PrimaryExp(value, dimension, solve_value, result);
    }
#ifdef PRINT_TOKEN
    std::cout << "<UnaryExp>" << std::endl;
#endif
}

// UnaryOp → '+' @<UnaryOp>@ | '-' @<UnaryOp>@ | '!' @<UnaryOp>@
void Parser::UnaryOp(){
    if(tokens[current_token_index].GetTokenType() == PLUS){
        NextMatch(PLUS);
    } else if(tokens[current_token_index].GetTokenType() == MINU){
        NextMatch(MINU);
    } else{
        NextMatch(NOT);
    }
#ifdef PRINT_TOKEN
    std::cout << "<UnaryOp>" << std::endl;
#endif
}

// FuncRParams → Exp { ',' Exp } @<FuncRParams>@
void Parser::FuncRParams(int &param_count, std::vector<int> &params_dimensions){
    int useless_value;
    int dimension;
    UglyQuaternionItem param;
    Exp(useless_value, dimension, false, param);
    params_dimensions.push_back(dimension);
    param_count = 1;
    inter_code->AddCode(InterCodeNS::PARAM, param, {}, {});
    while(tokens[current_token_index].GetTokenType() == COMMA){
        NextMatch(COMMA);
        param_count++;
        Exp(useless_value, dimension, false, param);
        inter_code->AddCode(InterCodeNS::PARAM, param, {}, {});
        params_dimensions.push_back(dimension);
    }
#ifdef PRINT_TOKEN
    std::cout << "<FuncRParams>" << std::endl;
#endif
}

// MulExp -> UnaryExp @<MulExp>@ MulExpTail
void Parser::MulExp(int &value, int &dimension, bool solve_value, UglyQuaternionItem &result){
    UnaryExp(value, dimension, solve_value, result);
#ifdef PRINT_TOKEN
    std::cout << "<MulExp>" << std::endl;
#endif
    MulExpTail(value, solve_value, result);
}

// MulExpTail -> {('*' | '/' | '%') UnaryExp @<MulExp>@}
void Parser::MulExpTail(int &value, bool solve_value, UglyQuaternionItem &result){
    UglyQuaternionItem temp_left, temp_right;
    temp_left = result;
    while(tokens[current_token_index].GetTokenType() == MULT
        || tokens[current_token_index].GetTokenType() == DIV
        || tokens[current_token_index].GetTokenType() == MOD){
        TokenType op_type = tokens[current_token_index].GetTokenType();
        if(op_type == MULT){
            NextMatch(MULT);
        } else if(op_type == DIV){
            NextMatch(DIV);
        } else{
            NextMatch(MOD);
        }
        int temp_value, useless_dimension;
        UnaryExp(temp_value, useless_dimension, solve_value, temp_right);
        if(!solve_value){
            auto result_entry = new TokenValue(inter_code->GetTempName(), TEMP_INT, 0, false);
            env->TryPut(result_entry->GetName(), result_entry);
            result = InterCode::MakeName(result_entry);
        }
        if(op_type == MULT){
            if(!solve_value){
                inter_code->AddCode(InterCodeNS::MUL, temp_left, temp_right, result);
            }
            value *= temp_value;
        } else if(op_type == DIV){
            if(!solve_value){
                inter_code->AddCode(InterCodeNS::DIV, temp_left, temp_right, result);
            }
            if(temp_value){
                value /= temp_value;
            }
        } else{
            if(!solve_value){
                inter_code->AddCode(InterCodeNS::MOD, temp_left, temp_right, result);
            }
            if(temp_value){
                value %= temp_value;
            }
        }
        if(!solve_value){
            temp_left = result;
        }
#ifdef PRINT_TOKEN
        std::cout << "<MulExp>" << std::endl;
#endif
    }
}

// AddExp -> MulExp @<AddExp>@ AddExpTail
void Parser::AddExp(int &value, int &dimension, bool solve_value, UglyQuaternionItem &result){
    MulExp(value, dimension, solve_value, result);
#ifdef PRINT_TOKEN
    std::cout << "<AddExp>" << std::endl;
#endif
    AddExpTail(value, solve_value, result);
}

// AddExpTail -> {('+' | '-') MulExp @<AddExp>@}
void Parser::AddExpTail(int &value, bool solve_value, UglyQuaternionItem &result){
    UglyQuaternionItem temp_left, temp_right;
    temp_left = result;
    while(tokens[current_token_index].GetTokenType() == PLUS
        || tokens[current_token_index].GetTokenType() == MINU){
        TokenType op_type = tokens[current_token_index].GetTokenType();
        if(op_type == PLUS){
            NextMatch(PLUS);
        } else{
            NextMatch(MINU);
        }
        int temp_value, useless_dimension;
        MulExp(temp_value, useless_dimension, solve_value, temp_right);
        if(!solve_value){
            auto result_entry = new TokenValue(inter_code->GetTempName(), TEMP_INT, 0, false);
            env->TryPut(result_entry->GetName(), result_entry);
            result = InterCode::MakeName(result_entry);
        }
        if(op_type == PLUS){
            if(!solve_value){
                inter_code->AddCode(InterCodeNS::ADD, temp_left, temp_right, result);
            }
            value += temp_value;
        } else{
            if(!solve_value){
                inter_code->AddCode(InterCodeNS::SUB, temp_left, temp_right, result);
            }
            value -= temp_value;
        }
        if(!solve_value){
            temp_left = result;
        }
#ifdef PRINT_TOKEN
        std::cout << "<AddExp>" << std::endl;
#endif
    }
}

// RelExp -> AddExp @<RelExp>@ RelExpTail
void Parser::RelExp(UglyQuaternionItem &result){
    int useless_value, useless_dimension;
    AddExp(useless_value, useless_dimension, false, result);
#ifdef PRINT_TOKEN
    std::cout << "<RelExp>" << std::endl;
#endif
    RelExpTail(result);
}

// RelExpTail -> {('<' | '>' | '<=' | '>=') AddExp @<RelExp>@}
void Parser::RelExpTail(UglyQuaternionItem &result){
    int useless_value, useless_dimension;
    UglyQuaternionItem temp_left, temp_right;
    temp_left = result;
    while(tokens[current_token_index].GetTokenType() == LSS
        || tokens[current_token_index].GetTokenType() == GRE
        || tokens[current_token_index].GetTokenType() == LEQ
        || tokens[current_token_index].GetTokenType() == GEQ){
        TokenType op_type = tokens[current_token_index].GetTokenType();
        if(op_type == LSS){
            NextMatch(LSS);
        } else if(op_type == GRE){
            NextMatch(GRE);
        } else if(op_type == LEQ){
            NextMatch(LEQ);
        } else{
            NextMatch(GEQ);
        }
        AddExp(useless_value, useless_dimension, false, temp_right);
        auto result_entry = new TokenValue(inter_code->GetTempName(), TEMP_INT, 0, false);
        env->TryPut(result_entry->GetName(), result_entry);
        result = InterCode::MakeName(result_entry);
        if(op_type == LSS){
            inter_code->AddCode(InterCodeNS::LSS, temp_left, temp_right, result);
        } else if(op_type == GRE){
            inter_code->AddCode(InterCodeNS::GRE, temp_left, temp_right, result);
        } else if(op_type == LEQ){
            inter_code->AddCode(InterCodeNS::LEQ, temp_left, temp_right, result);
        } else{
            inter_code->AddCode(InterCodeNS::GEQ, temp_left, temp_right, result);
        }
        temp_left = result;
#ifdef PRINT_TOKEN
        std::cout << "<RelExp>" << std::endl;
#endif
    }
}

// EqExp -> RelExp @<EqExp>@ EqExpTail
void Parser::EqExp(UglyQuaternionItem &result){
    RelExp(result);
#ifdef PRINT_TOKEN
    std::cout << "<EqExp>" << std::endl;
#endif
    EqExpTail(result);
}

// EqExpTail -> {('==' | '!=') RelExp @<EqExp>@}
void Parser::EqExpTail(UglyQuaternionItem &result){
    UglyQuaternionItem temp_left, temp_right;
    temp_left = result;
    while(tokens[current_token_index].GetTokenType() == EQL
        || tokens[current_token_index].GetTokenType() == NEQ){
        TokenType op_type = tokens[current_token_index].GetTokenType();
        if(op_type == EQL){
            NextMatch(EQL);
        } else{
            NextMatch(NEQ);
        }
        RelExp(temp_right);
        auto result_entry = new TokenValue(inter_code->GetTempName(), TEMP_INT, 0, false);
        env->TryPut(result_entry->GetName(), result_entry);
        result = InterCode::MakeName(result_entry);
        if(op_type == EQL){
            inter_code->AddCode(InterCodeNS::EQ, temp_left, temp_right, result);
        } else{
            inter_code->AddCode(InterCodeNS::NEQ, temp_left, temp_right, result);
        }
        temp_left = result;
#ifdef PRINT_TOKEN
        std::cout << "<EqExp>" << std::endl;
#endif
    }
}

// LAndExp -> EqExp @<LAndExp>@ LAndExpTail
void Parser::LAndExp(UglyQuaternionItem &result){
    EqExp(result);
#ifdef PRINT_TOKEN
    std::cout << "<LAndExp>" << std::endl;
#endif
    LAndExpTail(result);
}

// LAndExpTail -> {'&&' EqExp @<LAndExp>@}
void Parser::LAndExpTail(UglyQuaternionItem &result){
    auto temp_result_entry = new TokenValue(inter_code->GetTempName(), TEMP_INT, 0, false);
    env->TryPut(temp_result_entry->GetName(), temp_result_entry);
    UglyQuaternionItem temp_result = InterCode::MakeName(temp_result_entry);
    std::vector<int> short_circuit_branches;
    if(result.type != InterCodeNS::NAME || !result.value.name_value->IsTemp()){
        inter_code->AddCode(InterCodeNS::ADD, result, InterCode::MakeImm(0), temp_result);
        result = temp_result;
    }
    while(tokens[current_token_index].GetTokenType() == AND){
        /***********短路求值***************/
        int brf_id = inter_code->AddCode(InterCodeNS::BRF, result, {}, {});
        short_circuit_branches.push_back(brf_id);
        /********************************/
        NextMatch(AND);
        EqExp(temp_result);
        inter_code->AddCode(InterCodeNS::AND, result, temp_result, result);
#ifdef PRINT_TOKEN
        std::cout << "<LAndExp>" << std::endl;
#endif
    }
    for(int id : short_circuit_branches){
        inter_code->GetQuaternion(id)->result = InterCode::MakeLabel(inter_code->GetCurrentId());
    }
}

// LOrExp -> LAndExp @<LOrExp>@ LOrExpTail
void Parser::LOrExp(UglyQuaternionItem &result){
    LAndExp(result);
#ifdef PRINT_TOKEN
    std::cout << "<LOrExp>" << std::endl;
#endif
    LOrExpTail(result);
}

// LOrExpTail -> {'||' LAndExp @<LOrExp>@}
void Parser::LOrExpTail(UglyQuaternionItem &result){
    auto temp_result_entry = new TokenValue(inter_code->GetTempName(), TEMP_INT, 0, false);
    env->TryPut(temp_result_entry->GetName(), temp_result_entry);
    UglyQuaternionItem temp_result = InterCode::MakeName(temp_result_entry);
    std::vector<int> short_circuit_branches;
    if(!result.value.name_value->IsTemp()){
        inter_code->AddCode(InterCodeNS::ADD, result, InterCode::MakeImm(0), temp_result);
        result = temp_result;
    }
    while(tokens[current_token_index].GetTokenType() == OR){
        /***********短路求值***************/
        int brt_id = inter_code->AddCode(InterCodeNS::BRT, result, {}, {});
        short_circuit_branches.push_back(brt_id);
        /********************************/
        NextMatch(OR);
        LAndExp(temp_result);
        inter_code->AddCode(InterCodeNS::OR, result, temp_result, result);
#ifdef PRINT_TOKEN
        std::cout << "<LOrExp>" << std::endl;
#endif
    }
    for(int id : short_circuit_branches){
        inter_code->GetQuaternion(id)->result = InterCode::MakeLabel(inter_code->GetCurrentId());
    }
}

// ConstExp → AddExp @<ConstExp>@
void Parser::ConstExp(int &value){
    int useless_dimension;
    UglyQuaternionItem useless_result;
    // 求解常量表达式的时候，需要抑制中间代码生成，从而抑制中间变量生成，防止破坏函数参数的连续性
    env->disabled = true;
    AddExp(value, useless_dimension, true, useless_result);
    env->disabled = false;
#ifdef PRINT_TOKEN
    std::cout << "<ConstExp>" << std::endl;
#endif
}

void Parser::HandleError(){

}

void Parser::AddError(int line_num, Error error){
    has_error = true;
    switch(error){
        case REDEFINED:errors[line_num] = 'b';
            break;
        case UNDEFINED:errors[line_num] = 'c';
            break;
        case PARAM_COUNT_MISMATCH:errors[line_num] = 'd';
            break;
        case PARAM_TYPE_MISMATCH:errors[line_num] = 'e';
            break;
        case RETURN_MISMATCH:errors[line_num] = 'f';
            break;
        case RETURN_MISSING:errors[line_num] = 'g';
            break;
        case ASSIGN_TO_CONST:errors[line_num] = 'h';
            break;
        case FORMAT_MISMATCH:errors[line_num] = 'l';
            break;
        case SEMICOLON_MISSING:errors[line_num] = 'i';
            break;
        case PARENT_MISSING:errors[line_num] = 'j';
            break;
        case BRACKET_MISSING:errors[line_num] = 'k';
            break;
        case BREAK_CONTINUE_ERROR:errors[line_num] = 'm';
            break;
        case ILLEGAL_FORMAT:errors[line_num] = 'a';
            break;
    }
}

void Parser::PrintErrors(){
#ifdef PRINT_ERROR
    auto iter = errors.begin();
    while(iter != errors.end()){
        std::cout << iter->first << " " << iter->second << std::endl;
        iter++;
    }
#endif
}


