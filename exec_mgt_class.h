#ifndef EXEC_MGT_CLASS
#define EXEC_MGT_CLASS

#include <vector>

extern int process_number;
extern int num_script;
extern std::vector <char*> filename_list;

int exec_all_scripts(int);
int do_system(int, int);
#endif
