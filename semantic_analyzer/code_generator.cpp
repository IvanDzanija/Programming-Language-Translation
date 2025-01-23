#include "code_generator.hpp"
#include <fstream>

int logical_skip = 0;
std::ofstream code("a.frisc");
std::unordered_map<std::string, std::pair<std::string, std::string>>
	code_global_variables;
std::string current_global_variable;
std::unordered_map<std::string, std::string> code_constants;
std::unordered_map<std::string, std::string> code_functions;

void code_init(void) {
	code << "\tMOVE 40000, R7" << std::endl;
	code << "\tSUB R7, %D 8, R2" << std::endl;
	code << "\tCALL F0" << std::endl;
	code << "\tHALT" << std::endl;
	// code.close();
}
void save_context(void) {
	code << "\tPUSH R0" << std::endl;
	code << "\tPUSH R1" << std::endl;
	code << "\tPUSH R2" << std::endl;
	code << "\tPUSH R3" << std::endl;
	code << "\tPUSH R4" << std::endl;
	code << "\tPUSH R5" << std::endl;
}
void refresh_context(void) {
	code << "\tPOP R5" << std::endl;
	code << "\tPOP R4" << std::endl;
	code << "\tPOP R3" << std::endl;
	code << "\tPOP R2" << std::endl;
	code << "\tPOP R1" << std::endl;
	code << "\tPOP R0" << std::endl;
}

void fn_def(std::string name) { code << "F" << name << std::endl; }

void return_sp(void) {
	code << "\tMOVE R2, R7" << std::endl;
	code << "\tRET" << std::endl;
}

void load_ret_val(void) { code << "\tPOP R6" << std::endl; }

void unary_operation(int8_t op) {
	if (op == -1) {
		code << "\tPOP R0" << std::endl;
		code << "\tSUB R0, 1, RO" << std::endl;
		code << "\tPUSH R0" << std::endl;
	} else {
		code << "\tPOP R0" << std::endl;
		code << "\tADD R0, 1, RO" << std::endl;
		code << "\tPUSH R0" << std::endl;
	}
}

void binary_operation(int8_t op) {
	// Not commutative
	if (op == -1) {
		code << "\tPOP R0" << std::endl; // second operand
		code << "\tPOP R1" << std::endl; // first operand
		code << "\tSUB R1, R0, RO" << std::endl;
		code << "\tPUSH R0" << std::endl;
	} else {
		code << "\tPOP R0" << std::endl; // second operand
		code << "\tPOP R1" << std::endl; // first operand
		code << "\tADD R1, R0, RO" << std::endl;
		code << "\tPUSH R0" << std::endl;
	}
}

void bitwise_operation(uint8_t op) {
	// Commutative, Associative
	if (op == 1) {
		// AND
		code << "\tPOP R0" << std::endl;
		code << "\tPOP R1" << std::endl;
		code << "\tAND R1, R0, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
	} else if (op == 2) {
		// OR
		code << "\tPOP R0" << std::endl;
		code << "\tPOP R1" << std::endl;
		code << "\tOR R1, R0, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
	} else if (op == 3) {
		// XOR
		code << "\tPOP R0" << std::endl;
		code << "\tPOP R1" << std::endl;
		code << "\tXOR R1, R0, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
	}
}

void logical_operation(uint8_t op) {
	if (op == 1) {
		// &&
		code << "\tPOP R0" << std::endl;
		code << "\tCMP R0, 0" << std::endl;
		code << "\tJP_EQ S" << logical_skip << std::endl;
		code << "\tPOP R1" << std::endl;
		code << "\tCMP R1, 0" << std::endl;
		code << "\tJP_EQ S" << logical_skip + 1 << std::endl;
		code << "S" << logical_skip++ << "\tAND R0, R1, R0 " << std::endl;
		code << "\tPUSH R0" << std::endl;
	} else {
		// ||
		code << "\tPOP R0" << std::endl;
		code << "\tCMP R0, 0" << std::endl;
		code << "\tJP_NE S" << logical_skip << std::endl;
		code << "\tPOP R1" << std::endl;
		code << logical_skip++ << "\tOR R0, R1, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
	}
}

void load_const(std::string var) {
	code << "\tLOAD R0, " << '(' << var << ')' << std::endl;
	code << "\tPUSH R0" << std::endl;
}

void fill_globals(void) {
	for (auto x : code_global_variables) {
		code << x.second.first << "\tDW %D " << x.second.second << std::endl;
	}
}

void fill_consts(void) {
	for (auto x : code_constants) {
		code << x.second << "\tDW %D " << x.first << std::endl;
	}
	code.close();
}
