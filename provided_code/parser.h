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

struct poly_eval_t;

enum StmtType { STMT_INPUT, STMT_OUTPUT, STMT_ASSIGN };

struct stmt_t {
    StmtType type;
    int var = -1;
    int lhs = -1;
    void* eval = nullptr;
    stmt_t* next = nullptr;
};

struct poly_eval_t {
    std::string name;
    std::vector<std::string> args;
};

class Parser {
  public:
    void parse_program();

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
    std::vector<int> wrong_arity_lines;
    // ====== Memory and Execution State for Task 2 ======
    std::map<std::string, int> location_table;
    std::vector<int> memory = std::vector<int>(1000);
    std::vector<int> input_values;
    int next_available = 0;
    int next_input = 0;
    stmt_t* stmt_list_head = nullptr;
    int input_counter = 0;
    bool in_inputs_section = false;

    // ====== Parser methods ======
    void parse_tasks_section();
    void parse_num_list();
    void parse_poly_section();
    void parse_poly_decl_list();
    void parse_poly_decl();
    void parse_poly_header();
    void parse_poly_name();
    std::vector<std::string> parse_id_list();
    void parse_poly_body();
    void parse_term_list();
    void parse_term();
    void parse_monomial_list();
    void parse_monomial();
    void parse_primary();
    void parse_exponent();
    void parse_add_operator();
    void parse_coefficient();
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
    void execute_program();
};

#endif

