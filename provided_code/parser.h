/*
 * Copyright (C) Rida Bazzi, 2019
 *
 * Do not share this file with anyone
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include "lexer.h"
#include <map>
#include <string>
#include <vector>
#include <set>

enum StmtType { STMT_INPUT, STMT_OUTPUT, STMT_ASSIGN };
enum PrimaryKind { VAR, TERM_LIST };

struct primary_t {
  PrimaryKind kind;
  struct term_list_t* term_list;
  std::string var_name;
};

struct monomial_t {
  primary_t* primary;
  int exponent;
};

struct term_t {
  int coefficient;
  std::vector<monomial_t*> monomial_list;
};

enum OperatorType { OP_PLUS, OP_MINUS, OP_NONE };

struct term_list_t {
  term_t* term;
  OperatorType op = OP_NONE;
  term_list_t* next = nullptr;
};

struct poly_body_t {
  term_list_t* terms;
};

struct stmt_t {
    StmtType type;
    int var = -1;
    int lhs = -1;
    void* eval = nullptr;
    stmt_t* next = nullptr;
    int line_no;
};

struct poly_eval_t {
    std::string name;
    std::vector<std::string> args;
};

class Parser {
  public:
    void parse_program();
    void execute_program();
    void check_useless_assignments();
    std::set<int> task_numbers;
    std::set<std::string> initialized_vars;
    std::vector<int> warning_lines_uninitialized;
    std::vector<int> useless_assignments;
    std::vector<int> wrong_arity_lines;
    std::map<std::string, int> poly_degree_table;


  private:
    LexicalAnalyzer lexer;
    void syntax_error();
    Token expect(TokenType expected_type);

    // ====== Internal state for semantic checks ======
    std::map<std::string, std::vector<int>> poly_decl_lines;
    std::string current_poly;
    std::map<std::string, std::vector<std::string>> poly_params;
    std::vector<int> invalid_lines;
    std::vector<int> undeclared_eval_lines;
    // ====== Memory and Execution State for Task 2 ======
    std::map<std::string, int> location_table;
    std::vector<int> memory = std::vector<int>(1000);
    std::vector<int> input_values;
    int next_available = 0;
    int next_input = 0;
    stmt_t* stmt_list_head = nullptr;
    int input_counter = 0;
    bool in_inputs_section = false;
    std::map<std::string, poly_body_t*> poly_bodies;
    std::vector<std::string> input_vars_in_order;

    // ====== Parser methods ======
    void parse_tasks_section();
    void parse_num_list();
    void parse_poly_section();
    void parse_poly_decl_list();
    void parse_poly_decl();
    void parse_poly_header();
    void parse_poly_name();
    std::vector<std::string> parse_id_list();
    poly_body_t* parse_poly_body();
    term_list_t* parse_term_list();
    term_t* parse_term();
    std::vector<monomial_t*> parse_monomial_list();
    monomial_t* parse_monomial();
    primary_t* parse_primary();
    int parse_exponent();
    OperatorType parse_add_operator();
    int parse_coefficient();
    void parse_execute_section();
    stmt_t* parse_statement_list();
    stmt_t* parse_statement();
    stmt_t* parse_input_statement();
    stmt_t* parse_output_statement();
    stmt_t* parse_assign_statement();
    poly_eval_t* parse_poly_evaluation();
    std::vector<std::string> parse_argument_list();
    void parse_argument(std::vector<std::string>& args);
    void parse_inputs_section();
    int evaluate_poly(poly_body_t* body, const std::map<std::string, int>& arg_values, const std::map<std::string, int>& location_table);
    int evaluate_term(term_t* term, const std::map<std::string, int>& arg_values, const std::map<std::string, int>& location_table);
    int evaluate_monomial(monomial_t* monomial, const std::map<std::string, int>& arg_values, const std::map<std::string, int>& location_table);
    int evaluate_primary(primary_t* primary, const std::map<std::string, int>& arg_values, const std::map<std::string, int>& location_table);
    int get_degree(term_list_t* term_list);
    int get_degree(term_t* term);
    int get_degree(const std::vector<monomial_t*>& monomial_list);
    int get_degree(monomial_t* monomial);
    int get_degree(primary_t* primary);
};

#endif

