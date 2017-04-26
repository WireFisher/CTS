#include <iostream>
#include "script_mgt_class.h"
#include <cstdio>
#include <fstream>
#include <cstring>
#include <cassert>

#define MAX_STRING_LONG_LENGTH 64
#define MAX_STRING_LENGTH 32
#define MAX_STRING_SHORT_LENGTH 16

#define MAX_FILE_SIZE 4096

#define PATH_TEMPLATE "./template/"

#define SCRIPT_TEMPLATE_VAR "src_var= read_field(src_grid_for_var, src_grid_file, \"%s\")\ndst_var= alloc_field(dst_grid_for_var)\n"
#define SCRIPT_TEMPLATE_OPT "remap_oprt = new_operator(\"%s\", src_grid_for_remap, dst_grid_for_remap)\n"
#define SCRIPT_TEMPLATE_SET "set_parameter(remap_oprt,\"%s\", \"%s\")\n"
#define SCRIPT_TEMPLATE_WRITE "remap_strategy = combine_operators(remap_oprt)\nremap(remap_strategy, src_var, dst_var)\noutput_data_file = add_nc_file(\"./output/%s.nc\",\"w\")\nwrite_field(output_data_file, dst_var)\n"

#define SCRIPT_TEMPLATE_VAR_RE "var_nc_file = add_nc_file(\"./output/%s.nc\",\"r\")\nsrc_var = alloc_field(src_grid_for_var)\ndst_var = read_field(dst_grid_for_var, var_nc_file, \"%s\")\n"
#define SCRIPT_TEMPLATE_OPT_RE "remap_oprt = new_operator(\"%s\", dst_grid_for_remap, src_grid_for_remap)"
#define SCRIPT_TEMPLATE_SET_RE
#define SCRIPT_TEMPLATE_WRITE_RE "remap_strategy = combine_operators(remap_oprt)\nremap(remap_strategy, dst_var, src_var)\noutput_data_file = add_nc_file(\"./output/%s_RE.nc\",\"w\")\nwrite_field(output_data_file, src_var)\n"

json cfg_json;
int process_number;
int num_script;
vector<char*> filename_list;

void strip(char * str, char chr)
{
    int length, j = 0;
    length = strlen(str);
    for(int i=0; i < length; i++)
    {
        if( chr == str[i])
            continue;
        str[j++] = str[i];
    }
    str[j] = '\0';
}

int no_extension(char *dest, char *src)
{
    char *p;
    p = strstr(src, ".cor");
    if(p == NULL)
        strncpy(dest, src, MAX_STRING_LONG_LENGTH);
    else
        strncpy(dest, src, p - src);
    dest[p - src] = '\0';
    return strlen(dest);
}

template <typename T> int Container<T>::go_next()
{
    this->index = (this->index + 1) % this->num;
    return this->index;
}

template <typename T> Container<T>::Container()
{
    this->index = 0;
    this->num = 0;
}

template <typename T> Container<T>::~Container()
{/*
    for(int i=0; i < this->num; i++)
        delete[] this->contents[i];
        */
    this->contents.clear();
}

template <typename T> T Container<T>::get_content()
{
    return this->contents[this->index];
}

template <typename T> T Container<T>::get_content(int ind)
{
    return this->contents[ind];
}

template <typename T> int Container<T>::get_index()
{
    return this->index;
}

template <typename T> void Container<T>::push_back_to_contents(T content)
{
    this->contents.push_back(content);
}



template <typename T> void Container<T>::set_num(int number)
{
    this->num = number;
}

template <typename T> Container_strategy<T>::~Container_strategy()
{/*
    for(int i=0; i < this->num; i++)
    {
        delete[] this->contents[i];
        delete[] this->attributes[i];
    }
    */
    this->contents.clear();
    this->attributes.clear();
}

template <typename T> void Container_strategy<T>::push_back_to_attris(T attri)
{
    this->attributes.push_back(attri);
}

template <typename T> T Container_strategy<T>::get_attribute()
{
    return this->attributes[this->index];
}

template <typename T> T Container_src<T>::get_attribute()
{
    return this->attributes[this->index_attri];
}

template <typename T> Container_src<T>::Container_src()
{
    this->index_attri = 0;
    this->num_attri = 0;

}

template <typename T> Container_src<T>::~Container_src()
{
    this->clean_attris();
    /*
    for(int i=0; i < this->num; i++)
        delete[] this->contents[i];
        */
    this->contents.clear();
}

template <typename T> void Container_src<T>::print_attris()
{
    for(int i=0; i < num_attri; i++)
    {
        printf("%s\n", this->attributes[i]);
    }
}
template <typename T> void Container_src<T>::split_and_save_into_attris(char* str)
{
    if(strlen(str) == 0)
        throw 1;
    char* var_name;
    var_name = strstr(str, "&&");
    this->push_back_to_attris(str);
    this->num_attri = 1;
    while(var_name)
    {
        *var_name = '\0';
        this->push_back_to_attris(var_name + 2);
        num_attri++;
        var_name = strstr(var_name + 2, "&&");
    }
}

template <typename T> void Container_src<T>::clean_attris()
{
    delete [] this->attributes[0];
    this->attributes.clear();
    this->num_attri = 0;
    this->index_attri = 0;
}

template <typename T> void Container_src<T>::search_attribute_of_content()
{
    int num;
    if(this->num_attri != 0)
        this->clean_attris();
    string str_obj;
    char *var_names;
    num = cfg_json["variable"].size();
    for(int i=0; i < num; i++)
    {
        str_obj = cfg_json["variable"][i]["cor_file"];
        if(strncmp(this->contents[this->index], str_obj.c_str(), MAX_STRING_LONG_LENGTH) == 0)
        {
            str_obj = cfg_json["variable"][i]["name"];
            var_names = new char[MAX_STRING_LONG_LENGTH];
            strncpy(var_names, str_obj.c_str(), MAX_STRING_LONG_LENGTH);
            strip(var_names, ' ');
            split_and_save_into_attris(var_names);
            return;
        }
    }
    CTS_exception ecpt("No variable found for one of the source grid.\n");
    throw ecpt;
}



template <typename T> int Container_src<T>::go_next()
{
    this->index_attri = (this->index_attri + 1) % this->num_attri;
    if(this->index_attri == 0)
    {
        this->index = (this->index + 1) % this->num;
        if(this->index != 0)
            this->search_attribute_of_content();
    }
    return this->index + this->index_attri;
}

void Script_mgt_class::parse_json_from_cfg(char* filename)
{
    int num_grid_pair = 0, size;
    string str_obj;
    char *cor_file, *src_cor, *dst_cor, *str_brk, *str_pt;
    ifstream in_fp(filename);
    try{
        in_fp >> cfg_json;
    }
    catch(invalid_argument emsg){
        printf("Parse error: %s\n", emsg.what());
        //throw
    }
    
    process_number = cfg_json["common"]["process_number"];
    //parse grid pair section
    
    if(cfg_json["grid_pair_auto"]["enable"] == true)
    {
        num_grid_pair = cfg_json["grid_pair_auto"]["cor_file"].size();
        if(num_grid_pair < 1)
        {
            CTS_exception ecpt("No grid pair cor file found.\n");
            throw ecpt;
        }
        this->lock_dst_to_src = false;
        this->src_grid_box.set_num(num_grid_pair);
        this->dst_grid_box.set_num(num_grid_pair);

        for(int i = 0; i < num_grid_pair; i++)
        {
            str_obj = cfg_json["grid_pair_auto"]["cor_file"][i];
            cor_file = new char[MAX_STRING_LONG_LENGTH];
            strncpy(cor_file, str_obj.c_str(), MAX_STRING_LONG_LENGTH - 1);
            this->src_grid_box.push_back_to_contents(cor_file);
            this->dst_grid_box.push_back_to_contents(cor_file);
        }
    }
    else if(cfg_json["grid_pair_manual"]["enable"] == true)
    {
        num_grid_pair = cfg_json["grid_pair_manual"]["cor_file"].size();
        if(num_grid_pair < 1)
        {
            CTS_exception ecpt("No grid pair cor file found.\n");
            throw ecpt;
        }
        this->lock_dst_to_src = true;
        this->src_grid_box.set_num(num_grid_pair);
        this->dst_grid_box.set_num(num_grid_pair);
        for(int i = 0; i < num_grid_pair; i++)
        {
            str_obj = cfg_json["grid_pair_manual"]["cor_file"][i];
            src_cor = new char[MAX_STRING_LONG_LENGTH];
            dst_cor = new char[MAX_STRING_LONG_LENGTH];
            strncpy(src_cor, str_obj.c_str(), MAX_STRING_LONG_LENGTH);
            strip(src_cor, ' ');
            str_brk = strstr(src_cor, "&&");
            if(!str_brk)
            {
                CTS_exception ecpt("Found grid not in pair, check the grammar.\n");
                throw(ecpt);
            }

            strncpy(dst_cor, str_brk+2, MAX_STRING_LONG_LENGTH);
            src_cor[str_brk - src_cor] = '\0';
            this->src_grid_box.push_back_to_contents(src_cor);
            this->dst_grid_box.push_back_to_contents(dst_cor);
        }
    }
    else
        printf("Error, no grid\n");
        //throw
   /* 
    //variable section
    size = cfg_json["variable"].size();
    if(size < 1)
    {
        CTS_exception ecpt("No variable found. \n");
        throw(ecpt);
    }
    this->src_grid_box.set_num_attris(size);
    for(int i=0; i < size; i++)
    {
    */    

    //strategy section
    size = cfg_json["strategy"].size();
    if(size < 1)
    {
        CTS_exception ecpt("No remap strategy found.\n");
        throw(ecpt);
    }
    this->strategy_box.set_num(size);
    for(int i=0; i < size; i++)
    {
            str_obj = cfg_json["strategy"][i]["name"];
            str_pt = new char[MAX_STRING_LENGTH];
            strncpy(str_pt, str_obj.c_str(), MAX_STRING_LENGTH);
            this->strategy_box.push_back_to_contents(str_pt);
            
        if(!cfg_json["strategy"][i]["parameter"].is_null())
        {
            str_obj = cfg_json["strategy"][i]["parameter"];
            str_pt = new char[MAX_STRING_LONG_LENGTH];
            strncpy(str_pt, str_obj.c_str(), MAX_STRING_LONG_LENGTH);
            this->strategy_box.push_back_to_attris(str_pt);
        }
        else
            this->strategy_box.push_back_to_attris(NULL);
            //warning
    }
}

Script_mgt_class::Script_mgt_class(char* filename)
{
    this->whole = new char[MAX_FILE_SIZE];
    this->whole_re = new char[MAX_FILE_SIZE];
    this->splited = new char[MAX_FILE_SIZE/8];
    this->splited_re = new char[MAX_FILE_SIZE/8];
    this->parameter = new char[MAX_FILE_SIZE/8];
    this->lock_dst_to_src = false;
    this->output_file_id = 0;
    parse_json_from_cfg(filename);
    src_grid_box.search_attribute_of_content();
}

Script_mgt_class::~Script_mgt_class()
{
    delete[] this->whole;
    delete[] this->whole_re;
    delete[] this->splited;
    delete[] this->splited_re;
    delete[] this->parameter;
}



void Script_mgt_class::replace_SD_with(char* str, const char* replace_with)
{
    char* substr;
    while(substr = strstr(str, "$SD"))
    {
        for(int i=0; i < 3; i++)
            substr[i] = replace_with[i];
    }
}

int Script_mgt_class::set_output_file_name()
{
    int length_name = 0;
    this->output_file_name = new char[MAX_STRING_LONG_LENGTH];
    memset(this->output_file_name, 0, MAX_STRING_LONG_LENGTH);
    strncpy(this->output_file_name, this->src_grid_box.get_content(), MAX_STRING_LONG_LENGTH);
    length_name = no_extension(this->output_file_name, this->output_file_name);
    this->output_file_name[length_name++] = '_';
    if(this->lock_dst_to_src)
        strncpy(this->output_file_name + length_name, this->dst_grid_box.get_content(src_grid_box.get_index()), MAX_STRING_LONG_LENGTH - length_name);
    else
        strncpy(this->output_file_name + length_name, this->dst_grid_box.get_content(), MAX_STRING_LONG_LENGTH - length_name);
    length_name = no_extension(this->output_file_name, this->output_file_name);
    this->output_file_name[length_name++] = '_';

    strncpy(this->output_file_name + length_name, this->src_grid_box.get_attribute(), MAX_STRING_LONG_LENGTH - length_name);//
    length_name = strlen(this->output_file_name);
    this->output_file_name[length_name++] = '_';

    strncpy(this->output_file_name + length_name, this->strategy_box.get_content(), MAX_STRING_LONG_LENGTH - length_name);
    return strlen(this->output_file_name);
}

int Script_mgt_class::read_file_to_str(char* filename, char* str, int offset)
{
    FILE* fp;
    char fullname[MAX_STRING_LONG_LENGTH];
    int size;
    assert(offset >= 0);
    strncpy(fullname, PATH_TEMPLATE, MAX_STRING_LONG_LENGTH);
    strncat(fullname, filename, MAX_STRING_LONG_LENGTH);
    fp = fopen(fullname, "r");
    size = fread(str + offset, sizeof(char), MAX_FILE_SIZE - offset, fp);
    if( size >= MAX_FILE_SIZE)
        throw 3;
    fclose(fp);
    return size;
}

void Script_mgt_class::generate_script_file()
{
    int filesize, filesize_re, num_parameter, filesize_prev;
    char script_filename[MAX_STRING_LONG_LENGTH], *ptr_split, *ptr_word, *ptr_sprt, *ptr_end;
    FILE *fp;
    memset(this->whole, '\0', MAX_FILE_SIZE);
    memset(this->whole_re, '\0', MAX_FILE_SIZE);
    memset(this->splited, '\0', MAX_FILE_SIZE/8);
    memset(this->splited_re, '\0', MAX_FILE_SIZE/8);
    this->is_src_splited = false;
    this->is_dst_splited = false;

    //src grid defination
    filesize = this->read_file_to_str(src_grid_box.get_content(), this->whole, 0);
    this->whole[filesize++] = '\n';
    replace_SD_with(this->whole, "src");
    ptr_split = strstr(this->whole, "#SPLIT\n");
    if(ptr_split != NULL)
    {
        this->is_src_splited = true;
        *ptr_split = '\0';
        filesize = strlen(this->whole);
        strncpy(this->splited, ptr_split + 7, MAX_FILE_SIZE / 8);
        
        if(strstr(this->whole, "mask"))
            CTS_WARNING("Mask seems to be defined twice in \"%s\" as src grid.\n", src_grid_box.get_content());
    }
    filesize_prev = filesize;

    //dst grid defination
    if(this->lock_dst_to_src)
        filesize += this->read_file_to_str(dst_grid_box.get_content(src_grid_box.get_index()), this->whole, filesize);
    else
        filesize += this->read_file_to_str(dst_grid_box.get_content(), this->whole, filesize);
    this->whole[filesize++] = '\n';
    replace_SD_with(this->whole, "dst");
    ptr_split = strstr(this->whole, "#SPLIT\n");
    if(ptr_split != NULL)
    {
        this->is_dst_splited = true;
        *ptr_split = '\0';
        filesize = strlen(this->whole);
        strncpy(this->splited_re, ptr_split + 7, MAX_FILE_SIZE / 8);
        
        if(strstr(this->whole + filesize_prev, "mask"))
            CTS_WARNING("Mask seems to be defined twice in \"%s\" as dst grid.\n", this->lock_dst_to_src?dst_grid_box.get_content(src_grid_box.get_index()):dst_grid_box.get_content());
    }

    //from here on, whole_re is different with whole
    strncpy(this->whole_re, this->whole, MAX_FILE_SIZE);
    filesize_re = filesize;

    //The "whole" part 
    //Variable defination
    sprintf(this->whole + filesize, SCRIPT_TEMPLATE_VAR, src_grid_box.get_attribute());
    filesize = strlen(this->whole);
    this->whole[filesize++] = '\n';
    
    //Mask defination
    if(this->is_src_splited)
    {

        strncat(this->whole, this->splited, MAX_FILE_SIZE);
        filesize = strlen(this->whole);
        this->whole[filesize++] = '\n';
    }
    
    //Remap operator defination
    sprintf(this->whole + filesize, SCRIPT_TEMPLATE_OPT, strategy_box.get_content());
    filesize = strlen(this->whole);
    this->whole[filesize++] = '\n';

    this->set_output_file_name();
    filename_list.push_back(this->output_file_name);
    
    //Operator's parameter defination
    memset(this->parameter, '\0', MAX_FILE_SIZE/8);
    strncpy(this->parameter, strategy_box.get_attribute(), MAX_FILE_SIZE/8);
    strip(this->parameter, ' ');
    ptr_sprt = this->parameter;
    if(strlen(this->parameter) == 0)
        num_parameter = 0;
    else
        for(num_parameter = 1; ptr_sprt = strstr(ptr_sprt, ","); num_parameter++);

    ptr_word = this->parameter;
    ptr_sprt = ptr_word;
    ptr_end = ptr_word;
    for(int i = 0; i < num_parameter; i++)
    {
        ptr_sprt = strstr(ptr_word, "=");
        if(ptr_sprt)
            *ptr_sprt = '\0';
        else
            ;//throw
        ptr_end = strstr(ptr_sprt, ",");
        if(ptr_end)
            *ptr_end = '\0';
        sprintf(this->whole + filesize, SCRIPT_TEMPLATE_SET, ptr_word, ptr_sprt + 1);
        filesize = strlen(this->whole);
        ptr_word = ptr_end + 1;
        ptr_sprt = ptr_word;
        ptr_end = ptr_word;
    }

    //Output defination
    sprintf(this->whole + filesize, SCRIPT_TEMPLATE_WRITE, this->output_file_name);
    filesize = strlen(this->whole);
    assert(filesize <= MAX_FILE_SIZE);
    
    //Write to .cor file
    sprintf(script_filename, "./script/%s.cor", this->output_file_name);
    fp = fopen(script_filename, "w");
    fwrite(this->whole, sizeof(char), filesize, fp);
    fclose(fp);

    //The "whole_re" part
    //Variable defination
    sprintf(this->whole_re + filesize_re, SCRIPT_TEMPLATE_VAR_RE, this->output_file_name, src_grid_box.get_attribute());
    filesize_re = strlen(this->whole_re);
    this->whole_re[filesize_re++] = '\n';
    
    //Both masks of src grid and dst grid are needed
    //Mask defination
    if(this->is_src_splited)
    {
        strncat(this->whole_re, this->splited, MAX_FILE_SIZE);
        filesize_re = strlen(this->whole_re);
        this->whole_re[filesize_re++] = '\n';
    }
    
    //Mask defination
    if(this->is_dst_splited)
    {

        strncat(this->whole_re, this->splited_re, MAX_FILE_SIZE);
        filesize_re = strlen(this->whole_re);
        this->whole_re[filesize_re++] = '\n';
    }

    //Remap operator defination
    sprintf(this->whole_re + filesize_re, SCRIPT_TEMPLATE_OPT_RE, strategy_box.get_content());
    filesize_re = strlen(this->whole_re);
    this->whole_re[filesize_re++] = '\n';

    //Operator's parameter defination
    memset(this->parameter, '\0', MAX_FILE_SIZE/8);
    strncpy(this->parameter, strategy_box.get_attribute(), MAX_FILE_SIZE/8);
    strip(this->parameter, ' ');
    ptr_sprt = this->parameter;
    if(strlen(this->parameter) == 0)
        num_parameter = 0;
    else
        for(num_parameter = 1; ptr_sprt = strstr(ptr_sprt, ","); num_parameter++);

    ptr_word = this->parameter;
    ptr_sprt = ptr_word;
    ptr_end = ptr_word;

    for(int i = 0; i < num_parameter; i++)
    {
        ptr_sprt = strstr(ptr_word, "=");
        if(ptr_sprt)
            *ptr_sprt = '\0';
        else
            ;//throw
        ptr_end = strstr(ptr_sprt, ",");
        if(ptr_end)
            *ptr_end = '\0';
        sprintf(this->whole_re + filesize_re, SCRIPT_TEMPLATE_SET, ptr_word, ptr_sprt + 1);
        filesize_re = strlen(this->whole_re);
        ptr_word = ptr_end + 1;
        ptr_sprt = ptr_word;
        ptr_end = ptr_word;
    }
   
    //Output defination
    sprintf(this->whole_re + filesize_re, SCRIPT_TEMPLATE_WRITE_RE, this->output_file_name);
    filesize_re = strlen(this->whole_re);
    assert(filesize_re <= MAX_FILE_SIZE);
    
    //Write to .cor file
    sprintf(script_filename, "./script/%s_RE.cor", this->output_file_name);
    fp = fopen(script_filename, "w");
    fwrite(this->whole_re, sizeof(char), filesize_re, fp);
    fclose(fp);

    this->output_file_id++;
    num_script = this->output_file_id;
}

bool Script_mgt_class::is_src_dst_same()
{
    return strncmp(this->src_grid_box.get_content(), this->dst_grid_box.get_content(), MAX_STRING_LONG_LENGTH) == 0;
}

void Script_mgt_class::loop()
{
    int index_sttg, index_dst, index_src;
    while(1)
    {
        if(!this->is_src_dst_same())
            this->generate_script_file();

        index_sttg = strategy_box.go_next();
        if(index_sttg == 0)
        {
            if(!lock_dst_to_src)
            {   
                index_dst = dst_grid_box.go_next();
                if(index_dst == 0)
                {
                    index_src = src_grid_box.go_next();
                    if(index_src == 0)
                        break;
                }
            }
            else
            {   
                index_src = src_grid_box.go_next();
                if(index_src == 0)
                    break;
            }
        }

    }
}
