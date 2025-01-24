#include "code_generator.hpp"
#include <fstream>
#include <iostream>
#include <unordered_map>

int logical_skip = 0;
int local_stack = 0;
std::ofstream code("a.frisc");
std::unordered_map<std::string, std::string>
	code_global_variables; // name -> address
std::unordered_map<std::string, int> global_var_init;
std::unordered_map<std::string, std::pair<std::string, int>> code_global_arrays;
std::string current_global_variable;
std::string current_global_array;
std::unordered_map<int, std::string> code_constants;
std::unordered_map<std::string, std::string> code_functions;
std::multimap<std::string, int> code_local_variables;
std::unordered_multimap<std::string, std::pair<int, int>> code_local_arrays;

void code_init(void) {
	code << "\tMOVE 40000, R7" << std::endl;
	code << "\tSUB R7, %D 4, R2" << std::endl;
	code << "\tCALL F0" << std::endl;
	code << "\tHALT" << std::endl;
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

void fn_def(std::string name, int argc) {
	// code << name << std::endl;
	code << name << "\tSUB R7, %D " << 4 * argc << ", R7" << std::endl;
}

void return_sp(void) {
	code << "\tMOVE R2, R7" << std::endl;
	code << "\tRET" << std::endl;
}

void call_fn(std::string name, size_t argc) {
	int cnt = argc;
	if (argc > 0) {
		do {
			code << "\tADD R7, %D 4, R7" << std::endl;
		} while (argc--);
	}

	code << "\tSUB R7, %D 4, R2" << std::endl;
	code << "\tCALL " << code_functions.at(name) << std::endl;
	while (cnt--) {
		code << "\tSUB R7, %D 4, R7" << std::endl;
	}
}
void load_var(std::string name) {
	// first check local defs!
	// for (auto x : code_local_variables) {
	// 	std::cout << x.first << ' ' << x.second << std::endl;
	// }
	if (code_local_variables.count(name)) {
		auto range = code_local_variables.equal_range(name);
		code << "\tLOAD R0, " << "(R2-0" << std::hex << std::uppercase
			 << std::prev(range.second)->second << ')' << std::endl;
		code << std::dec;
		code << "\tPUSH R0" << std::endl;

	} else if (code_global_variables.count(name)) {
		code << "\tLOAD R0, " << "(" << code_global_variables.at(name) << ')'
			 << std::endl;
		code << "\tPUSH R0" << std::endl;
	}
}

void load_array(std::string name) {
	// saved on stack -> moving up
	if (code_local_arrays.count(name)) {
		auto range = code_local_arrays.equal_range(name);
		int length = prev(range.second)->second.second;
		int loc = prev(range.second)->second.first;
		for (int i = 0; i < length; ++i) {
			code << "\tLOAD R0, " << "(R7 +" << loc - i * 4 << ')' << std::endl;
			code << "\tPUSH R0" << std::endl;
		}
	}
	// saved in memory -> moving down
	else if (code_global_arrays.count(name)) {
		std::string loc = code_global_arrays.at(name).first;
		int length = code_global_arrays.at(name).second;
		code << "\tMOVE " << loc << ", R1" << std::endl;
		for (int i = 0; i < length; ++i) {
			code << "\tLOAD R0, (R1)" << std::endl;
			code << "\tPUSH R0" << std::endl;
			code << "\t ADD R1, %D 4, R1" << std::endl;
		}
	}
}

void store_global(std::string name) {
	code << "\tSTORE R0, " << '(' << code_global_variables.at(name) << ')'
		 << std::endl;
}

void load_ret_val(void) { code << "\tPOP R6" << std::endl; }
void push_ret_val(void) { code << "\tPUSH R6" << std::endl; }

void unary_operation(int8_t op) {
	if (op == -1) {
		code << "\tPOP R0" << std::endl;
		code << "\tSUB R0, 1, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
	} else {
		code << "\tPOP R0" << std::endl;
		code << "\tADD R0, 1, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
	}
}

void binary_operation(int8_t op) {
	// Not commutative
	if (op == -1) {
		code << "\tPOP R0" << std::endl; // second operand
		code << "\tPOP R1" << std::endl; // first operand
		code << "\tSUB R1, R0, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
	} else {
		code << "\tPOP R0" << std::endl; // second operand
		code << "\tPOP R1" << std::endl; // first operand
		code << "\tADD R1, R0, R0" << std::endl;
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
		if (global_var_init.count(x.first)) {
			code << x.second << "\tDW %D " << global_var_init.at(x.first)
				 << std::endl;
		} else {
			code << x.second << std::endl;
		}
	}
}

void fill_consts(void) {
	for (auto x : code_constants) {
		code << x.second << "\tDW %D " << x.first << std::endl;
	}
	code.close();
}
