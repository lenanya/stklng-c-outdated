#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include <nob.h>
#define ALEXER_IMPLEMENTATION
#include <alexer.h>

#define liberate(ptr) free(ptr)
#define da_liberate(da) da_free(da)

typedef enum Type{
	T_Int,
	T_Float,
	T_Bool,
	T_String,
} Type;

typedef enum CmpType{
	le,
	lt,
	ge,
	gt,
	ne,
	eq,
} CmpType;

typedef union Value{
	int i;
	float f;
	bool b;
	char *s;
} Value;

typedef struct Node {
	Type t;
	Value v;
} Node;

typedef struct Stack{
	size_t count;
	size_t capacity;
	Node *items;
} Stack;

typedef enum FunctionType{
	F_push,
	F_pop,
	F_pop_many,
	F_addi,
	F_addf,
	F_scat,
	F_isub,
	F_fsub,
	F_icmp,
	F_fcmp,
	F_scmp,
	F_prstk,
	F_print,
	F_brn,
	F_jmp,
	F_gosub,
	F_return,
	F_swp,
} FunctionType;
 
typedef struct Function{
	FunctionType ft;
	CmpType ct;
	Node n;
	size_t op;
	size_t index;
} Function;

typedef struct Program {
	size_t capacity;
	size_t count;
	Function *items;
} Program;

typedef struct Returns{
	size_t count;
	size_t capacity;
	size_t *items;
} Returns;

void push(Stack *s, Node n) {
	da_append(s, n);
}

void pop_many_d(Stack *s, size_t amount, char *file, size_t line) {
	if (amount > s->count) {
		printf("[ERROR] Cannot pop more items than there are on the Stack [%s:%ld]\n", file, line);
		exit(1);
	}
	s->count -= amount;
}

#define pop_many(s, a) pop_many_d(s, a, __FILE__, __LINE__)
#define pop(s) pop_many(s, 1)

void prstk(Stack *s) {
	printf("----     Stack    ----\n");
	for (size_t i = 0; i < s->count; ++i) {
		switch (s->items[i].t) {
			case (T_Int):
				printf("%ld: Int: %d\n", i, s->items[i].v.i);
				break;
			case (T_Float):
				printf("%ld: Float: %f\n", i, s->items[i].v.f);
				break;
			case (T_Bool):
				if (s->items[i].v.b == 0) {
					printf("%ld: Bool: false\n", i);
				} else {
					printf("%ld: Bool: true\n", i);
				}
				break;
			case (T_String):
				printf("%ld: String: %s\n", i, s->items[i].v.s);
				break;
			default:
				break;
		}
	}
	printf("---- End of Stack ----\n\n");
}

void addi_d(Stack *s, char *file, size_t line) {
	if (s->items[s->count-1].t != T_Int || s->items[s->count - 2].t != T_Int || s->count < 2) {
		printf("[ERROR] addi requires 2 Integers at the top of the stack [%s:%ld]\n", file, line);
		exit(1);
	}
	int addend = s->items[s->count-1].v.i;
	pop(s);
	s->items[s->count-1].v.i += addend;
}

#define addi(s) addi_d(s, __FILE__, __LINE__)

void addf_d(Stack *s, char *file, size_t line) {
	if (s->items[s->count-1].t != T_Float || s->items[s->count - 2].t != T_Float || s->count < 2) {
		printf("[ERROR] addf requires 2 Floats at the top of the stack [%s:%ld]\n", file, line);
		exit(1);
	}
	float addend = s->items[s->count-1].v.f;
	pop(s);
	s->items[s->count-1].v.f += addend;
}

#define addf(s) addf_d(s, __FILE__, __LINE__)

void isub_d(Stack *s, char *file, size_t line) {
	if (s->items[s->count-1].t != T_Int || s->items[s->count - 2].t != T_Int || s->count < 2) {
		printf("[ERROR] isub requires 2 Integers at the top of the stack [%s:%ld]\n", file, line);
		exit(1);
	}
	int subtrahend = s->items[s->count-1].v.i;
	pop(s);
	s->items[s->count-1].v.i -= subtrahend;
}

#define isub(s) isub_d(s, __FILE__, __LINE__)

void fsub_d(Stack *s, char *file, size_t line) {
	if (s->items[s->count-1].t != T_Float || s->items[s->count - 2].t != T_Float || s->count < 2) {
		printf("[ERROR] fsub requires 2 Floats at the top of the stack [%s:%ld]\n", file, line);
		exit(1);
	}
	float subtrahend = s->items[s->count-1].v.f;
	pop(s);
	s->items[s->count-1].v.f -= subtrahend;
}

#define fsub(s) fsub_d(s, __FILE__, __LINE__)

void scat_d(Stack *s, char* file, size_t line) {
	if (s->items[s->count-1].t != T_String || s->items[s->count - 2].t != T_String || s->count < 2) {
		printf("[ERROR] scat requires 2 Strings at the top of the stack [%s:%ld]\n", file, line);
		exit(1);
	}
	char *second = s->items[s->count-1].v.s;
	pop(s);
	Node strres;
	strres.t = T_String;
	strres.v.s = malloc(strlen(s->items[s->count-1].v.s + strlen(second)));
	strres.v.s = strcat(strres.v.s, s->items[s->count-1].v.s);
	strres.v.s = strcat(strres.v.s, second);
	pop(s);
	push(s, strres);
}

#define scat(s) scat_d(s, __FILE__, __LINE__)

void icmp_d(Stack *s, CmpType t, char *file, size_t line) {
	if (s->items[s->count-1].t != T_Int || s->items[s->count - 2].t != T_Int || s->count < 2) {
		printf("[ERROR] icmp requires 2 Integers at the top of the stack [%s:%ld]\n", file, line);
		exit(1);
	}
	int comperand_left, comperand_right;
	comperand_left = s->items[s->count-2].v.i;
	comperand_right = s->items[s->count-1].v.i;
	Node cmp_bool;
	cmp_bool.t = T_Bool;
	switch (t) {
		case (lt):
			cmp_bool.v.b = comperand_left < comperand_right;
			break;
		case (le):
			cmp_bool.v.b = comperand_left <= comperand_right;
			break;
		case (gt):
			cmp_bool.v.b = comperand_left > comperand_right;
			break;
		case (ge):
			cmp_bool.v.b = comperand_left >= comperand_right;
			break;
		case (eq):
			cmp_bool.v.b = comperand_left == comperand_right;
			break;
		case (ne):
			cmp_bool.v.b = comperand_left != comperand_right;
			break;
		default:
			printf("[ERROR] icmp requires valid comparison type! [%s:%ld]\n", file, line);
			exit(1);
	}
	push(s, cmp_bool);
}

#define icmp(s, t) icmp_d(s, t, __FILE__, __LINE__)

void fcmp_d(Stack *s, CmpType t, char *file, size_t line) {
	if (s->items[s->count-1].t != T_Float || s->items[s->count - 2].t != T_Float || s->count < 2) {
		printf("[ERROR] fcmp requires 2 Floats at the top of the stack [%s:%ld]\n", file, line);
		exit(1);
	}
	float comperand_left, comperand_right;
	comperand_left = s->items[s->count-1].v.f;
	comperand_right = s->items[s->count-2].v.f;
	Node cmp_bool;
	cmp_bool.t = T_Bool;
	switch (t) {
		case (lt):
			cmp_bool.v.b = comperand_left < comperand_right;
			break;
		case (le):
			cmp_bool.v.b = comperand_left <= comperand_right;
			break;
		case (gt):
			cmp_bool.v.b = comperand_left > comperand_right;
			break;
		case (ge):
			cmp_bool.v.b = comperand_left >= comperand_right;
			break;
		case (eq):
			cmp_bool.v.b = comperand_left == comperand_right;
			break;
		case (ne):
			cmp_bool.v.b = comperand_left != comperand_right;
			break;
		default:
			printf("[ERROR] fcmp requires valid comparison type! [%s:%ld]\n", file, line);
			exit(1);
	}
	push(s, cmp_bool);
}

#define fcmp(s, t) fcmp_d(s, t, __FILE__, __LINE__)

void scmp_d(Stack *s, CmpType t, char* file, size_t line) {
	if (t != eq && t != ne) {
		printf("[ERROR] scmp can only be done with eq or ne [%s:%ld]\n", file, line);
		exit(1);
	}
	char *comperand_left, *comperand_right;
	comperand_left = s->items[s->count-1].v.s;
	comperand_right = s->items[s->count-2].v.s;
	Node cmp_bool;
	cmp_bool.t = T_Bool;
	switch (t) {
		case (eq):
			cmp_bool.v.b = strcmp(comperand_left, comperand_right);
			break;
		case (le):
			cmp_bool.v.b = strcmp(comperand_left, comperand_right);
			break;
		default:
			printf("[ERROR] scmp requires valid comparison type [%s:%ld]\n", file, line);
			exit(1);
	}
	push(s, cmp_bool);
} 

#define scmp(s, t) scmp_d(s, t, __FILE__, __LINE__);

void print(Stack *s, size_t len) {
	if (s->count < len) {
		printf("[ERROR] Not enough characters on the Stack");
		exit(1);
	}
	for (int i = len; i >= 0; --i) {
		Node n = s->items[s->count - 1 - i];
		if (n.t != T_Int) {
			printf("[ERROR] Value to be printed not an integer");
			exit(1);
		}
		if (n.v.i == 10) {
			printf("\n");
		} else {
			printf("%c", (int)n.v.i);
		}
	}
}

bool brn(Stack *s) {
	if (s->items[s->count-1].t != T_Bool) {
		printf("[ERROR] Cannot branch based on non boolean");
		exit(1);
	}
	if (s->items[s->count-1].v.b == true) {
		return true;
	};
	return false;
}

void swp(Stack *s) {
	if (s->count < 2) {
		printf("[ERROR] swp requires at least 2 elements on the Stack");
		exit(1);
	}
	Node temp = s->items[s->count-1];
	pop(s);
	Node temp2 = s->items[s->count-1];
	pop(s);
	push(s, temp);
	push(s, temp2);
}

size_t getFunctionIndex(Program *p, size_t index) {
	for (size_t i = 0; i < p->count; ++i) {
		if (p->items[i].index == index) return i;
	}
	printf("[ERROR] Jumping to undefined");
	exit(1);
}

size_t eval(Stack *s, Function f, size_t i, Program *p, Returns *r) {
	switch (f.ft) {
		case (F_push):
			push(s, f.n);
			break;
		case (F_pop):
			pop(s);
			break;
		case (F_pop_many):
			pop_many(s, f.op);
			break;
		case (F_addi):
			addi(s);
			break;
		case (F_addf):
			addf(s);
			break;
		case (F_isub):
			isub(s);
			break;
		case (F_fsub):
			fsub(s);
			break;
		case (F_scat):
			scat(s);
			break;
		case (F_icmp):
			icmp(s, f.ct);
			break;
		case (F_fcmp):
			fcmp(s, f.ct);
			break;
		case (F_scmp):
			scmp(s, f.ct);
			break;
		case (F_prstk):
			prstk(s);
			break;
		case (F_print):
			print(s, f.n.v.i);
			break;
		case (F_brn):
			if (brn(s) == true) return getFunctionIndex(p, f.n.v.i);
			break;
		case (F_jmp):
			return getFunctionIndex(p, f.n.v.i);
		case (F_gosub):
			da_append(r, i+1);
			return getFunctionIndex(p, f.n.v.i);
		case (F_return):
			size_t newi = r->items[r->count-1];
			r->count--;
			return newi;
		case (F_swp):
			swp(s);
			break;
		default:
			printf("[ERROR] Invalid Function Type called");
			exit(1);
	}
	return ++i;
}

void exec(Program *p, Returns *r) {
	Stack s = {0};
	for (size_t i = 0; i < p->count;) {
		i = eval(&s, p->items[i], i, p, r);
	}

	da_liberate(s);
	da_liberate(*p);
	da_liberate(*r);
}

typedef enum PunctIndex {
	P_semicolon,
	P_count,
} PunctIndex;

const char *puncts[P_count] = {
	[P_semicolon] = ";",
};

typedef enum KeywordIndex{
	K_push,
	K_pop,
	K_addi,
	K_addf,
	K_scat,
	K_icmp,
	K_fcmp,
	K_scmp,
	K_isub,
	K_fsub,
	K_prstk,
	K_print,
	K_brn,
	K_jmp,
	K_le,
	K_lt,
	K_eq,
	K_gt,
	K_ge,
	K_ne,
	K_gosub,
	K_return,
	K_swp,
	K_true,
	K_false,
	K_count,
} KeywordIndex;

const char *keywords[K_count] = {
	[K_push] 	= "push",
	[K_pop] 	= "pop",
	[K_addi] 	= "addi",
	[K_addf] 	= "addf",
	[K_scat] 	= "scat",
	[K_icmp] 	= "icmp",
	[K_fcmp] 	= "fcmp",
	[K_scmp] 	= "scmp",
	[K_isub] 	= "isub",
	[K_fsub] 	= "fsub",
	[K_prstk] 	= "prstk",	
	[K_brn]		= "brn",
	[K_jmp]		= "jmp",
	[K_le] 		= "le",
	[K_lt] 		= "lt",
	[K_eq] 		= "eq",
	[K_gt] 		= "gt",
	[K_ge] 		= "ge",
	[K_ne] 		= "ne",	
	[K_gosub]	= "gosub",
	[K_return]	= "return",
	[K_swp]		= "swp",
	[K_true]	= "true",
	[K_false]	= "false",
	[K_print]   = "print",
};

const char *comments[] = {
	"//",
};

void createFromFile(char *fp, Program *p) {
	String_Builder sb = {0};
	read_entire_file(fp, &sb);
	char *ps = malloc(sb.count * 2); // we love off by one errors!
	String_View sv = sb_to_sv(sb);
	sprintf(ps, SV_Fmt"\n", SV_Arg(sv));
	Alexer l = alexer_create(fp, ps, strlen(ps));	
	l.puncts = puncts;
	l.puncts_count = ALEXER_ARRAY_LEN(puncts);
	l.keywords = keywords;
	l.keywords_count = ALEXER_ARRAY_LEN(keywords);
	l.sl_comments = comments;
	l.sl_comments_count = ALEXER_ARRAY_LEN(comments);
	Alexer_Token t  = {0};
	while (alexer_get_token(&l, &t)) {
		Function f;
		if (!alexer_expect_id(&l, t, ALEXER_INT)) {
			exit(1);
		}
		f.index = t.int_value;
		alexer_get_token(&l, &t);
		// TODO: check for keyword???
		//uint64_t expected_kw[] = {K_addi, K_icmp, K_isub, K_pop, K_prstk, K_push, K_scat, K_scmp};
		//if (!alexer_expect_one_of_ids(&l, t, expected_kw, ALEXER_ARRAY_LEN(expected_kw))) { // TODO: somehow fix ????????
		//	exit(1);
		//}
		switch (ALEXER_INDEX(t.id)) {
			case (K_push):
				f.ft = F_push;
				alexer_get_token(&l, &t);
				if (ALEXER_KIND(t.id) != ALEXER_INT) { // kinda hacky but works
					if (ALEXER_INDEX(t.id) != K_true && ALEXER_INDEX(t.id) != K_false) {
						printf("[ERROR] Pushing a Boolean requires `true` or `false`\n");
						exit(1);
					}
				}

				switch (ALEXER_KIND(t.id)) {
					case (ALEXER_INT):
						Node n;
						n.t = T_Int;
						n.v.i = t.int_value;
						f.n = n;
						da_append(p, f);	
						break;
					case (ALEXER_KEYWORD):
						Node nb;
						nb.t = T_Bool;
						if (ALEXER_INDEX(t.id) == K_true) {
							nb.v.b = true;
						} else {
							nb.v.b = false;
						}
						f.n = nb;
						da_append(p, f);
						break;
					default:
						UNREACHABLE("Other kinds of push not implemented");
						break;
				}
				alexer_get_token(&l, &t);
				if (!alexer_expect_id(&l, t, ALEXER_PUNCT)) {
					exit(1);
				}
				break;
			case (K_addi):
				f.ft = F_addi;
				alexer_get_token(&l, &t);
				if (!alexer_expect_id(&l, t, ALEXER_PUNCT)) {
					exit(1);
				}
				da_append(p, f);
				break;
			case (K_prstk):
				f.ft = F_prstk;
				alexer_get_token(&l, &t);
				if (!alexer_expect_id(&l, t, ALEXER_PUNCT)) {
					exit(1);
				}
				da_append(p, f);
				break;	
			case (K_icmp):
				f.ft = F_icmp;
				alexer_get_token(&l, &t);
				//uint64_t expected_ct[] = {K_le, K_ge, K_lt, K_gt, K_le, K_ne};
				//if (!alexer_expect_one_of_ids(&l, t, expected_ct, ALEXER_ARRAY_LEN(expected_ct))) {
				//	exit(1);
				//}
				switch (ALEXER_INDEX(t.id)) {
					case (K_le):
						f.ct = le;
						break;
					case (K_lt):
						f.ct = lt;
						break;
					case (K_ge):
						f.ct = ge;
						break;
					case (K_gt):
						f.ct = gt;
						break;
					case (K_eq):
						f.ct = eq;
						break;
					case (K_ne):
						f.ct = ne;
						break;
					default:
						UNREACHABLE("compare type doesnt exist");
				}
				alexer_get_token(&l, &t);
				if (!alexer_expect_id(&l, t, ALEXER_PUNCT)) {
					exit(1);
				}
				da_append(p, f);
				break;
			case (K_isub):
				f.ft = F_isub;
				alexer_get_token(&l, &t);
				if (!alexer_expect_id(&l, t, ALEXER_PUNCT)) {
					exit(1);
				}
				da_append(p, f);
				break;
			case (K_pop):
				f.ft = F_pop;
				alexer_get_token(&l, &t);
				if (!alexer_expect_id(&l, t, ALEXER_PUNCT)) {
					exit(1);
				}
				da_append(p, f);
				break;
			case (K_print):
				f.ft = F_print;
				alexer_get_token(&l, &t);
				if (!alexer_expect_id(&l, t, ALEXER_INT)) {
					exit(1);
				}
				Node n;
				n.t = T_Int;
				n.v.i = t.int_value;
				f.n = n;
				alexer_get_token(&l, &t);
				if (!alexer_expect_id(&l, t, ALEXER_PUNCT)) {
					exit(1);
				}
				da_append(p, f);
				break;
			case (K_brn):
				f.ft = F_brn;
				alexer_get_token(&l, &t);
				if (!alexer_expect_id(&l, t, ALEXER_INT)) {
					exit(1);
				}
				Node n2;
				n2.t = T_Int;
				n2.v.i = t.int_value;
				f.n = n2;
				alexer_get_token(&l, &t);
				if (!alexer_expect_id(&l, t, ALEXER_PUNCT)) {
					exit(1);
				}
				da_append(p, f);
				break;
			case (K_jmp):
				f.ft = F_jmp;
				alexer_get_token(&l, &t);
				if (!alexer_expect_id(&l, t, ALEXER_INT)) {
					exit(1);
				}
				Node n3;
				n3.t = T_Int;
				n3.v.i = t.int_value;
				f.n = n3;
				alexer_get_token(&l, &t);
				if (!alexer_expect_id(&l, t, ALEXER_PUNCT)) {
					exit(1);
				}
				da_append(p, f);
				break;
			case (K_gosub):
				f.ft = F_gosub;
				alexer_get_token(&l, &t);
				if (!alexer_expect_id(&l, t, ALEXER_INT)) {
					exit(1);
				}
				Node n4;
				n4.t = T_Int;
				n4.v.i = t.int_value;
				f.n = n4;
				alexer_get_token(&l, &t);
				if (!alexer_expect_id(&l, t, ALEXER_PUNCT)) {
					exit(1);
				}
				da_append(p, f);
				break;
			case (K_return):
				f.ft = F_return;
				alexer_get_token(&l, &t);
				if (!alexer_expect_id(&l, t, ALEXER_PUNCT)) {
					exit(1);
				}
				da_append(p, f);
				break;
			case (K_swp):
				f.ft = F_swp;
				alexer_get_token(&l, &t);
				if (!alexer_expect_id(&l, t, ALEXER_PUNCT)) {
					exit(1);
				}
				da_append(p, f);
				break;	
			default:
				UNREACHABLE("Function not implemented or doesnt exist");
		}
	}
	
	if (!alexer_expect_id(&l, t, ALEXER_END)) {
		exit(1);
	}
}

int sortByIndex(const void* a, const void* b) {
	return ((Function*)a)->index - ((Function*)b)->index;
}

void sortFunctions(Program *p) {
	qsort(p->items, p->count, sizeof(Function), sortByIndex);
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("[ERROR] You need to supply a program to run.");
		return 1;
	}
	char *fp = argv[1];
	Program p = {0};
	Returns r = {0};
	createFromFile(fp, &p);	
	sortFunctions(&p);
	exec(&p, &r);
	
	return 0;
}