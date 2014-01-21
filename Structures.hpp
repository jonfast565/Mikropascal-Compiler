#pragma once

#ifndef structures
#define structures

// standard includes
#include "Standard.hpp"

// stack node
template <class Item>
class StackElement
{
private:
	// position and item
	Item item;
	bool is_base;

	// previous and next pointers
	// StackNode* previous;
	StackElement* next;

public:
	// ctor and dtor
	StackElement(Item new_item) { this->item = new_item; this->is_base = false; this->next = nullptr;}
	StackElement() { this->is_base = true; this->next = nullptr;}
	~StackElement() {};

	// getter and setter methods
	Item get_item()
	{
		return item;
	}

	void set_item(Item new_item)
	{
		this->item = new_item;
	}

	/*
	void set_prev(StackElement* prev_node)
	{
		this->previous = prev_node;
	}
	*/

	void set_next(StackElement* next_node)
	{
		this->next = next_node;
	}

	/*
	StackElement* get_prev()
	{
		return this->previous;
	}
	*/

	StackElement* get_next()
	{
		return this->next;
	}
};

// stack structure
template <class Item>
class Stack
{
private:
	// keep track of the stack bounds
	StackElement<Item>* top;
	StackElement<Item>* base;
	// keep track of the length
	// int length;

	void set_top(StackElement<Item> * new_top)
	{
		this->top = new_top;
	}

	void set_next(StackElement<Item>* current, StackElement<Item>* new_next)
	{
		current->set_next(new_next);
	}

	StackElement<Item>* get_base()
	{
		return this->base;
	}

	void set_base(StackElement<Item>* new_base)
	{
		this->base = new_base;
	}

public:
	Stack()
	{ 
		this->set_base(new StackElement<Item>());
		this->top = nullptr;
		this->set_top(this->base);
	}

	~Stack()
	{
		while (top != base)
		{
			StackElement<Item>* current_top = this->get_top();
			this->set_top(top->get_next());
			delete current_top;
		}
		delete base;
	};

	StackElement<Item>* get_top()
	{
		return this->top;
	}

	Item pop()
	{
		if (this->get_top() != this->get_base())
		{
			StackElement<Item>* last_top = get_top();
			Item ret_item = peek();
			this->set_top(top->get_next());
			delete last_top;
			return ret_item;
		} else return (Item) nullptr;
	}

	Item peek()
	{
		if (this->get_top() != this->get_base()) return top->get_item();
		else return (Item) nullptr;
	}

	void push(Item item) 
	{
		StackElement<Item>* new_node = new StackElement<Item>(item);
		if (this->get_top() == this->get_base())
		{
			this->set_top(new_node);
			this->set_next(top, base);
		}
		else
		{
			StackElement<Item>* prev_node = this->get_top();
			this->set_top(new_node);
			this->set_next(new_node, prev_node);
		}
	}

	void print()
	{
		StackElement<Item>* current = this->get_top();
		while(current != this->get_base())
		{
			cout << current->get_item() << " -> ";
			current = current->get_next();
		}
		cout << "Base\n" << endl;
	}

	unique_ptr<string> to_string()
	{
		unique_ptr<string> stack_rep(new string(""));
		StackElement<Item>* current = this->get_top();
		while(current != this->get_base())
		{
			ostringstream s;
			s << current->get_item();
			stack_rep->append(s.str());
			stack_rep->append(" -> ");
			current = current->get_next();
		}
		stack_rep->append("Base\n");
		return stack_rep;
	}
};

// linked list
template <class Item>
class Link {
private:
    bool root;
    Item data;
    Link<Item>* next_link;
    Link<Item>* prev_link;
public:
    Link(Item data) {
    	this->root = false;
        this->data = data;
        this->next_link = nullptr;
        this->prev_link = nullptr;
    };
    virtual ~Link(){};
    Item getData() {
        return this->data;
    }
    void set_data(Item new_data) {
        this->data = new_data;
    }
    Link<Item>* get_next() {
        return this->next_link;
    }
    void set_next(Link<Item>* next_link) {
        this->next_link = next_link;
    }
    void set_prev(Link<Item>* prev_link) {
        this->prev_link = prev_link;
    }
    Link<Item>* get_prev() {
        return this->prev_link;
    }
};

template <class Item>
class LinkList {
private:
    Link<Item>* begin;
    Link<Item>* end;
    Link<Item>* iterator;
    unsigned int num_elements;
public:
    LinkList() {
        this->begin = nullptr;
        this->end = nullptr;
        this->iterator = nullptr;
        this->num_elements = 0;
    }
    virtual ~LinkList() {
        Link<Item>* iterator = begin;
        while (iterator->next_link != nullptr) {
        	Link<Item>* prevIterator = iterator;
        	iterator = iterator->next_link;
        	delete prevIterator;
        }
        delete this;
    }
    Link<Item>* get_first() {
        return this->begin;
    }
    Link<Item>* get_last() {
        return this->end;
    }
    unsigned int get_num_elements() {
        return this->num_elements;
    }
    void add_link(Item new_item) {
		Link<Item>* new_link = new Link(new_item);
    	if (get_num_elements == 0) {
    		this->begin = new_link;
    		this->end = new_link;
    	} else {
    		this->end->set_prev(new_link);
        	this->end = new_link;
        	new_link->set_next(this->end);
    	}
    }
    /* // not implemented
    Item remove_link(Link<Item>* link) {
        return (Item) nullptr;
    }
    Item remove_link(unsigned int link_number) {
        if (link_number < num_elements) {
        	return (Item) nullptr;
        } else return (Item) nullptr;
    }
    */
};

#endif

