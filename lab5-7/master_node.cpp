#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <zmq.hpp>
#include <windows.h>

std::map<int, PROCESS_INFORMATION> nodes;
std::map<int, int> parentMap;
std::map<int, std::vector<int>> childMap;

void createNode(int id, int parentId) {
    if (nodes.find(id) != nodes.end()) {
        std::cout << "Error: Already exists" << std::endl;
        return;
    }

    if (parentId != -1 && nodes.find(parentId) == nodes.end()) {
        std::cout << "Error: Parent not found" << std::endl;
        return;
    }

    if (parentId != -1) {
        zmq::context_t context(1);
        zmq::socket_t socket(context, ZMQ_REQ);
        socket.connect("tcp://localhost:" + std::to_string(4000 + parentId));

        zmq::message_t request(5);
        memcpy(request.data(), "ping", 5);
        socket.send(request);

        zmq::message_t reply;
        if (!socket.recv(&reply)) {
            std::cout << "Error: Parent is unavailable" << std::endl;
            return;
        }
    }

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::string command = "computing_node.exe " + std::to_string(id) + " " + std::to_string(parentId);
    if (!CreateProcess(NULL, const_cast<char*>(command.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cout << "Error: Failed to create process" << std::endl;
        return;
    }

    nodes[id] = pi;
    parentMap[id] = parentId;
    if (parentId != -1) {
        childMap[parentId].push_back(id);
    }
    std::cout << "Ok: " << pi.dwProcessId << std::endl;
}

void execCommand(int id, const std::string& params) {
    if (nodes.find(id) == nodes.end()) {
        std::cout << "Error:id: Not found" << std::endl;
        return;
    }

    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REQ);
    socket.connect("tcp://localhost:" + std::to_string(4000 + id));

    zmq::message_t request(params.size());
    memcpy(request.data(), params.data(), params.size());
    socket.send(request);

    zmq::message_t reply;
    if (!socket.recv(&reply)) {
        std::cout << "Error:id: Node is unavailable" << std::endl;
        return;
    }

    std::string result(static_cast<char*>(reply.data()), reply.size());
    std::cout << "Ok:id: " << result << std::endl;
}

void pingAll() {
    std::string unavailableNodes;
    for (auto& node : nodes) {
        zmq::context_t context(1);
        zmq::socket_t socket(context, ZMQ_REQ);
        socket.connect("tcp://localhost:" + std::to_string(4000 + node.first));

        zmq::message_t request(5);
        memcpy(request.data(), "ping", 5);
        socket.send(request);

        zmq::message_t reply;
        if (!socket.recv(&reply)) {
            if (!unavailableNodes.empty()) {
                unavailableNodes += ";";
            }
            unavailableNodes += std::to_string(node.first);
        }
    }
    std::cout << unavailableNodes << std::endl;
}

int main() {
    std::string command;
    while (std::getline(std::cin, command)) {
        if (command.find("create") == 0) {
            int id, parentId;
            sscanf(command.c_str(), "create %d %d", &id, &parentId);
            createNode(id, parentId);
        } else if (command.find("exec") == 0) {
            int id;
            char params[256];
            sscanf(command.c_str(), "exec %d %[^\n]", &id, params);
            execCommand(id, std::string(params));
        } else if (command == "pingall") {
            pingAll();
        }
    }
    return 0;
}