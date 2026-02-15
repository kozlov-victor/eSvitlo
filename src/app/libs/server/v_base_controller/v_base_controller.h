#ifndef BASE_CONTROLLER_H
#define BASE_CONTROLLER_H

class VBaseController {
private:
    
public:
    virtual void initRoutes() = 0;
    virtual ~VBaseController(){}
};

#endif