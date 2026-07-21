import grpc
import logger_pb2
import logger_pb2_grpc

def main():
    # Open a gRPC channel to the server (Update IP as needed for remote machines)
    server_ip = input("Enter the gRPC server IP (e.g., localhost:50051): ").strip()
    channel = grpc.insecure_channel(server_ip) 

    # Create the stub (client proxy)
    stub = logger_pb2_grpc.EventLoggerStub(channel) 

    node_name = input("Enter this node's identifier (e.g., WebServer_1): ").strip()
    not_quit = True 

    while not_quit:
        print("\n--- Distributed Event Reporter (gRPC) ---")
        severity = input("Enter event severity [INFO, WARN, ERROR]: ").strip()
        message = input("Enter the event message: ").strip() 

        try:
            # Package the inputs into a LogRequest message and call the RPC
            request = logger_pb2.LogRequest(
                node_name=node_name,
                severity=severity,
                message=message
            )
            response = stub.LogEvent(request) 

            if response.success:
                print(">> Event successfully transmitted to the central logger.") 

        except grpc.RpcError as e:
            print(f">> Connection or execution error: {e.details()}") 

        key_in = input("\nDo you want to report another event? [y/n]: ").strip()
        if key_in.lower() != 'y':
            not_quit = False 

if __name__ == '__main__':
    main()
