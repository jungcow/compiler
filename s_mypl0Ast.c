
#include "s_mypl0Ast.h"

myAstNode * root;

myAstNode * buildTree(int op, myAstNode * left) {
	myAstNode * tmp;
	tmp = (myAstNode*)malloc(sizeof(myAstNode));
	tmp->op = op;
	tmp->left = left;		tmp->right = NULL;
	return tmp;
}

myAstNode * buildNode(int op, Leaf value) {
	myAstNode * tmp;
	tmp = (myAstNode*)malloc(sizeof(myAstNode));
	tmp->op = op;
	tmp->value = value;
	tmp->left = NULL; 		tmp->right = NULL;
	return tmp;
}

myAstNode * linking(myAstNode * first, myAstNode * second) {
	myAstNode * tmp=first;
	if (first) {
		while (tmp->right) tmp = tmp->right;
		tmp->right = second;
		return first; 
	}
	else return second;
}

void printTree(myAstNode * node, int depth) {
	int i;
	if(node == NULL ) return;
	for(i = 0 ; i < depth ; i++) printf(" ");
	if (node->op==-1) 
		printf("%d\n", node->value.num); 
	else if (node->op==-2)
		printf("%s\n", node->value.ident);
	else { 
		switch(node->op) {
			case '+': printf("+ "); break;
			case '-': printf("- "); break;
			case '*': printf("* "); break;
			case '/': printf("/ "); break;
			case ODD: printf("odd "); break;
			case NEG: printf("u- "); break;
			case '=': printf("= "); break;
			case '<': printf("< "); break;
			case '>': printf("> "); break;
			case NE: printf("NE "); break;
			case GE: printf("GE "); break;
			case LE: printf("LE "); break;
			case TCONST: printf("TCONST "); break;
			case TVAR: printf("TVAR "); break;
			case TPROC: printf("TPROC "); break;
			case ASSIGN: printf("ASSIGN "); break;
			case TCALL: printf("TCALL "); break;
			case TBEGIN: printf("TBEGIN "); break; 
			case TIF: printf("TIF "); break;
			case TWHILE: printf("TWHILE "); break;
		}
		printf("   %d\n", node->op);
		printTree(node->left, depth+5);	
	}
	if (node->right) printTree(node->right, depth);
}


