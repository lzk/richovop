#ifndef VERSION
#define VERSION

#ifdef VERSION_DEFINE
const char* vop_name = "Ricoh Virtual Panel";
const char* vop_version = "Version 0.8-11";
const char* copy_right = "Copyright(C) 2016 Ricoh Co., Ltd.";
#else
extern const char* vop_name;
extern const char* vop_version;
extern const char* copy_right;
#endif

#endif // VERSION

