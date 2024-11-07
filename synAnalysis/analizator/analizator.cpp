#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <unordered_set>
#include <vector>
using ll = int64_t;

std::map<int, std::vector<std::string>> reds;

void travers(int red_cnt, int space) {
	std::vector<std::string> current_node = reds[red_cnt];
	for (int i = 0; i < current_node.size(); ++i) {
		std::string a = current_node.at(i);
		for (int j = 0; j < space; ++j) {
			std::cout << ' ';
		}
		if (a.at(0) == '<') {
			std::string left = a.substr(0, a.find(' '));
			a = a.substr(current_node.at(i).find(' ') + 1);
			int right = std::stoi(a);
			std::cout << left << std::endl;
			travers(right, space + 1);
		} else {
			std::cout << current_node.at(i) << std::endl;
		}
	}
}

int main(void) {
	std::ifstream table("table.txt");
	std::map<std::pair<int, std::string>, std::vector<std::string>> actions;
	std::map<std::pair<int, std::string>, int> new_states;
	std::unordered_set<std::string> sync;
	int reader = 0;
	std::string state_s;
	int state;
	std::string sym;
	if (table.is_open()) {
		std::string line;
		while (std::getline(table, line)) {
			if (line == "ACTION TABLE") {
				reader = 1;
			} else if (line == "NEW STATE TABLE") {

				reader = 2;
			} else if (line == "SYNC SYMBOLS") {
				reader = 3;
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
	int reduction_counter = 1;

	while (std::getline(std::cin, stdin)) {
		input.push_back(stdin);
	}
	input.push_back("!END!");
	for (int i = 0; i < input.size();) {
		std::string line = input.at(i);
		std::string reading = line.substr(0, line.find(' '));
		std::string current_state_s = stack.top();

		int current_state = std::stoi(current_state_s);
		if (actions.count(std::make_pair(current_state, reading))) {
			std::vector<std::string> action =
				actions.at(std::make_pair(current_state, reading));
			if (action.size() > 1) {

				std::vector<std::string> new_nodes;
				std::vector<std::string> clean;
				for (int j = 1; j <= (action.size() - 1) * 2; ++j) {
					if (j & 1) {
						stack.pop();
						continue;
					} else {
						std::string a = stack.top();
						new_nodes.push_back(a.substr(0, a.find(' ')));
						clean.push_back(a);
						stack.pop();
					}
				}

				std::vector<std::string> current;
				for (int j = new_nodes.size() - 1; j >= 0; --j) {
					current.push_back(new_nodes.at(j));
				}
				std::vector<std::string> temp(action.begin() + 1, action.end());

				if (current == temp) {
					std::string top_s = stack.top();
					top_s = top_s.substr(0, top_s.find(' '));
					int top = std::stoi(top_s);

					std::string new_state_s;
					if (new_states.count(std::make_pair(top, action.at(0)))) {
						int new_state =
							new_states.at(std::make_pair(top, action.at(0)));

						new_state_s = std::to_string(new_state);
					} else {
						std::cerr << "err" << std::endl;
					}
					stack.push(action.at(0) + ' ' +
							   std::to_string(reduction_counter));
					stack.push(new_state_s);
					std::vector<std::string> b(clean.rbegin(), clean.rend());
					reds[reduction_counter] = b;
					++reduction_counter;
				} else {
					break;
				}

			} else if (action.at(0).at(0) == '<') {

				std::string top_s = stack.top();
				int top = std::stoi(top_s);

				std::string new_state_s;
				if (new_states.count(std::make_pair(top, action.at(0)))) {
					int new_state =
						new_states.at(std::make_pair(top, action.at(0)));

					new_state_s = std::to_string(new_state);
				} else {
				}
				stack.push(action.at(0) + ' ' +
						   std::to_string(reduction_counter));
				stack.push(new_state_s);
				std::vector<std::string> b = {"$"};
				reds[reduction_counter] = b;
				++reduction_counter;
			} else if (action.at(0) == "ACCEPT") {
				++i;
				stack.pop();
				reds[reduction_counter] = {stack.top()};
			} else {
				++i;
				stack.push(line);
				stack.push(action.at(0));
			}
		} else {
			std::string no_of_line = line.substr(line.find(' ') + 1);
			std::cerr << "Syntax error line: "
					  << no_of_line.substr(0, no_of_line.find(' '))
					  << std::endl;
			std::cerr << line << std::endl;
			std::string reading = line.substr(0, line.find(' '));
			while (!sync.count(reading)) {
				++i;
				reading = input.at(i).substr(0, line.find(' '));
			}
			std::string top_s = stack.top();
			int top = std::stoi(top_s);
			while (!actions.count(std::make_pair(top, reading))) {
				stack.pop();
				stack.pop();
				top_s = stack.top();
				top = std::stoi(top_s);
			}
			std::vector<std::string> action =
				actions.at(std::make_pair(top, reading));
			if (action.size() > 1) {

				std::vector<std::string> new_nodes;
				std::vector<std::string> clean;
				for (int j = 1; j <= (action.size() - 1) * 2; ++j) {
					if (j & 1) {
						stack.pop();
						continue;
					} else {
						std::string a = stack.top();
						new_nodes.push_back(a.substr(0, a.find(' ')));
						clean.push_back(a);
						stack.pop();
					}
				}

				std::vector<std::string> current;
				for (int j = new_nodes.size() - 1; j >= 0; --j) {
					current.push_back(new_nodes.at(j));
				}
				std::vector<std::string> temp(action.begin() + 1, action.end());

				if (current == temp) {
					std::string top_s = stack.top();
					top_s = top_s.substr(0, top_s.find(' '));
					int top = std::stoi(top_s);

					std::string new_state_s;
					if (new_states.count(std::make_pair(top, action.at(0)))) {
						int new_state =
							new_states.at(std::make_pair(top, action.at(0)));

						new_state_s = std::to_string(new_state);
					} else {
					}
					stack.push(action.at(0) + ' ' +
							   std::to_string(reduction_counter));
					stack.push(new_state_s);
					std::vector<std::string> b(clean.rbegin(), clean.rend());
					reds[reduction_counter] = b;
					++reduction_counter;
				} else {
					break;
				}

			} else if (action.at(0).at(0) == '<') {

				std::string top_s = stack.top();
				int top = std::stoi(top_s);

				std::string new_state_s;
				if (new_states.count(std::make_pair(top, action.at(0)))) {
					int new_state =
						new_states.at(std::make_pair(top, action.at(0)));

					new_state_s = std::to_string(new_state);
				} else {
				}
				stack.push(action.at(0) + ' ' +
						   std::to_string(reduction_counter));
				stack.push(new_state_s);
				std::vector<std::string> b = {"$"};
				reds[reduction_counter] = b;
				++reduction_counter;
			} else if (action.at(0) == "ACCEPT") {
				++i;
				stack.pop();
				reds[reduction_counter] = {stack.top()};
			} else {
				++i;
				stack.push(line);
				stack.push(action.at(0));
			}
		}
	}
	travers(reduction_counter, 0);
	return 0;
}
