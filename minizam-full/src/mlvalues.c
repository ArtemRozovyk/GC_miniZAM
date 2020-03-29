#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mlvalues.h"
#include "alloc.h"
#include "instruct.h"
#include "primitives.h"
#include "domain_state.h"
#include "config.h"

void add_new_page(caml_domain_state *c_s) {
    mlvalue *page = malloc(sizeof(mlvalue) * (Page_size / sizeof(mlvalue) + 1));
    page[0] = Make_header(Page_size / sizeof(mlvalue), RED, PAGE_T);
    long l = page[0];

    Caml_state->page_list = pushHead(Ptr_val(page), Caml_state->page_list);
    c_s->free_list = pushHead(Ptr_val(page + 1), c_s->free_list);
    c_s->free_list_sz += 1;
}

//returns header pointer of a block
mlvalue *find_first_fit(caml_domain_state *pState, size_t size);

mlvalue make_empty_block(tag_t tag) {
    if (Caml_state->free_list_sz == 0) {
        //alocate page and add entirely to freelist
        add_new_page(Caml_state);
    }
    mlvalue *free_block = find_first_fit(Caml_state, 1);
    if (!free_block) {
        //block of sufficient size wasn't found,new page is needed.
        add_new_page(Caml_state);
        free_block = find_first_fit(Caml_state, 1);
    }
    //block has been found
    free_block[0] = Make_header(1, WHITE, tag);
    return Val_ptr(free_block + 1);
}


mlvalue make_block(size_t size, tag_t tag) {
    //if an object has half of pages size we consider it big
    if (size * sizeof(mlvalue) > Page_size / 2) {
        mlvalue *block = caml_alloc((size + 1) * sizeof(mlvalue));
        block[0] = Make_header(size, WHITE, tag);
        Caml_state->big_list = pushHead(block + 1, Caml_state->big_list);
        Caml_state->big_list_size++;
        return Val_ptr(block + 1);
    }
    if (Caml_state->free_list_sz == 0) {
        //alocate page and add entirely to freelist
        add_new_page(Caml_state);
    }
    mlvalue *free_block = find_first_fit(Caml_state, size);
    if (!free_block) {
        //block of sufficient size wasn't found,new page is needed.
        add_new_page(Caml_state);
        free_block = find_first_fit(Caml_state, size);
    }
    //block has been found
    free_block[0] = Make_header(size, WHITE, tag);
    return Val_ptr(free_block + 1);
}

mlvalue *find_first_fit(caml_domain_state *pState, size_t size) {
    ml_list curr = pState->free_list;
    while (curr && curr->val) {
        header_t header = Hd_val(curr->val);
        size_t bl_sz = Size(curr->val);
        mlvalue *fit = curr->val - 1;

        if ((bl_sz >= size && Tag(curr->val) != PAGE_T) || bl_sz > size) {

            if (bl_sz != size) {
                Field(curr->val, size) =
                        Make_header(bl_sz - (size + 1), RED, Tag(curr->val));
            }

            //
            curr->val = Ptr_val(curr->val + size + (size==bl_sz ? 0 : 1));


            return fit;
        }
        curr = curr->next;
    }
    return NULL;
}


mlvalue make_closure(uint64_t addr, mlvalue env) {

    if (Caml_state->free_list_sz == 0) {
        //alocate page and add entirely to freelist
        add_new_page(Caml_state);
    }
    mlvalue *free_block = find_first_fit(Caml_state, 2);
    if (!free_block) {
        //block of sufficient size wasn't found,new page is needed.
        add_new_page(Caml_state);
        free_block = find_first_fit(Caml_state, 2);
    }
    //block has been found
    free_block[0] = Make_header(2, WHITE, CLOSURE_T);
    free_block[1] = Val_long(addr);
    free_block[2] = env;
    return Val_ptr(free_block + 1);
}


void print_val(mlvalue val) {
    char *val_str = val_to_str(val);
    printf("%s", val_str);
    free(val_str);
}


char *long_to_str(mlvalue val) {
    uint64_t n = Long_val(val);
    int length = snprintf(NULL, 0, "%ld", n) + 1;
    char *buffer = malloc(length * sizeof(*buffer));
    snprintf(buffer, length, "%ld", n);
    return buffer;
}

char *block_content_to_str(mlvalue block) {
    if (Size(block) == 0) return strdup("");
    char **content = malloc(Size(block) * sizeof(*content));
    int lengths[Size(block)];
    int length = 1;
    for (unsigned int i = 0; i < Size(block); i++) {
        content[i] = val_to_str(Field(block, i));
        lengths[i] = strlen(content[i]);
        length += lengths[i] + 1;
    }
    char *buffer = malloc(length * sizeof(*buffer));
    char *buff_ptr = buffer;
    for (unsigned int i = 0; i < Size(block); i++) {
        memcpy(buff_ptr, content[i], lengths[i]);
        free(content[i]);
        buff_ptr += lengths[i];
        *buff_ptr++ = ',';
    }
    buff_ptr--; // To allow the last ',' to be removed
    *buff_ptr = '\0';
    return buffer;
}

#define DELIMITED_BLOCK_TO_STR(name, delim_start, delim_end)      \
  char* name##_to_str(mlvalue block) {                          \
    char* content_str = block_content_to_str(block);            \
    int length = strlen(content_str);                           \
    char* ret_str = malloc(length + 3);                         \
    ret_str[0] = delim_start;                                   \
    memcpy(ret_str+1, content_str, length);                     \
    ret_str[length+1] = delim_end;                                \
    ret_str[length+2] = '\0';                                     \
    free(content_str);                                          \
    return ret_str;                                             \
  }                                                             \


DELIMITED_BLOCK_TO_STR(block, '[', ']')

DELIMITED_BLOCK_TO_STR(env, '<', '>')


char *closure_to_str(mlvalue closure) {
    mlvalue env = Env_closure(closure);
    int64_t addr = Addr_closure(closure);
    char *env_str = env_to_str(env);
    int length = snprintf(NULL, 0, "{ %lu, %s }", addr, env_str) + 1;
    char *buffer = malloc(length * sizeof(*buffer));
    snprintf(buffer, length, "{ %lu, %s }", addr, env_str);
    free(env_str);
    return buffer;
}


char *val_to_str(mlvalue val) {
    if (Is_long(val)) {
        return long_to_str(val);
    } else {
        switch (Tag(val)) {
            case ENV_T:
                return env_to_str(val);

            case CLOSURE_T:
                return closure_to_str(val);

            case BLOCK_T:
                return block_to_str(val);

            default:
                fprintf(stderr, "Unknown value type: %lu\n", Tag(val));
                exit(EXIT_FAILURE);
        }
    }
}

/* Returns the value of |pc| after moving to the next instruction */
int print_instr(code_t *prog, int pc) {
    switch (prog[pc++]) {
        case CONST:
            printf("CONST %lu\n", prog[pc++]);
            break;

        case PRIM:
            printf("PRIM ");
            switch (prog[pc++]) {
                case ADD:
                    printf("+\n");
                    break;
                case SUB:
                    printf("-\n");
                    break;
                case DIV:
                    printf("/\n");
                    break;
                case MUL:
                    printf("*\n");
                    break;
                case OR:
                    printf("or\n");
                    break;
                case AND:
                    printf("and\n");
                    break;
                case NOT:
                    printf("not\n");
                    break;
                case NE:
                    printf("ne\n");
                    break;
                case EQ:
                    printf("eq\n");
                    break;
                case LT:
                    printf("lt\n");
                    break;
                case LE:
                    printf("le\n");
                    break;
                case GT:
                    printf("gt\n");
                    break;
                case GE:
                    printf("ge\n");
                    break;
                case PRINT:
                    printf("print\n");
                    break;
            }
            break;

        case BRANCH:
            printf("BRANCH %lu\n", prog[pc++]);
            break;

        case BRANCHIFNOT:
            printf("BRANCHIFNOT %lu\n", prog[pc++]);
            break;

        case PUSH:
            printf("PUSH\n");
            break;

        case POP:
            printf("POP\n");
            break;

        case ACC:
            printf("ACC %lu\n", prog[pc++]);
            break;

        case ENVACC:
            printf("ENVACC %lu\n", prog[pc++]);
            break;

        case CLOSURE:
            printf("CLOSURE %lu, %lu\n", prog[pc], prog[pc + 1]);
            pc += 2;
            break;

        case CLOSUREREC:
            printf("CLOSUREREC %lu, %lu\n", prog[pc], prog[pc + 1]);
            pc += 2;
            break;

        case OFFSETCLOSURE:
            printf("OFFSETCLOSURE\n");
            break;

        case APPLY:
            printf("APPLY %lu\n", prog[pc++]);
            break;

        case APPTERM:
            printf("APPTERM %lu, %lu\n", prog[pc], prog[pc + 1]);
            pc += 2;
            break;

        case RETURN:
            printf("RETURN %lu\n", prog[pc++]);
            break;

        case MAKEBLOCK:
            printf("MAKEBLOCK %lu\n", prog[pc++]);
            break;

        case GETFIELD:
            printf("GETFIELD %lu\n", prog[pc++]);
            break;

        case VECTLENGTH:
            printf("VECTLENGTH\n");
            break;

        case GETVECTITEM:
            printf("GETVECTITEM\n");
            break;

        case SETFIELD:
            printf("SETFIELD %lu\n", prog[pc++]);
            break;

        case SETVECTITEM:
            printf("SETVECTITEM\n");
            break;

        case ASSIGN:
            printf("ASSIGN %lu\n", prog[pc++]);
            break;

        case PUSHTRAP:
            printf("PUSHTRAP %lu\n", prog[pc++]);
            break;

        case POPTRAP:
            printf("POPTRAP\n");
            break;

        case RAISE:
            printf("RAISE\n");
            break;

        case STOP:
            printf("STOP\n");
            return -1;
    }

    return pc;
}

void print_prog(code_t *prog) {
    int i = 0;
    while (i != -1) {
        i = print_instr(prog, i);
    }
}
