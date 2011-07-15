#ifndef _SFMPOSTLINE_H                       // duplication check
#define _SFMPOSTLINE_H

#include <common.h>

using namespace std;

namespace superfastmatch
{

  struct PostLineHeader{
    uint64_t doc_type;
    uint64_t length;
    
    PostLineHeader():
    doc_type(0),length(0){}
    
    PostLineHeader(uint64_t doc_type,uint64_t length):
    doc_type(doc_type),length(length){}
  };

  class PostLineCodec{
  public:
    virtual size_t encodeHeader(const vector<PostLineHeader>& header,unsigned char* start)=0;
    virtual size_t decodeHeader(const unsigned char* start, vector<PostLineHeader>& header)=0;
    virtual size_t encodeSection(const vector<uint32_t>& section,unsigned char* start)=0;
    virtual size_t decodeSection(const unsigned char* start, const size_t length, vector<uint32_t>& section)=0;
  };
  
  class VarIntCodec: public PostLineCodec{
  public:
    size_t encodeHeader(const vector<PostLineHeader>& header,unsigned char* start);
    size_t decodeHeader(const unsigned char* start, vector<PostLineHeader>& header);
    size_t encodeSection(const vector<uint32_t>& section,unsigned char* start);
    size_t decodeSection(const unsigned char* start, const size_t length, vector<uint32_t>& section);
  };

  class PostLine{
  private:
    PostLineCodec* codec_;
    unsigned char* start_;
    vector<PostLineHeader> header_;
    vector<uint32_t> section_;
    uint32_t updated_section_;
    size_t old_section_length_;
    size_t new_section_length_;
    size_t old_header_length_;
    size_t new_header_length_;
    unsigned char* new_header_;
    unsigned char* new_section_;
  
  public:
    PostLine(PostLineCodec* codec,uint32_t max_length);
    ~PostLine();

    void load(const unsigned char* start);
    // out must have a length greater than or equal to getLength()
    void commit(unsigned char* out);
    // commit must be called after each add/delete operation
    void addDocument(const uint32_t doc_type,const uint32_t doc_id);
    void deleteDocument(const uint32_t doc_type,const uint32_t doc_id);
    size_t getLength();
    size_t getLength(const uint32_t doc_type);
    void getDocTypes(vector<uint32_t>& doc_types);
    void getDocIds(const uint32_t doc_type,vector<uint32_t>& doc_ids);
    void getDeltas(const uint32_t doc_type,vector<uint32_t>& deltas);
    
    friend std::ostream& operator<< (std::ostream& stream, PostLine& postline);
  private:
    PostLine(const PostLine&);
    PostLine& operator=(const PostLine&);
  };
}
#endif