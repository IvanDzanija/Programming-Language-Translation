#include <cstddef>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using tpl =
	std::tuple<std::string, std::vector<std::string>, std::set<std::string>>;
using ll = int64_t;

class NDFA {
  private:
	// initial productions and symbols
	std::unordered_set<std::string> non_terminals;
	std::unordered_set<std::string> terminals;
	std::unordered_set<std::string> sync;
	std::multimap<std::string, std::vector<std::string>> productions;
	std::string starting_production;

	// LR(1) items
	std::set<std::vector<std::string>> items;

	// map of transitions
	std::multimap<std::pair<tpl, std::string>, tpl> transitions;

	// NDFA states
	std::set<tpl> states;

	// hash set of empty non terminal symbols
	std::unordered_set<std::string> empty_symbols;
	// hash map of immediate starts of non terminal symbols
	std::unordered_map<std::string, std::vector<std::string>> immediate_starts;
	// hash map of all posible starts of non terminal symbols
	std::unordered_map<std::string, std::vector<std::string>> symbol_starts;
	// hash map of all poss starts relations
	std::unordered_map<std::string, std::vector<std::string>>
		non_terminal_starts;
	std::map<std::vector<std::string>, std::set<std::string>> starts;

	bool is_starter(std::vector<std::string> to_check) {
		return to_check.at(0) == ".";
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
		int previous_size = 0;
		int new_size = this->empty_symbols.size();
		while (previous_size != new_size) {
			for (auto production : productions) {
				int right_side_size = production.second.size();
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
			for (int i = 0;
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
			for (int i = 0; i < right.size(); ++i) {
				std::string current_right = right.at(i);
				this->symbol_starts[left].push_back(current_right);
				if (this->immediate_starts.count(current_right)) {
					std::vector<std::string> additions =
						this->immediate_starts[current_right];
					std::queue<std::string> q;
					std::set<std::string> visited;
					for (int j = 0; j < additions.size(); ++j) {
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

	void find_starts() {
		find_starts_symbols();
		for (auto current : this->symbol_starts) {
			for (auto pos : current.second) {
				if (terminals.count(pos)) {
					this->non_terminal_starts[current.first].push_back(pos);
				}
			}
		}
		for (auto item : items) {
			int i = 0;
			for (; i < item.size() && item.at(i) != "."; ++i)
				;
			if (i >= item.size() - 2) {
				continue;
			} else {
				bool flag = false;
				for (int j = i + 2; j < item.size() && !flag; ++j) {
					if (terminals.count(item.at(j))) {
						this->starts[item].insert(item.at(j));
						flag = true;

					} else if (!empty_symbols.count(item.at(j))) {
						for (auto temp : non_terminal_starts[item.at(j)]) {
							this->starts[item].insert(temp);
							flag = true;
						}
					} else {
						for (auto temp : non_terminal_starts[item.at(j)]) {
							this->starts[item].insert(temp);
						}
					}
				}
			}
		}
	}

	void get_items() {
		for (auto production : productions) {
			std::string left = production.first;
			std::vector<std::string> right = production.second;
			if (right.size() == 1 && right.at(0) == "$") {
				items.insert(std::vector<std::string>(1, "."));
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
				items.insert(vec);
			}
		}
		find_starts();
		return;
	}

	void get_transitions() {
		std::string left = "START";
		std::vector<std::string> right = {".", starting_production};
		std::set<std::string> context = {"END"};
		std::queue<tpl> q;
		q.push(std::make_tuple(left, right, context));

		while (!q.empty()) {
			tpl front = q.front();
			auto [l, r, c] = front;
			q.pop();
			if (states.count(front)) {
				continue;
			}
			states.insert(front);
			int i = 0;

			for (; i < r.size() && r.at(i) != "."; ++i)
				;

			if (i + 1 < r.size()) {
				if (l != "START") {
					std::vector<std::string> tmp;
					for (int j = 0; j < r.size(); ++j) {
						if (r.at(j) == ".") {
							continue;
						}
						tmp.push_back(r.at(j));
					}
					tmp.insert(tmp.begin() + i + 1, ".");
					tpl second_new = std::make_tuple(l, tmp, c);
					transitions.insert(make_pair(
						std::make_pair(front, r.at(i + 1)), second_new));
					q.push(second_new);
				}

				std::set<std::string> next_context;
				if (i + 2 == r.size()) {
					next_context = c;
				} else {
					bool checking = false;
					bool poss = true;
					bool helper = false;
					for (auto it = r.begin(); it != r.end() && poss;
						 it = std::next(it)) {
						if (checking) {
							if (!empty_symbols.count(*it)) {
								poss = false;
							}
						}
						if (helper) {
							checking = true;
						}
						if (*it == ".") {
							helper = true;
						}
					}
					if (poss) {
						for (auto x : c) {
							next_context.insert(x);
						}
					}
					if (starts.count(r)) {
						for (auto x : starts[r]) {
							next_context.insert(x);
						}
					}
				}

				auto range = productions.equal_range(r.at(i + 1));
				while (range.first != range.second) {
					std::string next_left = range.first->first;
					std::vector<std::string> next_right = range.first->second;
					// for (auto x : next_right) {
					// 	std::cout << x << ' ';
					// }
					// std::cout << std::endl;
					std::vector<std::string> first_right = next_right;

					first_right.insert(first_right.begin(), ".");
					if (next_right.size() == 1 && next_right.at(0) == "$") {
						first_right = {"."};
					}

					tpl first_new =
						std::make_tuple(next_left, first_right, next_context);
					if (states.count(first_new)) {
						transitions.insert(make_pair(
							std::make_pair(make_tuple(l, r, c), "EPSILON"),
							first_new));
					} else {
						transitions.insert(make_pair(
							std::make_pair(make_tuple(l, r, c), "EPSILON"),
							first_new));
						q.push(first_new);
					}
					range.first = std::next(range.first);
				}
			}
		}

		return;
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

	void print_starts() {
		for (auto x : starts) {
			for (auto y : x.first) {
				std::cout << y << ' ';
			}
			std::cout << ": ";
			for (auto y : x.second) {
				std::cout << y << ' ';
			}
			std::cout << std::endl;
		}
	}
	void print_transitions() {
		std::cout << transitions.size() << std::endl;
		for (auto x : transitions) {
			auto s = x.first.second;
			auto [d, e, f] = x.second;
			auto [a, b, c] = x.first.first;

			std::cout << a << ":";
			for (auto y : b) {
				std::cout << y;
			}
			std::cout << "{";
			for (auto y : c) {
				std::cout << y;
			}
			std::cout << "}";
			std::cout << " ->" + s + ": ";
			std::cout << d << ":";
			for (auto y : e) {
				std::cout << y;
			}
			std::cout << "{";
			for (auto y : f) {
				std::cout << y;
			}
			std::cout << "}";
			std::cout << std::endl;
			std::cout << std::endl;
		}
	}

  public:
	NDFA(std::multimap<std::string, std::vector<std::string>> productions,
		 std::string starting_production,
		 std::unordered_set<std::string> non_terminals,
		 std::unordered_set<std::string> terminals,
		 std::unordered_set<std::string> sync) {
		this->terminals = terminals;
		this->non_terminals = non_terminals;
		this->sync = sync;
		this->productions = productions;
		this->starting_production = starting_production;

		get_items();
		get_transitions();
		// print_transitions();
		for (auto x : this->states) {
			auto [a, b, c] = x;
			std::cout << a << ": ";
			for (auto y : b) {
				std::cout << y;
			}
			std::cout << "  kont: ";
			for (auto y : c) {
				std::cout << y;
			}
			std::cout << std::endl;
		}
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
