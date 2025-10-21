/*
 * File: statement.cpp
 * -------------------
 * This file implements the constructor and destructor for
 * the Statement class itself.  Your implementation must do
 * the same for the subclasses you define for each of the
 * BASIC statements.
 */

#include "statement.hpp"
#include "program.hpp"


/* Implementation of the Statement class */

int stringToInt(std::string str);

Statement::Statement() = default;

Statement::~Statement() = default;

// RemStatement
RemStatement::RemStatement(TokenScanner &scanner) {
    (void) scanner;
}
void RemStatement::execute(EvalState &state, Program &program) {
    (void) state; (void) program;
}

// LetStatement
LetStatement::LetStatement(TokenScanner &scanner) {
    varName = scanner.nextToken();
    if (varName.empty() || !isalnum(varName[0])) error("SYNTAX ERROR");
    scanner.verifyToken("=");
    expr = readE(scanner);
    if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
}
LetStatement::~LetStatement() { delete expr; }
void LetStatement::execute(EvalState &state, Program &program) {
    (void) program;
    CompoundExp assign("=", new IdentifierExp(varName), expr);
    expr = nullptr; // ownership moved to assign temporary
    assign.eval(state);
}

// PrintStatement
PrintStatement::PrintStatement(TokenScanner &scanner) {
    expr = readE(scanner);
    if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
}
PrintStatement::~PrintStatement() { delete expr; }
void PrintStatement::execute(EvalState &state, Program &program) {
    (void) program;
    int v = expr->eval(state);
    std::cout << v << std::endl;
}

// InputStatement
InputStatement::InputStatement(TokenScanner &scanner) {
    varName = scanner.nextToken();
    if (varName.empty() || !isalnum(varName[0])) error("SYNTAX ERROR");
    if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
}
void InputStatement::execute(EvalState &state, Program &program) {
    (void) program;
    int x;
    if (!(std::cin >> x)) {
        std::cin.clear();
        std::string dummy;
        std::getline(std::cin, dummy);
        x = 0;
    }
    std::string rest;
    std::getline(std::cin, rest);
    state.setValue(varName, x);
}

// EndStatement
EndStatement::EndStatement(TokenScanner &scanner) {
    if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
}
void EndStatement::execute(EvalState &state, Program &program) {
    (void) state;
    program.requestStop();
}

// GotoStatement
GotoStatement::GotoStatement(TokenScanner &scanner) {
    std::string tok = scanner.nextToken();
    if (tok.empty() || !isdigit(tok[0])) error("SYNTAX ERROR");
    target = stringToInteger(tok);
    if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
}
void GotoStatement::execute(EvalState &state, Program &program) {
    (void) state;
    program.requestJump(target);
}

// IfStatement
IfStatement::IfStatement(TokenScanner &scanner) {
    lhs = readE(scanner);
    op = scanner.nextToken();
    if (!(op == "=" || op == "<" || op == ">")) error("SYNTAX ERROR");
    rhs = readE(scanner);
    scanner.verifyToken("THEN");
    std::string tok = scanner.nextToken();
    if (tok.empty() || !isdigit(tok[0])) error("SYNTAX ERROR");
    target = stringToInteger(tok);
    if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
}
IfStatement::~IfStatement() {
    delete lhs;
    delete rhs;
}
void IfStatement::execute(EvalState &state, Program &program) {
    int lv = lhs->eval(state);
    int rv = rhs->eval(state);
    bool cond = false;
    if (op == "=") cond = (lv == rv);
    else if (op == "<") cond = (lv < rv);
    else if (op == ">") cond = (lv > rv);
    if (cond) program.requestJump(target);
}
