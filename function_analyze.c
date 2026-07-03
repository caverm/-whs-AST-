#include "json_c.c"

#define MAX_ARG_NUMBER 10
#define MAX_ARG_LENGTH 30

typedef struct Function_Box {
    int index;
    char* func_name;
    char* returnType;
    char argsType[MAX_ARG_NUMBER][MAX_ARG_LENGTH];
    char argsName[MAX_ARG_NUMBER][MAX_ARG_LENGTH];
    int if_number;
} FB;

void print_FB(FB* fb, int func_count) {

    printf("총 함수 개수 : %d\n\n", func_count);

    for (int i = 0; i < func_count; i++) {
        printf("Function Number : %d\n", fb[i].index);
        printf("Name : %s\n", fb[i].func_name);
        printf("Return Type : %s\n", fb[i].returnType);
        printf("Arguments - \n");
        for (int j = 0; j < MAX_ARG_NUMBER; j++) {
            if (fb[i].argsType[j][0] != '\0') { 
                printf("    Type: %s\n    Name: %s\n\n", fb[i].argsType[j], fb[i].argsName[j]);
            }
        }
        printf("If Statement Count : %d\n", fb[i].if_number);

        printf("===========================================\n\n");
    }
}

char* get_FunctionName(json_value decl) {
    char* func_name = json_get_string(decl, "name");
    return func_name;
}

char* get_ReturnType(json_value decl) {

    char* pointer = "*";
    int pointer_count = 0;
    
    while (decl.value != NULL) { 

        char* nodetype = json_get_string(decl, "_nodetype");

        if (strncmp(nodetype, "PtrDecl", strlen("PtrDecl")) == 0) {
            pointer_count = 1;
        } 

        if (nodetype != NULL && strncmp(nodetype, "IdentifierType", strlen("IdentifierType")) == 0) {
            char* returnType = json_get_string(decl, "names", 0);

            //포인터 타입 생각하기
            int len = strlen(returnType) + 2; // 포인터랑 널 공간 2
            char* final_returnType = (char*)malloc(len);

            strlcpy(final_returnType, returnType, len);

            if (pointer_count == 1) {
                strlcat(final_returnType, pointer, len);
            }

            return final_returnType;
        } else {
            decl = json_get(decl, "type");
        }
    }

    printf("탐색 중 오류 발생");
    return NULL;
}

char* get_ArgsType(json_value params, int arg_index) {
    json_value param = json_get(params, arg_index);
    return get_ReturnType(param);
}

char* get_ArgsName(json_value params, int arg_index) {
    json_value param = json_get(params, arg_index);
    return json_get_string(param, "name");
}

/* 
if문 구하는 아이디어
일단 단순 변수 선언은 제외. if를 가질 수 있는 경우는 다음과 같음
조건문, 반복문 정도... ( if, while, for, do-while, switch-case )
이를 대상으로 _nodetype이 If인 경우를 찾아 count를 증가시키기...
재귀를 통해 탐색해야함
*/
int get_If(json_value node) {
    //"_nodetype": "If" 이걸 추출해야해...
    if (node.value == NULL) {
        return 0; // 빈 노드
    }

    int count = 0; 
    char* nodetype = json_get_string(node, "_nodetype");

    if (nodetype == NULL) return 0;

    // 조건문(if) 인 경우...
    if(strncmp(nodetype, "If", strlen("If")) == 0) {
        count++;

        //iffalse, iftrue 확인
        count = count + get_If(json_get(node, "iffalse"));
        count = count + get_If(json_get(node, "iftrue"));
    } else if ( strncmp(nodetype, "Compound", strlen("Compound")) == 0 ) {
        // Compound 노드는 blockitems 배열을 가지고 있음. 배열 재탐색 필요...
        json_value compound_array = json_get(node, "block_items");
        
        if (compound_array.value != NULL) {
            int last_index = json_get_last_index(compound_array);
            for (int i = 0; i <= last_index; i++) {
                count = count + get_If(json_get(compound_array, i));
            }
        } else { // 비어있는 compound는 탐색하지 않아도 됨 
        }
    } else if (strncmp(nodetype, "While", strlen("While")) == 0 || 
               strncmp(nodetype, "For", strlen("For")) == 0 || 
               strncmp(nodetype, "DoWhile", strlen("DoWhile")) == 0 || 
               strncmp(nodetype, "Switch", strlen("Switch")) == 0)  {
               // 안에 if문을 가질 수 있는 반복문, switch-case 탐색필요
        count = count + get_If(json_get(node, "stmt"));
               }
    return count;
}

int main(void) 
{
    FILE *fp;
    char line_buff[100] = {0};
    fp = fopen("ast.json", "r");

    if (fp == NULL) printf("파일 열기 실패!\n");
    else printf("파일 열기 성공!\n");

    //파일 크기 구하기
    int size = 0;
    int c;
    while ((c = fgetc(fp)) != EOF) {
        size++;
    }
    
    //동적 할당(0초기화) & 커서 앞으로 
    char* json_buff = (char*)malloc(size + 1); 
    memset(json_buff, 0, size);
    fseek(fp, 0, SEEK_SET);

    while(fgets(line_buff, sizeof(line_buff), fp) != NULL) {
        strlcat(json_buff, line_buff, size);
    } 

    json_value json = json_create(json_buff);

    //ast.json 파일의 ext 배열에서 원소 하나 구조

    // 1. 함수 개수 구하기
    // 3. 함수 이름 추출하기
    // ext 배열 안에는 전역 변수, 타입 및 구조체 정의, 함수 자체가 들어가 있다.
    // C언어의 모든 함수는 AST로 변환되면 무조건 최상단 노드인 ext 배열 안에 모여있게 된다...

    int func_count = 0;
    json_value ext = json_get(json, "ext");

    // 마지막 인덱스 구하기
    int last_index = json_get_last_index(ext); 

    // 함수 정보 담을 상자 만들기 (일단 index 수만큼...)
    FB* fb = (FB*)malloc(sizeof(FB) * last_index);

    /*
     json_get 함수를 json_c.c 파일에서 확인하면 json_get의 정말 다양한 버전을 확인할 수 있다.
     이를 이용해 json value 를 자유롭게 가져올 수 있다...
     매크로 만들어주셨으니 잘 이용하도록 하자. 
    */

    for (int i = 0; i <= last_index; i++) {
            
        //ext array 의 i번째 인덱스 원소 가저와서 _nodetype이 FuncDef 인지 확인
        json_value json_array_element = json_get(ext, i);

        char* nodetype = json_get_string(json_array_element, "_nodetype");
        
        if (strncmp(nodetype, "FuncDef", strlen("FuncDef")) == 0) {
            
            //"decl" 정의
            json_value decl = json_get(json_array_element, "decl");

            //fb 상자 index 설정
            fb[func_count].index = func_count+1;

            //함수 이름 구하기
            char* func_name = get_FunctionName(decl);
            fb[func_count].func_name = func_name;

            //리턴 타입도 뽑아보기 ext 파일에서 리턴 타입의 경로
            //_nodetype 이 IdentifierType 이면 names를 찍는 식으로
            //이렇게 하지 않으면 type을 4번 쓸 때도 있고 3번 쓸 때도 있음(1,2번 함수)
            char* returnType = get_ReturnType(decl);
            fb[func_count].returnType = returnType;

            //파라미터 타입, 변수명 뽑아보기
            json_value args = json_get(decl, "type", "args");

            if (args.value != NULL) {
                json_value params = json_get(args, "params");
                int args_count = json_get_last_index(params);
                
                for (int j = 0; j <= args_count; j++) {
                    char* argType = get_ArgsType(params, j);
                    char* argName = get_ArgsName(params, j);

                    //fb 구조체에 집어넣기
                    strlcpy(fb[func_count].argsType[j], argType, MAX_ARG_LENGTH);
                    strlcpy(fb[func_count].argsName[j], argName, MAX_ARG_LENGTH);

                    free(argType); //해지
                }
            } else {
                //args가 NULL이면 argsType, argsName에 NULL 넣기
                strlcpy(fb[func_count].argsType[0], "NO ARGS", MAX_ARG_LENGTH);
                strlcpy(fb[func_count].argsName[0], "NO ARGS", MAX_ARG_LENGTH);
            }

            // if문 개수 구하기
            json_value body = json_get(json_array_element, "body");
            json_value block_items = json_get(body, "block_items");
            int last_block_index = json_get_last_index(block_items);
            
            for (int k = 0; k <= last_block_index; k++) {
                json_value block_item = json_get(block_items, k);
                int if_count = get_If(block_item);
                fb[func_count].if_number += if_count;
            }

        func_count++;
        }
    }

    print_FB(fb, func_count);

    // 해제...
    for (int i = 0; i < func_count; i++) {
        free(fb[i].returnType);
    }

    fclose(fp);
    free(json_buff);
    return 0;
}