# For the Analytical Placer:

https://github.com/theebank/Analytical-Placer/tree/main

# To run:
Unzip the folder

CD into the folder 

Run the following command:

    make ./visualization

Once the executable file is created, run it like so:

    visualization

Note: the window may take some time to open depending on the chosen test file


## Choosing a test circuit:

alter line 110 

    g.processFile("tests/cct1.txt");

to whichever test file that is desired

## Swapping between Baseline and Swappable input:

Line 113 starts the baseline router
Line 114 starts the swappable input router

    g.mazeRouter(); //Line 113
    g.si_mazeRouter(); //Line 114

Comment out one or the other to swap between inputs
