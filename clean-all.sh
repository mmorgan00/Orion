set echo on

echo "Cleaning everything..."

# Engine
make -f "Makefile.engine.linux.mak" clean
ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then
echo "Error:"$ERRORLEVEL && exit
fi

# Testbed
make -f "Makefile.testbed.linux.mak" clean
ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then
echo "Error:"$ERRORLEVEL && exit
fi

echo "All assemblies cleaned successfully." 
