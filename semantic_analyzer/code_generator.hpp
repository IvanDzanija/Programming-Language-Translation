#include <cstdint>
#include <map>
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
extern std::multimap<std::string, int> code_local_variables;
extern std::unordered_multimap<std::string, std::pair<int, int>>
	code_local_arrays;
extern std::unordered_map<std::string, int> global_var_init;
extern std::unordered_map<std::string, std::vector<int>> global_arr_init;

void code_init(void);
void return_sp(void);
void fn_def(std::string name, int argc);
void load_ret_val(void);
void save_context(void);
void refresh_context(void);
void unary_operation(int8_t op);
void binary_operation(int8_t op);
void bitwise_operation(uint8_t op);
void logical_start(uint8_t op);
void logical_check(uint8_t op);
void logical_end(void);
void fill_globals(void);
void fill_consts(void);
void load_const(std::string var);
void load_var(std::string name);
void load_array(std::string name);
void push_ret_val(void);
void store_global_var(std::string name);
void store_local_var(std::string name);
void store_global_arr(std::string name, int index);
void call_fn(std::string name, size_t);
void branch_if(void);
void branch_else(void);
void equal_comparison(bool eq);
void relation_comparison(std::string op);
void variable_increment_before(std::string, bool plus);
void while_start(void);
void while_check(void);
void while_end(void);
