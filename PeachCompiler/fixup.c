#include "compiler.h"
#include <stdlib.h>
#include "helpers/vector.h"

struct fixup;

/**
 * Returns true if fixup resolution is successful
 */
typedef bool(*FIXUP_FIX)(struct fixup* fixup); // function pointer that will resolve the fixup
/**
 * Signified fixup has been cleaned up from memory. The implementor of this function pointer should free any memory
 * related to the fixup
 */
typedef void(*FIXUP_END)(struct fixup* fixup);

struct fixup_system* fixup_sys_new()
{
    struct fixup_system* system = calloc(1, sizeof(struct fixup_system));
    system->fixups = vector_create(sizeof(struct fixup));
    return system;
}

struct fixup_config* fixup_config(struct fixup* fixup)
{
    return &fixup->config;
}

void fixup_free(struct fixup* fixup)
{
    // Allows implementor to call end function
    fixup->config.end(fixup);
    free(fixup);
}

void fixup_start_iteration(struct fixup_system* system)
{
    vector_set_peek_pointer(system->fixups, 0);
}

struct fixup* fixup_next(struct fixup_system* system)
{
    return vector_peek_ptr(system->fixups);
}

void fixup_system_fixups_free(struct fixup_system* system)
{
    fixup_start_iteration(system);
    struct fixup* fixup = fixup_next(system);
    while (fixup)
    {
        fixup_free(fixup);
        fixup = fixup_next(system);
    }
}

void fixup_system_free(struct fixup_system* system)
{
    fixup_system_fixups_free(system);
    vector_free(system->fixups);
    free(system);
}

int fixup_system_unresolved_fixups_count(struct fixup_system* system)
{
    size_t c = 0;
    fixup_start_iteration(system);
    struct fixup* fixup = fixup_next(system);
    while (fixup)
    {
        if (fixup->flags & FIXUP_FLAG_RESOLVED)
        {
            fixup = fixup_next(system);
            continue;
        }
        c++;
        fixup = fixup_next(system);
    }
    return c;
}

struct fixup* fixup_register(struct fixup_system* system, struct fixup_config* config)
{
    struct fixup* fixup = calloc(1, sizeof(struct fixup));
    memcpy(&fixup->config, config, sizeof(struct fixup_config));
    fixup->system = system;
    vector_push(system->fixups, fixup);
    return fixup;
}

bool fixup_resolve(struct fixup* fixup)
{
    if (fixup_config(fixup)->fix(fixup))
    {
        fixup->flags |= FIXUP_FLAG_RESOLVED;
        return true;
    }
    return false;
}

void* fixup_private(struct fixup* fixup)
{
    return fixup_config(fixup)->private;
}

bool fixups_resolve(struct fixup_system* system)
{
    fixup_start_iteration(system);
    struct fixup* fixup = fixup_next(system);
    while (fixup)
    {
        if (fixup->flags & FIXUP_FLAG_RESOLVED)
        {
            continue;
        }
        fixup_resolve(fixup);
        fixup = fixup_next(system);
    }

    return fixup_system_unresolved_fixups_count(system) == 0;
}