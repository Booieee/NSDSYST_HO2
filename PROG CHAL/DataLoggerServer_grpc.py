import grpc
from concurrent import futures
import time
import datetime
import logger_pb2
import logger_pb2_grpc

class EventLoggerServicer(logger_pb2_grpc.EventLoggerServicer):
    def __init__(self):
        # Node tracking and log storage
        self.nodes = {}
        self.logs = {}
        print("Distributed gRPC Event Logger initialized.") 

    def LogEvent(self, request, context):
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S") 

        # 1. Register or update the node metadata
        if request.node_name not in self.nodes:
            self.nodes[request.node_name] = {
                "first_seen": timestamp,
                "last_active": timestamp,
                "total_logs": 0
            }
            self.logs[request.node_name] = []
            print(f"\n[!] New Node Registered: {request.node_name}")
        else:
            self.nodes[request.node_name]["last_active"] = timestamp 
        
        self.nodes[request.node_name]["total_logs"] += 1

        # 2. Store the structured log entry
        log_entry = {
            "timestamp": timestamp,
            "severity": request.severity.upper(),
            "message": request.message,
            "node_name": request.node_name
        }
        self.logs[request.node_name].append(log_entry)

        # 3. Print the server status
        print(f"\n--- New Event Received ---")
        print(f"Node: {request.node_name} | Severity: {request.severity.upper()} | Message: {request.message}")
        print(f"Active Node Registry: {self.nodes}") 

        # Return a LogResponse object confirming success
        return logger_pb2.LogResponse(success=True)

    def GetNodeLogs(self, request, context):
        print(f"\n--- Log Request Received for Node: {request.node_name} ---") 

        log_list = []
        if request.node_name in self.logs:
            # Convert internal dictionaries into gRPC LogEntry objects
            for entry in self.logs[request.node_name]:
                log_list.append(logger_pb2.LogEntry(
                    timestamp=entry["timestamp"],
                    severity=entry["severity"],
                    message=entry["message"],
                    node_name=entry["node_name"]
                ))

        # Return the repeated LogEntry message
        return logger_pb2.LogList(logs=log_list)

    # NEW: Implement the cross-node severity query
    def GetSeverityLogs(self, request, context):
        target_severity = request.severity.upper()
        print(f"\n--- Global Severity Query Received: {target_severity} ---") 

        log_list = []
        # Iterate through all node_name keys and their lists of logs
        for node, logs in self.logs.items():
            for entry in logs:
                # Filter by the target severity
                if entry["severity"] == target_severity:
                    log_list.append(logger_pb2.LogEntry(
                        timestamp=entry["timestamp"],
                        severity=entry["severity"],
                        message=entry["message"],
                        node_name=entry["node_name"]
                    ))

        # Return the consolidated list
        return logger_pb2.LogList(logs=log_list)

def serve():
    # Set up the gRPC server using a thread pool
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    logger_pb2_grpc.add_EventLoggerServicer_to_server(EventLoggerServicer(), server)

    # Bind to port 50051 on all available IP addresses
    server.add_insecure_port('[::]:50051')
    server.start()

    print("gRPC Central Logging Server Ready. Listening on port 50051...")
    try:
        while True:
            time.sleep(86400) # Keep thread alive
    except KeyboardInterrupt:
        server.stop(0)

if __name__ == '__main__':
    serve()