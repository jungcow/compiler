#ifndef __MYAST_H__
#define __MYAST_H__
#include <stdio.h>
#include <stdlib.h>

typedef union {
	int num; // NUM value 값 
	char ident[10];  // ID string 값 
} Leaf;
typedef struct _myAst {
	int op;	// 중간노드 종류			
	Leaf value; // 터미널의 경우 op=-1 (NUM), op=-2 (ID)
	struct _myAst * left;	// 터미널의 경우 NULL
	struct _myAst * right;
} myAstNode;

myAstNode * buildTree(int op, myAstNode * left);
myAstNode * buildNode(int op, Leaf value);
myAstNode * linking( myAstNode * first, myAstNode * second );
void printTree(myAstNode * node, int depth);
void Traverse(myAstNode * node);

extern myAstNode * root;
#endif
