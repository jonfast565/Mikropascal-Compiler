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

class SymProcedure;
class SymFunction;
class SymVariable;
class SymIdentifier;
class SymLiteral;
class SymArgument;

// type predecls
using ArgumentPtr = shared_ptr<SymArgument>;
using ArgumentList = vector<ArgumentPtr>;
using ArgumentListPtr = shared_ptr<vector<ArgumentList>>;
using SymbolPtr = shared_ptr<Symbol>;

// symbol types
enum SymType {
	SYM_PROCEDURE,
	SYM_FUNCTION,
	SYM_VARIABLE,
	SYM_IDENTIFIER,
	SYM_LITERAL
};

// variable types
enum VarType {
	STRING,
	INTEGER,
	FLOATING,
	BOOLEAN
};

enum LiteralType {
	STRING_LITERAL,
	INTEGER_LITERAL,
	FLOATING_LITERAL
};

enum PassType {
	VALUE,
	REFERENCE
};

enum Scope {
	GLOBAL,
	LOCAL
};

// symbol class
class Symbol {
private:
	SymType symbol_type;
	string symbol_name;
	Scope symbol_scope;
public:
	Symbol(string name, SymType type, Scope scope):
		symbol_name(name), symbol_type(type), symbol_scope(scope){};
	virtual ~Symbol() = default;

};

class SymProcedure : public Symbol {
private:
	ArgumentListPtr argument_list;
public:
	SymProcedure(string name, Scope scope): Symbol(name, SYM_PROCEDURE, scope) {
		this->argument_list = ArgumentListPtr(new ArgumentList());
	}
	virtual ~SymProcedure() = default;
};

class SymFunction : public SymProcedure {
private:
	VarType return_type;
public:
	SymFunction(string name, VarType return_type, Scope scope): Symbol(name, SYM_FUNCTION, scope),
	return_type(return_type){};
	virtual ~SymFunction() = default;
};

class SymVariable : public Symbol {
private:
	VarType variable_type;
public:
	SymVariable(string name, VarType type, Scope scope):
		Symbol(name, SYM_VARIABLE, scope), variable_type(type){};
	virtual ~SymVariable() = default;
};

class SymIdentifier : public Symbol {
private:
	VarType variable_type;
public:
	SymIdentifier(string name, VarType type, Scope scope):
		Symbol(name, SYM_IDENTIFIER, scope), variable_type(type){};
	virtual ~SymIdentifier() = default;
};

class SymLiteral : public Symbol {
private:
	LiteralType literal_type;
public:
	SymLiteral(string name, LiteralType type, Scope scope):
	Symbol(name, SYM_LITERAL, scope), literal_type(type){};
	virtual ~SymIdentifier() = default;
};

class SymArgument : public SymVariable {
private:
	PassType pass_type;
public:
	SymArgument(string name, VarType type, Scope scope, PassType pass):
		SymVariable(name, type, scope), pass_type(pass){}
	virtual ~SymArgument() = default;
};

#endif
