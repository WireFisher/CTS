#include <cstdio>
#include "script_mgt_class.h"
#include "exec_mgt_class.h"

const char str_usage[] = "Usage: cts config_filename\n";
int main(int argc, char * argv[])
{
    if( argc == 2)
    {
        //Config_mgt_class config_manager(argv[1]);
        Script_mgt_class script_gen(argv[1]);
        script_gen.loop();
        if(exec_all_scripts(0))
            if(exec_all_scripts(1))
                exec_all_scripts(2);
        return 0;
    }
    else
    {
        printf(str_usage);
        return 1;
    }
}
