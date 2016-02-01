dvr_auto_test runs a list of tests automatically and reports pass/failure.
The test list is in testlist.txt.

To add a new test,
1. Edit auto_test.c:addTestByName() so your new testDvrTestXXX() can be picked up from testlist.txt
2. Add your test to testlist.txt.
3. cd rockford/unittests/applibs/ocap/dvr; make; 
4. The files (dvr_auto_test, and testlist.txt) will be copied to nexus/bin.


