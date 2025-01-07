#include <algorithm>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Node {
  public:
	std::string type = "undefined", inherited_type = "undefined", name,
				return_type = "undefined";
	bool lhs = false;
	int row, depth, element_count, input_row;
	std::vector<std::string> arg_types, arg_names, param_types;
	std::string symbol, value;
	std::vector<std::shared_ptr<Node>> children;

	Node(std::string symbol, int row, std::string value, int depth,
		 int input_row) {
		this->symbol = symbol;
		this->row = row;
		this->value = value;
		this->depth = depth;
		this->input_row = input_row;
	}
	~Node() { std::cerr << "Destroyed node: " << this->input_row << std::endl; }
	void print_node(void) {
		std::cout << "Node: " << symbol << std::endl;
		std::cout << type << std::endl;
		if (row != -1) {
			std::cout << value << ' ' << row << ' ' << std::endl;
		}
	}

	int semantic_error(void) {
		// for debug
		// std::cout << input_row << std::endl;
		std::cout << symbol << " ::=";
		for (const std::shared_ptr<Node> &current : children) {
			std::cout << ' ' << current->symbol;
			if (current->row >= 0) {
				std::cout << '(' << current->row << ',' << current->value
						  << ')';
			}
		}
		std::cout << std::endl;
		exit(1);
		return 1;
	}
};
