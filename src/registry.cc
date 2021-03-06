#include <registry.h>
#include <gflags/gflags.h>
#include <validators.h>
#include <posting.h>
#include <document.h>
#include <queue.h>

namespace superfastmatch{
  // Templates
  RegisterTemplateFilename(STATUS_JSON, "JSON/status.tpl");
  
  // Command line flags
  
  DEFINE_bool(daemonize,false,"Run process in the background. File paths must be absolute if this is true.");

  DEFINE_string(log_file,"-","File to write logs to (defaults to stdout)");
  
  DEFINE_int32(port, 8080, "What port to listen on");
  static const bool port_dummy = google::RegisterFlagValidator(&FLAGS_port, &ValidatePort);

  DEFINE_string(address,"" , "What address to listen on (defaults to all addresses)");
  static const bool address_dummy = google::RegisterFlagValidator(&FLAGS_address, &ValidateAddress);

  DEFINE_int32(thread_count,8,"Number of threads for serving requests");
  static const bool thread_count__dummy = google::RegisterFlagValidator(&FLAGS_thread_count, &ValidateThreads);

  DEFINE_int32(slot_count,4,"Number of slots to divide the index into (one thread per slot)");
  static const bool slot_count_dummy = google::RegisterFlagValidator(&FLAGS_slot_count, &ValidateThreads);

  DEFINE_int32(cache,2048,"Number of megabytes to use for caching");
  static const bool cache_dummy = google::RegisterFlagValidator(&FLAGS_cache, &ValidateCache);

  DEFINE_int32(hash_width,26,"Number of bits to use to hash windows of text");
  static const bool hash_width_dummy = google::RegisterFlagValidator(&FLAGS_hash_width, &ValidateHashWidth);

  DEFINE_int32(window_size,30,"Number of characters to use as a window of text for hashing");
  static const bool window_size_dummy = google::RegisterFlagValidator(&FLAGS_window_size, &ValidateWindowSize);
  
  DEFINE_double(white_space_threshold,0.5,"Percentage of non alphanumeric characters in a window above which window is considered whitespace");
  static const bool white_space_threshold_dummy = google::RegisterFlagValidator(&FLAGS_white_space_threshold,&ValidateWhiteSpaceThreshold);
    
  DEFINE_int32(max_posting_threshold,100,"Number of entries for a hash above which are ignored for search");

  DEFINE_int32(num_results,10,"Minimum number of documents to associate with");
 
  DEFINE_string(data_path,"data","Path to store data files");

  DEFINE_string(public_path,"public","Path to store static files for web serving");
  
  DEFINE_bool(reset,false,"Reset index and remove all documents");

  DEFINE_string(template_path,"templates","Path where HTML/JSON templates are located");

  DEFINE_bool(debug_templates,false,"Forces template reload for every template file change");
  
  DEFINE_bool(debug,false,"Outputs debug information to specified log_file");


  uint32_t FlagsRegistry::getHashWidth() const{
    return FLAGS_hash_width;
  }

  uint32_t FlagsRegistry::getHashMask() const{
    return (1L<<getHashWidth())-1;
  };

  uint32_t FlagsRegistry::getWhiteSpaceHash(bool posting) const{
    uint32_t hash= WhiteSpaceHash(posting?getPostingWindowSize():getWindowSize());
    if (posting)
      return((hash>>getHashWidth())^(hash&getHashMask()));
    return hash;
  }
  
  uint32_t FlagsRegistry::getWhiteSpaceThreshold() const{
    return getWindowSize()*FLAGS_white_space_threshold; 
  }
  
  uint32_t FlagsRegistry::getWindowSize() const{
    return FLAGS_window_size;
  };
  
  // This is shorter than window size to remove false positives in the search algorithm
  // 7 is the magic number of bytes used in the algorithm, which seems to map to 6 (TODO!)
  uint32_t FlagsRegistry::getPostingWindowSize() const{
    return getWindowSize()-6;
  };
  
  uint32_t FlagsRegistry::getThreadCount() const{
    return FLAGS_thread_count;
  };
  
  uint32_t FlagsRegistry::getSlotCount() const{
    return FLAGS_slot_count;
  };
  
  size_t FlagsRegistry::getPageSize() const{
    return 100;
  };
  
  size_t FlagsRegistry::getNumResults() const{
    return FLAGS_num_results;
  };
  
  size_t FlagsRegistry::getMaxLineLength() const{
    return 1<<10;
  };
  
  size_t FlagsRegistry::getMaxHashCount() const{
    return 1<<getHashWidth();
  };
  
  size_t FlagsRegistry::getMaxBatchCount() const{
    return 20000;
  };

  size_t FlagsRegistry::getMaxPostingThreshold() const{
    return FLAGS_max_posting_threshold;
  }
  
  size_t FlagsRegistry::getMaxDistance() const{
    return 1;
  };
  
  double FlagsRegistry::getTimeout() const{
    return 10.0;
  };
  
  string FlagsRegistry::getDataPath() const{
    return FLAGS_data_path;
  };
  
  string FlagsRegistry::getPublicPath() const{
    return FLAGS_public_path;
  };
  
  string FlagsRegistry::getAddress() const{
    return FLAGS_address;
  };
  
  uint32_t FlagsRegistry::getPort() const{
    return FLAGS_port;
  };
  
  bool FlagsRegistry::isDaemonized() const{
    return FLAGS_daemonize;
  }
  
  string FlagsRegistry::getLogFile() const{
    return FLAGS_log_file;
  }
  
  uint32_t FlagsRegistry::getMode(){
    return kc::PolyDB::OWRITER|kc::PolyDB::OCREATE|(FLAGS_reset?(kc::PolyDB::OTRUNCATE):0);
  };
  
  kc::PolyDB* FlagsRegistry::getQueueDB(){
    return queueDB_;
  };
  
  kc::PolyDB* FlagsRegistry::getPayloadDB(){
    return payloadDB_;
  };
  
  kc::PolyDB* FlagsRegistry::getDocumentDB(){
    return documentDB_;
  };

  kc::PolyDB* FlagsRegistry::getMetaDB(){
    return metaDB_;
  };

  kc::PolyDB* FlagsRegistry::getOrderedMetaDB(){
    return orderedMetaDB_;
  };

  kc::PolyDB* FlagsRegistry::getAssociationDB(){
    return associationDB_;
  };

  kc::PolyDB* FlagsRegistry::getMiscDB(){
    return miscDB_;
  };

  TemplateCache* FlagsRegistry::getTemplateCache(){
    if (FLAGS_debug_templates){
      templates_->ReloadAllIfChanged(TemplateCache::LAZY_RELOAD); 
    }
    return templates_;
  };

  Logger* FlagsRegistry::getLogger(){
    return logger_;
  };

  Posting* FlagsRegistry::getPostings(){
    return postings_;
  };
  
  DocumentManager* FlagsRegistry::getDocumentManager(){
    return documentManager_;
  }
  
  AssociationManager* FlagsRegistry::getAssociationManager(){
    return associationManager_;
  }
  
  QueueManager* FlagsRegistry::getQueueManager(){
    return queueManager_;
  }

  InstrumentGroupPtr FlagsRegistry::getInstrumentGroup(const int32_t group){
    return instruments_[group];
  }

  FlagsRegistry::FlagsRegistry():
  queueDB_(new kc::PolyDB()),
  payloadDB_(new kc::PolyDB()),
  documentDB_(new kc::PolyDB()),
  metaDB_(new kc::PolyDB()),
  orderedMetaDB_(new kc::PolyDB()),
  associationDB_(new kc::PolyDB()),
  miscDB_(new kc::PolyDB()),
  templates_(mutable_default_template_cache()),
  logger_(new Logger(FLAGS_debug)),
  postings_(0),
  documentManager_(0),
  associationManager_(0),
  queueManager_(0),
  isClosing_(false)
  {
    logger_->open(getLogFile().c_str());
    if (isDaemonized() && !daemonize()){
      logger_->log(Logger::ERROR, "Failed to daemonize!");
    }
    if (!openDatabases()){
      logger_->log(Logger::ERROR,"Error opening databases");
    }
    instruments_.push_back(InstrumentGroupPtr(new InstrumentGroup("Worker Instruments",100,20)));
    instruments_.push_back(InstrumentGroupPtr(new InstrumentGroup("Queue Instruments",100,20)));
    instruments_.push_back(InstrumentGroupPtr(new InstrumentGroup("Index Instruments",100,20)));
    postings_ = new Posting(this);
    documentManager_ = new DocumentManager(this);
    associationManager_ = new AssociationManager(this);
    queueManager_ = new QueueManager(this);
    templates_->SetTemplateRootDirectory(FLAGS_template_path);
  }

  FlagsRegistry::~FlagsRegistry(){
    if (queueDB_!=0){
      queueDB_->close(); 
    }
    if (payloadDB_!=0){
      payloadDB_->close(); 
    }
    if (documentDB_!=0){
      documentDB_->close();
    }
    if (metaDB_!=0){
      metaDB_->close();
    }
    if (orderedMetaDB_!=0){
      metaDB_->close();
    }
    if (associationDB_!=0){
      associationDB_->close();
    }
    if (miscDB_!=0){
      miscDB_->close();
    }
    if (logger_!=0){
      logger_->close();
    }
    delete documentDB_;
    delete metaDB_;
    delete orderedMetaDB_;
    delete associationDB_;
    delete queueDB_;
    delete payloadDB_;
    delete miscDB_;
    delete postings_;
    delete documentManager_;
    delete associationManager_;
    delete queueManager_;
    delete logger_;
  }
  
  bool FlagsRegistry::isClosing(){
    return isClosing_;
  }
  
  void FlagsRegistry::close(){
    isClosing_=true;
  }
  
  bool FlagsRegistry::openDatabases(){
    uint32_t cache=FLAGS_cache/16;
    return documentDB_->open(getDataPath()+"/document.kch#log=+#logkinds=error#bnum=20m#opts=c#msiz="+toString(cache*12)+"m",getMode()) && \
           queueDB_->open(getDataPath()+"/queue.kct#log=+#logkinds=error#bnum=1m#opts=lc",getMode()) && \
           payloadDB_->open(getDataPath()+"/payload.kch#log=+#logkinds=error#bnum=100k#opts=c#msiz="+toString(cache)+"m",getMode()) && \
           metaDB_->open(getDataPath()+"/meta.kct#log=+#logkinds=error#bnum=1m#opts=lc#msiz="+toString(cache)+"m",getMode()) && \
           orderedMetaDB_->open(getDataPath()+"/orderedmeta.kct#log=+#logkinds=error#bnum=1m#opts=lc#msiz="+toString(cache)+"m",getMode()) && \
           associationDB_->open(getDataPath()+"/association.kct#log=+#logkinds=error#bnum=1m#opts=lc#msiz="+toString(cache)+"m",getMode()) && \
           miscDB_->open(getDataPath()+"/misc.kch#log=+#logkinds=error",getMode());
  }
  
  void FlagsRegistry::fillPerformanceDictionary(TemplateDictionary* dict){
    stringstream s;
    TemplateDictionary* worker_dict = dict->AddSectionDictionary("INSTRUMENT_SET");
    TemplateDictionary* queue_dict = dict->AddSectionDictionary("INSTRUMENT_SET");
    TemplateDictionary* index_dict = dict->AddSectionDictionary("INSTRUMENT_SET");
    s << *getInstrumentGroup(WORKER);
    worker_dict->SetValue("NAME","Worker Instruments");
    worker_dict->SetValue("INSTRUMENTS",s.str());
    s.str(string());
    s << *getInstrumentGroup(QUEUE);
    queue_dict->SetValue("NAME","Queue Instruments");
    queue_dict->SetValue("INSTRUMENTS",s.str());
    s.str(string());
    s << *getInstrumentGroup(INDEX);
    index_dict->SetValue("NAME","Index Instruments");
    index_dict->SetValue("INSTRUMENTS",s.str());
  }
  
  void FlagsRegistry::fillStatusDictionary(TemplateDictionary* dict){
    // TODO: This section needs to be conditional on TCMALLOC being linked and present
    size_t memory=0;
    const int kBufferSize = 16 << 12;
    char* buffer = new char[kBufferSize];
    // MallocExtension::instance()->GetNumericProperty("generic.current_allocated_bytes",&memory);
    // MallocExtension::instance()->GetStats(buffer,kBufferSize);
    dict->SetFormattedValue("MEMORY","%.4f",double(memory)/1024/1024/1024);
    dict->SetValue("MEMORY_STATS",string(buffer));
    delete [] buffer;
    // End TCMALLOC section
    dict->SetIntValue("WHITESPACE_HASH",getWhiteSpaceHash());
    fillDbDictionary(dict,getQueueDB(),"Queue DB");
    fillDbDictionary(dict,getPayloadDB(),"Payload DB");
    fillDbDictionary(dict,getDocumentDB(),"Document DB");
    fillDbDictionary(dict,getMetaDB(),"Meta DB");
    fillDbDictionary(dict,getOrderedMetaDB(),"Ordered Meta DB");
    fillDbDictionary(dict,getAssociationDB(),"Association DB");
    fillDbDictionary(dict,getMiscDB(),"Misc DB");
  }

  void FlagsRegistry::fillDbDictionary(TemplateDictionary* dict, kc::PolyDB* db, const string name){
    stringstream s;
    status(s,db);
    TemplateDictionary* db_dict = dict->AddSectionDictionary("DB");
    db_dict->SetValue("NAME",name);
    db_dict->SetValue("STATS",s.str());       
  }
  
  void FlagsRegistry::status(std::ostream& s, kc::PolyDB* db){
    std::map<std::string, std::string> status;
    status["opaque"] = "";
    status["fbpnum_used"] = "";
    status["bnum_used"] = "";
    status["cusage_lcnt"] = "";
    status["cusage_lsiz"] = "";
    status["cusage_icnt"] = "";
    status["cusage_isiz"] = "";
    status["tree_level"] = "";
    db->status(&status);
    uint32_t type = kc::atoi(status["realtype"].c_str());
    oprintf(s,"type: %s (type=0x%02X) (%s)\n",
          status["type"].c_str(), type, kc::PolyDB::typestring(type));
    uint32_t chksum = kc::atoi(status["chksum"].c_str());
    oprintf(s,"format version: %s (libver=%s.%s) (chksum=0x%02X)\n", status["fmtver"].c_str(),
          status["libver"].c_str(), status["librev"].c_str(), chksum);
    oprintf(s,"path: %s\n", status["path"].c_str());
    int32_t flags = kc::atoi(status["flags"].c_str());
    oprintf(s,"status flags:");
    if (flags & kc::TreeDB::FOPEN) oprintf(s," open");
    if (flags & kc::TreeDB::FFATAL) oprintf(s," fatal");
    oprintf(s," (flags=%d)", flags);
    if (kc::atoi(status["recovered"].c_str()) > 0) oprintf(s," (recovered)");
    if (kc::atoi(status["reorganized"].c_str()) > 0) oprintf(s," (reorganized)");
    if (kc::atoi(status["trimmed"].c_str()) > 0) oprintf(s," (trimmed)");
    oprintf(s,"\n", flags);
    int32_t apow = kc::atoi(status["apow"].c_str());
    oprintf(s,"alignment: %d (apow=%d)\n", 1 << apow, apow);
    int32_t fpow = kc::atoi(status["fpow"].c_str());
    int32_t fbpnum = fpow > 0 ? 1 << fpow : 0;
    int32_t fbpused = kc::atoi(status["fbpnum_used"].c_str());
    int64_t frgcnt = kc::atoi(status["frgcnt"].c_str());
    oprintf(s,"free block pool: %d (fpow=%d) (used=%d) (frg=%lld)\n",
          fbpnum, fpow, fbpused, (long long)frgcnt);
    int32_t opts = kc::atoi(status["opts"].c_str());
    oprintf(s,"options:");
    if (opts & kc::TreeDB::TSMALL) oprintf(s," small");
    if (opts & kc::TreeDB::TLINEAR) oprintf(s," linear");
    if (opts & kc::TreeDB::TCOMPRESS) oprintf(s," compress");
    oprintf(s," (opts=%d)\n", opts);
    oprintf(s,"comparator: %s\n", status["rcomp"].c_str());
    if (status["opaque"].size() >= 16) {
      const char* opaque = status["opaque"].c_str();
      oprintf(s,"opaque:");
    if (std::count(opaque, opaque + 16, 0) != 16) {
      for (int32_t i = 0; i < 16; i++) {
        oprintf(s," %02X", ((unsigned char*)opaque)[i]);
      }
    } else {
      oprintf(s," 0");
    }
    oprintf(s,"\n");
    }
    int64_t bnum = kc::atoi(status["bnum"].c_str());
    int64_t bnumused = kc::atoi(status["bnum_used"].c_str());
    int64_t count = kc::atoi(status["count"].c_str());
    int64_t pnum = kc::atoi(status["pnum"].c_str());
    int64_t lcnt = kc::atoi(status["lcnt"].c_str());
    int64_t icnt = kc::atoi(status["icnt"].c_str());
    int32_t tlevel = kc::atoi(status["tree_level"].c_str());
    int32_t psiz = kc::atoi(status["psiz"].c_str());
    double load = 0;
    if (pnum > 0 && bnumused > 0) {
      load = (double)pnum / bnumused;
      if (!(opts & kc::TreeDB::TLINEAR)) load = std::log(load + 1) / std::log(2.0);
    }
    oprintf(s,"buckets: %lld (used=%lld) (load=%.2f)\n",
          (long long)bnum, (long long)bnumused, load);
    oprintf(s,"pages: %lld (leaf=%lld) (inner=%lld) (level=%d) (psiz=%d)\n",
          (long long)pnum, (long long)lcnt, (long long)icnt, tlevel, psiz);
    int64_t pccap = kc::atoi(status["pccap"].c_str());
    int64_t cusage = kc::atoi(status["cusage"].c_str());
    int64_t culcnt = kc::atoi(status["cusage_lcnt"].c_str());
    int64_t culsiz = kc::atoi(status["cusage_lsiz"].c_str());
    int64_t cuicnt = kc::atoi(status["cusage_icnt"].c_str());
    int64_t cuisiz = kc::atoi(status["cusage_isiz"].c_str());
    oprintf(s,"cache: %lld (cap=%lld) (ratio=%.2f) (leaf=%lld:%lld) (inner=%lld:%lld)\n",
          (long long)cusage, (long long)pccap, (double)cusage / pccap,
          (long long)culsiz, (long long)culcnt, (long long)cuisiz, (long long)cuicnt);
    std::string cntstr = unitnumstr(count);
    oprintf(s,"count: %lld (%s)\n", count, cntstr.c_str());
    int64_t size = kc::atoi(status["size"].c_str());
    int64_t msiz = kc::atoi(status["msiz"].c_str());
    int64_t realsize = kc::atoi(status["realsize"].c_str());
    std::string sizestr = unitnumstrbyte(size);
    oprintf(s,"size: %lld (%s) (map=%lld)", size, sizestr.c_str(), (long long)msiz);
    if (size != realsize) oprintf(s," (gap=%lld)", (long long)(realsize - size));
    oprintf(s,"\n");
  }
}
