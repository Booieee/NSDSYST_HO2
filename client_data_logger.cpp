#include <iostream>
#include <string>
#include <memory>

// Include gRPC libraries
#include <grpcpp/grpcpp.h>

// Include the generated Protocol Buffer classes
#include "logger.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using logger::EventLogger;
using logger::LogRequest;
using logger::LogResponse;

// Create a wrapper class for the client
class EventLoggerClient {
public:
    // Initialize the client with a channel to the server
    EventLoggerClient(std::shared_ptr<Channel> channel)
        : stub_(EventLogger::NewStub(channel)) {} 

    // Method to call the remote LogEvent RPC
    bool LogEvent(const std::string& node_name, const std::string& severity, const std::string& message) {
        // 1. Prepare the request payload
        LogRequest request;
        request.set_node_name(node_name);
        request.set_severity(severity);
        request.set_message(message);

        // 2. Container for the server's response
        LogResponse response;

        // 3. Context for the client (used to pass metadata, timeouts, etc.)
        ClientContext context;

        // 4. Actually make the Remote Procedure Call
        Status status = stub_->LogEvent(&context, request, &response); 

        // 5. Handle the result
        if (status.ok()) {
            return response.success();
        } else {
            std::cout << ">> gRPC connection or execution error: "
                      << status.error_code() << ": " << status.error_message()
                      << std::endl;
            return false;
        }
    }

private:
    // The stub acts as the local proxy to the remote server
    std::unique_ptr<EventLogger::Stub> stub_;
};

int main(int argc, char** argv) {
    std::string server_ip;
    std::cout << "Enter the gRPC server IP (e.g., localhost:50051): ";
    std::getline(std::cin, server_ip);

    // Create a connection channel to the server
    EventLoggerClient client(grpc::CreateChannel(server_ip, grpc::InsecureChannelCredentials()));

    std::string node_name;
    std::cout << "Enter this node's identifier (e.g., WebServer_1): "; 
    std::getline(std::cin, node_name);

    bool not_quit = true; 
    while (not_quit) {
        std::cout << "\n--- Distributed Event Reporter (C++ gRPC) ---" << std::endl;

        std::string severity, message;
        std::cout << "Enter event severity [INFO, WARN, ERROR]: "; 
        std::getline(std::cin, severity);

        std::cout << "Enter the event message: "; 
        std::getline(std::cin, message);

        // Invoke the remote method via our wrapper class
        bool success = client.LogEvent(node_name, severity, message); 
        if (success) {
            std::cout << ">> Event successfully transmitted to the central logger." << std::endl;
        }

        std::string key_in;
        std::cout << "\nDo you want to report another event? [y/n]: "; 
        std::getline(std::cin, key_in);

        if (key_in != "y" && key_in != "Y") { 
            not_quit = false;
        }
    }

    return 0;
}
