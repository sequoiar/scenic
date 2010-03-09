#include <string>

class CapsClient {
    public:
    CapsClient(const std::string &host, const std::string &port);

    std::string getCaps();

    private:
    const std::string host_;
    const std::string port_;
};
