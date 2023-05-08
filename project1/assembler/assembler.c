/* Assembler code fragment for LC-2K */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAXLINELENGTH 1000

struct LABEL{
    char label[MAXLINELENGTH];
    int address;
    struct LABEL *next;
};

struct LABEL *head;

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);

int isLetter(char *str){
    return ((65 <= str[0] && str[0] <= 90) || (97 <= str[0] && str[0] <= 122));
}

int isSymbol(char *str){
    for (int i = 0; i < strlen(str); i++){
        if (48 < str[i] || (58 <= str[i] && str[i] <= 64) || (91 <= str[i] && str[i] <= 96) || 123 <= str[i]){
            return 0;
        }
    }
    return 1;
}

void addLabel(char *label, int address){
    struct LABEL *now;
    struct LABEL *node = (struct LABEL *)malloc(sizeof(struct LABEL));
    strcpy(node->label, label);
    node->address = address;
    node->next = NULL;
    now = head;

    if (now->next == NULL){
        now->next = node;
    }
    else{
        while (now->next != NULL){
            now = now->next;
            if (strcmp(now->label, label) == 0){
                printf("error: duplicate label (%s)\n", now->label);
                exit(1);
            }
        }
        now->next = node;
    }
}

int findLabel(char *label){
    struct LABEL *now;
    now = head;

    if (now->next == NULL){
        printf("error: label list empty\n");
        exit(1);
    }
    else{
        now = now->next;
        while (1){
            if (!strcmp(now->label, label)){
                return now->address;
            }
            now = now->next;
            if (now == NULL){
                break;
            }
        }
    }
    printf("No such label(%s) in list\n", label);
    exit(1);
}

int check(char *str){
    if (isNumber(str)){
        return atoi(str);
    }
    else{
        return findLabel(str);
    }
}

int bitSize(int num, int size){
    int n = 1;
    for (int i = 0; i < size; i++){
        n *= 2;
    }
    if (num < n){
        return 1;
    }
    else{
        return 0;
    }
}

int makeRtype(int opcode, char *arg0, char *arg1, char *arg2){
    int regA = check(arg0);
    int regB = check(arg1);
    int dest = check(arg2);
    int unused = 0;

    if (bitSize(regA,3) && bitSize(regB,3) && bitSize(dest,3)){
        return ((unused << 31) | (opcode << 22) | (regA << 19) | (regB << 16) | dest);
    }
    else{
        printf("error: too many bits in R type\n");
        exit(1);
    }
}

int makeItype(int pc, int opcode, char *arg0, char *arg1, char *arg2){
    int regA = check(arg0);
    int regB = check(arg1);
    int address = check(arg2);
    int unused = 0;
    int bitmask = 0x0000FFFF;

    if (bitSize(regA,3) && bitSize(regB,3) && bitSize(address,16)){
        if (opcode == 4 && !isNumber(arg2)){
            bitmask = bitmask & (address - (pc + 1));
            return ((unused << 31) | (opcode << 22) | (regA << 19) | (regB << 16) | bitmask);
        }
        else{
            return ((unused << 31) | (opcode << 22) | (regA << 19) | (regB << 16) | address);
        }
    }
    else{
        printf("error: too many bits in I type\n");
        exit(1);
    }
}

int makeJtype(int opcode, char *arg0, char *arg1){
    int regA = check(arg0);
    int regB = check(arg1);
    int unused = 0;
    if (bitSize(regA,3) && bitSize(regB,3)){
        return ((unused << 31) | (opcode << 22) | (regA << 19) | (regB << 16));
    }
    else{
        printf("error: too many bits in J type\n");
        exit(1);
    }
}

int makeOtype(int opcode){
    int unused = 0;
    return ((unused << 31) | (opcode << 22));
}

int main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
        arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
    int pc = -1, len;

    head = (struct LABEL *)malloc(sizeof(struct LABEL));
    head->next = NULL;

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
               argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];
    inFilePtr = fopen(inFileString, "r");

    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }

    outFilePtr = fopen(outFileString, "w");

    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    /* here is an example for how to use readAndParse to read a line from
        inFilePtr */
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)){
        /* reached end of file */
        pc++;
        len = strlen(label);
        printf("%s", label);
        if (len != 0){
            if (len > 6){
                printf("error: label(%s) is over 6 characters\n", label);
                fclose(inFilePtr);
                fclose(outFilePtr);
                exit(1);
            }
            else if (!isLetter(label)){
                printf("error: label(%s) has to start with a letter\n", label);
                fclose(inFilePtr);
                fclose(outFilePtr);
                exit(1);
            }
            else if (isSymbol(label)){
                printf("error: label(%s) has to consist of letters and numbers\n", label);
                fclose(inFilePtr);
                fclose(outFilePtr);
                exit(1);
            }
            addLabel(label, pc);
        }
    }

    /* this is how to rewind the file ptr so that you start reading from the
        beginning of the file */
    rewind(inFilePtr);

    /* after doing a readAndParse, you may want to do the following to test the
        opcode */
    pc = -1;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)){
        pc++;
        if (!strcmp(opcode, "add")){
            fprintf(outFilePtr, "%i\n", makeRtype(0, arg0, arg1, arg2));
        }
        else if (!strcmp(opcode, "nor")){
            fprintf(outFilePtr, "%i\n", makeRtype(1, arg0, arg1, arg2));
        }
        else if (!strcmp(opcode, "lw")){
            fprintf(outFilePtr, "%i\n", makeItype(pc, 2, arg0, arg1, arg2));
        }
        else if (!strcmp(opcode, "sw")){
            fprintf(outFilePtr, "%i\n", makeItype(pc, 3, arg0, arg1, arg2));
        }
        else if (!strcmp(opcode, "beq")){
            fprintf(outFilePtr, "%i\n", makeItype(pc, 4, arg0, arg1, arg2));
        }
        else if (!strcmp(opcode, "jalr")){
            fprintf(outFilePtr, "%i\n", makeJtype(5, arg0, arg1));
        }
        else if (!strcmp(opcode, "halt")){
            fprintf(outFilePtr, "%i\n", makeOtype(6));
        }
        else if (!strcmp(opcode, "noop")){
            fprintf(outFilePtr, "%i\n", makeOtype(7));
        }
        else if (!strcmp(opcode, ".fill")){
            if (isNumber(arg0)){
                fprintf(outFilePtr, "%i\n", atoi(arg0));
            }
            else{
                fprintf(outFilePtr, "%i\n", findLabel(arg0));
            }
        }
        else{
            printf("error: (%s) does not match any opcode\n", opcode);
            fclose(inFilePtr);
            fclose(outFilePtr);
            exit(1);
        }
    }
    exit(0);
} 

/*
   * Read and parse a line of the assembly-language file.  Fields are returned
   * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
   * allocated to them).
   *
   * Return values:
   *     0 if reached end of file
   *     1 if all went well
   *
   * exit(1) if line is too long.
   */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
                 char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;
    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';
    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL)
    {
        /* reached end of file */
        return (0);
    }
    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL)
    {
        /* line too long */
        printf("error: line too long\n");
        exit(1);
    }
    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label))
    {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }
    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
           opcode, arg0, arg1, arg2);
    return (1);
}

int isNumber(char *string){ 
    /* return 1 if string is a number */
    int i;
    return ((sscanf(string, "%d", &i)) == 1);
}