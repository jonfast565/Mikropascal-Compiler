/*
 * Automata.hpp
 *
 *  Created on: Jan 19, 2014
 *  Author: Jon
 *  NOTE: If you value your life,
 *  this file will remain untouched by project
 *  partners. Thanks!
 */

#ifndef automata_h
#define automata_h

#include "Standard.hpp"

#define AUTOMATA_DEBUG 0

class FiniteAutomataContainer;
class FiniteAutomataState;

// type aliases
using StatePtr = shared_ptr<FiniteAutomataState>;
using StateList = vector<StatePtr>;
using StateListPtr = shared_ptr<StateList>;
using StateIterator = StateList::iterator;
using StateIteratorList = shared_ptr<vector<StateIterator>>;
using TransitionMap = map<char, StateList::iterator>;
using TransitionMapPtr = shared_ptr<TransitionMap>;

// class that creates a state in an FA
class FiniteAutomataState {
private:
	bool is_final;
	bool is_initial;
	string name;
	TransitionMapPtr to_state;
public:
	// ctor
	FiniteAutomataState(bool is_initial, bool is_final, string name) :
			is_final(is_final), is_initial(is_initial), name(name) {
		this->to_state = TransitionMapPtr(new TransitionMap);
	}
	// dtor
	virtual ~FiniteAutomataState() {
		to_state->clear();
	}
	// get initial state
	bool get_is_initial() {
		return this->is_initial;
	}
	// get final state
	bool get_is_final() {
		return this->is_final;
	}
	// get FA name
	string get_name() {
		return this->name;
	}
	// set FA name
	void set_name(string name) {
		this->name = name;
	}
	// set initial state
	void set_is_initial(bool is_initial) {
		this->is_initial = is_initial;
	}
	// set final state
	void set_is_final(bool is_final) {
		this->is_final = is_final;
	}
	// add a transition to this state
	void add_transition(char on, StateIterator new_state) {
		this->to_state->insert(pair<char, StateIterator>(on, new_state));
	}
	// remove a transition from this state
	void remove_transition(char on) {
		this->to_state->erase(on);
	}
	// modify a transition from this state to another
	void change_transition(char on, StateIterator new_state) {
		this->to_state->erase(on);
		this->to_state->insert(pair<char, StateIterator>(on, new_state));
	}
	// return the map of character to transitions (iterators to new states)
	shared_ptr<map<char, StateIterator>> get_transitions() {
		return this->to_state;
	}
	// return a particular transition to another state (iterator)
	StateIterator get_transition(char input_char) {
		return (*to_state)[input_char];
	}
	// determine if a particular transition exists
	bool transition_exists(char through) {
		// ensure that a state we find is not invalid
		// by comparing against the end() iterator
		if (this->to_state->find(through) != this->to_state->end())
			return true;
		else
			return false;
	}
	// state comparison by pointed to type via shared_ptr<T>
	// names are compared for equality
	inline bool operator==(const StatePtr rhs) {
		if (this->name.compare(rhs->name) == 0)
			return true;
		else
			return false;
	}
};

// class that creates a container in an FA
class FiniteAutomataContainer {
private:
	// consists of a vector of states, and an iterator over that
	// vector for running the machine
	shared_ptr<vector<StatePtr>> states;
	StateIterator run_ptr;
	shared_ptr<vector<StatePtr>> dead_states;
	// info variables like a machine name
	// and if a DEAD state exists
	bool dead_state_exists;
	string name;

	// get an iterator for the first element of the state vector
	StateIterator get_begin_iter() {
		return this->states->begin();
	}

	// get an iterator for the last element of the state vector
	StateIterator get_end_iter() {
		return this->states->end();
	}

public:
	// construct an entire finite automata (ctor)
	FiniteAutomataContainer(string name) :
			dead_state_exists(true), name(name) {
		// create a list of states
		this->states = shared_ptr<vector<StatePtr>>(new vector<StatePtr>);
		this->dead_states = shared_ptr<vector<StatePtr>>(new vector<StatePtr>);
		// automatically create a dead state (hack), that the iterator goes to if a match is not found
		this->dead_states->push_back(
				StatePtr(new FiniteAutomataState(false, false, "DEAD")));
		// move the run pointer to the end state (as for now it is invalid)
		// this is the equivalent of the pointer in an array of char
		this->run_ptr = this->states->end();
	}

	// construct an FA with the option of setting if a dead state exists
	// performs the same function as the ctor above
	FiniteAutomataContainer(string name, bool dead_states_enabled) :
			dead_state_exists(dead_states_enabled), name(name) {
		this->states = shared_ptr<vector<StatePtr>>(new vector<StatePtr>);
        this->dead_states = shared_ptr<vector<StatePtr>>(new vector<StatePtr>);
		if (dead_state_exists) {
			this->dead_states->push_back(
					StatePtr(new FiniteAutomataState(false, false, "DEAD")));
		}
		this->run_ptr = this->states->end();
	}

	// destructor clears all loose states
	virtual ~FiniteAutomataContainer() {
		this->states->clear();
	}

	// get this automata's name
	string get_name() {
		return this->name;
	}

	// set this automata's name
	void set_name(string name) {
		this->name = name;
	}

	// add a state to this automata
	void add_state(string state_name, bool is_initial, bool is_final) {
		try {
			// ensure that the state we're adding is not a copy of a previous one
			StateIterator old_state = this->get_state(state_name);
			// error throws a copy state and exits
			if (old_state != states->end())
				throw string("COPY STATE");
			else {
				// otherwise add this as a state
				this->states->push_back(
						StatePtr(new FiniteAutomataState(is_initial, is_final, state_name)));
				// get a state iterator to this new state, so we can edit it a little
				StateIterator new_state_iter = this->get_state(state_name);
				// set new initial and final states if this node has
				// been constructed as initial/final
				if ((*new_state_iter)->get_is_initial() == true) {
					this->set_initial_state((*new_state_iter)->get_name());
				}
				if ((*new_state_iter)->get_is_final() == true) {
					this->set_final_state((*new_state_iter)->get_name(), true);
				}
			}
			// reset the run pointer to a new initial state if necessary
			this->reset();
		} catch (string& exception) {
			return;
		}
	}

	// remove a state
	void remove_state(string state_name) {
		try {
			// get an iterator to a null state (meaning we need to search for the correct one)
			StateIterator rem_target = states->end();
			// get the exact item for removal
			for (StateIterator i = this->get_begin_iter();
					i != this->get_end_iter(); ++i)
				// we need to find an item with the same state name, which is the correct
				if ((*i)->get_name().compare(state_name) == 0) {
					// iterator now set to remove i
					rem_target = i;
					break;
				}
			// if state found is null
			if (rem_target == states->end())
				throw string("NO SUCH STATE");
			// otherwise, remove all invalid transitions to the state we took away
			// look through each state's transition map to find the appropriate matches
			for (StateIterator i = this->get_begin_iter();
					i != this->get_end_iter(); ++i) {
				// iterate through the map
				for (map<char, StateIterator>::iterator t =
						(*i)->get_transitions()->begin();
						t != (*i)->get_transitions()->end(); ++t) {
					// if the removal iterator rem_target is equivalent to any iterator
					// that points to itself
					if ((*i) == *(t->second)) {
						// erase the transition that is pointed to by the map iterator
						(*i)->get_transitions()->erase(t);
					}
				}
			}
			// finally, erase the state when all transitions are dealt with
			this->states->erase(rem_target);
		} catch (string &exception) {
			// fall through, can't remove a state that's not there
			return;
		}
	}

	// get a state
	StateIterator get_state(string state_name) {
		for (StateIterator i = this->get_begin_iter();
				i != this->get_end_iter(); ++i) {
			// compare names of states to determine if we can obtain
			if ((*i)->get_name().compare(state_name) == 0) {
				return i;
			}
		}
		// otherwise, return null state
		return states->end();
	}

	// return all states that are not equivalent to the one given
	StateIteratorList get_states_not(string opposite_state_name) {
		// create a vector to retrieve all other states than the one described by
		// the input string
		StateIteratorList opposite_states = StateIteratorList(
				new vector<StateIterator>);
		// iterate through all states
		for (StateIterator i = this->get_begin_iter();
				i != this->get_end_iter(); ++i) {
			// if names are not equal
			if ((*i)->get_name().compare(opposite_state_name) != 0) {
				// add the opposite state to the vector
				opposite_states->push_back(i);
			}
		}
		// if no states have been added, push back the null state (error)
		if (opposite_states->size() == 0) {
			opposite_states->push_back(states->end());
		}
		// return a pointer to a vector of opposite states
		return opposite_states;
	}

	// set a new initial state
	void set_initial_state(string state_name) {
		// set initial
		StateIterator new_initial = this->get_state(state_name);
		(*new_initial)->set_is_initial(true);
		StateIteratorList new_non_initial = this->get_states_not(state_name);

		// if the first element isn't invalid
		if ((*new_non_initial)[0] != states->end()) {
			// set all others to false as initial
			for (vector<StateIterator>::iterator i = new_non_initial->begin();
					i != new_non_initial->end(); ++i) {
				(*(*i))->set_is_initial(false);
			}
		}
		// clear it
		new_non_initial->clear();

		// move the run iterator to the new starting position
		this->run_ptr = this->get_state(state_name);
	}

	// sets the final state of this machine
	void set_final_state(string state_name, bool is_final) {
		StateIterator new_final = this->get_state(state_name);
		// if the state selected is valid, add a new finals state
		if (new_final != states->end())
			(*new_final)->set_is_final(is_final);
	}

	// iterate through states to find a final state
	StateIteratorList get_final_states() {
		StateIteratorList final_states = StateIteratorList(
				new vector<StateIterator>);
		for (StateIterator i = this->get_begin_iter();
				i != this->get_end_iter(); ++i) {
			if ((*i)->get_is_final() == true)
				final_states->push_back(i);
		}
		return final_states;
	}

	// iterate through states to find an initial state
	StateIterator get_initial_state() {
		for (StateIterator i = this->get_begin_iter();
				i != this->get_end_iter(); ++i) {
			if ((*i)->get_is_initial() == true)
				return i;
		}
		return states->end();
	}

	// iterate through states to find the dead state (there should only be one)
	// however, the hackyness of this issue needs to be resolved.
	StateIterator get_dead_state() {
		if (this->dead_states->size() > 0)
			return this->dead_states->begin();
		else
			return states->end();
	}

	// add a transition from one state to another
	void add_transition(string first_state, char through, string second_state) {
		// get iterators pointing to both new states
		StateIterator first_state_iter = this->get_state(first_state);
		StateIterator second_state_iter = this->get_state(second_state);
		if (*first_state_iter != nullptr && *second_state_iter != nullptr) {
			// check to see if there is already a transition through that same character
			if (!(*first_state_iter)->transition_exists(through)) {
				(*first_state_iter)->add_transition(through, second_state_iter);
			} else {
				return;
			}
		}
	}

	// remove a transition
	void remove_transition(string first_state, char through) {
		StateIterator first_state_iter = this->get_state(first_state);
		if (*first_state_iter != nullptr) {
			// check to see if the through state is in the map
			if ((*first_state_iter)->transition_exists(through)) {
				(*first_state_iter)->remove_transition(through);
			} else
				return;
		}
	}

	// add all alphabetic characters as transitions between these two states
	void add_alphabet(string first_state, string second_state) {
		// assumes ascii is used!!!!
		StateIterator first_state_iter = this->get_state(first_state);
		StateIterator second_state_iter = this->get_state(second_state);
		if (*first_state_iter != nullptr && *second_state_iter != nullptr) {
			for (int i = 65; i <= 90; i++)
				this->add_transition(first_state, (char) i, second_state);
			for (int i = 97; i <= 122; i++)
				this->add_transition(first_state, (char) i, second_state);
		}
	}

	// add all digits as transitions between two states
	void add_digits(string first_state, string second_state) {
		// assumes ascii
		StateIterator first_state_iter = this->get_state(first_state);
		StateIterator second_state_iter = this->get_state(second_state);
		if (*first_state_iter != nullptr && *second_state_iter != nullptr) {
			for (int i = 48; i <= 57; i++)
				this->add_transition(first_state, (char) i, second_state);
		}
	}

	// give a detailed description of this FA
	void print() {
		cout << "Name: " << this->name << endl;
		cout << "-" << endl;
		cout << "States:" << endl;
		for (StateIterator i = this->get_begin_iter();
				i != this->get_end_iter(); ++i) {
			cout << (*i)->get_name();
			if ((*i)->get_is_initial())
				cout << ": Initial ";
			if ((*i)->get_is_final())
				cout << ": Final ";
			cout << endl;
		}
		cout << "-" << endl;
		cout << "Transitions: " << endl;
		for (StateIterator i = this->get_begin_iter();
				i != this->get_end_iter(); ++i) {
			for (map<char, StateIterator>::iterator t =
					(*i)->get_transitions()->begin();
					t != (*i)->get_transitions()->end(); ++t) {
				cout << (*i)->get_name() << " -> " << (*(t->second))->get_name()
						<< ": '" << t->first << "'" << endl;
			}
		}
		cout << "-" << endl;
	}

	// determines if this FA has accepted
	bool accepted() {
		// invalid state of the iterator
		if (this->run_ptr == this->states->end())
			return false;
		else {
			//else we need to determine if the iterator is on a final state
			if ((*this->run_ptr)->get_is_final() == true)
				return true;
			else
				return false;
		}
	}

	// runs a string on this FA, provided it is not empty
	void run(string input_str) {
		if (input_str.compare("") != 0)
			// go through each char in the string and step
			for (string::iterator i = input_str.begin(); i != input_str.end();
					++i)
				this->step(*i);
	}

	// resets this FA to its initial state
	void reset() {
		// reset the run iterator to the initial state
		this->run_ptr = this->get_initial_state();
	}

	// steps through this FA one character at a time
	void step(char input_char) {
		// get transitions for the current state
		if ((*this->run_ptr)->transition_exists(input_char)) {
			this->run_ptr = (*this->run_ptr)->get_transition(input_char);
		} else {
			// dead state
			if (this->dead_state_exists == true) {
				this->run_ptr = this->get_dead_state();
			}
		}
	}

    // returns all variants of a character (upper and lowercase)
    pair<char, char> all_char_variants(char c) {
        pair<char, char> char_pair;
        if (c == toupper(c)) {
            char_pair.first = c;
        } else {
            char_pair.first = toupper(c);
        }
        if (c == tolower(c)) {
            char_pair.second = c;
        } else {
            char_pair.second = tolower(c);
        }
        return char_pair;
    }
    
	// builds an FA in this container from a keyword
	void build_keyword(string keyword) {
		// build single stage dfas for keywords
		// only run on a newly formed automata
		int state_counter = 0;
		string prev_state;
		string next_state;
		for (unsigned int i = 0; i < keyword.size(); i++) {
			if (i == 0)
				this->add_state(to_string(i), true, false);
			else
				this->add_state(to_string(i), false, false);
			state_counter++;
		}
		// add final state
		this->add_state(to_string(state_counter), false, true);

		// add transitions
		for (unsigned int i = 0; i < keyword.size(); i++) {
			pair<char, char> char_pair = all_char_variants(
					keyword[i]);
			this->add_transition(to_string(i), char_pair.first,
					to_string(i + 1));
			this->add_transition(to_string(i), char_pair.second,
					to_string(i + 1));
		}
	}
};

#endif
