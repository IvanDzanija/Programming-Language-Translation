#include "helper_functions.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
// helper functions
void debug() { std::cout << "I'm here" << std::endl; }

bool accepted_char(std::string to_check) {
	if (to_check.size() == 1) {
		return true;
	} else if (to_check.at(0) != '\\') {
		return false;
	}
	std::vector<char> accepted = {'n', 't', '0', '"', '\'', '\\'};
	bool error = true;
	for (char check : accepted) {
		if (check == to_check.at(1)) {
			error = false;
		}
	}
	return !error;
}

bool implicit_conversion(std::string t1, std::string t2) {
	if (t1 == t2) {
		// same types
		return true;
	} else if (t1 == "int" && t2 == "const(int)") {
		return true;
	} else if (t1 == "const(int)" && t2 == "int") {
		return true;
	} else if (t1 == "char" &&
			   (t2 == "const(char)" || t2 == "int" || t2 == "const(int)")) {
		return true;
	} else if (t1 == "const(char)" &&
			   (t2 == "char" || t2 == "int" || t2 == "const(int)")) {
		return true;
	} else if (t1 == "niz(int)" && t2 == "niz(const(int))") {
		return true;

	} else if (t1 == "niz(char)" && t2 == "niz(const(char))") {
		return true;
	} else {
		return false;
	}
}

bool explicit_conversion(std::string t1, std::string t2) {
	if (t1 == t2) {
		return true;
	} else if (implicit_conversion(t1, t2)) {
		return true;
	} else if (t2 == "char" || t2 == "const(char)") {
		if (t1 == "int" || t1 == "const(int)") {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

bool is_array(std::string type) { return type.substr(0, 3) == "niz"; }

bool is_const(std::string type) { return type.substr(0, 5) == "const"; }

bool same_arguments(std::vector<std::string> args1,
					std::vector<std::string> args2) {
	if (args1.size() != args2.size()) {
		return false;
	}
	// trying implicit conversion of arguments of args1 to types of args2
	for (size_t i = 0; i < args1.size(); ++i) {
		if (!implicit_conversion(args1.at(i), args2.at(i))) {
			return false;
		}
	}
	return true;
}

int string_length(std::string to_check) {
	// accounts for double quotes and all escape characters
	return to_check.size() -
		   std::count(to_check.begin(), to_check.end(), '\\') - 2;
}

std::string remove_const(std::string type) {
	if (is_const(type)) {
		return type.substr(6, type.size() - 1 - 6);
	} else {
		return type;
	}
}

std::string remove_array(std::string type) {
	if (is_array(type)) {
		return type.substr(4, type.size() - 1 - 4);
	} else {
		return type;
	}
}
