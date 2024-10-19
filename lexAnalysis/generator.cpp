#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
using ll = int64_t;

class Rule {
  private:
	std::string state;
	std::string regex;
	std::vector<std::string> args;

  public:
	Rule() {};
	Rule(const std::string &state, const std::string &regex,
		 const std::vector<std::string> &arr) {
		this->state = state;
		this->regex = regex;
		this->args = arr;
	}
	std::string get_state() const { return state; }
	std::string get_regex() const { return regex; }
	std::vector<std::string> get_args() const { return args; }
};

int global_state;

class Automata {
  public:
	Rule rule;
	int start;
	int accepted;
	int state_count;
	std::unordered_set<int> states;
	std::multimap<std::pair<int, char>, int> transitions;

  public:
	Automata(Rule rule) {
		this->state_count = 0;
		this->rule = rule;
		std::pair<int, int> res = convert(this->rule.get_regex());
		this->start = res.first;
		this->accepted = res.second;
	}

	void new_trans(std::pair<int, int> states, char sign) {
		int current = states.first;
		int next = states.second;
		std::pair<int, char> pair = std::make_pair(current, sign);
		transitions.emplace(pair, next);
	}

	int new_state() {

		++this->state_count;
		this->states.insert(global_state);
		++global_state;
		return global_state - 1;
	}
	bool is_operator(std::string exp, int i) {
		int cnt = 0;
		while (i - 1 >= 0 && exp.at(i - 1) == '\\') {
			++cnt;
			--i;
		}
		return cnt % 2 == 0;
	}
	std::pair<int, int> convert(std::string expr) {
		std::vector<std::string> choices;
		int brackets = 0;
		int start = 0;
		for (int i = 0; i < expr.size(); ++i) {
			if (expr.at(i) == '(' && is_operator(expr, i)) {
				++brackets;
			} else if (expr.at(i) == ')' && is_operator(expr, i)) {
				--brackets;
			} else if (brackets == 0 && expr.at(i) == '|' &&
					   is_operator(expr, i)) {
				choices.push_back(expr.substr(start, i - start));
				start = i + 1;
			}
		}

		if (choices.size() > 0) {
			choices.push_back(expr.substr(start, expr.size() - start));
		}
		int first = new_state();
		int second = new_state();
		if (choices.size() > 0) {
			for (int i = 0; i < choices.size(); ++i) {
				std::pair<int, int> temp = convert(choices.at(i));
				new_trans(std::make_pair(first, temp.first), '$');
				new_trans(std::make_pair(second, temp.second), '$');
			}
		} else {
			bool prefixed = false;
			int last_state = first;
			for (int i = 0; i < expr.size(); ++i) {
				int a, b;
				if (prefixed) {
					prefixed = false;
					char trans_sign;
					if (expr.at(i) == 't') {
						trans_sign = '\t';
					} else if (expr.at(i) == 'n') {
						trans_sign = '\n';
					} else if (expr.at(i) == '_') {
						trans_sign = ' ';
					} else {
						trans_sign = expr.at(i);
					}
					a = new_state();
					b = new_state();
					new_trans(std::make_pair(a, b), trans_sign);
				} else {
					if (expr.at(i) == '\\') {
						prefixed = true;
						continue;
					}
					if (expr.at(i) != '(') {
						a = new_state();
						b = new_state();
						new_trans(std::make_pair(a, b), expr.at(i));
					} else {
						bool closed_bracket = false;
						int nested_brackets = 0;
						int j = i + 1;
						while (closed_bracket == false) {
							if (nested_brackets == 0 && expr.at(j) == ')' &&
								is_operator(expr, j)) {
								closed_bracket = true;

							} else if (expr.at(j) == '(' &&
									   is_operator(expr, j)) {
								++nested_brackets;

							} else if (expr.at(j) == ')' &&
									   is_operator(expr, j)) {
								--nested_brackets;
							}
							if (!closed_bracket) {
								++j;
							}
						}

						std::pair<int, int> temp =
							convert(expr.substr(i + 1, j - i - 1));
						a = temp.first;
						b = temp.second;
						i = j;
					}
				}
				if (i + 1 < expr.size() && expr.at(i + 1) == '*') {
					int x = a;
					int y = b;
					a = new_state();
					b = new_state();
					new_trans(std::make_pair(a, x), '$');
					new_trans(std::make_pair(y, b), '$');
					new_trans(std::make_pair(a, b), '$');
					new_trans(std::make_pair(y, x), '$');
					++i;
				}
				new_trans(std::make_pair(last_state, a), '$');
				last_state = b;
			}
			new_trans(std::make_pair(last_state, second), '$');
		}

		return std::make_pair(first, second);
	}
};

std::unordered_map<std::string, std::string> regexes;
std::unordered_set<std::string> lex_states;
std::unordered_set<std::string> lex_names;
std::vector<Rule> rules;
std::vector<Automata> atms;

std::string reg_def(std::string value) {
	size_t beg = value.find('{');
	size_t end = value.find('}');
	std::string final_value;
	while (beg != std::string::npos && end != std::string::npos) {
		// std::cout << beg << ' ' << end << std::endl;
		std::string reg_key = value.substr(beg + 1, end - beg - 1);
		if (regexes.count(reg_key) > 0) {
			final_value.append(value.substr(0, beg) + '(' + regexes[reg_key] +
							   ')');
		} else {
			final_value.append(value.substr(0, end + 1));
		}

		value = value.substr(end + 1);
		beg = value.find('{');
		end = value.find('}');
	}
	final_value.append(value);
	return final_value;
}

int main(void) {
	global_state = 0;
	std::string line;
	std::string read_state = "reg_def";
	std::string state_input;
	std::string regex_input;
	std::vector<std::string> args(4, "");

	int arg_num = 0;
	while (std::getline(std::cin, line)) {
		// Check definition order
		if (line.substr(0, 2) == "%X") {
			read_state = "states_def";
			line = line.substr(3);
		} else if (line.substr(0, 2) == "%L") {
			read_state = "name_def";
			line = line.substr(3);
		} else if (line.at(0) == '<') {
			read_state = "rule_def";
		}

		// Parse input_file
		if (read_state == "reg_def") {
			std::string key = line.substr(1, line.find('}') - 1);
			std::string value = line.substr(line.find(' ') + 1);
			std::string final_value = reg_def(value);
			regexes[key] = final_value;
		} else if (read_state == "states_def") {
			std::stringstream ss(line);
			std::string token;
			while (std::getline(ss, token, ' ')) {
				lex_states.insert(token);
			}

		} else if (read_state == "name_def") {
			std::stringstream ss(line);
			std::string token;
			while (std::getline(ss, token, ' ')) {
				lex_names.insert(token);
			}

		} else {
			if (line.at(0) == '{') {
				arg_num = 0;
				args = {"", "", "", ""};
				continue;
			} else if (line.at(0) == '}') {
				Rule currRule(state_input, regex_input, args);
				rules.push_back(currRule);
				Automata currAtm(currRule);
				atms.push_back(currAtm);
				continue;
			}
			size_t beg = line.find('<');
			size_t end = line.find('>');
			if (beg != std::string::npos && end != std::string::npos) {
				state_input = line.substr(beg + 1, end - beg - 1);
				regex_input = line.substr(end + 1);
				regex_input = reg_def(regex_input);
			} else {
				args.at(arg_num) = line;
				++arg_num;
			}
		}
	}
	std::ofstream file("analizator/automat.tab");
	if (file.is_open()) {
		for (auto x : atms) {
			file << x.rule.get_state() << "\n";
			for (auto y : x.rule.get_args()) {
				file << y << '\n';
			}

			// file << "STATE_COUNT " << x.state_count << '\n';
			// file << "STATES\n";
			// for (auto y : x.states) {
			// 	file << y << '\n';
			// }
			file << "START " << x.start << '\n';
			file << "ACCEPTED " << x.accepted << '\n';
			file << "TRANSITIONS\n";
			for (auto y : x.transitions) {
				auto z = y.first;
				if (z.second == '\n') {
					file << z.first << ' ' << "|n" << ' ' << y.second << '\n';
					continue;
				} else if (z.second == '\t') {
					file << z.first << ' ' << "|t" << ' ' << y.second << '\n';
					continue;
				} else if (z.second == ' ') {
					file << z.first << ' ' << "|_" << ' ' << y.second << '\n';
					continue;

				} else {
					file << z.first << ' ' << z.second << ' ' << y.second
						 << '\n';
				}
			}
		}
	} else {
		std::cerr << "Unable to open file.\n";
	}
	file.close();

	// for (auto x : atms) {
	// 	for (auto y : x.transitions) {
	// 		auto z = y.first;
	// 		for (auto w : y.second) {
	// 			std::cout << z.first << "->" << z.second << w << std::endl;
	// 		}
	// 	}
	// }
	// for (auto reg : regexes) {
	// 	std::cout << reg.first << ": " << reg.second << std::endl;
	// }
	// for (auto st : lex_states) {
	// 	std::cout << st << std::endl;
	// }
	// for (auto nm : lex_names) {
	// 	std::cout << nm << std::endl;
	// }
	// for (auto x : rules) {
	// 	std::cout << x.get_state() << ' ' << x.get_regex() << std::endl;
	// 	for (auto y : x.get_args()) {
	// 		std::cout << y << std::endl;
	// 	}
	// 	std::cout << std::endl;
	// }

	return 0;
}
