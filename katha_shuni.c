#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<termios.h>

char name[10];
char custom_commands[] [10] = {"cmd_prev"};
char ***cmd_prev_list;
int n_cmd_prev = -1;
int n_cust_cmd = 1;
int prev_cmd_mode = 0;

void welcomeMessage();
void introduce_yourself();
void special_intructions();
void others();
void shell();
char get_char_to_work_instantly();
char** get_instructions_and_tokenize();
int execute_instructions(char **instruct);
void previous_cmd_mode();
void reset_system();

#define NRM  "\x1B[0m"
#define YEL  "\x1B[33m"
#define CYN  "\x1B[36m"
#define RED  "\x1B[31m"
static struct termios current, old;

int main()
{
    welcomeMessage();
    introduce_yourself();
    special_intructions();
    others();

    shell();
}


void shell()
{
    char **instruct;

    do
    {
        sleep(1);
        if(!prev_cmd_mode)
        {
            printf("\r%sYour Command: ",CYN);
            printf("%s",NRM);
            instruct = get_instructions_and_tokenize();
            cmd_prev_list [++n_cmd_prev % 10] = instruct;
            execute_instructions(instruct);
        }
        else
        {
            previous_cmd_mode();
        }

    }
    while(1);
}

void reset_system()
{
    current.c_lflag |= ECHO;
    current.c_lflag |= ICANON;
    tcsetattr(0, TCSANOW, &current);
}

char get_char_to_work_instantly()
{
    tcgetattr(0, &old);
    current = old;
    current.c_lflag &= ~ICANON;
    current.c_lflag &= ~ECHO;

    tcsetattr(0, TCSANOW, &current);
    char c = getchar();

    return c;
}

void instruction_printer(char **cmd)
{
    int i=0;
    while(cmd [i] != NULL)
    {
        printf("%s ", cmd[i]);
        i += 1;
    }
    printf("\n");
}

void previous_cmd_mode()
{
    int c, first_char, second_char, n_cmd_prev_local;
    n_cmd_prev_local = n_cmd_prev;

    printf("%s%s, %sto get the previous and next command, you have to"
           " press up and down arrow button respectively%s\n\n", YEL, name, YEL, NRM);
    printf("%sYour Choice%s\n", CYN, NRM);

    while(prev_cmd_mode)
    {
        c = get_char_to_work_instantly();

        if( c == 27 || c== 'e')
        {
            first_char = c;
        }
        else if( (first_char == 27 && c == 91 ) ||
                 (first_char == 'e' && c == 'x' ) )
        {
            second_char = c;
        }
        else if( (first_char == 27 && second_char == 91) ||
                 (first_char == 'e' && second_char == 'x' ))
        {
            if(c == 65)
            {
                if(n_cmd_prev_local < 1)
                {
                    printf("%sSorry %s%s, %sthere is no more previous commands" 							" which I have stored.%s\n", RED, YEL, name, RED, NRM);
                    n_cmd_prev_local = -1;
                }
                else
                {
                    instruction_printer(cmd_prev_list[--n_cmd_prev_local]);
                }
            }
            else if(c == 66)
            {
                if(n_cmd_prev_local > (n_cmd_prev-1))
                {
                    printf("%sSorry %s%s, %sthere is no more commands which" 								" you have entered.%s\n", RED, YEL, name, RED, NRM);
                    n_cmd_prev_local = n_cmd_prev + 1;
                }
                else
                {
                    instruction_printer(cmd_prev_list[++n_cmd_prev_local]);
                }
            }
            else if(c == 'p')
            {
                prev_cmd_mode = 0;
                printf("\n\n%sExited from previous commands area%s\n\n", RED, NRM);
                reset_system();
            }
            else if(c == 10)
            {
                execute_instructions (cmd_prev_list[n_cmd_prev_local]);
            }
            else
            {
                printf("\n%sSorry %s%s, %swrong keyword%s", RED, YEL, name, RED, NRM);
            }
        }
        else
        {
            printf("\n%sSorry %s%s, %swrong keyword.\n%s", RED, YEL, name, RED, NRM);
        }
    }
}

int check_custom_command(char *cmd)
{
    int i=0;
    int FLAG = 0;
    for( ; i < n_cust_cmd && !FLAG; )
    {
        if (!strcmp(cmd, custom_commands[i]) )
        {
            FLAG = 1;
        }

        i += 1;
    }

    return FLAG;
}

int execute_instructions(char **instruct)
{
    int err_no;

    int custom_checker = check_custom_command(instruct [0]);

    if(!custom_checker)
    {
        pid_t pid = fork();

        if(!pid)
        {
            int status = execvp(instruct[0], instruct);
            if (status == -1)
            {
                printf("%sSorry %s%s, %s%s%s\n", RED, YEL, name, RED, strerror(errno), NRM);
            }
        }
    }
    else
    {
        printf("\n%sPrevious history mode%s\n\n", RED, NRM);
        prev_cmd_mode = 1;
    }
    return 1;
}

char** get_instructions_and_tokenize()
{
    char c;
    char ** tokens = malloc(5 * sizeof(char * ));
    int i = 0, n_row = 0, n_column = 0;

    while (i <= 5)
    {
        tokens[i] = malloc(10 * sizeof(char * ));
        i++;
    }

    do
    {
        c = getchar();

        if (c != ' ' && c != '\n')
        {
            tokens[n_row][n_column] = c;
            n_column += 1;
        }
        else if (c == ' ')
        {
            n_row++;
            n_column = 0;
        }

    }
    while (c != '\n');

    tokens[n_row + 1] = NULL;
    return tokens;
}

void others()
{
    int i, j, k;
    cmd_prev_list = malloc(10 * 10* 10* sizeof(char));
}

void special_intructions()
{
    printf("%s%s, %sHere are some special instructions for you.\n"
           "%s1.%s\nYou can store at most ten previously entered commands.\n"
           "Write command cmd_prev to execute previous commands.\n\n"
           "Once you execute cmd_prev, you must have to exit from it"
           "to execute new commands.\n"
           "Write command, ext to exit from previous commands"
           "area.\n\n",YEL, name, CYN, NRM, YEL);
    printf("%sLet's start\n\n", NRM);
}

void introduce_yourself()
{
    printf("%sPlease, let me know your nickname: ", CYN);
    printf("%s",NRM);
    gets(name);

    system("clear");
    printf("Hello %s%s,", YEL, name);
    printf("\n\t%sWelcome to %sকথা  শুনি\n\n", NRM, YEL);
    printf("%sI will not just listen and execute your commands.\n"
           "Rather, I will talk with you when you will make mistake.\n\n", NRM);
}

void welcomeMessage()
{
    printf("\n\tWelcome to %sকথা   শুনি\n\n", YEL);
}
