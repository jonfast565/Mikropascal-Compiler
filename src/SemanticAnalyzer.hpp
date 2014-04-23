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
class AssignmentBlock;
class ConditionalBlock;
class ActivationBlock;
class LoopBlock;
class IOBlock;
class JumpBlock;

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
using AssignmentBlockPtr = shared_ptr<AssignmentBlock>;
using ConditionalBlockPtr = shared_ptr<ConditionalBlock>;
using ActivationBlockPtr = shared_ptr<ActivationBlock>;
using LoopBlockPtr = shared_ptr<LoopBlock>;
using IOBlockPtr = shared_ptr<IOBlock>;
using JumpBlockPtr = shared_ptr<JumpBlock>;

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
    ACTIVATION_BLOCK,
    JUMP_BLOCK
};

enum InstructionType {
    MOV,
    PUSH,
    POP,
    HLT
};

class Generator {
public:
    virtual void generate_pre() = 0;
    virtual void generate_post() = 0;
    virtual bool validate() = 0;
    virtual void preprocess() = 0;
    virtual ~Generator() = default;
};

class CodeBlock: public Generator {
private:
    CodeBlockListPtr block_list;
    BlockType block_type;
    TokenListPtr unprocessed;
    SymbolListPtr temp_symbols;
    CodeBlockPtr parent_block;
    SemanticAnalyzerPtr parent_analyzer;
    bool valid;
public:
    CodeBlock(BlockType block_type, CodeBlockPtr parent_block):
    block_type(block_type), parent_block(parent_block){
        this->block_list = CodeBlockListPtr(new CodeBlockList());
        this->unprocessed = TokenListPtr(new TokenList());
        this->parent_analyzer = nullptr;
        this->temp_symbols = SymbolListPtr(new SymbolList());
        this->valid = true;
    };
    virtual ~CodeBlock() = default;
    CodeBlockListPtr get_block_list() { return this->block_list; }
    BlockType get_block_type() { return this->block_type; }
    TokenListPtr get_unprocessed() { return this->unprocessed; }
    SymbolListPtr get_symbol_list() { return this->temp_symbols; }
    CodeBlockPtr get_parent_block() { return this->parent_block; }
    SemanticAnalyzerPtr get_analyzer() { return this->parent_analyzer; }
    bool get_valid() { return this->valid; }
    void set_valid(bool new_valid) { this->valid = new_valid; }
    virtual void generate_pre();
    virtual void generate_post();
    virtual void preprocess();
    virtual bool validate();
    virtual void catch_token(TokenPtr symbol);
    void append(CodeBlockPtr block);
    void set_parent(CodeBlockPtr parent);
    void set_analyzer(SemanticAnalyzerPtr analyzer);
    unsigned int get_nesting_level();
    bool check_filter_size(SymbolListPtr filtered);
    static bool is_operator(SymbolPtr character);
    static bool is_operand(SymbolPtr character);
    static bool is_lparen(SymbolPtr character);
    static bool is_rparen(SymbolPtr character);
    static int compare_ops(SymbolPtr c1, SymbolPtr c2);
    static int op_precendence(SymbolPtr c1);
    void convert_postfix();
    void emit(InstructionType ins, vector<string> operands);
    VarType make_cast(VarType v1, VarType v2);
    VarType generate_expr(SymbolListPtr expr_list);
    CodeBlockPtr get_parent();
    CodeBlockList::iterator inner_begin();
    CodeBlockList::iterator inner_end();
    SymbolPtr translate(TokenPtr token);
};

class ProgramBlock: public CodeBlock {
public:
    ProgramBlock(): CodeBlock(PROGRAM_BLOCK, nullptr){};
    ~ProgramBlock() = default;
    virtual void generate_pre();
    virtual void generate_post();
    virtual void preprocess();
    virtual bool validate();
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
    ~IOBlock() = default;
    virtual void generate_pre();
    virtual void generate_post();
    virtual void preprocess();
    virtual void catch_token(TokenPtr symbol);
    virtual bool validate();
};

enum LoopType {
    WHILELOOP,
    FORLOOP,
    RPTUNTLLOOP
};

class LoopBlock: public CodeBlock {
private:
    LoopType type;
    string cond_label;
    string body_label;
    string exit_label;
public:
    LoopBlock(LoopType type): CodeBlock(LOOP_BLOCK, nullptr),
    type(type) {
        this->cond_label = "";
        this->body_label = "";
        this->exit_label = "";
    };
    ~LoopBlock() = default;
    virtual void generate_pre();
    virtual void generate_post();
    virtual void preprocess();
    virtual void catch_token(TokenPtr symbol);
    virtual bool validate();
};

enum CondType {
    COND_IF,
    COND_ELSE
};

class ConditionalBlock: public CodeBlock {
private:
    ConditionalBlockPtr connected;
    CondType cond;
    string body_label;
    string else_label;
    string exit_label;
public:
    ConditionalBlock(CondType cond): CodeBlock(CONDITIONAL_BLOCK, nullptr),
    connected(nullptr), cond(cond){
        this->body_label = "";
        this->else_label = "";
        this->exit_label = "";
    };
    ConditionalBlock(ConditionalBlockPtr connected, CondType cond): CodeBlock(CONDITIONAL_BLOCK, nullptr),
    connected(connected), cond(cond){
        this->body_label = "";
        this->else_label = "";
        this->exit_label = "";
    };
    ~ConditionalBlock() = default;
    virtual void generate_pre();
    virtual void generate_post();
    virtual void preprocess();
    virtual void catch_token(TokenPtr symbol);
    virtual bool validate();
    CondType get_conditional_type();
    void set_else_label(string else_label);
    void generate_exit_label();
    string get_exit_label();
    void set_connected(ConditionalBlockPtr connected);
};

class AssignmentBlock: public CodeBlock {
private:
    SymbolPtr assigner;
    VarType expr_type;
    bool expr_only;
public:
    AssignmentBlock(bool expr_only): CodeBlock(ASSIGNMENT_BLOCK, nullptr), expr_type(VOID), expr_only(expr_only){
        this->assigner = SymbolPtr(new SymConstant("", VOID));
    };
    ~AssignmentBlock() = default;
    virtual void generate_pre();
    virtual void generate_post();
    virtual void preprocess();
    virtual void catch_token(TokenPtr symbol);
    virtual bool validate();
    SymbolPtr get_assigner();
    VarType get_expr_type();
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

class JumpBlock: public CodeBlock {
private:
    string program_section;
    bool jump_around;
public:
    JumpBlock(bool jump_around): CodeBlock(JUMP_BLOCK, nullptr),
    jump_around(jump_around) {
        this->program_section = "";
    }
    ~JumpBlock() = default;
    virtual void generate_pre();
    virtual void generate_post();
    virtual void preprocess();
    virtual void catch_token(TokenPtr symbol);
    virtual bool validate();
};

class ActivationBlock: public CodeBlock {
private:
    ActivationType activation;
    ActivityType activity;
    SymCallablePtr record;
    string caller_name;
    string begin_label;
public:
    ActivationBlock(ActivationType activation, ActivityType activity, SymCallablePtr record):
    CodeBlock(ACTIVATION_BLOCK, nullptr), activation(activation), activity(activity) {
        this->record = record;
        this->begin_label = "";
    }
    ~ActivationBlock() = default;
    virtual void generate_pre();
    virtual void generate_post();
    virtual void preprocess();
    virtual void catch_token(TokenPtr symbol);
    virtual bool validate();
    string get_start();
    ActivityType get_activity();
    void set_caller_name(string id);
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
    unique_ptr<stack<CodeBlockPtr>> block_stack;
    // labels for code generation
    unsigned int label_count;
    // file for writing stuff
    unique_ptr<ofstream> file_writer;
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
    void write_raw(string raw);
    CodeBlockPtr get_top_block();
    string generate_label();
};

#endif
