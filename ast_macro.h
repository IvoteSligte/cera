#define UPPER_name NAME
#define UPPER_integer INTEGER
#define UPPER_string STRING
#define UPPER_unary UNARY
#define UPPER_binary BINARY
#define UPPER_function_call FUNCTION_CALL
#define UPPER_function FUNCTION
#define UPPER_param PARAM
#define UPPER_for_loop FOR_LOOP
#define UPPER_assign ASSIGN
#define UPPER_return_stmt RETURN_STMT
#define UPPER_declaration DECLARATION
#define UPPER_module MODULE

#define CASE(name, ...)                                                        \
  case UPPER_##name: {                                                         \
    __auto_type name = node->name;                                             \
    __VA_ARGS__                                                                \
    break;                                                                     \
  }

#define ITER_ARRAY(start_index, length, element_name, ...)                     \
  {                                                                            \
    size_t element_name##_index = start_index;                                 \
    for (size_t i = 0; i < length; i++) {                                      \
      __VA_ARGS__;                                                             \
      element_name##_index += node_array[element_name##_index].tree_size;      \
    }                                                                          \
  }
