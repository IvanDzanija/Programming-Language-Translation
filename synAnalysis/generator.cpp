#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <vector>
using ll = int64_t;

class NDFA {
  private:
	// initial productions
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
	std::vector<std::vector<std::string>> right_context;
	// map of transitions
	std::multimap<std::pair<int, std::string>, int> transitions;

	int new_state() { return ++this->state_count - 1; }

	bool is_starter(int check) {
		std::vector<std::string> state = states[check];
		return state.at(0) == ".";
	}

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
		while (!q.empty()) {
			atm_state = q.front();
			std::vector<std::string> state = states[atm_state];
			q.pop();
			for (size_t i = 0; i < state.size(); ++i) {
				std::string current_sign = state[i];
				if (current_sign == "." && i + 1 < state.size()) {
					auto new_states = left_states.equal_range(state[i + 1]);
					while (new_states.first != new_states.second) {
						if (is_starter(new_states.first->second)) {
							this->transitions.emplace(std::make_pair(
								std::make_pair(atm_state, "EPSILON"),
								new_states.first->second));
							// q.push(new_states.first->second);
						}
						std::string left_side = states_left[atm_state];
						auto found = states.equal_range(left_side);
						while (found.first != found.second) {
							found.first->
							// std::vector<std::string> current_right =
							// 	found.first->second
						}

						new_states.first = std::next(new_states.first);
					}
				}
			}
		}

		return;
	}
	void get_contexts() {
		std::queue<std::pair<int, std::string>> q;

		return;
	}
	void print_states() {
		for (auto state : states) {
			std::cout << state.first << ": ";
			for (auto right_side : state.second) {
				std::cout << right_side << ' ';
			}
			std::cout << std::endl;
		}
		return;
	}
	void print_states_left() {
		for (auto state : states_left) {
			std::cout << state.first << ' ' << state.second << std::endl;
		}
	}
	void print_transitions() {
		for (auto transition : transitions) {
			std::cout << transition.first.first << " "
					  << transition.first.second << " " << transition.second
					  << std::endl;
		}
	}

  public:
	NDFA(std::multimap<std::string, std::vector<std::string>> productions,
		 std::string starting_production) {
		this->productions = productions;
		this->starting_production = starting_production;
		this->start = new_state();
		get_states();
		// print_states();
		// print_states_left();
		// get_transitions();
		// print_transitions();
		get_contexts();
	}
};
int main(void) {
	std::string line;
	std::vector<std::string> terminals;
	std::vector<std::string> non_terminals;
	std::vector<std::string> sync;
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
					non_terminals.push_back(line);
					line.clear();
				} else {
					non_terminals.push_back(line.substr(0, line.find(' ')));

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
						terminals.push_back(line);
						line.clear();
					} else {
						terminals.push_back(line.substr(0, line.find(' ')));

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
						sync.push_back(line);
						line.clear();
					} else {
						sync.push_back(line.substr(0, line.find(' ')));

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

	NDFA enka(productions, starting_production);

	return 0;
}
