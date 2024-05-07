@ECHO off
SetLocal EnableDelayedExpansion

REM Get list of all .c files
SET cFileNames=
FOR /R %%f in (*.c) do (
    SET cFileNames=!cFileNames! %%f
)

REM Get list of all .h files
SET hFileNames=
FOR /R %%f in (*.h) do (
    SET hFileNames=!hFileNames! %%f
)

clang-format %cFileNames% %hFileNames% src\renderer\vulkan\vulkan_types.inl -i