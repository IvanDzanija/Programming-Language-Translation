#include <iostream>
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

std::unordered_map<std::string, std::string> regexes;
std::unordered_set<std::string> lex_states;
std::unordered_set<std::string> lex_names;
std::vector<Rule> rules;

void reg_def(std::string line) {
	std::string key = line.substr(1, line.find('}') - 1);
	std::string value = line.substr(line.find(' ') + 1);
	size_t beg = value.find('{');
	size_t end = value.find('}');
	std::string final_value;
	while (beg != std::string::npos && end != std::string::npos) {
		// std::cout << beg << ' ' << end << std::endl;
		std::string reg_key = value.substr(beg + 1, end - beg - 1);
		final_value.append(value.substr(0, beg) + '(' + regexes[reg_key] + ')');

		value = value.substr(end + 1);
		beg = value.find('{');
		end = value.find('}');
	}
	final_value.append(value);
	// std::cout << final_value << std::endl;
	regexes[key] = final_value;
}

int main(void) {
	std::string line;
	std::string read_state = "reg_def";
	std::string state_input;
	std::string regex_input;
	std::vector<std::string> args(3, "");

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
			reg_def(line);
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
				args = {"", "", ""};
				continue;
			} else if (line.at(0) == '}') {
				Rule curr(state_input, regex_input, args);
				rules.push_back(curr);
				continue;
			}
			size_t beg = line.find('<');
			size_t end = line.find('>');
			if (beg != std::string::npos && end != std::string::npos) {
				state_input = line.substr(beg + 1, end - beg - 1);
				regex_input = line.substr(end + 1);
			} else {
				args.at(arg_num) = line;
				++arg_num;
			}
		}
	}
	// for (auto x : lex_states) {
	// 	std::cout << x << std::endl;
	// }
	// for (auto x : lex_names) {
	// 	std::cout << x << std::endl;
	// }
	// for (auto x : rules) {
	// 	std::cout << x.get_state() << ' ' << x.get_regex() << std::endl;
	// 	for (auto y : x.get_args()) {
	// 		std::cout << y << std::endl;
	// 	}
	// }

	return 0;
}
