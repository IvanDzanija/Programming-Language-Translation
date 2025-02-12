#include "code_generator.hpp"
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <unordered_map>

int logical_skip = 0;
int local_stack = 0;
std::ofstream code("a.frisc");
std::unordered_map<std::string, std::string>
	code_global_variables; // name -> address
std::unordered_map<std::string, int> global_var_init;
std::unordered_map<std::string, std::vector<int>> global_arr_init;
std::multimap<std::string, std::pair<std::string, int>> code_global_arrays;
std::unordered_map<int, std::string> code_constants;
std::unordered_map<std::string, std::string> code_functions;
std::multimap<std::string, int> code_local_variables;

std::vector<std::pair<std::string, bool>> increment_after;
std::unordered_map<int, std::vector<std::pair<std::string, int>>>
	for_var_update;
std::unordered_map<std::string, int> function_arrays;

int loop_counter = 0;
int mod_op = 0;
int div_op = 0;
int mul_op = 0;
int for_loop_skip = 0;

void code_init(void) {
	code << std::dec;
	code << "\tMOVE 40000, R7" << std::endl;
	code << "\tSUB R7, %D 4, R2" << std::endl;
	code << "\tCALL F0" << std::endl;
	code << "\tHALT" << std::endl;
}
void save_context(void) {
	code << std::dec;
	code << "\tPUSH R0" << std::endl;
	code << "\tPUSH R1" << std::endl;
	code << "\tPUSH R2" << std::endl;
	code << "\tPUSH R3" << std::endl;
	code << "\tPUSH R4" << std::endl;
	code << "\tPUSH R5" << std::endl;
}
void refresh_context(void) {
	code << std::dec;
	code << "\tPOP R5" << std::endl;
	code << "\tPOP R4" << std::endl;
	code << "\tPOP R3" << std::endl;
	code << "\tPOP R2" << std::endl;
	code << "\tPOP R1" << std::endl;
	code << "\tPOP R0" << std::endl;
}

void fn_def(std::string name, int argc) {
	code << std::dec;
	code << name << "\tSUB R7, %D " << 4 * argc << ", R7" << std::endl;
}

void return_sp(void) {
	code << std::dec;
	code << "\tMOVE R2, R7" << std::endl;
	code << "\tRET" << std::endl;
}

void branch_if() {
	code << std::dec;
	code << "\tPOP R0" << std::endl;
	code << "\tCMP R0,0" << std::endl;
	code << "\tJP_EQ S" << logical_skip << std::endl;
}
void branch_else() {
	code << std::dec;
	code << "S" << logical_skip++ << std::endl;
}
void operation_mod(void) {
	code << std::dec;
	code << "MD" << mod_op << "\tPOP R0" << std::endl; // second operand
	code << "\tPOP R1" << std::endl;				   // first operand
	code << "\tCMP R1, R0" << std::endl;
	code << "\tJP_SLT MD" << mod_op + 1 << std::endl;
	code << "\tSUB R1, R0, R1" << std::endl;
	code << "\tPUSH R1" << std::endl;
	code << "\tPUSH R0" << std::endl;
	code << "\tJP MD" << mod_op << std::endl;
	code << "MD" << ++mod_op << "\tPUSH R1" << std::endl;
	++mod_op;
}

void operation_mul(void) {
	code << std::dec;
	code << "\t MOVE 0, R4" << std::endl;
	code << "ML" << mul_op << "\tPOP R0" << std::endl; // second operand
	code << "\tPOP R1" << std::endl;				   // first operand
	code << "\tCMP R0, 0" << std::endl;
	code << "\tJP_EQ ML" << mul_op + 1 << std::endl;
	code << "\tADD R4, R1, R4" << std::endl;
	code << "\tSUB R0, 1, R0" << std::endl;
	code << "\tPUSH R1" << std::endl;
	code << "\tPUSH R0" << std::endl;
	code << "\tJP ML" << mul_op << std::endl;
	code << "ML" << ++mul_op << "\tPUSH R4" << std::endl;
	++mul_op;
}

void operation_div(void) {
	// we subtract the second operand from the first operand while the first is
	// larger then the second, on each cycle the result in register R4 is
	// incremented by 1

	code << std::dec;
	code << "\t MOVE 0, R4" << std::endl;
	code << "DV" << div_op << "\tPOP R0" << std::endl; // second operand
	code << "\tPOP R1" << std::endl;				   // first operand
	code << "\tCMP R1, R0" << std::endl;
	code << "\tJP_ULT DV" << div_op + 1 << std::endl;
	code << "\tADD R4, 1, R4" << std::endl;
	code << "\tSUB R1, R0, R1" << std::endl;
	code << "\tPUSH R1" << std::endl;
	code << "\tPUSH R0" << std::endl;
	code << "\tJP DV" << div_op << std::endl;
	code << "DV" << ++div_op << "\tPUSH R4" << std::endl;
	++mul_op;
}

void call_fn(std::string name, size_t argc) {
	code << std::dec;
	int cnt = argc;
	if (argc > 0) {
		do {
			code << "\tADD R7, %D 4, R7" << std::endl;
		} while (argc--);
	}

	code << "\tSUB R7, %D 4, R2" << std::endl;
	code << "\tCALL " << code_functions.at(name) << std::endl;
	if (cnt) {
		code << "\tSUB R7, %D 4, R7" << std::endl;
	}
}
void load_var(std::string name) {
	// first check local defs!
	code << std::dec;
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
	code << std::dec;
	code << "\tPOP R0" << std::endl;
	code << "\tSHL R0, 2, R0" << std::endl;
	if (function_arrays.count(name)) {
		int loc = function_arrays.at(name);
		code << "\tLOAD R1, " << "(R2-0" << std::hex << std::uppercase << loc
			 << ')' << std::endl;
		code << "\tADD R0, R1, R1" << std::endl;
		code << "\tLOAD R0, (R1)" << std::endl;
		code << "\tPUSH R0" << std::endl;
	}
	// saved in memory -> moving down
	else if (code_global_arrays.count(name)) {
		auto range = code_global_arrays.equal_range(name);
		std::string loc = std::prev(range.second)->second.first;
		code << "\tMOVE " << loc << ", R1" << std::endl;
		code << "\tADD R0, R1, R1" << std::endl;
		code << "\tLOAD R0, (R1)" << std::endl;
		code << "\tPUSH R0" << std::endl;
	}
}

void store_global_var(std::string name) {
	code << "\tPOP R0" << std::endl;
	code << "\tSTORE R0, " << '(' << code_global_variables.at(name) << ')'
		 << std::endl;
}
void store_local_var(std::string name) {
	code << "\tPOP R0" << std::endl;
	code << "\tSTORE R0, " << "(R2-0" << std::hex << std::uppercase
		 << std::prev(code_local_variables.equal_range(name).second)->second
		 << ')' << std::endl;
	code << std::dec << std::endl;
}
void store_global_arr(std::string name, int index) {
	code << "\tPOP R3" << std::endl;
	code << "\tPOP R0" << std::endl;
	code << "\tSHL R0, 2, R0" << std::endl;
	code << "\tMOVE "
		 << std::prev(code_global_arrays.equal_range(name).second)->second.first
		 << ", R1" << std::endl;
	code << "\tADD R1, R0, R1" << std::endl;
	code << "\tSTORE R3, (R1)" << std::endl;
	code << std::dec;
}

// void store_local_arr(std::string name, int index) {
// 	code << "\tPOP R3" << std::endl;
// 	code << "\tMOVE %D 4, R0" << std::endl;
// 	code << "\tPUSH R0" << std::endl;
// 	operation_mul();
// 	code << "\tPOP R0" << std::endl;
// 	code << "\tSUB R2, %D "
// 		 << std::prev(code_local_arrays.equal_range(name).second)->second.first
// 		 << ", R1" << std::endl;
// 	code << "\tSUB R1, R0, R1" << std::endl;
// 	code << "\tSTORE R3, (R1)" << std::endl;
// 	code << std::dec;
// }

void store_func_arr(std::string name, int index) {
	code << "\tPOP R3" << std::endl; // updated value in register R3

	// next thing on stack should always be the index
	// we take the index and multiply by 4 -> left shift by 2 bits
	code << "\tPOP R0" << std::endl;
	code << "\tSHL R0, 2, R0" << std::endl;
	code << "\tLOAD R1, " << "(R2-0" << std::hex << std::uppercase
		 << function_arrays.at(name) << ')' << std::endl;
	code << std::dec;
	code << "\tADD R1, R0, R1" << std::endl;
	code << "\tSTORE R3, (R1)" << std::endl;
}

void send_arr(std::string name) {
	code << "\tMOVE "
		 << std::prev(code_global_arrays.equal_range(name).second)->second.first
		 << ", R0" << std::endl;
	code << "\tPUSH R0" << std::endl;
}

void equal_comparison(bool eq) {
	code << std::dec;
	if (eq) {
		code << "\tPOP R0" << std::endl;
		code << "\tPOP R1" << std::endl;
		code << "\tCMP R0, R1" << std::endl;
		code << "\tJP_EQ S" << logical_skip << std::endl;
		code << "\tMOVE %D 0, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
		code << "\tJP S" << logical_skip + 1 << std::endl;
		code << "S" << logical_skip++ << std::endl;
		code << "\tMOVE %D 1, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
		code << "S" << logical_skip++ << std::endl;
	} else {
		code << "\tPOP R0" << std::endl;
		code << "\tPOP R1" << std::endl;
		code << "\tCMP R0, R1" << std::endl;
		code << "\tJP_NE S" << logical_skip << std::endl;
		code << "\tMOVE %D 0, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
		code << "\tJP S" << logical_skip + 1 << std::endl;
		code << "S" << logical_skip++ << std::endl;
		code << "\tMOVE %D 1, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
		code << "S" << logical_skip++ << std::endl;
	}
}

void relation_comparison(std::string op) {
	code << std::dec;
	if (op == "OP_LT") {
		code << "\tPOP R0" << std::endl;
		code << "\tPOP R1" << std::endl;
		code << "\tCMP R1, R0" << std::endl;
		code << "\tJP_SLT S" << logical_skip << std::endl;
		code << "\tMOVE %D 0, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
		code << "\tJP S" << logical_skip + 1 << std::endl;
		code << "S" << logical_skip++ << std::endl;
		code << "\tMOVE %D 1, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
		code << "S" << logical_skip++ << std::endl;
	} else if (op == "OP_GT") {
		code << "\tPOP R0" << std::endl;
		code << "\tPOP R1" << std::endl;
		code << "\tCMP R1, R0" << std::endl;
		code << "\tJP_SGT S" << logical_skip << std::endl;
		code << "\tMOVE %D 0, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
		code << "\tJP S" << logical_skip + 1 << std::endl;
		code << "S" << logical_skip++ << std::endl;
		code << "\tMOVE %D 1, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
		code << "S" << logical_skip++ << std::endl;
	} else if (op == "OP_LTE") {
		code << "\tPOP R0" << std::endl;
		code << "\tPOP R1" << std::endl;
		code << "\tCMP R1, R0" << std::endl;
		code << "\tJP_SLE S" << logical_skip << std::endl;
		code << "\tMOVE %D 0, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
		code << "\tJP S" << logical_skip + 1 << std::endl;
		code << "S" << logical_skip++ << std::endl;
		code << "\tMOVE %D 1, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
		code << "S" << logical_skip++ << std::endl;
	} else if (op == "OP_GTE") {
		code << "\tPOP R0" << std::endl;
		code << "\tPOP R1" << std::endl;
		code << "\tCMP R1, R0" << std::endl;
		code << "\tJP_SGE S" << logical_skip << std::endl;
		code << "\tMOVE %D 0, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
		code << "\tJP S" << logical_skip + 1 << std::endl;
		code << "S" << logical_skip++ << std::endl;
		code << "\tMOVE %D 1, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
		code << "S" << logical_skip++ << std::endl;
	}
}

void variable_increment_before(std::string var, bool plus) {
	code << std::dec;
	load_var(var);
	code << "\tPOP R0" << std::endl;
	if (plus) {
		code << "\tADD R0, 1, R0" << std::endl;

		code << "\tPUSH R0" << std::endl;
	} else {
		code << "\tSUB R0, 1, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
	}
	if (code_local_variables.count(var)) {
		store_local_var(var);
	} else if (code_global_variables.count(var)) {
		store_global_var(var);
	}
	load_var(var);
}

void variable_increment_after() {
	code << std::dec;
	for (std::pair<std::string, int> var : increment_after) {
		load_var(var.first);
		if (var.second == false) {
			code << "\tSUB R0, %D 1, R0" << std::endl;
			code << "\tPUSH R0" << std::endl;
		} else {
			code << "\tADD R0, %D 1, R0" << std::endl;
			code << "\tPUSH R0" << std::endl;
		}
		if (code_local_variables.count(var.first)) {
			store_local_var(var.first);
		} else if (code_global_variables.count(var.first)) {
			store_global_var(var.first);
		}
	}
	increment_after.clear();
}

void while_start() { code << "L" << loop_counter << std::endl; }
void while_check() {
	code << "\tPOP R0" << std::endl;
	code << "\tCMP R0, 0" << std::endl;
	code << "\tJP_EQ E" << loop_counter << std::endl;
}
void while_end() {
	code << "\tJP L" << loop_counter << std::endl;
	code << "E" << loop_counter++ << std::endl;
}
void forc_start() {
	code << "L" << loop_counter << std::endl;
	code << "\tMOVE %D 0, R5" << std::endl;
}

void forc_check() {
	code << "\tPOP R0" << std::endl;
	code << "\tCMP R0, 0" << std::endl;
	code << "\tJP_EQ E" << loop_counter << std::endl;
}
void forc_skip_first() {
	code << "I" << for_loop_skip << std::endl;
	code << "\tCMP R5, 0" << std::endl;
	code << "\tJP_EQ K" << for_loop_skip << std::endl;
}
void forc_skip_second() {
	code << "\tCMP R5, 1" << std::endl;
	code << "\tJP_EQ J" << for_loop_skip << std::endl;
	code << "\tCMP R5, 2" << std::endl;
	code << "\tJP_EQ AE" << for_loop_skip << std::endl;
	code << "K" << for_loop_skip << std::endl;
}

void forc_end() {
	code << "\tMOVE %D 1, R5" << std::endl;
	code << "\tJP I" << for_loop_skip << std::endl;
	code << "J" << for_loop_skip << "\tJP L" << loop_counter << std::endl;
	code << "E" << loop_counter++ << "\tMOVE %D 2, R5" << std::endl;
	code << "\tJP I" << for_loop_skip << std::endl;
	code << "AE" << for_loop_skip++ << std::endl;
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
void logical_start(uint8_t op) {
	if (op == 1) {
		// &&
		code << "\tPOP R0" << std::endl;
		code << "\tCMP R0, 0" << std::endl;
		code << "\tJP_EQ S" << logical_skip << std::endl;
		code << "\tPUSH R0" << std::endl;
	} else {
		// ||
		code << "\tPOP R0" << std::endl;
		code << "\tCMP R0, 0" << std::endl;
		code << "\tJP_NE S" << logical_skip << std::endl;
		code << "\tPUSH R0" << std::endl;
	}
}
void logical_check(uint8_t op) {
	if (op == 1) {
		// &&
		code << "\tPOP R0" << std::endl;
		code << "\tPOP R1" << std::endl;
		code << "\tAND R0, R1, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;

	} else {
		// ||
		code << "\tPOP R0" << std::endl;
		code << "\tPOP R1" << std::endl;
		code << "\tOR R0, R1, R0" << std::endl;
		code << "\tPUSH R0" << std::endl;
	}
}

void logical_end(void) { code << "S" << logical_skip++ << std::endl; }

void load_const(std::string var) {
	code << "\tLOAD R0, " << '(' << var << ')' << std::endl;
	code << "\tPUSH R0" << std::endl;
}

void fill_globals(void) {
	code << std::dec;
	for (auto x : code_global_variables) {
		if (global_var_init.count(x.first)) {
			code << x.second << "\tDW %D " << global_var_init.at(x.first)
				 << std::endl;
		} else {
			code << x.second << std::endl;
		}
	}
	for (auto x : code_global_arrays) {
		if (global_arr_init.count(x.first)) {
			if (global_arr_init.at(x.first).size() > 0) {
				code << x.second.first << "\tDW %D "
					 << global_arr_init.at(x.first).at(0) << std::endl;
			} else {
				code << x.second.first << std::endl;
			}
			for (size_t i = 1; i < x.second.second; ++i) {
				if (global_arr_init.at(x.first).size() > i) {
					code << "\tDW %D " << global_arr_init.at(x.first).at(i)
						 << std::endl;
				} else {
					code << "\tDW %D  0" << std::endl;
				}
			}
		} else {
			code << x.second.first << "\tDW %D 0" << std::endl;
			for (size_t i = 1; i < x.second.second; ++i) {
				code << "\tDW %D 0" << std::endl;
			}
		}
	}
}

void fill_consts(void) {
	code << std::dec;
	for (auto x : code_constants) {
		code << x.second << "\tDW %D " << x.first << std::endl;
	}
	code.close();
}
