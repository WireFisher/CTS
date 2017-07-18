#include "exec_mgt_class.h"
#include "cstdio"
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include "cts_exception.h"
using namespace std;

int do_system(int file_index, int flag)
{
    char cmd[64];
    if(filename_list[file_index] == NULL)
    {
        CTS_exception ecpt("No script found for one of the CoR.\n");
        throw ecpt;
    }
    if(flag == 0)
        sprintf(cmd, "./CoR ./script/%s.cor > ./log/%s.log 2>&1", filename_list[file_index], filename_list[file_index]);
    else if(flag == 1)
        sprintf(cmd, "./CoR ./script/%s_RE.cor > ./log/%s_RE.log 2>&1", filename_list[file_index], filename_list[file_index]);
    else if(flag == 2)
        sprintf(cmd, "./CoR ./script/%s_ERROR.cor > ./log/%s_ERROR.log 2>&1", filename_list[file_index], filename_list[file_index]);
    else
        sprintf(cmd, "", filename_list[file_index], filename_list[file_index]);
    printf("Executing CMD: %s\n", cmd);
    return system(cmd);
}

int exec_all_scripts(int flag)
{
    int *pid;
    int current_todo, current_done;
    int num_file;

    num_file = filename_list.size();
    if(num_file != num_script)
    {
        CTS_exception ecpt("Wrong amount of script file\n");
        throw ecpt;
    }
    pid = new int[process_number];
    current_todo = 0;
    current_done = 0;
    
    for(int i=0; i < process_number; i++)
    {
        if(current_todo >= num_script)
            break;

        pid[i] = fork();
        if(pid[i] < 0)
        {
            CTS_exception ecpt("Fork Error.\n");
            throw ecpt;
        }
        else if(pid[i] == 0)
        {
            do_system(current_todo, flag);
            return 0;
        }
        else
            current_todo++;
    }
    
    while(current_done < num_script)
    {

        pid[0] = wait(NULL);
        current_done++;
        printf("[Main] %d done.\n", pid[0]);
        if(current_todo >= num_script)
            continue;

        pid[0] = fork();
        if(pid[0] < 0)
        {
            CTS_exception ecpt("Fork Error.\n");
            throw ecpt;
        }
        else if(pid[0] == 0)
        {
            do_system(current_todo, flag);
            return 0;
        }
        else
            current_todo++;
    }
    delete [] pid;
    return 1;
}
