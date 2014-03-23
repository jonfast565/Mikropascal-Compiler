/*
 * SemanticAnalyzer.hpp
 *
 *  Created on: Mar 19, 2014
 *      Author: Jon
 */

#ifndef semanticanalyzer_h
#define semanticanalyzer_h

#include "Parser.hpp"
#include "Standard.hpp"
#include "Rules.hpp"
#include "Tokens.hpp"
#include "Symbols.hpp"

class Generator {
public:
    virtual void generate_pre() = 0;
    virtual void generate_post() = 0;
};

class CodeBlock: public Generator {
private:
    string rule;
public:
    CodeBlock(string rule): rule(rule){};
    virtual ~CodeBlock() = default;
};

enum IOAction {
    IO_READ,
    IO_WRITE
};

class IOBlock: public CodeBlock {
private:
    SymbolListPtr args;
    IOAction action;
public:
    IOBlock(IOAction action): CodeBlock("IOBlock"), action(action) {
        this->args = SymbolListPtr(new SymbolList());
    }
    virtual ~IOBlock() = default;
};

enum LoopType {
    WHILELOOP,
    FORLOOP
};

class LoopBlock: public CodeBlock {
private:
    LoopType type;
public:
    LoopBlock(LoopType type): CodeBlock("LoopBlock"), type(type){};
    virtual ~LoopBlock() = default;
};

class ConditionalBlock: public CodeBlock {
private:
public:
    ConditionalBlock();
    virtual ~ConditionalBlock() = default;
};

class AssignmentBlock: public CodeBlock {
private:
    bool processed;
public:
    AssignmentBlock();
    virtual ~AssignmentBlock() = default;
    void convert_postfix();
};

enum ActivationType {
    PROCEDURE,
    FUNCTION
};

class ActivationBlock: public CodeBlock {
private:
    SymbolPtr record;
    ActivationType type;
public:
    ActivationBlock(ActivationType type, SymbolPtr record):
    CodeBlock("ActivationType"), type(type) {
        this->record = record;
    }
    virtual ~ActivationBlock() = default;
};

class SemanticAnalyzer {
private:
	AbstractTreePtr ast;
    SymbolListPtr symbols;
    bool processed;
public:
    SemanticAnalyzer();
    SemanticAnalyzer(AbstractTreePtr program_syntax);
    virtual ~SemanticAnalyzer() = default;
    void attach_syntax(AbstractTreePtr program_syntax);
    void generate_symbols();
    void generate_blocks();
};

#endif
