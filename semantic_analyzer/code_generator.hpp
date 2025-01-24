#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

extern std::unordered_map<std::string, std::string> code_global_variables;
extern std::string current_global_variable;
extern std::string current_global_array;
extern std::unordered_map<std::string, std::pair<std::string, int>>
	code_global_arrays;
extern std::unordered_map<int, std::string> code_constants;
extern std::unordered_map<std::string, std::string> code_functions;
extern std::unordered_multimap<std::string, int> code_local_variables;
extern std::unordered_multimap<std::string, std::pair<int, int>>
	code_local_arrays;
extern std::unordered_map<std::string, int> global_var_init;

void code_init(void);
void return_sp(void);
void fn_def(std::string name, int argc);
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
void load_var(std::string name);
void load_array(std::string name);
void store_global(std::string name);
void call_fn(std::string name, std::vector<std::string>);
