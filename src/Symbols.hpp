//
//  Symbols.h
//  CompilerTest
//
//  Created by Jonathan Fast on 2/9/14.
//  Copyright (c) 2014 JonFast. All rights reserved.
//

#ifndef symbols_h
#define symbols_h

#include "Standard.hpp"

class Symbol {
    
};

class SymbolTable {
private:
    shared_ptr<vector<shared_ptr<Symbol>>> symbol_table;
public:
    SymbolTable();
    virtual ~SymbolTable();
};

#endif
