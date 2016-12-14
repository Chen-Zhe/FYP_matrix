// Generated by the gRPC protobuf plugin.
// If you make any local change, they will be lost.
// source: google/cloud/dataproc/v1/jobs.proto

#include "google/cloud/dataproc/v1/jobs.pb.h"
#include "google/cloud/dataproc/v1/jobs.grpc.pb.h"

#include <grpc++/impl/codegen/async_stream.h>
#include <grpc++/impl/codegen/async_unary_call.h>
#include <grpc++/impl/codegen/channel_interface.h>
#include <grpc++/impl/codegen/client_unary_call.h>
#include <grpc++/impl/codegen/method_handler_impl.h>
#include <grpc++/impl/codegen/rpc_service_method.h>
#include <grpc++/impl/codegen/service_type.h>
#include <grpc++/impl/codegen/sync_stream.h>
namespace google {
namespace cloud {
namespace dataproc {
namespace v1 {

static const char* JobController_method_names[] = {
  "/google.cloud.dataproc.v1.JobController/SubmitJob",
  "/google.cloud.dataproc.v1.JobController/GetJob",
  "/google.cloud.dataproc.v1.JobController/ListJobs",
  "/google.cloud.dataproc.v1.JobController/CancelJob",
  "/google.cloud.dataproc.v1.JobController/DeleteJob",
};

std::unique_ptr< JobController::Stub> JobController::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  std::unique_ptr< JobController::Stub> stub(new JobController::Stub(channel));
  return stub;
}

JobController::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_SubmitJob_(JobController_method_names[0], ::grpc::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetJob_(JobController_method_names[1], ::grpc::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_ListJobs_(JobController_method_names[2], ::grpc::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_CancelJob_(JobController_method_names[3], ::grpc::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_DeleteJob_(JobController_method_names[4], ::grpc::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status JobController::Stub::SubmitJob(::grpc::ClientContext* context, const ::google::cloud::dataproc::v1::SubmitJobRequest& request, ::google::cloud::dataproc::v1::Job* response) {
  return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_SubmitJob_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::google::cloud::dataproc::v1::Job>* JobController::Stub::AsyncSubmitJobRaw(::grpc::ClientContext* context, const ::google::cloud::dataproc::v1::SubmitJobRequest& request, ::grpc::CompletionQueue* cq) {
  return new ::grpc::ClientAsyncResponseReader< ::google::cloud::dataproc::v1::Job>(channel_.get(), cq, rpcmethod_SubmitJob_, context, request);
}

::grpc::Status JobController::Stub::GetJob(::grpc::ClientContext* context, const ::google::cloud::dataproc::v1::GetJobRequest& request, ::google::cloud::dataproc::v1::Job* response) {
  return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_GetJob_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::google::cloud::dataproc::v1::Job>* JobController::Stub::AsyncGetJobRaw(::grpc::ClientContext* context, const ::google::cloud::dataproc::v1::GetJobRequest& request, ::grpc::CompletionQueue* cq) {
  return new ::grpc::ClientAsyncResponseReader< ::google::cloud::dataproc::v1::Job>(channel_.get(), cq, rpcmethod_GetJob_, context, request);
}

::grpc::Status JobController::Stub::ListJobs(::grpc::ClientContext* context, const ::google::cloud::dataproc::v1::ListJobsRequest& request, ::google::cloud::dataproc::v1::ListJobsResponse* response) {
  return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_ListJobs_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::google::cloud::dataproc::v1::ListJobsResponse>* JobController::Stub::AsyncListJobsRaw(::grpc::ClientContext* context, const ::google::cloud::dataproc::v1::ListJobsRequest& request, ::grpc::CompletionQueue* cq) {
  return new ::grpc::ClientAsyncResponseReader< ::google::cloud::dataproc::v1::ListJobsResponse>(channel_.get(), cq, rpcmethod_ListJobs_, context, request);
}

::grpc::Status JobController::Stub::CancelJob(::grpc::ClientContext* context, const ::google::cloud::dataproc::v1::CancelJobRequest& request, ::google::cloud::dataproc::v1::Job* response) {
  return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_CancelJob_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::google::cloud::dataproc::v1::Job>* JobController::Stub::AsyncCancelJobRaw(::grpc::ClientContext* context, const ::google::cloud::dataproc::v1::CancelJobRequest& request, ::grpc::CompletionQueue* cq) {
  return new ::grpc::ClientAsyncResponseReader< ::google::cloud::dataproc::v1::Job>(channel_.get(), cq, rpcmethod_CancelJob_, context, request);
}

::grpc::Status JobController::Stub::DeleteJob(::grpc::ClientContext* context, const ::google::cloud::dataproc::v1::DeleteJobRequest& request, ::google::protobuf::Empty* response) {
  return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_DeleteJob_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::google::protobuf::Empty>* JobController::Stub::AsyncDeleteJobRaw(::grpc::ClientContext* context, const ::google::cloud::dataproc::v1::DeleteJobRequest& request, ::grpc::CompletionQueue* cq) {
  return new ::grpc::ClientAsyncResponseReader< ::google::protobuf::Empty>(channel_.get(), cq, rpcmethod_DeleteJob_, context, request);
}

JobController::Service::Service() {
  (void)JobController_method_names;
  AddMethod(new ::grpc::RpcServiceMethod(
      JobController_method_names[0],
      ::grpc::RpcMethod::NORMAL_RPC,
      new ::grpc::RpcMethodHandler< JobController::Service, ::google::cloud::dataproc::v1::SubmitJobRequest, ::google::cloud::dataproc::v1::Job>(
          std::mem_fn(&JobController::Service::SubmitJob), this)));
  AddMethod(new ::grpc::RpcServiceMethod(
      JobController_method_names[1],
      ::grpc::RpcMethod::NORMAL_RPC,
      new ::grpc::RpcMethodHandler< JobController::Service, ::google::cloud::dataproc::v1::GetJobRequest, ::google::cloud::dataproc::v1::Job>(
          std::mem_fn(&JobController::Service::GetJob), this)));
  AddMethod(new ::grpc::RpcServiceMethod(
      JobController_method_names[2],
      ::grpc::RpcMethod::NORMAL_RPC,
      new ::grpc::RpcMethodHandler< JobController::Service, ::google::cloud::dataproc::v1::ListJobsRequest, ::google::cloud::dataproc::v1::ListJobsResponse>(
          std::mem_fn(&JobController::Service::ListJobs), this)));
  AddMethod(new ::grpc::RpcServiceMethod(
      JobController_method_names[3],
      ::grpc::RpcMethod::NORMAL_RPC,
      new ::grpc::RpcMethodHandler< JobController::Service, ::google::cloud::dataproc::v1::CancelJobRequest, ::google::cloud::dataproc::v1::Job>(
          std::mem_fn(&JobController::Service::CancelJob), this)));
  AddMethod(new ::grpc::RpcServiceMethod(
      JobController_method_names[4],
      ::grpc::RpcMethod::NORMAL_RPC,
      new ::grpc::RpcMethodHandler< JobController::Service, ::google::cloud::dataproc::v1::DeleteJobRequest, ::google::protobuf::Empty>(
          std::mem_fn(&JobController::Service::DeleteJob), this)));
}

JobController::Service::~Service() {
}

::grpc::Status JobController::Service::SubmitJob(::grpc::ServerContext* context, const ::google::cloud::dataproc::v1::SubmitJobRequest* request, ::google::cloud::dataproc::v1::Job* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status JobController::Service::GetJob(::grpc::ServerContext* context, const ::google::cloud::dataproc::v1::GetJobRequest* request, ::google::cloud::dataproc::v1::Job* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status JobController::Service::ListJobs(::grpc::ServerContext* context, const ::google::cloud::dataproc::v1::ListJobsRequest* request, ::google::cloud::dataproc::v1::ListJobsResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status JobController::Service::CancelJob(::grpc::ServerContext* context, const ::google::cloud::dataproc::v1::CancelJobRequest* request, ::google::cloud::dataproc::v1::Job* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status JobController::Service::DeleteJob(::grpc::ServerContext* context, const ::google::cloud::dataproc::v1::DeleteJobRequest* request, ::google::protobuf::Empty* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace google
}  // namespace cloud
}  // namespace dataproc
}  // namespace v1
