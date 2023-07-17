#include "util/redismodule.h"
#include "util/version.h"

#include "basic_sketch/basic_sketch.h"
#include "hyperbf/hyperbf.h"
#include "calm/calm.h"

#include <assert.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C"
{
#endif

    int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc)
    {
        if (RedisModule_Init(ctx, "RedisSketches", SKETCHES_MODULE_VERSION, REDISMODULE_APIVER_1) !=
            REDISMODULE_OK){
            return REDISMODULE_ERR;
        }

        Basic_Sketch_Module_onLoad<calm>(ctx, argv, argc);
        Basic_Sketch_Module_onLoad<basic_hb>(ctx, argv, argc);
        
        return REDISMODULE_OK;
    }

#ifdef __cplusplus
}
#endif