#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// Operator precedence and associativity
struct Operator {
	int precedence;
	bool rightAssociative;
};

const std::unordered_map<std::string, Operator> OPERATORS = {
	{"PLUS", {1, false}},		{"MINUS", {1, false}},
	{"OP_PUTA", {2, false}},	{"OP_DIJELI", {2, false}},
	{"OP_MOD", {2, false}},		{"OP_BIN_I", {4, false}},
	{"OP_BIN_ILI", {4, false}}, {"OP_BIN_XILI", {4, false}},
	{"OP_TILDA", {6, true}},	{"OP_NEG", {6, true}}};

bool isOperator(const std::string &token) {
	return OPERATORS.find(token) != OPERATORS.end();
}

bool isUnary(const std::string &token) {
	return token == "OP_TILDA" || token == "OP_NEG";
}

int evaluate(const std::vector<std::string> &expr) {
	std::vector<std::string> output;
	std::stack<std::string> opStack;

	for (size_t i = 0; i < expr.size(); ++i) {
		const std::string &token = expr[i];

		try {
			int number = std::stoi(token);
			output.push_back(token);
		} catch (std::invalid_argument arg) {
			if (isOperator(token)) { // Operator
				while (!opStack.empty() && isOperator(opStack.top())) {
					const std::string &topOp = opStack.top();
					if ((OPERATORS.at(token).rightAssociative &&
						 OPERATORS.at(token).precedence <
							 OPERATORS.at(topOp).precedence) ||
						(!OPERATORS.at(token).rightAssociative &&
						 OPERATORS.at(token).precedence <=
							 OPERATORS.at(topOp).precedence)) {
						output.push_back(opStack.top());
						opStack.pop();
					} else {
						break;
					}
				}
				opStack.push(token);
			} else if (token == "(") {
				opStack.push(token);
			} else if (token == ")") {
				while (!opStack.empty() && opStack.top() != "(") {
					output.push_back(opStack.top());
					opStack.pop();
				}
				if (!opStack.empty() && opStack.top() == "(") {
					opStack.pop();
				}
			}
		}
	}

	while (!opStack.empty()) {
		output.push_back(opStack.top());
		opStack.pop();
	}

	std::stack<int> values;

	for (const std::string &token : output) {
		try { // Operand
			values.push(std::stoi(token));
		} catch (std::invalid_argument arg) {
			if (isOperator(token)) { // Operator
				if (isUnary(token)) {
					if (values.empty())
						throw std::runtime_error("Invalid expression");
					int val = values.top();
					values.pop();
					if (token == "OP_TILDA")
						values.push(~val);
					else if (token == "OP_NEG")
						values.push(!val);
				} else {
					if (values.size() < 2)
						throw std::runtime_error("Invalid expression");
					int b = values.top();
					values.pop();
					int a = values.top();
					values.pop();

					if (token == "PLUS")
						values.push(a + b);
					else if (token == "MINUS")
						values.push(a - b);
					else if (token == "OP_PUTA")
						values.push(a * b);
					else if (token == "OP_DIJELI")
						values.push(a / b);
					else if (token == "OP_MOD")
						values.push(a % b);
					else if (token == "OP_BIN_XILI")
						values.push(a ^ b);
					else if (token == "OP_BIN_I")
						values.push(a & b);
					else if (token == "OP_BIN_ILI")
						values.push(a | b);
				}
			}
		}
	}

	if (values.size() != 1)
		throw std::runtime_error("Invalid expression");
	return values.top();
}
