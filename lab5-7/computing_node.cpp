#include <iostream>
#include <zmq.hpp>
#include <windows.h>
#include <sstream>
#include <vector>

int main(int argc, char* argv[]) {
    int id = std::stoi(argv[1]);
    int parentId = std::stoi(argv[2]);

    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_REP);
    socket.bind("tcp://*:" + std::to_string(4000 + id));

    while (true) {
        zmq::message_t request;
        socket.recv(&request);
        std::string message(static_cast<char*>(request.data()), request.size());

        if (message == "ping") {
            zmq::message_t reply(5);
            memcpy(reply.data(), "pong", 5);
            socket.send(reply);
        } else if (message.find("exec") == 0) {
            std::istringstream iss(message);
            std::string cmd;
            int n;
            iss >> cmd >> n;

            std::vector<int> numbers(n);
            for (int i = 0; i < n; ++i) {
                iss >> numbers[i];
            }

            int sum = 0;
            for (int num : numbers) {
                sum += num;
            }

            std::string result = std::to_string(sum);
            zmq::message_t reply(result.size());
            memcpy(reply.data(), result.data(), result.size());
            socket.send(reply);
        }
    }
    return 0;
}