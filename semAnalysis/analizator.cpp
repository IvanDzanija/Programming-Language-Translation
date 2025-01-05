#include <iostream>
#include <string>
#include <vector>

// prebaciti funckije produckije u poseban file
// stvoriti klasu node

// helper structures;
struct node {
	std::string type = "undefined";
	bool lhs = false;
	int row, depth;
	std::vector<std::string> arg_types;
	std::string symbol, value;
	std::vector<node> children;
};

std::vector<node> all_nodes;
int loop_depth = 0; // tracks the depth of the loop
std::vector<std::string> current_function_argument_types;
std::string current_function_return_type = "";

// helper functions
void print_node(node node) {
	std::cout << "Node: " << node.symbol << std::endl;
	std::cout << node.type << std::endl;
	if (node.row != -1) {
		std::cout << node.value << ' ' << node.row << ' ' << std::endl;
	}
}

void semantic_error(node *root) {
	std::cout << root->symbol << " ::=";
	for (node current : root->children) {
		std::cout << ' ' << current.symbol;
		if (current.row >= 0) {
			std::cout << '(' << current.row << ',' << current.value << ')';
		}
	}
	std::cout << std::endl;
	exit(0);
}

bool accepted_char(std::string to_check) {
	if (to_check.at(0) != '\\') {
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
	} else if (t1 == "char " &&
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

// production functions
void primarni_izraz(node *root);
void postfiks_izraz(node *root);
void lista_argumenata(node *root);
void unarni_izraz(node *root);
void cast_izraz(node *root);
void ime_tipa(node *root);
void specifikator_tipa(node *root);
void multiplikativni_izraz(node *root);
void aditivni_izraz(node *root);
void odnosni_izraz(node *root);
void jednakosni_izraz(node *root);
void bin_i_izraz(node *root);
void bin_xili_izraz(node *root);
void bin_ili_izraz(node *root);
void log_i_izraz(node *root);
void log_ili_izraz(node *root);
void izraz_pridruzivanja(node *root);
void izraz(node *root);
void slozena_naredba(node *root);
void lista_naredbi(node *root);
void naredba(node *root);
void izraz_naredba(node *root);
void naredba_grananja(node *root);
void naredba_petlje(node *root);
void naredba_skoka(node *root);
void prijevodna_jedinica(node *root);
void vanjska_deklaracija(node *root);

void primarni_izraz(node *root) {
	std::string symbol = root->children.at(0).symbol;
	std::string value = root->children.at(0).value;
	if (root->children.at(0).symbol == "IDN") {
		// implement
		return;
	} else if (symbol == "BROJ") {
		root->type = "int";
		root->lhs = false;
		try {
			int temp = std::stoi(value);
		} catch (const std::out_of_range &oor) {
			semantic_error(root);
		}
		return;
	} else if (root->children.at(0).symbol == "ZNAK") {
		root->type = "char";
		root->lhs = false;
		if (value.size() > 4) {
			semantic_error(root);
		} else if (value.at(0) != '\'' || value.at(value.size() - 1) != '\'') {
			semantic_error(root);
		} else {
			value = value.substr(1, value.size() - 2);
			if (!accepted_char(value)) {
				semantic_error(root);
			}
		}
		return;

	} else if (root->children.at(0).symbol == "NIZ_ZNAKOVA") {
		root->type = "niz(const(char))";
		root->lhs = false;
		if (value.at(0) != '"' || value.at(value.size() - 1) != '"' ||
			value.at(value.size() - 2) == '\\') {
			semantic_error(root);
		} else {
			for (size_t i = 1; i < value.size() - 2; ++i) {
				std::string to_check = "";
				to_check += (value.at(i));
				to_check += (value.at(i + 1));
				if (!accepted_char(to_check)) {
					semantic_error(root);
				}
			}
		}
	} else if (root->children.at(0).symbol == "L_ZAGRADA") {
		izraz(&root->children.at(1));
		root->type = root->children.at(1).type;
		root->lhs = root->children.at(1).lhs;
	} else {
		semantic_error(root);
	}
	return;
}
void postfiks_izraz(node *root) {
	// possibly have to check that the whole production is exactly the same

	//<postfiks_izraz> ::= <primarni_izraz>
	if (root->children.at(0).symbol == "<primarni_izraz>") {
		primarni_izraz(&root->children.at(0));
		root->type = root->children.at(0).type;
		root->lhs = root->children.at(0).lhs;
	}
	// <postfiks_izraz> ::= <postfiks_izraz> L_UGL_ZAGRADA <izraz>
	// D_UGL_ZAGRADA
	else if (root->children.at(2).symbol == "<izraz>") {
		// 1. provjeri(<postfiks_izraz>)
		// 2. <postfiks_izraz>.tip = niz(X)
		// 3. provjeri(<izraz>)
		// 4. <izraz>.tip ∼ int
		postfiks_izraz(&root->children.at(0));
		if (!is_array(root->children.at(0).type)) {
			semantic_error(root);
		} else {
			izraz(&root->children.at(2));
			if (!implicit_conversion(root->children.at(2).type, "int")) {
				semantic_error(root);
			} else {
				// tip ← X
				// l-izraz ← X ̸= const(T)
				std::string X = root->children.at(0).type;
				X = X.substr(4);
				X.pop_back();
				root->type = X;
				root->lhs = !(is_const(X));
			}
		}
	}
	// <postfiks_izraz>:: = <postfiks_izraz> L_ZAGRADA D_ZAGRADA
	// 	tip ← pov
	// 	l-izraz ← 0
	else if (root->children.size() == 3) {
		// 1. provjeri(<postfiks_izraz>)
		// 2. < postfiks_izraz >.tip = funkcija(void → pov)
		postfiks_izraz(&root->children.at(0));
		if (root->children.at(0).type != "funkcija(void → pov)") {
			semantic_error(root);
		} else {
			root->type = "pov"; // temporary
			root->lhs = false;
		}
	}
	// <postfiks_izraz> ::= <postfiks_izraz> L_ZAGRADA <lista_argumenata>
	// D_ZAGRADA
	// tip ← pov
	// l-izraz ← 0
	else if (root->children.at(2).symbol == "<lista_argumenata>") {
		// 1. provjeri(<postfiks_izraz>)
		// 2. provjeri(<lista_argumenata>)
		// 3. <postfiks_izraz>.tip = funkcija(params → pov) i redom po
		// elementima arg-tip iz <lista_argumenata>.tipovi i param-tip iz params
		// vrijedi arg-tip ∼ param-tip
		postfiks_izraz(&root->children.at(0));
		lista_argumenata(&root->children.at(2));
		if (same_arguments(root->children.at(2).arg_types,
						   root->children.at(0).arg_types)) {
			semantic_error(root);
		} else {
			root->type = "pov"; // temporary
			root->lhs = false;
		}
	}
	// <postfiks_izraz> ::= <postfiks_izraz> (OP_INC | OP_DEC)
	//  tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 2) {
		// 1. provjeri(<postfiks_izraz>)
		// 2. <postfiks_izraz>.l-izraz = 1 i <postfiks_izraz>.tip ∼ int
		postfiks_izraz(&root->children.at(0));
		if (!root->children.at(0).lhs) {
			semantic_error(root);
		} else if (!implicit_conversion(root->children.at(0).type, "int")) {
			semantic_error(root);
		} else {
			root->type = "int";
			root->lhs = false;
		}
	} else {
		semantic_error(root);
	}
	return;
}
void lista_argumenata(node *root) {
	// <lista_argumenata> ::= <izraz_pridruzivanja>
	// tipovi ← [<izraz_pridruzivanja>.tip ]
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "<izraz_pridruzivanja>") {
		// 1. provjeri(<izraz_pridruzivanja>)
		izraz_pridruzivanja(&root->children.at(0));
		root->arg_types.push_back(root->children.at(0).type);
	}
	// <lista_argumenata> ::= <lista_argumenata> ZAREZ <izraz_pridruzivanja>
	// tipovi ← <lista_argumenata>.tipovi + [ <izraz_pridruzivanja>.tip ]
	else if (root->children.size() == 3) {
		// 1. provjeri(<lista_argumenata>)
		// 2. provjeri(<izraz_pridruzivanja>)
		lista_argumenata(&root->children.at(0));
		izraz_pridruzivanja(&root->children.at(2));
		root->arg_types = root->children.at(0).arg_types;
		root->arg_types.push_back(root->children.at(2).type);
	} else {
		semantic_error(root);
	}
	return;
}

void unarni_izraz(node *root) {
	// 	<unarni_izraz> ::= <postfiks_izraz>
	// tip ← <postfiks_izraz>.tip
	// l-izraz ← <postfiks_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "<postfiks_izraz> ") {
		// 1. provjeri(<postfiks_izraz>)
		postfiks_izraz(&root->children.at(0));
		root->type = root->children.at(0).type;
		root->lhs = root->children.at(0).lhs;
	}
	// <unarni_izraz> ::= (OP_INC | OP_DEC) <unarni_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 2 &&
			 (root->children.at(0).symbol == "OP_INC" ||
			  root->children.at(0).symbol == "OP_DEC") &&
			 root->children.at(1).symbol == "<unarni_izraz>") {
		// 1. provjeri(<unarni_izraz>)
		// 2. <unarni_izraz>.l-izraz = 1 i <unarni_izraz>.tip ∼ int
		unarni_izraz(&root->children.at(1));
		if (!root->children.at(1).lhs) {
			semantic_error(root);
		} else if (!implicit_conversion(root->children.at(1).type, "int")) {
			semantic_error(root);
		} else {
			root->type = "int";
			root->lhs = false;
		}
	}
	// <unarni_izraz> ::= <unarni_operator> <cast_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 2 &&
			 (root->children.at(0).symbol == "<unarni_operator>" &&
			  root->children.at(1).symbol == "<cast_izraz>")) {
		// 1. provjeri(<cast_izraz>)
		// 2. <cast_izraz>.tip ∼ int
		cast_izraz(&root->children.at(1));
		if (!implicit_conversion(root->children.at(1).type, "int")) {
			semantic_error(root);
		} else {
			root->type = "int";
			root->lhs = false;
		}
	} else {
		semantic_error(root);
	}
	return;
}

void cast_izraz(node *root) {
	// <cast_izraz> ::= <unarni_izraz>
	// tip ← <unarni_izraz>.tip
	// l-izraz ← <unarni_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "<unarni_izraz>") {
		// 1. provjeri(<unarni_izraz>)
		unarni_izraz(&root->children.at(0));
		root->type = root->children.at(0).type;
		root->lhs = root->children.at(0).lhs;
	}
	// <cast_izraz> ::= L_ZAGRADA <ime_tipa> D_ZAGRADA <cast_izraz>
	// tip ← <ime_tipa>.tip
	// l-izraz ← 0
	else if (root->children.size() == 4) {
		// 1. provjeri(<ime_tipa>)
		// 2. provjeri(<cast_izraz>)
		// 3. <cast_izraz>.tip se moze pretvoriti u <ime_tipa>.tip
		ime_tipa(&root->children.at(1));
		cast_izraz(&root->children.at(3));
		if (!explicit_conversion(root->children.at(3).type,
								 root->children.at(1).type)) {
			semantic_error(root);
		} else {
			root->type = root->children.at(1).type;
			root->lhs = false;
		}
	} else {
		semantic_error(root);
	}
	return;
}

void ime_tipa(node *root) {
	// <ime_tipa> ::= <specifikator_tipa>
	// tip ← <specifikator_tipa>.tip
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "<specifikator_tipa>") {
		// 1. provjeri(<specifikator_tipa>)
		specifikator_tipa(&root->children.at(0));
		root->type = root->children.at(0).type;
	}
	// <ime_tipa> ::= KR_CONST <specifikator_tipa>
	//  tip ← const(<specifikator_tipa>.tip)
	else if (root->children.size() == 2) {
		// 1. provjeri(<specifikator_tipa>)
		// 2. <specifikator_tipa>.tip != void
		specifikator_tipa(&root->children.at(1));
		if (root->children.at(1).type == "void") {
			semantic_error(root);
		} else {
			std::string X = "const(";
			X += root->children.at(1).type;
			X += ')';
			root->type = X;
		}
	} else {
		semantic_error(root);
	}
	return;
}

void specifikator_tipa(node *root) {
	if (root->children.size() == 1) {
		// <specifikator_tipa> ::= KR_VOID
		// tip ← void
		if (root->children.at(0).symbol == "KR_VOID") {
			root->type = "void";
		}
		// <specifikator_tipa> ::= KR_CHAR
		// tip ← char
		else if (root->children.at(0).symbol == "KR_CHAR") {
			root->type = "char";
		}
		// <specifikator_tipa> ::= KR_INT
		// tip ← int
		else if (root->children.at(0).symbol == "KR_INT") {
			root->type = "int";
		} else {
			semantic_error(root);
		}
	} else {
		semantic_error(root);
	}
	return;
}

void multiplikativni_izraz(node *root) {
	// <multiplikativni_izraz> ::= <cast_izraz>
	// tip ← <cast_izraz>.tip
	// l-izraz ← <cast_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "<cast_izraz>") {
		// 1. provjeri(<cast_izraz>)
		cast_izraz(&root->children.at(0));
		root->type = root->children.at(0).type;
		root->lhs = root->children.at(0).lhs;
	}
	// <multiplikativni_izraz> ::= <multiplikativni_izraz> (OP_PUTA | OP_DIJELI
	// | OP_MOD) <cast_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 (root->children.at(1).symbol == "OP_PUTA" ||
			  root->children.at(1).symbol == "OP_DIJELI" ||
			  root->children.at(1).symbol == "OP_MOD")) {
		// 1. provjeri(<multiplikativni_izraz>)
		// 2. <multiplikativni_izraz>.tip ∼ int
		// 3. provjeri(<cast_izraz>)
		// 4. <cast_izraz>.tip ∼ int
		multiplikativni_izraz(&root->children.at(0));
		if (!implicit_conversion(root->children.at(0).type, "int")) {
			semantic_error(root);
		} else {
			cast_izraz(&root->children.at(2));
			if (!implicit_conversion(root->children.at(2).type, "int")) {
				semantic_error(root);
			} else {
				root->type = "int";
				root->lhs = false;
			}
		}

	} else {
		semantic_error(root);
	}
	return;
}

void aditivni_izraz(node *root) {
	// 	<aditivni_izraz> ::= <multiplikativni_izraz>
	// tip ←<multiplikativni_izraz>.tip
	// l-izraz ← <multiplikativni_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "<multiplikativni_izraz>") {
		// 1. provjeri(<multiplikativni_izraz>)
		multiplikativni_izraz(&root->children.at(0));
		root->type = root->children.at(0).type;
		root->lhs = root->children.at(0).lhs;
	}
	// <aditivni_izraz> ::= <aditivni_izraz> (PLUS | MINUS)
	// <multiplikativni_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 (root->children.at(1).symbol == "PLUS" ||
			  root->children.at(1).symbol == "MINUS")) {
		// 1. provjeri(<aditivni_izraz>)
		// 2. <aditivni_izraz>.tip ∼ int
		// 3. provjeri(<multiplikativni_izraz>)
		// 4. <multiplikativni_izraz>.tip ∼ int
		aditivni_izraz(&root->children.at(0));
		if (!implicit_conversion(root->children.at(0).type, "int")) {
			semantic_error(root);
		} else {
			multiplikativni_izraz(&root->children.at(2));
			if (!implicit_conversion(root->children.at(2).type, "int")) {
				semantic_error(root);
			} else {
				root->type = "int";
				root->lhs = false;
			}
		}
	} else {
		semantic_error(root);
	}
	return;
}

void odnosni_izraz(node *root) {
	// <odnosni_izraz> ::= <aditivni_izraz>
	// tip ← <aditivni_izraz>.tip
	// l-izraz ← <aditivni_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "<aditivni_izraz>") {
		// 1. provjeri(<aditivni_izraz>)
		aditivni_izraz(&root->children.at(0));
		root->type = root->children.at(0).type;
		root->lhs = root->children.at(0).lhs;
	}
	// <odnosni_izraz> ::= <odnosni_izraz> (OP_LT | OP_GT | OP_LTE | OP_GTE)
	// <aditivni_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 (root->children.at(1).symbol == "OP_LT" ||
			  root->children.at(1).symbol == "OP_GT" ||
			  root->children.at(1).symbol == "OP_LTE" ||
			  root->children.at(1).symbol == "OP_GTE")) {
		// 1. provjeri(<odnosni_izraz>)
		// 2. <odnosni_izraz>.tip ∼ int
		// 3. provjeri(<aditivni_izraz>)
		// 4. <aditivni_izraz>.tip ∼ int
		odnosni_izraz(&root->children.at(0));
		if (!implicit_conversion(root->children.at(0).type, "int")) {
			semantic_error(root);
		} else {
			aditivni_izraz(&root->children.at(2));
			if (!implicit_conversion(root->children.at(2).type, "int")) {
				semantic_error(root);
			} else {
				root->type = "int";
				root->lhs = false;
			}
		}
	} else {
		semantic_error(root);
	}
	return;
}

void jednakosni_izraz(node *root) {
	// <jednakosni_izraz> ::= <odnosni_izraz>
	// tip ← <odnosni_izraz>.tip
	// l-izraz ← <odnosni_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "<odnosni_izraz>") {
		// 1. provjeri(<odnosni_izraz>)
		odnosni_izraz(&root->children.at(0));
		root->type = root->children.at(0).type;
		root->lhs = root->children.at(0).lhs;
	}
	// <jednakosni_izraz> ::= <jednakosni_izraz> (OP_EQ | OP_NEQ)
	// <odnosni_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 (root->children.at(1).symbol == "OP_EQ" ||
			  root->children.at(1).symbol == "OP_NEQ")) {
		// 1. provjeri(<jednakosni_izraz>)
		// 2. <jednakosni_izraz>.tip ∼ int
		// 3. provjeri(<odnosni_izraz>)
		// 4. <odnosni_izraz>.tip ∼ int
		jednakosni_izraz(&root->children.at(0));
		if (!implicit_conversion(root->children.at(0).type, "int")) {
			semantic_error(root);
		} else {
			odnosni_izraz(&root->children.at(2));
			if (!implicit_conversion(root->children.at(2).type, "int")) {
				semantic_error(root);
			} else {
				root->type = "int";
				root->lhs = false;
			}
		}
	} else {
		semantic_error(root);
	}
	return;
}

void bin_i_izraz(node *root) {
	// <bin_i_izraz> ::= <jednakosni_izraz>
	// tip ← <jednakosni_izraz>.tip
	// l-izraz ← <jednakosni_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "<jednakosni_izraz>") {
		// 1. provjeri(<jednakosni_izraz>)
		jednakosni_izraz(&root->children.at(0));
		root->type = root->children.at(0).type;
		root->lhs = root->children.at(0).lhs;
	}
	// <bin_i_izraz> ::= <bin_i_izraz> OP_BIN_I <jednakosni_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 root->children.at(1).symbol == "OP_BIN_I") {
		// 1. provjeri(<bin_i_izraz>)
		// 2. <bin_i_izraz>.tip ∼ int
		// 3. provjeri(<jednakosni_izraz>)
		// 4. <jednakosni_izraz>.tip ∼ int
		bin_i_izraz(&root->children.at(0));
		if (!implicit_conversion(root->children.at(0).type, "int")) {
			semantic_error(root);
		} else {
			jednakosni_izraz(&root->children.at(2));
			if (!implicit_conversion(root->children.at(2).type, "int")) {
				semantic_error(root);
			} else {
				root->type = "int";
				root->lhs = false;
			}
		}
	} else {
		semantic_error(root);
	}
	return;
}

void bin_xili_izraz(node *root) {
	// 	<bin_xili_izraz> ::= <bin_i_izraz>
	// tip ← <bin_i_izraz>.tip
	// l-izraz ← <bin_i_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "<bin_i_izraz>") {
		// 1. provjeri(<bin_i_izraz>)
		bin_i_izraz(&root->children.at(0));
		root->type = root->children.at(0).type;
		root->lhs = root->children.at(0).lhs;
	}
	// 	<bin_xili_izraz> ::= <bin_xili_izraz> OP_BIN_XILI <bin_i_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3) {
		// 1. provjeri(<bin_xili_izraz>)
		// 2. <bin_xili_izraz>.tip ∼ int
		// 3. provjeri(<bin_i_izraz>)
		// 4. <bin_i_izraz>.tip ∼ int
		bin_xili_izraz(&root->children.at(0));
		if (!implicit_conversion(root->children.at(0).type, "int")) {
			semantic_error(root);
		} else {
			bin_i_izraz(&root->children.at(2));
			if (!implicit_conversion(root->children.at(2).type, "int")) {
				semantic_error(root);
			} else {
				root->type = "int";
				root->lhs = false;
			}
		}
	} else {
		semantic_error(root);
	}
	return;
}

void bin_ili_izraz(node *root) {
	// <bin_ili_izraz> ::= <bin_xili_izraz>
	// tip ← <bin_xili_izraz>.tip
	// l-izraz ← <bin_xili_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "<bin_xili_izraz>") {
		// 1. provjeri(<bin_xili_izraz>)
		bin_xili_izraz(&root->children.at(0));
		root->type = root->children.at(0).type;
		root->lhs = root->children.at(0).lhs;
	}
	// <bin_ili_izraz> ::= <bin_ili_izraz> OP_BIN_ILI <bin_xili_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 root->children.at(1).symbol == "OP_BIN_ILI") {
		// 1. provjeri(<bin_ili_izraz>)
		// 2. <bin_ili_izraz >.tip ∼ int
		// 3. provjeri(<bin_xili_izraz>)
		// 4. <bin_xili_izraz >.tip ∼ int
		bin_ili_izraz(&root->children.at(0));
		if (!implicit_conversion(root->children.at(0).type, "int")) {
			semantic_error(root);
		} else {
			bin_xili_izraz(&root->children.at(2));
			if (!implicit_conversion(root->children.at(2).type, "int")) {
				semantic_error(root);
			} else {
				root->type = "int";
				root->lhs = false;
			}
		}
	} else {
		semantic_error(root);
	}
	return;
}

void log_i_izraz(node *root) {
	// <log_i_izraz> ::= <bin_ili_izraz>
	// tip ← <bin_ili_izraz>.tip
	// l-izraz ← <bin_ili_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "<bin_ili_izraz>") {
		// 1. provjeri(<bin_ili_izraz>)
		bin_ili_izraz(&root->children.at(0));
		root->type = root->children.at(0).type;
		root->lhs = root->children.at(0).lhs;
	}
	// <log_i_izraz> ::= <log_i_izraz> OP_I <bin_ili_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 root->children.at(1).symbol == "OP_I") {
		// 1. provjeri(<log_i_izraz>)
		// 2. <log_i_izraz>.tip ∼ int
		// 3. provjeri(<bin_ili_izraz>)
		// 4. <bin_ili_izraz>.tip ∼ int
		log_i_izraz(&root->children.at(0));
		if (!implicit_conversion(root->children.at(0).type, "int")) {
			semantic_error(root);
		} else {
			bin_ili_izraz(&root->children.at(2));
			if (!implicit_conversion(root->children.at(2).type, "int")) {
				semantic_error(root);
			} else {
				root->type = "int";
				root->lhs = false;
			}
		}
	} else {
		semantic_error(root);
	}
	return;
}

void log_ili_izraz(node *root) {
	// <log_ili_izraz> ::= <log_i_izraz>
	// tip ← <log_i_izraz>.tip
	// l-izraz ← <log_i_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "<log_i_izraz>") {
		// 1. provjeri(<log_i_izraz>)
		log_i_izraz(&root->children.at(0));
		root->type = root->children.at(0).type;
		root->lhs = root->children.at(0).lhs;
	}
	// <log_ili_izraz> ::= <log_ili_izraz> OP_ILI <log_i_izraz>
	// tip ← int
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 root->children.at(1).symbol == "OP_ILI") {
		// 1. provjeri(<log_ili_izraz>)
		// 2. <log_ili_izraz>.tip ∼ int
		// 3. provjeri(<log_i_izraz>)
		// 4. <log_i_izraz>.tip ∼ int
		log_ili_izraz(&root->children.at(0));
		if (!implicit_conversion(root->children.at(0).type, "int")) {
			semantic_error(root);
		} else {
			log_i_izraz(&root->children.at(2));
			if (!implicit_conversion(root->children.at(2).type, "int")) {
				semantic_error(root);
			} else {
				root->type = "int";
				root->lhs = false;
			}
		}
	} else {
		semantic_error(root);
	}
	return;
}

void izraz_pridruzivanja(node *root) {
	// <izraz_pridruzivanja> ::= <log_ili_izraz>
	// tip ← <log_ili_izraz>.tip
	// l-izraz ← <log_ili_izraz>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "<log_ili_izraz>") {
		// 1. provjeri(<log_ili_izraz>)
		log_ili_izraz(&root->children.at(0));
		root->type = root->children.at(0).type;
		root->lhs = root->children.at(0).lhs;
	}
	// <izraz_pridruzivanja> ::= <postfiks_izraz> OP_PRIDRUZI
	// <izraz_pridruzivanja>
	// tip ← <postfiks_izraz>.tip
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 root->children.at(1).symbol == "OP_PRIDRUZI") {
		// 1. provjeri(<postfiks_izraz>)
		// 2. <postfiks_izraz>.l-izraz = 1
		// 3. provjeri(<izraz_pridruzivanja>)
		// 4. <izraz_pridruzivanja>.tip ∼ <postfiks_izraz>.tip
		postfiks_izraz(&root->children.at(0));
		if (!root->children.at(0).lhs) {
			semantic_error(root);
		} else {
			izraz_pridruzivanja(&root->children.at(2));
			if (!implicit_conversion(root->children.at(2).type,
									 root->children.at(0).type)) {
				semantic_error(root);
			} else {
				root->type = root->children.at(0).type;
				root->lhs = false;
			}
		}
	} else {
		semantic_error(root);
	}
	return;
}

void izraz(node *root) {
	// <izraz> ::= <izraz_pridruzivanja>
	// tip ← <izraz_pridruzivanja>.tip
	// l-izraz ← <izraz_pridruzivanja>.l-izraz
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "<izraz_pridruzivanja>") {
		// 1. provjeri(<izraz_pridruzivanja>)
		izraz_pridruzivanja(&root->children.at(0));
		root->type = root->children.at(0).type;
		root->lhs = root->children.at(0).lhs;
	}
	// <izraz> ::= <izraz> ZAREZ <izraz_pridruzivanja>
	// tip ← <izraz_pridruzivanja>.tip
	// l-izraz ← 0
	else if (root->children.size() == 3 &&
			 root->children.at(0).symbol == "<izraz>" &&
			 root->children.at(1).symbol == "ZAREZ" &&
			 root->children.at(2).symbol == "<izraz_pridruzivanja>") {
		// 1. provjeri(<izraz>)
		// 2. provjeri(<izraz_pridruzivanja>)
		izraz(&root->children.at(0));
		izraz_pridruzivanja(&root->children.at(2));
		root->type = root->children.at(2).type;
		root->lhs = false;
	} else {
		semantic_error(root);
	}
	return;
}

void slozena_naredba(node *root) {
	// <slozena_naredba> ::= L_VIT_ZAGRADA <lista_naredbi> D_VIT_ZAGRADA
	if (root->children.size() == 3 &&
		root->children.at(1).symbol == "<lista_naredbi>") {
		// 1. provjeri(<lista_naredbi>)
		lista_naredbi(&root->children.at(1));
	}
	// <slozena_naredba> ::= L_VIT_ZAGRADA <lista_deklaracija> <lista_naredbi>
	// D_VIT_ZAGRADA
	else if (root->children.size() == 3 &&
			 root->children.at(1).symbol == "<lista_deklaracija>" &&
			 root->children.at(2).symbol == "<lista_naredbi>") {
		// 1. provjeri(<lista_deklaracija>)
		// 2. provjeri(<lista_naredbi>)
		lista_deklaracija(&root->children.at(1));
		lista_naredbi(&root->children.at(2));
	} else {
		semantic_error(root);
	}
	return;
}

void lista_naredbi(node *root) {
	// <lista_naredbi> ::= <naredba>
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "<naredba>") {
		// 1. provjeri(<naredba>)
		naredba(&root->children.at(0));
	}
	// <lista_naredbi> ::= <lista_naredbi> <naredba>
	else if (root->children.size() == 2 &&
			 root->children.at(0).symbol == "<lista_naredbi>" &&
			 root->children.at(1).symbol == "<naredba>") {
		// 1. provjeri(<lista_naredbi>)
		// 2. provjeri(<naredba>)
		lista_naredbi(&root->children.at(0));
		naredba(&root->children.at(1));
	} else {
		semantic_error(root);
	}
	return;
}

void naredba(node *root) {
	// Nezavrsni znak <naredba> generira blokove (<slozena_naredba>) i
	// razlicite vrste jednostavnih naredbi (<izraz_naredba>,
	// <naredba_grananja>, <naredba_petlje> i <naredba_skoka>). Kako su sve
	// produkcije jedinicne (s desne strane imaju jedan nezavrsni znak) i u
	// svim produkcijama se provjeravaju semanticka pravila na znaku s desne
	// strane, produkcije ovdje nisu prikazane.
	if (root->children.size() == 1) {
		if (root->children.at(0).symbol == "<slozena_naredba>") {
			slozena_naredba(&root->children.at(0));
		} else if (root->children.at(0).symbol == "<izraz_naredba>") {
			izraz_naredba(&root->children.at(0));
		} else if (root->children.at(0).symbol == "<naredba_grananja>") {
			naredba_grananja(&root->children.at(0));
		} else if (root->children.at(0).symbol == "<naredba_petlje>") {
			naredba_petlje(&root->children.at(0));
		} else if (root->children.at(0).symbol == "<naredba_skoka>") {
			naredba_skoka(&root->children.at(0));
		} else {
			semantic_error(root);
		}
	} else {
		semantic_error(root);
	}
	return;
}

void izraz_naredba(node *root) {
	// <izraz_naredba> ::= TOCKAZAREZ
	// tip ← int
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "TOCKAZAREZ") {
		root->type = "int";
	}
	// 	<izraz_naredba> ::= <izraz> TOCKAZAREZ
	// tip ← <izraz>.tip
	else if (root->children.size() == 2 &&
			 root->children.at(0).symbol == "<izraz>" &&
			 root->children.at(1).symbol == "TOCKAZAREZ") {
		// 1. provjeri(<izraz>)
		izraz(&root->children.at(0));
		root->type = root->children.at(0).type;
	} else {
		semantic_error(root);
	}
	return;
}

void naredba_grananja(node *root) {
	// 	<naredba_grananja> ::= KR_IF L_ZAGRADA <izraz> D_ZAGRADA <naredba>
	if (root->children.size() == 5) {
		// 1. provjeri(<izraz>)
		// 2. <izraz>.tip ∼ int
		// 3. provjeri(<naredba>)
		izraz(&root->children.at(2));
		if (!implicit_conversion(root->children.at(2).type, "int")) {
			semantic_error(root);
		} else {
			naredba(&root->children.at(4));
		}
	}
	// <naredba_grananja> ::= KR_IF L_ZAGRADA <izraz> D_ZAGRADA <naredba>1
	// KR_ELSE <naredba>2
	else if (root->children.size() == 7) {
		// 1. provjeri(<izraz>)
		// 2. <izraz>.tip ∼ int
		// 3. provjeri(<naredba>1)
		// 4. provjeri(<naredba>2)
		izraz(&root->children.at(2));
		if (!implicit_conversion(root->children.at(2).type, "int")) {
			semantic_error(root);
		} else {
			naredba(&root->children.at(4));
			naredba(&root->children.at(6));
		}
	} else {
		semantic_error(root);
	}
	return;
}

void naredba_petlje(node *root) {
	++loop_depth;
	// <naredba_petlje> ::= KR_WHILE L_ZAGRADA <izraz> D_ZAGRADA <naredba>
	if (root->children.size() == 5) {
		// 1. provjeri(<izraz>)
		// 2. <izraz>.tip ∼ int
		// 3. provjeri(<naredba>)
		izraz(&root->children.at(2));
		if (!implicit_conversion(root->children.at(2).type, "int")) {
			semantic_error(root);
		} else {
			naredba(&root->children.at(4));
		}
	}
	// <naredba_petlje> ::= KR_FOR L_ZAGRADA <izraz_naredba>1 <izraz_naredba>2
	// D_ZAGRADA <naredba>
	else if (root->children.size() == 6) {
		// 1. provjeri(<izraz_naredba>1)
		// 2. provjeri(<izraz_naredba>2)
		// 3.<izraz_naredba>2.tip ∼ int
		// 4. provjeri(<naredba>)
		izraz_naredba(&root->children.at(2));
		izraz_naredba(&root->children.at(3));
		if (!implicit_conversion(root->children.at(3).type, "int")) {
			semantic_error(root);
		} else {
			naredba(&root->children.at(5));
		}
	}
	// <naredba_petlje> ::= KR_FOR L_ZAGRADA <izraz_naredba>1 <izraz_naredba>2
	// <izraz> D_ZAGRADA <naredba>
	else if (root->children.size() == 7) {
		// 1. provjeri(<izraz_naredba>1)
		// 2. provjeri(<izraz_naredba>2)
		// 3. <izraz_naredba>2.tip ∼ int
		// 4. provjeri(<izraz>)
		// 5. provjeri(<naredba>)
		izraz_naredba(&root->children.at(2));
		izraz_naredba(&root->children.at(3));
		if (!implicit_conversion(root->children.at(3).type, "int")) {
			semantic_error(root);
		} else {
			izraz(&root->children.at(4));
			naredba(&root->children.at(6));
		}
	} else {
		semantic_error(root);
	}
	--loop_depth;
	return;
}

void naredba_skoka(node *root) {
	// <naredba_skoka> ::= (KR_CONTINUE | KR_BREAK) TOCKAZAREZ
	if (root->children.size() == 2 &&
		(root->children.at(0).symbol == "KR_CONTINUE" ||
		 root->children.at(0).symbol == "KR_BREAK") &&
		root->children.at(1).symbol == "TOCKAZAREZ") {
		// 1. naredba se nalazi unutar petlje ili unutar bloka koji je
		// ugnijezden u petlji
		if (loop_depth == 0) {
			semantic_error(root);
		}
	}
	// <naredba_skoka> ::= KR_RETURN TOCKAZAREZ
	else if (root->children.size() == 2 &&
			 root->children.at(0).symbol == "KR_RETURN" &&
			 root->children.at(1).symbol == "TOCKAZAREZ") {
		// 1. naredba se nalazi unutar funkcije tipa funkcija(params → void)
		if (current_function_return_type != "void") {
			semantic_error(root);
		}

	}
	// <naredba_skoka> ::= KR_RETURN <izraz> TOCKAZAREZ
	else if (root->children.size() == 3) {
		// 1. provjeri(<izraz>)
		// 2. naredba se nalazi unutar funkcije tipa funkcija(params → pov) i
		// vrijedi <izraz>.tip ∼ pov
		izraz(&root->children.at(1));
		if (current_function_return_type == "void" ||
			current_function_return_type == "" ||
			!implicit_conversion(root->children.at(1).type,
								 current_function_return_type)) {
			semantic_error(root);
		}
	} else {
		semantic_error(root);
	}
	return;
}

void prijevodna_jedinica(node *root) {
	// <prijevodna_jedinica> ::= <vanjska_deklaracija>
	if (root->children.size() == 1 &&
		root->children.at(0).symbol == "<vanjska_deklaracija>") {
		// 1. provjeri(<vanjska_deklaracija>)
		vanjska_deklaracija(&root->children.at(0));
	}
	// <prijevodna_jedinica> ::= <prijevodna_jedinica> <vanjska_deklaracija>
	else if (root->children.size() == 2 &&
			 root->children.at(0).symbol == "<prijevodna_jedinica>" &&
			 root->children.at(1).symbol == "<vanjska_deklaracija>") {
		// 1. provjeri(<prijevodna_jedinica>)
		// 2. provjeri(<vanjska_deklaracija>)
		prijevodna_jedinica(&root->children.at(0));
		vanjska_deklaracija(&root->children.at(1));
	} else {
		semantic_error(root);
	}
	return;
}

void vanjska_deklaracija(node *root) {
	// Nezavrsni znak <vanjska_deklaracija> generira ili definiciju funkcije
	// (znak <definicija_funkcije>) ili deklaraciju varijable ili funkcije (znak
	// <deklaracija>). Obje produkcije su jedinicne
	// i u obje se provjeravaju pravila u podstablu kojem je znak s desne strane
	// korijen.
	if (root->children.size() == 1) {
		if (root->children.at(0).symbol == "<definicija_funkcije>") {
			definicija_funkcije(&root->children.at(0));
		} else if (root->children.at(0).symbol == "<deklaracija>") {
			deklaracija(&root->children.at(0));
		} else {
			semantic_error(root);
		}
	} else {
		semantic_error(root);
	}
	return;
}

int main(void) {
	std::string line;
	while (std::getline(std::cin, line)) {
		int depth = 0;
		node current_node;

		// depth of current node
		while (line.at(depth) == ' ') {
			++depth;
		}
		current_node.depth = depth;
		if (line.at(depth) == '<') {
			current_node.row = -1;
			current_node.symbol = line.substr(depth);
		} else {
			line = line.substr(depth);
			current_node.symbol = line.substr(0, line.find(' '));
			line = line.substr(line.find(' ') + 1);
			current_node.row = std::stoi(line.substr(0, line.find(' ')));
			line = line.substr(line.find(' ') + 1);
			current_node.value = line;
		}
		if (all_nodes.empty()) {
			all_nodes.push_back(current_node);
			continue;
		}
		size_t iter;
		for (iter = all_nodes.size() - 1;
			 iter >= 0 && all_nodes.at(iter).depth != depth - 1; --iter)
			;
		all_nodes.at(iter).children.push_back(current_node);
		all_nodes.push_back(current_node);
	}
	// primarni_izraz(&all_nodes.at(0));
	// assuming there is something in the input if not it it correct ?
	// prijevodna_jedinica(all_nodes.at(0));

	return 0;
}
