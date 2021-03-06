// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: google/appengine/v1/service.proto

#ifndef PROTOBUF_google_2fappengine_2fv1_2fservice_2eproto__INCLUDED
#define PROTOBUF_google_2fappengine_2fv1_2fservice_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3001000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3001000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/map.h>
#include <google/protobuf/map_field_inl.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
#include "google/api/annotations.pb.h"
// @@protoc_insertion_point(includes)

namespace google {
namespace appengine {
namespace v1 {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_google_2fappengine_2fv1_2fservice_2eproto();
void protobuf_InitDefaults_google_2fappengine_2fv1_2fservice_2eproto();
void protobuf_AssignDesc_google_2fappengine_2fv1_2fservice_2eproto();
void protobuf_ShutdownFile_google_2fappengine_2fv1_2fservice_2eproto();

class Service;
class TrafficSplit;

enum TrafficSplit_ShardBy {
  TrafficSplit_ShardBy_UNSPECIFIED = 0,
  TrafficSplit_ShardBy_COOKIE = 1,
  TrafficSplit_ShardBy_IP = 2,
  TrafficSplit_ShardBy_TrafficSplit_ShardBy_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  TrafficSplit_ShardBy_TrafficSplit_ShardBy_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool TrafficSplit_ShardBy_IsValid(int value);
const TrafficSplit_ShardBy TrafficSplit_ShardBy_ShardBy_MIN = TrafficSplit_ShardBy_UNSPECIFIED;
const TrafficSplit_ShardBy TrafficSplit_ShardBy_ShardBy_MAX = TrafficSplit_ShardBy_IP;
const int TrafficSplit_ShardBy_ShardBy_ARRAYSIZE = TrafficSplit_ShardBy_ShardBy_MAX + 1;

const ::google::protobuf::EnumDescriptor* TrafficSplit_ShardBy_descriptor();
inline const ::std::string& TrafficSplit_ShardBy_Name(TrafficSplit_ShardBy value) {
  return ::google::protobuf::internal::NameOfEnum(
    TrafficSplit_ShardBy_descriptor(), value);
}
inline bool TrafficSplit_ShardBy_Parse(
    const ::std::string& name, TrafficSplit_ShardBy* value) {
  return ::google::protobuf::internal::ParseNamedEnum<TrafficSplit_ShardBy>(
    TrafficSplit_ShardBy_descriptor(), name, value);
}
// ===================================================================

class Service : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:google.appengine.v1.Service) */ {
 public:
  Service();
  virtual ~Service();

  Service(const Service& from);

  inline Service& operator=(const Service& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Service& default_instance();

  static const Service* internal_default_instance();

  void Swap(Service* other);

  // implements Message ----------------------------------------------

  inline Service* New() const { return New(NULL); }

  Service* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Service& from);
  void MergeFrom(const Service& from);
  void Clear();
  bool IsInitialized() const;

  size_t ByteSizeLong() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(Service* other);
  void UnsafeMergeFrom(const Service& from);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional string name = 1;
  void clear_name();
  static const int kNameFieldNumber = 1;
  const ::std::string& name() const;
  void set_name(const ::std::string& value);
  void set_name(const char* value);
  void set_name(const char* value, size_t size);
  ::std::string* mutable_name();
  ::std::string* release_name();
  void set_allocated_name(::std::string* name);

  // optional string id = 2;
  void clear_id();
  static const int kIdFieldNumber = 2;
  const ::std::string& id() const;
  void set_id(const ::std::string& value);
  void set_id(const char* value);
  void set_id(const char* value, size_t size);
  ::std::string* mutable_id();
  ::std::string* release_id();
  void set_allocated_id(::std::string* id);

  // optional .google.appengine.v1.TrafficSplit split = 3;
  bool has_split() const;
  void clear_split();
  static const int kSplitFieldNumber = 3;
  const ::google::appengine::v1::TrafficSplit& split() const;
  ::google::appengine::v1::TrafficSplit* mutable_split();
  ::google::appengine::v1::TrafficSplit* release_split();
  void set_allocated_split(::google::appengine::v1::TrafficSplit* split);

  // @@protoc_insertion_point(class_scope:google.appengine.v1.Service)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr name_;
  ::google::protobuf::internal::ArenaStringPtr id_;
  ::google::appengine::v1::TrafficSplit* split_;
  mutable int _cached_size_;
  friend void  protobuf_InitDefaults_google_2fappengine_2fv1_2fservice_2eproto_impl();
  friend void  protobuf_AddDesc_google_2fappengine_2fv1_2fservice_2eproto_impl();
  friend void protobuf_AssignDesc_google_2fappengine_2fv1_2fservice_2eproto();
  friend void protobuf_ShutdownFile_google_2fappengine_2fv1_2fservice_2eproto();

  void InitAsDefaultInstance();
};
extern ::google::protobuf::internal::ExplicitlyConstructed<Service> Service_default_instance_;

// -------------------------------------------------------------------

class TrafficSplit : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:google.appengine.v1.TrafficSplit) */ {
 public:
  TrafficSplit();
  virtual ~TrafficSplit();

  TrafficSplit(const TrafficSplit& from);

  inline TrafficSplit& operator=(const TrafficSplit& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const TrafficSplit& default_instance();

  static const TrafficSplit* internal_default_instance();

  void Swap(TrafficSplit* other);

  // implements Message ----------------------------------------------

  inline TrafficSplit* New() const { return New(NULL); }

  TrafficSplit* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const TrafficSplit& from);
  void MergeFrom(const TrafficSplit& from);
  void Clear();
  bool IsInitialized() const;

  size_t ByteSizeLong() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(TrafficSplit* other);
  void UnsafeMergeFrom(const TrafficSplit& from);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------


  typedef TrafficSplit_ShardBy ShardBy;
  static const ShardBy UNSPECIFIED =
    TrafficSplit_ShardBy_UNSPECIFIED;
  static const ShardBy COOKIE =
    TrafficSplit_ShardBy_COOKIE;
  static const ShardBy IP =
    TrafficSplit_ShardBy_IP;
  static inline bool ShardBy_IsValid(int value) {
    return TrafficSplit_ShardBy_IsValid(value);
  }
  static const ShardBy ShardBy_MIN =
    TrafficSplit_ShardBy_ShardBy_MIN;
  static const ShardBy ShardBy_MAX =
    TrafficSplit_ShardBy_ShardBy_MAX;
  static const int ShardBy_ARRAYSIZE =
    TrafficSplit_ShardBy_ShardBy_ARRAYSIZE;
  static inline const ::google::protobuf::EnumDescriptor*
  ShardBy_descriptor() {
    return TrafficSplit_ShardBy_descriptor();
  }
  static inline const ::std::string& ShardBy_Name(ShardBy value) {
    return TrafficSplit_ShardBy_Name(value);
  }
  static inline bool ShardBy_Parse(const ::std::string& name,
      ShardBy* value) {
    return TrafficSplit_ShardBy_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  // optional .google.appengine.v1.TrafficSplit.ShardBy shard_by = 1;
  void clear_shard_by();
  static const int kShardByFieldNumber = 1;
  ::google::appengine::v1::TrafficSplit_ShardBy shard_by() const;
  void set_shard_by(::google::appengine::v1::TrafficSplit_ShardBy value);

  // map<string, double> allocations = 2;
  int allocations_size() const;
  void clear_allocations();
  static const int kAllocationsFieldNumber = 2;
  const ::google::protobuf::Map< ::std::string, double >&
      allocations() const;
  ::google::protobuf::Map< ::std::string, double >*
      mutable_allocations();

  // @@protoc_insertion_point(class_scope:google.appengine.v1.TrafficSplit)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  typedef ::google::protobuf::internal::MapEntryLite<
      ::std::string, double,
      ::google::protobuf::internal::WireFormatLite::TYPE_STRING,
      ::google::protobuf::internal::WireFormatLite::TYPE_DOUBLE,
      0 >
      TrafficSplit_AllocationsEntry;
  ::google::protobuf::internal::MapField<
      ::std::string, double,
      ::google::protobuf::internal::WireFormatLite::TYPE_STRING,
      ::google::protobuf::internal::WireFormatLite::TYPE_DOUBLE,
      0 > allocations_;
  int shard_by_;
  mutable int _cached_size_;
  friend void  protobuf_InitDefaults_google_2fappengine_2fv1_2fservice_2eproto_impl();
  friend void  protobuf_AddDesc_google_2fappengine_2fv1_2fservice_2eproto_impl();
  friend void protobuf_AssignDesc_google_2fappengine_2fv1_2fservice_2eproto();
  friend void protobuf_ShutdownFile_google_2fappengine_2fv1_2fservice_2eproto();

  void InitAsDefaultInstance();
};
extern ::google::protobuf::internal::ExplicitlyConstructed<TrafficSplit> TrafficSplit_default_instance_;

// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// Service

// optional string name = 1;
inline void Service::clear_name() {
  name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Service::name() const {
  // @@protoc_insertion_point(field_get:google.appengine.v1.Service.name)
  return name_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Service::set_name(const ::std::string& value) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:google.appengine.v1.Service.name)
}
inline void Service::set_name(const char* value) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:google.appengine.v1.Service.name)
}
inline void Service::set_name(const char* value, size_t size) {
  
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:google.appengine.v1.Service.name)
}
inline ::std::string* Service::mutable_name() {
  
  // @@protoc_insertion_point(field_mutable:google.appengine.v1.Service.name)
  return name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Service::release_name() {
  // @@protoc_insertion_point(field_release:google.appengine.v1.Service.name)
  
  return name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Service::set_allocated_name(::std::string* name) {
  if (name != NULL) {
    
  } else {
    
  }
  name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), name);
  // @@protoc_insertion_point(field_set_allocated:google.appengine.v1.Service.name)
}

// optional string id = 2;
inline void Service::clear_id() {
  id_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Service::id() const {
  // @@protoc_insertion_point(field_get:google.appengine.v1.Service.id)
  return id_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Service::set_id(const ::std::string& value) {
  
  id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:google.appengine.v1.Service.id)
}
inline void Service::set_id(const char* value) {
  
  id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:google.appengine.v1.Service.id)
}
inline void Service::set_id(const char* value, size_t size) {
  
  id_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:google.appengine.v1.Service.id)
}
inline ::std::string* Service::mutable_id() {
  
  // @@protoc_insertion_point(field_mutable:google.appengine.v1.Service.id)
  return id_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Service::release_id() {
  // @@protoc_insertion_point(field_release:google.appengine.v1.Service.id)
  
  return id_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Service::set_allocated_id(::std::string* id) {
  if (id != NULL) {
    
  } else {
    
  }
  id_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), id);
  // @@protoc_insertion_point(field_set_allocated:google.appengine.v1.Service.id)
}

// optional .google.appengine.v1.TrafficSplit split = 3;
inline bool Service::has_split() const {
  return this != internal_default_instance() && split_ != NULL;
}
inline void Service::clear_split() {
  if (GetArenaNoVirtual() == NULL && split_ != NULL) delete split_;
  split_ = NULL;
}
inline const ::google::appengine::v1::TrafficSplit& Service::split() const {
  // @@protoc_insertion_point(field_get:google.appengine.v1.Service.split)
  return split_ != NULL ? *split_
                         : *::google::appengine::v1::TrafficSplit::internal_default_instance();
}
inline ::google::appengine::v1::TrafficSplit* Service::mutable_split() {
  
  if (split_ == NULL) {
    split_ = new ::google::appengine::v1::TrafficSplit;
  }
  // @@protoc_insertion_point(field_mutable:google.appengine.v1.Service.split)
  return split_;
}
inline ::google::appengine::v1::TrafficSplit* Service::release_split() {
  // @@protoc_insertion_point(field_release:google.appengine.v1.Service.split)
  
  ::google::appengine::v1::TrafficSplit* temp = split_;
  split_ = NULL;
  return temp;
}
inline void Service::set_allocated_split(::google::appengine::v1::TrafficSplit* split) {
  delete split_;
  split_ = split;
  if (split) {
    
  } else {
    
  }
  // @@protoc_insertion_point(field_set_allocated:google.appengine.v1.Service.split)
}

inline const Service* Service::internal_default_instance() {
  return &Service_default_instance_.get();
}
// -------------------------------------------------------------------

// TrafficSplit

// optional .google.appengine.v1.TrafficSplit.ShardBy shard_by = 1;
inline void TrafficSplit::clear_shard_by() {
  shard_by_ = 0;
}
inline ::google::appengine::v1::TrafficSplit_ShardBy TrafficSplit::shard_by() const {
  // @@protoc_insertion_point(field_get:google.appengine.v1.TrafficSplit.shard_by)
  return static_cast< ::google::appengine::v1::TrafficSplit_ShardBy >(shard_by_);
}
inline void TrafficSplit::set_shard_by(::google::appengine::v1::TrafficSplit_ShardBy value) {
  
  shard_by_ = value;
  // @@protoc_insertion_point(field_set:google.appengine.v1.TrafficSplit.shard_by)
}

// map<string, double> allocations = 2;
inline int TrafficSplit::allocations_size() const {
  return allocations_.size();
}
inline void TrafficSplit::clear_allocations() {
  allocations_.Clear();
}
inline const ::google::protobuf::Map< ::std::string, double >&
TrafficSplit::allocations() const {
  // @@protoc_insertion_point(field_map:google.appengine.v1.TrafficSplit.allocations)
  return allocations_.GetMap();
}
inline ::google::protobuf::Map< ::std::string, double >*
TrafficSplit::mutable_allocations() {
  // @@protoc_insertion_point(field_mutable_map:google.appengine.v1.TrafficSplit.allocations)
  return allocations_.MutableMap();
}

inline const TrafficSplit* TrafficSplit::internal_default_instance() {
  return &TrafficSplit_default_instance_.get();
}
#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace v1
}  // namespace appengine
}  // namespace google

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::google::appengine::v1::TrafficSplit_ShardBy> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::google::appengine::v1::TrafficSplit_ShardBy>() {
  return ::google::appengine::v1::TrafficSplit_ShardBy_descriptor();
}

}  // namespace protobuf
}  // namespace google
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_google_2fappengine_2fv1_2fservice_2eproto__INCLUDED
