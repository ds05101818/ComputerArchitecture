/* Assembler code fragment for LC-2K */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
int getLabel(char *);
void checkReg(char *);

struct instruction{
    char label[MAXLINELENGTH];
    char opcode[MAXLINELENGTH];
    char arg0[MAXLINELENGTH];
    char arg1[MAXLINELENGTH];
    char arg2[MAXLINELENGTH];
} instruction;

struct instruction instructions[55];
int countline = 0;

int main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
        arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
    int count;

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
        if(countline!=0&&strcmp(label,"")!=0){
            for(int i=0;i<countline;i++){
                if(!strcmp(instructions[countline].label,label)){
                    printf("error: duplicate label\n");
                    exit(1);
                }
            }
        }
        strcpy(instructions[countline].label, label);
        strcpy(instructions[countline].opcode, opcode);
        strcpy(instructions[countline].arg0, arg0);
        strcpy(instructions[countline].arg1, arg1);
        strcpy(instructions[countline].arg2, arg2);
        countline++;
        /* reached end of file */
    }

    /* TODO: Phase-1 label calculation */

    /* this is how to rewind the file ptr so that you start reading from the
     beginning of the file */

    rewind(inFilePtr);

    /* TODO: Phase-2 generate machine codes to outfile */
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)){
        int x = 0;
        /* after doing a readAndParse, you may want to do the following to test the opcode */
        if (!strcmp(instructions[count].opcode, "add") || !strcmp(instructions[count].opcode, "nor")){
            /* do whatever you need to do for opcode "add" */
            if (!strcmp(instructions[count].opcode, "add")){
                x += 0;
            }
            else{
                x += 1;
            }
            checkReg(instructions[count].arg0);
            x = x << 3;
            x += atoi(instructions[count].arg0);
            checkReg(instructions[count].arg1);
            x = x << 3;
            x += atoi(instructions[count].arg1);
            x = x << 13;
            checkReg(instructions[count].arg2);
            x = x << 3;
            x += atoi(instructions[count].arg2);
        }
        else if (!strcmp(instructions[count].opcode, "lw") || !strcmp(instructions[count].opcode, "sw") || !strcmp(instructions[count].opcode, "beq")){
            int offset;
            if (!strcmp(instructions[count].opcode, "lw")){
                x += 2;
            }
            else if (!strcmp(instructions[count].opcode, "sw")){
                x += 3;
            }
            else{
                x += 4;
            }
            checkReg(instructions[count].arg0);
            x = x << 3;
            x += atoi(instructions[count].arg0);
            checkReg(instructions[count].arg1);
            x = x << 3;
            x += atoi(instructions[count].arg1);
            x = x << 16;

            if (isNumber(instructions[count].arg2)){
                offset = atoi(instructions[count].arg2);
            }
            else{
                if (getLabel(instructions[count].arg2) == -1){
                    printf("error: undefined label\n");
                    exit(1);
                }
                offset = getLabel(instructions[count].arg2);
                if (!strcmp(instructions[count].opcode, "beq")){
                    offset = offset - count - 1;
                }
            }
            if (offset < -32768 || 32767 < offset){
                printf("error: offsetFields over\n");
                exit(1);
            }
            offset = 0x0000ffff & offset;
            x += offset;
        }
        else if (!strcmp(instructions[count].opcode, "jalr")){
            x += 5;
            checkReg(instructions[count].arg0);
            x = x << 3;
            x += atoi(instructions[count].arg0);
            checkReg(instructions[count].arg1);
            x = x << 3;
            x += atoi(instructions[count].arg1);
            x = x << 16;
        }
        else if (!strcmp(instructions[count].opcode, "halt") || !strcmp(instructions[count].opcode, "noop")){
            if (!strcmp(instructions[count].opcode, "halt")){
                x += 6;
            }
            else{
                x += 7;
            }
            x = x << 22;
        }
        else if (!strcmp(instructions[count].opcode, ".fill")){
            if (isNumber(instructions[count].arg0)){
                x = atoi(instructions[count].arg0);
            }
            else{
                x = getLabel(instructions[count].arg0);
                if (x == -1){
                    printf("error : undefined label\n");
                    exit(1);
                }
            }
        }
        else{
            printf("error : unknown opcode\n");
            exit(1);
        }
        fprintf(outFilePtr,"%d\n",x);
        count++;
    }

    if (inFilePtr){
        fclose(inFilePtr);
    }
    if (outFilePtr){
        fclose(outFilePtr);
    }
    return (0);
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
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%"
                "[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
           opcode, arg0, arg1, arg2);
    return (1);
}

int isNumber(char *string){
    /* return 1 if string is a number */
    int i;
    return ((sscanf(string, "%d", &i)) == 1);
}

int getLabel(char *string){
    for (int i = 0; i < countline; i++){
        if (strcmp(instructions[i].label, string) == 0){
            return i;
        }
    }
    printf("error: undefined label\n");
    exit(1);
}

void checkReg(char *reg){
    if (!isNumber(reg)){
        printf("error: no number register\n");
        exit(1);
    }
    if (atoi(reg) < 0 || 7 < atoi(reg)){
        printf("error: out of bound\n");
        exit(1);
    }
}