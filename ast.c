//함수 수동으로 복원해보기...
// 2,6, 

//2번 함수 my_realloc
/*
Function Number : 2
Name : my_realloc
Return Type : char*
Arguments - 
    Type: char*
    Name: old

    Type: int
    Name: oldlen

    Type: int
    Name: newlen

If Statement Count : 0
*/

char* my_realloc(char* old, int oldlen, int newlen) {
    char* new = (char*)malloc(newlen);
    int i = 0;
    while( i <= oldlen - 1) {
        new[i] = old[i];
        i = i + 1;
    }
    return new;
}


// 11번 함수 emit
/*
Function Number : 11
Name : emit
Return Type : void
Arguments - 
    Type: int
    Name: n

    Type: char
    Name: s

If Statement Count : 1
===========================================
*/

void emit(int n, char s) {
    i = 0;
    if ( code_size <= codepos + n ) {
        int x = (codepos + n) << 1;
        code = my_realloc(code, code_size, x);
        code_size = x;
    }

    while ( i <= n - 1 ) {
        code[codepos] = s[i];
        codepose = codepos + 1;
        i = i + 1;
    }
}

// 8번 함수 expect 
/*
Function Number : 8
Name : expect
Return Type : void
Arguments - 
    Type: char*
    Name: s

If Statement Count : 1
===========================================
*/

void expect(char* s) {
    if ( accept(s) == 0 ) error();
}