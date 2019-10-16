#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

#define BUFFSIZE 1024

pid_t pid_list[100];
int index_pid = -1;



void signal_handler(int sig_num);

void signal_handler(int sig_num)
{
        printf("Exiting... \n");
        signal(SIGINT, signal_handler);
        fflush(stdout);
}

char **parse_pipe(char *input_for_pipe,char *temp_pointer,char *temp_pointer1,int count)
{
        char** pipe_tokens=malloc(BUFFSIZE*sizeof(char*));
        temp_pointer1=strtok_r(input_for_pipe,"|", &temp_pointer);
        while(temp_pointer1!=NULL) {
                pipe_tokens[count++]=temp_pointer1;
                temp_pointer1=strtok_r(NULL,"|", &temp_pointer);
        }
        pipe_tokens[count]=NULL;
        return pipe_tokens;

}

char **parse(char *input_for_parse, int count1, char *temp_pointer2)
{
        char** parse_tokens=malloc(BUFFSIZE*sizeof(char*));
        temp_pointer2=strtok(input_for_parse," \t\n");
        while(temp_pointer2!=NULL) {
                parse_tokens[count1++]=temp_pointer2;
                temp_pointer2=strtok(NULL," \t\n");
        }
        parse_tokens[count1]=NULL;
        return parse_tokens;
}

int exec_pipe(char *input)
{
        char** seperate_fd=parse_pipe(input,NULL,NULL,0);
        int i=0;
        int temp;
        while (seperate_fd[i]!= NULL) {

                int fd[2];
                pipe(fd);
                int pid;
                char** cmds=parse(seperate_fd[i],0,NULL);
                pid=fork();
                if (pid< 0) {
                        fprintf(stderr, "Error with Fork.\n" );
                }
                else if (pid==0) {

                        if (i==0) {
                                dup2(0,STDIN_FILENO);
                        }
                        else{
                                dup2(temp,STDIN_FILENO);
                        }
                        close(fd[0]);
                        //printf("HERE X\n");
                        if( seperate_fd[i+1] != NULL) {
                                dup2(fd[1],STDOUT_FILENO);
                        }
                        close(fd[0]);

                        execvp(cmds[0], cmds);
                        printf("Error in PIPE. \n");
                        exit(EXIT_FAILURE);
                }
                else{
                        wait(NULL);
                        temp=fd[0];
                        close(fd[1]);
                }
                i++;
                free(cmds);
        }
        return 0;
}





void minish_loop()
{

        int bg_flag = 0;
        int sh_exit = 0;
        char buffer[100];
        signal(SIGINT, signal_handler);
        //  memset(input,0,sizeof(input));
        while(sh_exit != 1)
        {
                int ip_flag = 0;
                int op_flag = 0;
                int pipe_flag = 0;
                int pipe_count = 0;
                char input[200];
                char *arguments[10];
                char line1[200];
                char *input_file = NULL;
                char *output_file = NULL;
                int i = 0;

                printf("minish->");
                fgets(input,100,stdin);
                if (strcmp(input,"\n") == 0)
                {
                        continue;
                }
                else
                {
                        input[strcspn(input,"\n")]='\0';
                }
                strcpy(buffer, input);
                strcpy(line1,input);
                char *token = strtok(buffer," ");
                while(token)
                {
                        if(strcmp(token,"&")==0)
                        {
                                bg_flag++;
                                break;
                        }

                        arguments[i]=token;

                        i++;
                        token = strtok(NULL, " ");
                }
                arguments[i]=NULL;

                int j = 0;
                while(arguments[j]!= NULL)
                {
                        if(strcmp(arguments[j],">")==0)
                        {

                                op_flag = 1;

                                output_file = arguments[j + 1];
                                break;

                        }
                        else if(strcmp(arguments[j],"<")==0)
                        {
                                ip_flag = 1;
                                input_file = arguments[j + 1];
                                break;
                        }
                        else if (strcmp(arguments[j],"|")==0)
                        {
                                pipe_flag++;
                                pipe_count++;
                        }
                        j++;
                }

                int k = 0;
                int count = 0;
                while (arguments[k] != NULL) {
                        count++;
                        k++;
                }

                if((strcmp(input,"exit") == 0)||(strcmp(input,"quit") == 0)|| (strcmp(input,"q") == 0))
                {
                        for(int m = 0; m < 100; m++)
                        {
                                if(kill(pid_list[m],9)==-1)
                                {
                                        printf("ERROR IN KILL\n");
                                }
                        }

                        sh_exit = 1;
                }
                else if (strcmp(arguments[0],"cd") == 0)
                {
                        if (chdir(arguments[1]) != 0) {
                                fprintf(stderr, "Error: cd  to path %s failed\n", arguments[1]);
                        }
                }
                else if (strcmp(arguments[0],"pwd") == 0)
                {
                        char cwd[1024];
                        if (getcwd(cwd, sizeof(cwd)) != NULL) {
                                printf("Current Directory: %s\n", cwd);
                        } else {
                                perror("Error: pwd failed");
                        }
                }
                else if ((strcmp(arguments[0],"echo") == 0) && ip_flag == 1)
                {
                        perror("echo dosen't read std input \n");
                        break;
                }




                else if (arguments[0] != NULL)
                {
                        pid_t pid;
                        int process_status;
                        pid = fork();

                        if(pid<0)
                        {
                                perror("fork failed!!\n");
                                exit(-1);
                        }
                        else if(pid == 0)
                        {      if (ip_flag == 1 && input_file != NULL)
                               {
                                       int fd0;
                                       if ((fd0 = open(input_file, O_RDONLY, 0)) < 0) {
                                               perror("Couldn't open input file");
                                               exit(0);
                                       }
                                       dup2(fd0, 0);
                                       close(fd0);
                               }
                               if (op_flag == 1 && output_file != NULL)
                               {
                                       int fd1;
                                       if ((fd1 = creat(output_file,0644)) < 0) {
                                               perror("Couldn't open the output file");
                                               exit(0);
                                       }

                                       dup2(fd1, 1);
                                       close(fd1);
                               }

                               if(ip_flag == 1 || op_flag ==1)
                               {
                                  arguments[count-1] = NULL;
                                  arguments[count - 2] = NULL;
                               }
                               if(pipe_flag > 0)
                               {
                                 exec_pipe(line1);
                               }
                               else{
                                       execvp(arguments[0], arguments);
                                       printf("%s:command not found\n",arguments[0]);
                                       exit(-1);
                               }}

                        if (bg_flag>0) {
                                index_pid++;
                                pid_list[index_pid] = pid;
                                printf("Process %d in background mode \n",(int)getpid() );
                                bg_flag = 0;
                        } else {
                                waitpid(pid, &process_status, 0);
                        }

                }



        }


}

int main(int argc, char const *argv[]) {



        minish_loop();

        return 0;
}
