
#if DEBUG
#include "s_MyInterpreter.c"
#else
#include "s_interpreter.c"
#endif
#include "s_mypl0Ast.h"
#include "y.tab.h"

typedef int bool;
#define true 1
#define false 0

#define CONST 0
#define VAR 1
#define PROC 2
#define IDENT 3 /* CONST + VAR */
#define INDEXED 4 // for array

#define TBSIZE 200
#define BKSIZE 17
#define LVLMAX 20  // max. block level depth
#define WRNMAX 20

void initSymbolTable(void);
void Block(myAstNode *);
void Statement(myAstNode *);
void Expression(myAstNode *);

int Lookup(char *, int);            // lookup sym name for type,
									// if found setup LDiff/OFFSET and return 1 else return 0
void Enter(char *, int, int, int, int);  // symbol insert
void SetBlock();
void ResetBlock();
void DisplayTable();                // symbol table dump
void printWarning();

/**
 * @func: GenLab()
 * Label은 Jmp를 위해 만들어야 한다.
 * 이 때, Label의 mnemonic 표기는 "Lab[No]"로 표기하고,
 * 이를 위해 지금까지 Label이 얼마나 생성되었는지를 나타내는 Lno 변수가 필요하다.
 * 변수를 정리하자면 다음과 같다.
 *
 * @var: Lab - 배열, 해당 label의 Code address(instruction의 주소)를 담는다.
 * @var: label - "Lab[No]" 에 해당하는 문자열, mnemonic code 출력을 위해 필요하다.
 * @var: Lno - 현재 레이블이 몇개가 만들어졌는지를 알기 위한 전역변수이다.
 * @var: cdx - 현재 code가 몇개 만들어졌는지를 저장하기 위한 전역변수이다.
 */

/**
 * @func: GenLab()
 * Lab[Lno]에 현재 code address를 넣음으로써,
 * Lab[현재 label] = cdx : 이 JMP 명령어를 실행시킬 Code address(index)가 cdx라는 뜻
 * Code[Lab[현재 label]].a = cdx : Jmp할 곳의 Code address(index)가 cdx라는 뜻
 */
int GenLab(char *);                 // label 생성(번호와 label) 및 현재 코드주소 저장
void EmitLab(int);                  // label 출력 및 forward jump 주소 확정
void EmitLab1(int);                 // label 출력 및 backward jump 주소 저장
void EmitPname(char *label);        // procedure 이름 출력
void Emit1(char *, fct, int, int);  // INT, LOD, LIT, STO 코드생성
void Emit2(char *, int, char *);    // CAL 코드생성
void Emit3(char *, fct, int);       // jmp, jpc 코드 생성
void Emit(char *, int);             // opr 코드생성

unsigned int hash(char *name, int type);  // generate index, pointing an table element

// symbol table
struct
{
	char name[20];
	int type; /* 0-CONST	1-VARIABLE	2-PROCEDURE */
	int lvl;
	int offst;
	int link; // for collision chain(backward link) 
	int size;
} table[TBSIZE];

int block[LVLMAX];  // for nesting block
int bucket[BKSIZE];

int tx = 0, cdx = 0, level = 0, lev = 0;
int LDiff = 0, Lno = 0, OFFSET = 0, ARRSIZE = 0;
Instruction i;
int Lab[20];

char warning[20][100];
int warningIdx = 0;

void Traverse(myAstNode *node)
{
	initSymbolTable();
	Block(node);
	Emit("END", 7);
#if DEBUG
	DisplayTable();
#endif
	printWarning();
}

void initSymbolTable()
{
	for (int i = 0; i < TBSIZE; i++)
	{
		table[i].link = -1;
	}
	for (int i = 0; i < BKSIZE; i++)
	{
		bucket[i] = -1;
	}
}

void Block(myAstNode *node)
{
	myAstNode *link;
	char Lname[10];
	int lab;
	int tx_save, offset = 3;

	if (node == NULL)
		return;

	if (node->op != 0)
		printf("block dec error\n");
	lab = GenLab(Lname);
	Emit3("JMP", Jmp, lab);  // jump to block main body
	// processing declaration part
	node = node->left;
	if (node && node->op == TCONST)
	{
		link = node->left;
		while (link)
		{
			Enter(link->value.ident, CONST, level, link->right->value.num, -1);
			/**
			 * TCONST
			 * |
			 * id-num-id-num
			 */
			link = link->right->right;
		}
		node = node->right;
	}
	if (node && node->op == TVAR)
	{
		link = node->left;
		while (link)
		{
			if (link->left)
			{
				myAstNode *tmp = link->left;
				int size = tmp->right->value.num;
				Enter(tmp->value.ident, INDEXED, level, offset, size);
				offset += size;
			}
			else
			{
				Enter(link->value.ident, VAR, level, offset++, -1);
			}
			link = link->right;
		}
		node = node->right;
	}
	while (node && node->op == TPROC)
	{
		link = node->left;

		Enter(link->value.ident, PROC, level, cdx, -1);
		tx_save = tx;
		EmitPname(link->value.ident);  // print procedure name

		SetBlock();  // block[level++] = tx
		Block(link->right);
		ResetBlock();  // block[--level];
		Emit("RET", 0);  // store "RET 0" instrunction to Code[]

		tx = tx_save;  // restore a table top index
		node = node->right;
	}
	EmitLab(lab);                  // set lab address
	Emit1("INT", Int, 0, offset);
	Statement(node);
}

void Condition(myAstNode *node)
{
	if (node == NULL)
	{
		printf(" expression error in Condition\n");
		return;
	}
	switch (node->op)
	{
	case ODD:
	{
		// ODD Expression
		Expression(node->left);
		Emit("ODD", 6);
		break;
	}
	// binary condition
	case '=':  // Expression '=' Expression
	{
		Expression(node->left);
		Expression(node->left->right);
		Emit("EQ", 8);
		break;
	}
	case NE:  // Expression NE Expression
	{
		Expression(node->left);
		Expression(node->left->right);
		Emit("NE", 9);
		break;
	}
	case '<':  // Expression '<' Expression
	{
		Expression(node->left);
		Expression(node->left->right);
		Emit("LT", 10);
		break;
	}
	case '>':  // Expression '>' Expression
	{
		Expression(node->left);
		Expression(node->left->right);
		Emit("GT", 12);
		break;
	}
	case GE:  // Expression GE Expression
	{
		Expression(node->left);
		Expression(node->left->right);
		Emit("GE", 11);
		break;
	}
	case LE:  // Expression LE Expression
	{
		Expression(node->left);
		Expression(node->left->right);
		Emit("LE", 13);
		break;
	}
	}
}

void Expression(myAstNode *node)
{
	if (node == NULL)
	{
		printf(" expression error\n");
		return;
	}
	switch (node->op)
	{
	case -1:
	{
		Emit1("LIT", Lit, 0, node->value.num);
		break;
	}
	case -2:
	{
		/**
		 * procedure : error
		 * const : Lit
		 * var : Lod
		 */
		if (Lookup(node->value.ident, 0))  // const
			Emit1("LIT", Lit, LDiff, OFFSET);
		else if (Lookup(node->value.ident, 1))  // var
			Emit1("LOD", Lod, LDiff, OFFSET);
		else if (Lookup(node->value.ident, 4))
			Emit1("LDA", Lda, LDiff, OFFSET);
		else
		{
			if (Lookup(node->value.ident, 2))  // procedure
				printf("symbol reference error: procedure is only for call instruction");
			else
				printf("undefined symbol error\n");
			exit(1);
		}
		break;
	}
	// unary operator
	case NEG:
	{
		Expression(node->left);
		Emit("NEG", 1);
		break;
	}
	case '[':
	{
		if (!Lookup(node->left->value.ident, 4))
		{
			printf("reference error: variable : %s is not an array\n", node->left->value.ident);
			exit(1);
		}
		if (ARRSIZE <= node->left->right->value.num)
		{
			sprintf(warning[warningIdx++], "warning: array index %d is past the end of the array (which contains %d elements)\n", 
					node->left->right->value.num, ARRSIZE);
			printf("size: %d\n", node->left->right->value.num);
		}

		Expression(node->left);
		Expression(node->left->right);
		Emit("ADD", 2);
		Emit1("LDI", Ldi, 0, 0);
		break;
	}
	// binary operator
	case '+':
	{
		Expression(node->left);
		Expression(node->left->right);
		Emit("ADD", 2);
		break;
	}
	case '-':
	{
		Expression(node->left);
		Expression(node->left->right);
		Emit("SUB", 3);
		break;
	}
	case '*':
	{
		Expression(node->left);
		Expression(node->left->right);
		Emit("MUL", 4);
		break;
	}
	case '/':
	{
		Expression(node->left);
		Expression(node->left->right);
		Emit("DIV", 5);
		break;
	}
	}
}

void Statement(myAstNode *node)
{
	// Statement Part
	char Lname1[10], Lname2[10];
	int lab1, lab2;
	myAstNode *temp;

	if (node == NULL)
		return;
	temp = node->left;
	switch (node->op)
	{
	case ASSIGN:
	{
		/**
		 * Var ASSIGN Expression
		 * temp : Var
		 * temp->right : Expression
		 */
		if (temp->op == '[')
		{
			if (!Lookup(temp->left->value.ident, 4))
			{
				printf("reference error: variable : %s is not an array\n", temp->left->value.ident);
				exit(1);
			}
			if (ARRSIZE <= temp->left->right->value.num)
			{
				sprintf(warning[warningIdx++], "warning: array index %d is past the end of the array (which contains %d elements)\n", 
						temp->left->right->value.num, ARRSIZE);

				printf("size: %d\n", temp->left->right->value.num);
			}
			Expression(temp->left);
			Expression(temp->left->right);
			Emit("ADD", 2);
			Expression(temp->right);
			Emit1("STI", Sti, LDiff, OFFSET);
			break;
		}
		else
		{
			Expression(temp->right);
			if (Lookup(temp->value.ident, 1))
			{
				Emit1("STO", Sto, LDiff, OFFSET);
				break;
			}
		}
		printf("undefined variable symbol error : %s\n", temp->value.ident);
		exit(1);
		break;
	}
	case TCALL:
	{
		/**
		 * TCALL ID
		 * temp : ID
		 */
		if (!Lookup(temp->value.ident, 2) || LDiff < 0)
		{
			printf("undefined procedure symbol error: %s\n", temp->value.ident);
			exit(1);
			break;
		}
		Emit2("CAL", LDiff, temp->value.ident);
		break;
	}
	case TBEGIN:
	{
		while (temp)
		{
			Statement(temp);
			temp = temp->right;
		}
		break;
	}
	case TIF:
	{
		/**
		 * TIF Condition TTHEN Statement TELSE Statement
		 * temp : Condition
		 * temp->right : Statement (then의 statement)
		 * temp->right->right : Statement (else의 statement)
		 */
		Condition(temp); //condition of if
		lab1 = GenLab(Lname1);
		Emit3("JPC", Jpc, lab1);
		Statement(temp->right);
		if (temp->right->right)
		{
			// after then
			lab2 = GenLab(Lname2);
			Emit3("JMP", Jmp, lab2);
		}
		EmitLab(lab1); // else or after if-then-else block
		if (temp->right->right)
		{
			Statement(temp->right->right);
			EmitLab(lab2);
		}
		break;
	}
	case TWHILE:
	{
		lab1 = GenLab(Lname1);
		EmitLab1(lab1);
		Condition(temp);
		lab2 = GenLab(Lname2);
		Emit3("JPC", Jpc, lab2);
		Statement(temp->right);
		Emit3("JMP", Jmp, lab1);
		EmitLab(lab2);
		break;
	}
	break;
	}
}

bool	Lookup(char *name, int type)
{
	int idx = tx;
	LDiff = -88;
	OFFSET = -88;

	int bucketIdx = hash(name, type);
	int tableIdx = bucket[bucketIdx];
	while (tableIdx >= 0)
	{
		if (strcmp(table[tableIdx].name, name) == 0 && table[tableIdx].type == type)
		{
			LDiff = level - table[tableIdx].lvl;
			OFFSET = table[tableIdx].offst;
			ARRSIZE = table[tableIdx].size;
			return true;
		}
		tableIdx = table[tableIdx].link;
	}
	if (strcmp(table[tableIdx].name, name) == 0 && table[tableIdx].type == type)
	{
		LDiff = level - table[tableIdx].lvl;
		OFFSET = table[tableIdx].offst;
		ARRSIZE = table[tableIdx].size;
		return true;
	}
	return false;
}

void Enter(char *name, int type, int lvl, int offst, int size)
{
	if (Lookup(name, type))
	{
		if (LDiff == 0)
		{
			printf("Redefined error : [ name: %s, type: %d, level: %d, offst: %d ]\n",
						name, type, lvl, offst);
			exit(1);
		}
	}
	unsigned int bucketIdx = hash(name, type);
	if (bucket[bucketIdx] != -1)
	{
		// backward linking
		int origin = bucket[bucketIdx];
		bucket[bucketIdx] = tx++;
		table[bucket[bucketIdx]].link = origin;
#if DEBUG
		printf("<<<<<<<<<<<  backward linking occur!! >>>>>>>>>>>>>\n");
#endif
	}
	else
	{
		bucket[bucketIdx] = tx++;
	}
	strcpy(table[bucket[bucketIdx]].name, name);
	table[bucket[bucketIdx]].type = type;
	table[bucket[bucketIdx]].lvl = lvl;
	table[bucket[bucketIdx]].offst = offst;
	table[bucket[bucketIdx]].size = size;
#if DEBUG
	printf("[ Symbol Table Enter ] name: %s, type; %d, lvl: %d, offst: %d, link: %d, size: %d, hash: %d\n",
			name, type, lvl, offst, table[bucket[bucketIdx]].link, size, bucketIdx);
#endif
}

void DisplayTable()
{
	int idx = tx;
	while (--idx >= 0)
	{
		printf("%s	%d	%d	%d\n", table[idx].name,
			   table[idx].type, table[idx].lvl, table[idx].offst);
	}
}

void printWarning()
{
	for (int i = 0; i < warningIdx; i++)
	{
		printf("%s\n", warning[i]);
	}
}

int GenLab(char *label)
{
	Lab[Lno] = cdx;
	sprintf(label, "LAB%d", Lno);
	return Lno++;
}

void EmitLab(int label)
{
	Code[Lab[label]].a = cdx;
	printf("LAB%d\n", label);
}

void EmitLab1(int label)
{
	//	Code[Lab[label]].a=cdx;
	printf("LAB%d\n", label);
}

void EmitPname(char *label)
{
	printf("%s\n", label);
}

void Emit1(char *code, fct op, int ld, int offst)
{
	printf("	%s	%d	%d\n", code, ld, offst);
	i.f = op;
	i.l = ld;
	i.a = offst;
#if DEBUG
	printf("[ Lod ] cdx: %d, f: %d, l: %d, a: %d\n", cdx, i.f, i.l, i.a);
#endif
	Code[cdx++] = i;
}

void Emit2(char *code, int ld, char *name)
{  // emit "Cal l,addr"
	printf("	%s	%d	%s\n", code, ld, name);
	i.f = Cal;
	i.l = ld;
	i.a = OFFSET;
#if DEBUG
	printf("cdx: %d, f: %d, l: %d, a: %d\n", cdx, i.f, i.l, i.a);
#endif
	Code[cdx++] = i;
}

void Emit3(char *code, fct op, int label)
{
	printf("	%s	LAB%d\n", code, label);
	i.f = op;
	i.l = 0;
	i.a = Lab[label];
#if DEBUG
	printf("cdx: %d, f: %d, l: %d, a: %d\n", cdx, i.f, i.l, i.a);
#endif
	Code[cdx++] = i;
}

void Emit(char *code, int op)
{
	printf("	%s\n", code);
	i.f = Opr;
	i.l = 0;
	i.a = op;

#if DEBUG
	printf("cdx: %d, f: %d, l: %d, a: %d\n", cdx, i.f, i.l, i.a);
#endif
	Code[cdx++] = i;
}

unsigned int hash(char *name, int type)
{
	unsigned int h = 0;

	for (int i = 0; name[i] != '\0'; i++)
	{
		h = (h << 4) + name[i];
	}
	h = (h << 4) + type;
#if DEBUG
	printf("name: %s, type: %d => hash : %u\n", name, type, (h % BKSIZE));
#endif
	return h % BKSIZE;
}

void SetBlock(void)
{
	block[level++] = tx;
}

void ResetBlock(void)
{
	int previousBlockTx = block[--level];

	for (; tx > previousBlockTx; tx--)
	{
		int backlink = table[tx - 1].link;
		if (backlink < 0)
			continue;
#if DEBUG
		printf("symbol name: %s -> [[[[[[  Reset Collision Chain ]]]]]]]]]]\n", table[tx - 1].name);
#endif
		char *name = table[tx - 1].name;
		int type = table[tx - 1].type;
		bucket[hash(name, type)] = backlink;
	}
}
