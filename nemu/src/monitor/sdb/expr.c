#include <isa.h>
#include <memory/paddr.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NEQ, TK_REG, TK_REVERSE,
  TK_DEREF, TK_DEC, TK_HEX,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {"\\s+", TK_NOTYPE},  // spaces
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},       // unequal
  {"\\+", '+'},         // plus
  {"-", '-'},           // minus
  {"\\*", '*'},         // multiply
  {"\\/", '/'},         // divide
  {"&&", '&'},          // and
  {"\\(", '('},         // left bracket
  {"\\)", ')'},         // right bracket
  {"0x[0-9a-zA-Z]+", TK_HEX},   // hexadecimal
  {"[0-9]+", TK_DEC},     // decimal numbers
  {"\\$\\w{1,3}", TK_REG} // value of register
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[128] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        
        //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
          //i, rules[i].regex, position, substr_len, substr_len, substr_start);
        
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case '+': case '-': case '*': case '/': case '(': case ')':
          case '&': case TK_EQ: case TK_NEQ:
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
          case TK_DEC: case TK_HEX: case TK_REG:
            if (substr_len > 31) {puts("Too long numbers not supported"); return false;}
            tokens[nr_token].type = rules[i].token_type;
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;
          case TK_NOTYPE: break;
          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static word_t eval(uint p, uint q, bool *success);

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  return eval(0, nr_token - 1, success);
}


static bool check_parentheses(uint p, uint q) {
  uint i;
  if (tokens[p].type != '(' || tokens[q].type != ')') 
    return false;     // not started with a '(' or ended with a ')'
  else{
    int count = 0;
    for (i = p; i <= q; i++) {
      if (tokens[i].type == '(') count++;
      else if (tokens[i].type == ')') count--;

      if (i != q && count <= 0) return false;
    }
    if (count != 0) return false; else return true;
  }
}

static uint find_main_op(uint p, uint q, bool *success) {
  int bracket_count = 0;
  bool exist_eq = false;
  bool exist_plusminus = false;
  bool exist_muldiv = false;
  bool exist_deref = false;
  bool exist_reverse = false;
  unsigned int op = 0;

  *success = false;
  /* scanning */
  for (uint i = p; i <= q; i++) {
    /* if in brackets */
    if (tokens[i].type == '(') {
      bracket_count++;
      continue;
    }
    else if (tokens[i].type == ')') {
      bracket_count--;
      continue;
    }
    if (bracket_count < 0) {
      *success = false;
      return 0;
    }

    /* not in a bracket */
    if (bracket_count == 0){

      if (tokens[i].type == '*') {
        /* '*' */
        if (i == p || 
            (tokens[i - 1].type != TK_DEC && tokens[i - 1].type != TK_HEX && tokens[i - 1].type != ')' && tokens[i - 1].type != TK_REG)
        ){
          /* DEREF */
          tokens[i].type = TK_DEREF;
          if (!exist_eq && !exist_plusminus && !exist_muldiv && !exist_deref) {
            op = i;
            exist_deref = true;
            *success = true;
          }
        }
        else{
          if (!exist_eq && !exist_plusminus){
            exist_muldiv = true;
            op = i;
            *success = true;
          }
        }
      }
      else if (tokens[i].type == '-') {
        /* '-' */
        if (i == p || 
            (tokens[i - 1].type != TK_DEC && tokens[i - 1].type != TK_HEX && tokens[i - 1].type != ')' && tokens[i - 1].type != TK_REG)
        ) {
          /* REVERSE */
          tokens[i].type = TK_REVERSE;
          if (!exist_eq && !exist_plusminus && !exist_muldiv && !exist_reverse) {
            op = i;
            exist_reverse = true;
            *success = true;
          }
        }
        else{
          if (!exist_eq) {
            exist_plusminus = true;
            op = i;
            *success = true;
          }
        }
      }
      else if (tokens[i].type == '&') {
        /* and */
        *success = true;
        return i; // max priority
      }
      else if (tokens[i].type == TK_EQ || tokens[i].type == TK_NEQ) {
        /* == or != */
        exist_eq = true;
        op = i;
        *success = true;
      }
      else if(tokens[i].type == '+' && !exist_eq) {
        /* '+' */
        exist_plusminus = true;
        op = i;
        *success = true;
      }
      else if(tokens[i].type == '/' && !exist_eq && !exist_plusminus) {
        /* '/' */
        exist_muldiv = true;
        op = i;
        *success = true;
      }
      else if (tokens[i].type == TK_DEREF || tokens[i].type == TK_REVERSE) {
        /* deref */
        if (tokens[i].type == TK_DEREF && !exist_eq && !exist_plusminus && !exist_muldiv && !exist_deref) {
          op = i;
          exist_deref = true;
          *success = true;
        }
        else if (tokens[i].type == TK_REVERSE && !exist_eq && !exist_plusminus && !exist_muldiv && !exist_reverse) {
          op = i;
          exist_reverse = true;
          *success = true;
        }
      }
    }
  }

  /* result */
  if (*success) return op;
  else return 0;

}

static word_t eval(uint p, uint q, bool *success) {
  if (p > q) {
    /* Bad expression */
    Log("Bad expression");
    *success = false;
    return 0;
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    word_t num;

    if (tokens[p].type == TK_DEC) {
      sscanf(tokens[p].str, "%ld", &num);
      *success = true;
    }
    else if (tokens[p].type == TK_HEX){
      sscanf(tokens[p].str, "0x%lx", &num);
      *success = true;
    }
    else {
      /* register */
      num = isa_reg_str2val(tokens[p].str + 1, success);
    }
    
    return num;
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1, success);
  }
  else {
    bool find_success, success1, success2;
    uint op = find_main_op(p, q, &find_success);
    if (tokens[op].type == TK_DEREF){
      word_t addr = eval(op + 1, q, &success1);
      *success = find_success && success1;
      if (*success){
        return paddr_read(addr, 4);
      }
      else{
        return 0;
      }
    }
    else if (tokens[op].type == TK_REVERSE) {
      word_t val = eval(op + 1, q, &success1);
      *success = find_success && success1;
      if (*success){
        return -val;
      }
      else{
        return 0;
      }
    }
    else {
      word_t val1 = eval(p, op - 1, &success1);
      word_t val2 = eval(op + 1, q, &success2);
      *success = find_success && success1 && success2;
      if (*success){
        switch (tokens[op].type) {
          case '+': return val1 + val2;
          case '-': return val1 - val2;
          case '*': return val1 * val2;
          case '/': return val1 / val2;
          case '&': return val1 && val2;
          case TK_EQ: return val1 == val2;
          case TK_NEQ: return val1 != val2;
          default: assert(0);
        }
      }
      else{
        return 0;
      }
    }
  }
}