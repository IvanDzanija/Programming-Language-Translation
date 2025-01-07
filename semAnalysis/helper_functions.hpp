#include <string>
#include <vector>

void debug();
bool accepted_char(std::string to_check);
bool implicit_conversion(std::string t1, std::string t2);
bool explicit_conversion(std::string t1, std::string t2);
bool is_array(std::string type);
bool is_const(std::string type);
bool same_arguments(std::vector<std::string> args1,
					std::vector<std::string> args2);
int string_length(std::string to_check);
std::string remove_const(std::string type);
std::string remove_array(std::string type);
