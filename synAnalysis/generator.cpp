#include <cstddef>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using ll = int64_t;

class NDFA {
  private:
	// initial productions and symbols
	std::unordered_set<std::string> terminals;
	std::unordered_set<std::string> non_terminals;
	std::unordered_set<std::string> sync;
	std::multimap<std::string, std::vector<std::string>> productions;
	std::string starting_production;

	int start;
	int state_count = 0;

	// all accepted states of NDFA
	std::vector<ll> finals;
	std::map<int, std::vector<std::string>> states;

	// pairs of left-sides of production with index of state
	std::multimap<std::string, int> left_states;
	std::map<int, std::string> states_left;
	// right context saved to position of production index
	std::map<int, std::set<std::string>> right_context;
	// map of transitions
	std::multimap<std::pair<int, std::string>, int> transitions;

	// hash set of empty non terminal symbols
	std::unordered_set<std::string> empty_symbols;
	// hash map of immediate starts of non terminal symbols
	std::unordered_map<std::string, std::vector<std::string>> immediate_starts;
	// hash map of all posible starts of non terminal symbols
	std::unordered_map<std::string, std::vector<std::string>> symbol_starts;

	int new_state() { return ++this->state_count - 1; }

	bool is_starter(int check) {
		std::vector<std::string> state = states[check];
		return state.at(0) == ".";
	}

	void find_empty_symbols() {
		std::queue<std::string> q;
		for (auto production : productions) {
			for (auto right_element : production.second) {
				if (right_element == "$") {
					this->empty_symbols.insert(production.first);
				}
			}
		}
		size_t previous_size = 0;
		size_t new_size = this->empty_symbols.size();
		while (previous_size != new_size) {
			for (auto production : productions) {
				size_t right_side_size = production.second.size();
				for (auto right_element : production.second) {
					if (this->empty_symbols.count(right_element)) {
						--right_side_size;
					}
				}
				if (right_side_size == 0) {
					this->empty_symbols.insert(production.first);
				}
			}
			previous_size = new_size;
			new_size = this->empty_symbols.size();
		}
	}

	void find_immediate_starts() {
		find_empty_symbols();
		for (auto production : productions) {
			std::string left = production.first;
			std::vector<std::string> right = production.second;
			if (right.size() == 1 && right.at(0) == "$") {
				continue;
			}
			this->immediate_starts[left].push_back(right.at(0));
			for (size_t i = 0;
				 i < right.size() - 1 && empty_symbols.count(right.at(i));
				 ++i) {
				this->immediate_starts[left].push_back(right.at(i + 1));
			}
		}
	}

	void find_starts_symbols() {
		find_immediate_starts();
		for (auto table : this->immediate_starts) {
			std::string left = table.first;
			this->symbol_starts[left].push_back(left);
			std::vector<std::string> right = table.second;
			for (size_t i = 0; i < right.size(); ++i) {
				std::string current_right = right.at(i);
				this->symbol_starts[left].push_back(current_right);
				if (this->immediate_starts.count(current_right)) {
					std::vector<std::string> additions =
						this->immediate_starts[current_right];
					std::queue<std::string> q;
					std::set<std::string> visited;
					for (size_t j = 0; j < additions.size(); ++j) {
						q.push(additions.at(j));
					}
					while (!q.empty()) {
						std::string current = q.front();
						q.pop();
						if (visited.count(current)) {
							continue;
						}
						visited.insert(current);
						this->symbol_starts[left].push_back(current);
						std::vector<std::string> news =
							this->immediate_starts[current];
						for (auto x : news) {
							q.push(x);
						}
					}
				}
			}
		}
	}

	void find_starts() { find_starts_symbols(); }

	void get_states() {
		for (auto production : productions) {
			std::string left = production.first;
			std::vector<std::string> right = production.second;
			if (right.size() == 1 && right.at(0) == "$") {
				int current_state = new_state();
				states[current_state] = std::vector<std::string>(1, ".");
				left_states.emplace(make_pair(left, current_state));
				states_left.emplace(make_pair(current_state, left));
				continue;
			}
			std::vector<std::string> vec(right.size() + 1, "");
			for (int i = 0; i <= right.size(); ++i) {
				vec.at(i) = ".";
				int j = 0, k = 0;
				while (k < right.size()) {
					if (i == j) {
						++j;
						continue;
					} else {
						vec.at(j) = right.at(k);
						++j;
						++k;
					}
				}
				int current_state = new_state();
				states[current_state] = vec;
				left_states.emplace(make_pair(left, current_state));
				states_left.emplace(make_pair(current_state, left));
			}
		}
		return;
	}
	void get_transitions() {
		int atm_state = start;
		auto initials = left_states.equal_range(this->starting_production);
		std::queue<int> q;
		while (initials.first != initials.second) {
			if (is_starter(initials.first->second)) {
				this->transitions.emplace(
					std::make_pair(std::make_pair(atm_state, "EPSILON"),
								   initials.first->second));
			}
			q.push(initials.first->second);
			initials.first = std::next(initials.first);
		}
		std::set<int> visited;
		while (!q.empty()) {
			atm_state = q.front();
			std::vector<std::string> state = states[atm_state];
			q.pop();
			if (visited.count(atm_state)) {
				continue;
			}
			visited.insert(atm_state);
			size_t i = 0;
			for (; i < state.size() && state.at(i) != "."; ++i)
				;
			std::string current_sign = state[i];
			if (i + 1 < state.size()) {
				auto new_states = left_states.equal_range(state.at(i + 1));
				while (new_states.first != new_states.second) {
					if (is_starter(new_states.first->second)) {
						this->transitions.emplace(
							std::make_pair(std::make_pair(atm_state, "EPSILON"),
										   new_states.first->second));
						q.push(new_states.first->second);
					}
					new_states.first = std::next(new_states.first);
				}

				std::string left_side = states_left[atm_state];
				auto found = left_states.equal_range(left_side);
				while (found.first != found.second) {
					if (found.first->second == atm_state) {
						found.first = std::next(found.first);
						continue;
					}
					std::vector<std::string> reading_states =
						states[found.first->second];
					size_t j = 0;
					for (; j < reading_states.size() &&
						   reading_states.at(j) != ".";
						 ++j)
						;
					if (j > 0 && reading_states.at(j - 1) == state.at(i + 1)) {
						this->transitions.emplace(std::make_pair(
							std::make_pair(atm_state, reading_states.at(j - 1)),
							found.first->second));
						q.push(found.first->second);
					}
					// std::vector<std::string> current_right =
					// 	found.first->second
					found.first = std::next(found.first);
				}
			}
		}
		return;
	}
	void get_contexts() {
		/* provjeriti samo episilon prijelaze
		ako je znak A->.BA ==> desni kontekst = ZAPOČINJE(a)
		+ ako A može otići u prazan niz onda moramo i očuvati dosadašnji
		 */
		find_starts();
		// int atm_state = start;
		// std::queue<int> q;
		// auto initials =
		// 	transitions.equal_range(std::make_pair(atm_state, "EPSILON"));
		// while (initials.first != initials.second) {
		// 	q.push(initials.first->second);
		// 	initials.first = std::next(initials.first);
		// }
		// std::set<int> visited;
		// while (!q.empty()) {
		// 	atm_state = q.front();
		// 	std::vector<std::string> state = states[atm_state];
		// 	q.pop();
		// 	if (visited.count(atm_state)) {
		// 		continue;
		// 	}
		// 	visited.insert(atm_state);
		// }

		// return;
	}
	void print_states() {
		for (auto state : this->states) {
			std::cout << state.first << ": ";
			for (auto right_side : state.second) {
				std::cout << right_side << ' ';
			}
			std::cout << std::endl;
		}
		return;
	}
	void print_states_left() {
		for (auto state : this->states_left) {
			std::cout << state.first << ' ' << state.second << std::endl;
		}
	}
	void print_transitions() {
		for (auto transition : this->transitions) {
			std::cout << transition.first.first << " "
					  << transition.first.second << " " << transition.second
					  << std::endl;
		}
	}
	void print_empty_symbols() {
		for (auto symbol : this->empty_symbols) {
			std::cout << symbol << std::endl;
		}
	}
	void print_immediate_starts() {
		for (auto table : this->immediate_starts) {
			std::cout << table.first << ": ";
			for (auto symb : table.second) {
				std::cout << symb << ' ';
			}

			std::cout << std::endl;
		}
	}
	void print_symbol_starts() {
		for (auto table : this->symbol_starts) {
			std::cout << table.first << ": ";
			for (auto symb : table.second) {
				std::cout << symb << ' ';
			}

			std::cout << std::endl;
		}
	}

  public:
	NDFA(std::multimap<std::string, std::vector<std::string>> productions,
		 std::string starting_production,
		 std::unordered_set<std::string> terminals,
		 std::unordered_set<std::string> non_terminals,
		 std::unordered_set<std::string> sync) {
		this->terminals = terminals;
		this->non_terminals = non_terminals;
		this->sync = sync;
		this->productions = productions;
		this->starting_production = starting_production;
		this->start = new_state();
		get_states();
		// print_states();
		// print_states_left();
		get_transitions();
		// print_transitions();
		get_contexts();
	}
};
int main(void) {
	std::string line;
	std::unordered_set<std::string> terminals;
	std::unordered_set<std::string> non_terminals;
	std::unordered_set<std::string> sync;
	std::multimap<std::string, std::vector<std::string>> productions;
	std::string current_sign;
	std::string starting_production;
	while (std::getline(std::cin, line)) {
		// Parse non-terminal symbols
		if (line.substr(0, 2) == "%V") {
			line = line.substr(line.find(' ') + 1);
			starting_production = line.substr(0, line.find(' '));
			// std::cout << starting_production << std::endl;
			while (!line.empty()) {
				if (line.find(' ') == std::string::npos) {
					non_terminals.insert(line);
					line.clear();
				} else {
					non_terminals.insert(line.substr(0, line.find(' ')));

					line = line.substr(line.find(' ') + 1);
				}
			}
		}

		// Parse terminal symbols
		else if (line.substr(0, 2) == "%T") {
			line = line.substr(line.find(' ') + 1);

			for (int i = 0; i < line.size(); ++i) {
				while (!line.empty()) {
					if (line.find(' ') == std::string::npos) {
						terminals.insert(line);
						line.clear();
					} else {
						terminals.insert(line.substr(0, line.find(' ')));

						line = line.substr(line.find(' ') + 1);
					}
				}
			}
		}
		// Parse syncing symbols
		else if (line.substr(0, 4) == "%Syn") {
			line = line.substr(line.find(' ') + 1);
			for (int i = 0; i < line.size(); ++i) {
				while (!line.empty()) {
					if (line.find(' ') == std::string::npos) {
						sync.insert(line);
						line.clear();
					} else {
						sync.insert(line.substr(0, line.find(' ')));

						line = line.substr(line.find(' ') + 1);
					}
				}
			}
		}
		// Parse productions
		else if (line.at(0) != ' ') {
			current_sign = line;
		} else {
			line = line.substr(1);
			std::vector<std::string> right_production;
			while (!line.empty()) {
				if (line.find(' ') == std::string::npos) {
					right_production.push_back(line);
					line.clear();
				} else {
					right_production.push_back(line.substr(0, line.find(' ')));
					line = line.substr(line.find(' ') + 1);
				}
			}
			productions.emplace(current_sign, right_production);
		}
	}
	// for (auto x : non_terminals) {
	// 	std::cout << x << ' ';
	// }
	// std::cout << std::endl;
	// for (auto x : terminals) {
	// 	std::cout << x << ' ';
	// }
	// std::cout << std::endl;
	// for (auto x : sync) {
	// 	std::cout << x << ' ';
	// }
	// std::cout << std::endl;
	// for (auto x : productions) {
	// 	std::cout << x.first << " -> ";
	// 	for (auto y : x.second) {
	// 		std::cout << y << ' ';
	// 	}
	// 	std::cout << std::endl;
	// }

	NDFA enka(productions, starting_production, non_terminals, terminals, sync);

	return 0;
}
