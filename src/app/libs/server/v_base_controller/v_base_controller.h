#ifndef BASE_CONTROLLER_H
#define BASE_CONTROLLER_H
#include "../v_request/v_request.h"
class VServer;

class VBaseController {
private:

protected:
    VServer *server;
public:
    explicit VBaseController(VServer *server):server(server) {}
    virtual void initRoutes() = 0;
    virtual void tick() {}
    virtual boolean authorise(VRequest*) {
        return true;
    }
    virtual ~VBaseController(){}
};

#endif