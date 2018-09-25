#include <cstring>
#include <stdexcept>
#include "jdata.h"

jdata_layout_field::jdata_layout_field(const char * name, int type) : name(name), type(type) {}

int jdata::structure(short stid)
{
    int res;
    char* st=(char*)(&res);
    st[1]=stid % 256;
    st[0]=(stid / 256) | JDATA_STRUCTURE;
    return res;
}

int jdata::data(char dtype)
{
    int res;
    char* st=(char*)(&res);
    st[0]=dtype;
    return res;
}

int jdata::ref_structure(short stid)
{
    int res;
    char* st=(char*)(&res);
    st[1]=stid % 256;
    st[0]=(stid / 256) | JDATA_STRUCTURE | JDATA_REFERENCED;
    return res;
}

int jdata::ref_data(char dtype)
{
    int res;
    char* st=(char*)(&res);
    st[0]=dtype|JDATA_REFERENCED;
    return res;
}

int jdata::structure_arr(short stid,char arr_size_value)
{
    int res;
    char* st=(char*)(&res);
    st[1]=stid % 256;
    st[0]=(stid / 256) | JDATA_STRUCTURE | JDATA_ARRAY;
    st[2]=arr_size_value;
    return res;
}

int jdata::data_arr(char dtype,char arr_size_value)
{
    int res;
    char* st=(char*)(&res);
    st[0]=dtype| JDATA_ARRAY;
    st[1]=arr_size_value;
    return res;
}

int jdata::ref_structure_arr(short stid,char arr_size_value)
{
    int res;
    char* st=(char*)(&res);
    st[1]=stid % 256;
    st[0]=(stid / 256) | JDATA_STRUCTURE | JDATA_REFERENCED| JDATA_ARRAY;
    st[2]=arr_size_value;
    return res;
}

int jdata::ref_data_arr(char dtype,char arr_size_value)
{
    int res;
    char* st=(char*)(&res);
    st[0]=dtype|JDATA_REFERENCED| JDATA_ARRAY;
    st[1]=arr_size_value;
    return res;
}

bool jdata::is_structure(int type) {return (*(char*)(&type) & JDATA_STRUCTURE) !=0;}
bool jdata::is_array(int type) {return (*(char*)(&type) & JDATA_ARRAY) !=0;}
bool jdata::is_referenced(int type) {return (*(char*)(&type) & JDATA_REFERENCED) !=0;}
char jdata::get_data_type(int type) {return (*(char*)(&type) & 31);}
short jdata::get_struct_number(int type) {char* t=(char*)(&type); return ((t[0] & 0x1F)*256+t[1]);}
char jdata::get_array_size(int type) {char* t=(char*)(&type);if(jdata::is_structure(type)) return t[2];else return t[1];}
char jdata::get_flags(int type) {char* t=(char*)(&type); return (t[0] & 0xE0);}

int jdata::size_of_type(int type) {return 1+is_structure(type)+is_array(type);}

bool jdata_layout_field::is_structure() {return jdata::is_structure(type);}
bool jdata_layout_field::is_array() {return jdata::is_array(type);}
bool jdata_layout_field::is_referenced() {return jdata::is_referenced(type);}
char jdata_layout_field::get_data_type() {return jdata::get_data_type(type);}
short jdata_layout_field::get_struct_number() {return jdata::get_struct_number(type);}

jdata_layout::jdata_layout() {}

jdata_layout::jdata_layout(char * input, int input_size)
{
    bool is_new_struct=true;
    char* p=input;
    char* end=input+input_size;
    jdata_layout_structure* last_structure;
    while (p<end)
    {
        char* fieldname;
        //Check if new struct
        if (*p==0x00)
        {
            is_new_struct=true;
            fieldname=++p;
        }
        else
        {
            fieldname=p++;
        }
        //parse field name
        do
        {
            if (p>=end) return;         //Unexpected end of data
        } while (*(p++)!=0x00);

        if (p>=end) return;             //Unexpected end of data
        
        //parse field type
        int type;
        int type_size=1;
        if ((*p & JDATA_STRUCTURE)!=0) type_size++;
        if ((*p & JDATA_ARRAY)!=0) type_size++;
        if ((p+type_size)>end) return;  //Unexpected end of data
        memcpy(&type,p,type_size);
        p+=type_size;
        
        //Add field to current struct
        if (is_new_struct)
        {
            structures.push_back(jdata_layout_structure());
            last_structure=&structures.back();
            is_new_struct=false;
        }
        last_structure->push_back(jdata_layout_field(fieldname,type));
    }
}

jdata_layout_structure* jdata_layout::get_main_structure() {return (jdata_layout_structure*)(&structures.back());}

int jdata_layout::find_field_index(int structureID,const char* field_name)
{
    jdata_layout_structure* st=&structures[structureID];
    int n=0;
    for (jdata_layout_structure::iterator i=st->begin();i!=st->end();i++)
    {
        if (strcmp(i->name, field_name)==0) return n;
        n++;
    }
    return -1;
}

vector<char> jdata_layout::serialize()
{
    vector<char> out;
    
    //We serialize all the structures
    for (vector<jdata_layout_structure>::iterator i=structures.begin();i!=structures.end();i++)
    {
        //We serialize all the structure fields
        for (jdata_layout_structure::iterator j=i->begin();j!=i->end();j++)
        {
            //field name
            const char* c=j->name;
            do {out.push_back(*c);} while (*(c++)!=0x00);
            
            //field type
            int type_size=1;
            if (j->is_structure()) type_size++;
            if (j->is_array()) type_size++;
            c=(char*)(&(j->type));
            for (int k=0;k<type_size;k++) out.push_back(*(c++));
            
        }
        out.push_back(0x00);    //End of structure
    }
    out.pop_back();    //We discard last 0x00 character
    return out;
}

///////////////////
//
// JData Data
//
/////////////////

jdata_data_element::jdata_data_element() {}
jdata_data_element::jdata_data_element(const void* pointer, int arraysize) {set(pointer,arraysize);}
jdata_data_element::jdata_data_element(long i) {set(i);}
jdata_data_element::jdata_data_element(int i) {set(i);}
jdata_data_element::jdata_data_element(short s) {set(s);}
jdata_data_element::jdata_data_element(char c) {set(c);}
jdata_data_element::jdata_data_element(float f) {set(f);}
jdata_data_element::jdata_data_element(double d) {set(d);}
jdata_data_element::jdata_data_element(const char* str) {set(str);}
jdata_data_element::jdata_data_element(bool b) {set(b);}

void jdata_data_element::set(const void* pointer, int arraysize) {array_length=arraysize; memcpy(&value,&pointer,sizeof(const void*));}
void jdata_data_element::set(long i) {value=i;}
void jdata_data_element::set(int i) {memcpy(&value,&i,sizeof(int));}
void jdata_data_element::set(short s) {memcpy(&value,&s,sizeof(short));}
void jdata_data_element::set(char c) {memcpy(&value,&c,sizeof(char));}
void jdata_data_element::set(float f) {memcpy(&value,&f,sizeof(float));}
void jdata_data_element::set(double d) {memcpy(&value,&d,sizeof(double));}
void jdata_data_element::set(const char* str) {memcpy(&value,&str,sizeof(const char*));}
void jdata_data_element::set(bool b) {value=b;}

long jdata_data_element::get_long() {return value;}
int jdata_data_element::get_int() {return *(int*)(&value);}
short jdata_data_element::get_short() {return *(short*)(&value);}
char jdata_data_element::get_char() {return *(char*)(&value);;}
float jdata_data_element::get_float() {return *(float*)(&value);}
double jdata_data_element::get_double() {return *(double*)(&value);}
const char* jdata_data_element::get_str_pointer() {return *(const char**)(&value);}
bool jdata_data_element::get_bool() {return value;}


jdata_data::jdata_data() {}
jdata_data::jdata_data(jdata_layout* layout, char* input, int input_size)
{
    //TODO
}

vector<char> jdata_data::serialize(jdata_layout* layout)
{
    return jdata_data_serializer(layout,this).data;
}

jdata_data_element jdata_data::create_structure_data_element(int structureID, jdata_data_list jdl)
{
    structures_data.push_back(jdl);
    return jdata_data_element((int)(structures_data.size()-1));
}

int jdata_data::push_referenced_element(jdata_data_element el, int type)
{
    el.type=type;
    referenced_elements.push_back(el);
    return referenced_elements.size();
}



void jdata_data_serializer::write_bytes(const char* in,int count)
{
    for (int i=0;i<count;i++) data.push_back(in[i]);
}

void jdata_data_serializer::write_reference(int refid)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define REFID_LE refid
#else
    int REFID_LE=to_le_4b((char*)(&refid));
#endif
    if (referenced_counter<=0xFF)
        write_bytes((char*)(&REFID_LE),1);
    else if (referenced_counter<=0xFFFF)
        write_bytes((char*)(&REFID_LE),2);
    else
        write_bytes((char*)(&REFID_LE),4);
#undef REFID_LE
}
void jdata_data_serializer::write_arraysize(int arrsize,int type)
{
    switch (jdata::get_array_size(type))
    {
        case JDATA_ARRAYSIZE_NONE:  //do nothing
            break;
        case JDATA_ARRAYSIZE_UINT8:
        {
            unsigned char n=arrsize;
            write_bytes((char*)(&arrsize),1);
            break;
        }
        case JDATA_ARRAYSIZE_UINT16:
        {
            unsigned short n=arrsize;
            unsigned short n_le=to_le_2b((char*)(&n));
            write_bytes((char*)(&n_le),2);
            break;
        }
        case JDATA_ARRAYSIZE_UINT32:
        {
            unsigned int n=arrsize;
            unsigned int n_le=to_le_4b((char*)(&n));
            write_bytes((char*)(&n_le),4);
            break;
        }
        default:
            throw std::invalid_argument("Invalid JData array size");
    }
}

void jdata_data_serializer::write_struct(int structures_data_id,int structID)
{
    write_struct(&(jdd->structures_data.at(structures_data_id)),structID);
}

void jdata_data_serializer::write_struct(jdata_data_list* st_data,int structID)
{
    int el=0;
    for (jdata_layout_structure::iterator i=jds->structures.at(structID).begin();i!=jds->structures.at(structID).end();i++)
    {
        write_element(st_data->at(el++),i->type);
    }
}

void jdata_data_serializer::write_type_and_element(jdata_data_element el, int type)
{
    write_type(type);
    write_element(el,type);
}

void jdata_data_serializer::write_type(int type)
{
    write_bytes((char*)(&type),jdata::size_of_type(type));
}

void jdata_data_serializer::write_element(jdata_data_element el, int type)
{
    //Note: referenced types are not checked
    if (jdata::is_referenced(type))     //referenced element
    {
        if (jdata::is_array(type))  //referenced struct array
        {
            const int* ref_IDs=(const int*)(el.get_str_pointer());
            for (int i=0;i<el.array_length;i++) write_reference(ref_IDs[i]);
        }
        else                        //referenced struct
        {
            write_reference(el.get_int());
        }
    }
    else if (!jdata::is_structure(type)) //data types
    {
        switch (jdata::get_data_type(type))
        {
                case JDATA_WHATEVER: //data type must be read from the element
                {
                    if (jdata::is_array(type))
                    {
                        throw std::invalid_argument("JDATA_WHATEVER arrays must use referenced type");
                    }
                    if (!jdata::is_structure(el.type) && jdata::get_data_type(el.type)==JDATA_WHATEVER)  //Throw ex if whatever type here too
                        throw std::invalid_argument("JDATA_WHATEVER type value is also set as JDATA_WHATEVER");
                    //Push type value into data
                    write_type(el.type);
                    //new write element call
                    write_element(el,el.type);
                    //Note: array flag is not checked when whatever type. Whatever arrays are not allowed in JData.
                    break;
                }
            case JDATA_NULL:    //do nothing
                break;
            case JDATA_BOOLEAN:
            {
                booleans.push_back(el.get_bool());
                break;                              //Note: for boolean types, references and arrays are not allowed
            }
            case JDATA_STRING:
            {
                if (jdata::is_array(type))
                {
                    write_arraysize(el.array_length,type);
                    const char** string_arr=(const char**)(el.get_str_pointer());
                    for (int i=0;i<el.array_length;i++)
                        write_bytes(string_arr[i],strlen(string_arr[i])+1);
                }
                else
                {
                    write_bytes(el.get_str_pointer(),strlen(el.get_str_pointer())+1);
                }
                break;
            }
            case JDATA_UINT8:
            case JDATA_INT8:
            {
                if (jdata::is_array(type))
                {
                    write_arraysize(el.array_length,type);
                    write_bytes(el.get_str_pointer(),el.array_length);
                }
                else
                {
                    char res=el.get_char();
                    write_bytes(&res,1);
                }
                break;
            }
            case JDATA_UINT16:
            case JDATA_INT16:
            {
                if (jdata::is_array(type))
                {
                    write_arraysize(el.array_length,type);
                    //Little endian only!!
                    write_bytes(el.get_str_pointer(),el.array_length*2);
                }
                else
                {
                    short res=el.get_short();
                    short res_le=to_le_2b((char*)(&res));
                    write_bytes((char*)(&res_le),2);
                }
                break;
            }
            case JDATA_UINT32:
            case JDATA_INT32:
            case JDATA_FLOAT:
            {
                if (jdata::is_array(type))
                {
                    write_arraysize(el.array_length,type);
                    //Little endian only!!
                    write_bytes(el.get_str_pointer(),el.array_length*4);
                }
                else
                {
                    int res=el.get_int();
                    int res_le=to_le_4b((char*)(&res));
                    write_bytes((char*)(&res_le),4);
                }
                break;
            }
            case JDATA_UINT64:
            case JDATA_INT64:
            case JDATA_DOUBLE:
            {
                if (jdata::is_array(type))
                {
                    write_arraysize(el.array_length,type);
                    //Little endian only!!
                    write_bytes(el.get_str_pointer(),el.array_length*8);
                }
                else
                {
                    long res=el.get_long();
                    long res_le=to_le_8b((char*)(&res));
                    write_bytes((char*)(&res_le),8);
                }
                break;
            }
            default:
                throw std::invalid_argument("Invalid JData type");
        }
    }
    else    //structure types
    {
        if (jdata::is_array(type))
        {
            write_arraysize(el.array_length,type);
            const int* struct_data_IDs=(const int*)(el.get_str_pointer());
            for (int i=0;i<el.array_length;i++)
            {
                write_struct(struct_data_IDs[i],jdata::get_struct_number(type));
            }
        }
        else
        {
            write_struct(el.get_int(),jdata::get_struct_number(type));
        }
    }
}

short jdata_data_serializer::to_le_2b(char* in)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return *(short*)(in);
#else
    short res;
    char* res_c=&res;
    res_c[0]=in[1];
    res_c[1]=in[0];
    return res;
#endif
}
int jdata_data_serializer::to_le_4b(char* in)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return *(int*)(in);
#else
    int res;
    char* res_c=&res;
    res_c[0]=in[3];
    res_c[1]=in[2];
    res_c[2]=in[1];
    res_c[3]=in[0];
    return res;
#endif
}
long jdata_data_serializer::to_le_8b(char* in)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return *(long*)(in);
#else
    long res;
    char* res_c=&res;
    res_c[0]=in[7];
    res_c[1]=in[6];
    res_c[2]=in[5];
    res_c[3]=in[4];
    res_c[4]=in[3];
    res_c[5]=in[2];
    res_c[6]=in[1];
    res_c[7]=in[0];
    return res;
#endif
}

jdata_data_serializer::jdata_data_serializer(jdata_layout* jds_in,jdata_data* jdd_in) : jds(jds_in),jdd(jdd_in)
{
    //write referenced data
    for (jdata_data_list::iterator i=jdd->referenced_elements.begin();i!=jdd->referenced_elements.end();i++)
    {
        write_type_and_element((jdata_data_element)(*i), i->type);
        referenced_counter++;
    }
    
    //write main structure data
    short main_struct_id=jds->structures.size()-1;
    write_type(jdata::structure(main_struct_id));
    write_struct(&(jdd->main_structure),main_struct_id);
    
    //write booleans
    int bit=0;
    char curchar=0x00;
    vector<char> boolbytes;
    for (vector<bool>::iterator i=booleans.begin();i!=booleans.end();i++)
    {
        if (*i) curchar|=(1<<(bit));
        if ((++bit)==8)
        {
            bit=0;
            boolbytes.push_back(curchar);
            curchar=0x00;
        }
    }
    write_bytes(boolbytes.data(),boolbytes.size());
    if (bit>0) write_bytes(&curchar,1);
}
