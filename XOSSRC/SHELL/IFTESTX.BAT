if %1 == %2 goto baz
echo %1 and %2 are not equal
goto end

:baz
echo %1 and %2 are equal

:end

