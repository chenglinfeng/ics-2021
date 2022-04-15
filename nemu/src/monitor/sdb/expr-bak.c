#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

#include "memory/vaddr.h"

enum {
	NOTYPE = 256, EQ, NEQ, NUM, HEX, REG, SYMB, LS, RS, NG, NL, AND, OR, DEREF, NEG

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  // {" +", TK_NOTYPE},    // spaces
  // {"\\+", '+'},         // plus
  // {"==", TK_EQ},        // equal

	{" +",	NOTYPE},				// white space
	{"\\+", '+'},
	{"\\-", '-'},
	{"==", EQ},
	{"!=",NEQ},
	{"0[x,X][0-9a-fA-F]+", HEX},
	{"[0-9]+", NUM},
  {"\\$pc|\\$0|\\$at|\\$ra|\\$gp|\\$sp|\\$v[0-1]|\\$a[0-3]|\\$t[0-9]|\\$s[0-8]|\\$k[0-1]", REG},
	{"[a-zA-Z]+[a-zA-Z0-9_]*", SYMB},
	{"\\*", '*'},
	{"/", '/'},
	{"%", '%'},
	{"\\(", '('},
	{"\\)", ')'},
	{"<<", LS},						//left shift
	{">>", RS},						//right shift
	{"<=", NG},						//not greater than
	{">=", NL},						//not less than
	{"<", '<'},
	{">", '>'},
	{"&&",AND},
	{"\\|\\|",OR},
	{"&",'&'},
	{"\\^",'^'},
	{"\\|",'|'},
	{"!",'!'},
	{"~",'~'},

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

static Token tokens[32] __attribute__((used)) = {};
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

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
      
        int j;
				switch(rules[i].token_type) {
					case NOTYPE:break;
					case REG:
					case NUM: 
					case HEX:
					case SYMB:
							for(j=0;j<substr_len;j++)
							{
								tokens[nr_token].str[j]=*(substr_start+j);
							}
							tokens[nr_token].str[j]='\0';
					default: 
							tokens[nr_token].type = rules[i].token_type;
							nr_token ++;
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

static struct Node {
	int operand;
	int priority;
} table[] = {
	{OR,12},
	{AND,11},
	{'|',10},
	{'^',9},
	{'&',8},
	{EQ,7},
	{NEQ,7},
	{'>',6},
	{'<',6},
	{NG,6},
	{NL,6},
	{LS,5},
	{RS,5},
	{'+',4},
	{'-',4},
	{'*',3},
	{'/',3},
	{'%',3},
	{'!',2},
	{'~',2},
	{DEREF,2},
	{NEG,2},
};
int NR_TABLE = sizeof(table) / sizeof(table[0]);

int check_parentheses(int p, int q) {
	int i,j=0;
	for(i=p;i<q;i++)
	{
		if(tokens[i].type=='(')
			j++;
		else if(tokens[i].type==')')
			j--;
		if(j==0)
			return 0;
	}
	if(tokens[q].type==')')
		j--;
	return (j==0)&&(tokens[p].type=='(')&&(tokens[q].type==')');
}

int isoperand(int i){
	return tokens[i].type!=NOTYPE && tokens[i].type!=NUM && tokens[i].type!=REG && tokens[i].type!=SYMB && tokens[i].type!=HEX && tokens[i].type!='(' && tokens[i].type!=')';
}
uint32_t eval(int p,int q,bool *success) {
	if(p > q) {
	/* Bad expression */
		*success=false;
		return 0;
	}
	else if(p == q) { 
	/* Single token.
	* For now this token should be a number. 
	* Return the value of the number.
	*/ 
		if(tokens[p].type==HEX)
		{
			uint32_t result=0;
			sscanf(tokens[p].str,"%x",&result);
			return result;
		}
		else if(tokens[p].type==NUM)
		{
			uint32_t result=0;
			sscanf(tokens[p].str,"%d",&result);
			return result;
		}
		else if(tokens[p].type==REG)
		{
			if(tokens[p].str[2]=='0')
				return cpu.gpr[0]._64;
			else if(tokens[p].str[2]=='r'&&tokens[p].str[3]=='a')
				return cpu.gpr[1]._64;
			else if(tokens[p].str[2]=='s'&&tokens[p].str[p]=='p')
				return cpu.gpr[2]._64;
			else if(tokens[p].str[2]=='g'&&tokens[p].str[2]=='p')
				return cpu.gpr[3]._64;
			else if(tokens[p].str[2]=='t'&&tokens[p].str[3]=='p')
				return cpu.gpr[4]._64;
			else
				return cpu.pc;
		}
		// else
		// 	return look_up_symtab(tokens[p].str, success);
	}
	else if(check_parentheses(p, q) == true) {
	/* The expression is surrounded by a matched pair of parentheses. 
	* If that is the case, just throw away the parentheses.
	*/
		return eval(p + 1, q - 1,success); 
	}
	else {
		int op=-1;
		int op_priority=0;
		int i;
		for(i=p;i<=q;i++)
		{
			if(tokens[i].type=='(')
			{
				int k=1;
				while(k!=0)
				{
					i++;
					if(tokens[i].type=='(')
						k++;
					else if(tokens[i].type==')')
						k--;
				}
			}
			else if(isoperand(i))
			{
				int j;
				for(j=0;j<NR_TABLE;j++)
					if(table[j].operand==tokens[i].type)
						break;
				if(table[j].priority>=op_priority)
				{
					op_priority=table[j].priority;
					op=i;
				}
			}
		}
		int op_type=tokens[op].type;
		uint32_t val1=0,val2=0;
		if(op_type!='!'&&op_type!='~'&&op_type!=NEG&&op_type!=DEREF)
			val1 = eval(p, op - 1, success);
		val2 = eval(op + 1, q, success);
		switch(op_type) {
			case '+': return val1 + val2;break;
			case '-': return val1 - val2;break;
			case '*': return val1 * val2;break;
			case '/': return val1 / val2;break;
			case '%': return val1 % val2;break;
			case LS: return val1 << val2;break;
			case RS: return val1 >> val2;break;
			case '>': return val1 > val2;break;
			case '<': return val1 < val2;break;
			case NG: return val1 <= val2;break;
			case NL: return val1 >= val2;break;
			case EQ: return val1 == val2;break;
			case NEQ: return val1 != val2;break;
			case '&': return val1 & val2;break;
			case '^': return val1 ^ val2;break;
			case '|': return val1 | val2;break;
			case AND: return val1 && val2;break;
			case OR: return val1 || val2;break;
			case '!': return !val2;break;
			case '~': return ~val2;break;
			case DEREF: return vaddr_read(val2,1);break;
			case NEG: return -val2;break;
			default: assert(0);
		}
	}
  return 0;
}

word_t expr(char *e, bool *success) {
  *success=true;
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  for(int i = 0;i < nr_token;i++)
  {
    if(tokens[i].type == '*' && (i == 0 || isoperand(i-1)))
		tokens[i].type = DEREF;
	else if(tokens[i].type == '-' && (i == 0 || isoperand(i-1)))
		tokens[i].type = NEG;
  }

  unsigned int ans = eval(0,nr_token-1,success);
  //printf("%u\n",ans);
  
  return ans;
}