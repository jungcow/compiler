## Execute

```shell
lex s_pl0Ast.l && yacc -d s_pl0Ast.y && gcc y.tab.c && ./a.out < sample/[sample file]
```

-   Hash Symbol Table
    -   [x] hash
    -   [x] Enter
    -   [x] Lookup
    -   [x] SetBlock
    -   [x] Resetblock
-   Block
    -   [x] node->op == TCONST
    -   [x] node->op == TVAR
    -   [x] node->op == TPROC
-   Statement
    -   [x] ASSIGN
    -   [x] TCALL
    -   [x] TBEGIN
    -   [x] TIF
    -   [x] TIF (Extended: TIF Condition TTHEN Statement TELSE Statement)
    -   [x] TWHILE
    -   [ ] OTHER???
-   Expression
    -   [x] -1
    -   [x] -2
    -   [x] NEG
    -   [x] '+'
    -   [x] '-'
    -   [x] '\*'
    -   [x] '/'
-   Condition

    -   [x] ODD
    -   [x] '='
    -   [x] NE
    -   [x] '<'
    -   [x] '>'
    -   [x] GE
    -   [x] LE

-   More
    -   [ ] Indexed Variable
    -   [ ] Function call using parameter
