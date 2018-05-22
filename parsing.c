#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

// If we are compiling on Windows compile these functions
#ifdef _WIN32

#include <string.h>
static char buffer[2048];

char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

void add_history(char* unused) {}

#else

#include <editline/readline.h>
#include <editline/history.h>

#endif

/* int number_of_nodes(mpc_ast_t* t) { */
/*     if (t->children_num == 0) { return 1; } */
/*     if (t->children_num >= 1) { */
/*         int total = 1; */
/*         for (int i = 0; i < t->children_num; i++) { */
/*             total = total + number_of_nodes(t->children[i]); */
/*         } */
/*         return total; */
/*     } */
/*     return 0; */
/* } */

enum { LVAL_NUM, LVAL_ERR };
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

typedef struct {
    int type;
    long num;
    int err;
} lval;

lval lval_num(long x){
    lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return x;
}

lval lval_err(int x){
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}

void lval_print(lval v) {
    switch (v.type) {
        /* In the case the type is a number print it */
        /* Then 'break' out of the switch. */
    case LVAL_NUM: printf("%li", v.num); break;

        /* In the case the type is an error */
    case LVAL_ERR:
        /* Check what type of error it is and print it */
        if (v.err == LERR_DIV_ZERO) {
            printf("Error: Division By Zero!");
        }
        if (v.err == LERR_BAD_OP)   {
            printf("Error: Invalid Operator!");
        }
        if (v.err == LERR_BAD_NUM)  {
            printf("Error: Invalid Number!");
        }
        break;
    }
}

void lval_println(lval v) { lval_print(v); putchar('\n'); }

long eval_op(long x, char* op, long y) {
    if (strcmp(op, "+") == 0) { return x + y; }
    if (strcmp(op, "-") == 0) { return x - y; }
    if (strcmp(op, "*") == 0) { return x * y; }
    if (strcmp(op, "/") == 0) { return x / y; }
    return 0;
}

long eval(mpc_ast_t* t){
    // If tagged as number:
    if(strstr(t->tag, "number")){
        return atoi(t->contents);
    }

    // Else
    char* op = t->children[1]->contents;
    long x = eval(t->children[2]);
    int i = 3;
    while(strstr(t->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    return x;
}

int main(int argc, char** argv){
    // Create Some Parsers
    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr     = mpc_new("expr");
    mpc_parser_t* Lispy    = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
  "                                                     \
    number   : /-?[0-9]+/ ;                             \
    operator : '+' | '-' | '*' | '/' ;                  \
    expr     : <number> | '(' <operator> <expr>+ ')' ;  \
    lispy    : /^/ <operator> <expr>+ /$/ ;             \
  ",
              Number, Operator, Expr, Lispy);

    puts("Lispy Version 0.1");
    puts("Press Ctrl+c to Exit\n");

    while(1){
        char* input = readline("lispy> ");
        add_history(input);

        /* printf("%s\n", input); */

        /* mpc_result_t r; */
        /* if(mpc_parse("<stdin>", input, Lispy, &r)) { */
        /*     mpc_ast_print(r.output); */
        /*     mpc_ast_delete(r.output); */
        /* } else { */
        /*     mpc_err_print(r.error); */
        /*     mpc_err_delete(r.error); */
        /* } */

        mpc_result_t r;
        if(mpc_parse("<stdin>", input, Lispy, &r)) {
            long result = eval(r.output);
            printf("%li\n", result);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    mpc_cleanup(4, Number, Operator, Expr, Lispy);
    return 0;
}
