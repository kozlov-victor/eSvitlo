#ifndef BASE_CONTROLLER_H
#define BASE_CONTROLLER_H
#include "../v_request/v_request.h"
class VServer;

#define Route(url,Method,Controller,ControllerMethod) \
            server->getRegistry()->registerRoute( \
                url,Method,this, \
                [](void* ctx, VRequest* req, VResponse* resp){ \
                    auto* self = static_cast<Controller*>(ctx); \
                    self->ControllerMethod(req,resp); \
                } \
            );

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