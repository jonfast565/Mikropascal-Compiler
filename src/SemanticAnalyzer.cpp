/*
 * SemanticAnalyzer.cpp
 *
 *  Created on: Mar 19, 2014
 *      Author: Jon
 */

#include "SemanticAnalyzer.hpp"

SemanticAnalyzer::SemanticAnalyzer() {
    // semantic analyzer and no AST
    this->ast = nullptr;
    this->symbols = SymbolListPtr(new SymbolList());
}

SemanticAnalyzer::SemanticAnalyzer(AbstractTreePtr program_syntax) {
    // semantic analyzer with AST
    this->ast = program_syntax;
    this->symbols = SymbolListPtr(new SymbolList());
}

void SemanticAnalyzer::attach_syntax(AbstractTreePtr program_syntax) {
    // set program syntax
    this->ast = program_syntax;
}

void SemanticAnalyzer::generate_symbols() {
    // generate symbols if there's an AST
    if (this->ast == nullptr) {
        return;
    }
    // stacks
    AbstractStackPtr iter_stack =
    AbstractStackPtr(new AbstractNodeStack());
    AbstractStackPtr reversal_stack =
    AbstractStackPtr(new AbstractNodeStack());
    // iterate through each
    while (!iter_stack->empty()) {
        // get the top and pop
        AbstractNodePtr current = iter_stack->top();
        iter_stack->pop();
        if (current->get_is_rule()) {
            // push on in reverse
            for (auto i = current->get_child_begin();
                 i != current->get_child_end(); i++) {
                reversal_stack->push(*i);
            }
            // reverse
            while(!reversal_stack->empty()) {
                iter_stack->push(reversal_stack->top());
                reversal_stack->pop();
            }
        }
    }
}