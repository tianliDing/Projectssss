/**
 * Shell Lab
 * CS 241 - Spring 2019
 */
#include "format.h"
#include "shell.h"
#include "vector.h"
#include "sstring.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>


typedef struct process {
    char *command;
    pid_t pid;
} process;

static process* parray[50];
static int pid[50];
static int pindex = 0;
static bool backgroundFlag = 0;

void kill_fgd(){}

int externals(char* result);

int shell(int argc, char *argv[]) {
    if(argc == 2 | argc == 4 | argc >= 6){
      print_usage();
    }
    bool h =0;
    bool f =0;

    /*----------------- which option? -----------------*/
    FILE* fpt;
    int ch;

    char* filename = argv[2];
    if((ch = getopt(argc, argv, "h:f:"))!=1){
      switch(ch){
        case 'h':
              h = 1;
              //path = get_full_path(filename);
              fpt = fopen(filename, "w");
              if(fpt == NULL){
                print_history_file_error();
                exit(1);  //???
              }
              break;
        case 'f':
              f = 1;
              fpt = fopen(argv[2], "r");
              if(fpt == NULL){
                print_script_file_error();
                exit(1);  //???
              }
              break;
        }
      }
      //vector* pidv = vector_create(NULL,NULL,NULL);
      //int value = (int)getpid();
      //vector_push_back(pidv, &value);
      pid[pindex] = (int)getpid();
      pindex ++;
      signal(SIGINT, kill_fgd);

      /*------------- start with the interaction functions ------------*/
      vector* history_record =vector_create(string_copy_constructor,string_destructor,string_default_constructor);

      char cwd[PATH_MAX];

      char* result = NULL;
      size_t n = 0;
      ssize_t bytesread = 0;

      while(1){
        // char* result = malloc(sizeof(char)*50);
        backgroundFlag = 0;
        print_prompt(getcwd(cwd, sizeof(cwd)), getpid());
  if(f){
        bytesread = getline(&result, &n, fpt);
        if(bytesread == -1)
          break;
        if(bytesread > 0 && result[strlen(result)-1] == '\n')
          result[strlen(result)-1] = '\0';
        print_command(result);
  }
  else{
        bytesread  = getline(&result, &n, stdin);
        if(bytesread == -1)
          break;
        //result = fgets(c,50, stdin);
        if(*result == EOF) { exit(0);}
        if(result[strlen(result)-1] == '\n')
          result[strlen(result)-1] = '\0';
  }

        /*----------- kill <pid> -----------*/
        char * killchar;
        if((killchar = strstr(result, "kill")) != NULL && killchar == result) {
          int ig;
          sscanf(result, "%*[^0123456789]%d\n", &ig);
          bool killsuccess = 0;

          for (int i = 0; i < pindex; i++) {
            if (pid[i] == ig) {
              kill(ig, SIGTERM);
              print_killed_process(ig, parray[i]->command);
              int status;
              waitpid(ig, &status, 0);
              killsuccess = 1;
              break;
            }
          }
          if(!killsuccess)
            print_no_process_found(ig);
          continue;
        }

      /*----------- stop <pid> -----------*/
      char * stopchar;
      if((stopchar = strstr(result, "stop")) != NULL && stopchar == result) {
        int ig2;
        sscanf(result, "%*[^0123456789]%d\n", &ig2);
        bool stopsuccess = 0;

        for (int i = 0; i < pindex; i++) {
          if (pid[i] == ig2) {
            kill(ig2, SIGTSTP);
            print_stopped_process(ig2, parray[i]->command);
            stopsuccess = 1;
            break;
          }
        }
        if(!stopsuccess)
          print_no_process_found(ig2);
        continue;
      }

      /*----------- cont <pid> -----------*/
      char * contchar;
      if((contchar = strstr(result, "cont")) != NULL && contchar == result) {
        int ig3;
        sscanf(result, "%*[^0123456789]%d\n", &ig3);
        bool contsuccess = 0;

        for (int i = 0; i < pindex; i++) {
          if (pid[i] == ig3) {
            kill(ig3, SIGCONT);
            contsuccess = 1;
            break;
          }
        }
        if(!contsuccess)
          print_no_process_found(ig3);
        continue;
      }

      //fflush(output);
      /*------------- exit --------------*/
      if(strcmp(result, "exit")==0){    //ctrl+d  //  exit
        vector_destroy(history_record);
        for (int i = 1; i < pindex; i++) {
          kill(pid[i], SIGTERM);
        }
        free(result);
        exit(0);
      }
      //
       /*------------- background ------------*/
      if(result[strlen(result)-1] == '&'){
          backgroundFlag = 1;
       }


      /*-------------- !history -------------*/
      if (strcmp(result, "!history") == 0) {
        for(size_t i = 0; i < vector_size(history_record); i++){
        print_history_line(i, *vector_at(history_record, i));
        }
        continue;
      }

      /*--------------- #<n> -----------------*/
      if (*result == '#') {
        int integ;
        sscanf(result, "%*[^0123456789]%d\n", &integ);
        if(integ < (int)vector_size(history_record)){
          print_command(*(vector_at(history_record, integ)));
          //result = *(vector_at(history_record, integ));
          strcpy(result, *(vector_at(history_record, integ)));
        }
        else{
          print_invalid_index();
          continue;  //changed
        }
      }

      /*---------------- !<prefix> -----------------*/
      if (*result == '!') {
        if (vector_size(history_record)==0) {
          print_no_history_match();
          continue;
        }
        char* ret;
        char* string;
        for(int i = vector_size(history_record)-1; i >= 0; i--){
          string = *vector_at(history_record, i);
          ret = strstr(string, result+1);
          if( ret == string){
            print_command(string);
            result = *(vector_at(history_record, i));
            break;
          }
        }
          if( ret != string){
            print_no_history_match();
            continue;
          }
      }


      vector_push_back(history_record, result);
      if(h) fprintf(fpt, "%s\n", result);     //write to file


        /*--------------- ps ----------------*/
        if (strcmp(result, "ps") == 0) {
          print_process_info_header();

          for(int i = 0; i < pindex; i++){
            char szStatStr[2048];
            process_info* pinfo = malloc(sizeof(process_info));
            char filename[100];
            sprintf(filename, "/proc/%d/stat", pid[i]);
            FILE *fp;
            if ((fp = fopen (filename, "r")) == NULL)
              continue;
            fgets (szStatStr, 2048, fp);
            sscanf(szStatStr, "%u", &(pinfo->pid));
            pinfo->start_str = malloc(sizeof(char)*100);
            if(i==0) pinfo->command = "./shell";
            else{
              pinfo->command = parray[i]->command;
            }
            time_t rawtime;
            unsigned long long starttime;
            unsigned long utime, stime, rawduration;
            struct tm *info;

            char *t = strchr (szStatStr, ')');
            sscanf (t+2, "%c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %lu %lu %*ld %*ld %*ld %*ld %ld %*ld %llu %lu %*ld %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu",
              &pinfo->state, &utime, &stime, &pinfo->nthreads, &starttime, &pinfo->vsize);
              pinfo->vsize /= 1024;
            //if it is a zombie, then kill it and wait
            if (pinfo->state == 'Z') {
              kill(pid[i], SIGTERM);
              int status2;
              waitpid(pid[i], &status2, 0);
            }

            FILE* filebtime;
            char* stringb= "/proc/stat";
            if( (filebtime = fopen(stringb, "r")) == NULL){
              exit(0);
            }
            int btime = 0;
            size_t temp = 0;
            char* bline;
            char* buf;
            while(getline(&bline, &temp, filebtime)){
              if((buf=strstr(bline, "btime")) != NULL){
                sscanf(buf, "%*[^0123456789]%d\n", &btime);
                break;
              }
            }
             //printf("%d\n", btime);

            rawtime = (starttime/sysconf(_SC_CLK_TCK)) + btime; //1548879746;
            info = localtime(&rawtime);
            char* res = asctime(info);
            res = res+11;
            *(res+5) = '\0';
            pinfo->start_str = res;

            rawduration= (utime+stime)/sysconf(_SC_CLK_TCK);
            char duration[10];
            sprintf(duration, "%.2lu:%.2lu", rawduration/60, rawduration%60);
            pinfo->time_str = duration;
            fclose(fp);
            print_process_info(pinfo);
            free(pinfo);
          }
          continue;
        }


      /*--------------- pfd ----------------*/
      char * pfdchar;
      if((pfdchar = strstr(result, "pfd")) != NULL && pfdchar == result) {
        int ig4;
        sscanf(result, "%*[^0123456789]%d\n", &ig4);
        char pfdfile[100];
        struct dirent *dp;
        sprintf(pfdfile, "/proc/%d/fd", ig4);
        DIR *direc = opendir(pfdfile);
        if (direc == NULL) {
          print_no_process_found(ig4);
          continue;
        }
        print_process_fd_info_header();

        while ( (dp=readdir(direc)) !=NULL ) {
          char *name = dp->d_name;
          if (strcmp(name, ".") && strcmp(name, "..") ) {
            //FD_NO
            size_t f = (size_t) atoi(name);
            char pos[100];                  //POS
            sprintf(pos, "/proc/%d/fdinfo/%zu", ig4, f);
            FILE *secFile = fopen(pos, "r");
            size_t c = 0;
            char *secline = NULL;
            getline(&secline, &c, secFile);
            char *t = secline;
            for (size_t i = 0; i < strlen(secline); i++) {
              if(isdigit(*t)) {
                t[strlen(t)-1] = '\0';
                break;
              }
              t++;
            }
            size_t ha = (size_t)atoi(t);
            free(secline);
            fclose(secFile);

            char path[100];
            sprintf(path, "/proc/%d/fd/%zu", ig4, f);
            char path2[100];
            ssize_t size = readlink(path, path2, 100);
            path2[size] = '\0';
            print_process_fd_info(f, ha, path2);
          }
        }
        closedir(direc);
        continue;
      }


      /*---------------- && -----------------*/
      char* and_ptr = strstr(result, " && ");
      if (and_ptr!=NULL && strlen(and_ptr) > 4) {
        char* and_ptr2 = and_ptr + 4;
        *and_ptr = '\0';
        int and_result = externals(result);    //if it is 1, then not executed.
        if(and_result == 0) externals(and_ptr2);   //it
        continue;
        }

      /*------------- || --------------*/
      char* or_ptr = strstr(result, " || ");
      if (or_ptr!=NULL && strlen(or_ptr) > 4) {
        char* or_ptr2 = or_ptr + 4;
        *or_ptr = '\0';
        int or_result = externals(result);    //if it is 1, then not executed.
        if(or_result == 1) externals(or_ptr2);   //it
        continue;
        }

      /*------------- ; --------------*/
      char* sep_ptr = strstr(result, "; ");
      if (sep_ptr!=NULL && strlen(sep_ptr) > 2) {
        char* sep_ptr2 = sep_ptr + 2;
        *sep_ptr = '\0';
        externals(result);    //if it is 1, then not executed.
        externals(sep_ptr2);   //it
        continue;
        }

        /*----------- multiple operations ---------*/
        externals(result);

    } //while loop ending
    vector_destroy(history_record);
    return 1;
}


int externals(char* result) {
      char* temp;
      /*------------- background ------------*/
      if(backgroundFlag){
        temp = strdup(result);
        if(result[strlen(result)-2] == ' ')
          result[strlen(result)-2] = '\0';
        else
          result[strlen(result)-1] = '\0';
      }

     vector* text = vector_create(string_copy_constructor,string_destructor,string_default_constructor);
     sstring* st = cstr_to_sstring(result);
     text = sstring_split(st, ' ');
     char **cmd = malloc(sizeof(char*) * 20);

     for(size_t i = 0; i < vector_size(text); i++){
       cmd[i] = malloc(sizeof(char) * 20);
       strcpy(cmd[i], *(vector_at(text, i)));
     }
     vector_destroy(text);


     /*------------- cd <path> --------------*/
     if(strcmp(cmd[0], "cd")==0){
       if(cmd[1] == NULL || chdir(cmd[1]) == -1)
         print_no_directory(cmd[1]);
       return 0;
     }


    fflush(stdout);
    pid_t child = fork();
    if (child == -1){
      print_fork_failed();
      exit(1);
    }
    if (child == 0){ //it is child
      print_command_executed(getpid());
      execvp(cmd[0], cmd);
      print_exec_failed(result);
      exit(1);
    }
    else{   //it is parent
      if (backgroundFlag == 1 && setpgid(child, child) == -1) {
          print_setpgid_failed();
          exit(1);
        }

      int status = 0;
      if (backgroundFlag){
        process* pro = malloc(sizeof(process));
        pro->command = malloc(sizeof(char)*20);
        strcpy(pro->command, temp);
        pro->pid = child;
        pid[pindex] = (int)child;  parray[pindex] = pro;   pindex++;
        waitpid(child, &status, WNOHANG);
      }
      else
        waitpid(child, &status, 0);

      if(WIFEXITED(status)&&WEXITSTATUS(status)==1)
      return 1;
    }
    return 0;
}
