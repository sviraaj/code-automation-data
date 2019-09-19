Code Automation Data
============================
> Device drivers to use as samples for trying to automate code


### Directory structure 

    .
    ├── modules                 # System software modules along with datsheets 
    |   ├── <sys_sw module>     # System software module Eg: Flash, Sensors, Modem
    |   |	├── datasheets      # Sys SW device datasheets to parse
    |   |	├── lib             # Sys SW library which uses devlib to expose functionality
    |   |	├── samples         # samples using lib
    |   |	├── <sys_sw_devlib> # System software device library
    ├── docs                    # Documentation files (alternatively `doc`)
    ├── tools                   # Tools and utilities
    ├── LICENSE
    └── README.md

### Initial thoughts
- Maintaining lib which uses device libs. Both need to be platform agnostic and the
  platform specific stuff implemented through software hooks. Add samples specific
  to particular platforms.
- Datsheets need to be named according to dev libs.

### TODO
1. Add more modules
2. Make a proper doc structure to maintain readability for automation
3. Decide on device system software architecture
4. Maintain coding preferences files for parsing
5. Fix on datasheet structure
6. Check how to handle incomplete implementations from datasheet
   (For eg: For flash devices only SPI normal mode might be implemented
   and unneccessary implementation left out. Implementations might not have
   all functionality from datasheets. Maybe the automation script can show
   the usage by highlighting the stuff implemented.)

