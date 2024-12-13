#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include <nob.h>
#define ALEXER_IMPLEMENTATION
#include <alexer.h>

#define liberate(ptr) free(ptr)
#define da_liberate(da) da_free(da)

typedef enum {
	T_Int,
	T_Float,
	T_Bool,
	T_String,
} Type;

typedef enum {
	le,
	lt,
	ge,
	gt,
	ne,
	eq,
} CmpType;

typedef union {
	size_t i;
	float f;
	bool b;
	char *s;
} Value;

typedef struct {
	Type t;
	Value v;
} Node;

typedef struct {
	size_t count;
	size_t capacity;
	Node *items;
} Stack;

typedef enum {
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
} FunctionType;

typedef struct {
	FunctionType ft;
	CmpType ct;
	Node n;
	size_t op;
} Function;

typedef struct {
	size_t capacity;
	size_t count;
	Function *items;
} Program;

void push(Stack *s,Node n) {
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
				printf("%ld: Int: %ld\n", i, s->items[i].v.i);
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
	comperand_left = s->items[s->count-1].v.i;
	comperand_right = s->items[s->count-2].v.i;
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

void eval(Stack *s, Function f) {
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
		default:
			printf("[ERROR] Invalid Function Type called");
			exit(1);
	}
}

void exec(Program *p) {
	Stack s = {0};
	for (size_t i = 0; i < p->count; ++i) {
		eval(&s, p->items[i]);
	}

	da_liberate(s);
	da_liberate(*p);
}

typedef enum {
	P_semicolon,
	P_count,
} PunctIndex;

const char *puncts[P_count] = {
	[P_semicolon] = ";",
};

typedef enum {
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
	[K_print]   = "print",
};

const char *comments[] = {
	"//",
};

void createFromFile(char *fp, Program *p) {
	String_Builder sb = {0};
	read_entire_file(fp, &sb);
	char *ps = malloc(sb.count + 1); // we love off by one errors!
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
		if (!alexer_expect_id(&l, t, ALEXER_INT)) {
			exit(1);
		}
		alexer_get_token(&l, &t);
		// TODO: check for keyword???
		//uint64_t expected_kw[] = {K_addi, K_icmp, K_isub, K_pop, K_prstk, K_push, K_scat, K_scmp};
		//if (!alexer_expect_one_of_ids(&l, t, expected_kw, ALEXER_ARRAY_LEN(expected_kw))) { // TODO: somehow fix ????????
		//	exit(1);
		//}
		Function f;
		switch (ALEXER_INDEX(t.id)) {
			case (K_push):
				f.ft = F_push;
				alexer_get_token(&l, &t);
				uint64_t expected[] = {ALEXER_INT}; // TODO: add other kinds
				if (!alexer_expect_one_of_ids(&l, t, expected, ALEXER_ARRAY_LEN(expected))) {
					exit(1);
				}
				switch (ALEXER_KIND(t.id)) {
					case (ALEXER_INT):
						Node n;
						n.t = T_Int;
						n.v.i = t.int_value;
						f.n = n;
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
				printf("uh oh (not implemented)\n");
				exit(1);
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
			default:
				UNREACHABLE("Function not implemented or doesnt exist");
		}
	}
	
	if (!alexer_expect_id(&l, t, ALEXER_END)) {
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("[ERROR] You need to supply a program to run.");
		exit(1);
	}
	char *fp = argv[1];
	Program p = {0};
	createFromFile(fp, &p);	
	exec(&p);
	
	return 0;
}