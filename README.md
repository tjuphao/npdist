# npdist
A lightweight C utility that resolves norming point IDs into absolute track positions and calculates distance using route, segment, and offset mapping. Designed for railway signaling and odometry reference workflows

# Example
```
Format: ./npdist <datafile> NPID_Begin NPID_End
tjuphao@Ton:~/npdist$ ./npdist ./src/sample.dat 2 3
NP 2: R=1 Seg=2 Off=333  => pos=1523
NP 3: R=1 Seg=2 Off=763  => pos=1953
Distance between NP 2 and NP 3 (Route 1) = 430 (same units as Len/Off)
```

**Arguments:**
- `datafile`: Path to the data file containing route/segment/norming point definitions (e.g., `./src/sample.dat`)
- `NPID_Begin`: Norming point ID for the start point (integer)
- `NPID_End`: Norming point ID for the end point (integer)
