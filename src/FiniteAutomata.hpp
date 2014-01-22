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
#define RUN_DEBUG 0

class FiniteAutomataContainer;
class FiniteAutomataState;

class FiniteAutomataState {
private:
	bool is_final;
	bool is_initial;
	string name;
	shared_ptr<map<char, vector<shared_ptr<FiniteAutomataState>>::iterator>> to_state;
public:
	FiniteAutomataState(bool is_initial,
                        bool is_final, string name) :
	is_final(is_final), is_initial(is_initial), name(name) {
		this->to_state =
		shared_ptr<map<char, vector<shared_ptr<FiniteAutomataState>>::iterator>>
		(new map<char, vector<shared_ptr<FiniteAutomataState>>::iterator>);
	}
	virtual ~FiniteAutomataState() {
		to_state->clear();
	}
	bool get_is_initial() {
		return this->is_initial;
	}
	bool get_is_final() {
		return this->is_final;
	}
	string get_name() {
		return this->name;
	}
	void set_name(string name) {
		this->name = name;
	}
	void set_is_initial(bool is_initial) {
		this->is_initial = is_initial;
	}
	void set_is_final(bool is_final) {
		this->is_final = is_final;
	}
	void add_transition(char on,
                        vector<shared_ptr<FiniteAutomataState>>::iterator new_state) {
		this->to_state->insert(
                               pair<char, vector<shared_ptr<FiniteAutomataState>>::iterator>(on,
                                                                                             new_state));
	}
	void remove_transition(char on) {
		this->to_state->erase(on);
	}
	void change_transition(char on,
                           vector<shared_ptr<FiniteAutomataState>>::iterator new_state) {
		this->to_state->erase(on);
		this->to_state->insert(
                               pair<char, vector<shared_ptr<FiniteAutomataState>>::iterator>(on,
                                                                                             new_state));
	}
	shared_ptr<map<char, vector<shared_ptr<FiniteAutomataState>>::iterator>> get_transitions() {
		return this->to_state;
	}
	vector<shared_ptr<FiniteAutomataState>>::iterator get_transition(char input_char) {
		return (*to_state)[input_char];
	}
	bool transition_exists(char through) {
		if (this->to_state->find(through) != this->to_state->end())
            return true;
		else
            return false;
	}
	inline bool operator==(const FiniteAutomataState& rhs) {
		// object comparison is by name alone, not by pointed type
		if (this->name.compare(rhs.name) == 0)
            return true;
		else
            return false;
	}
	inline bool operator==(const FiniteAutomataState* rhs) {
		if (this->name.compare(rhs->name) == 0)
            return true;
		else
            return false;
	}
	inline bool operator==(const shared_ptr<FiniteAutomataState> rhs) {
		if (this->name.compare(rhs->name) == 0)
            return true;
		else
            return false;
	}
};

// pre decls
class FiniteAutomataContainer {
private:
	shared_ptr<vector<shared_ptr<FiniteAutomataState>>> states;
	vector<shared_ptr<FiniteAutomataState>>::iterator run_iter;
	bool dead_state;
	string name;
	// private methods
	vector<shared_ptr<FiniteAutomataState>>::iterator get_begin_iter() {
		return this->states->begin();
	}
	vector<shared_ptr<FiniteAutomataState>>::iterator get_end_iter() {
		return this->states->end();
	}
public:
	FiniteAutomataContainer(string name) : dead_state(true), name(name) {
		this->states = shared_ptr<vector<shared_ptr<FiniteAutomataState>>>(new vector<shared_ptr<FiniteAutomataState>>);
		this->states->push_back(
                                shared_ptr<FiniteAutomataState>(new FiniteAutomataState(false, false, "DEAD")));
		this->run_iter = this->states->end();
	}
	FiniteAutomataContainer(string name, bool dead_states_enabled) :
	dead_state(dead_states_enabled), name(name) {
		this->states = shared_ptr<vector<shared_ptr<FiniteAutomataState>>>(new vector<shared_ptr<FiniteAutomataState>>);
		if (dead_state) {
			this->states->push_back(
                                    shared_ptr<FiniteAutomataState>(new FiniteAutomataState(false, false, "DEAD")));
		};
		this->run_iter = this->states->end();
	}
	virtual ~FiniteAutomataContainer() {
		this->states->clear();
	}
	void add_state(string state_name, bool is_initial, bool is_final) {
		try {
			vector<shared_ptr<FiniteAutomataState>>::iterator old_state = this->get_state(
                                                                                          state_name);
			if (old_state != states->end())
                throw string("COPY STATE");
			else {
				this->states->push_back(
                                        shared_ptr<FiniteAutomataState>(new FiniteAutomataState(is_initial, is_final,
                                                                                                state_name)));
				vector<shared_ptr<FiniteAutomataState>>::iterator new_state_iter =
				this->get_state(state_name);
				// set new initial and final states if this node has
				// been constructed as initial/final
				if ((*new_state_iter)->get_is_initial() == true) {
					this->set_initial_state((*new_state_iter)->get_name());
				}
				if ((*new_state_iter)->get_is_final() == true) {
					this->set_final_state((*new_state_iter)->get_name());
				}
			}
			this->reset();
		} catch (string& exception) {
			return;
		}
	}
	void remove_state(string state_name) {
		try {
			vector<shared_ptr<FiniteAutomataState>>::iterator rem_target = states->end();
			// get the exact item for removal
			for (vector<shared_ptr<FiniteAutomataState>>::iterator i =
                 this->get_begin_iter(); i != this->get_end_iter(); ++i)
                // item with the same state name is the object
                if ((*i)->get_name().compare(state_name) == 0) {
                    rem_target = i;
                    break;
                }
			if (rem_target == states->end())
                throw string("NO SUCH STATE");
			// remove all transitions
			for (vector<shared_ptr<FiniteAutomataState>>::iterator i =
                 this->get_begin_iter(); i != this->get_end_iter(); ++i) {
				for (map<char, vector<shared_ptr<FiniteAutomataState>>::iterator>::iterator t =
                     (*i)->get_transitions()->begin();
                     t != (*i)->get_transitions()->end(); ++t) {
					if ((*i) == *(t->second)) {
						(*i)->get_transitions()->erase(t);
					}
				}
			}
			// finally, erase the state
			this->states->erase(rem_target);
		} catch (string &exception) {
			return;
		}
	}
	vector<shared_ptr<FiniteAutomataState>>::iterator get_state(string state_name) {
		for (vector<shared_ptr<FiniteAutomataState>>::iterator i = this->get_begin_iter();
             i != this->get_end_iter(); ++i) {
			if ((*i)->get_name().compare(state_name) == 0) {
				return i;
			}
		}
		return states->end();
	}
	shared_ptr<vector<vector<shared_ptr<FiniteAutomataState>>::iterator>> get_states_not(
                                                                                         string opposite_state_name) {
		shared_ptr<vector<vector<shared_ptr<FiniteAutomataState>>::iterator>> opposite_states =
		shared_ptr<vector<vector<shared_ptr<FiniteAutomataState>>::iterator>>
		(new vector<vector<shared_ptr<FiniteAutomataState>>::iterator>);
		for (vector<shared_ptr<FiniteAutomataState>>::iterator i = this->get_begin_iter();
             i != this->get_end_iter(); ++i) {
			if ((*i)->get_name().compare(opposite_state_name) != 0) {
				opposite_states->push_back(i);
			}
		}
		if (opposite_states->size() == 0) {
			opposite_states->push_back(states->end());
		}
		return opposite_states;
	}
	void set_initial_state(string state_name) {
		// set initial
		vector<shared_ptr<FiniteAutomataState>>::iterator new_initial = this->get_state(
                                                                                        state_name);
		(*new_initial)->set_is_initial(true);
		shared_ptr<vector<vector<shared_ptr<FiniteAutomataState>>::iterator>> new_non_initial =
		this->get_states_not(state_name);

		// if the first element isn't invalid
		if ((*new_non_initial)[0] != states->end()) {
			// set all others to false as initial
			for (vector<vector<shared_ptr<FiniteAutomataState>>::iterator>::iterator i =
                 new_non_initial->begin(); i != new_non_initial->end();
                 ++i) {
				(*(*i))->set_is_initial(false);
			}
		}
		// free memory?
		new_non_initial->clear();

		// move the run iterator to the new starting position
		this->run_iter = this->get_state(state_name);
	}
	void set_final_state(string state_name) {
		vector<shared_ptr<FiniteAutomataState>>::iterator new_final = this->get_state(
                                                                                      state_name);
		(*new_final)->set_is_final(true);
		shared_ptr<vector<vector<shared_ptr<FiniteAutomataState>>::iterator>> new_non_final =
		this->get_states_not(state_name);
		for (vector<vector<shared_ptr<FiniteAutomataState>>::iterator>::iterator i =
             new_non_final->begin(); i != new_non_final->end(); ++i) {
			(*(*i))->set_is_final(false);
		}
		new_non_final->clear();
	}
	vector<shared_ptr<FiniteAutomataState>>::iterator get_final_state() {
		for (vector<shared_ptr<FiniteAutomataState>>::iterator i = this->get_begin_iter();
             i != this->get_end_iter(); ++i) {
			if ((*i)->get_is_final() == true)
                return i;
		}
		return states->end();
	}
	vector<shared_ptr<FiniteAutomataState>>::iterator get_initial_state() {
		for (vector<shared_ptr<FiniteAutomataState>>::iterator i = this->get_begin_iter();
             i != this->get_end_iter(); ++i) {
			if ((*i)->get_is_initial() == true)
                return i;
		}
		return states->end();
	}
	vector<shared_ptr<FiniteAutomataState>>::iterator get_dead_state() {
		for (vector<shared_ptr<FiniteAutomataState>>::iterator i = this->get_begin_iter();
             i != this->get_end_iter(); ++i) {
			if ((*i)->get_name().compare("DEAD") == 0)
                return i;
		}
		return states->end();
	}
	void add_transition(string first_state, char through, string second_state) {
		vector<shared_ptr<FiniteAutomataState>>::iterator first_state_iter =
		this->get_state(first_state);
		vector<shared_ptr<FiniteAutomataState>>::iterator second_state_iter =
		this->get_state(second_state);
		if (*first_state_iter != nullptr && *second_state_iter != nullptr) {
			// check to see if there is already a transition through that same character
			if (!(*first_state_iter)->transition_exists(through)) {
				(*first_state_iter)->add_transition(through, second_state_iter);
			} else {
				return;
			}
		}
	}
	void remove_transition(string first_state, char through) {
		vector<shared_ptr<FiniteAutomataState>>::iterator first_state_iter =
		this->get_state(first_state);
		if (*first_state_iter != nullptr) {
			// check to see if the through state is in the map
			if ((*first_state_iter)->transition_exists(through)) {
				(*first_state_iter)->remove_transition(through);
			} else
                return;
		}
	}
	void add_alphabet(string first_state, string second_state) {
		// assumes ascii
		vector<shared_ptr<FiniteAutomataState>>::iterator first_state_iter =
		this->get_state(first_state);
		vector<shared_ptr<FiniteAutomataState>>::iterator second_state_iter =
		this->get_state(second_state);
		if (*first_state_iter != nullptr && *second_state_iter != nullptr) {
			for (int i = 65; i <= 90; i++)
                this->add_transition(first_state, (char) i, second_state);
			for (int i = 97; i <= 122; i++)
                this->add_transition(first_state, (char) i, second_state);
		}
	}
	void add_digits(string first_state, string second_state) {
		// assumes ascii
		vector<shared_ptr<FiniteAutomataState>>::iterator first_state_iter =
		this->get_state(first_state);
		vector<shared_ptr<FiniteAutomataState>>::iterator second_state_iter =
		this->get_state(second_state);
		if (*first_state_iter != nullptr && *second_state_iter != nullptr) {
			for (int i = 48; i <= 57; i++)
                this->add_transition(first_state, (char) i, second_state);
		}
	}
	void print() {
		cout << "Name: " << this->name << endl;
		cout << "-" << endl;
		cout << "States:" << endl;
		for (vector<shared_ptr<FiniteAutomataState>>::iterator i = this->get_begin_iter();
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
		for (vector<shared_ptr<FiniteAutomataState>>::iterator i = this->get_begin_iter();
             i != this->get_end_iter(); ++i) {
			for (map<char, vector<shared_ptr<FiniteAutomataState>>::iterator>::iterator t =
                 (*i)->get_transitions()->begin();
                 t != (*i)->get_transitions()->end(); ++t) {
				cout << (*i)->get_name() << " -> " << (*(t->second))->get_name()
				<< ": '" << t->first << "'" << endl;
			}
		}
		cout << "-" << endl;
	}
	bool accepted() {
		// invalid state of the iterator
		if (this->run_iter == this->states->end())
            return false;
		else {
			//else we need to determine if the iterator is on a final state
			if ((*this->run_iter)->get_is_final() == true)
                return true;
			else
                return false;
		}
	}
	void run(string input_str) {
		if (input_str.compare("") != 0)
            // go through each char in the string and step
            for (string::iterator i = input_str.begin(); i != input_str.end();
                 ++i)
                this->step(*i);
	}
	void reset() {
		// reset the run iterator to the initial state
		this->run_iter = this->get_initial_state();
	}
	void step(char input_char) {
		// get transitions for the current state
		if ((*this->run_iter)->transition_exists(input_char)) {
			this->run_iter = (*this->run_iter)->get_transition(input_char);
			if (RUN_DEBUG)
                cout << "In: " << (*this->run_iter)->get_name() << endl;
		} else {
			// dead state
			if (RUN_DEBUG) {
				cout << "Unable to transition..." << endl;
			}
			if (this->dead_state == true) {
				this->run_iter = this->get_dead_state();
			}
		}
	}
	void build_keyword(string kword) {
		// build single stage fsas for keywords
		// only run on a newly formed automata
        int state_counter = 0;
        string prev_state;
        string next_state;
		for (int i = 0; i < kword.size(); i++) {
            if (i == 0)
                this->add_state(to_string(i), true, false);
            else
                this->add_state(to_string(i), false, false);
            state_counter++;
        }
        // add final state
        this->add_state(to_string(state_counter), false, true);

        // add transitions
        for (int i = 0; i < kword.size(); i++) {
            pair<char, char> char_pair = all_char_variants(kword[i]);
            this->add_transition(to_string(i), char_pair.first, to_string(i + 1));
            this->add_transition(to_string(i), char_pair.second, to_string(i + 1));
        }
	}
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
};

#endif
