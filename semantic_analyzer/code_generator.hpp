#include <cstdint>
#include <string>
#include <unordered_map>

extern std::unordered_map<std::string, std::pair<std::string, std::string>>
	code_global_variables;
extern std::string current_global_variable;
extern std::unordered_map<std::string, std::string> code_constants;
extern std::unordered_map<std::string, std::string> code_functions;

void code_init(void);
void return_sp(void);
void load_ret_val(void);
void save_context(void);
void refresh_context(void);
void unary_operation(int8_t op);
void binary_operation(int8_t op);
void bitwise_operation(uint8_t op);
void logical_operation(uint8_t op);
void fill_globals(void);
void fill_consts(void);
void load_const(std::string var);
