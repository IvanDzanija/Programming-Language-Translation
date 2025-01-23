#include <fstream>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using tpl =
	std::tuple<std::string, std::vector<std::string>, std::set<std::string>>;
using ll = int64_t;
using ull = uint64_t;

struct PairHash {
	ull operator()(const std::pair<int, std::string> &p) const {
		std::string s = p.second + std::to_string(p.first);
		ull hash = 5381;
		for (auto c : s)
			hash = ((hash << 5) + hash) + c;

		return hash;
	}
};
class ENFA {
  private:
	std::unordered_set<std::string> non_terminals;
	std::unordered_set<std::string> terminals;
	std::unordered_set<std::string> sync;
	std::multimap<std::string, std::vector<std::string>> productions;
	std::string starting_production;

	std::set<std::vector<std::string>> items;

	std::multimap<std::pair<tpl, std::string>, tpl> transitions;
	std::map<std::vector<std::string>, int> reductions;

	std::set<tpl> states;

	std::unordered_set<std::string> empty_symbols;
	std::unordered_map<std::string, std::unordered_set<std::string>>
		immediate_starts;
	std::unordered_map<std::string, std::unordered_set<std::string>>
		symbol_starts;
	std::unordered_map<std::string, std::unordered_set<std::string>>
		non_terminal_starts;
	std::map<std::vector<std::string>, std::unordered_set<std::string>> starts;

	std::set<std::set<tpl>> DFA_states;
	std::unordered_map<int, std::set<tpl>> saver_left;
	std::map<std::set<tpl>, int> saver_right;
	std::unordered_map<std::pair<int, std::string>, int, PairHash>
		DFA_transitions;
	std::map<tpl, std::set<tpl>> memoization;

	std::map<std::pair<int, std::string>, std::vector<std::string>> action;
	std::map<std::pair<int, std::string>, int> new_state;

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
			this->immediate_starts[left].insert(right.at(0));
			for (int i = 0;
				 i < right.size() - 1 && empty_symbols.count(right.at(i));
				 ++i) {
				this->immediate_starts[left].insert(right.at(i + 1));
			}
		}
	}

	void find_starts_symbols() {
		find_immediate_starts();
		for (auto table : this->immediate_starts) {
			std::string left = table.first;
			this->symbol_starts[left].insert(left);
			std::unordered_set<std::string> right = table.second;
			for (auto i : right) {
				std::string current_right = i;
				this->symbol_starts[left].insert(current_right);
				if (this->immediate_starts.count(current_right)) {
					std::unordered_set<std::string> additions =
						this->immediate_starts[current_right];
					std::queue<std::string> q;
					std::set<std::string> visited;
					for (auto j : additions) {
						q.push((j));
					}
					while (!q.empty()) {
						std::string current = q.front();
						q.pop();
						if (visited.count(current)) {
							continue;
						}
						visited.insert(current);
						this->symbol_starts[left].insert(current);
						if (this->immediate_starts.count(current)) {
							std::unordered_set<std::string> news =
								this->immediate_starts[current];
							for (auto x : news) {
								q.push(x);
							}
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
					this->non_terminal_starts[current.first].insert(pos);
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
		std::set<std::string> context = {"!END!"};
		std::queue<tpl> q;
		q.push(std::make_tuple(left, right, context));
		std::set<tpl> visited;
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
				std::vector<std::string> tmp;
				for (int j = 0; j < r.size(); ++j) {
					if (r.at(j) == ".") {
						continue;
					}
					tmp.push_back(r.at(j));
				}
				tmp.insert(tmp.begin() + i + 1, ".");
				tpl second_new = std::make_tuple(l, tmp, c);
				transitions.insert(
					make_pair(std::make_pair(front, r.at(i + 1)), second_new));
				q.push(second_new);

				std::set<std::string> next_context;
				if (i + 2 == r.size()) {
					next_context = c;
				} else {

					bool poss = true;

					for (auto it = r.begin() + 2; it != r.end();
						 it = std::next(it)) {

						if (!empty_symbols.count(*it)) {
							poss = false;
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

	std::set<tpl> epsi_closure(tpl state) {
		if (memoization.count(state)) {
			return memoization.at(state);
		}
		std::set<tpl> ans;
		std::stack<tpl> stack;
		std::string epsi = "EPSILON";
		stack.push(state);

		while (!stack.empty()) {
			tpl top = stack.top();
			stack.pop();
			if (ans.count(top)) {
				continue;
			}
			ans.insert(top);
			auto range = transitions.equal_range(std::make_pair(top, epsi));
			while (range.first != range.second) {

				stack.push(range.first->second);
				range.first = std::next(range.first);
			}
		}

		memoization[state] = ans;
		return ans;
	}

	void create_DFA() {
		int counter = 0;
		std::string left = "START";
		std::vector<std::string> right = {".", starting_production};
		std::set<std::string> context = {"!END!"};
		std::queue<std::set<tpl>> q;
		q.push(epsi_closure(std::make_tuple(left, right, context)));
		while (!q.empty()) {
			std::set<tpl> top = q.front();
			q.pop();
			if (DFA_states.count(top)) {
				continue;
			}
			if (saver_right.count(top)) {
				counter = saver_right.at(top);
			}
			if (counter == 0) {
				saver_left[counter] = top;
				saver_right[top] = counter;
			}

			DFA_states.insert(top);
			for (auto sign : terminals) {
				std::set<tpl> transitions_states;
				for (auto state : top) {
					auto range =
						transitions.equal_range(std::make_pair(state, sign));
					for (; range.first != range.second;
						 range.first = std::next(range.first)) {

						std::set<tpl> current_closure =
							epsi_closure(range.first->second);
						for (auto x : current_closure) {
							transitions_states.insert(x);
						}
					}
				}
				if (!transitions_states.empty()) {
					if (saver_right.count(transitions_states)) {
						DFA_transitions.emplace(
							std::make_pair(std::make_pair(counter, sign),
										   saver_right.at(transitions_states)));
					} else {
						int inner = saver_right.size();
						DFA_transitions.emplace(std::make_pair(
							std::make_pair(counter, sign), inner));
						saver_left[inner] = transitions_states;
						saver_right[transitions_states] = inner;
					}
					q.push(transitions_states);
				}
			}
			for (auto sign : non_terminals) {
				std::set<tpl> transitions_states;
				for (auto state : top) {
					auto range =
						transitions.equal_range(std::make_pair(state, sign));
					for (; range.first != range.second;
						 range.first = std::next(range.first)) {
						std::set<tpl> current_closure =
							epsi_closure(range.first->second);
						for (auto x : current_closure) {
							transitions_states.insert(x);
						}
					}
				}
				if (!transitions_states.empty()) {
					if (saver_right.count(transitions_states)) {
						DFA_transitions.emplace(
							std::make_pair(std::make_pair(counter, sign),
										   saver_right.at(transitions_states)));
					} else {
						int inner = saver_right.size();
						DFA_transitions.emplace(std::make_pair(
							std::make_pair(counter, sign), inner));
						saver_left[inner] = transitions_states;
						saver_right[transitions_states] = inner;
					}
					q.push(transitions_states);
				}
			}
			++counter;
		}
	}

	void build_tables() {
		for (auto x : saver_left) {
			int first_num = x.first;
			std::set<tpl> state = x.second;
			for (auto [l, r, c] : state) {
				if (r.at(r.size() - 1) == ".") {
					if (l == "START" && c.size() == 1 &&
						*c.begin() == "!END!") {
						action[std::make_pair(first_num, "!END!")] = {"ACCEPT"};
					} else {
						std::unordered_set<std::string> current_terms =
							terminals;
						current_terms.insert("!END!");
						for (auto terminal : current_terms) {
							if (c.count(terminal)) {
								std::vector<std::string> vec(r.begin(),
															 r.end() - 1);
								vec.insert(vec.begin(), l);
								if (action.count(
										std::make_pair(first_num, terminal)) &&
									action.at(std::make_pair(first_num,
															 terminal))
											.at(0)
											.at(0) == '<' &&
									reductions.at(vec) >
										reductions.at(action.at(std::make_pair(
											first_num, terminal)))) {

								} else {
									action[std::make_pair(first_num,
														  terminal)] = {l};
									for (int i = 0; i < r.size() - 1; ++i) {
										std::string sign = r.at(i);
										action[std::make_pair(first_num,
															  terminal)]
											.push_back(sign);
									}
								}
							}
						}
					}
				} else {
					for (auto terminal : terminals) {
						if (DFA_transitions.count(
								std::make_pair(first_num, terminal))) {
							int next = DFA_transitions.at(
								std::make_pair(first_num, terminal));
							action[std::make_pair(first_num, terminal)] = {
								std::to_string(next)};
						}
					}
				}
			}
		}
		for (auto x : DFA_transitions) {
			int first = x.first.first;
			int second = x.second;
			std::string s = x.first.second;
			if (non_terminals.count(s)) {
				new_state[std::make_pair(first, s)] = second;
			}
		}
	}

  public:
	ENFA(std::multimap<std::string, std::vector<std::string>> productions,
		 std::string starting_production,
		 std::unordered_set<std::string> non_terminals,
		 std::unordered_set<std::string> terminals,
		 std::unordered_set<std::string> sync,
		 std::map<std::vector<std::string>, int> reductions) {
		this->terminals = terminals;
		this->non_terminals = non_terminals;
		this->sync = sync;
		this->productions = productions;
		this->starting_production = starting_production;
		this->reductions = reductions;
		get_items();
		get_transitions();
		create_DFA();
		build_tables();
	}

	std::map<std::pair<int, std::string>, std::vector<std::string>>
	get_action() {
		return this->action;
	}
	std::map<std::pair<int, std::string>, int> get_new_state() {
		return this->new_state;
	}
};
int main(void) {
	int red_cnt = 0;
	std::string line;
	std::unordered_set<std::string> terminals;
	std::unordered_set<std::string> non_terminals;
	std::unordered_set<std::string> sync;
	std::multimap<std::string, std::vector<std::string>> productions;
	std::map<std::vector<std::string>, int> reductions;
	std::string current_sign;
	std::string starting_production;
	std::vector<std::string> temp;
	while (std::getline(std::cin, line)) {

		if (line.substr(0, 2) == "%V") {
			line = line.substr(line.find(' ') + 1);
			starting_production = line.substr(0, line.find(' '));

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

		else if (line.at(0) != ' ') {
			current_sign = line;
			temp = {line};
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
			if (right_production.size() == 1 && right_production.at(0) == "$") {
				reductions[temp] = red_cnt++;
				productions.emplace(current_sign, right_production);
			} else {
				for (auto y : right_production) {
					temp.push_back(y);
				}
				reductions[temp] = red_cnt++;
				productions.emplace(current_sign, right_production);
			}
		}
	}
	ENFA enka(productions, starting_production, non_terminals, terminals, sync,
			  reductions);
	std::map<std::pair<int, std::string>, std::vector<std::string>> action =
		enka.get_action();
	std::map<std::pair<int, std::string>, int> new_state = enka.get_new_state();
	std::ofstream file("./analizator/table.txt");
	if (file.is_open()) {
		file << "ACTION TABLE\n";
		for (auto x : action) {
			file << ' ' << x.first.first << ' ' << x.first.second << std::endl;
			for (auto y : x.second) {
				file << y << ' ';
			}
			file << std::endl;
		}
		file << "NEW STATE TABLE\n";
		for (auto x : new_state) {
			file << x.first.first << ' ' << x.first.second << ' ' << x.second
				 << std::endl;
		}
		file << "SYNC SYMBOLS\n";
		for (auto x : sync) {
			file << x << ' ';
		}
		file << std::endl;

	} else {
		std::cerr << "Unable to open file.\n";
	}
	file.close();

	return 0;
}
