#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <vector>
using ll = int64_t;

struct NDFA {
	std::string lex_state;

	std::vector<std::vector<std::string>> args;
	std::multimap<std::pair<int, char>, int> transitions;
	std::set<int> acc;
	std::set<int> start;

	NDFA() {};
};
std::vector<NDFA> ndfas;

// find index of NDFA based on lex_state
int find_index(std::string src) {
	int i;
	for (i = 0; i < ndfas.size(); ++i) {
		if (ndfas.at(i).lex_state == src) {
			return i;
		}
	}
	return i + 1;
}

// find epsilon closure
std::set<int> epsi_closure(std::set<int> states, int index) {
	std::multimap<std::pair<int, char>, int> ts = ndfas.at(index).transitions;
	std::stack<int> stack;
	std::set<int> res;
	for (auto el : states) {
		stack.push(el);
	}
	while (!stack.empty()) {
		int top = stack.top();
		stack.pop();
		auto range = ts.equal_range(std::make_pair(top, '$'));
		for (auto it = range.first; it != range.second; ++it) {
			if (res.count(it->second) == 0) {
				stack.push(it->second);
				res.insert(it->second);
			}
		}
	}
	return res;
}

// find the intersection of sets
std::set<int> intersect(std::set<int> first_set, std::set<int> second_set) {
	std::set<int> intersection;
	std::set_intersection(first_set.begin(), first_set.end(),
						  second_set.begin(), second_set.end(),
						  std::inserter(intersection, intersection.begin()));
	return intersection;
}

// find all transitions
std::set<int> transitions(std::set<int> set, char sign, int index) {
	std::multimap<std::pair<int, char>, int> ts = ndfas.at(index).transitions;
	std::set<int> res;
	for (auto x : set) {
		auto range = ts.equal_range(std::make_pair(x, sign));
		for (auto it = range.first; it != range.second; ++it) {
			if (res.count(it->second) == 0) {
				res.insert(it->second);
			}
		}
	}
	return res;
}

int main(void) {
	std::string line;
	std::ifstream automata_tab("automat.tab");

	std::vector<std::string> lex_states;
	if (automata_tab.is_open()) {
		std::string line;
		int read_trans = 0;
		int read_args = 0;
		int index;
		std::vector<std::string> current_args;
		while (std::getline(automata_tab, line)) {
			if (line.substr(0, 2) == "S_") {
				auto it = std::find(lex_states.begin(), lex_states.end(), line);
				if (it == lex_states.end()) {
					NDFA current;
					current.lex_state = line;
					ndfas.push_back(current);
					index = ndfas.size() - 1;
					lex_states.push_back(line);
				} else {
					index = std::distance(lex_states.begin(), it);
				}
				current_args.erase(current_args.begin(), current_args.end());
				read_trans = 0;
				read_args = 4;
			} else if (read_args > 0) {
				if (!line.empty()) {
					current_args.push_back(line);
				}
				if (read_args == 1) {
					ndfas.at(index).args.push_back(current_args);
				}
				--read_args;

			} else if (line.substr(0, line.find(' ')) == "START") {
				ndfas.at(index).start.insert(
					std::stoi(line.substr(line.find(' ') + 1)));
			} else if (line.substr(0, line.find(' ')) == "ACCEPTED") {
				ndfas.at(index).acc.insert(
					std::stoi(line.substr(line.find(' ') + 1)));
			} else if (line == "TRANSITIONS") {
				read_trans = 1;
				continue;
			} else if (read_trans == 1) {
				int first = std::stoi(line.substr(0, line.find(' ')));
				line = line.substr(line.find(' ') + 1);
				std::string sign = (line.substr(0, line.find(' ')));
				char real_sign;
				if (sign == "|n") {
					real_sign = '\n';
				} else if (sign == "|t") {
					real_sign = '\t';
				} else if (sign == "|_") {
					real_sign = ' ';
				} else {
					real_sign = sign.at(0);
				}
				line = line.substr(line.find(' ') + 1);
				int second = std::stoi(line);
				ndfas.at(index).transitions.emplace(
					std::make_pair(first, real_sign), second);
			}
		}
		automata_tab.close();
	} else {
		std::cerr << "Unable to open file.\n";
	}
	// for (auto x : ndfas) {
	// 	std::cout << x.lex_state << std::endl;
	// 	for (auto y : x.args) {
	// 		for (auto z : y) {
	// 			std::cout << z << std::endl;
	// 		}
	// 	}
	// 	for (auto y : x.start) {
	// 		std::cout << y << std::endl;
	// 	}
	// 	for (auto y : x.acc) {
	// 		std::cout << y << std::endl;
	// 	}
	// 	for (auto y : x.transitions) {
	// 		int f = y.first.first;
	// 		char t = y.first.second;
	// 		int s = y.second;
	// 		std::cout << f << ' ' << t << ' ' << s << std::endl;
	// 	}
	// }

	std::string curr_lex_state = ndfas.at(0).lex_state;
	std::set<int> atm_states = ndfas.at(0).start;
	std::set<int> R = epsi_closure(atm_states, 0);
	std::set<int> F = ndfas.at(0).acc;

	int line_counter = 0;
	while (std::getline(std::cin, line)) {
		int last = 0;
		int first = 0;
		int second = 0;
		char reading;
		int accepted_expr;
		while (last < line.size()) {
			std::set<int> inter = intersect(R, F);
			if (!R.empty() && inter.empty()) {
				reading = line.at(last);
				++last;
				std::set<int> Q(R);
				int ind = find_index(curr_lex_state);
				R = epsi_closure(transitions(Q, reading, ind), ind);
			} else if (!inter.empty()) {
				accepted_expr = *inter.begin();
				second = last;
				++last;
				std::set<int> Q(R);
				reading = line.at(last);
				int ind = find_index(curr_lex_state);
				R = epsi_closure(transitions(Q, reading, ind), ind);
			}
		}
	}

	return 0;
}
