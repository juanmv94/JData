#ifndef JDATA_H
#define JDATA_H

#include <vector>

using namespace std;

///////////////
// Data types
//////////////

#define JDATA_WHATEVER 0
#define JDATA_NULL 1
#define JDATA_BOOLEAN 2
#define JDATA_STRING 3
#define JDATA_UINT8 4
#define JDATA_INT8 5
#define JDATA_UINT16 6
#define JDATA_INT16 7
#define JDATA_UINT32 8
#define JDATA_INT32 9
#define JDATA_UINT64 10
#define JDATA_INT64 11
#define JDATA_FLOAT 12
#define JDATA_DOUBLE 13

///////////////////
// Data type flags
/////////////////

#define JDATA_ARRAY 32
#define JDATA_STRUCTURE 64
#define JDATA_REFERENCED 128

///////////////////
// Array sizes
/////////////////

#define JDATA_ARRAYSIZE_NONE 0
#define JDATA_ARRAYSIZE_UINT8 1
#define JDATA_ARRAYSIZE_UINT16 2
#define JDATA_ARRAYSIZE_UINT32 3


class jdata
{
public:
    static int structure(short stid);
    static int data(char dtype);
    static int ref_structure(short stid);
    static int ref_data(char dtype);
    static int structure_arr(short stid, char arr_size_value);
    static int data_arr(char dtype,char arr_size_value);
    static int ref_structure_arr(short stid, char arr_size_value);
    static int ref_data_arr(char dtype, char arr_size_value);
    
    static bool is_structure(int type);    //checks structure flag
    static bool is_array(int type);        //checks array flag
    static bool is_referenced(int type);   //checks referenced flag
    static char get_data_type(int type);   //gets data type without flags. Assumes data type.
    static short get_struct_number(int type); //gets struct type without flags. Assumes struct type.
    static char get_array_size(int type);   //gets the array size byte value
    
    static char get_flags(int type);        //gets a byte with only the flags
    
    static int size_of_type(int type);      //gets the size in bytes from a data type
};

///////////////////
// Layout field struct
/////////////////

class jdata_layout_field
{
public:
    const char * name;
    int type;
    
    jdata_layout_field(const char * name, int type);
    
    bool is_structure();    //checks structure flag
    bool is_array();        //checks array flag
    bool is_referenced();   //checks referenced flag
    char get_data_type();   //gets data type without flags. Assumes data type.
    short get_struct_number(); //gets struct type without flags. Assumes struct type.
    char get_array_size();   //gets the array size byte value
    
};

///////////////////
// Layout structure
/////////////////

#define jdata_layout_structure vector<jdata_layout_field>

///////////////////
// Layout class
/////////////////

class jdata_layout
{
private:
    
public:
    vector<jdata_layout_structure> structures;
    
    jdata_layout();
    jdata_layout(char * input, int input_size);
    
    jdata_layout_structure* get_main_structure();
    int find_field_index(int structureID,const char* field_name);     //Finds a field index from a structure by its name. Returns -1 if not exists.
    vector<char> serialize();
};

///////////////////
//
// JData Data
//
/////////////////


#define jdata_data_list vector<jdata_data_element>

class jdata_data_element
{
private:
    
public:
    long value;             //This 8byte field stores all number types, pointers (string,array,...), or referenced/structure value IDs.
    int array_length;       //only used on arrays
    int type;               //only used in referenced elements vector, and whatever data types
    
    jdata_data_element();
    jdata_data_element(const void* pointer, int arraysize);     //This is the way to create arrays
    jdata_data_element(long i);
    jdata_data_element(int i);
    jdata_data_element(short s);
    jdata_data_element(char c);
    jdata_data_element(float f);
    jdata_data_element(double d);
    jdata_data_element(const char * str);
    jdata_data_element(bool b);
    
    void set(const void* pointer, int arraysize);     //This is the way to create arrays
    void set(long i);
    void set(int i);
    void set(short s);
    void set(char c);
    void set(float f);
    void set(double d);
    void set(const char * str);
    void set(bool b);

    long get_long();
    int get_int();
    short get_short();
    char get_char();
    float get_float();
    double get_double();
    const char* get_str_pointer();  //use this to get either strings or either array pointers
    bool get_bool();
};

class jdata_data
{
private:
    
public:
    jdata_data_list main_structure;
    jdata_data_list referenced_elements;
    vector<jdata_data_list> structures_data;
    
    jdata_data();                               //Creates empty data
    jdata_data(jdata_layout* layout, char* input, int input_size);   //Parsed JData data for layout
    
    vector<char> serialize(jdata_layout* layout);
    
    jdata_data_element create_structure_data_element(int structureID, jdata_data_list jdl);  //creates a structure jdata_data_element with jdata_data_list jdl

    int push_referenced_element(jdata_data_element el, int type);       //Sets jdata_data_element type value before inserting into referenced_elements and returns ID
};

//JData data serializer and parser have their own classes due to complexity

class jdata_data_serializer
{
private:
    int referenced_counter=0;
    vector<bool> booleans;
    jdata_data* jdd;
    jdata_layout* jds;
    void write_bytes(const char* in,int count);
    void write_reference(int refid);                    //writes UINT8, UINT16, or UINT32 refid depending referenced_counter
    void write_arraysize(int arrsize,int type);
    void write_struct(int structures_data_id,int structID);
    void write_struct(jdata_data_list* st_data,int structID);
    void write_type_and_element(jdata_data_element el, int type);
    void write_type(int type);
    void write_element(jdata_data_element el, int type);
    
    //Little endian encoding
    short to_le_2b(char* in);
    int to_le_4b(char* in);
    long to_le_8b(char* in);
    
public:
    vector<char> data;
    jdata_data_serializer(jdata_layout* jds_in,jdata_data* jdd_in);
};

#endif /* JDATA_H */