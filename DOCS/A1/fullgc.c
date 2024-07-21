/*
    * Author: Bratin Mondal
    * Roll Number: 21CS10016

    * Department of Computer Science and Engineering
    * Indian Institute of Technology, Kharagpur
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "lgc.h"

// #define DEBUG // Comment this line to disable memory stats

/**
 * Display memory statistics for the Lua state
 *
 * @param state The Lua state
 * @param label The label to display before the stats
 *
 * @return void
 */
void display_memory_stats(lua_State *state, const char *label)
{
    int memory_kb = lua_gc(state, LUA_GCCOUNT, 0);
    int memory_kb_fraction = lua_gc(state, LUA_GCCOUNTB, 0);

    printf("Memory Usage Stats (%s):\n", label);
    printf("----------------------------------------\n");
    printf("Total memory in use: %d.%03d KB\n", memory_kb, memory_kb_fraction);
    printf("\n");
}

/**
 * Run full garbage collection and execute the 'test' function
 *
 * @param L The Lua state
 *
 * @return void
 */
static void fullgc(lua_State *L)
{
    if (lua_gc(L, LUA_GCSTOP, 0) == -1)
    {
        fprintf(stderr, "Error calling lua_gc\n");
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    display_memory_stats(L, "Before calling test");
#endif

    // Check if "test" is a valid Lua function
    lua_getglobal(L, "test");
    if (lua_isfunction(L, -1))
    {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            fprintf(stderr, "Error running function 'test': %s\n", lua_tostring(L, -1));
            lua_pop(L, 1); // Remove error message from the stack
        }
    }
    else
    {
        fprintf(stderr, "'test' is not a valid function\n");
        lua_pop(L, 1); // Remove the non-function from the stack
    }

#ifdef DEBUG
    display_memory_stats(L, "After calling test");
#endif

    // Restart the garbage collector
    if (lua_gc(L, LUA_GCRESTART, 0) == -1)
    {
        fprintf(stderr, "Error calling lua_gc\n");
        exit(EXIT_FAILURE);
    }

    // Collect garbage
    if (lua_gc(L, LUA_GCCOLLECT, 0) == -1)
    {
        fprintf(stderr, "Error calling lua_gc\n");
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    display_memory_stats(L, "After full GC");
#endif

    lua_close(L);
}

/**
 * Main function
 *
 * @param argc The number of command-line arguments
 * @param argv The command-line arguments
 *
 * @return The exit status
 */
int main(int argc, char *argv[])
{
    // Check arguments
    const char *filename = "testbench.lua";
    if (argc > 1)
    {
        filename = argv[1];
    }

    // Initialize Lua
    lua_State *L = luaL_newstate();
    if (L == NULL)
    {
        fprintf(stderr, "Failed to create Lua state\n");
        return EXIT_FAILURE;
    }
    luaL_openlibs(L);

    // Load and run the script
    if (luaL_dofile(L, filename) != LUA_OK)
    {
        fprintf(stderr, "Error loading file '%s': %s\n", filename, lua_tostring(L, -1));
        lua_pop(L, 1); // Remove error message from the stack
        lua_close(L);
        return EXIT_FAILURE;
    }

    // Run fullgc
    fullgc(L);

    return EXIT_SUCCESS;
}
