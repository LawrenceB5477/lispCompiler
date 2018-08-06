#include <stdio.h>
#include <stdlib.h>
//<> searches system locations first, "" searches in directory first
#include  "mpc.h"

//Normally checks to see if expression was defined using #define directive
// you can only use literal constants and identifiers defined using #define in this expression
// #define defines a macro, tells preprocessor to replace instances of identifier with
// the replacement list
// #undef allows multiple versions of the macro I guess?
#ifdef _WIN32
#include <string.h>

//global variable that can be accessed anywhere
//static makes it local to file
static char buffer[2048];

// This is a fake readline function
char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer) + 1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy) - 1] = '\0';
  return cpy;
}


// Fake history function
void add_history(char* notUsed) {}
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

//Definition of the lval structure
typedef struct {
  int type;
  long num;
  int err;
  double flt;
} lval;

//Decleration of type enums for lval struct
enum {LVAL_NUM, LVAL_ERR, LVAL_FLT};

//Decleration of error type enums
enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

//Create a number lvalue
lval lval_num(long x) {
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

//Create an error lvalue
lval lval_err(int x) {
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}

//Create a floating point lvalue
lval lval_flt(double x) {
  lval v;
  v.type = LVAL_FLT;
  v.flt = x;
  return v;
}

//Function to print out lvalue
void lval_print(lval v) {
  switch (v.type) {
    case LVAL_NUM:
      printf("%li", v.num);
      break;
    case LVAL_FLT:
      printf("%f", v.flt);
      break;
    case LVAL_ERR:
      if (v.err == LERR_DIV_ZERO) {
        printf("Error: Division by zero.");
      } else if (v.err == LERR_BAD_OP) {
        printf("Error: Bad operator.");
      } else if (v.err == LERR_BAD_NUM) {
        printf("Error: Bad number.");
      } else {
        printf("There was an unspecified error");
      }
      break;
  }
}

//Prints out lvalue followed by new line
void lval_println(lval v) {
  lval_print(v); putchar('\n');
}

int number_of_nodes(mpc_ast_t* t) {
  if (t->children_num == 0)
    return 1;
  if (t->children_num >= 1) {
    //Accounting for itself
    int children = 1;
    for (int i = 0; i < t->children_num; i++) {
      children += number_of_nodes(t->children[i]);
    }
    return children;
  }
  //Something went wrong if this happens
  puts("There was an issue, something went wrong");
  return 0;
}

int number_of_leaves(mpc_ast_t* r) {
  if (strstr(r->tag, "number") || strstr(r->tag, "operator")) {
    return 1;
  }
  int i = 1;
  int leaves = 0;
  while(strstr(r->children[i]->tag, "operator") || strstr(r->children[i]->tag, "expr")) {
    leaves += number_of_leaves(r->children[i]);
    i++;
  }
 return leaves;
}

int number_of_branches(mpc_ast_t* r) {
  int children = 0;
  int branches = 1;
  for (int i = 1; i < r->children_num - 1; i++) {
    if (r->children[i]->children_num) {
      children++;
      branches += number_of_branches(r->children[i]);
    }
  }
  if (!children)
    return 1;
  return branches;
}

int most_leaves(mpc_ast_t* r, int max) {
  int children = r->children_num - 2;

  //If any of the children have children, see if they have more
  for (int i = 1; i < r->children_num - 1; i++) {
    if (r->children[i]->children_num)
      children = most_leaves(r->children[i], children);
  }
  if (children > max)
    return children;
  else
    return max;
}

long power(long x, long y) {
    int res = 1;
    for (int i = 0; i < y; i++)
      res *= x;
    return res;
}

double power_flt(double x, double y) {
  double res = 1;
  for (int i = 0; i < (int)y; i++) {
    res *= x;
  }
  return res;
}

//Does arithmetic computations
lval eval_op(lval x, char* op, lval y) {
  //If the data is bad to begin with
  if (x.type == LVAL_ERR) {
    return x;
  }
  if (y.type == LVAL_ERR) {
     return y;
   }

  //For both integer number types
  if (x.type == LVAL_NUM && y.type == LVAL_NUM) {
    if (!strcmp(op, "+"))
      return lval_num(x.num + y.num);
    if (!strcmp(op, "-"))
      return lval_num(x.num - y.num);
    if (!strcmp(op, "*"))
      return lval_num(x.num * y.num);
    if (!strcmp(op, "/"))
      // Check for division by zero
      return (y.num == 0) ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
    if (!strcmp(op, "%"))
      return lval_num(x.num % y.num);
    if (!strcmp(op, "^"))
        return lval_num(power(x.num, y.num));
    if (!strcmp(op, "min")) {
      if (x.num < y.num)
        return x;
      else
        return y;
    }
    if (!strcmp(op, "max")) {
      if (x.num > y.num)
        return x;
      else
        return y;
    }
    return lval_err(LERR_BAD_OP);

    //For both double types
  } else if (x.type == LVAL_FLT && y.type == LVAL_FLT) {
    if (!strcmp(op, "+")) {
      return lval_flt(x.flt + y.flt);
    }
    if (!strcmp(op, "-"))
      return lval_flt(x.flt - y.flt);
    if (!strcmp(op, "*"))
      return lval_flt(x.flt * y.flt);
    if (!strcmp(op, "/"))
      // Check for division by zero
      return (y.num == 0) ? lval_err(LERR_DIV_ZERO) : lval_flt(x.flt / y.flt);
    if (!strcmp(op, "%"))
      return lval_err(LERR_BAD_OP);
    if (!strcmp(op, "^"))
        return lval_flt(power_flt(x.flt, y.flt));
    if (!strcmp(op, "min")) {
      if (x.num < y.num)
        return x;
      else
        return y;
    }
    if (!strcmp(op, "max")) {
      if (x.num > y.num)
        return x;
      else
        return y;
    }
    return lval_err(LERR_BAD_OP);
    //If the numbers are not of the same type
    //TODO do this
  } else {
    return lval_err(LERR_BAD_NUM);
  }
}

//This evaluates the polish notation expressions
lval eval(mpc_ast_t* t) {
  // If the node is a number, return the number
  if (strstr(t->tag, "number")) {
    if (strstr(t->contents, ".")) {
      double x = strtod(t->contents, NULL);
      lval test =  (x == 0.0) ? lval_err(LERR_BAD_NUM) : lval_flt(x);
      return test;
    } else {
      errno = 0;
      long x = strtol(t->contents, NULL, 10);
      return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }
  }

  //Else, the node is probably the root of a nested expression
  //Find the operator
  char* op = t->children[1]->contents;

  //Get the first operand
  lval x = eval(t->children[2]);

  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }
  if (i==3 && !strcmp(op, "-"))
    return (x.type == LVAL_NUM) ? lval_num(-x.num) : lval_flt(-x.flt);
  return x;
}



// stdin and stdout are special file variables representing input and output to
// the command line

int main(int argc, char** argv) {
  // ---- Make the parser ----
  // Declare re-write rules
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");

  mpca_lang(MPCA_LANG_DEFAULT,
    "                                                     \
      number   : /-?[0-9]+([.][0-9]+)?/;                    \
      operator : '+' | '-' | '*' | '/' | '%' | '^'        \
      | \"min\" | \"max\";                                \
      expr     : <number> | '(' <operator> <expr>+ ')' ;  \
      lispy    : /^/ <operator> <expr>+ /$/ ;             \
    ",
    Number, Operator, Expr, Lispy);

  //Grammar for doge language
  mpc_parser_t* Adjective = mpc_new("adjective");
  mpc_parser_t* Noun = mpc_new("noun");
  mpc_parser_t* Phrase = mpc_new("phrase");
  mpc_parser_t* Doge = mpc_new("doge");

  mpca_lang(MPCA_LANG_DEFAULT,
  "                                                                 \
    adjective: \"wow\" | \"many\" | \"so\" | \"such\";              \
    noun: \"lisp\" | \"language\" | \"c\" | \"book\" | \"build\";   \
    phrase: <adjective> <noun>;                                     \
    doge: /^/ <phrase>* /$/;                                                \
  ",
  Adjective, Noun, Phrase, Doge);


  puts("Lispy Version 0.0.0.0.1");
  puts("*** This is still in development ***");
  puts("Press Ctrl+c or type \"Exit\" \n");

  while (1) {
    char* input = readline("Lispy>");
    // reads at most n -1 chars, stores in array, stops if EOF or \n found
    // reads \n and writes \0 after last character in array (why it's n -1)
    if (!strcmp(input, "Exit") || !strcmp(input, "exit") || !strcmp(input, "e")) {
      puts("Bye");
      break;
    }

    //Try parse the user input
    mpc_result_t r;
    //If parsing input with the Lispy parser works
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      // if successful print the ast
      mpc_ast_t* root = r.output;
      // printf("\n-- The root has %d children\n", (*root).children_num);
      // printf("-- The number of nodes in this tree is: %d\n", number_of_nodes(root));
      // printf("-- You inputed: %s\n\n", input);
      mpc_ast_print(r.output);

      printf("\nNumber of leaves: %d", number_of_leaves(root));
      printf("\nNumber of branches: %d", number_of_branches(root));
      printf("\nLargest number of leaves: %d\n", most_leaves(root, 0));
      lval result = eval(root);
      lval_println(result);
      //printf("Result: %li\n\n", eval(root));
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }



  // Delete the parsers
  mpc_cleanup(4, Number, Operator, Expr, Lispy);
  //on linux and mac, by default arrow keys add weird input instead of moving
  // around on the input
  // to get behvaior where the arrow keys let us move around on input, we
  // have to use a library called editline
  return 0;
}
