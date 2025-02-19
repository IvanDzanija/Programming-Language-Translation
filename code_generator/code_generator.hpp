#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

extern std::unordered_map<std::string, std::string> code_global_variables;
extern std::multimap<std::string, std::pair<std::string, int>>
	code_global_arrays;
extern std::unordered_map<int, std::string> code_constants;
extern std::unordered_map<std::string, std::string> code_functions;
extern std::multimap<std::string, int> code_local_variables;
extern std::unordered_map<std::string, std::vector<std::string>>
	global_var_init;
extern std::unordered_map<std::string, std::vector<std::vector<std::string>>>
	global_arr_init;
extern std::vector<std::pair<std::string, bool>> increment_after;
extern std::unordered_map<int, std::vector<std::pair<std::string, int>>>
	for_var_update;
extern std::unordered_map<std::string, int> function_arrays;

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
void store_local_arr(std::string name, int index);
void store_func_arr(std::string name, int index);
void call_fn(std::string name, size_t);
void branch_if(void);
void branch_else(void);
void equal_comparison(bool eq);
void relation_comparison(std::string op);
void variable_increment_before(std::string, bool plus);
void variable_increment_after(void);
void while_start(void);
void while_check(void);
void while_end(void);
void forc_start(void);
void forc_check(void);
void forc_end(void);
void forc_skip_first(void);
void forc_skip_second(void);
void operation_div(void);
void operation_mul(void);
void operation_mod(void);
void send_arr(std::string name);
void not_operator();
void tilde_operator();
void push_empty_var();
void return_to_stack(bool array);
