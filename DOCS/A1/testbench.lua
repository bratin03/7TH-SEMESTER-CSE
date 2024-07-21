-- This is a testbench . This file is to be used to benchmark Lua ’s GC .
-- It is a simple matrix creation of size m x n .
function test()
    local matrix = {}
    -- Parameters m and n .
    -- Change these values to test the performance of the GC .
    local m = 1000
    local n = 100
    -- Do not change the following code
    for i = 1, m do
        local var = {}
        matrix[i] = var
        for j = 1, n do
            local temp_var = {}
            temp_var[1] = { "123" }
            temp_var[10] = { "hello" }
            local temp_var2 = {}
            temp_var2[1] = { "178" }
            temp_var2[10] = { "world" }
            matrix[i][j] = temp_var
        end
    end
end
