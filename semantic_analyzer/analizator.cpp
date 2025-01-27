#include "../code_generator/code_generator.hpp"
#include "productions.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

int main(void) {
	code_init();
	std::string line;
	int input_row = 0;
	std::vector<std::shared_ptr<Node>> all_nodes;
	while (std::getline(std::cin, line)) {
		++input_row;
		int current_depth = 0, current_row = -1;
		std::string current_symbol, current_value;
		// depth of current node
		while (line.at(current_depth) == ' ') {
			++current_depth;
		}
		if (line.at(current_depth) == '<') {
			current_symbol = line.substr(current_depth);
		} else {
			line = line.substr(current_depth);
			current_symbol = line.substr(0, line.find(' '));
			line = line.substr(line.find(' ') + 1);
			current_row = std::stoi(line.substr(0, line.find(' ')));
			line = line.substr(line.find(' ') + 1);
			current_value = line;
		}
		std::shared_ptr<Node> current_node = std::make_unique<Node>(
			Node(current_symbol, current_row, current_value, current_depth,
				 input_row));
		if (all_nodes.empty()) {
			all_nodes.push_back(current_node);
			continue;
		}
		size_t iter;
		for (iter = all_nodes.size() - 1;
			 iter >= 0 && all_nodes.at(iter)->depth != current_depth - 1;
			 --iter)
			;
		if (iter >= 0) {
			all_nodes.at(iter)->children.push_back(current_node);
			all_nodes.push_back(current_node);
		}
	}
	if (prijevodna_jedinica(all_nodes.at(0)) == 0) {
		if (!main_defined) {
			std::cout << "main" << std::endl;
		} else if (declared_functions.size() > 0) {
			std::cout << "funkcija" << std::endl;
		}
	}
	fill_globals();
	fill_consts();
	return 0;
}
