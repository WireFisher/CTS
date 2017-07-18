#ifndef SCRIPT_MGT_CLASS
#define SCRIPT_MGT_CLASS

#include <vector>
#include "json.hpp"
#include "cts_exception.h"
#include "exec_mgt_class.h"

using json = nlohmann::json;
using namespace std;

//json cfg_json;
template <typename T> class Container
{
    protected:
        vector <T> contents;
        int index, num;

    public:
        Container();
        ~Container();
        virtual int go_next(); 
        virtual T get_content();
        T get_content(int);
        int get_index();
        void push_back_to_contents(T);
        void set_num(int);
};

template <typename T> class Container_strategy: public Container<T>
{
    protected:
        vector <T> attributes;

    public:
        ~Container_strategy();
        virtual T get_attribute();
        void push_back_to_attris(T);
};

template <typename T> class Container_src: public Container_strategy<T>
{
    private:
        int index_attri, num_attri;
        
        void clean_attris();
        void split_and_save_into_attris(char *);
    public:
        Container_src();
        ~Container_src();
        void set_num_attris(int);
        virtual T get_attribute();
        void search_attribute_of_content();
        int go_next();
        void print_attris();
};

class Script_mgt_class
{
    private:
        char *whole;
        char *whole_re;
        char *whole_cmp;
        char *splited;
        char *splited_re;
        char *parameter;
        //change to const
        Container_src<char *> src_grid_box;
        Container<char *> dst_grid_box;
        Container_strategy<char *> strategy_box;
        bool lock_dst_to_src;
        int output_file_id;
        char *output_file_name;
        bool is_src_splited;
        bool is_dst_splited;

        bool is_src_dst_same();
        int read_file_to_str(char*, char*, int);
        void replace_SD_with(char* ,const char*);
        int set_output_file_name();
    public:
        Script_mgt_class(char*);
        ~Script_mgt_class();
        void parse_json_from_cfg(char*);
        void generate_script_file();
        void loop();
};

#endif
