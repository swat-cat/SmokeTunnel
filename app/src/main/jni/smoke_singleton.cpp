#include "smoke_singleton.h"
bool smoke_singleton::instanceFlag = false;
smoke_singleton* smoke_singleton::single = NULL;
smoke_singleton* smoke_singleton::getInstance()
{
    if(! instanceFlag)
    {
        single = new smoke_singleton();
        instanceFlag = true;
        return single;
    }
    else
    {
        return single;
    }
}

void smoke_singleton::run()
{
    if(smokeInstance==NULL){
        smokeInstance = new smoke();
    }
    smokeInstance->run();
}

void smoke_singleton::stop()
{
    if(smokeInstance!=NULL){
        smokeInstance->close_smoke();
    }

}
