/*
 * File: Basic.cpp
 * ---------------
 * This file is the starter project for the BASIC interpreter.
 */

#include <cctype>
#include <iostream>
#include <string>
#include "exp.hpp"
#include "parser.hpp"
#include "program.hpp"
#include "Utils/error.hpp"
#include "Utils/tokenScanner.hpp"
#include "Utils/strlib.hpp"


/* Function prototypes */

void processLine(std::string line, Program &program, EvalState &state);

/* Main program */

int main() {
    EvalState state;
    Program program;
    //cout << "Stub implementation of BASIC" << endl;
    while (true) {
        try {
            std::string input;
            if (!getline(std::cin, input)) break;
            if (input.empty()) continue;
            processLine(input, program, state);
        } catch (ErrorException &ex) {
            std::cout << ex.getMessage() << std::endl;
        }
    }
    return 0;
}

/*
 * Function: processLine
 * Usage: processLine(line, program, state);
 * -----------------------------------------
 * Processes a single line entered by the user.  In this version of
 * implementation, the program reads a line, parses it as an expression,
 * and then prints the result.  In your implementation, you will
 * need to replace this method with one that can respond correctly
 * when the user enters a program line (which begins with a number)
 * or one of the BASIC commands, such as LIST or RUN.
 */

void processLine(std::string line, Program &program, EvalState &state) {
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.setInput(line);

    // Decide if program line (starts with number) or command
    std::string first = scanner.nextToken();
    if (first == "") return;
    TokenType t = scanner.getTokenType(first);
    if (t == NUMBER) {
        int lineNumber = stringToInteger(first);
        // If rest is empty -> delete
        if (!scanner.hasMoreTokens()) {
            program.removeSourceLine(lineNumber);
            return;
        }
        std::string rest = line.substr(line.find_first_of(" \t") + 1);
        program.addSourceLine(lineNumber, rest);
        program.setLineNumberString(lineNumber, first);

        // parse statement for storage
        TokenScanner sc2;
        sc2.ignoreWhitespace();
        sc2.scanNumbers();
        sc2.setInput(rest);
        std::string kw = toUpperCase(sc2.nextToken());
        Statement *stmt = nullptr;
        if (kw == "REM") {
            stmt = new RemStatement(sc2);
        } else if (kw == "LET") {
            stmt = new LetStatement(sc2);
        } else if (kw == "PRINT") {
            stmt = new PrintStatement(sc2);
        } else if (kw == "INPUT") {
            stmt = new InputStatement(sc2);
        } else if (kw == "END") {
            stmt = new EndStatement(sc2);
        } else if (kw == "GOTO") {
            stmt = new GotoStatement(sc2);
        } else if (kw == "IF") {
            stmt = new IfStatement(sc2);
        } else {
            error("SYNTAX ERROR");
        }
        program.setParsedStatement(lineNumber, stmt);
        return;
    }

    // Immediate commands
    std::string cmd = toUpperCase(first);
    if (cmd == "REM") {
        // ignore rest
        return;
    } else if (cmd == "LET") {
        Statement *stmt = new LetStatement(scanner);
        stmt->execute(state, program);
        delete stmt;
    } else if (cmd == "PRINT") {
        Statement *stmt = new PrintStatement(scanner);
        stmt->execute(state, program);
        delete stmt;
    } else if (cmd == "INPUT") {
        Statement *stmt = new InputStatement(scanner);
        stmt->execute(state, program);
        delete stmt;
    } else if (cmd == "END") {
        // END in immediate mode: do nothing
        if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
    } else if (cmd == "GOTO") {
        GotoStatement tmp(scanner);
        // In immediate mode, jumping is undefined; treat as error
        error("SYNTAX ERROR");
    } else if (cmd == "IF") {
        IfStatement tmp(scanner);
        error("SYNTAX ERROR");
    } else if (cmd == "RUN") {
        if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
        program.run(state);
    } else if (cmd == "LIST") {
        if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
        // print stored program preserving original line number token
        for (const auto &p : program.getAllLines()) {
            std::cout << p.first << ' ' << p.second << std::endl;
        }
    } else if (cmd == "CLEAR") {
        if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
        program.clear();
        state.Clear();
    } else if (cmd == "QUIT") {
        if (scanner.hasMoreTokens()) error("SYNTAX ERROR");
        exit(0);
    } else if (cmd == "HELP") {
        // optional: ignore
    } else {
        error("SYNTAX ERROR");
    }
}

