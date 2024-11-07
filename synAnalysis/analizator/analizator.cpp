#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <unordered_set>
#include <vector>
using ll = int64_t;

int main(void) {
	std::ifstream table("table.txt");
	std::map<std::pair<int, std::string>, std::vector<std::string>> actions;
	std::map<std::pair<int, std::string>, int> new_states;
	std::map<std::pair<std::string, std::vector<std::string>>, int> reductions;
	std::unordered_set<std::string> sync;

	int reader = -1;
	int index = 0;
	std::string state_s;
	int state;
	std::string sym;
	if (table.is_open()) {
		std::string line;
		while (std::getline(table, line)) {
			if (line == "REDUCTION ORDER") {
				reader = 0;
			} else if (line == "ACTION TABLE") {
				reader = 1;
			} else if (line == "NEW STATE TABLE") {
				reader = 2;
			} else if (line == "SYNC SYMBOLS") {
				reader = 3;
			} else if (reader == 0) {
				line = line.substr(line.find(' ') + 1);
				std::string left = line.substr(0, line.find('-'));
				std::vector<std::string> current_right;
				line = line.substr(line.find('-') + 1);
				while (!line.empty()) {
					if (line.find(' ') != std::string::npos) {
						current_right.push_back(line.substr(0, line.find(' ')));
						line = line.substr(line.find(' ') + 1);
					} else {
						current_right.push_back(line);
						line.clear();
					}
				}
				reductions[std::make_pair(left, current_right)] = index++;
			} else if (reader == 1) {
				if (line.at(0) == ' ') {
					line = line.substr(1);
					state_s = line.substr(0, line.find(' '));
					state = std::stoi(state_s);
					line = line.substr(line.find(' ') + 1);
					sym = line;
				} else {
					std::vector<std::string> temp;
					while (!line.empty()) {
						std::string current;
						if (line.find(' ') != std::string::npos) {
							current = line.substr(0, line.find(' '));
							line = line.substr(line.find(' ') + 1);
						} else {
							current = line;
							line.clear();
						}
						temp.push_back(current);
					}
					actions[std::make_pair(state, sym)] = temp;
				}
			} else if (reader == 2) {
				std::string first_s = line.substr(0, line.find(' '));
				int first = std::stoi(first_s);
				line = line.substr(line.find(' ') + 1);
				std::string sign = line.substr(0, line.find(' '));
				line = line.substr(line.find(' ') + 1);
				std::string second_s = line;
				int second = std::stoi(second_s);
				new_states[std::make_pair(first, sign)] = second;
			} else if (reader == 3) {
				while (!line.empty()) {
					if (line.find(' ') != std::string::npos) {
						sync.insert(line.substr(0, line.find(' ')));
						line = line.substr(line.find(' ') + 1);
					} else {
						sync.insert(line);
						line.clear();
					}
				}
			}
		}
	} else {
		std::cerr << "Unable to open file.\n";
	}
	std::stack<std::string> stack;
	std::vector<std::string> input;
	std::string stdin;
	stack.push("0");
	while (std::getline(std::cin, stdin)) {
		input.push_back(stdin);
	}
	input.push_back("!END!");
	for (int i = 0; i < input.size();) {
		std::string line = input.at(i);
		std::string reading = line.substr(0, line.find(' '));
		std::string current_state_s = stack.top();
		// std::cout << reading << ' ' << current_state_s << std::endl;
		int current_state = std::stoi(current_state_s);
		if (actions.count(std::make_pair(current_state, reading))) {
			std::vector<std::string> action =
				actions.at(std::make_pair(current_state, reading));
			if (action.size() > 1) {
				std::cout << "REDUKCIJA: " << reading << ' ' << current_state
						  << ' ';
				for (auto z : action) {
					std::cout << z << ' ';
				}
				std::cout << std::endl;
				for (int i = 1; i <= (action.size() - 1) * 2; ++i) {
					if (i & 1) {
						stack.pop();
						continue;
					} else {
						std::string a = stack.top();
						// std::cout << a << ' ';
						stack.pop();
					}
				}
				std::string top_s = stack.top();
				int top = std::stoi(top_s);
				std::cout << "Stanje ispod: " << top << std::endl;
				std::string new_state_s;
				if (new_states.count(std::make_pair(top, action.at(0)))) {
					int new_state =
						new_states.at(std::make_pair(top, action.at(0)));
					std::cout << "Novo: " << new_state << std::endl;
					new_state_s = std::to_string(new_state);
				} else {
					std::cout << "err" << std::endl;
				}
				stack.push(action.at(0));
				stack.push(new_state_s);
				// std::cout << std::endl;
			} else if (action.at(0).at(0) == '<') {
				std::cout << '$' << std::endl;
				std::string top_s = stack.top();
				int top = std::stoi(top_s);
				std::cout << "Stanje ispod: " << top << std::endl;
				std::string new_state_s;
				if (new_states.count(std::make_pair(top, action.at(0)))) {
					int new_state =
						new_states.at(std::make_pair(top, action.at(0)));
					std::cout << "Novo: " << new_state << std::endl;
					new_state_s = std::to_string(new_state);
				} else {
					std::cout << "err" << std::endl;
				}
				stack.push(action.at(0));
				stack.push(new_state_s);
			} else {
				++i;
				std::cout << reading << ' ' << action.at(0) << std::endl;
				stack.push(reading);
				stack.push(action.at(0));
			}
		}
	}
	return 0;
}
