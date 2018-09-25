#include <cstring>
#include <fstream>
#include "jdata.h"

using namespace std;

int main(int argc, char** argv) {
    
    jdata_layout sample;    //We create a new empty JData layout
    
    ////////////////////////////////////
    // We create our JData structures, and we add them to the JData layout
    //////////////////////////////////
    
    //category structure (0)
    sample.structures.push_back({
        jdata_layout_field("name",jdata::data(JDATA_STRING)),
        jdata_layout_field("is_adult",jdata::data(JDATA_BOOLEAN))                
    });
    
    //author structure (1)
    sample.structures.push_back({
        jdata_layout_field("name",jdata::data(JDATA_STRING)),
        jdata_layout_field("github",jdata::data(JDATA_STRING))
    });
    
    //photo structure (2)
    sample.structures.push_back({
        jdata_layout_field("photo_name",jdata::data(JDATA_STRING)),
        jdata_layout_field("photo_jpg",jdata::data_arr(JDATA_UINT8,JDATA_ARRAYSIZE_UINT32)),
        jdata_layout_field("category",jdata::ref_structure(0)),    //category structure
        jdata_layout_field("author",jdata::ref_structure(1))       //author structure
    });
    
    //main structure (3)
    sample.structures.push_back({
        jdata_layout_field("photo_count",jdata::data(JDATA_UINT32)),
        jdata_layout_field("photos",jdata::structure_arr(2,JDATA_ARRAYSIZE_UINT32))
    });
    
    ////////////////////////////////////
    // We get our JData layout data and save it in a binary file
    //////////////////////////////////
    vector<char> output=sample.serialize();
    char* output_data=output.data();
    int output_size=output.size();
    
    ofstream jds;
    jds.open("sample.jds",ios::binary);
    if (jds.is_open())
        jds.write(output_data,output_size);
    
    //////////////////////////////////////////
    // Now lets parse the serialized layout data
    ///////////////////////////////////////
    jdata_layout sample_recovered(output_data,output_size);
    

    //////////////////////////////////////////
    // We create empty JData data
    ///////////////////////////////////////
    jdata_data sample_data;
    
    //////////////////////////////////////////
    // We fill the category and author referenced structures and insert them into referenced_elements
    ///////////////////////////////////////
    
    //Referenced ID=0, structure_data ID=0, category (structure 0): porn
    //We use the find_field_index function to get the field index for "name" and "is_adult" this time
    jdata_data_list p_list(sample_recovered.structures[0].size());
    p_list[sample_recovered.find_field_index(0,"name")]=jdata_data_element("porn");
    p_list[sample_recovered.find_field_index(0,"is_adult")]=jdata_data_element(true);
    sample_data.push_referenced_element(sample_data.create_structure_data_element(0,p_list),jdata::structure(0));
    
    //Referenced ID=1, structure_data ID=1, category (structure 0): landscape
    sample_data.push_referenced_element(sample_data.create_structure_data_element(0,{
        jdata_data_element("landscape"),                                        //name
        jdata_data_element(false)                                               //is_adult
    }),jdata::structure(0));
    
    //Referenced ID=2, structure_data ID=2, author (structure 1): Juan
    sample_data.push_referenced_element(sample_data.create_structure_data_element(1,{
        jdata_data_element("Juan"),                                             //Name
        jdata_data_element("Juanmv94")                                          //GitHub
    }),jdata::structure(1));
    
    //////////////////////////////////////////
    // We fill the two photos and insert them into structures_data
    ///////////////////////////////////////
    const char* placeholderdata="BINARY_DATA";
    
    //structure_data ID=3 photo (structure 2): Juanmv94 profile photo
    sample_data.structures_data.push_back({
        jdata_data_element("Juanmv94 profile photo"),                           //Photo name
        jdata_data_element(placeholderdata,strlen(placeholderdata)),            //Photo jpg (array)
        jdata_data_element(0),                                                  //Referenced ID=0;
        jdata_data_element(2)                                                   //Referenced ID=2;
    });
    
    //structure_data ID=4 photo (structure 2): Albacete Beach
    sample_data.structures_data.push_back({
        jdata_data_element("Albacete beach"),                                   //Photo name
        jdata_data_element(placeholderdata,strlen(placeholderdata)),            //Photo jpg (array)
        jdata_data_element(1),                                                  //Referenced ID=1;
        jdata_data_element(2)                                                   //Referenced ID=2;
    });
    
    const int photos[]={3,4};

    //////////////////////////////////////////
    // We fill the main structure
    ///////////////////////////////////////
    sample_data.main_structure.push_back(jdata_data_element(2));
    sample_data.main_structure.push_back(jdata_data_element(photos,2));
    
    ////////////////////////////////////
    // We get our JData data and save it in a binary file
    //////////////////////////////////
    vector<char> jddv=sample_data.serialize(&sample_recovered);
    char* jdds=jddv.data();
    int jdds_size=jddv.size();
    ofstream jdd;
    jdd.open("sample.jdd",ios::binary);
    if (jdd.is_open())
        jdd.write(jdds,jdds_size);

    return 0;
}