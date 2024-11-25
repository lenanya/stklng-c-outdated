#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include <nob.h>

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
	int i;
	float f;
	bool b;
	char * s;
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

void push(Stack *s,Node n) {
	da_append(s, n);
}

void pop_many_d(Stack *s, size_t amount, char *file, size_t line) {
	if (amount > s->count) {
		printf("[ERROR] Cannot pop more items than there are on the Stack [%s:%d]\n", file, line);
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
				printf("%d: Int: %d\n", i, s->items[i].v.i);
				break;
			case (T_Float):
				printf("%d: Float: %f\n", i, s->items[i].v.f);
				break;
			case (T_Bool):
				if (s->items[i].v.b == 0) {
					printf("%d: Bool: false\n", i);
				} else {
					printf("%d: Bool: true\n", i);
				}
				break;
			case (T_String):
				printf("%d: String: %s\n", i, s->items[i].v.s);
				break;
			default:
				break;
		}
	}
	printf("---- End of Stack ----\n\n");
}

void addi_d(Stack *s, char *file, size_t line) {
	if (s->items[s->count-1].t != T_Int || s->items[s->count - 2].t != T_Int || s->count < 2) {
		printf("[ERROR] addi requires 2 Integers at the top of the stack [%s:%d]\n", file, line);
		exit(1);
	}
	int addend = s->items[s->count-1].v.i;
	pop(s);
	s->items[s->count-1].v.i += addend;
}

#define addi(s) addi_d(s, __FILE__, __LINE__)

void addf_d(Stack *s, char *file, size_t line) {
	if (s->items[s->count-1].t != T_Float || s->items[s->count - 2].t != T_Float || s->count < 2) {
		printf("[ERROR] addf requires 2 Floats at the top of the stack [%s:%d]\n", file, line);
		exit(1);
	}
	float addend = s->items[s->count-1].v.f;
	pop(s);
	s->items[s->count-1].v.f += addend;
}

#define addf(s) addf_d(s, __FILE__, __LINE__)

void isub_d(Stack *s, char *file, size_t line) {
	if (s->items[s->count-1].t != T_Int || s->items[s->count - 2].t != T_Int || s->count < 2) {
		printf("[ERROR] isub requires 2 Integers at the top of the stack [%s:%d]\n", file, line);
		exit(1);
	}
	int subtrahend = s->items[s->count-1].v.i;
	pop(s);
	s->items[s->count-1].v.i -= subtrahend;
}

#define isub(s) isub_d(s, __FILE__, __LINE__)

void fsub_d(Stack *s, char *file, size_t line) {
	if (s->items[s->count-1].t != T_Float || s->items[s->count - 2].t != T_Float || s->count < 2) {
		printf("[ERROR] fsub requires 2 Floats at the top of the stack [%s:%d]\n", file, line);
		exit(1);
	}
	float subtrahend = s->items[s->count-1].v.f;
	pop(s);
	s->items[s->count-1].v.f -= subtrahend;
}

#define fsub(s) fsub_d(s, __FILE__, __LINE__)

void scat_d(Stack *s, char* file, size_t line) {
	if (s->items[s->count-1].t != T_String || s->items[s->count - 2].t != T_String || s->count < 2) {
		printf("[ERROR] scat requires 2 Strings at the top of the stack [%s:%d]\n", file, line);
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
		printf("[ERROR] icmp requires 2 Integers at the top of the stack [%s:%d]\n", file, line);
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
			printf("[ERROR] icmp requires valid comparison type! [%s:%d]\n", file, line);
			exit(1);
	}
	push(s, cmp_bool);
}

#define icmp(s, t) icmp_d(s, t, __FILE__, __LINE__)

void fcmp_d(Stack *s, CmpType t, char *file, size_t line) {
	if (s->items[s->count-1].t != T_Float || s->items[s->count - 2].t != T_Float || s->count < 2) {
		printf("[ERROR] fcmp requires 2 Floats at the top of the stack [%s:%d]\n", file, line);
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
			printf("[ERROR] fcmp requires valid comparison type! [%s:%d]\n", file, line);
			exit(1);
	}
	push(s, cmp_bool);
}

#define fcmp(s, t) fcmp_d(s, t, __FILE__, __LINE)

void scmp_d(Stack *s, CmpType t, char* file, size_t *line) {
	if (t != eq && t != ne) {
		printf("[ERROR] scmp can only be done with eq or ne [%s:%d]\n", file, line);
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
			printf("[ERROR] scmp requires valid comparison type [%s:%s]\n", file, line);
			exit(1);
	}
	push(s, cmp_bool);
} 

#define scmp(s, t) scmp_d(s, t, __FILE__, __LINE__);

int main(int argc, char *argv[])
{
	Stack s = {0};
	Node strn1, strn2, strn3;
	strn1.t = T_String;
	strn2.t = T_String;
	strn3.t = T_String;
	char *str1 = "ur ";
	char *str2 = "mom ";
	char *str3 = "gey";
	strn1.v.s = str1;
	strn2.v.s = str2;
	strn3.v.s = str3;
	push(&s, strn1);
	push(&s, strn2);
	push(&s, strn3);
	prstk(&s);
	scat(&s);
	prstk(&s);
	scat(&s);
	prstk(&s);
	pop_many(&s, 3);

	da_free(s);

	return 0;
}