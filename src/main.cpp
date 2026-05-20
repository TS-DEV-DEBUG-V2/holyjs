/*
COPYRIGHT 2026 TS-DEV-DEBUG-V2
HolyJS Compiler
License: MIT 
https://github.com/TS-DEV-DEBUG-V2/holyjs
*/
For more info visit 
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <filesystem>

enum class TokenType {
    FN, LET, MUT, RET, IF, ELSE, FOR, IN, WHILE, MATCH,
    IMPORT, FROM, AS, EXPORT, LOG, STRUCT, NEW, NIL, USE,
    TRUE_LIT, FALSE_LIT,
    IDENT, NUMBER, STRING, TEMPLATE_STRING,
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    COMMA, DOT, COLON, SEMICOLON, QUESTION,
    PLUS, MINUS, STAR, SLASH, PERCENT,
    EQ, EQEQ, BANGEQ, BANG,
    LT, GT, LTEQ, GTEQ,
    ARROW, FAT_ARROW, PIPE, AMP, AMPAMP, PIPEPIPE,
    PLUSEQ, MINUSEQ, STAREQ, SLASHEQ,
    DOTDOT, SPREAD,
    NEWLINE, EOF_TOKEN
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int col;
};

static const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"fn", TokenType::FN}, {"let", TokenType::LET}, {"mut", TokenType::MUT},
    {"ret", TokenType::RET}, {"if", TokenType::IF}, {"else", TokenType::ELSE},
    {"for", TokenType::FOR}, {"in", TokenType::IN}, {"while", TokenType::WHILE},
    {"match", TokenType::MATCH}, {"import", TokenType::IMPORT}, {"from", TokenType::FROM},
    {"as", TokenType::AS}, {"export", TokenType::EXPORT}, {"log", TokenType::LOG},
    {"struct", TokenType::STRUCT}, {"new", TokenType::NEW}, {"nil", TokenType::NIL},
    {"use", TokenType::USE}, {"true", TokenType::TRUE_LIT}, {"false", TokenType::FALSE_LIT},
};

class Lexer {
    std::string src;
    size_t pos;
    int line;
    int col;

    char peek() { return pos < src.size() ? src[pos] : '\0'; }
    char peek2() { return pos + 1 < src.size() ? src[pos + 1] : '\0'; }
    char advance() {
        char c = src[pos++];
        if (c == '\n') { line++; col = 1; } else { col++; }
        return c;
    }

public:
    Lexer(const std::string& source) : src(source), pos(0), line(1), col(1) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (pos < src.size()) {
            skipWhitespace();
            if (pos >= src.size()) break;

            int tLine = line, tCol = col;

            if (peek() == '/' && peek2() == '/') {
                while (pos < src.size() && peek() != '\n') advance();
                continue;
            }
            if (peek() == '/' && peek2() == '*') {
                advance(); advance();
                while (pos < src.size() && !(peek() == '*' && peek2() == '/')) advance();
                if (pos < src.size()) { advance(); advance(); }
                continue;
            }

            if (peek() == '\n') {
                advance();
                tokens.push_back({TokenType::NEWLINE, "\\n", tLine, tCol});
                continue;
            }

            if (peek() == '.' && peek2() == '.' && pos + 2 < src.size() && src[pos+2] == '.') {
                advance(); advance(); advance();
                tokens.push_back({TokenType::SPREAD, "...", tLine, tCol});
                continue;
            }
            if (peek() == '.' && peek2() == '.') {
                advance(); advance();
                tokens.push_back({TokenType::DOTDOT, "..", tLine, tCol});
                continue;
            }

            if (peek() == '=' && peek2() == '>') { advance(); advance(); tokens.push_back({TokenType::FAT_ARROW, "=>", tLine, tCol}); continue; }
            if (peek() == '=' && peek2() == '=') { advance(); advance(); tokens.push_back({TokenType::EQEQ, "==", tLine, tCol}); continue; }
            if (peek() == '!' && peek2() == '=') { advance(); advance(); tokens.push_back({TokenType::BANGEQ, "!=", tLine, tCol}); continue; }
            if (peek() == '<' && peek2() == '=') { advance(); advance(); tokens.push_back({TokenType::LTEQ, "<=", tLine, tCol}); continue; }
            if (peek() == '>' && peek2() == '=') { advance(); advance(); tokens.push_back({TokenType::GTEQ, ">=", tLine, tCol}); continue; }
            if (peek() == '-' && peek2() == '>') { advance(); advance(); tokens.push_back({TokenType::ARROW, "->", tLine, tCol}); continue; }
            if (peek() == '&' && peek2() == '&') { advance(); advance(); tokens.push_back({TokenType::AMPAMP, "&&", tLine, tCol}); continue; }
            if (peek() == '|' && peek2() == '>') { advance(); advance(); tokens.push_back({TokenType::PIPE, "|>", tLine, tCol}); continue; }
            if (peek() == '|' && peek2() == '|') { advance(); advance(); tokens.push_back({TokenType::PIPEPIPE, "||", tLine, tCol}); continue; }
            if (peek() == '+' && peek2() == '=') { advance(); advance(); tokens.push_back({TokenType::PLUSEQ, "+=", tLine, tCol}); continue; }
            if (peek() == '-' && peek2() == '=') { advance(); advance(); tokens.push_back({TokenType::MINUSEQ, "-=", tLine, tCol}); continue; }
            if (peek() == '*' && peek2() == '=') { advance(); advance(); tokens.push_back({TokenType::STAREQ, "*=", tLine, tCol}); continue; }
            if (peek() == '/' && peek2() == '=') { advance(); advance(); tokens.push_back({TokenType::SLASHEQ, "/=", tLine, tCol}); continue; }

            static const std::unordered_map<char, TokenType> singles = {
                {'(', TokenType::LPAREN}, {')', TokenType::RPAREN},
                {'{', TokenType::LBRACE}, {'}', TokenType::RBRACE},
                {'[', TokenType::LBRACKET}, {']', TokenType::RBRACKET},
                {',', TokenType::COMMA}, {'.', TokenType::DOT},
                {':', TokenType::COLON}, {';', TokenType::SEMICOLON},
                {'+', TokenType::PLUS}, {'-', TokenType::MINUS},
                {'*', TokenType::STAR}, {'/', TokenType::SLASH},
                {'%', TokenType::PERCENT}, {'=', TokenType::EQ},
                {'<', TokenType::LT}, {'>', TokenType::GT},
                {'!', TokenType::BANG}, {'?', TokenType::QUESTION},
                {'|', TokenType::PIPE}, {'&', TokenType::AMP},
            };

            auto it = singles.find(peek());
            if (it != singles.end()) {
                std::string v(1, advance());
                tokens.push_back({it->second, v, tLine, tCol});
                continue;
            }

            if (peek() == '`') {
                advance();
                std::string str;
                while (pos < src.size() && peek() != '`') {
                    if (peek() == '\\') { str += advance(); if (pos < src.size()) str += advance(); }
                    else str += advance();
                }
                if (pos < src.size()) advance();
                tokens.push_back({TokenType::TEMPLATE_STRING, str, tLine, tCol});
                continue;
            }

            if (peek() == '"' || peek() == '\'') {
                char q = advance();
                std::string str;
                while (pos < src.size() && peek() != q) {
                    if (peek() == '\\') { str += advance(); if (pos < src.size()) str += advance(); }
                    else str += advance();
                }
                if (pos < src.size()) advance();
                tokens.push_back({TokenType::STRING, str, tLine, tCol});
                continue;
            }

            if (isdigit(peek())) {
                std::string num;
                if (peek() == '0' && (peek2() == 'x' || peek2() == 'X')) {
                    num += advance(); num += advance();
                    while (pos < src.size() && isxdigit(peek())) num += advance();
                } else {
                    while (pos < src.size() && (isdigit(peek()) || peek() == '.')) num += advance();
                }
                tokens.push_back({TokenType::NUMBER, num, tLine, tCol});
                continue;
            }

            if (isalpha(peek()) || peek() == '_' || peek() == '$') {
                std::string ident;
                while (pos < src.size() && (isalnum(peek()) || peek() == '_' || peek() == '$')) ident += advance();
                auto kw = KEYWORDS.find(ident);
                if (kw != KEYWORDS.end()) {
                    tokens.push_back({kw->second, ident, tLine, tCol});
                } else {
                    tokens.push_back({TokenType::IDENT, ident, tLine, tCol});
                }
                continue;
            }

            advance();
        }
        tokens.push_back({TokenType::EOF_TOKEN, "", line, col});
        return tokens;
    }

private:
    void skipWhitespace() {
        while (pos < src.size() && (peek() == ' ' || peek() == '\t' || peek() == '\r')) advance();
    }
};

enum class NodeType {
    Program, FnDecl, LambdaExpr, LetDecl, Return, If, ForIn, ForRange, While, Match,
    MatchArm, Import, UseImport, Export, StructDecl, NewExpr, ExprStatement,
    BinOp, UnaryOp, Call, Member, Index, Assign, CompoundAssign,
    NumLit, StrLit, TemplateLit, BoolLit, NilLit, Ident,
    ArrayLit, ObjectLit, ObjectProp, SpreadExpr, Block,
    TernaryExpr, PipeExpr
};

struct ASTNode {
    NodeType type;
    std::string strVal;
    std::string strVal2;
    double numVal = 0;
    bool boolVal = false;
    std::vector<std::shared_ptr<ASTNode>> children;
    std::vector<std::string> params;
    std::vector<std::string> paramDefaults;
    bool isMutable = false;
    bool isExported = false;
    bool isRest = false;
    int line = 0;
};

using Node = std::shared_ptr<ASTNode>;

Node makeNode(NodeType type, int line = 0) {
    auto n = std::make_shared<ASTNode>();
    n->type = type;
    n->line = line;
    return n;
}

class Parser {
    std::vector<Token> tokens;
    size_t pos;

    Token& peek() { return tokens[pos]; }
    Token& peek2() { return tokens[std::min(pos + 1, tokens.size() - 1)]; }

    Token eat(TokenType t) {
        if (tokens[pos].type != t) {
            throw std::runtime_error("Line " + std::to_string(tokens[pos].line) +
                ":" + std::to_string(tokens[pos].col) +
                " - Expected " + std::to_string((int)t) + ", got '" + tokens[pos].value + "'");
        }
        return tokens[pos++];
    }

    void skipNL() { while (peek().type == TokenType::NEWLINE) pos++; }

    bool check(TokenType t) { return peek().type == t; }

    bool match(TokenType t) {
        if (check(t)) { pos++; return true; }
        return false;
    }

public:
    Parser(const std::vector<Token>& toks) : tokens(toks), pos(0) {}

    Node parse() {
        auto prog = makeNode(NodeType::Program);
        skipNL();
        while (!check(TokenType::EOF_TOKEN)) {
            prog->children.push_back(parseStatement());
            skipNL();
        }
        return prog;
    }

private:
    Node parseStatement() {
        if (check(TokenType::EXPORT)) return parseExport();
        if (check(TokenType::IMPORT)) return parseImport();
        if (check(TokenType::USE)) return parseUse();
        if (check(TokenType::FN)) return parseFn();
        if (check(TokenType::LET) || check(TokenType::MUT)) return parseLet();
        if (check(TokenType::RET)) return parseReturn();
        if (check(TokenType::IF)) return parseIf();
        if (check(TokenType::FOR)) return parseFor();
        if (check(TokenType::WHILE)) return parseWhile();
        if (check(TokenType::MATCH)) return parseMatch();
        if (check(TokenType::STRUCT)) return parseStruct();
        return parseExprStatement();
    }

    Node parseExport() {
        eat(TokenType::EXPORT);
        skipNL();
        auto stmt = parseStatement();
        stmt->isExported = true;
        return stmt;
    }

    Node parseImport() {
        auto n = makeNode(NodeType::Import, peek().line);
        eat(TokenType::IMPORT);
        if (check(TokenType::LBRACE)) {
            eat(TokenType::LBRACE);
            while (!check(TokenType::RBRACE)) {
                std::string name = eat(TokenType::IDENT).value;
                std::string alias = name;
                if (match(TokenType::AS)) alias = eat(TokenType::IDENT).value;
                n->params.push_back(name);
                n->paramDefaults.push_back(alias);
                if (!check(TokenType::RBRACE)) eat(TokenType::COMMA);
            }
            eat(TokenType::RBRACE);
            eat(TokenType::FROM);
            n->strVal = eat(TokenType::STRING).value;
            n->strVal2 = "named";
        } else if (check(TokenType::STAR)) {
            eat(TokenType::STAR);
            eat(TokenType::AS);
            n->strVal2 = "namespace";
            n->params.push_back(eat(TokenType::IDENT).value);
            eat(TokenType::FROM);
            n->strVal = eat(TokenType::STRING).value;
        } else {
            std::string name = eat(TokenType::IDENT).value;
            n->params.push_back(name);
            eat(TokenType::FROM);
            n->strVal = eat(TokenType::STRING).value;
            n->strVal2 = "default";
        }
        return n;
    }

    Node parseUse() {
        auto n = makeNode(NodeType::UseImport, peek().line);
        eat(TokenType::USE);
        n->strVal = eat(TokenType::STRING).value;
        if (match(TokenType::AS)) {
            n->strVal2 = eat(TokenType::IDENT).value;
        }
        return n;
    }

    Node parseFn(bool isExpr = false) {
        auto n = makeNode(NodeType::FnDecl, peek().line);
        eat(TokenType::FN);
        if (check(TokenType::IDENT) && !isExpr) {
            n->strVal = eat(TokenType::IDENT).value;
        }
        eat(TokenType::LPAREN);
        parseParams(n);
        eat(TokenType::RPAREN);
        if (check(TokenType::ARROW)) {
            eat(TokenType::ARROW);
        }
        if (check(TokenType::LBRACE)) {
            n->children = parseBlock();
        } else {
            auto ret = makeNode(NodeType::Return, peek().line);
            ret->children.push_back(parseExpr());
            n->children.push_back(ret);
        }
        return n;
    }

    void parseParams(Node& n) {
        while (!check(TokenType::RPAREN)) {
            bool isRest = false;
            if (check(TokenType::SPREAD)) {
                eat(TokenType::SPREAD);
                isRest = true;
            }
            std::string name = eat(TokenType::IDENT).value;
            if (isRest) name = "..." + name;
            n->params.push_back(name);
            std::string def = "";
            if (match(TokenType::EQ)) {
                auto defExpr = parseExpr();
                std::ostringstream oss;
                oss << "__DEFAULT__";
                def = oss.str();
                n->paramDefaults.push_back(def);
                if (!check(TokenType::RPAREN)) eat(TokenType::COMMA);
                continue;
            }
            n->paramDefaults.push_back(def);
            if (!check(TokenType::RPAREN)) eat(TokenType::COMMA);
        }
    }

    Node parseLet() {
        auto n = makeNode(NodeType::LetDecl, peek().line);
        bool isMut = check(TokenType::MUT);
        if (isMut) eat(TokenType::MUT); else eat(TokenType::LET);
        n->isMutable = isMut;
        n->strVal = eat(TokenType::IDENT).value;
        if (match(TokenType::EQ)) {
            n->children.push_back(parseExpr());
        }
        return n;
    }

    Node parseReturn() {
        auto n = makeNode(NodeType::Return, peek().line);
        eat(TokenType::RET);
        if (!check(TokenType::NEWLINE) && !check(TokenType::RBRACE) && !check(TokenType::EOF_TOKEN)) {
            n->children.push_back(parseExpr());
        }
        return n;
    }

    Node parseIf() {
        auto n = makeNode(NodeType::If, peek().line);
        eat(TokenType::IF);
        n->children.push_back(parseExpr());
        auto thenBlock = makeNode(NodeType::Block);
        thenBlock->children = parseBlock();
        n->children.push_back(thenBlock);
        skipNL();
        if (match(TokenType::ELSE)) {
            skipNL();
            if (check(TokenType::IF)) {
                auto elseBlock = makeNode(NodeType::Block);
                elseBlock->children.push_back(parseIf());
                n->children.push_back(elseBlock);
            } else {
                auto elseBlock = makeNode(NodeType::Block);
                elseBlock->children = parseBlock();
                n->children.push_back(elseBlock);
            }
        }
        return n;
    }

    Node parseFor() {
        eat(TokenType::FOR);
        std::string varName = eat(TokenType::IDENT).value;
        eat(TokenType::IN);
        auto iter = parseExpr();
        if (iter->type == NodeType::BinOp && iter->strVal == "..") {
            auto n = makeNode(NodeType::ForRange, peek().line);
            n->strVal = varName;
            n->children.push_back(iter->children[0]);
            n->children.push_back(iter->children[1]);
            auto body = parseBlock();
            for (auto& s : body) n->children.push_back(s);
            return n;
        }
        if (iter->type == NodeType::Call && iter->children[0]->type == NodeType::Ident && iter->children[0]->strVal == "range") {
            auto n = makeNode(NodeType::ForRange, peek().line);
            n->strVal = varName;
            if (iter->children.size() == 2) {
                n->children.push_back(makeNode(NodeType::NumLit));
                n->children[0]->numVal = 0;
                n->children[0]->strVal = "0";
                n->children.push_back(iter->children[1]);
            } else if (iter->children.size() >= 3) {
                n->children.push_back(iter->children[1]);
                n->children.push_back(iter->children[2]);
            }
            auto body = parseBlock();
            for (auto& s : body) n->children.push_back(s);
            return n;
        }
        auto n = makeNode(NodeType::ForIn, peek().line);
        n->strVal = varName;
        n->children.push_back(iter);
        auto body = parseBlock();
        for (auto& s : body) n->children.push_back(s);
        return n;
    }

    Node parseWhile() {
        auto n = makeNode(NodeType::While, peek().line);
        eat(TokenType::WHILE);
        n->children.push_back(parseExpr());
        auto body = parseBlock();
        for (auto& s : body) n->children.push_back(s);
        return n;
    }

    Node parseMatch() {
        auto n = makeNode(NodeType::Match, peek().line);
        eat(TokenType::MATCH);
        n->children.push_back(parseExpr());
        eat(TokenType::LBRACE);
        skipNL();
        while (!check(TokenType::RBRACE)) {
            auto arm = makeNode(NodeType::MatchArm);
            arm->children.push_back(parseMatchPattern());
            eat(TokenType::FAT_ARROW);
            skipNL();
            if (check(TokenType::LBRACE)) {
                auto block = makeNode(NodeType::Block);
                block->children = parseBlock();
                arm->children.push_back(block);
            } else {
                arm->children.push_back(parseExpr());
            }
            skipNL();
            match(TokenType::COMMA);
            skipNL();
        }
        eat(TokenType::RBRACE);
        return n;
    }

    Node parseMatchPattern() {
        if (check(TokenType::NUMBER)) {
            auto n = makeNode(NodeType::NumLit, peek().line);
            n->strVal = peek().value;
            n->numVal = std::stod(peek().value);
            pos++;
            return n;
        }
        if (check(TokenType::STRING)) {
            auto n = makeNode(NodeType::StrLit, peek().line);
            n->strVal = peek().value;
            pos++;
            return n;
        }
        if (check(TokenType::TRUE_LIT)) { pos++; auto n = makeNode(NodeType::BoolLit); n->boolVal = true; return n; }
        if (check(TokenType::FALSE_LIT)) { pos++; auto n = makeNode(NodeType::BoolLit); n->boolVal = false; return n; }
        if (check(TokenType::NIL)) { pos++; return makeNode(NodeType::NilLit); }
        if (check(TokenType::IDENT)) {
            auto n = makeNode(NodeType::Ident, peek().line);
            n->strVal = peek().value;
            pos++;
            return n;
        }
        throw std::runtime_error("Line " + std::to_string(peek().line) + " - Invalid match pattern");
    }

    Node parseMatchExpr() { return parseMatch(); }

    Node parseStruct() {
        auto n = makeNode(NodeType::StructDecl, peek().line);
        eat(TokenType::STRUCT);
        n->strVal = eat(TokenType::IDENT).value;
        eat(TokenType::LBRACE);
        skipNL();
        while (!check(TokenType::RBRACE)) {
            if (check(TokenType::FN)) {
                auto method = parseFn();
                n->children.push_back(method);
            } else {
                std::string field = eat(TokenType::IDENT).value;
                n->params.push_back(field);
                if (match(TokenType::EQ)) {
                    n->children.push_back(parseExpr());
                } else {
                    n->children.push_back(makeNode(NodeType::NilLit));
                }
            }
            skipNL();
            match(TokenType::COMMA);
            skipNL();
        }
        eat(TokenType::RBRACE);
        return n;
    }

    Node parseExprStatement() {
        auto expr = parseExpr();
        if (check(TokenType::EQ)) {
            eat(TokenType::EQ);
            auto n = makeNode(NodeType::Assign, expr->line);
            n->children.push_back(expr);
            n->children.push_back(parseExpr());
            return n;
        }
        if (check(TokenType::PLUSEQ) || check(TokenType::MINUSEQ) ||
            check(TokenType::STAREQ) || check(TokenType::SLASHEQ)) {
            auto n = makeNode(NodeType::CompoundAssign, expr->line);
            n->strVal = peek().value;
            pos++;
            n->children.push_back(expr);
            n->children.push_back(parseExpr());
            return n;
        }
        auto n = makeNode(NodeType::ExprStatement, expr->line);
        n->children.push_back(expr);
        return n;
    }

    std::vector<Node> parseBlock() {
        eat(TokenType::LBRACE);
        skipNL();
        std::vector<Node> stmts;
        while (!check(TokenType::RBRACE)) {
            stmts.push_back(parseStatement());
            skipNL();
        }
        eat(TokenType::RBRACE);
        return stmts;
    }

    int getPrec(TokenType t) {
        switch (t) {
            case TokenType::QUESTION: return 0;
            case TokenType::PIPEPIPE: return 1;
            case TokenType::AMPAMP: return 2;
            case TokenType::PIPE: return 3;
            case TokenType::EQEQ: case TokenType::BANGEQ: return 4;
            case TokenType::LT: case TokenType::GT: case TokenType::LTEQ: case TokenType::GTEQ: return 5;
            case TokenType::DOTDOT: return 6;
            case TokenType::PLUS: case TokenType::MINUS: return 7;
            case TokenType::STAR: case TokenType::SLASH: case TokenType::PERCENT: return 8;
            default: return -1;
        }
    }

    std::string opStr(TokenType t) {
        switch (t) {
            case TokenType::PLUS: return "+"; case TokenType::MINUS: return "-";
            case TokenType::STAR: return "*"; case TokenType::SLASH: return "/";
            case TokenType::PERCENT: return "%"; case TokenType::EQEQ: return "==";
            case TokenType::BANGEQ: return "!="; case TokenType::LT: return "<";
            case TokenType::GT: return ">"; case TokenType::LTEQ: return "<=";
            case TokenType::GTEQ: return ">="; case TokenType::AMPAMP: return "&&";
            case TokenType::PIPEPIPE: return "||"; case TokenType::PIPE: return "|>";
            case TokenType::DOTDOT: return "..";
            default: return "?";
        }
    }

    Node parseExpr(int minPrec = 0) {
        auto left = parseUnary();
        while (true) {
            auto t = peek().type;
            int prec = getPrec(t);
            if (prec < minPrec) break;

            if (t == TokenType::QUESTION) {
                pos++;
                auto n = makeNode(NodeType::TernaryExpr);
                n->children.push_back(left);
                n->children.push_back(parseExpr());
                eat(TokenType::COLON);
                n->children.push_back(parseExpr());
                left = n;
                continue;
            }

            if (t == TokenType::PIPE) {
                pos++;
                skipNL();
                auto n = makeNode(NodeType::PipeExpr);
                n->children.push_back(left);
                n->children.push_back(parseUnary());
                left = n;
                continue;
            }

            std::string op = opStr(t);
            pos++;
            auto right = parseExpr(prec + 1);
            auto n = makeNode(NodeType::BinOp);
            n->strVal = op;
            n->children.push_back(left);
            n->children.push_back(right);
            left = n;
        }
        return left;
    }

    Node parseUnary() {
        if (check(TokenType::BANG)) {
            pos++;
            auto n = makeNode(NodeType::UnaryOp);
            n->strVal = "!";
            n->children.push_back(parseUnary());
            return n;
        }
        if (check(TokenType::MINUS)) {
            pos++;
            auto n = makeNode(NodeType::UnaryOp);
            n->strVal = "-";
            n->children.push_back(parseUnary());
            return n;
        }
        if (check(TokenType::SPREAD)) {
            pos++;
            auto n = makeNode(NodeType::SpreadExpr);
            n->children.push_back(parseUnary());
            return n;
        }
        return parsePostfix();
    }

    Node parsePostfix() {
        auto node = parsePrimary();
        while (true) {
            if (check(TokenType::LPAREN)) {
                eat(TokenType::LPAREN);
                auto call = makeNode(NodeType::Call, node->line);
                call->children.push_back(node);
                while (!check(TokenType::RPAREN)) {
                    call->children.push_back(parseExpr());
                    if (!check(TokenType::RPAREN)) eat(TokenType::COMMA);
                }
                eat(TokenType::RPAREN);
                node = call;
            } else if (check(TokenType::DOT)) {
                eat(TokenType::DOT);
                auto mem = makeNode(NodeType::Member, node->line);
                mem->strVal = eat(TokenType::IDENT).value;
                mem->children.push_back(node);
                node = mem;
            } else if (check(TokenType::LBRACKET)) {
                eat(TokenType::LBRACKET);
                auto idx = makeNode(NodeType::Index, node->line);
                idx->children.push_back(node);
                idx->children.push_back(parseExpr());
                eat(TokenType::RBRACKET);
                node = idx;
            } else break;
        }
        return node;
    }

    Node parsePrimary() {
        if (check(TokenType::NUMBER)) {
            auto n = makeNode(NodeType::NumLit, peek().line);
            n->strVal = peek().value;
            n->numVal = std::stod(peek().value);
            pos++;
            return n;
        }
        if (check(TokenType::STRING)) {
            auto n = makeNode(NodeType::StrLit, peek().line);
            n->strVal = peek().value;
            pos++;
            return n;
        }
        if (check(TokenType::TEMPLATE_STRING)) {
            auto n = makeNode(NodeType::TemplateLit, peek().line);
            n->strVal = peek().value;
            pos++;
            return n;
        }
        if (check(TokenType::TRUE_LIT)) {
            auto n = makeNode(NodeType::BoolLit, peek().line);
            n->boolVal = true;
            pos++;
            return n;
        }
        if (check(TokenType::FALSE_LIT)) {
            auto n = makeNode(NodeType::BoolLit, peek().line);
            n->boolVal = false;
            pos++;
            return n;
        }
        if (check(TokenType::NIL)) {
            pos++;
            return makeNode(NodeType::NilLit, peek().line);
        }
        if (check(TokenType::MATCH)) {
            return parseMatchExpr();
        }
        if (check(TokenType::LOG)) {
            pos++;
            auto n = makeNode(NodeType::Ident, peek().line);
            n->strVal = "__hj_log";
            return n;
        }
        if (check(TokenType::FN)) {
            return parseFn(true);
        }
        if (check(TokenType::NEW)) {
            pos++;
            auto n = makeNode(NodeType::NewExpr, peek().line);
            n->children.push_back(parsePostfix());
            return n;
        }
        if (check(TokenType::IDENT)) {
            auto n = makeNode(NodeType::Ident, peek().line);
            n->strVal = peek().value;
            pos++;
            if (check(TokenType::FAT_ARROW)) {
                eat(TokenType::FAT_ARROW);
                auto lambda = makeNode(NodeType::LambdaExpr, n->line);
                lambda->params.push_back(n->strVal);
                if (check(TokenType::LBRACE)) {
                    auto block = makeNode(NodeType::Block);
                    block->children = parseBlock();
                    lambda->children.push_back(block);
                } else {
                    lambda->children.push_back(parseExpr());
                }
                return lambda;
            }
            return n;
        }
        if (check(TokenType::LBRACKET)) {
            eat(TokenType::LBRACKET);
            auto n = makeNode(NodeType::ArrayLit, peek().line);
            skipNL();
            while (!check(TokenType::RBRACKET)) {
                n->children.push_back(parseExpr());
                skipNL();
                if (!check(TokenType::RBRACKET)) { eat(TokenType::COMMA); skipNL(); }
            }
            eat(TokenType::RBRACKET);
            return n;
        }
        if (check(TokenType::LBRACE)) {
            eat(TokenType::LBRACE);
            auto n = makeNode(NodeType::ObjectLit, peek().line);
            skipNL();
            while (!check(TokenType::RBRACE)) {
                if (check(TokenType::SPREAD)) {
                    n->children.push_back(parseUnary());
                } else {
                    auto prop = makeNode(NodeType::ObjectProp);
                    prop->strVal = eat(TokenType::IDENT).value;
                    if (match(TokenType::COLON)) {
                        prop->children.push_back(parseExpr());
                    }
                    n->children.push_back(prop);
                }
                skipNL();
                match(TokenType::COMMA);
                skipNL();
            }
            eat(TokenType::RBRACE);
            return n;
        }
        if (check(TokenType::LPAREN)) {
            eat(TokenType::LPAREN);
            if (check(TokenType::RPAREN)) {
                eat(TokenType::RPAREN);
                eat(TokenType::FAT_ARROW);
                auto lambda = makeNode(NodeType::LambdaExpr, peek().line);
                if (check(TokenType::LBRACE)) {
                    auto block = makeNode(NodeType::Block);
                    block->children = parseBlock();
                    lambda->children.push_back(block);
                } else {
                    lambda->children.push_back(parseExpr());
                }
                return lambda;
            }

            auto first = parseExpr();
            if (check(TokenType::COMMA) || (check(TokenType::RPAREN) && peekAfterParen() == TokenType::FAT_ARROW)) {
                std::vector<std::string> params;
                if (first->type == NodeType::Ident) params.push_back(first->strVal);
                while (match(TokenType::COMMA)) {
                    params.push_back(eat(TokenType::IDENT).value);
                }
                eat(TokenType::RPAREN);
                eat(TokenType::FAT_ARROW);
                auto lambda = makeNode(NodeType::LambdaExpr, first->line);
                lambda->params = params;
                if (check(TokenType::LBRACE)) {
                    auto block = makeNode(NodeType::Block);
                    block->children = parseBlock();
                    lambda->children.push_back(block);
                } else {
                    lambda->children.push_back(parseExpr());
                }
                return lambda;
            }
            eat(TokenType::RPAREN);
            return first;
        }

        throw std::runtime_error("Line " + std::to_string(peek().line) +
            " - Unexpected token: '" + peek().value + "'");
    }

    TokenType peekAfterParen() {
        int depth = 0;
        size_t i = pos;
        for (; i < tokens.size(); i++) {
            if (tokens[i].type == TokenType::RPAREN) {
                if (depth == 0) { return tokens[std::min(i+1, tokens.size()-1)].type; }
                depth--;
            }
            if (tokens[i].type == TokenType::LPAREN) depth++;
        }
        return TokenType::EOF_TOKEN;
    }
};

class CodeGenerator {
    int indent;
    bool emitGC;

    std::string ind() { return std::string(indent * 2, ' '); }

    std::string genBlock(const std::vector<Node>& stmts) {
        indent++;
        std::string out;
        for (auto& s : stmts) out += ind() + gen(s) + "\n";
        indent--;
        return out;
    }

public:
    CodeGenerator(bool gc = true) : indent(0), emitGC(gc) {}

    std::string generate(Node root) {
        std::string out;
        if (emitGC) {
            out += gcRuntime();
            out += "\n";
        }
        out += gen(root);
        if (emitGC) {
            out += "\n__hj_gc.collectAll();\n";
        }
        return out;
    }

    std::string gcRuntime() {
        return R"(const __hj_gc = (() => {
  const registry = new FinalizationRegistry((ref) => {
    __refs.delete(ref);
  });
  const __refs = new Set();
  let allocCount = 0;
  const GC_THRESHOLD = 1000;
  function track(obj) {
    if (obj !== null && typeof obj === 'object') {
      const ref = new WeakRef(obj);
      __refs.add(ref);
      registry.register(obj, ref);
      allocCount++;
      if (allocCount >= GC_THRESHOLD) {
        sweep();
        allocCount = 0;
      }
    }
    return obj;
  }
  function sweep() {
    for (const ref of __refs) {
      if (ref.deref() === undefined) __refs.delete(ref);
    }
  }
  function stats() {
    let alive = 0;
    for (const ref of __refs) { if (ref.deref() !== undefined) alive++; }
    return { tracked: __refs.size, alive };
  }
  function collectAll() { sweep(); }
  return { track, sweep, stats, collectAll };
})();
function __hj_log(...args) { console.log(...args); }
function __hj_alloc(obj) { return __hj_gc.track(obj); }
)";
    }

private:
    std::string gen(Node n) {
        switch (n->type) {
            case NodeType::Program: {
                std::string out;
                for (auto& c : n->children) out += gen(c) + "\n";
                return out;
            }
            case NodeType::Import: {
                std::string out;
                std::string src = n->strVal;
                if (src.size() > 3 && src.substr(src.size()-3) == ".hj") {
                    src = src.substr(0, src.size()-3) + ".js";
                }
                if (n->strVal2 == "default") {
                    out = "import " + n->params[0] + " from \"" + src + "\";";
                } else if (n->strVal2 == "namespace") {
                    out = "import * as " + n->params[0] + " from \"" + src + "\";";
                } else {
                    out = "import { ";
                    for (size_t i = 0; i < n->params.size(); i++) {
                        if (i > 0) out += ", ";
                        if (n->params[i] != n->paramDefaults[i])
                            out += n->params[i] + " as " + n->paramDefaults[i];
                        else
                            out += n->params[i];
                    }
                    out += " } from \"" + src + "\";";
                }
                return out;
            }
            case NodeType::UseImport: {
                std::string src = n->strVal;
                if (n->strVal2.empty())
                    return "require(\"" + src + "\");";
                return "const " + n->strVal2 + " = require(\"" + src + "\");";
            }
            case NodeType::FnDecl: {
                std::string out;
                if (n->isExported) out += "export ";
                if (n->strVal.empty()) {
                    out += "function(";
                } else {
                    out += "function " + n->strVal + "(";
                }
                out += genParams(n) + ") {\n";
                out += genBlock(n->children);
                out += ind() + "}";
                return out;
            }
            case NodeType::LambdaExpr: {
                std::string out = "(";
                for (size_t i = 0; i < n->params.size(); i++) {
                    if (i > 0) out += ", ";
                    out += n->params[i];
                }
                out += ") => ";
                if (!n->children.empty() && n->children[0]->type == NodeType::Block) {
                    out += "{\n";
                    out += genBlock(n->children[0]->children);
                    out += ind() + "}";
                } else if (!n->children.empty()) {
                    out += gen(n->children[0]);
                }
                return out;
            }
            case NodeType::LetDecl: {
                std::string out;
                if (n->isExported) out += "export ";
                out += (n->isMutable ? "let " : "const ") + n->strVal;
                if (!n->children.empty()) {
                    std::string val = gen(n->children[0]);
                    if (emitGC && isAllocExpr(n->children[0])) {
                        val = "__hj_alloc(" + val + ")";
                    }
                    out += " = " + val;
                }
                out += ";";
                return out;
            }
            case NodeType::Return: {
                if (n->children.empty()) return "return;";
                return "return " + gen(n->children[0]) + ";";
            }
            case NodeType::If: {
                std::string out = "if (" + gen(n->children[0]) + ") {\n";
                out += genBlock(n->children[1]->children);
                out += ind() + "}";
                if (n->children.size() > 2) {
                    out += " else {\n";
                    out += genBlock(n->children[2]->children);
                    out += ind() + "}";
                }
                return out;
            }
            case NodeType::ForRange: {
                std::string v = n->strVal;
                std::string from = gen(n->children[0]);
                std::string to = gen(n->children[1]);
                std::string out = "for (let " + v + " = " + from + "; " + v + " < " + to + "; " + v + "++) {\n";
                std::vector<Node> body(n->children.begin() + 2, n->children.end());
                out += genBlock(body);
                out += ind() + "}";
                return out;
            }
            case NodeType::ForIn: {
                std::string out = "for (const " + n->strVal + " of " + gen(n->children[0]) + ") {\n";
                std::vector<Node> body(n->children.begin() + 1, n->children.end());
                out += genBlock(body);
                out += ind() + "}";
                return out;
            }
            case NodeType::While: {
                std::string out = "while (" + gen(n->children[0]) + ") {\n";
                std::vector<Node> body(n->children.begin() + 1, n->children.end());
                out += genBlock(body);
                out += ind() + "}";
                return out;
            }
            case NodeType::Match: {
                std::string target = gen(n->children[0]);
                std::string out = "(() => { const __m = " + target + ";\n";
                indent++;
                for (size_t i = 1; i < n->children.size(); i++) {
                    auto arm = n->children[i];
                    std::string cond = gen(arm->children[0]);
                    if (cond == "_") {
                        out += ind() + "return " + gen(arm->children[1]) + ";\n";
                    } else {
                        std::string prefix = (i == 1) ? "if" : "else if";
                        out += ind() + prefix + " (__m === " + cond + ") ";
                        if (arm->children[1]->type == NodeType::Block) {
                            out += "{\n";
                            indent++;
                            for (auto& s : arm->children[1]->children) out += ind() + gen(s) + "\n";
                            indent--;
                            out += ind() + "}\n";
                        } else {
                            out += "return " + gen(arm->children[1]) + ";\n";
                        }
                    }
                }
                indent--;
                out += ind() + "})()";
                return out;
            }
            case NodeType::StructDecl: {
                std::string out;
                if (n->isExported) out += "export ";
                out += "class " + n->strVal + " {\n";
                indent++;
                out += ind() + "constructor(";
                for (size_t i = 0; i < n->params.size(); i++) {
                    if (i > 0) out += ", ";
                    out += n->params[i];
                }
                out += ") {\n";
                indent++;
                for (auto& f : n->params) {
                    out += ind() + "this." + f + " = " + f + ";\n";
                }
                if (emitGC) out += ind() + "__hj_gc.track(this);\n";
                indent--;
                out += ind() + "}\n";
                for (auto& c : n->children) {
                    if (c->type == NodeType::FnDecl) {
                        out += ind() + c->strVal + "(" + genParams(c) + ") {\n";
                        out += genBlock(c->children);
                        out += ind() + "}\n";
                    }
                }
                indent--;
                out += "}";
                return out;
            }
            case NodeType::NewExpr: {
                std::string inner = gen(n->children[0]);
                return "new " + inner;
            }
            case NodeType::ExprStatement:
                return gen(n->children[0]) + ";";
            case NodeType::Assign:
                return gen(n->children[0]) + " = " + gen(n->children[1]) + ";";
            case NodeType::CompoundAssign:
                return gen(n->children[0]) + " " + n->strVal + " " + gen(n->children[1]) + ";";
            case NodeType::BinOp: {
                std::string op = n->strVal;
                if (op == "==") op = "===";
                if (op == "!=") op = "!==";
                return "(" + gen(n->children[0]) + " " + op + " " + gen(n->children[1]) + ")";
            }
            case NodeType::UnaryOp:
                return n->strVal + gen(n->children[0]);
            case NodeType::Call: {
                std::string callee = gen(n->children[0]);
                std::string out = callee + "(";
                for (size_t i = 1; i < n->children.size(); i++) {
                    if (i > 1) out += ", ";
                    out += gen(n->children[i]);
                }
                out += ")";
                return out;
            }
            case NodeType::Member:
                return gen(n->children[0]) + "." + n->strVal;
            case NodeType::Index:
                return gen(n->children[0]) + "[" + gen(n->children[1]) + "]";
            case NodeType::PipeExpr: {
                return gen(n->children[1]) + "(" + gen(n->children[0]) + ")";
            }
            case NodeType::TernaryExpr:
                return "(" + gen(n->children[0]) + " ? " + gen(n->children[1]) + " : " + gen(n->children[2]) + ")";
            case NodeType::NumLit: return n->strVal;
            case NodeType::StrLit: return "\"" + n->strVal + "\"";
            case NodeType::TemplateLit: return "`" + n->strVal + "`";
            case NodeType::BoolLit: return n->boolVal ? "true" : "false";
            case NodeType::NilLit: return "null";
            case NodeType::Ident: return n->strVal;
            case NodeType::ArrayLit: {
                std::string out = "[";
                for (size_t i = 0; i < n->children.size(); i++) {
                    if (i > 0) out += ", ";
                    out += gen(n->children[i]);
                }
                out += "]";
                return out;
            }
            case NodeType::ObjectLit: {
                std::string out = "{ ";
                for (size_t i = 0; i < n->children.size(); i++) {
                    if (i > 0) out += ", ";
                    out += gen(n->children[i]);
                }
                out += " }";
                return out;
            }
            case NodeType::ObjectProp: {
                if (n->children.empty()) return n->strVal;
                return n->strVal + ": " + gen(n->children[0]);
            }
            case NodeType::SpreadExpr:
                return "..." + gen(n->children[0]);
            case NodeType::Block: {
                std::string out = "{\n";
                out += genBlock(n->children);
                out += ind() + "}";
                return out;
            }
            default: return "/* unknown node */";
        }
    }

    std::string genParams(Node& n) {
        std::string out;
        for (size_t i = 0; i < n->params.size(); i++) {
            if (i > 0) out += ", ";
            out += n->params[i];
        }
        return out;
    }

    bool isAllocExpr(Node n) {
        return n->type == NodeType::ArrayLit ||
               n->type == NodeType::ObjectLit ||
               n->type == NodeType::NewExpr ||
               n->type == NodeType::Call;
    }
};

std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) throw std::runtime_error("Cannot open file: " + path);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

void writeFile(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    if (!f.is_open()) throw std::runtime_error("Cannot write file: " + path);
    f << content;
}

void printUsage() {
    std::cout << "HolyJS Compiler v1.0\n\n"
              << "Usage:\n"
              << "  holyjs <input.hj>                  Compile to stdout\n"
              << "  holyjs <input.hj> -o <output.js>   Compile to file\n"
              << "  holyjs <input.hj> --no-gc           Disable GC runtime\n"
              << "  holyjs <input.hj> --ast             Print AST (debug)\n"
              << "  holyjs --run <input.hj>             Compile and run with Node.js\n"
              << "  holyjs --help                       Show this help\n";
}

void printAST(Node n, int depth = 0) {
    std::string pad(depth * 2, ' ');
    std::cout << pad << (int)n->type;
    if (!n->strVal.empty()) std::cout << " [" << n->strVal << "]";
    if (!n->params.empty()) {
        std::cout << " params(";
        for (size_t i = 0; i < n->params.size(); i++) {
            if (i) std::cout << ",";
            std::cout << n->params[i];
        }
        std::cout << ")";
    }
    std::cout << "\n";
    for (auto& c : n->children) printAST(c, depth + 1);
}

int main(int argc, char* argv[]) {
    if (argc < 2) { printUsage(); return 1; }

    std::string inputFile;
    std::string outputFile;
    bool noGC = false;
    bool showAST = false;
    bool runAfter = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") { printUsage(); return 0; }
        else if (arg == "--no-gc") noGC = true;
        else if (arg == "--ast") showAST = true;
        else if (arg == "--run") runAfter = true;
        else if (arg == "-o" && i + 1 < argc) outputFile = argv[++i];
        else if (inputFile.empty()) inputFile = arg;
    }

    if (inputFile.empty()) { printUsage(); return 1; }

    try {
        std::string source = readFile(inputFile);
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parse();

        if (showAST) {
            printAST(ast);
            return 0;
        }

        CodeGenerator codegen(!noGC);
        std::string js = codegen.generate(ast);

        if (!outputFile.empty()) {
            writeFile(outputFile, js);
            std::cout << "Compiled " << inputFile << " -> " << outputFile << "\n";
        } else if (runAfter) {
            std::string tmpFile = "/tmp/__holyjs_run.js";
            writeFile(tmpFile, js);
            std::string cmd = "node " + tmpFile;
            int ret = system(cmd.c_str());
            std::filesystem::remove(tmpFile);
            return ret;
        } else {
            std::cout << js;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
