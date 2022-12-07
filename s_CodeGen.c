#include "s_interpreter.c"
#include "s_mypl0Ast.h"
#include "y.tab.h"

#define CONST 0
#define VAR 1
#define PROC 2
#define IDENT 3 /* CONST + VAR */

#define TBSIZE 200
#define BKSIZE 2
#define LVLMAX 20  // max. block level depth

void initSymbolTable(void);
void Block(myAstNode *);
void Statement(myAstNode *);
void Expression(myAstNode *);

int Lookup(char *, int);            // lookup sym name for type,
									// if found setup LDiff/OFFSET and return 1 else return 0
void Enter(char *, int, int, int);  // symbol insert
void SetBlock();
void ResetBlock();
void DisplayTable();                // symbol table dump
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
	int link;  // collision이 발생하면 충돌 해결 방법으로 backward linking을 사용하기 위함
} table[TBSIZE];

int block[LVLMAX];  // for nesting block
int bucket[BKSIZE];

int tx = 0, cdx = 0, level = 0, lev = 0;
int LDiff = 0, Lno = 0, OFFSET = 0;
Instruction i;
int Lab[20];

void Traverse(myAstNode *node)
{
	initSymbolTable();
	Block(node);
	Emit("END", 7);
#if DEBUG
	DisplayTable();
#endif
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
			Enter(link->value.ident, CONST, level, link->right->value.num);
			// 얘는 stack에 안들어가므로 offset을 늘려줄 필요가 없다.
			// link는 id-num-id-num과 같이 연결되어 있으므로, 다음 enter는 id-num을 건너뛰어야 한다.
			link = link->right->right;
		}
		node = node->right;
	}
	if (node && node->op == TVAR)
	{
		link = node->left;
		while (link)
		{
			Enter(link->value.ident, VAR, level, offset++);
			link = link->right;
		}
		node = node->right;
	}
	while (node && node->op == TPROC)
	{
		link = node->left;

		Enter(link->value.ident, PROC, level, cdx);
		tx_save = tx;
		EmitPname(link->value.ident);  // procedure의 이름을 출력한다.
		/**
		 * procedure들어가기 전에 Setblock으로 nesting block을 알림
		 * 이는 symbol table과 stack과 같은 block배열을 위함
		 */
		SetBlock();  // block[level++] = tx
		Block(link->right);
		Emit("RET", 0);  // Ret 0이라는 명령어를 Code에 저장

		/**
		 * 마찬가지로 procedure가 끝나면 reset block을 해줌
		 * 이 때 collision chain을 풀어줘야 한다. ResetBlock()함수가 그것을 해줄 것.
		 */
		ResetBlock();  // block[--level];
		tx = tx_save;  // 저장해뒀던 현재 table의 top index를 다시 가져온다. 아래에서 현재 level의 statement를 처리하기 위함
#if DEBUG
		DisplayTable();
#endif
		node = node->right;
	}
	EmitLab(lab);                  // lab에 대해 주소를 지정
	Emit1("INT", Int, 0, offset);  // 0(SL), 1(DL), 2(RA)를 포함해야 하므로 3을 추가하여 stack 공간을 구성
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
		 * procedure인지 파악 -> error처리, 아니면 -> const인지 variable인지를 파악, 명령어를 구분
		 * procedure : error
		 * const : Lit
		 * var : Lod
		 */
		if (Lookup(node->value.ident, 0))  // const
		{
			// printf("lit %s, offset: %d\n", node->value.ident, OFFSET);
			Emit1("LIT", Lit, LDiff, OFFSET);
			break;
		}
		else if (Lookup(node->value.ident, 1))  // var
		{
			// printf("lod %s, offset: %d\n", node->value.ident, OFFSET);
			Emit1("LOD", Lod, LDiff, OFFSET);
			break;
		}
		else if (Lookup(node->value.ident, 2))  // procedure
		{
			printf("symbol reference error: procedure is only for call instruction");
			break;
		}
		printf("undefined symbol error\n");
		exit(1);
		break;
	}
	// unary operator
	case NEG:
	{
		Expression(node->left);
		Emit("NEG", 1);
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
	{ /**
	   * ID ASSIGN Expression
	   * temp : ID
	   * temp->right : Expression
	   */
		Expression(temp->right);
		/**
		 * 변수로 LOOKUP -> LDiff와 OFFSET을 Emit1의 인자로 넣어준다.
		 * 여기서 OFFSET은 Expression에서 load한 값을 저장할 stack의 주소를 말한다.
		 * 변수는 symbol table에 값을 가지지 않고, offset을 가짐
		 * stack이라는 배열에 offset 위치에 변수에 해당하는 값을 넣어줌으로써 변수에 할당하는 것을 구현하는 것.
		 */

		// 음수라는 뜻은 바깥에서 안에서 선언된 변수를 참조한다는 뜻
		if (!Lookup(temp->value.ident, 1) || LDiff < 0)
		{
			printf("undefined variable symbol error : %s\n", temp->value.ident);
			break;
		}
		Emit1("STO", Sto, LDiff, OFFSET);
		break;
	}
	case TCALL:
	{ /**
	   * TCALL ID
	   * temp : ID
	   */
		if (!Lookup(temp->value.ident, 2) || LDiff < 0)
		{
			printf("undefined procedure symbol error: %s\n", temp->value.ident);
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
		 * TIF Condition TTHEN Statement
		 * temp : Condition
		 * temp->right : Statement
		 */
		Condition(temp);
		/**
		 * 위의 Condition이 false일 경우, 아래 JPC를 통해 lab1로 jump하게 된다.
		 * 따라서 그 밑에 있는 Statement에서 생성하는 binary code들은 실행하지 않고 건너뛰게 된다.
		 */
		lab1 = GenLab(Lname1);    // 이름 생성
		Emit3("JPC", Jpc, lab1);  // 출력만 해주기(현재 어디로 점프할지는 모름, 이름은 아는데, Code address는 모름)
		Statement(temp->right);
		EmitLab(lab1);  // 여기서 Code address를 넣어준다.
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

int Lookup(char *name, int type)
{
	int idx = tx;
	LDiff = -88;
	OFFSET = -88;

	/**
	 * hash 값으로 찾으면 복잡해짐 -> hash값이 같은 것이 있을 수도 있고,
	 * 또한 같은 것이 있다면 backward linking을 구성했을 테니, 이를 이용해 순회를 해서 찾아야 하기 때문
	 */
	int bucketIdx = hash(name, type);
	int tableIdx = bucket[bucketIdx];
	while (tableIdx >= 0)
	{
		if (strcmp(table[tableIdx].name, name) == 0 && table[tableIdx].type == type)
		{
			LDiff = level - table[tableIdx].lvl;
			OFFSET = table[tableIdx].offst;
			return 1;
		}
		tableIdx = table[tableIdx].link;
	}
	if (strcmp(table[tableIdx].name, name) == 0 && table[tableIdx].type == type)
	{
		LDiff = level - table[tableIdx].lvl;
		OFFSET = table[tableIdx].offst;
		return 1;
	}
	return 0;
}

void Enter(char *name, int type, int lvl, int offst)
{
	if (Lookup(name, type))
	{
		if (LDiff == 0)
		{
			printf("Redefined error\n");
			exit(1);
			return;
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
	// 0, 1, 2에는 return addr, static link, dynamic link가 있겠지만
	// procedure의 경우 Code의 index가 오므로 멋대로 + 3해서 계산하지 말 것, 계산해서 저장하지 말 것
	table[bucket[bucketIdx]].offst = offst;
#if DEBUG
	printf("[ Symbol Table Enter ] name: %s, type; %d, lvl: %d, offst: %d, link: %d, hash: %d\n", name, type, lvl, offst, table[bucket[bucketIdx]].link, bucketIdx);
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
int GenLab(char *label)
{
	/**
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
	 * Lab[Lno]에 현재 code address를 넣음으로써,
	 * Lab[현재 label] = cdx : 이 JMP 명령어를 실행시킬 Code address(index)가 cdx라는 뜻
	 * Code[Lab[현재 label]].a = cdx : Jmp할 곳의 Code address(index)가 cdx라는 뜻
	 */
	Lab[Lno] = cdx;
	sprintf(label, "LAB%d", Lno);
	return Lno++;
}

void EmitLab(int label)
{
	/**
	 * Lab[Lno]에 현재 code address를 넣음으로써,
	 * Lab[현재 label] = cdx : 이 JMP 명령어를 실행시킬 Code address(index)가 cdx라는 뜻
	 * Code[Lab[현재 label]].a = cdx : Jmp할 곳의 Code address(index)가 cdx라는 뜻
	 */
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
	/**
	 * Instruction Op : JMP
	 * level : 무시
	 * addr : label을 나타내는 Code.
	 */
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
	/**
	 * nesting block을 위한 stack에는 table의 현재 top idx가 들어간다.
	 * 이유 : table은 stack과 같이 0부터 위로 올라가며 쌓이는 구조.
	 * block이 끝나면 table의 top부터 이 전 block에서 가장 마지막으로
	 * 추가된 곳까지 pop을 해줘야 한다(ResetBlock) -> 마치 stack과 같다.
	 * 따라서 block의 현재 level에서의 가장 마지막에 있는 table의 index를 저장해놔야
	 * 그 지점까지 pop을 할 수 있을 것.
	 */
	block[level++] = tx;
}

void ResetBlock(void)
{
	/**
	 * hash table에서는 backwardlink로 충돌을 해결했기 때문에,
	 * reset 시 이를 원상복구 해주는 작업이 필요하다
	 */
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