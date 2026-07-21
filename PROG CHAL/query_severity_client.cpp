#include <iostream>
#include <string>
#include <memory>
#include <grpcpp/grpcpp.h>
#include "logger.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using logger::EventLogger;
using logger::SeverityRequest;
using logger::LogList;

// Wrapper class for the Query Client
class SeverityQueryClient {
public:
    SeverityQueryClient(std::shared_ptr<Channel> channel)
        : stub_(EventLogger::NewStub(channel)) {} 

    void GetSeverityLogs(const std::string& severity) {
        // 1. Prepare request payload
        SeverityRequest request;
        request.set_severity(severity);

        // 2. Container for the response
        LogList response;

        // 3. Client Context
        ClientContext context;

        // 4. Invoke RPC
        Status status = stub_->GetSeverityLogs(&context, request, &response); 

        // 5. Handle response and iterate through list
        if (status.ok()) {
            std::cout << "\n--- Consolidated Logs for Severity: " << severity << " ---" << std::endl;
            if (response.logs_size() == 0) {
                std::cout << "No logs found for this severity level." << std::endl;
            } else {
                for (int i = 0; i < response.logs_size(); i++) {
                    const logger::LogEntry& log = response.logs(i);
                    // Explicitly display node_name alongside timestamp and message
                    std::cout << "[" << log.timestamp() << "] " 
                              << "NODE: " << log.node_name() << " | "
                              << "MSG: " << log.message() << std::endl;
                }
            }
        } else {
            std::cout << ">> gRPC error: " << status.error_code() 
                      << ": " << status.error_message() << std::endl;
        }
    }

private:
    std::unique_ptr<EventLogger::Stub> stub_;
};

int main(int argc, char** argv) {
    std::string server_ip;
    std::cout << "Enter the gRPC server IP (e.g., localhost:50051): ";
    std::getline(std::cin, server_ip);

    // Create channel
    SeverityQueryClient client(grpc::CreateChannel(server_ip, grpc::InsecureChannelCredentials()));

    bool not_quit = true; 
    while (not_quit) {
        std::cout << "\n--- Global Severity Query Tool (C++ gRPC) ---" << std::endl;

        std::string severity;
        std::cout << "Enter target severity level [INFO, WARN, ERROR]: "; 
        std::getline(std::cin, severity);

        // Trigger remote query
        client.GetSeverityLogs(severity);

        std::string key_in;
        std::cout << "\nDo you want to run another query? [y/n]: "; 
        std::getline(std::cin, key_in);

        if (key_in != "y" && key_in != "Y") { 
            not_quit = false;
        }
    }

    return 0;
}