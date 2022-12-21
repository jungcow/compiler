// PL0 interpreter

//#define Debug 1
#include <stdio.h>
#include <string.h>
#include <unistd.h>
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
	Jpc,
	Lda,
	Ldi,
	Sti
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
	int tmp = 0;
	do
	{
		// if (sp > 0)
		// 	printf("sp: %d, pc : %d, *sp: %d ", sp, pc, s[sp]);
		// if (tmp > 50)
		// 	break;
		// tmp++;
		i = Code[pc++];          // fetch currrent instr.
		addr = base(i.l) + i.a;  // printf("addr=%d\n", addr);
		// if (i.f == 2 && i.a == 19)
		// 	addr = 3;
		printf("pc: %d, i-function: %d, i-level: %d, i-offset: %d -> addr: %d\n", pc - 1, i.f, i.l, i.a, addr);
#if Debug
		printf("%d	%d	%d	%d\n", pc - 1, i.f, i.l, i.a);
#endif
		switch (i.f)
		{  // branch by ft. code
		case Lit:
			printf("Lit %d\n", i.a);
			s[++sp] = i.a;
			break;
		case Opr:
			printf("OPR : ");
			switch (i.a)
			{
			case 0:
				printf("RET\n");
				sp = mp - 1;
				pc = s[sp + 3];
				mp = s[sp + 2];
				break;  // return
			case 1:
				printf("NEG, %d\n", -s[sp]);
				s[sp] = -s[sp];
				break;  // negate
			case 2:
				--sp;
				printf("%d + %d \n", s[sp], s[sp + 1]);
				s[sp] = s[sp] + s[sp + 1];
				break;  // +
			case 3:
				--sp;
				printf("%d - %d\n", s[sp], s[sp + 1]);
				s[sp] = s[sp] - s[sp + 1];
				break;  // -
			case 4:
				--sp;
				printf("%d * %d\n", s[sp], s[sp + 1]);
				s[sp] = s[sp] * s[sp + 1];
				break;  // *
			case 5:
				--sp;
				printf("%d / %d\n", s[sp], s[sp + 1]);
				s[sp] = s[sp] / s[sp + 1];
				break;  // /
			case 6:
				printf("ODD, %d\n", s[sp]);
				s[sp] = odd(s[sp]);
				break;  // odd
			case 7:
				printf("END\n");
				pc = 0;
				break;  // END
			case 8:
				--sp;
				printf("%d == %d\n", s[sp], s[sp + 1]);
				s[sp] = s[sp] == s[sp + 1];
				break;  // =
			case 9:
				--sp;
				printf("%d != %d\n", s[sp], s[sp + 1]);
				s[sp] = s[sp] != s[sp + 1];
				break;  // !=
			case 10:
				--sp;
				printf("%d < %d\n", s[sp], s[sp + 1]);
				s[sp] = s[sp] < s[sp + 1];
				break;  // <
			case 11:
				--sp;
				printf("%d >= %d\n", s[sp], s[sp + 1]);
				s[sp] = s[sp] >= s[sp + 1];
				break;  // >=
			case 12:
				--sp;
				printf("%d > %d\n", s[sp], s[sp + 1]);
				s[sp] = s[sp] > s[sp + 1];
				break;  // >
			case 13:
				--sp;
				printf("%d <= %d\n", s[sp], s[sp + 1]);
				s[sp] = s[sp] <= s[sp + 1];
				break;  // <=
			};
			break;
		case Lod:
			s[++sp] = s[addr];
			printf("Lod, s[%d] : %d, addr: %d\n", sp + 1, s[addr], addr);
			break;
		case Sto:
			printf("Sto, s[%d] : %d\n", addr, s[sp]);
			s[addr] = s[sp--];
			break;
		case Cal:
			printf("Cal\n");
			s[sp + 1] = base(i.l);
			s[sp + 2] = mp;
			s[sp + 3] = pc;
			mp = sp + 1;
			pc = i.a;
			break;
		case Int:
			printf("INT %d => %d -> %d\n", i.a, sp, sp + i.a);
			sp = sp + i.a;
			break;
		case Jmp:
			printf("Jmp %d\n", i.a);
			pc = i.a;
			break;
		case Jpc:
			--sp;
			printf("Jpc -> ");
			if (s[sp + 1] == 0)
			{
				printf("Jmp! %d, sp: %d\n", i.a, sp);
				pc = i.a;
				break;  // TODO: 지우기
			}
			printf("s[%d] != 0 -> No jmp.., sp: %d\n", sp + 1, sp);
			break;
		case Lda: s[++sp] = addr; 
				  printf("Interpreter: [LDA]\n");
				  break;
		case Ldi: s[sp] = s[s[sp]]; 
				  printf("Interpreter: [LDI]\n");
				  break;
		case Sti: s[s[sp-1]] = s[sp]; 
				  if (s[sp-1] != sp)
					  sp -=2;
				  else
					  --sp;
				  printf("Interpreter: [STI]\n");
				  break;
		};
		printf("===========[ stack ]===========\n");
		int sptmp = sp;
		for (; sptmp >= 0; sptmp--)
		{
			printf("sp[%d]: %d\n", sptmp, s[sptmp]);
		}
		printf("================================\n\n");
		usleep(200000);
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
