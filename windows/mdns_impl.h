




#define STRING_ARG_DECL(s) const char *buf_##s, size_t sz_##s
#define STRING_ARG_CALL(s) buf_##s, sz_##s
#define MAKE_STRING(s) std::string  _##s = std::string(buf_##s, sz_##s)

#ifdef __cplusplus
extern "C" {
#endif
extern void call_HandlePTRRecord (const void *p, const void *base, boolean last, STRING_ARG_DECL(service), STRING_ARG_DECL(name));
extern void call_HandleSRVRecord (const void *p, const void *base, boolean last, STRING_ARG_DECL(name), STRING_ARG_DECL(hostname),  int port);
extern void call_HandleARecord   (const void *p, const void *base, boolean last, STRING_ARG_DECL(hostname), STRING_ARG_DECL(address));
extern void call_HandleAAAARecord(const void *p, const void *base, boolean last, STRING_ARG_DECL(hostname), STRING_ARG_DECL(address));
extern void call_HandleTXTRecord (const void *p, const void *base, boolean last, STRING_ARG_DECL(key), STRING_ARG_DECL(value));
extern void call_HandleURecord   (const void *p, const void *base, boolean last);

extern int mdns_query(const void* plugin, const char* service_name);


#ifdef __cplusplus
}
#endif
