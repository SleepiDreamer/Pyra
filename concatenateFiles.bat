@echo off
(
for /r include %%f in (*) do (
    echo ===== %%f =====
    type "%%f"
    echo.
)

for /r source %%f in (*) do (
    echo ===== %%f =====
    type "%%f"
    echo.
)

for /r shaders %%f in (*) do (
    echo ===== %%f =====
    type "%%f"
    echo.
)

) > concatenated.txt
