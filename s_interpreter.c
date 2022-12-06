// PL0 interpreter

//#define Debug 1
#include <stdio.h>
#include <string.h>
#define cxmax 200
#define stksize 500

typedef enum
{
	Lit,
	Opr,
	Lod,
	Sto,
	Cal,
	Int,
	Jmp,
	Jpc
} fct;
typedef struct
{
	fct f;  // function code
	int l;  // level
	int a;  // offset
} Instruction;

int pc = 0, mp = 0, sp = -1;  // program, base, stacktop reg.

Instruction Code[cxmax];  // code_store indexed by pc
int s[stksize];           // data_store (stack storage) indexed by sp

// local stack frame (Ȱ�����ڵ�) based mp
// 	offset 0: s[mp]): SL
// 	offset 1: s[mp+1]: DL
// 	offset 2: s[mp+2]: RA

// lod/sto l,a
//	l: level difference
//	a: offset

//	cal l,a
//	l: level difference
//	a: procedure entry pt.

int base(int l)
{
	int b1;
	b1 = mp;  // find base l levels down
	while (l > 0)
	{
		b1 = s[b1];
		--l;
	}
	//	printf("base=%d\n", b1);
	return b1;
}  // end base

int odd(int a)
{
	if (a / 2 * 2 == a)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
void interprete()
{
	int t;
	Instruction i;  // IR reg.
	int addr = 0;
	// load pl0_code to Code_store
	while (scanf("%d %d %d %d", &addr, &i.f, &i.l, &i.a) != EOF)
	{
		Code[addr] = i;  // Code[addr].f, Code[addr].l, Code[addr].a
						 /*		printf("%d   %d   %d   %d\n", addr, i.f, i.l, i.a); // pl0_code load test  */
	}

	printf("=== start PL0 ===\n");
	s[0] = s[1] = s[2] = 0;  // stack clear
	// int tmp = 0;
	do
	{
		// if (sp > 0)
		// 	printf("sp: %d, pc : %d, *sp: %d ", sp, pc, s[sp]);
		// if (tmp > 50)
		// 	break;
		// tmp++;
		i = Code[pc++];          // fetch currrent instr.
		addr = base(i.l) + i.a;  // printf("addr=%d\n", addr);
#if Debug
		printf("%d	%d	%d	%d\n", pc - 1, i.f, i.l, i.a);
#endif
		switch (i.f)
		{  // branch by ft. code
		case Lit:
			s[++sp] = i.a;
			break;
		case Opr:
			switch (i.a)
			{
			case 0:
				sp = mp - 1;
				pc = s[sp + 3];
				mp = s[sp + 2];
				break;  // return
			case 1:
				s[sp] = -s[sp];
				break;  // negate
			case 2:
				--sp;
				s[sp] = s[sp] + s[sp + 1];
				break;  // +
			case 3:
				--sp;
				s[sp] = s[sp] - s[sp + 1];
				break;  // -
			case 4:
				--sp;
				s[sp] = s[sp] * s[sp + 1];
				break;  // *
			case 5:
				--sp;
				s[sp] = s[sp] / s[sp + 1];
				break;  // /
			case 6:
				s[sp] = odd(s[sp]);
				break;  // odd
			case 7:
				pc = 0;
				break;  // END
			case 8:
				--sp;
				s[sp] = s[sp] == s[sp + 1];
				break;  // =
			case 9:
				--sp;
				s[sp] = s[sp] != s[sp + 1];
				break;  // !=
			case 10:
				--sp;
				printf("\'<\'");
				printf("a < b -> a : %d, b : %d", s[sp], s[sp + 1]);
				s[sp] = s[sp] < s[sp + 1];
				break;  // <
			case 11:
				--sp;
				printf("\'>=\'");
				s[sp] = s[sp] >= s[sp + 1];
				break;  // >=
			case 12:
				--sp;
				printf("\'>\'");
				s[sp] = s[sp] > s[sp + 1];
				break;  // >
			case 13:
				--sp;
				printf("\'<=\'");
				s[sp] = s[sp] <= s[sp + 1];
				break;  // <=
			};
			break;
		case Lod:
			s[++sp] = s[addr];
			break;
		case Sto:
			s[addr] = s[sp--];
			break;
		case Cal:
			s[sp + 1] = base(i.l);
			s[sp + 2] = mp;
			s[sp + 3] = pc;
			mp = sp + 1;
			pc = i.a;
			sp += 3;
			break;
		case Int:
			sp = sp + i.a;
			break;
		case Jmp:
			pc = i.a;
			break;
		case Jpc:
			--sp;
			if (s[sp + 1] == 0)
			{
				printf("jmp");
				pc = i.a;
			}
			break;
		};
		// printf("\n");
		// int sptmp = sp;
		// for (; sptmp >= 0; sptmp--)
		// {
		// 	printf("sp[%d]: %d\n", sptmp, s[sptmp]);
		// }
	} while (pc);  // loop until pc=0
	printf("=== execution result(global var. contents) ===\n");
	while (sp > 2)
	{
		printf("stack:	%d	%d\n", sp, s[sp]);
		--sp;
	}
};

// void main() {
// 	interprete();
// }
