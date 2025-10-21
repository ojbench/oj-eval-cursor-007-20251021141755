/*
 * File: program.cpp
 * -----------------
 * This file is a stub implementation of the program.h interface
 * in which none of the methods do anything beyond returning a
 * value of the correct type.  Your job is to fill in the bodies
 * of each of these methods with an implementation that satisfies
 * the performance guarantees specified in the assignment.
 */

#include "program.hpp"
#include "statement.hpp"
#include "evalstate.hpp"



Program::Program() = default;

Program::~Program() = default;

void Program::clear() {
    // delete existing parsed statements
    for (auto &kv : lineNumberToStatement) {
        delete kv.second;
    }
    lineNumberToStatement.clear();
    lineNumberToSource.clear();
    lineNumberToTokenString.clear();
}

void Program::addSourceLine(int lineNumber, const std::string &line) {
    // If replacing an existing line, delete its parsed statement
    auto itStmt = lineNumberToStatement.find(lineNumber);
    if (itStmt != lineNumberToStatement.end()) {
        delete itStmt->second;
        lineNumberToStatement.erase(itStmt);
    }
    lineNumberToSource[lineNumber] = line;
    // do not touch token string here; caller may set it explicitly
}

void Program::removeSourceLine(int lineNumber) {
    auto itSrc = lineNumberToSource.find(lineNumber);
    if (itSrc != lineNumberToSource.end()) {
        lineNumberToSource.erase(itSrc);
    }
    auto itTok = lineNumberToTokenString.find(lineNumber);
    if (itTok != lineNumberToTokenString.end()) {
        lineNumberToTokenString.erase(itTok);
    }
    auto itStmt = lineNumberToStatement.find(lineNumber);
    if (itStmt != lineNumberToStatement.end()) {
        delete itStmt->second;
        lineNumberToStatement.erase(itStmt);
    }
}

std::string Program::getSourceLine(int lineNumber) {
    auto it = lineNumberToSource.find(lineNumber);
    if (it == lineNumberToSource.end()) return "";
    return it->second;
}

void Program::setParsedStatement(int lineNumber, Statement *stmt) {
    // must exist as a source line
    if (lineNumberToSource.find(lineNumber) == lineNumberToSource.end()) {
        error("LINE NUMBER ERROR");
    }
    auto it = lineNumberToStatement.find(lineNumber);
    if (it != lineNumberToStatement.end()) {
        delete it->second;
    }
    lineNumberToStatement[lineNumber] = stmt;
}

//void Program::removeSourceLine(int lineNumber) {

Statement *Program::getParsedStatement(int lineNumber) {
   auto it = lineNumberToStatement.find(lineNumber);
   if (it == lineNumberToStatement.end()) return nullptr;
   return it->second;
}

int Program::getFirstLineNumber() {
    if (lineNumberToSource.empty()) return -1;
    return lineNumberToSource.begin()->first;
}

int Program::getNextLineNumber(int lineNumber) {
    auto it = lineNumberToSource.upper_bound(lineNumber);
    if (it == lineNumberToSource.end()) return -1;
    return it->first;
}

void Program::run(EvalState &state) {
    stopped = false;
    jumped = false;
    jumpTarget = -1;
    currentLineNumber = getFirstLineNumber();
    while (!stopped && currentLineNumber != -1) {
        Statement *stmt = getParsedStatement(currentLineNumber);
        if (stmt == nullptr) {
            // Reparse if needed is out of scope; treat missing as REM
            currentLineNumber = getNextLineNumber(currentLineNumber);
            continue;
        }
        jumped = false;
        stmt->execute(state, *this);
        if (stopped) break;
        if (jumped) {
            currentLineNumber = jumpTarget;
            // If target not exists, stop
            if (lineNumberToSource.find(currentLineNumber) == lineNumberToSource.end()) {
                error("LINE NUMBER ERROR");
            }
            continue;
        }
        currentLineNumber = getNextLineNumber(currentLineNumber);
    }
}

void Program::requestJump(int lineNumber) {
    jumped = true;
    jumpTarget = lineNumber;
}

void Program::requestStop() {
    stopped = true;
}

int Program::getCurrentLineNumber() const {
    return currentLineNumber;
}

void Program::setLineNumberString(int lineNumber, const std::string &originalToken) {
    lineNumberToTokenString[lineNumber] = originalToken;
}

std::vector<std::pair<std::string, std::string>> Program::getAllLines() const {
    std::vector<std::pair<std::string, std::string>> res;
    res.reserve(lineNumberToSource.size());
    for (const auto &kv : lineNumberToSource) {
        int ln = kv.first;
        auto itTok = lineNumberToTokenString.find(ln);
        std::string lntxt = (itTok == lineNumberToTokenString.end() ? std::to_string(ln) : itTok->second);
        res.emplace_back(lntxt, kv.second);
    }
    return res;
}


