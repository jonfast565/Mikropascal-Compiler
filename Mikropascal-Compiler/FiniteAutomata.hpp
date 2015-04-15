#ifndef automata_h
#define automata_h

#include "Standard.hpp"

#define AUTOMATA_DEBUG 0

// class predefs
class FiniteMachineContainer;
class FiniteMachineState;

// type aliases
using StatePtr = shared_ptr<FiniteMachineState>;
using StateList = vector<StatePtr>;
using StateListIterator = StateList::iterator;
using StateListPtr = shared_ptr<StateList>;
using TransitionMap = map<char, StatePtr>;
using TransitionMapPtr = shared_ptr<TransitionMap>;
using TransitionMapIterator = TransitionMap::iterator;
using FSMachinePtr = shared_ptr<FiniteMachineContainer>;
using FSMachineList = vector<FSMachinePtr>;
using FSMachineListPtr = shared_ptr<FSMachineList>;

// const string reference values
static const char* dead_state_name = "DEAD";

// class that creates a state in an FA
class FiniteMachineState : public enable_shared_from_this<FiniteMachineState> {
private:
	bool is_final_state;
	bool is_initial_state;
	string machine_name;
	TransitionMapPtr transition_map;
public:
	// ctor
	FiniteMachineState(bool is_initial_state, bool is_final_state, string machine_name) :
			is_final_state(is_final_state), is_initial_state(is_initial_state), machine_name(machine_name) {
		this->transition_map = TransitionMapPtr(new TransitionMap);
	}
	// dtor
	virtual ~FiniteMachineState() {
		transition_map->clear();
	}
	// get initial state
	bool get_is_initial() {
		return this->is_initial_state;
	}
	// get final state
	bool get_is_final() {
		return this->is_final_state;
	}
	// get FA name
	string get_name() {
		return this->machine_name;
	}
	// set FA name
	void set_name(string name) {
		this->machine_name = name;
	}
	// set initial state
	void set_is_initial(bool is_initial) {
		this->is_initial_state = is_initial;
	}
	// set final state
	void set_is_final(bool is_final) {
		this->is_final_state = is_final;
	}
	// add a transition to this state
	void add_transition(char on, StatePtr new_state) {
		this->transition_map->insert(pair<char, StatePtr>(on, new_state));
	}
	// remove a transition from this state
	void remove_transition(char on) {
		this->transition_map->erase(on);
	}
	// modify a transition from this state to another
	void change_transition(char on, StatePtr new_state) {
		this->transition_map->erase(on);
		this->transition_map->insert(pair<char, StatePtr>(on, new_state));
	}
	// return the map of character to transitions (iterators to new states)
	TransitionMapPtr get_transitions() {
		return TransitionMapPtr(this->transition_map);
	}
	// return a particular transition to another state (iterator)
	StatePtr get_transition(char input_char) {
		return StatePtr((*this->transition_map)[input_char]);
	}
	// determine if a particular transition exists
	bool transition_exists(char through) {
		// ensure that a state we find is not invalid
		// by comparing against the end() iterator
		return (this->transition_map->find(through) != this->transition_map->end()) ? true : false;
	}
	// state comparison by pointed to type via shared_ptr<T>
	// names are compared for equality
	inline bool operator==(const StatePtr rhs) {
		return (this->machine_name.compare(rhs->machine_name) == 0) ? true : false;
	}
};

// class that creates a container in an FA
class FiniteMachineContainer : public enable_shared_from_this<FiniteMachineContainer> {
private:
	// consists of a vector of states, and an iterator over that
	// vector for running the machine
	StateListPtr state_list;
	StatePtr run_pointer;
	StatePtr dead_state;
	
	// info variables like a machine name
	// and if a dead state exists
	bool dead_state_exists;
	string machine_name;
	unsigned int machine_priority;

	// get an iterator for the first element of the state vector
	StateListIterator get_begin_iter() {
		return this->state_list->begin();
	}

	// get an iterator for the last element of the state vector
	StateListIterator get_end_iter() {
		return this->state_list->end();
	}

public:
	// construct an entire finite automata (ctor)
	FiniteMachineContainer(string name) :
			dead_state_exists(true), machine_name(name) {
		
		// create a list of states
		this->state_list = StateListPtr(new StateList);
		
		// automatically create a dead state (hack), that the iterator goes to if a match is not found
		this->dead_state = StatePtr(new FiniteMachineState(false, false, "DEAD"));
		
		// move the run pointer to the end state (as for now it is invalid)
		// this is the equivalent of the pointer in an array of char
		this->run_pointer = nullptr;
		this->machine_priority = 0;
	}

	// construct an FA with the option of setting if a dead state exists
	// performs the same function as the ctor above
	FiniteMachineContainer(string name, bool dead_states_enabled) :
			dead_state_exists(dead_states_enabled), machine_name(name) {
		this->state_list = StateListPtr(new StateList);
		this->dead_state = StatePtr(new FiniteMachineState(false, false, "DEAD"));
		this->run_pointer = StatePtr(this->dead_state);
		this->machine_priority = 0;
	}

	// destructor clears all loose states
	virtual ~FiniteMachineContainer() {
		this->state_list->clear();
	}

	// get this automata's name
	string get_name() {
		return this->machine_name;
	}

	// set this automata's name
	void set_name(string name) {
		this->machine_name = name;
	}
	
	// FSA priority (for use with a scanner)
	unsigned int get_priority() {
		return this->machine_priority;
	}
	
	// FSA priority (for use with a scanner)
	void set_priority(unsigned int new_priority) {
		this->machine_priority = new_priority;
	}

	// add a state to this automata
	void add_state(string state_name, bool is_initial, bool is_final) {
		try {
			// ensure that the state we're adding is not a copy of a previous one
			StatePtr old_state = this->get_state(state_name);
			
			// error throws a copy state and exits
			if (old_state != nullptr) {
				throw string("DFA contains a state that is a copy of another state");
			} else {
				// otherwise add this as a state
				this->state_list->push_back(StatePtr(new FiniteMachineState(is_initial, is_final, state_name)));

				// get a state iterator to this new state, so we can edit it a little
				StatePtr new_state_iter = this->get_state(state_name);

				// set new initial and final states if this node has
				// been constructed as initial/final
				if (new_state_iter->get_is_initial() == true) {
					this->set_initial_state(new_state_iter->get_name());
				}
				if (new_state_iter->get_is_final() == true) {
					this->set_final_state(new_state_iter->get_name(), true);
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
			StateListIterator rem_target = state_list->end();
			// get the exact item for removal
			for (StateListIterator i = this->get_begin_iter();
					i != this->get_end_iter(); ++i)
				// we need to find an item with the same state name, which is the correct
				if ((*i)->get_name().compare(state_name) == 0) {
					// iterator now set to remove i
					rem_target = i;
					break;
				}
			// if state found is null
			if (rem_target == state_list->end()) {
				throw string("Target state does not exist, cannot remove");
			}

			// otherwise, remove all invalid transitions to the state we took away
			// look through each state's transition map to find the appropriate matches
			for (StateListIterator i = this->get_begin_iter(); i != this->get_end_iter(); ++i) {
				// iterate through the map
				for (TransitionMapIterator t = (*i)->get_transitions()->begin();
					t != (*i)->get_transitions()->end(); ++t) {

					// if the removal iterator rem_target is equivalent to any iterator
					// that points to itself
					if ((*i)->get_name().compare(t->second->get_name()) == 0) {
						// erase the transition that is pointed to by the map iterator
						(*i)->get_transitions()->erase(t);
					}
				}
			}
			// finally, erase the state when all transitions are dealt with
			this->state_list->erase(rem_target);
		} catch (string &exception) {
			// fall through, can't remove a state that's not there
			return;
		}
	}

	// get a state
	StatePtr get_state(string state_name) {
		for (StateListIterator i = this->get_begin_iter();
				i != this->get_end_iter(); ++i) {
			// compare names of states to determine if we can obtain
			if ((*i)->get_name().compare(state_name) == 0) {
				return StatePtr(*i);
			}
		}
		// otherwise, return null state
		return nullptr;
	}

	// return all states that are not equivalent to the one given
	StateListPtr get_states_not(string opposite_state_name) {
		// create a vector to retrieve all other states than the one described by
		// the input string
		StateListPtr opposite_states = StateListPtr(new StateList);
		// iterate through all states
		for (StateListIterator i = this->get_begin_iter();
				i != this->get_end_iter(); ++i) {
			// if names are not equal
			if ((*i)->get_name().compare(opposite_state_name) != 0) {
				// add the opposite state to the vector
				opposite_states->push_back(StatePtr(*i));
			}
		}
		// if no states have been added, push back the null state (error)
		if (opposite_states->size() == 0) {
			return nullptr;
		}
		// return a pointer to a vector of opposite states
		return StateListPtr(opposite_states);
	}

	// set a new initial state
	void set_initial_state(string state_name) {
		// set initial
		StatePtr new_initial = this->get_state(state_name);
		new_initial->set_is_initial(true);
		StateListPtr new_non_initial = this->get_states_not(state_name);

		// if the first element isn't invalid
		if (new_non_initial != nullptr) {
			// set all others to false as initial
			for (StateListIterator i = new_non_initial->begin();
					i != new_non_initial->end(); ++i) {
				(*i)->set_is_initial(false);
			}
		}

		// move the run iterator to the new starting position
		this->run_pointer.swap(StatePtr(new_initial));
	}

	// sets the final state of this machine
	void set_final_state(string state_name, bool is_final) {
		// get the state with the associated name
		StatePtr new_final = this->get_state(state_name);

		// if the state selected is valid, add a new finals state
		if (new_final != nullptr) {
			new_final->set_is_final(is_final);
		}
	}

	// iterate through states to find a final state
	StateListPtr get_final_states() {
		StateListPtr final_states = StateListPtr(new StateList);
		for (StateListIterator i = this->get_begin_iter();
				i != this->get_end_iter(); ++i) {
			if ((*i)->get_is_final() == true)
				final_states->push_back(StatePtr(*i));
		}
		return final_states;
	}

	// iterate through states to find an initial state
	StatePtr get_initial_state() {
		for (StateListIterator i = this->get_begin_iter();
				i != this->get_end_iter(); ++i) {
			if ((*i)->get_is_initial() == true)
				return StatePtr(*i);
		}
		return nullptr;
	}

	// iterate through states to find the dead state (there should only be one)
	// however, the hackyness of this issue needs to be resolved.
	StatePtr get_dead_state() {
		return StatePtr(this->dead_state);
	}

	// add a transition from one state to another
	void add_transition(string first_state, char through, string second_state) {
		// get iterators pointing to both new states
		StatePtr first_state_iter = this->get_state(first_state);
		StatePtr second_state_iter = this->get_state(second_state);
		
		// check to see if both valid
		if (first_state_iter != nullptr && second_state_iter != nullptr) {

			// check to see if there is already a transition through that same character
			if (!first_state_iter->transition_exists(through)) {
				first_state_iter->add_transition(through, second_state_iter);
			}
		}
	}

	// remove a transition
	void remove_transition(string first_state, char through) {
		
		// get state
		StatePtr first_state_iter = this->get_state(first_state);
		
		// if valid
		if (first_state_iter != nullptr) {

			// check to see if the through state is in the map
			if (first_state_iter->transition_exists(through)) {
				first_state_iter->remove_transition(through);
			}
		}
	}

	// add all alphabetic characters as transitions between these two states
	void add_alphabet(string first_state, string second_state) {
		// assumes ascii is used!!!!
		StatePtr first_state_iter = this->get_state(first_state);
		StatePtr second_state_iter = this->get_state(second_state);
		if (first_state_iter != nullptr && second_state_iter != nullptr) {
			for (int i = 65; i <= 90; i++)
				this->add_transition(first_state, (char) i, second_state);
			for (int i = 97; i <= 122; i++)
				this->add_transition(first_state, (char) i, second_state);
		}
	}

	// add all digits as transitions between two states
	void add_digits(string first_state, string second_state) {
		// assumes ascii
		StatePtr first_state_iter = this->get_state(first_state);
		StatePtr second_state_iter = this->get_state(second_state);
		if (first_state_iter != nullptr && second_state_iter != nullptr) {
			for (int i = 48; i <= 57; i++)
				this->add_transition(first_state, (char) i, second_state);
		}
	}
	
	void add_symbols(string first_state, string second_state) {
		// assumes ascii
		StatePtr first_state_iter = this->get_state(first_state);
		StatePtr second_state_iter = this->get_state(second_state);
		if (first_state_iter != nullptr && second_state_iter != nullptr) {
			for (int i = 33; i <= 47; i++)
				this->add_transition(first_state, (char) i, second_state);
			for (int i = 58; i <= 64; i++)
				this->add_transition(first_state, (char) i, second_state);
			for (int i = 91; i <= 96; i++)
				this->add_transition(first_state, (char) i, second_state);
			for (int i = 123; i <= 126; i++)
				this->add_transition(first_state, (char) i, second_state);
		}
	}

	// give a detailed description of this FA
	void print() {
		cout << "Name: " << this->machine_name << endl;
		cout << "-" << endl;
		cout << "States:" << endl;
		for (StateListIterator i = this->get_begin_iter();
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
		for (StateListIterator i = this->get_begin_iter();
				i != this->get_end_iter(); ++i) {
			for (TransitionMapIterator t =
					(*i)->get_transitions()->begin();
					t != (*i)->get_transitions()->end(); ++t) {
				cout << (*i)->get_name() << " -> " << t->second->get_name()
						<< ": '" << t->first << "'" << endl;
			}
		}
		cout << "-" << endl;
	}

	// determines if this FA has accepted
	bool accepting() {
		// invalid state of the iterator
		if (this->run_pointer == nullptr || 
			this->run_pointer == this->dead_state) {
			return false;
		} else {
			// else we need to determine if
			// the iterator is on a final state
			return (this->run_pointer->get_is_final() == true) ? true : false;
		}
	}

	// runs a string on this FA, provided it is not empty
	void run(string input_str) {
		if (input_str.compare("") != 0)
			// go through each char in the string and step
			for (string::iterator i = input_str.begin(); i != input_str.end(); ++i)
				this->step(*i);
	}
	
	// is this FA dead?
	bool dead() {
		return (this->run_pointer == this->get_dead_state()) ? true : false;
	}
	
	// is it not dead?
	bool not_dead() {
		return (this->run_pointer == this->get_dead_state()) ? false : true;
	}

	// resets this FA to its initial state
	void reset() {
		// reset the run iterator to the initial state
		this->run_pointer = this->get_initial_state();
	}

	// steps through this FA one character at a time
	void step(char input_char) {
		// get transitions for the current state
		if (this->run_pointer->transition_exists(input_char)) {
			this->run_pointer = this->run_pointer->get_transition(input_char);
		} else {
			// dead state
			if (this->dead_state_exists == true) {
				this->run_pointer = this->get_dead_state();
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
		
		// create states with zero based names and Q
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

// FA comparison (based on priority only)
inline bool operator ==(FSMachinePtr& a, FSMachinePtr& b) {
	return (a->get_priority() == b->get_priority()) ? true : false;
}

inline bool operator >=(FSMachinePtr& a, FSMachinePtr& b) {
	return (a->get_priority() >= b->get_priority()) ? true : false;
}

inline bool operator <=(FSMachinePtr& a, FSMachinePtr& b) {
	return (a->get_priority() <= b->get_priority()) ? true : false;
}

inline bool operator <(FSMachinePtr& a, FSMachinePtr& b) {
	return (a->get_priority() < b->get_priority()) ? true : false;
}

inline bool operator >(FSMachinePtr& a, FSMachinePtr& b) {
	return (a->get_priority() > b->get_priority()) ? true : false;
}

inline bool operator !=(FSMachinePtr& a, FSMachinePtr& b) {
	return (a->get_priority() != b->get_priority()) ? true : false;
}

#endif
