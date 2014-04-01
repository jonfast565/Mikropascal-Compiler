/*
 * SemanticAnalyzer.hpp
 *
 *  Created on: Mar 19, 2014
 *      Author: Jon
 */

#ifndef semanticanalyzer_h
#define semanticanalyzer_h

#include "Standard.hpp"
#include "Rules.hpp"
#include "Tokens.hpp"
#include "Symbols.hpp"

class AbstractNode;
class AbstractTree;
class SemanticAnalyzer;
class CodeBlock;

using AbstractNodePtr = shared_ptr<AbstractNode>;
using AbstractNodeList = vector<AbstractNodePtr>;
using AbstractListPtr = shared_ptr<AbstractNodeList>;
using AbstractNodeStack = stack<AbstractNodePtr>;
using AbstractStackPtr = shared_ptr<AbstractNodeStack>;
using AbstractTreePtr = shared_ptr<AbstractTree>;
using SemanticAnalyzerPtr = shared_ptr<SemanticAnalyzer>;
using TokenPtr = shared_ptr<Token>;
using TokenList = vector<TokenPtr>;
using TokenListPtr = shared_ptr<TokenList>;
using CodeBlockPtr = shared_ptr<CodeBlock>;
using CodeBlockList = vector<CodeBlockPtr>;
using CodeBlockListPtr = shared_ptr<CodeBlockList>;

// AST Stuff
class AbstractNode {
private:
    bool is_root;
    bool is_rule;
    AbstractNodePtr parent_node;
    AbstractListPtr child_nodes;
    ParseType parse_type;
    TokenPtr token;
public:
    AbstractNode();
    AbstractNode(AbstractNodePtr parent_node, ParseType parse_type);
    AbstractNode(ParseType parse_type);
    AbstractNode(TokenPtr token);
    virtual ~AbstractNode() = default;
    void add_child_node(AbstractNodePtr child_node);
    void set_is_root(bool is_root);
    void set_parent(AbstractNodePtr parent_node);
    bool get_is_root();
    bool get_is_rule();
    bool get_is_epsilon();
    ParseType get_parse_type();
    TokenPtr get_token();
    AbstractNodePtr get_parent();
    AbstractNodeList::iterator get_child_begin();
    AbstractNodeList::iterator get_child_end();
};

class AbstractTree {
private:
	AbstractNodePtr root_node;
	AbstractNodePtr iterable;
	AbstractNodePtr get_current_parent();
	void display_tree_rec();
public:
	AbstractTree();
	AbstractTree(AbstractNodePtr root);
	virtual ~AbstractTree() = default;
    void add_move_child(AbstractNodePtr child_node);
	void goto_parent();
    void display_tree();
    void push_children(AbstractNodePtr current_node,
                       AbstractStackPtr current_symbols);
    AbstractNodePtr get_root_node();
};

enum BlockType {
    GENERIC_BLOCK,
    PROGRAM_BLOCK,
    IO_BLOCK,
    LOOP_BLOCK,
    CONDITIONAL_BLOCK,
    ASSIGNMENT_BLOCK,
    ACTIVATION_BLOCK
};

class Generator {
public:
    virtual void generate_pre() = 0;
    virtual void generate_post() = 0;
    virtual bool validate() = 0;
    virtual void preprocess() = 0;
};

class CodeBlock: public Generator {
protected:
    CodeBlockListPtr block_list;
    BlockType block_type;
    TokenListPtr unprocessed;
    SymbolListPtr temp_symbols;
    CodeBlockPtr parent_block;
    SemanticAnalyzerPtr parent_analyzer;
public:
    CodeBlock(BlockType block_type, CodeBlockPtr parent_block):
    block_type(block_type), parent_block(parent_block){
        this->block_list = CodeBlockListPtr(new CodeBlockList());
        this->unprocessed = TokenListPtr(new TokenList());
        this->parent_analyzer = nullptr;
        this->temp_symbols = SymbolListPtr(new SymbolList());
    };
    virtual ~CodeBlock() = default;
    void generate_pre();
    void generate_post();
    void preprocess();
    bool validate();
    void catch_token(TokenPtr symbol);
    void append(CodeBlockPtr block);
    void set_parent(CodeBlockPtr parent);
    void set_analyzer(SemanticAnalyzerPtr analyzer);
    SemanticAnalyzerPtr get_analyzer();
    CodeBlockPtr get_parent();
    CodeBlockList::iterator inner_begin();
    CodeBlockList::iterator inner_end();
};

class ProgramBlock: public CodeBlock {
public:
    ProgramBlock(): CodeBlock(PROGRAM_BLOCK, nullptr){};
    virtual ~ProgramBlock() = default;
    void generate_pre();
    void generate_post();
    void preprocess();
    bool validate();
};

enum IOAction {
    IO_READ,
    IO_WRITE
};

class IOBlock: public CodeBlock {
private:
    SymbolListPtr args;
    IOAction action;
    bool line_terminator;
public:
    IOBlock(IOAction action, bool newline): CodeBlock(IO_BLOCK, nullptr), action(action), line_terminator(newline) {
        this->args = SymbolListPtr(new SymbolList());
    }
    virtual ~IOBlock() = default;
    void generate_pre();
    void generate_post();
    void preprocess();
    bool validate();
};

enum LoopType {
    WHILELOOP,
    FORLOOP,
    RPTUNTLLOOP
};

class LoopBlock: public CodeBlock {
private:
    LoopType type;
public:
    LoopBlock(LoopType type): CodeBlock(LOOP_BLOCK, nullptr), type(type){};
    virtual ~LoopBlock() = default;
    void generate_pre();
    void generate_post();
    void preprocess();
    bool validate();
};

class ConditionalBlock: public CodeBlock {
private:
public:
    ConditionalBlock(): CodeBlock(CONDITIONAL_BLOCK, nullptr){};
    virtual ~ConditionalBlock() = default;
    void generate_pre();
    void generate_post();
    void preprocess();
    bool validate();
};

class AssignmentBlock: public CodeBlock {
private:
public:
    AssignmentBlock(): CodeBlock(ASSIGNMENT_BLOCK, nullptr){};
    virtual ~AssignmentBlock() = default;
    void convert_postfix();
    void generate_pre();
    void generate_post();
    void preprocess();
    bool validate();
};

enum ActivationType {
    PROCEDURE,
    FUNCTION,
    NO_CALL
};

enum ActivityType {
    DEFINITION,
    CALL
};

class ActivationBlock: public CodeBlock {
private:
    SymbolPtr record;
    ActivationType activation;
    ActivityType activity;
public:
    ActivationBlock(ActivationType activation, ActivityType activity, SymbolPtr record):
    CodeBlock(ACTIVATION_BLOCK, nullptr), activity(activity), activation(activation) {
        this->record = record;
    }
    virtual ~ActivationBlock() = default;
    void generate_pre();
    void generate_post();
    void preprocess();
    bool validate();
};

class SemanticAnalyzer {
private:
    // AST for debug
	AbstractTreePtr ast;
    // symbols for symbol table
    SymTablePtr symbols;
    // CondensedST is the tree that generates code
    CodeBlockPtr condensedst;
    // stack of pointer block is effectively the iterator
    shared_ptr<stack<CodeBlockPtr>> block_stack;
    // if processed correctly, allow generated code
    bool processed;
    // labels for code generation
    unsigned int label_count;
public:
    SemanticAnalyzer();
    virtual ~SemanticAnalyzer() = default;
    AbstractTreePtr get_ast();
    SymTablePtr get_symtable();
    bool is_scoped_any(string id);
    bool is_callable_scoped(string callable_id);
    bool is_data_scoped(string data_id);
    bool is_data_in_callable(string data_id, string callable_id);
    void print_symbols();
    void generate_all();
    void generate_one(CodeBlockPtr current);
    void feed_token(TokenPtr token);
    void append_block(CodeBlockPtr new_block);
    void rappel_block();
    CodeBlockPtr get_top_block();
    string generate_label();
};

#endif
