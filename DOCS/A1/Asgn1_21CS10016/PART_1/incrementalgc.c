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
 * Run incremental garbage collection and execute the 'test' function
 *
 * @param L The Lua state
 *
 * @return void
 */
static void incrementalgc(lua_State *L)
{
    // Start incremental garbage collection
    if (lua_gc(L, LUA_GCINC, LUAI_GCPAUSE, LUAI_GCMUL, LUAI_GCSTEPSIZE) == -1)
    {
        fprintf(stderr, "Error calling lua_gc\n");
        exit(EXIT_FAILURE);
    }

    // Get the global function 'test' from the Lua script
    lua_getglobal(L, "test");

#ifdef DEBUG
    display_memory_stats(L, "Before calling test");
#endif

    // Check if 'test' is a valid function
    if (lua_isfunction(L, -1))
    {
        // Call the 'test' function with 0 arguments and 0 return values
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            fprintf(stderr, "Error running function 'test': %s\n", lua_tostring(L, -1));
            lua_pop(L, 1); // Remove the error message from the stack
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

    // Close the Lua state
    lua_close(L);
}

/**
 * Main function to run the Lua script
 *
 * @param argc Number of command-line arguments
 * @param argv Array of command-line arguments
 *
 * @return int The exit status
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
        lua_pop(L, 1); // Remove the error message from the stack
        lua_close(L);
        return EXIT_FAILURE;
    }

    // Run incremental garbage collection and execute 'test'
    incrementalgc(L);

    return EXIT_SUCCESS;
}
