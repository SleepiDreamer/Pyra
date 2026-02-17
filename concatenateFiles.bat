@echo off
(for /r include %%f in (*) do (
    echo ===== %%f =====
    type "%%f"
    echo.
)) > concatenated.txt

(for /r source %%f in (*) do (
    echo ===== %%f =====
    type "%%f"
    echo.
)) >> concatenated.txt