## Purpose

`graphite-texel-plugins` is a suite of plug-ins offering tools for inspecting and interacting with tet/hex meshes.

## Prerequisites

 - Graphite >= `v3-1.8.8`
 - C++ >= 17
 - Git

## Installation

To get the plug-in suite up and running, you first need to get and compile Graphite. To do this, please follow the instructions on the official Graphite github: 
https://github.com/BrunoLevy/GraphiteThree. Note that you should follow the instructions for `Compiling sources` whether you're running Windows, Mac or Linux: 

 - https://github.com/BrunoLevy/GraphiteThree/wiki/compiling_Windows
 - https://github.com/BrunoLevy/GraphiteThree/wiki/compiling_Linux

Once the graphite installation is complete: 

 - Go to the `plugins/OGF` directory
 - Clone this repository `git clone https://github.com/ultimaille/graphite-texel-plugins`
 - Go to the root directory of Graphite
 - Re-configure graphite project following your OS instructions
 - Re-compile graphite !



If everything goes well, you should see the following messages printed on your console after graphite starts up:

```sh
Loading module: Inspect
Loading module: Interactions
```

## What's inside ?

### Tools

 - Tranform tool: move points in a more friendly way
 - N-Ring: visualize neighborhood of a cell in a volume mesh


### Commands

 - Conditional cell filter: filter cells by attribute value using a comparator

