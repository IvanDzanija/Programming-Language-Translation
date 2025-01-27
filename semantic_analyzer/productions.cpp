#include "code_generator.hpp"
#include "helper_functions.hpp"
#include "productions.hpp"
#include <memory>
#include <stack>
#include <string>

// global variables;
int loop_depth = 0; // tracks the depth of the loop
std::vector<std::string> current_function_argument_types;
std::vector<std::string> current_function_argument_names;
std::string current_function_return_type = "";
std::unordered_set<std::string> defined_functions;
std::multiset<std::string> declared_functions;
std::unordered_map<std::string,
				   std::pair<std::string, std::vector<std::string>>>
	global_declared_functions;

std::unordered_set<std::string> local_names;
std::unordered_multimap<std::string, std::pair<int, std::string>>
	available_variables;
std::unordered_multimap<std::string,
						std::pair<int, std::pair<std::string, int>>>
	available_arrays;
std::unordered_multimap<
	std::string,
	std::pair<int, std::pair<std::string, std::vector<std::string>>>>
	available_functions;
bool main_defined = false;
int block_count = 0;
bool is_minus = false;
bool from_function = false;

std::string current_global_variable;
std::string current_global_array;

std::stack<bool> calling_function;
std::stack<std::string> fn_call_name;

std::string indexed_array = "";
bool indexing_array = false;
std::stack<bool> updating;
std::stack<bool> updating_vars;
std::stack<bool> updating_arrs;
std::vector<std::string> vars_to_update;
std::vector<std::pair<std::string, int>> arrs_to_update;
bool sending_params;

bool incrementing = false;

std::string incrementing_after = "";
int command_count = 0;
std::string inc_op = "";

int primarni_izraz(std::shared_ptr<Node> root) {
	// <primarni_izraz> ::= IDN
	// tip ← IDN.tip
	// l-izraz ← IDN.l-izraz
	if (root->children.size() == 1 && root->children.at(0)->symbol == "IDN") {
		// 1. IDN.ime je deklarirano
		// check if its a variable
		int deepest_block = -1;
		if (available_variables.count(root->children.at(0)->value)) {
			auto range =
				available_variables.equal_range(root->children.at(0)->value);
			std::string current_type = "";
			for (auto it = range.first; it != range.second;
				 it = std::next(it)) {
				if (it->second.first > deepest_block) {
					deepest_block = it->second.first;
					current_type = it->second.second;
				}
			}
			root->type = current_type;
			if (is_const(current_type)) {
				root->lhs = false;
			} else {
				root->lhs = true;
			}

			if (code_local_variables.count(root->children.at(0)->value)) {
				if (incrementing) {
					variable_increment_before(root->children.at(0)->value,
											  inc_op == "OP_INC");
					incrementing = false;
					inc_op = "";
				} else {
					if (code_local_variables.count(
							root->children.at(0)->value)) {
						load_var(root->children.at(0)->value);
						if (!updating_vars.empty()) {
							vars_to_update.push_back(
								root->children.at(0)->value);
							updating_vars.pop();
						}
						if (incrementing_after != "") {
							increment_after.push_back(
								make_pair(root->children.at(0)->value,
										  incrementing_after == "OP_INC"));
							incrementing_after = "";
						}
					}
				}
			} else if (function_arrays.count(root->children.at(0)->value)) {
				if (indexing_array) {
					indexed_array = root->children.at(0)->value;
				}
				if (!updating_arrs.empty()) {
					arrs_to_update.push_back(
						std::make_pair(root->children.at(0)->value, 0));
					updating_arrs.pop();
				}
			} else if (code_global_variables.count(
						   root->children.at(0)->value)) {
				if (incrementing) {
					variable_increment_before(root->children.at(0)->value,
											  inc_op == "OP_INC");
					incrementing = false;
					inc_op = "";
				} else {
					if (code_global_variables.count(
							root->children.at(0)->value)) {
						load_var(root->children.at(0)->value);
						if (!updating_vars.empty()) {
							vars_to_update.push_back(
								root->children.at(0)->value);
							updating_vars.pop();
						}
						if (incrementing_after != "") {
							increment_after.push_back(
								make_pair(root->children.at(0)->value,
										  incrementing_after == "OP_INC"));
							incrementing_after = "";
						}
					}
				}
			}
		}

		// check if its a function
		if (available_functions.count(root->children.at(0)->value)) {
			auto range =
				available_functions.equal_range(root->children.at(0)->value);
			std::string current_type = "";
			bool has_arguments = false;
			std::vector<std::string> arguments;
			bool convert = false;
			for (auto it = range.first; it != range.second;
				 it = std::next(it)) {
				if (it->second.first > deepest_block) {
					deepest_block = it->second.first;
					current_type = it->second.second.first;
					convert = true;
					if (it->second.second.second.size() > 0) {
						has_arguments = true;
						arguments = it->second.second.second;
					}
				}
			}
			if (!calling_function.empty()) {
				fn_call_name.push(root->children.at(0)->value);
			}
			if (convert) {
				root->type = "funkcija(";
				if (has_arguments) {
					root->type += "params -> ";
					root->arg_types = arguments;
					root->param_types = arguments;
				} else {
					root->type += "void -> ";
				}
				root->type += "pov)";
				root->return_type = current_type;
				root->lhs = false;
			}
		}
		// check if its an array
		if (available_arrays.count(root->children.at(0)->value)) {
			auto range =
				available_arrays.equal_range(root->children.at(0)->value);
			std::string current_type = "";
			bool convert = false;
			for (auto it = range.first; it != range.second;
				 it = std::next(it)) {
				if (it->second.first > deepest_block) {
					convert = true;
					deepest_block = it->second.first;
					current_type = it->second.second.first;
				}
			}
			if (convert) {
				root->type = current_type;
				root->lhs = false;
			}
			if (indexing_array) {
				indexed_array = root->children.at(0)->value;
			} else if (indexing_array == false && sending_params) {
				send_arr(root->children.at(0)->value);
			}

			if (code_global_arrays.count(root->children.at(0)->value)) {
				if (!updating_arrs.empty()) {
					arrs_to_update.push_back(
						std::make_pair(root->children.at(0)->value, 0));
					updating_arrs.pop();
				}
			}

		} else if (deepest_block == -1) {
			return root->semantic_error();
		}

	}
	// <primarni_izraz> ::= BROJ
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 1 &&
			 root->children.at(0)->symbol == "BROJ") {
		// 1. vrijednost je u rasponu tipa int
		std::string current_value = "";
		if (is_minus) {
			current_value += "-";
		}
		is_minus = false;
		current_value += root->children.at(0)->value;
		int temp = 0;
		try {
			temp = std::stoi(current_value, nullptr, 0);
		} catch (const std::out_of_range &oor) {
			return root->semantic_error();
		}
		root->type = "int";
		root->lhs = false;
		if (!code_constants.count(temp)) {
			std::string next_name = "C";
			next_name += std::to_string(code_constants.size());
			code_constants.emplace(std::make_pair(temp, next_name));
		}
		load_const(code_constants.at(temp));
		if (current_global_variable != "") {
			if (!global_var_init.count(current_global_variable)) {
				global_var_init.emplace(
					make_pair(current_global_variable, temp));
			}
			current_global_variable = "";
		}
		if (current_global_array != "") {
			if (!global_arr_init.count(current_global_array)) {
				global_arr_init.emplace(
					make_pair(current_global_array, std::vector<int>(1, temp)));
			} else {
				global_arr_init.at(current_global_array).push_back(temp);
			}
		}

	}
	// <primarni_izraz> ::= ZNAK
	// tip ← char
	// l-izraz ← 0
	else if (root->children.size() == 1 &&
			 root->children.at(0)->symbol == "ZNAK") {
		// 1. znak je ispravan po 4.3.2
		if (root->children.at(0)->value.size() > 4) {
			return root->semantic_error();
		} else if (root->children.at(0)->value.at(0) != '\'' ||
				   root->children.at(0)->value.at(
					   root->children.at(0)->value.size() - 1) != '\'') {
			return root->semantic_error();
		} else {
			if (!accepted_char(root->children.at(0)->value.substr(
					1, root->children.at(0)->value.size() - 2))) {

				return root->semantic_error();
			} else {
				root->type = "char";
				root->lhs = false;

				int cval = (int)root->children.at(0)->value.at(
					root->children.at(0)->value.size() - 2);
				if (!code_constants.count(cval)) {
					std::string next_name = "C";
					next_name += std::to_string(code_constants.size());
					code_constants.emplace(std::make_pair(cval, next_name));
				}
				if (current_global_variable != "") {
					if (!global_var_init.count(current_global_variable)) {
						global_var_init.emplace(
							make_pair(current_global_variable, cval));
					}
					current_global_variable = "";
				}
				if (current_global_array != "") {
					if (!global_arr_init.count(current_global_array)) {
						global_arr_init.emplace(make_pair(
							current_global_array, std::vector<int>(1, cval)));
					} else {
						global_arr_init.at(current_global_array)
							.push_back(cval);
					}
				}
			}
		}
	}
	// <primarni_izraz> ::= NIZ_ZNAKOVA
	// tip ← niz(const(char))
	// l-izraz ← 0
	else if (root->children.size() == 1 &&
			 root->children.at(0)->symbol == "NIZ_ZNAKOVA") {
		// 1. konstantni niz znakova je ispravan po 4.3.2
		if (root->children.at(0)->value.at(0) != '"' ||
			root->children.at(0)->value.at(root->children.at(0)->value.size() -
										   1) != '"' ||
			root->children.at(0)->value.at(root->children.at(0)->value.size() -
										   2) == '\\') {
			return root->semantic_error();
		} else {
			for (size_t i = 1; i < root->children.at(0)->value.size() - 2;
				 ++i) {
				std::string to_check = "";
				to_check += (root->children.at(0)->value.at(i));
				to_check += (root->children.at(0)->value.at(i + 1));
				if (to_check.at(0) == '\\') {
					if (!accepted_char(to_check)) {
						return root->semantic_error();
					}
				}
			}
		}
		root->type = "niz(const(char))";
		root->lhs = false;
	}
	// 	<primarni_izraz> ::= L_ZAGRADA <izraz> D_ZAGRADA
	// tip ← <izraz>.tip
	// l-izraz ← <izraz>.l-izraz
	else if (root->children.size() == 3 &&
			 root->children.at(0)->symbol == "L_ZAGRADA" &&
			 root->children.at(2)->symbol == "D_ZAGRADA" &&
			 root->children.at(1)->symbol == "<izraz>") {
		// 1. provjeri(<izraz>)
		if (izraz(root->children.at(1))) {
			return 1;
		} else {
			root->type = root->children.at(1)->type;
			root->lhs = root->children.at(1)->lhs;
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int postfiks_izraz(std::shared_ptr<Node> root) {
	// <postfiks_izraz> ::= <primarni_izraz>
	// tip ← <primarni_izraz>.tip
	// l-izraz ← <primarni_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<primarni_izraz>") {
		// 1. provjeri(<primarni_izraz>)
		if (!updating.empty() && indexing_array == true) {
			updating_arrs.push(true);
			indexing_array = false;
		} else if (!updating.empty()) {
			updating_vars.push(true);
		}
		if (primarni_izraz(root->children.at(0))) {
			return 1;
		} else {
			root->return_type = root->children.at(0)->return_type;
			root->param_types = root->children.at(0)->param_types;
			root->type = root->children.at(0)->type;
			root->lhs = root->children.at(0)->lhs;
		}
	}
	// <postfiks_izraz> ::= <postfiks_izraz> L_UGL_ZAGRADA <izraz>
	// D_UGL_ZAGRADA
	// tip ← X
	// l-izraz ← X ̸= const(T)
	else if (root->children.size() == 4 &&
			 root->children.at(0)->symbol == "<postfiks_izraz>" &&
			 root->children.at(1)->symbol == "L_UGL_ZAGRADA" &&
			 root->children.at(2)->symbol == "<izraz>" &&
			 root->children.at(3)->symbol == "D_UGL_ZAGRADA") {
		// 1. provjeri(<postfiks_izraz>)
		// 2. <postfiks_izraz>.tip = niz(X)
		// 3. provjeri(<izraz>)
		// 4. <izraz>.tip ∼ int
		indexing_array = true;
		if (postfiks_izraz(root->children.at(0))) {
			return 1;
		} else {
			if (!is_array(root->children.at(0)->type)) {
				return root->semantic_error();
			} else {
				indexing_array = false;
				if (izraz(root->children.at(2))) {
					return 1;
				} else {
					if (!implicit_conversion(root->children.at(2)->type,
											 "int")) {
						return root->semantic_error();
					} else {
						std::string X =
							remove_array(root->children.at(0)->type);
						root->type = X;
						root->lhs = !(is_const(X));
						if (indexed_array != "") {
							load_array(indexed_array);
						}
						indexing_array = false;
					}
				}
			}
		}

	}
	// <postfiks_izraz>:: = <postfiks_izraz> L_ZAGRADA D_ZAGRADA
	// 	tip ← pov
	// 	l-izraz ← 0
	else if (root->children.size() == 3 &&
			 root->children.at(0)->symbol == "<postfiks_izraz>" &&
			 root->children.at(1)->symbol == "L_ZAGRADA" &&
			 root->children.at(2)->symbol == "D_ZAGRADA") {
		// 1. provjeri(<postfiks_izraz>)
		// 2. < postfiks_izraz >.tip = funkcija(void → pov)
		calling_function.push(true);
		save_context();
		if (postfiks_izraz(root->children.at(0))) {
			return 1;
		} else {
			calling_function.pop();
			if (root->children.at(0)->type != "funkcija(void -> pov)") {
				return root->semantic_error();
			} else {
				root->type = root->children.at(0)->return_type;

				root->lhs = false;
				std::string fn_name = fn_call_name.top();
				fn_call_name.pop();
				call_fn(fn_name, 0);
				refresh_context();
				push_ret_val();
			}
		}
	}
	// <postfiks_izraz> ::= <postfiks_izraz> L_ZAGRADA <lista_argumenata>
	// D_ZAGRADA
	// tip ← pov
	// l-izraz ← 0
	else if (root->children.size() == 4 &&
			 root->children.at(0)->symbol == "<postfiks_izraz>" &&
			 root->children.at(1)->symbol == "L_ZAGRADA" &&
			 root->children.at(3)->symbol == "D_ZAGRADA" &&
			 root->children.at(2)->symbol == "<lista_argumenata>") {
		// 1. provjeri(<postfiks_izraz>)
		// 2. provjeri(<lista_argumenata>)
		// 3. <postfiks_izraz>.tip = funkcija(params → pov) i redom po
		// elementima arg-tip iz <lista_argumenata>.tipovi i param-tip iz
		// params vrijedi arg-tip ∼ param-tip
		calling_function.push(true);
		save_context();
		if (postfiks_izraz(root->children.at(0))) {
			return 1;
		} else {
			sending_params = true;
			if (lista_argumenata(root->children.at(2))) {
				return 1;
			} else {
				if (!same_arguments(root->children.at(2)->arg_types,
									root->children.at(0)->param_types)) {
					return root->semantic_error();
				} else {
					root->type = root->children.at(0)->return_type;
					root->lhs = false;

					std::string fn_name = fn_call_name.top();
					fn_call_name.pop();
					call_fn(fn_name, root->children.at(2)->arg_types.size());
					refresh_context();
					push_ret_val();
				}
			}
			sending_params = false;
		}

	}
	// <postfiks_izraz> ::= <postfiks_izraz> (OP_INC | OP_DEC)
	//  tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 2 &&
			 (root->children.at(1)->symbol == "OP_INC" ||
			  root->children.at(1)->symbol == "OP_DEC") &&
			 root->children.at(0)->symbol == "<postfiks_izraz>") {
		// 1. provjeri(<postfiks_izraz>)
		// 2. <postfiks_izraz>.l-izraz = 1 i <postfiks_izraz>.tip ∼ int
		incrementing_after = root->children.at(1)->symbol;
		if (postfiks_izraz(root->children.at(0))) {
			return 1;
		} else {
			if (!root->children.at(0)->lhs) {
				return root->semantic_error();
			} else if (!implicit_conversion(root->children.at(0)->type,
											"int")) {
				return root->semantic_error();
			} else {
				root->type = "int";
				root->lhs = false;
				incrementing_after = "";
			}
		}

	} else {
		return root->semantic_error();
	}
	return 0;
}

int lista_argumenata(std::shared_ptr<Node> root) {
	// <lista_argumenata> ::= <izraz_pridruzivanja>
	// tipovi ← [<izraz_pridruzivanja>.tip ]
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<izraz_pridruzivanja>") {
		// 1. provjeri(<izraz_pridruzivanja>)
		if (izraz_pridruzivanja(root->children.at(0))) {
			return 1;
		} else {
			root->arg_types.push_back(root->children.at(0)->type);
			root->arg_names.push_back(root->children.at(0)->name);
		}
	}
	// <lista_argumenata> ::= <lista_argumenata> ZAREZ <izraz_pridruzivanja>
	// tipovi ← <lista_argumenata>.tipovi + [ <izraz_pridruzivanja>.tip ]
	else if (root->children.size() == 3 &&
			 root->children.at(0)->symbol == "<lista_argumenata>" &&
			 root->children.at(1)->symbol == "ZAREZ" &&
			 root->children.at(2)->symbol == "<izraz_pridruzivanja>") {
		// 1. provjeri(<lista_argumenata>)
		// 2. provjeri(<izraz_pridruzivanja>)
		if (lista_argumenata(root->children.at(0))) {
			return 1;
		} else if (izraz_pridruzivanja(root->children.at(2))) {
			return 1;
		} else {
			root->arg_types = root->children.at(0)->arg_types;
			root->arg_names = root->children.at(0)->arg_names;
			root->arg_names.push_back(root->children.at(0)->name);
			root->arg_types.push_back(root->children.at(2)->type);
		}

	} else {
		return root->semantic_error();
	}
	return 0;
}

int unarni_izraz(std::shared_ptr<Node> root) {
	// 	<unarni_izraz> ::= <postfiks_izraz>
	// tip ← <postfiks_izraz>.tip
	// l-izraz ← <postfiks_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<postfiks_izraz>") {
		// 1. provjeri(<postfiks_izraz>)
		if (postfiks_izraz(root->children.at(0))) {
			return 1;
		} else {
			root->type = root->children.at(0)->type;
			root->lhs = root->children.at(0)->lhs;
		}
	}
	// <unarni_izraz> ::= (OP_INC | OP_DEC) <unarni_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 2 &&
			 (root->children.at(0)->symbol == "OP_INC" ||
			  root->children.at(0)->symbol == "OP_DEC") &&
			 root->children.at(1)->symbol == "<unarni_izraz>") {
		// 1. provjeri(<unarni_izraz>)
		// 2. <unarni_izraz>.l-izraz = 1 i <unarni_izraz>.tip ∼ int
		incrementing = true;
		inc_op = root->children.at(0)->symbol;
		if (unarni_izraz(root->children.at(1))) {
			return 1;
		} else {

			if (!root->children.at(1)->lhs) {
				return root->semantic_error();
			} else if (!implicit_conversion(root->children.at(1)->type,
											"int")) {
				return root->semantic_error();
			} else {
				root->type = "int";
				root->lhs = false;
				auto it = root->children.at(1);
				incrementing = false;
				inc_op = "";
			}
		}

	}
	// <unarni_izraz> ::= <unarni_operator> <cast_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 2 &&
			 (root->children.at(0)->symbol == "<unarni_operator>" &&
			  root->children.at(1)->symbol == "<cast_izraz>")) {
		// 1. provjeri(<cast_izraz>)
		// 2. <cast_izraz>.tip ∼ int
		if (unarni_operator(root->children.at(0))) {
			return 1;
		} else if (cast_izraz(root->children.at(1))) {
			return 1;
		} else {
			if (!implicit_conversion(root->children.at(1)->type, "int")) {
				return root->semantic_error();
			} else {
				root->type = "int";

				root->lhs = false;
				is_minus = false;
			}
		}

	} else {
		return root->semantic_error();
	}
	return 0;
}

int unarni_operator(std::shared_ptr<Node> root) {
	if (root->children.at(0)->symbol == "MINUS") {
		is_minus = true;
	}
	return 0;
}

int cast_izraz(std::shared_ptr<Node> root) {
	// <cast_izraz> ::= <unarni_izraz>
	// tip ← <unarni_izraz>.tip
	// l-izraz ← <unarni_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<unarni_izraz>") {
		// 1. provjeri(<unarni_izraz>)
		if (unarni_izraz(root->children.at(0))) {
			return 1;
		} else {
			root->type = root->children.at(0)->type;
			root->lhs = root->children.at(0)->lhs;
		}
	}
	// <cast_izraz> ::= L_ZAGRADA <ime_tipa> D_ZAGRADA <cast_izraz>
	// tip ← <ime_tipa>.tip
	// l-izraz ← 0
	else if (root->children.size() == 4 &&
			 root->children.at(0)->symbol == "L_ZAGRADA" &&
			 root->children.at(2)->symbol == "D_ZAGRADA" &&
			 root->children.at(1)->symbol == "<ime_tipa>" &&
			 root->children.at(3)->symbol == "<cast_izraz>") {
		// 1. provjeri(<ime_tipa>)
		// 2. provjeri(<cast_izraz>)
		// 3. <cast_izraz>.tip se moze pretvoriti u <ime_tipa>.tip
		if (ime_tipa(root->children.at(1))) {
			return 1;
		} else if (cast_izraz(root->children.at(3))) {
			return 1;
		} else {
			if (!explicit_conversion(root->children.at(3)->type,
									 root->children.at(1)->type)) {
				return root->semantic_error();
			} else {
				root->type = root->children.at(1)->type;
				root->lhs = false;
			}
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int ime_tipa(std::shared_ptr<Node> root) {
	// <ime_tipa> ::= <specifikator_tipa>
	// tip ← <specifikator_tipa>.tip
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<specifikator_tipa>") {
		// 1. provjeri(<specifikator_tipa>)
		if (specifikator_tipa(root->children.at(0))) {
			return 1;
		} else {
			root->type = root->children.at(0)->type;
		}
	}
	// <ime_tipa> ::= KR_CONST <specifikator_tipa>
	//  tip ← const(<specifikator_tipa>.tip)
	else if (root->children.size() == 2 &&
			 root->children.at(0)->symbol == "KR_CONST" &&
			 root->children.at(1)->symbol == "<specifikator_tipa>") {
		// 1. provjeri(<specifikator_tipa>)
		// 2. <specifikator_tipa>.tip != void
		if (specifikator_tipa(root->children.at(1))) {
			return 1;
		} else {
			if (root->children.at(1)->type == "void") {
				return root->semantic_error();
			} else {
				std::string X = "const(";
				X += root->children.at(1)->type;
				X += ')';
				root->type = X;
			}
		}

	} else {
		return root->semantic_error();
	}
	return 0;
}

int specifikator_tipa(std::shared_ptr<Node> root) {
	if (root->children.size() == 1) {
		// <specifikator_tipa> ::= KR_VOID
		// tip ← void
		if (root->children.at(0)->symbol == "KR_VOID") {
			root->type = "void";
		}
		// <specifikator_tipa> ::= KR_CHAR
		// tip ← char
		else if (root->children.at(0)->symbol == "KR_CHAR") {
			root->type = "char";
		}
		// <specifikator_tipa> ::= KR_INT
		// tip ← int
		else if (root->children.at(0)->symbol == "KR_INT") {
			root->type = "int";
		} else {
			return root->semantic_error();
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int multiplikativni_izraz(std::shared_ptr<Node> root) {
	// <multiplikativni_izraz> ::= <cast_izraz>
	// tip ← <cast_izraz>.tip
	// l-izraz ← <cast_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<cast_izraz>") {
		// 1. provjeri(<cast_izraz>)
		if (cast_izraz(root->children.at(0))) {
			return 1;
		} else {
			root->type = root->children.at(0)->type;
			root->lhs = root->children.at(0)->lhs;
		}
	}
	// <multiplikativni_izraz> ::= <multiplikativni_izraz> (OP_PUTA |
	// OP_DIJELI | OP_MOD) <cast_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 (root->children.at(1)->symbol == "OP_PUTA" ||
			  root->children.at(1)->symbol == "OP_DIJELI" ||
			  root->children.at(1)->symbol == "OP_MOD") &&
			 root->children.at(2)->symbol == "<cast_izraz>" &&
			 root->children.at(0)->symbol == "<multiplikativni_izraz>") {
		// 1. provjeri(<multiplikativni_izraz>)
		// 2. <multiplikativni_izraz>.tip ∼ int
		// 3. provjeri(<cast_izraz>)
		// 4. <cast_izraz>.tip ∼ int
		if (multiplikativni_izraz(root->children.at(0))) {
			return 1;
		} else if (!implicit_conversion(root->children.at(0)->type, "int")) {
			return root->semantic_error();
		} else {
			if (cast_izraz(root->children.at(2))) {
				return 1;
			} else if (!implicit_conversion(root->children.at(2)->type,
											"int")) {
				return root->semantic_error();
			} else {
				root->type = "int";
				root->lhs = false;
				if (root->children.at(1)->symbol == "OP_MOD") {
					operation_mod();
				} else if (root->children.at(1)->symbol == "OP_PUTA") {
					operation_mul();
				} else {
					operation_div();
				}
			}
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int aditivni_izraz(std::shared_ptr<Node> root) {
	// 	<aditivni_izraz> ::= <multiplikativni_izraz>
	// tip ←<multiplikativni_izraz>.tip
	// l-izraz ← <multiplikativni_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<multiplikativni_izraz>") {
		// 1. provjeri(<multiplikativni_izraz>)
		if (multiplikativni_izraz(root->children.at(0))) {
			return 1;
		} else {
			root->type = root->children.at(0)->type;
			root->lhs = root->children.at(0)->lhs;
		}
	}
	// <aditivni_izraz> ::= <aditivni_izraz> (PLUS | MINUS)
	// <multiplikativni_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 root->children.at(0)->symbol == "<aditivni_izraz>" &&
			 root->children.at(2)->symbol == "<multiplikativni_izraz>" &&
			 (root->children.at(1)->symbol == "PLUS" ||
			  root->children.at(1)->symbol == "MINUS")) {
		// 1. provjeri(<aditivni_izraz>)
		// 2. <aditivni_izraz>.tip ∼ int
		// 3. provjeri(<multiplikativni_izraz>)
		// 4. <multiplikativni_izraz>.tip ∼ int
		if (aditivni_izraz(root->children.at(0))) {
			return 1;
		} else {
			if (!implicit_conversion(root->children.at(0)->type, "int")) {
				return root->semantic_error();
			} else {
				if (multiplikativni_izraz(root->children.at(2))) {
					return 1;
				} else {
					if (!implicit_conversion(root->children.at(2)->type,
											 "int")) {
						return root->semantic_error();
					} else {
						root->type = "int";
						root->lhs = false;

						if (root->children.at(1)->symbol == "PLUS") {
							binary_operation(1);
						} else {
							binary_operation(-1);
						}
					}
				}
			}
		}

	} else {
		return root->semantic_error();
	}
	return 0;
}

int odnosni_izraz(std::shared_ptr<Node> root) {
	// <odnosni_izraz> ::= <aditivni_izraz>
	// tip ← <aditivni_izraz>.tip
	// l-izraz ← <aditivni_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<aditivni_izraz>") {
		// 1. provjeri(<aditivni_izraz>)
		if (aditivni_izraz(root->children.at(0))) {
			return 1;
		} else {
			root->type = root->children.at(0)->type;
			root->lhs = root->children.at(0)->lhs;
		}
	}
	// <odnosni_izraz> ::= <odnosni_izraz> (OP_LT | OP_GT | OP_LTE | OP_GTE)
	// <aditivni_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 root->children.at(0)->symbol == "<odnosni_izraz>" &&
			 root->children.at(2)->symbol == "<aditivni_izraz>" &&
			 (root->children.at(1)->symbol == "OP_LT" ||
			  root->children.at(1)->symbol == "OP_GT" ||
			  root->children.at(1)->symbol == "OP_LTE" ||
			  root->children.at(1)->symbol == "OP_GTE")) {
		// 1. provjeri(<odnosni_izraz>)
		// 2. <odnosni_izraz>.tip ∼ int
		// 3. provjeri(<aditivni_izraz>)
		// 4. <aditivni_izraz>.tip ∼ int
		if (odnosni_izraz(root->children.at(0))) {
			return 1;
		} else if (!implicit_conversion(root->children.at(0)->type, "int")) {
			return root->semantic_error();
		} else {
			if (aditivni_izraz(root->children.at(2))) {
				return 1;
			} else if (!implicit_conversion(root->children.at(2)->type,
											"int")) {
				return root->semantic_error();
			} else {
				root->type = "int";
				root->lhs = false;
				relation_comparison(root->children.at(1)->symbol);
			}
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int jednakosni_izraz(std::shared_ptr<Node> root) {
	// <jednakosni_izraz> ::= <odnosni_izraz>
	// tip ← <odnosni_izraz>.tip
	// l-izraz ← <odnosni_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<odnosni_izraz>") {
		// 1. provjeri(<odnosni_izraz>)
		if (odnosni_izraz(root->children.at(0))) {
			return 1;
		} else {
			root->type = root->children.at(0)->type;
			root->lhs = root->children.at(0)->lhs;
		}
	}
	// <jednakosni_izraz> ::= <jednakosni_izraz> (OP_EQ | OP_NEQ)
	// <odnosni_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 root->children.at(0)->symbol == "<jednakosni_izraz>" &&
			 root->children.at(2)->symbol == "<odnosni_izraz>" &&
			 (root->children.at(1)->symbol == "OP_EQ" ||
			  root->children.at(1)->symbol == "OP_NEQ")) {
		// 1. provjeri(<jednakosni_izraz>)
		// 2. <jednakosni_izraz>.tip ∼ int
		// 3. provjeri(<odnosni_izraz>)
		// 4. <odnosni_izraz>.tip ∼ int
		if (jednakosni_izraz(root->children.at(0))) {
			return 1;
		} else if (!implicit_conversion(root->children.at(0)->type, "int")) {
			return root->semantic_error();
		} else {
			if (odnosni_izraz(root->children.at(2))) {
				return 1;
			} else if (!implicit_conversion(root->children.at(2)->type,
											"int")) {
				return root->semantic_error();
			} else {
				root->type = "int";
				root->lhs = false;
				equal_comparison(root->children.at(1)->symbol == "OP_EQ");
			}
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int bin_i_izraz(std::shared_ptr<Node> root) {
	// <bin_i_izraz> ::= <jednakosni_izraz>
	// tip ← <jednakosni_izraz>.tip
	// l-izraz ← <jednakosni_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<jednakosni_izraz>") {
		// 1. provjeri(<jednakosni_izraz>)
		if (jednakosni_izraz(root->children.at(0))) {
			return 1;
		} else {
			root->type = root->children.at(0)->type;
			root->lhs = root->children.at(0)->lhs;
		}
	}
	// <bin_i_izraz> ::= <bin_i_izraz> OP_BIN_I <jednakosni_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 root->children.at(0)->symbol == "<bin_i_izraz>" &&
			 root->children.at(2)->symbol == "<jednakosni_izraz>" &&
			 root->children.at(1)->symbol == "OP_BIN_I") {
		// 1. provjeri(<bin_i_izraz>)
		// 2. <bin_i_izraz>.tip ∼ int
		// 3. provjeri(<jednakosni_izraz>)
		// 4. <jednakosni_izraz>.tip ∼ int
		if (bin_i_izraz(root->children.at(0))) {
			return 1;
		} else if (!implicit_conversion(root->children.at(0)->type, "int")) {
			return root->semantic_error();
		} else {
			if (jednakosni_izraz(root->children.at(2))) {
				return 1;
			} else if (!implicit_conversion(root->children.at(2)->type,
											"int")) {
				return root->semantic_error();
			} else {
				root->type = "int";
				root->lhs = false;
				bitwise_operation(1);
			}
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int bin_xili_izraz(std::shared_ptr<Node> root) {
	// 	<bin_xili_izraz> ::= <bin_i_izraz>
	// tip ← <bin_i_izraz>.tip
	// l-izraz ← <bin_i_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<bin_i_izraz>") {
		// 1. provjeri(<bin_i_izraz>)
		if (bin_i_izraz(root->children.at(0))) {
			return 1;
		} else {
			root->type = root->children.at(0)->type;
			root->lhs = root->children.at(0)->lhs;
		}
	}
	// 	<bin_xili_izraz> ::= <bin_xili_izraz> OP_BIN_XILI <bin_i_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 root->children.at(0)->symbol == "<bin_xili_izraz>" &&
			 root->children.at(1)->symbol == "OP_BIN_XILI" &&
			 root->children.at(2)->symbol == "<bin_i_izraz>") {
		// 1. provjeri(<bin_xili_izraz>)
		// 2. <bin_xili_izraz>.tip ∼ int
		// 3. provjeri(<bin_i_izraz>)
		// 4. <bin_i_izraz>.tip ∼ int
		if (bin_xili_izraz(root->children.at(0))) {
			return 1;
		} else if (!implicit_conversion(root->children.at(0)->type, "int")) {
			return root->semantic_error();
		} else {
			if (bin_i_izraz(root->children.at(2))) {
				return 1;
			} else if (!implicit_conversion(root->children.at(2)->type,
											"int")) {
				return root->semantic_error();
			} else {
				root->type = "int";
				root->lhs = false;

				bitwise_operation(3);
			}
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int bin_ili_izraz(std::shared_ptr<Node> root) {
	// <bin_ili_izraz> ::= <bin_xili_izraz>
	// tip ← <bin_xili_izraz>.tip
	// l-izraz ← <bin_xili_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<bin_xili_izraz>") {
		// 1. provjeri(<bin_xili_izraz>)
		if (bin_xili_izraz(root->children.at(0))) {
			return 1;
		} else {
			root->type = root->children.at(0)->type;
			root->lhs = root->children.at(0)->lhs;
		}
	}
	// <bin_ili_izraz> ::= <bin_ili_izraz> OP_BIN_ILI <bin_xili_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 root->children.at(0)->symbol == "<bin_ili_izraz>" &&
			 root->children.at(2)->symbol == "<bin_xili_izraz>" &&
			 root->children.at(1)->symbol == "OP_BIN_ILI") {
		// 1. provjeri(<bin_ili_izraz>)
		// 2. <bin_ili_izraz >.tip ∼ int
		// 3. provjeri(<bin_xili_izraz>)
		// 4. <bin_xili_izraz >.tip ∼ int
		if (bin_ili_izraz(root->children.at(0))) {
			return 1;
		} else if (!implicit_conversion(root->children.at(0)->type, "int")) {
			return root->semantic_error();
		} else {
			if (bin_xili_izraz(root->children.at(2))) {
				return 1;
			} else if (!implicit_conversion(root->children.at(2)->type,
											"int")) {
				return root->semantic_error();
			} else {
				root->type = "int";
				root->lhs = false;
				bitwise_operation(2);
			}
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int log_i_izraz(std::shared_ptr<Node> root) {
	// <log_i_izraz> ::= <bin_ili_izraz>
	// tip ← <bin_ili_izraz>.tip
	// l-izraz ← <bin_ili_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<bin_ili_izraz>") {
		// 1. provjeri(<bin_ili_izraz>)
		if (bin_ili_izraz(root->children.at(0))) {
			return 1;
		} else {
			root->type = root->children.at(0)->type;
			root->lhs = root->children.at(0)->lhs;
		}
	}
	// <log_i_izraz> ::= <log_i_izraz> OP_I <bin_ili_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 root->children.at(0)->symbol == "<log_i_izraz>" &&
			 root->children.at(2)->symbol == "<bin_ili_izraz>" &&
			 root->children.at(1)->symbol == "OP_I") {
		// 1. provjeri(<log_i_izraz>)
		// 2. <log_i_izraz>.tip ∼ int
		// 3. provjeri(<bin_ili_izraz>)
		// 4. <bin_ili_izraz>.tip ∼ int
		if (log_i_izraz(root->children.at(0))) {
			return 1;
		} else if (!implicit_conversion(root->children.at(0)->type, "int")) {
			return root->semantic_error();
		} else {
			logical_start(1);
			if (bin_ili_izraz(root->children.at(2))) {
				return 1;
			} else if (!implicit_conversion(root->children.at(2)->type,
											"int")) {
				return root->semantic_error();
			} else {
				root->type = "int";
				root->lhs = false;
				logical_check(1);
				logical_end();
			}
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int log_ili_izraz(std::shared_ptr<Node> root) {
	// <log_ili_izraz> ::= <log_i_izraz>
	// tip ← <log_i_izraz>.tip
	// l-izraz ← <log_i_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<log_i_izraz>") {
		// 1. provjeri(<log_i_izraz>)
		if (log_i_izraz(root->children.at(0))) {
			return 1;
		} else {
			root->type = root->children.at(0)->type;
			root->lhs = root->children.at(0)->lhs;
		}
	}
	// <log_ili_izraz> ::= <log_ili_izraz> OP_ILI <log_i_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 root->children.at(0)->symbol == "<log_ili_izraz>" &&
			 root->children.at(2)->symbol == "<log_i_izraz>" &&
			 root->children.at(1)->symbol == "OP_ILI") {
		// 1. provjeri(<log_ili_izraz>)
		// 2. <log_ili_izraz>.tip ∼ int
		// 3. provjeri(<log_i_izraz>)
		// 4. <log_i_izraz>.tip ∼ int
		if (log_ili_izraz(root->children.at(0))) {
			return 1;
		} else if (!implicit_conversion(root->children.at(0)->type, "int")) {
			return root->semantic_error();
		} else {
			logical_start(2);
			if (log_i_izraz(root->children.at(2))) {
				return 1;
			} else if (!implicit_conversion(root->children.at(2)->type,
											"int")) {
				return root->semantic_error();
			} else {
				root->type = "int";
				root->lhs = false;
				logical_check(2);
				logical_end();
			}
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int izraz_pridruzivanja(std::shared_ptr<Node> root) {
	// <izraz_pridruzivanja> ::= <log_ili_izraz>
	// tip ← <log_ili_izraz>.tip
	// l-izraz ← <log_ili_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<log_ili_izraz>") {
		// 1. provjeri(<log_ili_izraz>)
		if (log_ili_izraz(root->children.at(0))) {
			return 1;
		} else {
			root->type = root->children.at(0)->type;
			root->lhs = root->children.at(0)->lhs;
		}
	}
	// <izraz_pridruzivanja> ::= <postfiks_izraz> OP_PRIDRUZI
	// <izraz_pridruzivanja>
	// tip ← <postfiks_izraz>.tip
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 root->children.at(1)->symbol == "OP_PRIDRUZI" &&
			 root->children.at(0)->symbol == "<postfiks_izraz>" &&
			 root->children.at(2)->symbol == "<izraz_pridruzivanja>") {
		// 1. provjeri(<postfiks_izraz>)
		// 2. <postfiks_izraz>.l-izraz = 1
		// 3. provjeri(<izraz_pridruzivanja>)
		// 4. <izraz_pridruzivanja>.tip ∼ <postfiks_izraz>.tip
		updating.push(true);
		if (postfiks_izraz(root->children.at(0))) {
			return 1;
		} else {
			if (!root->children.at(0)->lhs) {
				return root->semantic_error();
			} else {
				updating.pop();
				if (izraz_pridruzivanja(root->children.at(2))) {
					return 1;
				} else {
					if (!implicit_conversion(root->children.at(2)->type,
											 root->children.at(0)->type)) {
						return root->semantic_error();
					} else {
						root->type = root->children.at(0)->type;
						root->lhs = false;
						for (std::string var : vars_to_update) {
							if (code_local_variables.count(var)) {
								store_local_var(var);
							} else if (code_global_variables.count(var)) {
								store_global_var(var);
							}
						}
						vars_to_update.clear();

						for (std::pair<std::string, int> arr : arrs_to_update) {
							if (function_arrays.count(arr.first)) {
								store_func_arr(arr.first, arr.second);
							} else if (code_global_arrays.count(arr.first)) {
								store_global_arr(arr.first, arr.second);
							}
						}
						arrs_to_update.clear();
					}
				}
			}
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int izraz(std::shared_ptr<Node> root) {
	// <izraz> ::= <izraz_pridruzivanja>
	// tip ← <izraz_pridruzivanja>.tip
	// l-izraz ← <izraz_pridruzivanja>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<izraz_pridruzivanja>") {
		// 1. provjeri(<izraz_pridruzivanja>)
		if (izraz_pridruzivanja(root->children.at(0))) {
			return 1;
		} else {
			root->type = root->children.at(0)->type;
			root->lhs = root->children.at(0)->lhs;
		}
	}
	// <izraz> ::= <izraz> ZAREZ <izraz_pridruzivanja>
	// tip ← <izraz_pridruzivanja>.tip
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 root->children.at(0)->symbol == "<izraz>" &&
			 root->children.at(1)->symbol == "ZAREZ" &&
			 root->children.at(2)->symbol == "<izraz_pridruzivanja>") {
		// 1. provjeri(<izraz>)
		// 2. provjeri(<izraz_pridruzivanja>)
		if (izraz(root->children.at(0))) {
			return 1;
		} else {
			if (izraz_pridruzivanja(root->children.at(2))) {
				return 1;
			} else {
				root->type = root->children.at(2)->type;
				root->lhs = false;
			}
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int slozena_naredba(std::shared_ptr<Node> root) {
	++block_count;
	if (from_function) {
		local_names.clear();
		for (std::string name : current_function_argument_names) {
			local_names.insert(name);
		}
		from_function = false;
	} else {
		local_names.clear();
		from_function = false;
	}
	for (auto function : available_functions) {
		if (function.second.first == 0) {
			std::string key = function.first;
			std::pair<std::string, std::vector<std::string>> value =
				std::make_pair(function.second.second.first,
							   function.second.second.second);
			global_declared_functions.insert(std::make_pair(key, value));
		}
	}
	// <slozena_naredba> ::= L_VIT_ZAGRADA <lista_naredbi> D_VIT_ZAGRADA
	if (root->children.size() == 3 &&
		root->children.at(1)->symbol == "<lista_naredbi>") {
		// 1. provjeri(<lista_naredbi>)
		if (lista_naredbi(root->children.at(1))) {
			return 1;
		}
	}
	// <slozena_naredba> ::= L_VIT_ZAGRADA <lista_deklaracija>
	// <lista_naredbi> D_VIT_ZAGRADA
	else if (root->children.size() == 4 &&
			 root->children.at(0)->symbol == "L_VIT_ZAGRADA" &&
			 root->children.at(1)->symbol == "<lista_deklaracija>" &&
			 root->children.at(2)->symbol == "<lista_naredbi>" &&
			 root->children.at(3)->symbol == "D_VIT_ZAGRADA") {
		// 1. provjeri(<lista_deklaracija>)
		// 2. provjeri(<lista_naredbi>)
		if (lista_deklaracija(root->children.at(1))) {
			return 1;
		} else if (lista_naredbi(root->children.at(2))) {
			return 1;
		}
	} else {
		return root->semantic_error();
	}
	local_names.clear();

	// possibly extremely inefficient and needed a change to inplace deletion
	// while iterating
	std::unordered_multimap<std::string, std::pair<int, std::string>>
		new_available_variables;
	std::unordered_multimap<std::string,
							std::pair<int, std::pair<std::string, int>>>
		new_available_arrays;
	std::unordered_multimap<
		std::string,
		std::pair<int, std::pair<std::string, std::vector<std::string>>>>
		new_available_functions;
	for (auto array : available_arrays) {
		if (array.second.first != block_count) {
			new_available_arrays.insert(array);
		}
	}
	for (auto function : available_functions) {
		if (function.second.first != block_count) {
			new_available_functions.insert(function);
		}
		if (!defined_functions.count(function.first)) {
			declared_functions.insert(function.first);
		}
	}
	for (auto variable : available_variables) {
		if (variable.second.first != block_count) {
			new_available_variables.insert(variable);
		} else {
			auto range = code_local_variables.equal_range(variable.first);
			if (range.first != range.second) {
				code_local_variables.erase(std::prev(range.second));
			}
		}
	}
	available_arrays.swap(new_available_arrays);
	available_functions.swap(new_available_functions);
	available_variables.swap(new_available_variables);
	--block_count;
	return 0;
}

int lista_naredbi(std::shared_ptr<Node> root) {
	// <lista_naredbi> ::= <naredba>
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<naredba>") {
		// 1. provjeri(<naredba>)
		if (naredba(root->children.at(0))) {
			return 1;
		}
	}
	// <lista_naredbi> ::= <lista_naredbi> <naredba>
	else if (root->children.size() == 2 &&
			 root->children.at(0)->symbol == "<lista_naredbi>" &&
			 root->children.at(1)->symbol == "<naredba>") {
		// 1. provjeri(<lista_naredbi>)
		// 2. provjeri(<naredba>)
		if (lista_naredbi(root->children.at(0))) {
			return 1;
		} else if (naredba(root->children.at(1))) {
			return 1;
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int naredba(std::shared_ptr<Node> root) {
	// Nezavrsni znak <naredba> generira blokove (<slozena_naredba>) i
	// razlicite vrste jednostavnih naredbi (<izraz_naredba>,
	// <naredba_grananja>, <naredba_petlje> i <naredba_skoka>). Kako su sve
	// produkcije jedinicne (s desne strane imaju jedan nezavrsni znak) i u
	// svim produkcijama se provjeravaju semanticka pravila na znaku s desne
	// strane, produkcije ovdje nisu prikazane.
	if (root->children.size() == 1) {
		if (root->children.at(0)->symbol == "<slozena_naredba>") {
			if (slozena_naredba(root->children.at(0))) {
				return 1;
			}
		} else if (root->children.at(0)->symbol == "<izraz_naredba>") {
			if (izraz_naredba(root->children.at(0))) {
				return 1;
			}
		} else if (root->children.at(0)->symbol == "<naredba_grananja>") {
			if (naredba_grananja(root->children.at(0))) {
				return 1;
			}
		} else if (root->children.at(0)->symbol == "<naredba_petlje>") {
			if (naredba_petlje(root->children.at(0))) {
				return 1;
			}
		} else if (root->children.at(0)->symbol == "<naredba_skoka>") {
			if (naredba_skoka(root->children.at(0))) {
				return 1;
			}
		} else {
			return root->semantic_error();
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int izraz_naredba(std::shared_ptr<Node> root) {
	// <izraz_naredba> ::= TOCKAZAREZ
	// tip ← int
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "TOCKAZAREZ") {
		root->type = "int";
	}
	// 	<izraz_naredba> ::= <izraz> TOCKAZAREZ
	// tip ← <izraz>.tip
	else if (root->children.size() == 2 &&
			 root->children.at(0)->symbol == "<izraz>" &&
			 root->children.at(1)->symbol == "TOCKAZAREZ") {
		// 1. provjeri(<izraz>)
		if (izraz(root->children.at(0))) {
			return 1;
		} else {
			root->type = root->children.at(0)->type;
		}
	} else {
		return root->semantic_error();
	}
	variable_increment_after();
	return 0;
}

int naredba_grananja(std::shared_ptr<Node> root) {
	// 	<naredba_grananja> ::= KR_IF L_ZAGRADA <izraz> D_ZAGRADA <naredba>
	if (root->children.size() == 5 && root->children.at(0)->symbol == "KR_IF" &&
		root->children.at(1)->symbol == "L_ZAGRADA" &&
		root->children.at(2)->symbol == "<izraz>" &&
		root->children.at(3)->symbol == "D_ZAGRADA" &&
		root->children.at(4)->symbol == "<naredba>") {
		// 1. provjeri(<izraz>)
		// 2. <izraz>.tip ∼ int
		// 3. provjeri(<naredba>)
		if (izraz(root->children.at(2))) {
			return 1;
		} else if (!implicit_conversion(root->children.at(2)->type, "int")) {
			return root->semantic_error();
		} else {
			branch_if();
			if (naredba(root->children.at(4))) {
				return 1;
			}
			branch_else();
		}
	}
	// <naredba_grananja> ::= KR_IF L_ZAGRADA <izraz> D_ZAGRADA <naredba>1
	// KR_ELSE <naredba>2
	else if (root->children.size() == 7 &&
			 root->children.at(0)->symbol == "KR_IF" &&
			 root->children.at(1)->symbol == "L_ZAGRADA" &&
			 root->children.at(2)->symbol == "<izraz>" &&
			 root->children.at(3)->symbol == "D_ZAGRADA" &&
			 root->children.at(4)->symbol == "<naredba>" &&
			 root->children.at(5)->symbol == "KR_ELSE" &&
			 root->children.at(6)->symbol == "<naredba>") {
		// 1. provjeri(<izraz>)
		// 2. <izraz>.tip ∼ int
		// 3. provjeri(<naredba>1)
		// 4. provjeri(<naredba>2)
		if (izraz(root->children.at(2))) {
			return 1;
		} else if (!implicit_conversion(root->children.at(2)->type, "int")) {
			return root->semantic_error();
		} else {
			branch_if();
			if (naredba(root->children.at(4))) {
				return 1;
			} else {
				branch_else();
				if (naredba(root->children.at(6))) {
					return 1;
				}
			}
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int naredba_petlje(std::shared_ptr<Node> root) {
	++loop_depth;
	// <naredba_petlje> ::= KR_WHILE L_ZAGRADA <izraz> D_ZAGRADA <naredba>
	if (root->children.size() == 5 &&
		root->children.at(0)->symbol == "KR_WHILE" &&
		root->children.at(1)->symbol == "L_ZAGRADA" &&
		root->children.at(2)->symbol == "<izraz>" &&
		root->children.at(3)->symbol == "D_ZAGRADA" &&
		root->children.at(4)->symbol == "<naredba>") {
		// 1. provjeri(<izraz>)
		// 2. <izraz>.tip ∼ int
		// 3. provjeri(<naredba>)
		while_start();
		if (izraz(root->children.at(2))) {
			return 1;
		} else if (!implicit_conversion(root->children.at(2)->type, "int")) {
			return root->semantic_error();
		} else {
			while_check();
			if (naredba(root->children.at(4))) {
				return 1;
			} else {
				while_end();
			}
		}
	}
	// <naredba_petlje> ::= KR_FOR L_ZAGRADA <izraz_naredba>1
	// <izraz_naredba>2 D_ZAGRADA <naredba>
	else if (root->children.size() == 6 &&
			 root->children.at(0)->symbol == "KR_FOR" &&
			 root->children.at(1)->symbol == "L_ZAGRADA" &&
			 root->children.at(2)->symbol == "<izraz_naredba>" &&
			 root->children.at(3)->symbol == "<izraz_naredba>" &&
			 root->children.at(4)->symbol == "D_ZAGRADA" &&
			 root->children.at(5)->symbol == "<naredba>") {
		// 1. provjeri(<izraz_naredba>1)
		// 2. provjeri(<izraz_naredba>2)
		// 3.<izraz_naredba>2.tip ∼ int
		// 4. provjeri(<naredba>)
		if (izraz_naredba(root->children.at(2))) {
			return 1;
		}
		while_start();
		if (izraz_naredba(root->children.at(3))) {
			return 1;
		} else if (!implicit_conversion(root->children.at(3)->type, "int")) {
			return root->semantic_error();
		} else {
			while_check();
			if (naredba(root->children.at(5))) {
				return 1;
			} else {
				while_end();
			}
		}
	}
	// <naredba_petlje> ::= KR_FOR L_ZAGRADA <izraz_naredba>1
	// <izraz_naredba>2 <izraz> D_ZAGRADA <naredba>
	else if (root->children.size() == 7 &&
			 root->children.at(0)->symbol == "KR_FOR" &&
			 root->children.at(1)->symbol == "L_ZAGRADA" &&
			 root->children.at(2)->symbol == "<izraz_naredba>" &&
			 root->children.at(3)->symbol == "<izraz_naredba>" &&
			 root->children.at(4)->symbol == "<izraz>" &&
			 root->children.at(5)->symbol == "D_ZAGRADA" &&
			 root->children.at(6)->symbol == "<naredba>") {
		// 1. provjeri(<izraz_naredba>1)
		// 2. provjeri(<izraz_naredba>2)
		// 3. <izraz_naredba>2.tip ∼ int
		// 4. provjeri(<izraz>)
		// 5. provjeri(<naredba>)
		if (izraz_naredba(root->children.at(2))) {
			return 1;
		}
		forc_start();
		if (izraz_naredba(root->children.at(3))) {
			return 1;
		} else if (!implicit_conversion(root->children.at(3)->type, "int")) {
			return root->semantic_error();
		} else {
			forc_check();
			forc_skip_first();
			if (izraz(root->children.at(4))) {
				return 1;
			}
			forc_skip_second();
			if (naredba(root->children.at(6))) {
				return 1;
			}
			forc_end();
		}
	} else {
		return root->semantic_error();
	}
	--loop_depth;
	return 0;
}

int naredba_skoka(std::shared_ptr<Node> root) {
	// <naredba_skoka> ::= (KR_CONTINUE | KR_BREAK) TOCKAZAREZ
	if (root->children.size() == 2 &&
		(root->children.at(0)->symbol == "KR_CONTINUE" ||
		 root->children.at(0)->symbol == "KR_BREAK") &&
		root->children.at(1)->symbol == "TOCKAZAREZ") {
		// 1. naredba se nalazi unutar petlje ili unutar bloka koji je
		// ugnijezden u petlji
		if (loop_depth < 1) {
			return root->semantic_error();
		}
	}
	// <naredba_skoka> ::= KR_RETURN TOCKAZAREZ
	else if (root->children.size() == 2 &&
			 root->children.at(0)->symbol == "KR_RETURN" &&
			 root->children.at(1)->symbol == "TOCKAZAREZ") {
		// 1. naredba se nalazi unutar funkcije tipa funkcija(params → void)
		if (current_function_return_type != "void") {
			return root->semantic_error();
		} else {
			return_sp();
		}

	}
	// <naredba_skoka> ::= KR_RETURN <izraz> TOCKAZAREZ
	else if (root->children.size() == 3 &&
			 root->children.at(0)->symbol == "KR_RETURN" &&
			 root->children.at(1)->symbol == "<izraz>" &&
			 root->children.at(2)->symbol == "TOCKAZAREZ") {
		// 1. provjeri(<izraz>)
		// 2. naredba se nalazi unutar funkcije tipa funkcija(params → pov)
		// i vrijedi <izraz>.tip ∼ pov
		if (izraz(root->children.at(1))) {
			return 1;
		} else {
			if (current_function_return_type == "void" ||
				current_function_return_type == "" ||
				!implicit_conversion(root->children.at(1)->type,
									 current_function_return_type)) {
				return root->semantic_error();
			} else {
				load_ret_val();
				return_sp();
			}
		}

	} else {
		return root->semantic_error();
	}
	variable_increment_after();
	return 0;
}

int prijevodna_jedinica(std::shared_ptr<Node> root) {
	// <prijevodna_jedinica> ::= <vanjska_deklaracija>
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<vanjska_deklaracija>") {
		// 1. provjeri(<vanjska_deklaracija>)
		if (vanjska_deklaracija(root->children.at(0))) {
			return 1;
		}
	}
	// <prijevodna_jedinica> ::= <prijevodna_jedinica> <vanjska_deklaracija>
	else if (root->children.size() == 2 &&
			 root->children.at(0)->symbol == "<prijevodna_jedinica>" &&
			 root->children.at(1)->symbol == "<vanjska_deklaracija>") {
		// 1. provjeri(<prijevodna_jedinica>)
		// 2. provjeri(<vanjska_deklaracija>)
		if (prijevodna_jedinica(root->children.at(0))) {
			return 1;
		} else if (vanjska_deklaracija(root->children.at(1))) {
			return 1;
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int vanjska_deklaracija(std::shared_ptr<Node> root) {
	// Nezavrsni znak <vanjska_deklaracija> generira ili definiciju funkcije
	// (znak <definicija_funkcije>) ili deklaraciju varijable ili funkcije
	// (znak <deklaracija>). Obje produkcije su jedinicne i u obje se
	// provjeravaju pravila u podstablu kojem je znak s desne strane
	// korijen.
	if (root->children.size() == 1) {
		if (root->children.at(0)->symbol == "<definicija_funkcije>") {
			if (definicija_funkcije(root->children.at(0))) {
				return 1;
			}
		} else if (root->children.at(0)->symbol == "<deklaracija>") {
			if (deklaracija(root->children.at(0))) {
				return 1;
			}
		} else {
			return root->semantic_error();
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

// possibly void funcs have no return ;
int definicija_funkcije(std::shared_ptr<Node> root) {
	// <definicija_funkcije> ::= <ime_tipa> IDN L_ZAGRADA KR_VOID D_ZAGRADA
	// <slozena_naredba>
	if (root->children.size() == 6 &&
		root->children.at(0)->symbol == "<ime_tipa>" &&
		root->children.at(1)->symbol == "IDN" &&
		root->children.at(2)->symbol == "L_ZAGRADA" &&
		root->children.at(3)->symbol == "KR_VOID" &&
		root->children.at(4)->symbol == "D_ZAGRADA" &&
		root->children.at(5)->symbol == "<slozena_naredba>") {
		// 1. provjeri(<ime_tipa>)
		// 2. <ime_tipa>.tip ̸= const(T)
		// 3. ne postoji prije definirana funkcija imena IDN.ime
		// 4. ako postoji deklaracija imena IDN.ime u globalnom djelokrugu
		// onda je pripadni tip te deklaracije funkcija(void →
		// <ime_tipa>.tip)
		// 5. zabiljezi definiciju i deklaraciju funkcije
		// 6. provjeri(<slozena_naredba>)

		if (ime_tipa(root->children.at(0))) {
			return 1;
		} else {
			if (root->children.at(1)->value == "main" &&
				root->children.at(0)->type == "int") {
				main_defined = true;
			}
			if (is_const(root->children.at(0)->type)) {
				return root->semantic_error();
			} else {
				if (defined_functions.count(root->children.at(1)->value)) {
					return root->semantic_error();
				} else {
					std::vector<std::string> arguments(0);
					std::pair<std::string, std::vector<std::string>>
						function_key = std::make_pair(
							root->children.at(0)->type, arguments);
					if (global_declared_functions.count(
							root->children.at(1)->value) &&
						global_declared_functions.at(
							root->children.at(1)->value) != function_key) {
						return root->semantic_error();
					} else {
						defined_functions.insert(root->children.at(1)->value);
						if (declared_functions.count(
								root->children.at(1)->value)) {
							declared_functions.erase(
								declared_functions.lower_bound(
									root->children.at(1)->value));
						}

						// currenly in function
						available_functions.insert(std::make_pair(
							root->children.at(1)->value,
							std::make_pair(block_count, function_key)));
						local_names.insert(root->children.at(1)->value);
						current_function_return_type =
							root->children.at(0)->type;
						current_function_argument_types = arguments;
						from_function = true;

						// generate new label
						if (root->children.at(1)->value == "main") {
							code_functions.emplace(
								std::make_pair("main", "F0"));
							fn_def("F0", 0);
						} else if (code_functions.count("main")) {
							std::string next_name = "F";
							next_name += std::to_string(code_functions.size());
							code_functions.emplace(std::make_pair(
								root->children.at(1)->value, next_name));
							fn_def(next_name,
								   current_function_argument_types.size());
						} else {
							std::string next_name = "F";
							next_name +=
								std::to_string(code_functions.size() + 1);
							code_functions.emplace(std::make_pair(
								root->children.at(1)->value, next_name));
							fn_def(next_name,
								   current_function_argument_types.size());
						}
						if (slozena_naredba(root->children.at(5))) {
							return 1;
						} else {
							// reset
							from_function = false;
							current_function_return_type.clear();
							current_function_argument_types.clear();
						}
					}
				}
			}
		}

	}
	// <definicija_funkcije> ::= <ime_tipa> IDN L_ZAGRADA <lista_parametara>
	// D_ZAGRADA <slozena_naredba>
	else if (root->children.size() == 6 &&
			 root->children.at(0)->symbol == "<ime_tipa>" &&
			 root->children.at(1)->symbol == "IDN" &&
			 root->children.at(2)->symbol == "L_ZAGRADA" &&
			 root->children.at(3)->symbol == "<lista_parametara>" &&
			 root->children.at(4)->symbol == "D_ZAGRADA" &&
			 root->children.at(5)->symbol == "<slozena_naredba>") {
		// 1. provjeri(<ime_tipa>)
		// 2. <ime_tipa>.tip != const(T)
		// 3. ne postoji prije definirana funkcija imena IDN.ime
		// 4. provjeri(<lista_parametara>)
		// 5. ako postoji deklaracija imena IDN.ime u globalnom djelokrugu
		// onda je pripadni tip te deklaracije
		// funkcija(<lista_parametara>.tipovi → <ime_tipa>.tip)
		// 6. zabiljezi definiciju i deklaraciju funkcije
		// 7. provjeri(<slozena_naredba>) uz parametre funkcije koristeci
		// <lista_parametara>.tipovi i <lista_parametara>.imena.
		if (ime_tipa(root->children.at(0))) {
			return 1;
		} else {
			if (is_const(root->children.at(0)->type)) {
				return root->semantic_error();
			} else {
				if (defined_functions.count(root->children.at(1)->value)) {
					return root->semantic_error();
				} else {
					if (lista_parametara(root->children.at(3))) {
						return 1;
					} else {
						std::pair<std::string, std::vector<std::string>>
							function_key =
								std::make_pair(root->children.at(0)->type,
											   root->children.at(3)->arg_types);
						if (global_declared_functions.count(
								root->children.at(1)->value) &&
							global_declared_functions.at(
								root->children.at(1)->value) != function_key) {
							return root->semantic_error();
						} else {
							defined_functions.insert(
								root->children.at(1)->value);
							if (declared_functions.count(
									root->children.at(1)->value)) {
								declared_functions.erase(
									declared_functions.lower_bound(
										root->children.at(1)->value));
							}
							// save parametars of previous function
							// actually doesn't exist only is special gnu c
							// compiler extension std::string
							// previous_function_type =
							// 	current_function_return_type;
							// std::vector<std::string>
							// previous_function_argument_types =
							// current_function_argument_types; currenly in new
							// function
							available_functions.insert(std::make_pair(
								root->children.at(1)->value,
								std::make_pair(block_count, function_key)));

							local_names.insert(root->children.at(1)->value);
							current_function_return_type =
								root->children.at(0)->type;
							current_function_argument_types =
								root->children.at(3)->arg_types;

							// generate new label
							if (code_functions.count("main")) {
								std::string next_name = "F";
								next_name +=
									std::to_string(code_functions.size());
								code_functions.emplace(std::make_pair(
									root->children.at(1)->value, next_name));
								fn_def(next_name,
									   current_function_argument_types.size());
							} else {
								std::string next_name = "F";
								next_name +=
									std::to_string(code_functions.size() + 1);
								code_functions.emplace(std::make_pair(
									root->children.at(1)->value, next_name));
								fn_def(next_name,
									   current_function_argument_types.size());
							}

							// write parameter names as local variables
							for (size_t i = 0;
								 i < root->children.at(3)->arg_types.size();
								 ++i) {
								std::string current_type =
									root->children.at(3)->arg_types.at(i);
								std::string current_name =
									root->children.at(3)->arg_names.at(i);
								if (is_array(current_type)) {
									function_arrays.emplace(std::make_pair(
										current_name, (i + 1) * 4));
								} else {
									code_local_variables.emplace(std::make_pair(
										current_name, (i + 1) * 4));
								}
							}

							for (size_t i = 0;
								 i < root->children.at(3)->arg_types.size();
								 ++i) {
								available_variables.insert(std::make_pair(
									root->children.at(3)->arg_names.at(i),
									std::make_pair(block_count + 1,
												   root->children.at(3)
													   ->arg_types.at(i))));
								local_names.insert(
									root->children.at(3)->arg_names.at(i));
								current_function_argument_names.push_back(
									root->children.at(3)->arg_names.at(i));
							}
							from_function = true;
							if (slozena_naredba(root->children.at(5))) {
								return 1;
							} else {
								// reset
								// doesn't exist only is special gnu c compiler
								// extension current_function_return_type =
								// previous_function_type;
								// current_function_argument_types =
								// 	previous_function_argument_types;
								current_function_return_type.clear();
								current_function_argument_names.clear();
								current_function_argument_types.clear();
								from_function = false;
								function_arrays.clear();
							}
						}
					}
				}
			}
		}
	} else {
		return root->semantic_error();
	}
	return_sp();
	return 0;
}

int lista_parametara(std::shared_ptr<Node> root) {
	// 	<lista_parametara> ::= <deklaracija_parametra>
	// tipovi ← [<deklaracija_parametra>.tip ]
	// imena ← [ <deklaracija_parametra>.ime ]
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<deklaracija_parametra>") {
		// 1. provjeri(<deklaracija_parametra>)
		if (deklaracija_parametra(root->children.at(0))) {
			return 1;
		} else {
			root->arg_types.push_back(root->children.at(0)->type);
			root->arg_names.push_back(root->children.at(0)->name);
		}
	}
	// <lista_parametara> ::= <lista_parametara> ZAREZ
	// <deklaracija_parametra>
	// tipovi ← <lista_parametara>.tipovi + [<deklaracija_parametra>.tip ]
	// imena ← <lista_parametara>.imena + [ <deklaracija_parametra>.ime ]
	else if (root->children.size() == 3 &&
			 root->children.at(1)->symbol == "ZAREZ" &&
			 root->children.at(0)->symbol == "<lista_parametara>" &&
			 root->children.at(2)->symbol == "<deklaracija_parametra>") {
		// 1. provjeri(<lista_parametara>)
		// 2. provjeri(<deklaracija_parametra>)
		// 3. <deklaracija_parametra>.ime ne postoji u
		// <lista_parametara>.imena
		if (lista_parametara(root->children.at(0))) {
			return 1;
		} else {
			if (deklaracija_parametra(root->children.at(2))) {
				return 1;
			} else {
				for (std::string current_name :
					 root->children.at(0)->arg_names) {
					if (root->children.at(2)->name == current_name) {
						return root->semantic_error();
					}
				}
				root->arg_types = root->children.at(0)->arg_types;
				root->arg_names = root->children.at(0)->arg_names;
				root->arg_types.push_back(root->children.at(2)->type);
				root->arg_names.push_back(root->children.at(2)->name);
			}
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int deklaracija_parametra(std::shared_ptr<Node> root) {
	// <deklaracija_parametra> ::= <ime_tipa> IDN
	// tip ← <ime_tipa>.tip
	// ime ← IDN.ime
	if (root->children.size() == 2 &&
		root->children.at(0)->symbol == "<ime_tipa>" &&
		root->children.at(1)->symbol == "IDN") {
		// 1. provjeri(<ime_tipa>)
		// 2. <ime_tipa>.tip ̸= void
		if (ime_tipa(root->children.at(0))) {
			return 1;
		}
		if (root->children.at(0)->type == "void") {
			return root->semantic_error();
		} else {
			root->type = root->children.at(0)->type;
			root->name = root->children.at(1)->value;
		}
	}
	// <deklaracija_parametra> ::= <ime_tipa> IDN L_UGL_ZAGRADA
	// D_UGL_ZAGRADA
	// tip ← niz(<ime_tipa>.tip)
	// ime ← IDN.ime
	else if (root->children.size() == 4 &&
			 root->children.at(0)->symbol == "<ime_tipa>" &&
			 root->children.at(1)->symbol == "IDN" &&
			 root->children.at(2)->symbol == "L_UGL_ZAGRADA" &&
			 root->children.at(3)->symbol == "D_UGL_ZAGRADA") {
		// 1. provjeri(<ime_tipa>)
		// 2. <ime_tipa>.tip ̸= void
		if (ime_tipa(root->children.at(0))) {
			return 1;
		}
		if (root->children.at(0)->type == "void") {
			return root->semantic_error();
		} else {
			std::string current_type = "niz(";
			current_type += root->children.at(0)->type;
			current_type += ")";
			root->type = current_type;
			root->name = root->children.at(1)->value;
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int lista_deklaracija(std::shared_ptr<Node> root) {
	// <lista_deklaracija> ::= <deklaracija>
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<deklaracija>") {
		// 1. provjeri(<deklaracija>)
		if (deklaracija(root->children.at(0))) {
			return 1;
		}
	}
	// <lista_deklaracija> ::= <lista_deklaracija> <deklaracija>
	else if (root->children.size() == 2 &&
			 root->children.at(0)->symbol == "<lista_deklaracija>" &&
			 root->children.at(1)->symbol == "<deklaracija>") {
		// 1. provjeri(<lista_deklaracija>)
		// 2. provjeri(<deklaracija>)
		if (lista_deklaracija(root->children.at(0))) {
			return 1;
		} else {
			if (deklaracija(root->children.at(1))) {
				return 1;
			}
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int deklaracija(std::shared_ptr<Node> root) {
	// <deklaracija> ::= <ime_tipa> <lista_init_deklaratora> TOCKAZAREZ
	if (root->children.size() == 3 &&
		root->children.at(0)->symbol == "<ime_tipa>" &&
		root->children.at(1)->symbol == "<lista_init_deklaratora>" &&
		root->children.at(2)->symbol == "TOCKAZAREZ") {
		// 1. provjeri(<ime_tipa>)
		// 2. provjeri(<lista_init_deklaratora>) uz nasljedno svojstvo
		// <lista_init_deklaratora>.ntip ← <ime_tipa>.tip
		if (ime_tipa(root->children.at(0))) {
			return 1;
		} else {
			root->children.at(1)->inherited_type = root->children.at(0)->type;
			if (lista_init_deklaratora(root->children.at(1))) {
				return 1;
			}
		}
	} else {
		return root->semantic_error();
	}
	return 0;
	variable_increment_after();
}

int lista_init_deklaratora(std::shared_ptr<Node> root) {
	// <lista_init_deklaratora> ::= <init_deklarator>
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<init_deklarator>") {
		// 1. provjeri(<init_deklarator>) uz nasljedno svojstvo
		// <init_deklarator>.ntip ← <lista_init_deklaratora>.ntip
		root->children.at(0)->inherited_type = root->inherited_type;
		if (init_deklarator(root->children.at(0))) {
			return 1;
		}
	}
	// <lista_init_deklaratora>1 ::= <lista_init_deklaratora>2 ZAREZ
	// <init_deklarator>
	else if (root->children.size() == 3 &&
			 root->children.at(0)->symbol == "<lista_init_deklaratora>" &&
			 root->children.at(1)->symbol == "ZAREZ" &&
			 root->children.at(2)->symbol == "<init_deklarator>") {
		// 1. provjeri(<lista_init_deklaratora>2) uz nasljedno svojstvo
		// <lista_init_deklaratora>2.ntip ← <lista_init_deklaratora>1.ntip
		// 2. provjeri(<init_deklarator>) uz nasljedno svojstvo
		// <init_deklarator>.ntip ← <lista_init_deklaratora>1.ntip
		root->children.at(0)->inherited_type = root->inherited_type;
		if (lista_init_deklaratora(root->children.at(0))) {
			return 1;
		} else {
			root->children.at(2)->inherited_type = root->inherited_type;
			if (init_deklarator(root->children.at(2))) {
				return 1;
			}
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int init_deklarator(std::shared_ptr<Node> root) {
	// 	<init_deklarator> ::= <izravni_deklarator>
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<izravni_deklarator>") {
		// 1. provjeri(<izravni_deklarator>) uz nasljedno svojstvo
		// <izravni_deklarator>.ntip ← <init_deklarator>.ntip
		// 2. <izravni_deklarator>.tip ̸= const(T) i
		// <izravni_deklarator>.tip ̸= niz(const(T))
		root->children.at(0)->inherited_type = root->inherited_type;
		if (izravni_deklarator(root->children.at(0))) {
			return 1;
		} else {
			if (is_const(root->children.at(0)->type) ||
				is_const(remove_array(root->children.at(0)->type))) {
				return root->semantic_error();
			} else {
				current_global_array = "";
			}
		}
	}
	// <init_deklarator> ::= <izravni_deklarator> OP_PRIDRUZI
	// <inicijalizator>
	else if (root->children.size() == 3 &&
			 root->children.at(1)->symbol == "OP_PRIDRUZI" &&
			 root->children.at(0)->symbol == "<izravni_deklarator>" &&
			 root->children.at(2)->symbol == "<inicijalizator>") {
		// 1. provjeri(<izravni_deklarator>) uz nasljedno svojstvo
		// <izravni_deklarator>.ntip ← <init_deklarator>.ntip
		// 2. provjeri(<incijalizator>)
		// 3. ako je <izravni_deklarator>.tip T ili const(T)
		// <inicijalizator>.tip ∼ T inace ako je <izravni_deklarator>.tip
		// niz(T) ili niz(const(T)) <inicijalizator>.br-elem ≤
		// <izravni_deklarator>.br-elem za svaki U iz
		// <inicijalizator>.tipovi vrijedi U ∼ T inace greska
		root->children.at(0)->inherited_type = root->inherited_type;
		if (izravni_deklarator(root->children.at(0))) {
			return 1;
		} else {
			if (inicijalizator(root->children.at(2))) {
				return 1;
			}
			current_global_array = "";
			if (is_array(root->children.at(0)->type)) {
				if (!(root->children.at(2)->element_count <=
					  root->children.at(0)->element_count)) {
					return root->semantic_error();
				} else {
					std::string current_type =
						remove_const(remove_array(root->children.at(0)->type));
					if (root->children.at(2)->arg_types.size() !=
						root->children.at(2)->element_count) {
						return root->semantic_error();
					}
					for (std::string type : root->children.at(2)->arg_types) {
						if (!implicit_conversion(type, current_type)) {
							return root->semantic_error();
						}
					}
				}
			} else {
				// const(T) -> T
				std::string current_type =
					remove_const(root->children.at(0)->type);
				if (!implicit_conversion(root->children.at(2)->type,
										 current_type)) {
					return root->semantic_error();
				}
			}
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int izravni_deklarator(std::shared_ptr<Node> root) {
	// 	<izravni_deklarator> ::= IDN
	// tip ← ntip
	if (root->children.size() == 1 && root->children.at(0)->symbol == "IDN") {
		// 1. ntip ̸= void
		// 2. IDN.ime nije deklarirano u lokalnom djelokrugu
		// 3. zabiljezi deklaraciju IDN.ime s odgovarajucim tipom
		if (root->inherited_type == "void") {
			return root->semantic_error();
		} else {
			// IDN is variable
			if (local_names.count(root->children.at(0)->value)) {
				return root->semantic_error();
			} else {
				root->type = root->inherited_type;
				available_variables.insert(
					std::make_pair(root->children.at(0)->value,
								   std::make_pair(block_count, root->type)));
				local_names.insert(root->children.at(0)->value);
				if (block_count == 0) {
					current_global_variable = root->children.at(0)->value;
					std::string next_name = "V";
					next_name += std::to_string(code_global_variables.size());
					code_global_variables.emplace(
						std::make_pair(root->children.at(0)->value, next_name));
				} else {
					code_local_variables.emplace(std::make_pair(
						root->children.at(0)->value,
						(code_local_variables.size() + function_arrays.size()) *
								4 +
							4));
				}
			}
		}
	}
	// <izravni_deklarator> ::= IDN L_UGL_ZAGRADA BROJ D_UGL_ZAGRADA
	// tip ← niz(ntip)
	// br-elem ← BROJ.vrijednost
	else if (root->children.size() == 4 &&
			 root->children.at(0)->symbol == "IDN" &&
			 root->children.at(1)->symbol == "L_UGL_ZAGRADA" &&
			 root->children.at(2)->symbol == "BROJ" &&
			 root->children.at(3)->symbol == "D_UGL_ZAGRADA") {
		// 1. ntip ̸= void
		// 2. IDN.ime nije deklarirano u lokalnom djelokrugu
		// 3. BROJ.vrijednost je pozitivan broj (> 0) ne veci od 1024
		// 4. zabiljezi deklaraciju IDN.ime s odgovarajucim tipom
		if (root->inherited_type == "void") {
			return root->semantic_error();
		} else {
			// IDN is array
			if (local_names.count(root->children.at(0)->value)) {
				return root->semantic_error();
			} else {
				int number;
				try {
					number = std::stoi(root->children.at(2)->value);
					if (number < 0 || number > 1024) {
						return root->semantic_error();
					}
					std::string current_type = "niz(";
					current_type += root->inherited_type;
					current_type += ")";
					root->type = current_type;
					root->element_count = number;
					available_arrays.insert(std::make_pair(
						root->children.at(0)->value,
						std::make_pair(block_count,
									   std::make_pair(current_type, number))));
					local_names.insert(root->children.at(0)->value);
					current_global_array = root->children.at(0)->value;
					std::string next_name = "A";
					next_name += std::to_string(code_global_arrays.size());
					code_global_arrays.emplace(
						std::make_pair(root->children.at(0)->value,
									   std::make_pair(next_name, number)));
				} catch (const std::out_of_range &oor) {
					return root->semantic_error();
				}
			}
		}
	}
	// <izravni_deklarator> ::= IDN L_ZAGRADA KR_VOID D_ZAGRADA
	// tip ← funkcija(void → ntip)
	else if (root->children.at(0)->symbol == "IDN" &&
			 root->children.at(1)->symbol == "L_ZAGRADA" &&
			 root->children.at(2)->symbol == "KR_VOID" &&
			 root->children.at(3)->symbol == "D_ZAGRADA" &&
			 root->children.size() == 4) {
		// 1. ako je IDN.ime deklarirano u lokalnom djelokrugu, tip
		// prethodne deklaracije je jednak funkcija(void → ntip)
		// 2. zabiljezi deklaraciju IDN.ime s odgovarajucim tipom ako ista
		// funkcija vec nije deklarirana u lokalnom djelokrugu
		std::pair<int, std::pair<std::string, std::vector<std::string>>>
			function_value = std::make_pair(
				block_count, std::make_pair(root->inherited_type,
											std::vector<std::string>()));
		if (local_names.count(root->children.at(0)->value)) {
			if (available_functions.count(root->children.at(0)->value)) {
				auto range = available_functions.equal_range(
					root->children.at(0)->value);
				for (auto it = range.first; it != range.second;
					 it = std::next(it)) {
					if (it->second != function_value) {
						return root->semantic_error();
					}
				}
			} else {
				return root->semantic_error();
			}
		} else {
			available_functions.insert(
				std::make_pair(root->children.at(0)->value, function_value));
			local_names.insert(root->children.at(0)->value);
		}
		std::string current_type = "funkcija(void -> ";
		current_type += root->inherited_type;
		current_type += ")";
	}
	// <izravni_deklarator> ::= IDN L_ZAGRADA <lista_parametara> D_ZAGRADA
	// tip ← funkcija(<lista_parametara>.tipovi → ntip)
	else if (root->children.size() == 4 &&
			 root->children.at(0)->symbol == "IDN" &&
			 root->children.at(1)->symbol == "L_ZAGRADA" &&
			 root->children.at(2)->symbol == "<lista_parametara>" &&
			 root->children.at(3)->symbol == "D_ZAGRADA") {
		// 1. provjeri(<lista_parametara>)
		// 2. ako je IDN.ime deklarirano u lokalnom
		// djelokrugu, tip prethodne deklaracije je jednak
		// funkcija(<lista_parametara>.tipovi → ntip)
		// 3. zabiljezi deklaraciju IDN.ime s odgovarajucim tipom ako ista
		// funkcija vec nije deklarirana u lokalnom djelokrugu
		if (lista_parametara(root->children.at(2))) {
			return 1;
		} else {
			std::pair<int, std::pair<std::string, std::vector<std::string>>>
				function_value = std::make_pair(
					block_count,
					std::make_pair(root->inherited_type,
								   root->children.at(2)->arg_types));
			if (local_names.count(root->children.at(0)->value)) {
				if (available_functions.count(root->children.at(0)->value)) {
					auto range = available_functions.equal_range(
						root->children.at(0)->value);
					for (auto it = range.first; it != range.second;
						 it = std::next(it)) {
						if (it->second != function_value) {
							return root->semantic_error();
						}
					}
				} else {
					return root->semantic_error();
				}
			} else {
				available_functions.insert(std::make_pair(
					root->children.at(0)->value, function_value));
				local_names.insert(root->children.at(0)->value);
			}
			std::string current_type = "funkcija(params -> ";
			current_type += root->inherited_type;
			current_type += ")";
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}

int inicijalizator(std::shared_ptr<Node> root) {
	// 	<inicijalizator> ::= <izraz_pridruzivanja>
	// ako je <izraz_pridruzivanja> ⇒∗ NIZ_ZNAKOVA
	// br-elem ← duljina niza znakova + 1
	// tipovi ← lista duljine br-elem, svi elementi su char
	// inace tip ← <izraz_pridruzivanja>.tip
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<izraz_pridruzivanja>") {
		// 1. provjeri(<izraz_pridruzivanja>)
		if (izraz_pridruzivanja(root->children.at(0))) {
			return 1;
		} else {
			// U ovoj gramatici, navedena relacija je zadovoljena samo ako
			// se iz znaka <izraz_pridruzivanja> primjenom jedinicnih
			// produkcija konacno generira NIZ_ZNAKOVA, tj. slijed
			// generiranja je <izraz_pridruzivanja> ⇒ <log_ili_izraz> ⇒ ...
			// ⇒ <primarni_izraz> ⇒ NIZ_ZNAKOVA. only 1 element on right
			// side of all deeper productions
			std::shared_ptr<Node> current = root->children.at(0);
			while (current->children.size() == 1 &&
				   current->symbol != "NIZ_ZNAKOVA") {
				current = current->children.at(0);
			}
			if (current->symbol == "NIZ_ZNAKOVA") {
				root->element_count = string_length(current->value) + 1;
				root->arg_types =
					std::vector<std::string>(root->element_count, "char");
			} else {
				root->type = root->children.at(0)->type;
			}
		}
	}
	// 	<inicijalizator> ::= L_VIT_ZAGRADA <lista_izraza_pridruzivanja>
	// D_VIT_ZAGRADA
	// br-elem ← <lista_izraza_pridruzivanja>.br-elem
	// tipovi ← <lista_izraza_pridruzivanja>.tipovi
	else if (root->children.size() == 3 &&
			 root->children.at(0)->symbol == "L_VIT_ZAGRADA" &&
			 root->children.at(1)->symbol == "<lista_izraza_pridruzivanja>" &&
			 root->children.at(2)->symbol == "D_VIT_ZAGRADA") {
		// 1. provjeri(<lista_izraza_pridruzivanja>)
		if (lista_izraza_pridruzivanja(root->children.at(1))) {
			return 1;
		} else {
			root->element_count = root->children.at(1)->element_count;
			root->arg_types = root->children.at(1)->arg_types;
		}

	} else {
		return root->semantic_error();
	}
	return 0;
}

int lista_izraza_pridruzivanja(std::shared_ptr<Node> root) {
	// 	<lista_izraza_pridruzivanja> ::= <izraz_pridruzivanja>
	// tipovi ← [ <izraz_pridruzivanja>.tip ]
	// br-elem ← 1
	if (root->children.size() == 1 &&
		root->children.at(0)->symbol == "<izraz_pridruzivanja>") {
		// 1. provjeri(<izraz_pridruzivanja>)
		if (izraz_pridruzivanja(root->children.at(0))) {
			return 1;
		}
		root->arg_types.push_back(root->children.at(0)->type);
		root->element_count = 1;
	}
	// 	<lista_izraza_pridruzivanja> ::= <lista_izraza_pridruzivanja> ZAREZ
	// <izraz_pridruzivanja>
	// tipovi ← <lista_izraza_pridruzivanja>.tipovi +
	// [<izraz_pridruzivanja>.tip]
	// br-elem ← <lista_izraza_pridruzivanja>.br-elem + 1
	else if (root->children.size() == 3 &&
			 root->children.at(0)->symbol == "<lista_izraza_pridruzivanja>" &&
			 root->children.at(1)->symbol == "ZAREZ" &&
			 root->children.at(2)->symbol == "<izraz_pridruzivanja>") {
		// 1. provjeri(<lista_izraza_pridruzivanja>)
		// 2. provjeri(<izraz_pridruzivanja>)
		if (lista_izraza_pridruzivanja(root->children.at(0))) {
			return 1;
		} else {
			if (izraz_pridruzivanja(root->children.at(2))) {
				return 1;
			}
			root->arg_types = root->children.at(0)->arg_types;
			root->arg_types.push_back(root->children.at(2)->type);
			root->element_count = root->children.at(0)->element_count + 1;
		}
	} else {
		return root->semantic_error();
	}
	return 0;
}
