#How to run fc_exe

`./fc_exe file1 file2 --segments --headers --hdu`

--segments = compare pixels by segments.
 Will let you know if the segments are equal to one another between
 files or not

--headers = compare headers.
 Will let you know if the headers are equal to one another between files
 and if not, will print out the keys in each hdu that are not equal

--hdu = compare pixels by hdu.
 Will let you know if the hdus are equal to one another between files 
 or not.
