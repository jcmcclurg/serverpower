Aydan Wynos

Single Channel Acquisition for Agilent Oscilloscope

This program connects to and controls an Agilent Oscilloscope. It allows the user to change
the horizontal and vertical range, as well as capture data from a single Oscilloscope
channel using the :DIGitize command. The program then reads the captured data from the
scope and saves the data into a text file.


In order to connect with the oscilloscope, you must first download and install the
Keysight IO Libraries Suite. The link is provided below.

http://www.keysight.com/en/pd-1985909/io-libraries-suite?&cc=US&lc=eng


Next, you must reference the VISA COM IO Library from Visual Studio. To do this:

1. Choose Project > Add Reference
2. Under COM Type Libraries, find and check VISA COM 5.5 Type Library, then click OK


Each scope has a different VISA address, so make sure to change the address in the code
to match the corresponding scope before running the program.