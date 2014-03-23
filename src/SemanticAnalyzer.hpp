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

class SemanticAnalyzer {
private:
	AbstractNodePtr ast;
    SymbolListPtr symbols;
    bool processed;
public:
    SemanticAnalyzer();
    virtual ~SemanticAnalyzer() = default;
    void process_ast();
    void generate_symbols();
    void generate_blocks();
};

#endif
